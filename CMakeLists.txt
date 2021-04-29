cmake_minimum_required(VERSION 3.0.0)
project(PICKPTS VERSION 0.1.0)

SET(CMAKE_BUILD_TYPE "Release")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -std=c++14")

include_directories(${CMAKE_SOURCE_DIR}/inc)

#include_directories(${PROJECT_SOURCE_DIR}/inc
#${PROJECT_SOURCE_DIR}/include/eigen3 ${PROJECT_SOURCE_DIR}/include/eigen3/Eigen /usr/include/gdal)
aux_source_directory(./src SRC_DIR)

add_executable(${PROJECT_NAME} ${SRC_DIR})

#target_link_libraries(${PROJECT_NAME} /usr/lib/libgdal.so)


