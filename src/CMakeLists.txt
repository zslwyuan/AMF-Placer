cmake_minimum_required(VERSION 3.10)

project(AMFPlacer)

set(CMAKE_CXX_STANDARD 17)
find_package(OpenMP)
add_compile_options ( -std=c++17 -O3  -fopenmp -Wall )
if(OPENMP_FOUND)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 ${OpenMP_CXX_FLAGS}")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 ${OpenMP_C_FLAGS}")
else(OPENMP_FOUND)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 ")
  message (STATUS "OpenMP not found")
endif()


set(CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR})


# find required packages
include(FetchContent)



# Boost
# set(BOOST_ROOT "")

set(Boost_USE_STATIC_LIBS OFF)
set(BOOST_VER 1.65.1)
set(Boost_NO_BOOST_CMAKE ON)
set(BOOST_COMPONENTS filesystem program_options system log thread)
ADD_DEFINITIONS(-DBOOST_LOG_DYN_LINK)

# Workaround function to allow cmake call `find_package` twice. Avoide side effects from local variables, which are produced be `find_package`
function(findBoost Required)
    find_package(Boost ${BOOST_VER} ${Required} 
        COMPONENTS ${BOOST_COMPONENTS}
        OPTIONAL_COMPONENTS unit_test_framework
    )
    set(Boost_FOUND ${Boost_FOUND} PARENT_SCOPE)
    set(Boost_LIBRARIES ${Boost_LIBRARIES} PARENT_SCOPE)
    set(Boost_INCLUDE_DIRS ${Boost_INCLUDE_DIRS} PARENT_SCOPE)
endfunction()

findBoost("")

if(NOT Boost_FOUND)
  string(REPLACE "." "_" BOOST_VER_ ${BOOST_VER}) 
  set(BOOST_URL "https://boostorg.jfrog.io/artifactory/main/release/${BOOST_VER}/source/boost_${BOOST_VER_}.tar.bz2" CACHE STRING "Boost download URL")
  set(BOOST_URL_SHA256 "9807a5d16566c57fd74fb522764e0b134a8bbe6b6e8967b83afefd30dcd3be81" CACHE STRING "Boost download URL SHA256 checksum")
  option(BOOST_DISABLE_TESTS "Do not build test targets" OFF)
  include(FetchContent)
  set(FETCHCONTENT_QUIET OFF)
  FetchContent_Declare(
    Boost
    URL ${BOOST_URL}
    URL_HASH SHA256=${BOOST_URL_SHA256}
  )
  FetchContent_GetProperties(Boost)
  set(FETCHCONTENT_QUIET OFF)
  if(NOT Boost_POPULATED)
    message(STATUS "Fetching Boost")
    FetchContent_Populate(Boost)
    message(STATUS "Fetching Boost - done")
    message(STATUS " boost source dir is ${boost_SOURCE_DIR} ")
    message(STATUS " boost binary dir is ${boost_BINARY_DIR} ")
    string(JOIN "," BOOST_WITH_LIBRARIES ${BOOST_COMPONENTS}) 
    execute_process(
      COMMAND ./bootstrap.sh --prefix=${boost_BINARY_DIR} --with-libraries=${BOOST_WITH_LIBRARIES}
      WORKING_DIRECTORY ${boost_SOURCE_DIR}
      RESULT_VARIABLE result
    )
    if(NOT result EQUAL "0")
      message( FATAL_ERROR "Bad exit status of bootstrap")
    endif()
    execute_process(
      COMMAND ./b2 install -j8
      WORKING_DIRECTORY ${boost_SOURCE_DIR}
      RESULT_VARIABLE result
    )
    if(NOT result EQUAL "0")
      message( FATAL_ERROR "Bad exit status of b2")
    endif()
    set(BOOST_ROOT ${boost_BINARY_DIR} CACHE PATH "Root folder to find boost" FORCE)
    set(Boost_DIR ${boost_BINARY_DIR} CACHE PATH "Root folder to find boost" FORCE)
  endif()
  findBoost(REQUIRED)
