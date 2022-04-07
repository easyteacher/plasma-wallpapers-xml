cmake_minimum_required(VERSION 3.0)

project(plasma-wallpapers-xml)
set(PROJECT_VERSION "0.2.0")
set(PROJECT_VERSION_MAJOR 0)

set(KF5_MIN_VERSION "5.66.0")
set(QT_MIN_VERSION "5.15.0")

################# Disallow in-source build #################
if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
   message(FATAL_ERROR "plasma requires an out of source build. Please create a separate build directory and run 'cmake path_to_plasma [options]' there.")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FeatureSummary)

find_package(ECM ${KF5_MIN_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(ECMQMLModules)
include(ECMQtDeclareLoggingCategory)

ecm_find_qmlmodule(QtQuick 2.0)
ecm_find_qmlmodule(org.kde.kquickcontrolsaddons 2.0)

find_package(KF5 ${KF5_MIN_VERSION} REQUIRED COMPONENTS
    Config
    Declarative
    I18n
    KIO
    Notifications
    Package
    Plasma
    Solid
)

find_package(Qt${QT_MAJOR_VERSION} ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS
    Qml
    Quick
    Test
)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)

plasma_install_package(imagepackage com.github.easyteacher.plasma.wallpapers.xml wallpapers wallpaper)
plasma_install_package(slideshowpackage com.github.easyteacher.plasma.wallpapers.xml.slideshow wallpapers wallpaper)
add_subdirectory(plugin)

install(DIRECTORY imagepackage/contents/ui DESTINATION ${PLASMA_DATA_INSTALL_DIR}/wallpapers/com.github.easyteacher.plasma.wallpapers.xml.slideshow/contents PATTERN CMakeLists.txt EXCLUDE PATTERN Messages.sh EXCLUDE PATTERN main.qml EXCLUDE PATTERN ConfigGeneral.qml EXCLUDE)
