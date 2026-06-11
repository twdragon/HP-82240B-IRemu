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
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-stringop-overflow")

# Options
option(BUILD_PRINTER_TESTER
    "Builds printer tester application instead of the main driver"
    OFF
)

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

if(BUILD_PRINTER_TESTER)

    add_executable(test_printer
        src/redeye.c
        src/test_printer.c
    )

    pico_generate_pio_header(test_printer ${CMAKE_CURRENT_SOURCE_DIR}/pio/redeye.pio)

    if (PICO_CYW43_SUPPORTED)
        target_link_libraries(test_printer
            pico_cyw43_arch_none
        )
    endif()

    target_link_libraries(test_printer
        pico_stdlib
        pico_bit_ops
        hardware_pio
        hardware_clocks
        hardware_gpio
    )

    pico_enable_stdio_usb(test_printer 1)
    pico_add_extra_outputs(test_printer)
    
else(BUILD_PRINTER_TESTER)

    add_executable(printer
        src/redeye.c
        src/printer.c
    )

    pico_generate_pio_header(printer ${CMAKE_CURRENT_SOURCE_DIR}/pio/redeye.pio)

    if (PICO_CYW43_SUPPORTED)
        target_link_libraries(printer
            pico_cyw43_arch_none
        )
    endif()

    target_link_libraries(printer
        pico_stdlib
        pico_bit_ops
        hardware_pio
        hardware_clocks
        hardware_gpio
    )

    pico_enable_stdio_usb(printer 1)
    pico_add_extra_outputs(printer)

endif() #BUILD_PRINTER_TESTER

