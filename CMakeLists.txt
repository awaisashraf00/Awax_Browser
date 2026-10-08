
cmake_minimum_required(VERSION 3.16)

project(Awax_browser VERSION 1.0 LANGUAGES CXX)

# set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS
    Core
    Gui
    Qml
    Quick
    QuickControls2
)

qt_standard_project_setup(REQUIRES 6.8)

add_executable(Awax_browser
    src/main.cpp
)

target_include_directories(Awax_browser PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/browser
    ${CMAKE_CURRENT_SOURCE_DIR}/src/css
    ${CMAKE_CURRENT_SOURCE_DIR}/src/html
    ${CMAKE_CURRENT_SOURCE_DIR}/src/javascript
    ${CMAKE_CURRENT_SOURCE_DIR}/src/renderer
    ${CMAKE_CURRENT_SOURCE_DIR}/src/layout
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uri
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui
)

set_target_properties(Awax_browser PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

qt_add_qml_module(Awax_browser
    URI Awax_browser
    VERSION 1.0

    # OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/qml/Awax_browser"

    # QML_FILES
    #     qml/Main.qml

)

target_link_libraries(Awax_browser PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickControls2
)