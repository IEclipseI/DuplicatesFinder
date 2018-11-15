cmake_minimum_required(VERSION 3.12)
project(hw5)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS_DEBUG "-o3")
set(CMAKE_CXX_FLAGS_RELEASE "-o3")

#-fsanitize=address
#--coverage
set(CMAKE_PREFIX_PATH "/home/roman/Qt/5.11.2/gcc_64/lib")
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5Core REQUIRED)
find_package(Qt5Widgets REQUIRED)

include_directories(${Qt5Widgets_INCLUDE_DIRS})
include_directories(${Qt5Core_INCLUDE_DIRS})

add_executable(hw5 main.cpp
        mainwindow.cpp mainwindow.h mainwindow.ui
        confirmdeleting.cpp confirmdeleting.h confirmdeleting.ui
        stopsearch.cpp stopsearch.h stopsearch.ui)

target_link_libraries(hw5 Qt5::Widgets)
target_link_libraries(hw5 Qt5::Core)