cmake_minimum_required(VERSION 3.10)

add_subdirectory(3rdParty)
add_subdirectory(HiFPlacer)
add_subdirectory(utils)

file(GLOB INSTALL_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/*.py")
install(
    FILES ${INSTALL_SRCS} DESTINATION lib
    )