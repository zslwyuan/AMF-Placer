cmake_minimum_required(VERSION 3.10)

add_subdirectory(designInfo)
add_subdirectory(deviceInfo)
add_subdirectory(placement)
add_subdirectory(problemSolvers)

file(GLOB INSTALL_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/*.py")
install(
    FILES ${INSTALL_SRCS} DESTINATION lib/HiFPlacer
    )
