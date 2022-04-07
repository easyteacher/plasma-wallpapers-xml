set(image_SRCS
    imageplugin.cpp
    backend/imagebackend.cpp
    backend/slideshowbackend.cpp
    backend/xmlslideshowupdatetimer.cpp
    finder/imagefinder.cpp
    finder/xmlfinder.cpp
    finder/packagefinder.cpp
    finder/imagesizefinder.cpp
    model/abstractimagelistmodel.cpp
    model/imagelistmodel.cpp
    model/packagelistmodel.cpp
    model/xmlimagelistmodel.cpp
    model/xmlpreviewgenerator.cpp
    model/imageproxymodel.cpp
    model/slidemodel.cpp
    model/slidefiltermodel.cpp
    model/sortingmode.h
    provider/xmlimageprovider.cpp
    provider/packageimageprovider.cpp
)

ecm_qt_declare_logging_category(image_SRCS HEADER debug.h
                                           IDENTIFIER IMAGEWALLPAPER
                                           CATEGORY_NAME kde.wallpapers.image
                                           DEFAULT_SEVERITY Info)

add_library(plasma_xml_wallpaper SHARED ${image_SRCS})

target_link_libraries(plasma_xml_wallpaper
    Qt::Core
    Qt::Quick
    Qt::Qml
    KF5::ConfigCore
    KF5::I18n
    KF5::KIOCore
    KF5::KIOFileWidgets
    KF5::Notifications
    KF5::Package
    KF5::Plasma # Plasma::Theme
    KF5::Solid
)

install(TARGETS plasma_xml_wallpaper DESTINATION ${KDE_INSTALL_QMLDIR}/com/github/easyteacher/plasma/wallpapers/xml)
install(FILES qmldir DESTINATION ${KDE_INSTALL_QMLDIR}/com/github/easyteacher/plasma/wallpapers/xml)
