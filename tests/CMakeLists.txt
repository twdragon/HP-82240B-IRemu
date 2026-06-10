cmake_minimum_required(VERSION 3.13)

project(HP82240B-IRemu_tests C CXX)

enable_testing()

add_executable(test_ecc 
    ecc.c
)
add_executable(test_frame 
    frame.c
)

add_test(ecc
    test_ecc
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)
add_test(frame
    test_frame
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)
