
cmake_minimum_required(VERSION 3.10) 

aux_source_directory(. curDirectory)

#add_subdirectory(${abseil-cpp_SOURCE_DIR})

#include_directories(./ ../../../utils/ ../../designInfo/ ../../deviceInfo/ ../placementInfo  ${CMAKE_SOURCE_DIR}/thirdparty/ ${abseil-cpp_SOURCE_DIR} ${EIGEN3_INCLUDE_DIR})
set(curDirectory
   ${curDirectory}
   ${placementInfo_SRC}
   #${GlobalPlacement_SRC}
   #${ProblemSolvers_SRC}
   ${Util_SRC}
   ${Device_Src}
   ${Design_Src}
   #${Legalization_SRC}
   ${ZLIB_LIBRARIES}
   )

set(TARGET_NAME placementTiming)
set(INCLUDE_DIRS 
  .
  ../../../utils/
  ${CMAKE_SOURCE_DIR}/amfplacer/
  ../../designInfo/
  ../../deviceInfo/ 
  ../placementInfo 
  ${CMAKE_SOURCE_DIR}/thirdparty 
  ${abseil-cpp_SOURCE_DIR} 
  ${EIGEN3_INCLUDE_DIR}
  )
include_directories(${INCLUDE_DIRS})

if(CMAKE_PYBIND11_TYPE)
  #add_pybind11_extension(${TARGET_NAME}_cpp
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
  add_library(PlacementTiming ${curDirectory})
endif()

