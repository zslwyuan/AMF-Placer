set(INCLUDE_DIRS 
  .
  ${Boost_INCLUDE_DIR}
  ${ZLIB_INCLUDE_DIRS}
  ${CMAKE_SOURCE_DIR}/thirdparty/
  ${CMAKE_SOURCE_DIR}/thirdparty/PaToH/
  ${CMAKE_SOURCE_DIR}/amfplacer/
  ${CMAKE_SOURCE_DIR}/amfplacer/util/
  ${CMAKE_SOURCE_DIR}/amfplacer/utils/
  #${abseil-cpp_SOURCE_DIR} 
)
set(Util_SRC
${CMAKE_SOURCE_DIR}/amfplacer/utils/dumpZip.cc
${CMAKE_SOURCE_DIR}/amfplacer/utils/strPrint.cc
${CMAKE_SOURCE_DIR}/amfplacer/utils/stringCheck.cc
${CMAKE_SOURCE_DIR}/amfplacer/utils/sysInfo.cc
)
#include_directories(
#  ${Boost_INCLUDE_DIR}
#  ${EIGEN3_INCLUDE_DIR}
#  ${ZLIB_INCLUDE_DIRS}
#)

include_directories(${INCLUDE_DIRS})

set(PaToH_LIBS
${CMAKE_SOURCE_DIR}/thirdparty/PaToH/libpatoh.a 
)
#add_subdirectory(pybind11)
set(LINK_LIBS 
  ${LINK_LIBS}
  ${ZLIB_LIBRARIES}
  ${Boost_LIBRARIES} 
  Rendering Qt5::Widgets 
  blend2d
  #${abseil-cpp_SOURCE_DIR} 
)

set(TARGET_NAME app)
if(CMAKE_PYBIND11_TYPE)
  # set(LINK_LIBS 
  #  ${LINK_LIBS}
  #  #${abseil-cpp_SOURCE_DIR} 
  # )
  # set(AMF_Files ${Util_SRC} ${CMAKE_CURRENT_SOURCE_DIR}/app.cc)
  # add_pytorch_extension(${TARGET_NAME}_cpp
  #    ${AMF_Files}
  #    EXTRA_INCLUDE_DIRS ${INCLUDE_DIRS}
  #    EXTRA_LINK_LIBRARIES ${LINK_LIBS}
  #    #EXTRA_DEFINITIONS DRAWPLACE=${DRAWPLACE}
  #  )
  # install(TARGETS ${TARGET_NAME}_cpp DESTINATION amfplacer/app)
  # file(GLOB INSTALL_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/*.py)
  # install(FILES ${INSTALL_SRCS} DESTINATION amfplacer/app)
endif()
execute_process(COMMAND  cp ${CMAKE_SOURCE_DIR}/doc/NotoSans-Regular.ttf ${CMAKE_BINARY_DIR}/NotoSans-Regular.ttf OUTPUT_VARIABLE tmp)

add_executable(partitionHyperGraph ${CMAKE_SOURCE_DIR}/amfplacer/app/partitionHyperGraph.cc)
link_directories(${CMAKE_BINARY_DIR}/PaToH/)
target_link_libraries(partitionHyperGraph  ${Boost_LIBRARIES}  m ${PaToH_LIBS}  )
install(
  TARGETS partitionHyperGraph DESTINATION ${CMAKE_SOURCE_DIR}
  )

file(GLOB INSTALL_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/*.py)
install(FILES ${INSTALL_SRCS} DESTINATION amfplacer/app)
