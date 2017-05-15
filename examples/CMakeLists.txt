add_executable(effcee-example main.cc)

target_link_libraries(effcee-example effcee)
if(UNIX)
  target_link_libraries(effcee-example -pthread)
endif(UNIX)

if(EFFCEE_BUILD_TESTING)
  add_test(NAME effcee-example
          COMMAND ${PYTHON_EXE}
                  effcee-example-driver.py
                  $<TARGET_FILE:effcee-example>
                  example_data.txt
                  "CHECK: Hello"
                  "CHECK-SAME: world"
                  "CHECK-NEXT: Bees"
                  "CHECK-NOT: Sting"
                  "CHECK: Honey"
          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
endif(EFFCEE_BUILD_TESTING)
