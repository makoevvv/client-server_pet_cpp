cmake_minimum_required(VERSION 3.16)
project(common)

find_package(PkgConfig REQUIRED)
pkg_check_modules(nlohmann_json REQUIRED nlohmann_json)

add_library(common packet.cpp)
target_include_directories(common PUBLIC .)
target_link_libraries(common ${nlohmann_json_LIBRARIES})
target_include_directories(common PRIVATE ${nlohmann_json_INCLUDE_DIRS})
set_target_properties(common PROPERTIES AUTOMOC OFF AUTOUIC OFF AUTORCC OFF)