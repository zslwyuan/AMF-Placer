
cmake_minimum_required(VERSION 3.10) 

aux_source_directory(. curDirectory)

#add_subdirectory(${abseil-cpp_SOURCE_DIR})

#include_directories(./ ../../../utils/ ../../designInfo/ ../../deviceInfo/ ../placementInfo  ../../../3rdParty/ ${abseil-cpp_SOURCE_DIR} ${EIGEN3_INCLUDE_DIR})


set(TARGET_NAME PlacementTiming)
set(INCLUDE_DIRS 
  .
  ../../../utils/
  ../../designInfo/
  ../../deviceInfo/ 
  ../placementInfo 
  ../../../3rdParty 
  ${abseil-cpp_SOURCE_DIR} 
  ${EIGEN3_INCLUDE_DIR}
  )
include_directories(${INCLUDE_DIRS})

add_pybind11_extension(${TARGET_NAME}_cpp
#add_pytorch_extension(${TARGET_NAME}_cpp
    ${curDirectory}
    EXTRA_INCLUDE_DIRS ${INCLUDE_DIRS}
    EXTRA_LINK_LIBRARIES ${LINK_LIBS}
    #EXTRA_DEFINITIONS DRAWPLACE=${DRAWPLACE}
  )


if(NOT CMAKE_PYBIND11_TYPE)
  add_library(PlacementTiming ${curDirectory})
endif()



file(GLOB INSTALL_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/*.py")
install(
    FILES ${INSTALL_SRCS} DESTINATION lib/HiFPlacer/placement/placementTiming
    )
