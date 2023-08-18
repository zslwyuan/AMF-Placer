
cmake_minimum_required(VERSION 3.10) 

aux_source_directory(. GLOBAL_SRC_DIR)

#execute_process(COMMAND echo ${GLOBAL_SRC_DIR})

#include_directories(./ ../../../utils/ ../../designInfo/ ../../deviceInfo/ ../placementTiming ../placementInfo ${CMAKE_SOURCE_DIR}/thirdparty ../../problemSolvers ../legalization  ${abseil-cpp_SOURCE_DIR} ${EIGEN3_INCLUDE_DIR})

set(GLOBAL_SRC_DIR
   ${GLOBAL_SRC_DIR}
   ${placementInfo_SRC}
   ${ProblemSolvers_SRC}
   ${PlacementTimingSrc}
   ${Util_SRC}
   ${Device_Src}
   ${Design_Src}
   ${Legalization_SRC}
   ${ZLIB_LIBRARIES}
   )

set(LINKS_LIB
   ${LINKS_LIB}
   ${PaToH_LIBS}
)

set(TARGET_NAME globalPlacement)
set(INCLUDE_DIRS 
  .
  ../../../utils/
  ${CMAKE_SOURCE_DIR}/amfplacer/
  ../../designInfo/
  ../../deviceInfo/ 
  ../placementTiming 
  ../placementInfo 
  ${CMAKE_SOURCE_DIR}/thirdparty 
  ../../problemSolvers 
  #../legalization  
  ${CMAKE_SOURCE_DIR}/amfplacer/HiFPlacer/placement/legalization
  ${abseil-cpp_SOURCE_DIR} 
  ${EIGEN3_INCLUDE_DIR}
  )
include_directories(${INCLUDE_DIRS})

if(CMAKE_PYBIND11_TYPE)
  #add_pybind11_extension(${TARGET_NAME}_cpp
  add_pytorch_extension(${TARGET_NAME}_cpp
      ${GLOBAL_SRC_DIR}
      EXTRA_INCLUDE_DIRS ${INCLUDE_DIRS}
      EXTRA_LINK_LIBRARIES ${LINKS_LIB}
      #EXTRA_DEFINITIONS DRAWPLACE=${DRAWPLACE}
    )
    install(TARGETS ${TARGET_NAME}_cpp DESTINATION amfplacer/HiFPlacer/placement/${TARGET_NAME})
    file(GLOB INSTALL_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/*.py)
    install(FILES ${INSTALL_SRCS} DESTINATION amfplacer/HiFPlacer/placement/${TARGET_NAME})
else()
  add_library(GlobalPlacer ${GLOBAL_SRC_DIR})
endif()