cmake_minimum_required(VERSION 2.8.12)
project(effcee C CXX)
enable_testing()

option(EFFCEE_BUILD_TESTING "Enable testing for Effcee" ON)
if(${EFFCEE_BUILD_TESTING})
  message(STATUS "Configuring Effcee to build tests.")
  if(MSVC)
    # Our tests use ::testing::Combine.  Force the ability to use it, working
    # around googletest's possibly faulty compiler detection logic.
    # See https://github.com/google/googletest/issues/1352
    add_definitions(-DGTEST_HAS_COMBINE=1)
  endif(MSVC)
else()
  message(STATUS "Configuring Effcee to avoid building tests.")
endif()

option(EFFCEE_BUILD_SAMPLES "Enable building sample Effcee programs" ON)
if(${EFFCEE_BUILD_SAMPLES})
  message(STATUS "Configuring Effcee to build samples.")
else()
  message(STATUS "Configuring Effcee to avoid building samples.")
endif()

include(cmake/setup_build.cmake)
include(cmake/utils.cmake)

add_subdirectory(third_party)
add_subdirectory(effcee)

if(${EFFCEE_BUILD_SAMPLES})
  add_subdirectory(examples)
endif()
