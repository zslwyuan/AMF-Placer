
cmake_minimum_required(VERSION 3.10) 

set(TARGET_NAME utils)

aux_source_directory(. SRC_FILS)

set(INCLUDE_DIRS 
  .
  )
include_directories(${INCLUDE_DIRS})

add_pybind11_extension(${TARGET_NAME}_cpp
#add_pytorch_extension(${TARGET_NAME}_cpp
     ${SRC_FILS}
     EXTRA_INCLUDE_DIRS ${INCLUDE_DIRS}
     EXTRA_LINK_LIBRARIES ${LINK_LIBS}
     #EXTRA_DEFINITIONS DRAWPLACE=${DRAWPLACE}
   )
if(NOT CMAKE_PYBIND11_TYPE)
  add_library(Utils ${SRC_FILS})
endif()


file(GLOB INSTALL_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/*.py")
install(
    FILES ${INSTALL_SRCS} DESTINATION lib/utils
    )