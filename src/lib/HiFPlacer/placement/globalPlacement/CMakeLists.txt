
cmake_minimum_required(VERSION 3.10) 

aux_source_directory(. GLOBAL_SRC_DIR)

#execute_process(COMMAND echo ${GLOBAL_SRC_DIR})

#include_directories(./ ../../../utils/ ../../designInfo/ ../../deviceInfo/ ../placementTiming ../placementInfo ../../../3rdParty ../../problemSolvers ../legalization  ${abseil-cpp_SOURCE_DIR} ${EIGEN3_INCLUDE_DIR})

set(TARGET_NAME GlobalPlacer)
set(INCLUDE_DIRS 
  .
  ../../../utils/
  ../../designInfo/
  ../../deviceInfo/ 
  ../placementTiming 
  ../placementInfo 
  ../../../3rdParty 
  ../../problemSolvers 
  ../legalization  
  ${abseil-cpp_SOURCE_DIR} 
  ${EIGEN3_INCLUDE_DIR}
  )
include_directories(${INCLUDE_DIRS})


add_pybind11_extension(${TARGET_NAME}_cpp
#add_pytorch_extension(${TARGET_NAME}_cpp
    ${GLOBAL_SRC_DIR}
    EXTRA_INCLUDE_DIRS ${INCLUDE_DIRS}
    EXTRA_LINK_LIBRARIES ${LINK_LIBS}
    #EXTRA_DEFINITIONS DRAWPLACE=${DRAWPLACE}
  )
if(NOT CMAKE_PYBIND11_TYPE)
  add_library(${TARGET_NAME} ${GLOBAL_SRC_DIR})
endif()

file(GLOB INSTALL_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/*.py")
install(
    FILES ${INSTALL_SRCS} DESTINATION lib/HiFPlacer/placement/globalPlacement
    )