endif()

message(STATUS " boost libs ${Boost_LIBRARIES}")
message(STATUS " boost includes ${Boost_INCLUDE_DIRS}")
include_directories(${Boost_INCLUDE_DIRS})

# ABSL
if (NOT TARGET absl::strings OR NOT TARGET absl::status OR NOT TARGET absl::span)
  message(STATUS "AMFPlacer fetching packages: `absl` targets not found. Attempting to fetch contents...")
  FetchContent_Declare(
    abseil-cpp 
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib/3rdParty/abseil-cpp-src
    # GIT_REPOSITORY https://github.com/abseil/abseil-cpp.git
    # GIT_TAG        origin/master
  )
  FetchContent_MakeAvailable(abseil-cpp)
else()
  message(STATUS "AMFPlacer fetching packages: `absl` targets found.")
endif()

# EIGEN
# Eigen is usually available as a system package, so we use find_package.
if (NOT TARGET Eigen3::Eigen)
  message(STATUS "AMFPlacer fetching packages: `Eigen3` targets not found. Attempting to find package...")
  FetchContent_Declare(
    eigen
    # GIT_REPOSITORY https://gitlab.com/libeigen/eigen.git
    # GIT_TAG        origin/master
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib/3rdParty/eigen3-src
  )
  FetchContent_MakeAvailable(eigen)
  set(EIGEN3_INCLUDE_DIR ${eigen_SOURCE_DIR})
  # find_package(Eigen3 3.3.7 REQUIRED NO_MODULE)
else()
  message(STATUS "AMFPlacer fetching packages: `Eigen3` targets found.")
endif()

# OSQP
if (NOT TARGET osqpstatic)
  message(STATUS "AMFPlacer fetching packages: `osqp` targets not found. Attempting to find contents...")
  FetchContent_Declare(
    osqp
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib/3rdParty/osqp-src
    # GIT_REPOSITORY https://github.com/oxfordcontrol/osqp.git
    # GIT_TAG        origin/master
  )
  FetchContent_MakeAvailable(osqp)
else()
  message(STATUS "AMFPlacer fetching packages: `osqp` targets found.")
endif()

find_package(ZLIB)

include_directories(
  ${Boost_INCLUDE_DIR}
  ${EIGEN3_INCLUDE_DIR}
  ${ZLIB_INCLUDE_DIRS}
)

add_subdirectory(lib)

set(SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/app/AMFPlacer/main.cc)
add_executable(AMFPlacer ${SOURCE_FILES})
add_executable(partitionHyperGraph lib/3rdParty/partitionHyperGraph.cc)

include_directories(./lib/
./lib/HiFPlacer/designInfo/ 
./lib/HiFPlacer/deviceInfo/ 
./lib/utils/
./lib/HiFPlacer/placement/placementInfo/
./lib/HiFPlacer/placement/globalPlacement/
./lib/HiFPlacer/placement/legalization/
./lib/HiFPlacer/placement/packing/
./lib/HiFPlacer/placement/placementTiming/
./lib/3rdParty
./lib/HiFPlacer/problemSolvers
${abseil-cpp_SOURCE_DIR} 
)

link_directories(${CMAKE_SOURCE_DIR}/lib/3rdParty/PaToH/)

target_link_libraries(AMFPlacer GlobalPlacer DesignInfo DeviceInfo PlacementInfo PlacementTiming Packing Legalization ProblemSolvers Utils
                        ${CMAKE_SOURCE_DIR}/lib/3rdParty/PaToH/libpatoh.a 
                        pthread 
                        ${ZLIB_LIBRARIES}  ${Boost_LIBRARIES} ) #GL GLU glut GLEW
target_link_libraries(partitionHyperGraph  ${Boost_LIBRARIES}  m ${CMAKE_SOURCE_DIR}/lib/3rdParty/PaToH/libpatoh.a )

