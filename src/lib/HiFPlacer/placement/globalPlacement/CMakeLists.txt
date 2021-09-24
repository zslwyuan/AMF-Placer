
cmake_minimum_required(VERSION 3.10) 

aux_source_directory(. curDirectory)

include_directories(./ ../../../utils/ ../../designInfo/ ../../deviceInfo/ ../placementTiming ../placementInfo ../../../3rdParty ../../problemSolvers ../legalization  ${abseil-cpp_SOURCE_DIR} ${EIGEN3_INCLUDE_DIR})

add_library(GlobalPlacer ${curDirectory})
