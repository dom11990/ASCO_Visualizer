cmake_minimum_required(VERSION 3.10)

project(ASCO_Visualizer VERSION 1.0)

#set(CMAKE_PREFIX_PATH "/usr/local/Qt-5.12/")
set(CMAKE_QT5_DIR "/usr/include/qt")
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)



find_library(Qwt qwt PATHS)

find_package(Qt5 COMPONENTS Core Widgets Gui Concurrent Network
 REQUIRED)

 
 include_directories(
    /usr/include/qwt
    src
    "${CMAKE_CURRENT_SOURCE_DIR}"
)
 
 
 # Define the CXX sources
set ( CXX_SRCS
    src/main.cpp
    src/mainwindow.cpp
    src/displayer.cpp
    src/asco_parameter.cpp
    src/asco_design_variable_properties.cpp
    src/asco_measurement_properties.cpp
    src/asco_parameter_properties.cpp
    src/asco_measurement.cpp
    src/asco_design_variable.cpp
    src/asco_handler.cpp
    src/qucs_dat.cpp
    src/asco_handler.cpp
)
 

set_source_files_properties(
    ${CXX_SRCS} PROPERTIES COMPILE_FLAGS
    " -g -O0 -std=c++11 -Wall -Wno-psabi -w")

add_executable("${CMAKE_PROJECT_NAME}" ${CXX_SRCS})

target_link_libraries(ASCO_Visualizer
    Qt5::Core
    Qt5::Widgets
    Qt5::Gui
    Qt5::Concurrent
    Qt5::Network
    "${Qwt}")
