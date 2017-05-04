cmake_minimum_required(VERSION 2.8.12)
project(effcee C CXX)
enable_testing()

option(EFFCEE_SKIP_TESTS "Skip building tests" ${EFFCEE_SKIP_TESTS})
option(EFFCEE_ENABLE_SAMPLES "Enable building samples" ${EFFCEE_ENABLE_SAMPLES})

if(NOT ${EFFCEE_SKIP_TESTS})
  set(EFFCEE_ENABLE_TESTS ON)
endif()
if(${EFFCEE_ENABLE_TESTS})
  message(STATUS "Configuring Effcee to build tests.")
else()
  message(STATUS "Configuring Effcee to avoid building tests.")
endif()

include(cmake/setup_build.cmake)
include(cmake/utils.cmake)

add_subdirectory(third_party)
add_subdirectory(effcee)

if(${EFFCEE_ENABLE_SAMPLES})
  add_subdirectory(examples)
endif()