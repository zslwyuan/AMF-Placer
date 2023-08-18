
cmake_minimum_required(VERSION 3.10) 

set(TARGET_NAME utils)

find_package(Qt5Widgets REQUIRED)
find_package(Qt5Core REQUIRED)
find_package(Qt5Gui REQUIRED)

aux_source_directory(. SRC_FILS)
# aux_source_directory(${CMAKE_SOURCE_DIR}/thirdparty/Rendering SRC_FILS)

set(LINKS_LIB
${Boost_LIBRARIES} 
Rendering Qt5::Widgets 
blend2d
${ZLIB_LIBRARIES}
)

set(INCLUDE_DIRS 
  .
  ${CMAKE_SOURCE_DIR}/amfplacer/
  ${CMAKE_SOURCE_DIR}/thirdparty/
  )
include_directories(${INCLUDE_DIRS})
if(CMAKE_PYBIND11_TYPE)
  #add_pybind11_extension(${TARGET_NAME}_cpp
  add_pytorch_extension(${TARGET_NAME}_cpp
  #add_torch_extension(${TARGET_NAME}_cpp
       ${SRC_FILS}
       EXTRA_INCLUDE_DIRS ${INCLUDE_DIRS}
       EXTRA_LINK_LIBRARIES ${LINKS_LIB}
       #EXTRA_DEFINITIONS DRAWPLACE=${DRAWPLACE}
     )
  install(TARGETS ${TARGET_NAME}_cpp DESTINATION amfplacer/${TARGET_NAME})
  file(GLOB INSTALL_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/*.py)
  install(FILES ${INSTALL_SRCS} DESTINATION amfplacer/${TARGET_NAME})
else()
  add_library(Utils ${SRC_FILS})
endif()

