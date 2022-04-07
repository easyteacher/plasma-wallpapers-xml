/*
    SPDX-FileCopyrightText: 2007 Paolo Capriotti <p.capriotti@gmail.com>
    SPDX-FileCopyrightText: 2007 Aaron Seigo <aseigo@kde.org>
    SPDX-FileCopyrightText: 2008 Petri Damsten <damu@iki.fi>
    SPDX-FileCopyrightText: 2008 Alexis Ménard <darktears31@gmail.com>
    SPDX-FileCopyrightText: 2014 Sebastian Kügler <sebas@kde.org>
    SPDX-FileCopyrightText: 2015 Kai Uwe Broulik <kde@privat.broulik.de>
    SPDX-FileCopyrightText: 2019 David Redondo <kde@david-redondo.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "imagebackend.h"

#include <QFileDialog>
#include <QFileInfo>
#include <QGuiApplication>
#include <QImageReader>
#include <QMimeDatabase>
#include <QPalette>
#include <QScreen>
#include <QStandardPaths>
#include <QUrlQuery>

#include <KConfigGroup>
#include <KIO/CopyJob>
#include <KLocalizedString>
#include <KPackage/Package>
#include <KPackage/PackageLoader>
#include <KSharedConfig>
#include <Solid/Power>

#include <Plasma/Theme>

#include "model/imageproxymodel.h"

ImageBackend::ImageBackend(QObject *parent)
    : QObject(parent)
    , m_targetSize(qGuiApp->primaryScreen()->size() * qGuiApp->primaryScreen()->devicePixelRatio())
{
    connect(&m_xmlTimer, &QTimer::timeout, this, &ImageBackend::modelImageChanged);

    useDefaultImage();
}

ImageBackend::~ImageBackend() noexcept
{
    delete m_dialog;
}

void ImageBackend::classBegin()
{
}

void ImageBackend::componentComplete()
{
    // don't bother loading single image until all properties have settled
    // otherwise we would load a too small image (initial view size) just
    // to load the proper one afterwards etc etc
    m_ready = true;

    setModelImage();

    if (!m_usedInConfig) {
        connect(qGuiApp, &QGuiApplication::paletteChanged, this, &ImageBackend::slotUpdateXmlImage);
    }
}

QUrl ImageBackend::image() const
{
    return m_image;
}

void ImageBackend::setImage(const QUrl &url)
{
    if (m_image == url || url.isEmpty()) {
        return;
    }

    m_image = url;
    Q_EMIT imageChanged();

    setModelImage();
}

QUrl ImageBackend::modelImage() const
{
    return m_modelImage;
}

bool ImageBackend::isTransition() const
{
    return m_xmlTimer.isTransition;
}

QSize ImageBackend::targetSize() const
{
    return m_targetSize;
}

void ImageBackend::setTargetSize(const QSize &size)
{
    if (m_targetSize == size) {
        return;
    }

    m_targetSize = size;

    // The screen resolution may change, so clear the model image to force update.
    m_modelImage.clear();
    setModelImage();

    Q_EMIT targetSizeChanged(size);
}

void ImageBackend::useDefaultImage()
{
    m_image.clear();

    // Try from the look and feel package first, then from the plasma theme
    KPackage::Package lookAndFeelPackage = KPackage::PackageLoader::self()->loadPackage(QStringLiteral("Plasma/LookAndFeel"));
    KConfigGroup cg(KSharedConfig::openConfig(QStringLiteral("kdeglobals")), "KDE");
    const QString packageName = cg.readEntry("LookAndFeelPackage", QString());
    // If empty, it will be the default (currently Breeze)
    if (!packageName.isEmpty()) {
        lookAndFeelPackage.setPath(packageName);
    }

    KConfigGroup lnfDefaultsConfig = KConfigGroup(KSharedConfig::openConfig(lookAndFeelPackage.filePath("defaults")), "Wallpaper");

    const QString image = lnfDefaultsConfig.readEntry("Image", "");
    if (!image.isEmpty()) {
        KPackage::Package package = KPackage::PackageLoader::self()->loadPackage(QStringLiteral("Wallpaper/Images"));
        package.setPath(QStandardPaths::locate(QStandardPaths::GenericDataLocation, QStringLiteral("wallpapers/") + image, QStandardPaths::LocateDirectory));

        if (package.isValid()) {
            m_image = QUrl::fromLocalFile(package.path());
        }
    }

    // Try to get a default from the plasma theme
    if (m_image.isEmpty()) {
        Plasma::Theme theme;
        QString path = theme.wallpaperPath();
        int index = path.indexOf(QLatin1String("/contents/images/"));
        if (index > -1) { // We have file from package -> get path to package
            m_image = QUrl::fromLocalFile(path.left(index));
        } else {
            m_image = QUrl::fromLocalFile(path);
        }
    }

    Q_EMIT imageChanged();
    setModelImage();
}

ImageProxyModel *ImageBackend::imageModel()
{
    if (!m_imageModel) {
        m_imageModel = new ImageProxyModel({}, m_targetSize, this);
        connect(this, &ImageBackend::targetSizeChanged, m_imageModel, &ImageProxyModel::targetSizeChanged);
    }

    return m_imageModel;
}

void ImageBackend::releaseImageModel()
{
    if (m_imageModel) {
        m_imageModel->deleteLater();
        m_imageModel = nullptr;
    }
}

void ImageBackend::setUrl(const QString &_path)
{
    QString path = _path;

    if (_path.startsWith(QLatin1String("file://"))) {
        path.remove(0, 7);
    }

    if (path.startsWith(QStandardPaths::writableLocation(QStandardPaths::TempLocation))) {
        // Drag and drop image
        QFileInfo info(path);
        QDir wallpaperDir(QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + "/wallpapers/");

        const QString wallpaperPath = wallpaperDir.absoluteFilePath(info.fileName());

        if (wallpaperDir.mkpath(wallpaperDir.absolutePath()) && !info.fileName().isEmpty()) {
            KIO::CopyJob *job = KIO::copy(QUrl::fromLocalFile(path), QUrl::fromLocalFile(wallpaperPath), KIO::HideProgressInfo);

            connect(job, &KJob::result, this, &ImageBackend::slotCopyWallpaperResult);
        }

        return;
    }

    QStringList results = imageModel()->addBackground(path);

    m_imageModel->commitAddition();

    if (!m_usedInConfig) {
        releaseImageModel();
    }

    if (results.empty()) {
        return;
    }

    m_image = QUrl(results.constFirst());
    Q_EMIT imageChanged();

    setModelImage();
}

void ImageBackend::showFileDialog()
{
    if (!m_dialog) {
        QString path;
        const QStringList &locations = QStandardPaths::standardLocations(QStandardPaths::PicturesLocation);

        if (!locations.isEmpty()) {
            path = locations.at(0);
        } else {
            // HomeLocation is guaranteed not to be empty.
            path = QStandardPaths::standardLocations(QStandardPaths::HomeLocation).at(0);
        }

        QMimeDatabase db;
        QStringList imageGlobPatterns;
        const auto types = QImageReader::supportedMimeTypes();
        for (const QByteArray &mimeType : types) {
            QMimeType mime(db.mimeTypeForName(mimeType));
            imageGlobPatterns << mime.globPatterns();
        }

        // Support xml format
        imageGlobPatterns << QStringLiteral("*.xml");

        m_dialog = new QFileDialog(nullptr, i18n("Open Image"), path, i18n("Image Files") + " (" + imageGlobPatterns.join(' ') + ')');
        // i18n people, this isn't a "word puzzle". there is a specific string format for QFileDialog::setNameFilters

        m_dialog->setFileMode(QFileDialog::ExistingFiles);
        connect(m_dialog, &QDialog::accepted, this, &ImageBackend::slotWallpaperBrowseCompleted);
    }

    m_dialog->show();
    m_dialog->raise();
    m_dialog->activateWindow();
}

void ImageBackend::setModelImage()
{
    if (!m_ready || m_usedInConfig || m_image.isEmpty()) {
        return;
    }

    // supposedly QSize::isEmpty() is true if "either width or height are >= 0"
    if (!m_targetSize.width() || !m_targetSize.height()) {
        return;
    }

    if (m_image.isLocalFile()) {
        const QFileInfo info(m_image.toLocalFile());

        if (!info.exists()) {
            return;
        }

        if (info.isFile()) {
            m_providerType = Provider::Image;
        } else {
            m_providerType = Provider::Package;
        }
    } else if (m_image.scheme() == QLatin1String("image")) {
        if (m_image.host() == QLatin1String("gnome-wp-list")) {
            m_providerType = Provider::Xml;
        }
    } else {
        // The url can be without file://, try again.
        const QFileInfo info(m_image.toString());

        if (!info.exists()) {
            return;
        }

        if (info.isFile()) {
            m_providerType = Provider::Image;
        } else {
            m_providerType = Provider::Package;
        }

        m_image = QUrl::fromLocalFile(info.filePath());
    }

    switch (m_providerType) {
    case Provider::Image:
        m_modelImage = m_image;
        break;

    case Provider::Package: {
        QUrl url(QStringLiteral("image://package/get"));

        QUrlQuery urlQuery(url);
        urlQuery.addQueryItem(QStringLiteral("dir"), m_image.toLocalFile());

        url.setQuery(urlQuery);
        m_modelImage = url;
        break;
    }

    case Provider::Xml: {
        slotUpdateXmlImage(qGuiApp->palette());
        return;
    }
    }

    if (m_changeConnection) {
        disconnect(m_changeConnection);
    }

    if (m_resumeConnection) {
        disconnect(m_resumeConnection);
    }

    m_xmlTimer.stop();
    m_xmlTimer.isTransition = false;

    if (!m_modelImage.isEmpty()) {
        Q_EMIT modelImageChanged();
    }
}

void ImageBackend::slotUpdateXmlImage(const QPalette &palette)
{
    if (m_providerType != Provider::Xml || !m_ready || m_image.isEmpty()) {
        return;
    }

    // Check dark mode
    const bool useDark = qGray(palette.window().color().rgb()) < 192;

    QUrl url(m_image);
    QUrlQuery urlQuery(url);

    QString filename = useDark ? urlQuery.queryItemValue(QStringLiteral("filename_dark")) : urlQuery.queryItemValue(QStringLiteral("filename"));

    if (filename.isEmpty()) {
        // Dark mode is not available, fall back to light mode
        filename = urlQuery.queryItemValue(QStringLiteral("filename"));
    }

    if (useDark) {
        urlQuery.addQueryItem(QStringLiteral("darkmode"), QString::number(1));
        url.setQuery(urlQuery);
    }

    // CHeck if the timer should be activated
    if (filename.endsWith(QLatin1String(".xml"))) {
        // is slideshow
        m_xmlTimer.adjustInterval(filename);

        if (!m_changeConnection) {
            m_changeConnection = connect(this, &ImageBackend::modelImageChanged, &m_xmlTimer, &XmlSlideshowUpdateTimer::alignInterval);
        }

        // Refresh slideshow after resume
        if (!m_resumeConnection) {
            m_resumeConnection = connect(Solid::Power::self(), &Solid::Power::resumeFromSuspend, this, &ImageBackend::modelImageChanged);
        }
    } else {
        if (m_changeConnection) {
            disconnect(m_changeConnection);
        }

        if (m_resumeConnection) {
            disconnect(m_resumeConnection);
        }
        m_xmlTimer.stop();
        m_xmlTimer.isTransition = false;
    }

    m_modelImage = url;
    Q_EMIT modelImageChanged();
}

void ImageBackend::slotWallpaperBrowseCompleted()
{
    if (!m_dialog || !m_imageModel || m_dialog->selectedFiles().count() == 0) {
        return;
    }

    const QStringList selectedFiles = m_dialog->selectedFiles();

    for (const QString &p : selectedFiles) {
        m_imageModel->addBackground(p);
    }
}

void ImageBackend::slotCopyWallpaperResult(KJob *job)
{
    KIO::CopyJob *copyJob = qobject_cast<KIO::CopyJob *>(job);

    if (!copyJob || copyJob->error()) {
        return;
    }

    QStringList results = imageModel()->addBackground(copyJob->destUrl().toLocalFile());
    m_imageModel->commitAddition();

    if (!m_usedInConfig) {
        releaseImageModel();
    }

    if (results.empty()) {
        return;
    }

    m_image = QUrl(results.at(0));
    Q_EMIT imageChanged();

    setModelImage();
}
