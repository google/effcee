cmake_minimum_required(VERSION 2.8.12)
project(effcee C CXX)
enable_testing()

option(EFFCEE_BUILD_TESTING "Enable testing for Effcee" ON)
if(${EFFCEE_BUILD_TESTING})
  message(STATUS "Configuring Effcee to build tests.")
else()
  message(STATUS "Configuring Effcee to avoid building tests.")
endif()

include(cmake/setup_build.cmake)
include(cmake/utils.cmake)

add_subdirectory(third_party)
add_subdirectory(effcee)
add_subdirectory(examples)
