
cmake_minimum_required(VERSION 3.10) 

aux_source_directory(. curDirectory)

#include_directories(./ ../../utils/ ../deviceInfo/)

set(TARGET_NAME DesignInfo)
set(INCLUDE_DIRS 
  ../../utils/
  ../deviceInfo/ 
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
  add_library(DesignInfo ${curDirectory})
endif()



file(GLOB INSTALL_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/*.py")
install(
    FILES ${INSTALL_SRCS} DESTINATION lib/HiFPlacer/designInfo
    )