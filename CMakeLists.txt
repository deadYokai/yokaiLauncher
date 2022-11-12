cmake_minimum_required(VERSION 3.16)

set(VER 0.2)

project(yokaiLauncher VERSION ${VER} LANGUAGES CXX)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Network Svg UiTools Widgets)
find_package(yaml-cpp REQUIRED)


qt_standard_project_setup()

set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_CURRENT_SOURCE_DIR}/ui/)

set(UIC_UI_FILES
    ui/client.ui
    ui/melement.ui
    ui/dcon.ui
)

set(SOURCES 
    src/config_manager.cpp
    src/melement.cpp
    src/launcher.cpp
)

qt_add_executable(yokaiLauncher
    res.qrc
    ${SOURCES}
)

string(TIMESTAMP NOW "+%y%m%d%H%M%S")

target_compile_definitions(yokaiLauncher PRIVATE
    BUILDID=${NOW}
    _VERSION=${VER}
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

target_include_directories(yokaiLauncher PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include/)

