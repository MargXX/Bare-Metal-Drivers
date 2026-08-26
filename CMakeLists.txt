cmake_minimum_required(VERSION 3.18)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(PLATFORM "rp2040" CACHE STRING "Target platform (rp2040, stm32g4)")

set(BM_VALID_PLATFORMS rp2040 stm32g4)
if(NOT PLATFORM IN_LIST BM_VALID_PLATFORMS)
    message(FATAL_ERROR "Unknown PLATFORM '${PLATFORM}'. Valid options: ${BM_VALID_PLATFORMS}")
endif()

if(PLATFORM STREQUAL "stm32g4")
    set(CMAKE_TOOLCHAIN_FILE ${CMAKE_SOURCE_DIR}/cmake/stm32g4-toolchain.cmake)
else()
    set(PICO_SDK_PATH $ENV{PICO_SDK_PATH})
    set(CMAKE_TOOLCHAIN_FILE ${PICO_SDK_PATH}/cmake/preload/toolchains/pico_arm_cortex_m0plus_gcc.cmake)
endif()

if(PLATFORM STREQUAL "stm32g4")
    include_directories(${CMAKE_SOURCE_DIR}/platform/stm32g4/cmsis/core/Include)
    include_directories(${CMAKE_SOURCE_DIR}/platform/stm32g4/cmsis/device)
endif()

project(RP2040_Bare_Metal_Drivers)

set(CMAKE_C_STANDARD 11)


include_directories(${CMAKE_SOURCE_DIR}/platform/${PLATFORM})
include_directories(${CMAKE_SOURCE_DIR}/platform/${PLATFORM}/gpio)
include_directories(${CMAKE_SOURCE_DIR}/platform/${PLATFORM}/systick)
include_directories(${CMAKE_SOURCE_DIR}/platform/${PLATFORM}/uart)
include_directories(${CMAKE_SOURCE_DIR}/platform/${PLATFORM}/i2c)
include_directories(${CMAKE_SOURCE_DIR}/drivers/bmp390)
include_directories(${CMAKE_SOURCE_DIR}/board/pico-devboard)
include_directories(${CMAKE_SOURCE_DIR}/board/nucleo-g431rb)

include_directories(${CMAKE_SOURCE_DIR}/include)

# Warning flags applied to project source files only (never the SDK) —
# see bm_add_test_executable below for how the scoping works.
set(BM_WARNING_FLAGS -Wall -Wextra -Werror)


if(PLATFORM STREQUAL "rp2040")
    include(pico_sdk_import.cmake)
    pico_sdk_init()
endif()

# Defines an executable target from its own source list, applies strict
# warning flags to only those sources, sets the .elf output name, and
# links pico_runtime. One call per test executable.
function(bm_add_test_executable name)
    if(PLATFORM STREQUAL "stm32g4")
        add_executable(${name} ${ARGN} platform/stm32g4/startup/startup_stm32g431rb.c)
    else()
        add_executable(${name} ${ARGN})
    endif()

    set_source_files_properties(${ARGN}
        PROPERTIES COMPILE_OPTIONS "${BM_WARNING_FLAGS}"
        TARGET_DIRECTORY ${name}
    )
    set_target_properties(${name} PROPERTIES OUTPUT_NAME "${name}.elf")

    if(PLATFORM STREQUAL "rp2040")
        target_link_libraries(${name} pico_runtime)
    endif()
endfunction()

#pico tests
if(PLATFORM STREQUAL "rp2040")
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
        board/pico-devboard/debug_print.c
        board/pico-devboard/debug_blink.c
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
else() #stm32g431rb tests
    bm_add_test_executable(blink_test
        board/nucleo-g431rb/blink_test.c
    )
endif()

# using OpenOCD for flashing instead of picotool, so not generating uf2 files