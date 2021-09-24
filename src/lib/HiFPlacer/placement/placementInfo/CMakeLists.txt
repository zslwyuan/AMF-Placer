
cmake_minimum_required(VERSION 3.10) 

aux_source_directory(. curDirectory)

#add_subdirectory(${abseil-cpp_SOURCE_DIR})

include_directories(./ ../../../utils/ ../../designInfo/ ../../deviceInfo/ ../placementTiming/ ../../../3rdParty/ ${abseil-cpp_SOURCE_DIR} ${EIGEN3_INCLUDE_DIR})



add_library(PlacementInfo ${curDirectory})


