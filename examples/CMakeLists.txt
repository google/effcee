add_executable(effcee-example main.cc)

target_link_libraries(effcee-example effcee)
if(UNIX)
  target_link_libraries(effcee-example -pthread)
endif(UNIX)

add_test(NAME effcee-example
        COMMAND ${PYTHON_EXE}
                effcee-example-driver.py
                ${CMAKE_CURRENT_BINARY_DIR}/effcee-example
                example_data.txt
                "CHECK: Hello"
                "CHECK-SAME: world"
                "CHECK-NEXT: Bees"
                "CHECK-NOT: Sting"
                "CHECK: Honey"
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
