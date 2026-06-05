cmake_minimum_required(VERSION 3.16)
project(extrator-movimento VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt5 REQUIRED COMPONENTS Widgets)
find_package(PkgConfig REQUIRED)
pkg_check_modules(OpenCV REQUIRED opencv4)

add_executable(extrator-movimento
    src/main.cpp
    src/mainwindow.h
    src/mainwindow.cpp
    src/processor.h
    src/processor.cpp
    src/player.h
    src/player.cpp
)

target_include_directories(extrator-movimento PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${OpenCV_INCLUDE_DIRS}
)

target_link_libraries(extrator-movimento
    Qt5::Widgets
    ${OpenCV_LIBRARIES}
)
