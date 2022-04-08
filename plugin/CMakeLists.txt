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
    model/sortingmode.h
    model/slidefiltermodel.cpp
    provider/xmlimageprovider.cpp
    provider/packageimageprovider.cpp
)

add_library(plasma_wallpaper_imageplugin SHARED ${image_SRCS})

target_link_libraries(plasma_wallpaper_imageplugin
    Qt::Core
    Qt::Gui
    Qt::Quick
    Qt::Qml
    KF5::ConfigCore
    KF5::CoreAddons
    KF5::I18n
    KF5::KIOCore
    KF5::KIOFileWidgets
    KF5::Notifications
    KF5::Package
    KF5::Plasma # Plasma::Theme
    KF5::Solid
)

if(BUILD_TESTING)
   add_subdirectory(autotests)
endif()

install(TARGETS plasma_wallpaper_imageplugin DESTINATION ${KDE_INSTALL_QMLDIR}/com/github/easyteacher/plasma/wallpapers/xml)
install(FILES qmldir DESTINATION ${KDE_INSTALL_QMLDIR}/com/github/easyteacher/plasma/wallpapers/xml)
