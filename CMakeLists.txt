cmake_minimum_required(VERSION 3.21)

project(Awax_browser VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Quick QuickControls2)

qt_standard_project_setup(REQUIRES 6.8)

qt_add_executable(Awax_browser
    src/main.cpp
    src/url/URL.cpp
    src/url/URL.h
    src/network/HttpClient.h
    src/network/HttpClient.cpp
    src/network/DnsResolver.h
    src/network/DnsResolver.cpp 
    src/network/TcpConnection.h
    src/network/TcpConnection.cpp 
    src/browser/Browser.cpp
    src/browser/Browser.h
)

qt_add_qml_module(Awax_browser
    URI Awax_browser
    VERSION 1.0
    NO_LINT
    QML_FILES
        qml/Main.qml
        qml/SearchArea/SearchArea.qml
)

target_include_directories(Awax_browser PRIVATE
    src/browser
    src/network
    src/url
    src/html
    src/renderer
    src/ui
)

set_target_properties(Awax_browser PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

target_link_libraries(Awax_browser PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickControls2
)

# Standalone test executable for the URL parsing stage. It links only
# Qt6::Core (no GUI) so it runs fast and keeps the parser testable
# without launching the browser window.
enable_testing()
add_executable(test_url
    tests/test_url.cpp
    src/url/URL.cpp
    src/url/URL.h
)
target_include_directories(test_url PRIVATE src/url src/network)
target_link_libraries(test_url PRIVATE Qt6::Core)
add_test(NAME url COMMAND test_url)
