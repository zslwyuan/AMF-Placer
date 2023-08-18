
cmake_minimum_required(VERSION 3.10) 

aux_source_directory(. curDirectory)
message(STATUS designInfo=${curDirectory})
set(curDirectory
  ${curDirectory}
  ${CMAKE_SOURCE_DIR}/amfplacer/utils/strPrint.cc
  ${CMAKE_SOURCE_DIR}/amfplacer/utils/stringCheck.cc
  ${CMAKE_SOURCE_DIR}/amfplacer/utils/dumpZip.cc
)

set(LINKS_LIB
${ZLIB_LIBRARIES}
${Boost_LIBRARIES}
)
#include_directories(./ ../../utils/ ../deviceInfo/)

set(TARGET_NAME designInfo)
set(INCLUDE_DIRS 
  ${INCLUDE_DIRS}
  ../../utils/
  ../deviceInfo/ 
  ${CMAKE_SOURCE_DIR}/amfplacer/
  )
message(STATUS INCLUDE_DIRS=${INCLUDE_DIRS})
include_directories(${INCLUDE_DIRS})

if(CMAKE_PYBIND11_TYPE)
  #add_pybind11_extension(${TARGET_NAME}_cpp
  add_pytorch_extension(${TARGET_NAME}_cpp
      ${curDirectory}
      EXTRA_INCLUDE_DIRS ${INCLUDE_DIRS}
      EXTRA_LINK_LIBRARIES ${LINKS_LIB}
      #EXTRA_DEFINITIONS DRAWPLACE=${DRAWPLACE}
    )
  install(TARGETS ${TARGET_NAME}_cpp DESTINATION amfplacer/HiFPlacer/${TARGET_NAME})
  file(GLOB INSTALL_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/*.py)
  install(FILES ${INSTALL_SRCS} DESTINATION amfplacer/HiFPlacer/${TARGET_NAME})
else()
  add_library(DesignInfo ${curDirectory})
endif()
