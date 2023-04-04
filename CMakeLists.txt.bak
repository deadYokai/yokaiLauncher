cmake_minimum_required(VERSION 3.16)

project(yokaiLauncher VERSION 0.1.3 LANGUAGES CXX)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Network Svg UiTools Widgets)
find_package(yaml-cpp REQUIRED)

qt_standard_project_setup()

set(CMAKE_AUTORCC ON)

qt_add_executable(yokaiLauncher
    res.qrc
    config_manager.cpp
    dirdialog.cpp
    launcher.cpp
)

string(TIMESTAMP NOW "+%y%m%d%H%M%S")

target_compile_definitions(yokaiLauncher PRIVATE
    BUILDID=${NOW}
)

target_link_libraries(yokaiLauncher PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Network
    Qt6::Svg
    Qt6::UiTools
    Qt6::Widgets
    yaml-cpp
)

set_target_properties(yokaiLauncher PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE OFF
)

target_include_directories(yokaiLauncher PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

