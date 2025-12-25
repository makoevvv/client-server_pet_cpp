cmake_minimum_required(VERSION 3.16)
project(qt-client)

if(NOT DEFINED QT_VERSION_MAJOR)
    message(FATAL_ERROR "QT_VERSION_MAJOR is not defined. Configure the root CMake project.")
endif()

set(SOURCES
    main.cpp
    mainwindow.cpp
)

add_executable(qt-client ${SOURCES})

target_link_libraries(qt-client 
    Qt${QT_VERSION_MAJOR}::Core 
    Qt${QT_VERSION_MAJOR}::Widgets 
    Qt${QT_VERSION_MAJOR}::Network 
    common
)

target_include_directories(qt-client PRIVATE ../../common)