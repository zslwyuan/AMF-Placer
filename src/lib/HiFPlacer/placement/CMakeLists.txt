cmake_minimum_required(VERSION 3.10)

add_subdirectory(placementInfo)
add_subdirectory(globalPlacement)
add_subdirectory(packing)
add_subdirectory(legalization)
add_subdirectory(placementTiming)

file(GLOB INSTALL_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/*.py")
install(
    FILES ${INSTALL_SRCS} DESTINATION lib/HiFPlacer/placement
    )