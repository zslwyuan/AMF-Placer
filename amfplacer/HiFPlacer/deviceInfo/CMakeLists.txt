
cmake_minimum_required(VERSION 3.10) 

aux_source_directory(. curDirectory )
#aux_source_directory(${CMAKE_SOURCE_DIR}/amfplacer/utils/ curDirectory )

#include_directories(./ ../../utils/)

message(STATUS "curDirectory = ${curDirectory}")

set(TARGET_NAME deviceInfo)
set(INCLUDE_DIRS 
  ../../utils/
  ${CMAKE_SOURCE_DIR}/amfplacer/
  )
include_directories(${INCLUDE_DIRS})
message(STATUS device_dir=${INCLUDE_DIRS})

if(CMAKE_PYBIND11_TYPE)
  #add_pybind11_extension(${TARGET_NAME}_cpp
  add_pytorch_extension(${TARGET_NAME}_cpp
      ${curDirectory}
      ${CMAKE_SOURCE_DIR}/amfplacer/utils/strPrint.cc
      ${CMAKE_SOURCE_DIR}/amfplacer/utils/stringCheck.cc
      EXTRA_INCLUDE_DIRS ${INCLUDE_DIRS}
      EXTRA_LINK_LIBRARIES ${LINK_LIBS}
      #EXTRA_DEFINITIONS DRAWPLACE=${DRAWPLACE}
    )
  install(TARGETS ${TARGET_NAME}_cpp DESTINATION amfplacer/HiFPlacer/${TARGET_NAME})
  file(GLOB INSTALL_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/*.py)
  install(FILES ${INSTALL_SRCS} DESTINATION amfplacer/HiFPlacer/${TARGET_NAME})
else()
  add_library(DeviceInfo ${curDirectory})
endif()

