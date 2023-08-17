cmake_minimum_required(VERSION 3.10)

project(AMFPlacer)

set(CMAKE_CXX_STANDARD 14)
find_package(OpenMP)
add_compile_options ( -std=c++14 -O3  -fopenmp -Wall )
if(OPENMP_FOUND)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 ${OpenMP_CXX_FLAGS}")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 ${OpenMP_C_FLAGS}")
else(OPENMP_FOUND)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 ")
  message (STATUS "OpenMP not found")
endif()

# if(CMAKE_CXX_FLAGS)
# string(REGEX REPLACE "-std=c\\+\\+([0-9a-z]+)" " " CMAKE_CXX_FLAGS_INIT ${CMAKE_CXX_FLAGS})
# endif(CMAKE_CXX_FLAGS)
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_INIT}" CACHE STRING "Flags used by the compiler during all build types." FORCE)

# if(NOT CMAKE_CXX_ABI)
#     set(CMAKE_CXX_ABI 0 CACHE STRING
#         "Choose the value for _GLIBCXX_USE_CXX11_ABI, options are: 0|1."
#         FORCE)
# endif(NOT CMAKE_CXX_ABI)
# message(STATUS "CMAKE_CXX_ABI: _GLIBCXX_USE_CXX11_ABI=${CMAKE_CXX_ABI}")
# add_definitions(-D_GLIBCXX_USE_CXX11_ABI=${CMAKE_CXX_ABI})

if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set (CMAKE_INSTALL_PREFIX "${PROJECT_SOURCE_DIR}" CACHE PATH "Prefix prepended to install directories" FORCE )
    #set(CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR})
endif()
message(STATUS "CMAKE_INSTALL_PREFIX: ${CMAKE_INSTALL_PREFIX}")

SET(CMAKE_PREFIX_PATH "/usr/lib/x86_64-linux-gnu/cmake")
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Core REQUIRED)
find_package(Qt5Gui REQUIRED)


# Boost
# set(BOOST_ROOT "")

#set(Boost_USE_STATIC_LIBS OFF)
#set(BOOST_VER 1.65.1)
#set(Boost_NO_BOOST_CMAKE ON)
#set(BOOST_COMPONENTS filesystem program_options system log thread)
find_package(ZLIB REQUIRED)
#set(Boost_NO_BOOST_CMAKE TRUE)
#find_package(Boost 1.65.1 REQUIRED)
#ADD_DEFINITIONS(-DBOOST_LOG_DYN_LINK)

#find_package(ZLIB)




# include(cmake/TorchExtension.cmake)
include(cmake/configure.cmake)
if(NOT CMAKE_PYBIND11_TYPE)
  set(CMAKE_PYBIND11_TYPE false CACHE BOOL "wether to build c++ binary")
endif()
add_definitions( -DCMAKE_PYBIND11_TYPE=${CMAKE_PYBIND11_TYPE})
message(STATUS CMAKE_PYBIND11_TYPE=${CMAKE_PYBIND11_TYPE})
#add_subdirectory(lib/3rdParty/pybind11)
add_subdirectory(lib)
# add_subdirectory(app)

set(SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/app/AMFPlacer/main.cc)
add_executable(AMFPlacer ${SOURCE_FILES})
add_executable(partitionHyperGraph lib/3rdParty/partitionHyperGraph.cc)

include_directories(./lib/
${CMAKE_BINARY_DIR}/PaToH/
./lib/HiFPlacer/designInfo/ 
./lib/HiFPlacer/deviceInfo/ 
./lib/utils/
./lib/HiFPlacer/placement/placementInfo/
./lib/HiFPlacer/placement/globalPlacement/
./lib/HiFPlacer/placement/legalization/
./lib/HiFPlacer/placement/packing/
./lib/HiFPlacer/placement/placementTiming/
./lib/3rdParty
#./lib/3rdParty/eigen-src
${EIGEN3_INCLUDE_DIR}
./lib/HiFPlacer/problemSolvers
#${abseil-cpp_SOURCE_DIR} 
)

target_link_libraries(AMFPlacer GlobalPlacer DesignInfo DeviceInfo PlacementInfo PlacementTiming Packing Legalization ProblemSolvers Utils
                       ${CMAKE_BINARY_DIR}/PaToH/libpatoh.a 
                       pthread 
                       ${ZLIB_LIBRARIES}  ${Boost_LIBRARIES} Rendering Qt5::Widgets blend2d::blend2d) #GL GLU glut GLEW
target_link_libraries(partitionHyperGraph  ${Boost_LIBRARIES}  m ${CMAKE_BINARY_DIR}/PaToH/libpatoh.a )

execute_process(COMMAND  cp ${CMAKE_SOURCE_DIR}/../doc/NotoSans-Regular.ttf ${CMAKE_BINARY_DIR}/NotoSans-Regular.ttf OUTPUT_VARIABLE tmp)

file(GLOB INSTALL_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/app/*.py")
install(
    FILES ${INSTALL_SRCS} DESTINATION app
    )
file(GLOB unittest "${CMAKE_CURRENT_SOURCE_DIR}/unittest/*.py")
install(
    FILES ${INSTALL_SRCS} DESTINATION unittest
    )

