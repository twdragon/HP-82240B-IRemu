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

include(FetchContent)
set(FETCHCONTENT_QUIET OFF)

# Custom TinyUSB
# =============================================================================
FetchContent_Declare(tinyusb
    GIT_REPOSITORY "https://github.com/hathach/tinyusb.git"
    GIT_TAG "0.21.0"
    GIT_PROGRESS ON
)
FetchContent_MakeAvailable(tinyusb)
set(ENV{PICO_TINYUSB_PATH} "${tinyusb_SOURCE_DIR}")

# Custom BTStack
# =============================================================================
FetchContent_Declare(btstack
    GIT_REPOSITORY "https://github.com/bluekitchen/btstack.git"
    GIT_TAG "v1.8.2"
    GIT_PROGRESS ON
)
FetchContent_MakeAvailable(btstack)
set(ENV{PICO_BTSTACK_PATH} "${btstack_SOURCE_DIR}")

# PNGle
# =============================================================================
FetchContent_Declare(pngle
    GIT_REPOSITORY "https://github.com/kikuchan/pngle.git"
    GIT_TAG "v1.1.0"
    GIT_PROGRESS ON
)
FetchContent_MakeAvailable(pngle)

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
if(PICO_CYW43_SUPPORTED)
    option(USE_WIFI
        "Activates Wi-Fi support"
        OFF
    )
    option(USE_BLUETOOTH
        "Activates Bluetooth support"
        ON
    )
endif()
set(PICO_GPIO_PIN_OFFSET 2)
if(NOT DEFINED PICO_TRANSMITTER_PIN)
    set(PICO_TRANSMITTER_PIN 5)
endif()

# Printer hardware options
# =============================================================================
if(NOT DEFINED PRINTER_SERIAL_NUMBER)
    set(PRINTER_SERIAL_NUMBER "30300882")
endif()
string(LENGTH "${PRINTER_SERIAL_NUMBER}" PRINTER_SERIAL_NUMBER_LEN)
if(NOT PRINTER_SERIAL_NUMBER_LEN EQUAL 8)
    message(FATAL_ERROR 
        "The printer serial number should be exactly 8 symbols long!")
endif()
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
    ${CMAKE_BINARY_DIR}
    ${CMAKE_SOURCE_DIR}/pio
    ${CMAKE_SOURCE_DIR}/include
)

list(APPEND PRINTER_TARGET_FILES
    src/redeye.c
    src/usb_descriptors.c
    src/printer.c
)
if(USE_WIFI)
    list(APPEND PRINTER_TARGET_FILES
        src/wifi.c
    )
endif()
if(USE_BLUETOOTH)
    list(APPEND PRINTER_TARGET_FILES
        src/bluetooth.c
    )
endif()

if(BUILD_PRINTER_TESTER)
    add_executable(printer
    src/redeye.c
    src/printer_tester.c
)
else()
    add_executable(printer
        ${PRINTER_TARGET_FILES}
    )
endif() #BUILD_PRINTER_TESTER

add_library(utf8transcoder STATIC
    src/utf8transcoder.c
)

add_library(ringbuffer STATIC
    src/printbuffer.c
)

add_library(pngle STATIC
    ${pngle_SOURCE_DIR}/src/miniz.c
    ${pngle_SOURCE_DIR}/src/pngle.c
)
target_include_directories(pngle PUBLIC 
    ${pngle_SOURCE_DIR}/src
)

if(SIGNAL_PRINTER_READY)
    target_compile_definitions(printer PRIVATE SIGNAL_READY)
endif() #SIGNAL_PRINTER_READY

pico_generate_pio_header(printer ${CMAKE_SOURCE_DIR}/pio/redeye.pio)

configure_file(include/config.h.in 
               ${CMAKE_BINARY_DIR}/config.h)

if (PICO_CYW43_SUPPORTED)
    if(BUILD_PRINTER_TESTER)
        target_link_libraries(printer
            pico_cyw43_arch_none
        )
    else()
        if(USE_WIFI AND USE_BLUETOOTH)
            message(STATUS "Compiling Wi-Fi and Bluetooth support")
            target_link_libraries(printer
                pico_cyw43_arch_lwip_threadsafe_background
                pico_btstack_classic
                pico_btstack_cyw43
#                pico_btstack_ble 
                pico_lwip_iperf
            )
            target_compile_definitions(printer PRIVATE CONTROLLER_SUPPORTS_WIFI)
            target_compile_definitions(printer PRIVATE CONTROLLER_SUPPORTS_BLUETOOTH)
        elseif(USE_WIFI AND NOT USE_BLUETOOTH)
            message(STATUS "Compiling Wi-Fi support only")
            target_link_libraries(printer
                pico_cyw43_arch_lwip_threadsafe_background
                pico_lwip_iperf
            )
            target_compile_definitions(printer PRIVATE CONTROLLER_SUPPORTS_WIFI)
        elseif(USE_BLUETOOTH AND NOT USE_WIFI)
            message(STATUS "Compiling Bluetooth support only")
            target_link_libraries(printer
                pico_cyw43_arch_none
                pico_btstack_classic
                pico_btstack_cyw43
#                pico_btstack_ble 
            )
            target_compile_definitions(printer PRIVATE CONTROLLER_SUPPORTS_BLUETOOTH)
        else()
            message(STATUS "Wireless options OFF, dropping Bluetooth/Wi-Fi support")
            target_link_libraries(printer
                pico_cyw43_arch_none
            )
        endif()
    endif()
endif()

if(BUILD_PRINTER_TESTER)
    target_link_libraries(printer
        pico_stdlib
        pico_bit_ops
        pico_unique_id
        hardware_pio
        hardware_clocks
        hardware_gpio
        utf8transcoder
        ringbuffer
        pngle
    )
else()
    target_link_libraries(printer
        pico_stdlib
        pico_bit_ops
        pico_unique_id
        pico_multicore
        hardware_pio
        hardware_clocks
        hardware_gpio
        ringbuffer
        utf8transcoder
        tinyusb_device
        tinyusb_board
    )
    target_compile_definitions(printer PUBLIC PICO_RP2040_USB_DEVICE_ENUMERATION_FIX=1)
endif() #BUILD_PRINTER_TESTER

pico_enable_stdio_usb(printer 0)
pico_add_extra_outputs(printer)
