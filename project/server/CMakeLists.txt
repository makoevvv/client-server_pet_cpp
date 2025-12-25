cmake_minimum_required(VERSION 3.16)
project(qt-server)

if(NOT DEFINED QT_VERSION_MAJOR)
    message(FATAL_ERROR "QT_VERSION_MAJOR is not defined. Configure the root CMake project.")
endif()

set(SOURCES 
    main.cpp 
    mainwindow.cpp 
    clientinfo.cpp
)

add_executable(qt-server ${SOURCES})

target_link_libraries(qt-server 
    Qt${QT_VERSION_MAJOR}::Core 
    Qt${QT_VERSION_MAJOR}::Widgets 
    Qt${QT_VERSION_MAJOR}::Network 
    common
)

target_include_directories(qt-server PRIVATE ../../common)

add_custom_command(TARGET qt-server POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/limits.json
        $<TARGET_FILE_DIR:qt-server>/limits.json)