set(image_SRCS
    imageplugin.cpp
    backend/imagebackend.cpp
    backend/slideshowbackend.cpp
    backend/xmlslideshowupdatetimer.cpp
    backend/finder/imagefinder.cpp
    backend/finder/xmlfinder.cpp
    backend/finder/packagefinder.cpp
    backend/finder/imagesizefinder.cpp
    backend/model/abstractimagelistmodel.cpp
    backend/model/imagelistmodel.cpp
    backend/model/packagelistmodel.cpp
    backend/model/xmlimagelistmodel.cpp
    backend/model/xmlpreviewgenerator.cpp
    backend/model/imageproxymodel.cpp
    backend/model/slidemodel.cpp
    backend/model/slidefiltermodel.cpp
    backend/model/sortingmode.h
    backend/provider/xmlimageprovider.cpp
    backend/provider/packageimageprovider.cpp
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
