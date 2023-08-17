

## Boost
## set(BOOST_ROOT "")
#
set(Boost_USE_STATIC_LIBS OFF)
set(BOOST_VER 1.65.1)
set(Boost_NO_BOOST_CMAKE ON)
set(BOOST_COMPONENTS filesystem program_options system log thread)
ADD_DEFINITIONS(-DBOOST_LOG_DYN_LINK)
#
## Workaround function to allow cmake call `find_package` twice. Avoide side effects from local variables, which are produced be `find_package`
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
message(STATUS " boost libs ${Boost_LIBRARIES}")
message(STATUS " boost includes ${Boost_INCLUDE_DIRS}")

#if(NOT Boost_FOUND)
#  string(REPLACE "." "_" BOOST_VER_ ${BOOST_VER}) 
#  set(BOOST_URL "https://boostorg.jfrog.io/artifactory/main/release/${BOOST_VER}/source/boost_${BOOST_VER_}.tar.bz2" CACHE STRING "Boost download URL")
#  set(BOOST_URL_SHA256 "9807a5d16566c57fd74fb522764e0b134a8bbe6b6e8967b83afefd30dcd3be81" CACHE STRING "Boost download URL SHA256 checksum")
#  option(BOOST_DISABLE_TESTS "Do not build test targets" OFF)
#  include(FetchContent)
#  set(FETCHCONTENT_QUIET OFF)
#  FetchContent_Declare(
#    Boost
#    URL ${BOOST_URL}
#    URL_HASH SHA256=${BOOST_URL_SHA256}
#  )
#  FetchContent_GetProperties(Boost)
#  set(FETCHCONTENT_QUIET OFF)
#  if(NOT Boost_POPULATED)
#    message(STATUS "Fetching Boost")
#    FetchContent_Populate(Boost)
#    message(STATUS "Fetching Boost - done")
#    message(STATUS " boost source dir is ${boost_SOURCE_DIR} ")
#    message(STATUS " boost binary dir is ${boost_BINARY_DIR} ")
#    string(JOIN "," BOOST_WITH_LIBRARIES ${BOOST_COMPONENTS}) 
#    execute_process(
#      COMMAND ./bootstrap.sh --prefix=${boost_BINARY_DIR} --with-libraries=${BOOST_WITH_LIBRARIES}
#      WORKING_DIRECTORY ${boost_SOURCE_DIR}
#      RESULT_VARIABLE result
#    )
#    if(NOT result EQUAL "0")
#      message( FATAL_ERROR "Bad exit status of bootstrap")
#    endif()
#    execute_process(
#      COMMAND ./b2 install -j8
#      WORKING_DIRECTORY ${boost_SOURCE_DIR}
#      RESULT_VARIABLE result
#    )
#    if(NOT result EQUAL "0")
#      message( FATAL_ERROR "Bad exit status of b2")
#    endif()
#    set(BOOST_ROOT ${boost_BINARY_DIR} CACHE PATH "Root folder to find boost" FORCE)
#    set(Boost_DIR ${boost_BINARY_DIR} CACHE PATH "Root folder to find boost" FORCE)
#  endif()
#  findBoost(REQUIRED)
#endif()

message(STATUS "AMFPlacer fetching packages: `PaToH` == ${CMAKE_BINARY_DIR}")

if(NOT EXISTS ${CMAKE_BINARY_DIR}/PaToH/libpatoh.a)
   message(STATUS "AMFPlacer fetching packages: `PaToH` targets not found. Attempting to find package...")

   execute_process(COMMAND  cp -rf ${CMAKE_CURRENT_SOURCE_DIR}/lib/3rdParty/PaToH ${CMAKE_BINARY_DIR})
   #execute_process(COMMAND  wget https://faculty.cc.gatech.edu/~umit/PaToH/patoh-Linux-x86_64.tar.gz -P ${CMAKE_BINARY_DIR})
   #execute_process(COMMAND  mkdir ${CMAKE_BINARY_DIR}/PaToH)
   #execute_process(COMMAND  tar -xf  ${CMAKE_BINARY_DIR}/patoh-Linux-x86_64.tar.gz -C  ${CMAKE_BINARY_DIR}/PaToH --strip-components=2)
endif()

# EIGEN
# Eigen is usually available as a system package, so we use find_package.
include(FetchContent)
if (NOT TARGET Eigen3::Eigen)
  message(STATUS "AMFPlacer fetching packages: `Eigen3` targets not found. Attempting to find package...")
  FetchContent_Declare(
    eigen
    # GIT_REPOSITORY https://gitlab.com/libeigen/eigen.git
    # GIT_TAG        3.3.9
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib/3rdParty/eigen-src
  )
  FetchContent_MakeAvailable(eigen)
  set(EIGEN3_INCLUDE_DIR ${eigen_SOURCE_DIR})
  find_package(Eigen3 EXACT 3.3.9 REQUIRED NO_MODULE)
else()
  message(STATUS "AMFPlacer fetching packages: `Eigen3` targets found.")
endif()

add_subdirectory(lib/3rdParty/pybind11)
function(add_pybind11_extension target_name)
 set(multiValueArgs EXTRA_INCLUDE_DIRS EXTRA_LINK_LIBRARIES EXTRA_DEFINITIONS)
 cmake_parse_arguments(ARG "" "" "${multiValueArgs}" ${ARGN})
 list(FILTER ARG_UNPARSED_ARGUMENTS EXCLUDE REGEX ".*cu$")
 pybind11_add_module(${target_name} MODULE ${ARG_UNPARSED_ARGUMENTS})
 target_include_directories(${target_name} PRIVATE ${ARG_EXTRA_INCLUDE_DIRS})
 target_link_libraries(${target_name} PRIVATE ${ARG_EXTRA_LINK_LIBRARIES})
 target_compile_definitions(${target_name} PRIVATE 
   ${ARG_EXTRA_DEFINITIONS})
endfunction()