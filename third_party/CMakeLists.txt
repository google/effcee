# Suppress all warnings from third-party projects.
set_property(DIRECTORY APPEND PROPERTY COMPILE_OPTIONS -w)

# Set alternate root directory for third party sources.
set(EFFCEE_THIRD_PARTY_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}" CACHE STRING
  "Root location of all third_party projects")

# Find googletest and gmock
if(${googletest-distribution_SOURCE_DIR})
  set(EFFCEE_GOOGLETEST_DIR "${googletest-distribution_SOURCE_DIR}" CACHE STRING
	  "Location of googletest source")
else()
  set(EFFCEE_GOOGLETEST_DIR "${EFFCEE_THIRD_PARTY_ROOT_DIR}/googletest" CACHE STRING
          "Location of googletest source")
endif()

# Find re2
if(RE2_SOURCE_DIR)
  set(EFFCEE_RE2_DIR "${RE2_SOURCE_DIR}" CACHE STRING "Location of re2 source" FORCE)
else()
  set(EFFCEE_RE2_DIR "${EFFCEE_THIRD_PARTY_ROOT_DIR}/re2" CACHE STRING
    "Location of re2 source")
endif()

# Configure third party projects.
if(EFFCEE_BUILD_TESTING)
  # Our tests use ::testing::Combine.  Force the ability to use it, working
  # around googletest's possibly faulty compiler detection logic.
  # See https://github.com/google/googletest/issues/1352
  add_definitions(-DGTEST_HAS_COMBINE=1)

  if (NOT TARGET gmock)
    if (IS_DIRECTORY ${EFFCEE_GOOGLETEST_DIR})
      add_subdirectory(${EFFCEE_GOOGLETEST_DIR} googletest)
    endif()
  endif()
  if (NOT TARGET gmock)
    message(FATAL_ERROR "gmock was not found - required for tests")
  endif()
endif()

if (NOT TARGET re2)
  if (IS_DIRECTORY ${EFFCEE_RE2_DIR})
    add_subdirectory(${EFFCEE_RE2_DIR} re2)
  endif()
endif()
if (NOT TARGET re2)
  message(FATAL_ERROR "re2 was not found - required for compilation")
endif()
