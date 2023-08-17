
cmake_minimum_required(VERSION 3.10) 

aux_source_directory(. curDirectory)

#include_directories(./ ../../utils/ ../designInfo/ ../deviceInfo/ ../placement/placementInfo ../placement/placementTiming ../../3rdParty   ${abseil-cpp_SOURCE_DIR} ${EIGEN3_INCLUDE_DIR} ${Boost_INCLUDE_DIRS})
#include_directories(./ ../../utils/ ../designInfo/ ../deviceInfo/ ../placement/placementInfo ../placement/placementTiming ../../3rdParty ${EIGEN3_INCLUDE_DIR} ${Boost_INCLUDE_DIRS} ${CMAKE_BINARY_DIR}/PaToH/)

set(TARGET_NAME ProblemSolvers)

set(INCLUDE_DIRS 
  .
  ../../utils/
  ../designInfo/
  ../deviceInfo/ 
  ../placement/placementInfo 
  ../placement/placementTiming 
  ../../3rdParty 
  ${EIGEN3_INCLUDE_DIR}
  ${Boost_INCLUDE_DIRS} 
  ${CMAKE_BINARY_DIR}/PaToH/
  )
include_directories(${INCLUDE_DIRS})

set(LINK_LIBS
${LINK_LIBS}
${Boost_LIBRARIES}
)

#add_pybind11_extension(${TARGET_NAME}_cpp
##add_pytorch_extension(${TARGET_NAME}_cpp
#    ${curDirectory}
#    EXTRA_INCLUDE_DIRS ${INCLUDE_DIRS}
#    EXTRA_LINK_LIBRARIES ${LINK_LIBS}
#    #EXTRA_DEFINITIONS DRAWPLACE=${DRAWPLACE}
#  )
if(NOT CMAKE_PYBIND11_TYPE)
  add_library(ProblemSolvers ${curDirectory}  ${Boost_LIBRARIES})
  #target_link_libraries(ProblemSolvers osqp-cpp ${Boost_LIBRARIES} )
  target_link_libraries(ProblemSolvers  ${Boost_LIBRARIES} )
endif()

file(GLOB INSTALL_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/*.py")
install(
    FILES ${INSTALL_SRCS} DESTINATION lib/HiFPlacer/problemSolvers
    )