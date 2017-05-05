set(EFFCEE_THIRD_PARTY_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}" CACHE STRING
  "Root location of all third_party projects")
set(EFFCEE_GOOGLETEST_DIR "${EFFCEE_THIRD_PARTY_ROOT_DIR}/googletest" CACHE STRING
  "Location of googletest source")
set(EFFCEE_RE2_DIR "${EFFCEE_THIRD_PARTY_ROOT_DIR}/re2" CACHE STRING
  "Location of re2 source")

# Configure third party projects.
if(${EFFCEE_ENABLE_TESTS})
  if (IS_DIRECTORY ${EFFCEE_GOOGLETEST_DIR})
    add_subdirectory(${EFFCEE_GOOGLETEST_DIR} googletest)
  endif()
  if (NOT TARGET gmock)
    message(FATAL_ERROR "gmock was not found - required for tests")
  endif()
endif()

if (IS_DIRECTORY ${EFFCEE_RE2_DIR})

  if ("${MSVC}")
  # Disable RE2's Level 4 MSVC warnings:
  # 4201: nameless struct/union
  # 4815: zero-sized array in stack
  # EHs:  Exception handling
  add_compile_options(/wd4201 /wd4815 /EHs)
  endif()

  add_subdirectory(${EFFCEE_RE2_DIR} re2)
endif()
if (NOT TARGET re2)
  message(FATAL_ERROR "re2 was not found - required for compilation")
endif()
