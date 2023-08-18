
cmake_minimum_required(VERSION 3.10) 

aux_source_directory(. curDirectory)
file(GLOB PlacementTimingSrc ../placementTiming/*.cc)
#aux_source_directory(../../designInfo/ curDirectory)
set(curDirectory
${curDirectory}
../../designInfo/DesignInfo.cc
../../deviceInfo/DeviceInfo.cc
${PlacementTimingSrc}
${CMAKE_SOURCE_DIR}/amfplacer/utils/dumpZip.cc
${CMAKE_SOURCE_DIR}/amfplacer/utils/strPrint.cc
${CMAKE_SOURCE_DIR}/amfplacer/utils/stringCheck.cc
)
message(STATUS placementInfoDirectory=${curDirectory})

#add_subdirectory(${abseil-cpp_SOURCE_DIR})

#include_directories(./ ../../../utils/ ../../designInfo/ ../../deviceInfo/ ../placementTiming/ ${CMAKE_SOURCE_DIR}/thirdparty/ ${abseil-cpp_SOURCE_DIR} ${EIGEN3_INCLUDE_DIR})

set(LINKS_LIB
${ZLIB_LIBRARIES}
${Boost_LIBRARIES}
)

set(TARGET_NAME placementInfo)
set(INCLUDE_DIRS 
  .
  ../../../utils/
  ../../designInfo/
  ../../deviceInfo/ 
  ../placementTiming 
  ${CMAKE_SOURCE_DIR}/amfplacer/
  ${CMAKE_SOURCE_DIR}/thirdparty
  ${abseil-cpp_SOURCE_DIR} 
  ${EIGEN3_INCLUDE_DIR}
  )
include_directories(${INCLUDE_DIRS})
#link_directories(${CMAKE_SOURCE_DIR}/HiFPlacer/designInfo)
#set(LINK_LIBS ${CMAKE_SOURCE_DIR}/HiFPlacer/designInfo/designInfo_cpp.cpython-39-x86_64-linux-gnu.so)

if(CMAKE_PYBIND11_TYPE)
  #add_pybind11_extension(${TARGET_NAME}_cpp
  #set(designInfo_LIBRARY_DIR ${CMAKE_SOURCE_DIR}/amfplacer/HiFPlacer/designInfo)
  add_pytorch_extension(${TARGET_NAME}_cpp
      ${curDirectory}
      EXTRA_INCLUDE_DIRS ${INCLUDE_DIRS}
      EXTRA_LINK_LIBRARIES ${LINKS_LIB}
      #EXTRA_DEFINITIONS DRAWPLACE=${DRAWPLACE}
    )
    install(TARGETS ${TARGET_NAME}_cpp DESTINATION amfplacer/HiFPlacer/placement/${TARGET_NAME})
    file(GLOB INSTALL_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/*.py)
    install(FILES ${INSTALL_SRCS} DESTINATION amfplacer/HiFPlacer/placement/${TARGET_NAME})
else()
  add_library(PlacementInfo ${curDirectory})
endif()


