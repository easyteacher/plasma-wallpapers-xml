include(ECMMarkAsTest)

set(testWallpaperFinder_SRCS
    test_finder.cpp
    ../finder/imagefinder.cpp
    ../finder/packagefinder.cpp
    ../finder/xmlfinder.cpp
)

add_executable(testWallpaperFinder ${testWallpaperFinder_SRCS})

target_link_libraries(testWallpaperFinder
    Qt::Test
    Qt::Core
    Qt::Gui
    KF5::I18n
    KF5::Package
)

ecm_mark_as_test(testWallpaperFinder)

set(testWallpaperModel_SRCS
    test_model.cpp
    ../finder/imagefinder.cpp
    ../finder/imagesizefinder.cpp
    ../finder/packagefinder.cpp
    ../finder/xmlfinder.cpp
    ../model/abstractimagelistmodel.cpp
    ../model/imagelistmodel.cpp
    ../model/imageproxymodel.cpp
    ../model/packagelistmodel.cpp
    ../model/slidefiltermodel.cpp
    ../model/slidemodel.cpp
    ../model/xmlimagelistmodel.cpp
    ../model/xmlpreviewgenerator.cpp
    ../model/imageroles.h
)

add_executable(testWallpaperModel ${testWallpaperModel_SRCS})

target_link_libraries(testWallpaperModel
    Qt::Test
    Qt::Core
    Qt::Gui
    KF5::ConfigCore
    KF5::CoreAddons
    KF5::I18n
    KF5::KIOCore
    KF5::KIOFileWidgets
    KF5::Package
)

ecm_mark_as_test(testWallpaperModel)
