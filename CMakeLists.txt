cmake_minimum_required(VERSION 3.18)

set(PICO_SDK_PATH $ENV{PICO_SDK_PATH})
set(CMAKE_TOOLCHAIN_FILE ${PICO_SDK_PATH}/cmake/preload/toolchains/pico_arm_cortex_m0plus_gcc.cmake)

project(RP2040_Bare_Metal_Drivers)

set(CMAKE_C_STANDARD 11)

set(PLATFORM "rp2040")
include_directories(${CMAKE_SOURCE_DIR}/platform/${PLATFORM})
include_directories(${CMAKE_SOURCE_DIR}/GPIO)
include_directories(${CMAKE_SOURCE_DIR}/SysTick)
include_directories(${CMAKE_SOURCE_DIR}/UART)
include_directories(${CMAKE_SOURCE_DIR}/I2C)
include_directories(${CMAKE_SOURCE_DIR}/BMP390)
include_directories(${CMAKE_SOURCE_DIR}/Debug)

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
    GPIO/gpio.c
    SysTick/systick.c
    GPIO/gpio_test.c
)

bm_add_test_executable(uart_test
    GPIO/gpio.c
    SysTick/systick.c
    UART/uart.c
    UART/uart_test.c
)

bm_add_test_executable(i2c_test
    GPIO/gpio.c
    SysTick/systick.c
    I2C/i2c.c
    UART/uart.c
    I2C/i2c_test.c
)

bm_add_test_executable(bmp390_test
    GPIO/gpio.c
    SysTick/systick.c
    I2C/i2c.c
    BMP390/bmp390.c
    Debug/debug_print.c
    Debug/debug_blink.c
    UART/uart.c
    BMP390/bmp390_test.c
)

# using OpenOCD for flashing instead of picotool, so not generating uf2 files