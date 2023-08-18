
cmake_minimum_required(VERSION 3.10) 

aux_source_directory(. curDirectory)
file(GLOB placementInfo_SRC ../placementInfo/*.cc)
set(curDirectory
   ${curDirectory}
   ${placementInfo_SRC}
   ${PlacementTimingSrc}
   ${GlobalPlacement_SRC}
   ${ProblemSolvers_SRC}
   ${Util_SRC}
   ${Device_Src}
   ${Design_Src}
   ${Legalization_SRC}
   )

# message(STATUS packing_src=${curDirectory})

#add_subdirectory(${abseil-cpp_SOURCE_DIR})

#include_directories(./ ../../../utils/ ../../designInfo/ ../../deviceInfo/ ../placementTiming ../globalPlacement ../../problemSolvers  ${CMAKE_SOURCE_DIR}/thirdparty/ ../placementInfo ${abseil-cpp_SOURCE_DIR} ${EIGEN3_INCLUDE_DIR})

set(TARGET_NAME packing)
set(INCLUDE_DIRS 
  .
  ../../../utils/
  ${CMAKE_SOURCE_DIR}/amfplacer
  ../../designInfo/
  ../../deviceInfo/ 
  ../placementTiming 
  ../globalPlacement
  ../placementInfo 
  ${CMAKE_SOURCE_DIR}/thirdparty 
  ${Legalization_INCLUDE_DIR}
  ../../problemSolvers 
  ${abseil-cpp_SOURCE_DIR} 
  ${EIGEN3_INCLUDE_DIR}

  )
set(LINKS_LIB
${LINKS_LIB}
MaximalCardinalityMatching
${PaToH_LIBS}
)

if(CMAKE_PYBIND11_TYPE)
  set_target_properties(MaximalCardinalityMatching PROPERTIES POSITION_INDEPENDENT_CODE TRUE)
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
  include_directories(${INCLUDE_DIRS})
  add_library(Packing ${curDirectory})
  
  target_link_libraries(Packing MaximalCardinalityMatching)
endif()