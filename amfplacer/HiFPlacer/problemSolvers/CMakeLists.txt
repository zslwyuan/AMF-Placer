
cmake_minimum_required(VERSION 3.10) 

aux_source_directory(. curDirectory)

#include_directories(./ ../../utils/ ../designInfo/ ../deviceInfo/ ../placement/placementInfo ../placement/placementTiming ${CMAKE_SOURCE_DIR}/thirdparty   ${abseil-cpp_SOURCE_DIR} ${EIGEN3_INCLUDE_DIR} ${Boost_INCLUDE_DIRS})
#include_directories(./ ../../utils/ ../designInfo/ ../deviceInfo/ ../placement/placementInfo ../placement/placementTiming ${CMAKE_SOURCE_DIR}/thirdparty ${EIGEN3_INCLUDE_DIR} ${Boost_INCLUDE_DIRS} ${CMAKE_BINARY_DIR}/PaToH/)

set(TARGET_NAME problemSolvers)

set(INCLUDE_DIRS 
  .
  ../../utils/
  ../designInfo/
  ../deviceInfo/ 
  ../placement/placementInfo 
  ../placement/placementTiming 
  ${CMAKE_SOURCE_DIR}/thirdparty/PaToH/
  ${CMAKE_SOURCE_DIR}/thirdparty 
  ${EIGEN3_INCLUDE_DIR}
  ${Boost_INCLUDE_DIRS} 
  ${CMAKE_SOURCE_DIR}/amfplacer/
  )
include_directories(${INCLUDE_DIRS})

set(LINKS_LIB
${LINKS_LIB}
${Boost_LIBRARIES}
${PaToH_LIBS}
m 
)
##add_pybind11_extension(${TARGET_NAME}_cpp
if(CMAKE_PYBIND11_TYPE)
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
  #add_library(ProblemSolvers ${curDirectory}  ${Boost_LIBRARIES})
  add_library(ProblemSolvers ${curDirectory})
  # target_link_libraries(ProblemSolvers osqp-cpp ${Boost_LIBRARIES} )
  target_link_libraries(ProblemSolvers  ${Boost_LIBRARIES} )
endif()

file(GLOB INSTALL_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/*.py")
install(
    FILES ${INSTALL_SRCS} DESTINATION amfplacer/HiFPlacer/problemSolvers
    )