cmake_minimum_required(VERSION 3.10)




set(Util_SRC
${CMAKE_SOURCE_DIR}/amfplacer/utils/dumpZip.cc
${CMAKE_SOURCE_DIR}/amfplacer/utils/strPrint.cc
${CMAKE_SOURCE_DIR}/amfplacer/utils/stringCheck.cc
${CMAKE_SOURCE_DIR}/amfplacer/utils/sysInfo.cc
)

file(GLOB placementInfo_SRC ${CMAKE_SOURCE_DIR}/amfplacer/HiFPlacer/placement/placementInfo/*.cc)
file(GLOB PlacementTimingSrc ${CMAKE_SOURCE_DIR}/amfplacer/HiFPlacer/placement/placementTiming/*.cc)
file(GLOB GlobalPlacement_SRC ${CMAKE_SOURCE_DIR}/amfplacer/HiFPlacer/placement/globalPlacement/*.cc)
file(GLOB ProblemSolvers_SRC ${CMAKE_SOURCE_DIR}/amfplacer/HiFPlacer/problemSolvers/*.cc)
file(GLOB Device_Src ${CMAKE_SOURCE_DIR}/amfplacer/HiFPlacer/deviceInfo/*.cc)
file(GLOB Design_Src ${CMAKE_SOURCE_DIR}/amfplacer/HiFPlacer/designInfo/*.cc)
set(Legalization_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/amfplacer/HiFPlacer/placement/legalization)
file(GLOB Legalization_SRC ${CMAKE_SOURCE_DIR}/amfplacer/HiFPlacer/placement/legalization/*.cc)
 
set(LINKS_LIB
${ZLIB_LIBRARIES}
${Boost_LIBRARIES}
)

set(PaToH_LIBS
${CMAKE_SOURCE_DIR}/thirdparty/PaToH/libpatoh.a 
)

add_subdirectory(designInfo)
add_subdirectory(deviceInfo)
add_subdirectory(placement)
add_subdirectory(problemSolvers)

file(GLOB INSTALL_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/*.py")
install(
    FILES ${INSTALL_SRCS} DESTINATION lib/HiFPlacer
    )
