cmake_minimum_required(VERSION 3.18)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


set(PICO_SDK_PATH $ENV{PICO_SDK_PATH})
set(CMAKE_TOOLCHAIN_FILE ${PICO_SDK_PATH}/cmake/preload/toolchains/pico_arm_cortex_m0plus_gcc.cmake)

project(RP2040_Bare_Metal_Drivers)

set(CMAKE_C_STANDARD 11)

set(PLATFORM "rp2040")

set(PLATFORM "rp2040" CACHE STRING "Target platform (rp2040, stm32g431)")

set(BM_VALID_PLATFORMS rp2040 stm32g431)
if(NOT PLATFORM IN_LIST BM_VALID_PLATFORMS)
    message(FATAL_ERROR "Unknown PLATFORM '${PLATFORM}'. Valid options: ${BM_VALID_PLATFORMS}")
endif()

include_directories(${CMAKE_SOURCE_DIR}/platform/${PLATFORM})
include_directories(${CMAKE_SOURCE_DIR}/platform/${PLATFORM}/gpio)
include_directories(${CMAKE_SOURCE_DIR}/SysTick)
include_directories(${CMAKE_SOURCE_DIR}/platform/${PLATFORM}/systick)
include_directories(${CMAKE_SOURCE_DIR}/UART)
include_directories(${CMAKE_SOURCE_DIR}/platform/${PLATFORM}/uart)
include_directories(${CMAKE_SOURCE_DIR}/I2C)
include_directories(${CMAKE_SOURCE_DIR}/platform/${PLATFORM}/i2c)
include_directories(${CMAKE_SOURCE_DIR}/drivers/bmp390)
include_directories(${CMAKE_SOURCE_DIR}/board/pico-devboard)
include_directories(${CMAKE_SOURCE_DIR}/include)

# Warning flags applied to project source files only (never the SDK) —
# see bm_add_test_executable below for how the scoping works.
set(BM_WARNING_FLAGS -Wall -Wextra -Werror)

include(pico_sdk_import.cmake)
pico_sdk_init()

# Defines an executable target from its own source list, applies strict
# warning flags to only those sources, sets the .elf output name, and
# links pico_runtime. One call per test executable.
function(bm_add_test_executable name)
    add_executable(${name} ${ARGN})
    set_source_files_properties(${ARGN}
        PROPERTIES COMPILE_OPTIONS "${BM_WARNING_FLAGS}"
        TARGET_DIRECTORY ${name}
    )
    set_target_properties(${name} PROPERTIES OUTPUT_NAME "${name}.elf")
    target_link_libraries(${name} pico_runtime)
endfunction()

bm_add_test_executable(gpio_test
    platform/cortex-m/systick/systick.c
    platform/${PLATFORM}/gpio/gpio.c
    board/pico-devboard/gpio_test.c
)

bm_add_test_executable(uart_test
    platform/cortex-m/systick/systick.c
    platform/${PLATFORM}/gpio/gpio.c
    platform/${PLATFORM}/uart/uart.c
    board/pico-devboard/uart_test.c
)

bm_add_test_executable(i2c_test
    platform/cortex-m/systick/systick.c
    platform/${PLATFORM}/gpio/gpio.c
    platform/${PLATFORM}/i2c/i2c.c
    platform/${PLATFORM}/uart/uart.c
    board/pico-devboard/i2c_test.c
)

bm_add_test_executable(bmp390_test
    platform/cortex-m/systick/systick.c
    platform/${PLATFORM}/gpio/gpio.c
    platform/${PLATFORM}/i2c/i2c.c
    drivers/bmp390/bmp390.c
    board/pico-devboard/debug_print.c
    board/pico-devboard/debug_blink.c
    platform/${PLATFORM}/uart/uart.c
    board/pico-devboard/bmp390_test.c
)

# using OpenOCD for flashing instead of picotool, so not generating uf2 files