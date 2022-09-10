cmake_minimum_required(VERSION 3.13)

# Pull in SDK (must be before project)
include(pico_sdk_import.cmake)

project(han C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
if (PICO_SDK_VERSION_STRING VERSION_LESS "1.3.0")
    message(FATAL_ERROR "Raspberry Pi Pico SDK version 1.3.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()

pico_sdk_init()


if (NOT PICO_CYW43_SUPPORTED)
    message(FATAL_ERROR "Not a Pico PI W board with WiFi")
endif()
if (NOT TARGET pico_cyw43_arch)
    message(FATAL_ERROR "Must be a pico_cyw43_arch target")
endif()

set(WIFI_SSID "${WIFI_SSID}" CACHE INTERNAL "WiFi SSID")
set(WIFI_PASSWORD "${WIFI_PASSWORD}" CACHE INTERNAL "WiFi password")

if ("${WIFI_SSID}" STREQUAL "")
    message(FATAL_ERROR "WiFi SSID not set")
endif()
if ("${WIFI_PASSWORD}" STREQUAL "")
    message(FATAL_ERROR "WiFi Password not set")
endif()

if (NOT TARGET tinyusb_device)
    message(FATAL_ERROR "no TinyUSB submodule")
endif()

add_executable(han)
# pico_generate_pio_header(nnnn ${CMAKE_CURRENT_LIST_DIR}/xxx.pio)
target_sources(han
    PRIVATE
    main.cpp
    ntp.cpp
    crc16.cpp
)

target_compile_definitions(han
    PRIVATE
    WIFI_SSID=\"${WIFI_SSID}\"
    WIFI_PASSWORD=\"${WIFI_PASSWORD}\"
    TEST_TCP_SERVER_IP=\"${TEST_TCP_SERVER_IP}\"
)

target_include_directories(han
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(han
    pico_stdlib
    pico_printf
    pico_cyw43_arch_lwip_poll
#    pico_cyw43_arch_none

)

pico_enable_stdio_usb(han 1)
pico_enable_stdio_uart(han 0)

pico_add_extra_outputs(han)
