cmake_minimum_required(VERSION 3.27 FATAL_ERROR)

project(cpp_iterator)

# ##############################################################################
# # Set the CMake module path and include the functions
# ##############################################################################
include(FetchContent)

set(PROJ_CPP_STANDARD
    "17"
    CACHE STRING "C++ standard to use for the project")
set(CMAKE_VERBOSE_MAKEFILE false) # more compile info dump if set it to true
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Settings and Checking build binay in the root of source dir IS NOT ALLOWED
if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
  message(
    FATAL_ERROR
      "FATAL: In-source builds are not allowed.
           You should create a separate directory for build files.
           ")
endif()

# Generating a compilation database: compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ##############################################################################
# C++17 Support Check
# ##############################################################################
include(CheckCXXCompilerFlag)
if(NOT MSVC)
  check_cxx_compiler_flag("-std=c++17" COMPILER_SUPPORTS_CXX17)
else()
  check_cxx_compiler_flag("/std=c++17" COMPILER_SUPPORTS_CXX17)
endif()

if(COMPILER_SUPPORTS_CXX17)
  message(STATUS "c++17 is supported by the compiler")
else()
  message(FATAL_ERROR "
            FATAL: the compiler does not support c++17")
endif()

message(STATUS "CMAKE Version: ${CMAKE_VERSION}")
message(STATUS "System name: ${CMAKE_SYSTEM_NAME}")
message(STATUS "Host System name: ${CMAKE_HOST_SYSTEM_NAME}")
message(STATUS "System version: ${CMAKE_SYSTEM_VERSION}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C compiler: ${CMAKE_C_COMPILER}")
message(STATUS "C++ compiler: ${CMAKE_CXX_COMPILER}")

# ##############################################################################
# # Default built type (Release)
# ##############################################################################
# Acceptable build types: Debug, Release, RelWithDebInfo, MinSizeRel
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
  message(
    WARNING
      "No build type selected. You need to pass -DCMAKE_BUILD_TYPE=<type> in order to configure bats protocol.
  Available options are:
  * -DCMAKE_BUILD_TYPE=Release - For an optimized build with no assertions or debug info.
  * -DCMAKE_BUILD_TYPE=Debug - For an unoptimized build with assertions and debug info.
  * -DCMAKE_BUILD_TYPE=RelWithDebInfo - For an optimized build with no assertions but with debug info.
  * -DCMAKE_BUILD_TYPE=MinSizeRel - For a build optimized for size instead of speed.
  ")
endif()

# Set default compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_NO_WARN_FLAGS} -fPIC")

# Set build specific compiler flags
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -g")
set(CMAKE_CXX_FLAGS_RELEASE
    "${CMAKE_CXX_FLAGS_RELEASE} -Wall -Wextra -Werror -O3 -fstack-protector-all -fPIE -D_FORTIFY_SOURCE=2"
)
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -g -O0")

# strip all symbols from the compiled binary
if(CMAKE_BUILD_TYPE STREQUAL "Release")
  if(NOT MSVC)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-s -z now -pie")
  else()
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /RELEASE")
  endif()
endif()

FetchContent_Declare(
  googletest
  GIT_REPOSITORY git@github.com:google/googletest.git
  GIT_TAG v1.14.0
  FIND_PACKAGE_ARGS)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt
    ON
    CACHE BOOL "" FORCE)
set(gmock_build_tests
    OFF
    CACHE BOOL "" FORCE)
set(gtest_build_tests
    OFF
    CACHE BOOL "" FORCE)
set(INSTALL_GTEST
    OFF
    CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)
message("GoogleTest source directory is :" ${googletest_SOURCE_DIR})
include_directories(${googletest_SOURCE_DIR}/googletest/include)

# google test case
include(GoogleTest)
enable_testing()
add_custom_target(all_test ${CMAKE_CTEST_COMMAND} -V)
set(CTEST_OUTPUT_ON_FAILURE true)
set(GTEST_COLOR true)

add_subdirectory(src/test)

# add_executable(cpp_iterator src/cpp_iterator.cc)
