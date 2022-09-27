cmake_minimum_required(VERSION 3.16)
project(yokaiLauncher VERSION 1.0 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
include(GNUInstallDirs)
set(CMAKE_AUTOUIC ON)

find_package(QT NAMES Qt6 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Gui Network Svg UiTools Widgets)
find_package(WrapIconv)
find_package(WrapZSTD 1.3)

qt_add_executable(yokaiLauncher WIN32 MACOSX_BUNDLE
    assets/client.ui
    config_manager.cpp
    launcher.cpp launcher.h
)
target_include_directories(yokaiLauncher PRIVATE
    .
)

string(TIMESTAMP NOW "+%y%m%d%H%M%S")

target_compile_definitions(yokaiLauncher PRIVATE
    BUILDID=${NOW}
)

target_link_libraries(yokaiLauncher PRIVATE
    Qt::Core
    Qt::Gui
    Qt::Network
    Qt::Svg
    Qt::UiTools
    Qt::Widgets
    yaml-cpp
)


# Resources:
set(res_resource_files
    "assets/bg_1.png"
    "assets/client.ui"
    "assets/dcon.ui"
    "assets/defconf.yml"
    "assets/e-Ukraine-Bold.otf"
    "assets/e-Ukraine-Light.otf"
    "assets/e-Ukraine-Thin.otf"
    "assets/e-UkraineHead-LOGO.otf"
    "assets/icon.svg"
    "assets/settw.ui"
    "assets/default.qss"
)

qt_add_resources(yokaiLauncher "res"
    PREFIX
        "/"
    FILES
        ${res_resource_files}
)

if(WIN32)
    target_link_libraries(yokaiLauncher PRIVATE
        WrapIconv::WrapIconv
        WrapZSTD::WrapZSTD
        advapi32
        charset
        crypt32
        pthread
        wldap32
        ws2_32
        z
    )
endif()

install(TARGETS yokaiLauncher
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

