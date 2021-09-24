
cmake_minimum_required(VERSION 3.10) 

aux_source_directory(. curDirectory)

#add_subdirectory(${abseil-cpp_SOURCE_DIR})

include_directories(./ ../../../utils/ ../../designInfo/ ../../deviceInfo/ ../placementTiming ../../problemSolvers  ../../../3rdParty/ ../placementInfo ${abseil-cpp_SOURCE_DIR} ${EIGEN3_INCLUDE_DIR})

add_library(Packing ${curDirectory})

target_link_libraries(Packing MaximalCardinalityMatching)