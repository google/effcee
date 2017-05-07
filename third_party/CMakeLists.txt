# Suppress all warnings from third-party projects.
set_property(DIRECTORY APPEND PROPERTY COMPILE_OPTIONS -w)

set(EFFCEE_THIRD_PARTY_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}" CACHE STRING
  "Root location of all third_party projects")
set(EFFCEE_GOOGLETEST_DIR "${EFFCEE_THIRD_PARTY_ROOT_DIR}/googletest" CACHE STRING
  "Location of googletest source")
set(EFFCEE_RE2_DIR "${EFFCEE_THIRD_PARTY_ROOT_DIR}/re2" CACHE STRING
  "Location of re2 source")

# Configure third party projects.
if(EFFCEE_BUILD_TESTING)
  if (IS_DIRECTORY ${EFFCEE_GOOGLETEST_DIR})
    add_subdirectory(${EFFCEE_GOOGLETEST_DIR} googletest)
  endif()
  if (NOT TARGET gmock)
    message(FATAL_ERROR "gmock was not found - required for tests")
  endif()
endif()

if (IS_DIRECTORY ${EFFCEE_RE2_DIR})
  add_subdirectory(${EFFCEE_RE2_DIR} re2)
endif()
if (NOT TARGET re2)
  message(FATAL_ERROR "re2 was not found - required for compilation")
endif()
