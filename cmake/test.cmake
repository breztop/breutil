# Helper function to add a Boost.Test target
function(add_boost_test TARGET_NAME )
    set(SOURCE_FILE ${ARGN})
    message(STATUS "Adding Boost test target: ${TARGET_NAME}")
    add_executable(${TARGET_NAME} ${SOURCE_FILE})
    link_boost(${TARGET_NAME} unit_test_framework)

    # Add the test to CTest
    add_test(NAME ${TARGET_NAME} COMMAND ${TARGET_NAME})

    # Set the working directory for the test
    set_tests_properties(${TARGET_NAME} PROPERTIES
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )

    # Enable C++ standard if needed
    set_target_properties(${TARGET_NAME} PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED ON
    )

endfunction()
