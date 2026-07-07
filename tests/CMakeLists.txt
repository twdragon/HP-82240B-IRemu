cmake_minimum_required(VERSION 3.13)

project(HP82240B-IRemu_tests C CXX)

enable_testing()

add_executable(test_ecc 
    ecc.c
)
add_executable(test_frame 
    frame.c
)
add_executable(test_utf8transcode
    utf8transcode.c
)
add_executable(test_bytesplit
    bytesplit.c
)
add_executable(test_ringbuffer
    ringbuffer.c
)
add_executable(test_pngle
    pngle-snapshot/miniz.c
    pngle-snapshot/pngle.c
    pngle.c
)
target_include_directories(test_pngle PRIVATE pngle-snapshot)

add_test(ecc
    test_ecc
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)
add_test(frame
    test_frame
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)
add_test(utf8transcode
    test_utf8transcode
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)
add_test(bytesplit
    test_bytesplit
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)
add_test(ringbuffer
    test_ringbuffer
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)
add_test(pngle
    test_pngle
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)
