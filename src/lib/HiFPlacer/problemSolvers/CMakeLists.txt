
cmake_minimum_required(VERSION 3.10) 

aux_source_directory(. curDirectory)
find_package(Boost)
include_directories(./ ../../utils/ ../designInfo/ ../deviceInfo/ ../placement/placementInfo ../placement/placementTiming ../../3rdParty   ${abseil-cpp_SOURCE_DIR} ${EIGEN3_INCLUDE_DIR} ${Boost_INCLUDE_DIR})

add_library(ProblemSolvers ${curDirectory})
target_link_libraries(ProblemSolvers osqp-cpp ${Boost_LIBRARIES} )