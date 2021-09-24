
cmake_minimum_required(VERSION 3.10) 

add_library(osqp-cpp osqp++.cc)
target_link_libraries(osqp-cpp PUBLIC absl::strings absl::status Eigen3::Eigen PRIVATE osqpstatic ${CMAKE_DL_LIBS})
target_include_directories(osqp-cpp PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>")

