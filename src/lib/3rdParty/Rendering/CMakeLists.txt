
cmake_minimum_required(VERSION 3.10) 

aux_source_directory(. curDirectory)

set(CMAKE_AUTOMOC TRUE)

set(SAMPLES_DIR "${CMAKE_CURRENT_SOURCE_DIR}" CACHE PATH "Samples directory")
set(BLEND2D_DIR "${SAMPLES_DIR}/../blend2d" CACHE PATH "Location of 'blend2d'")

set(BLEND2D_STATIC TRUE)
include("${BLEND2D_DIR}/CMakeLists.txt")

find_package(Qt5Widgets REQUIRED)
add_library(Rendering ${curDirectory})

set_property(TARGET Rendering PROPERTY CXX_VISIBILITY_PRESET hidden)
target_link_libraries(Rendering Qt5::Widgets blend2d::blend2d)