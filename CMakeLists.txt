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

# Application options
# =============================================================================
option(BUILD_PRINTER_TESTER
    "Builds printer tester application instead of the main driver"
    OFF
)
option(SIGNAL_PRINTER_READY
    "Prints the signal 'Printer ready' after booting up the printer"
    ON
)
set(PICO_GPIO_PIN_OFFSET 2)
if(NOT DEFINED PICO_TRANSMITTER_PIN)
    set(PICO_TRANSMITTER_PIN 5)
endif()

# Printer hardware options
# =============================================================================
if(NOT DEFINED LINEFEED_DURATION)
    set(LINEFEED_DURATION 1200)
endif()
if(NOT DEFINED CHAR_LINE_WIDTH)
    set(CHAR_LINE_WIDTH 24)
endif()
if(NOT DEFINED WCHAR_LINE_WIDTH)
    set(WCHAR_LINE_WIDTH 12)
endif()
if(NOT ${WCHAR_LINE_WIDTH} LESS ${CHAR_LINE_WIDTH})
    message(FATAL_ERROR 
        "Please set WCHAR_LINE_WIDTH not more than CHAR_LINE_WIDTH!")
endif()
if(NOT DEFINED DOT_CHAR_WIDTH)
    set(DOT_CHAR_WIDTH 6)
endif()
if(NOT DEFINED DOT_CHAR_HEIGHT)
    set(DOT_CHAR_HEIGHT 8)
endif()
if(NOT DEFINED DOT_LINE_WIDTH)
    set(DOT_LINE_WIDTH 166)
endif()
# =============================================================================
# Printer controller options
# =============================================================================
if(NOT DEFINED PRINT_BUFFER_SIZE_FACTOR)
    set(PRINT_BUFFER_SIZE_FACTOR 426) # 426 lines 24 characters each, ~10 KiB
endif()
if(NOT DEFINED PRINT_INPUT_BUFFER_SIZE)
    set(PRINT_INPUT_BUFFER_SIZE 8192) # 8 KiB
endif()
# =============================================================================

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
    ${CMAKE_SOURCE_DIR}/pio
    ${CMAKE_SOURCE_DIR}/include
)

add_executable(printer
    src/redeye.c
    src/printer.c
)

add_library(utf8transcoder STATIC
    src/utf8transcoder.c
)

add_library(ringbuffer STATIC
    src/printbuffer.c
)

if(BUILD_PRINTER_TESTER)
    target_compile_definitions(printer PRIVATE BUILD_TESTER_APP)
endif() #BUILD_PRINTER_TESTER
if(SIGNAL_PRINTER_READY)
    target_compile_definitions(printer PRIVATE SIGNAL_READY)
endif() #SIGNAL_PRINTER_READY

pico_generate_pio_header(printer ${CMAKE_SOURCE_DIR}/pio/redeye.pio)

configure_file(include/config.h.in 
               ${CMAKE_BINARY_DIR}/config.h)

if (PICO_CYW43_SUPPORTED)
    target_link_libraries(printer
        pico_cyw43_arch_none
    )
endif()

target_link_libraries(printer
    pico_stdlib
    pico_bit_ops
    pico_unique_id
    hardware_pio
    hardware_clocks
    hardware_gpio
    utf8transcoder
    ringbuffer
)

target_compile_definitions(printer PUBLIC PICO_RP2040_USB_DEVICE_ENUMERATION_FIX=1)

pico_enable_stdio_usb(printer 1)
pico_add_extra_outputs(printer)

