cmake_minimum_required(VERSION 3.13)

set(PICO_SDK_IMPORTLIB_PATH $ENV{PICO_SDK_PATH}/external)

find_file(PICO_SDK_IMPORTLIB "pico_sdk_import.cmake" 
    HINTS ${PICO_SDK_IMPORTLIB_PATH}
          ../pico-sdk/external
          ../sdk/external
    REQUIRED)
get_filename_component(PICO_SDK_DIRECTORY ${PICO_SDK_IMPORTLIB} DIRECTORY)
get_filename_component(PICO_SDK_DIRECTORY ${PICO_SDK_DIRECTORY}/.. REALPATH)
set(ENV{PICO_SDK_PATH} "${PICO_SDK_DIRECTORY}")

include(${PICO_SDK_IMPORTLIB})

project(HP82240B-IRemu C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Tests: host architecture
include(ExternalProject)
ExternalProject_Add(HP82240B-IRemu_tests
    PREFIX tests
    SOURCE_DIR ${CMAKE_SOURCE_DIR}/tests
    BINARY_DIR ${CMAKE_BINARY_DIR}/tests
    CMAKE_ARGS "-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}"
    BUILD_ALWAYS ON
    INSTALL_COMMAND ""
    TEST_BEFORE_INSTALL ON
)

pico_sdk_init()

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/pio
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_executable(testprint
    src/redeye.c
    src/printer.c
)

# Generate the PIO header from the .pio source
pico_generate_pio_header(testprint ${CMAKE_CURRENT_SOURCE_DIR}/pio/redeye.pio)

if (PICO_CYW43_SUPPORTED)
    target_link_libraries(testprint 
        pico_cyw43_arch_none
    )
endif()

target_link_libraries(testprint
    pico_stdlib
    pico_bit_ops
    hardware_pio
    hardware_clocks
    hardware_gpio
)

# Enable USB or UART stdout if you wish (for debugging)
pico_enable_stdio_usb(testprint 1)

# Create UF2, bin, etc
pico_add_extra_outputs(testprint)

