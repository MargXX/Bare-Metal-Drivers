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
    include_directories(${CMAKE_SOURCE_DIR}/platform/stm32g4/rcc)
    # vendored ST HAL — SYSTEM so -Werror on project sources does not apply to its headers
    include_directories(SYSTEM ${CMAKE_SOURCE_DIR}/platform/stm32g4/hal/Inc)
    add_compile_definitions(STM32G431xx USE_HAL_DRIVER)
endif()

project(RP2040_Bare_Metal_Drivers)

set(CMAKE_C_STANDARD 11)


include_directories(${CMAKE_SOURCE_DIR}/platform/${PLATFORM})
include_directories(${CMAKE_SOURCE_DIR}/platform/${PLATFORM}/gpio)
include_directories(${CMAKE_SOURCE_DIR}/platform/${PLATFORM}/systick)
include_directories(${CMAKE_SOURCE_DIR}/platform/${PLATFORM}/uart)
include_directories(${CMAKE_SOURCE_DIR}/platform/${PLATFORM}/i2c)
include_directories(${CMAKE_SOURCE_DIR}/drivers/bmp390)
# each board has its own debug_print.h / debug_blink.h, so only one board
# directory can be on the include path
if(PLATFORM STREQUAL "stm32g4")
    include_directories(${CMAKE_SOURCE_DIR}/board/nucleo-g431rb)
else()
    include_directories(${CMAKE_SOURCE_DIR}/board/pico-devboard)
endif()

include_directories(${CMAKE_SOURCE_DIR}/include)

# Warning flags applied to project source files only (never the SDK) —
# see bm_add_test_executable below for how the scoping works.
set(BM_WARNING_FLAGS -Wall -Wextra -Werror)


if(PLATFORM STREQUAL "rp2040")
    include(pico_sdk_import.cmake)
    pico_sdk_init()
endif()

# ST HAL backend for the STM32 peripheral drivers. Built as a static library
# with no project warning flags, since it is vendored and unmodified. Only the
# modules enabled in platform/stm32g4/stm32g4xx_hal_conf.h are listed.
if(PLATFORM STREQUAL "stm32g4")
    set(BM_STM32_HAL_SRC ${CMAKE_SOURCE_DIR}/platform/stm32g4/hal/Src)
    add_library(stm32g4_hal STATIC
        ${BM_STM32_HAL_SRC}/stm32g4xx_hal.c
        ${BM_STM32_HAL_SRC}/stm32g4xx_hal_cortex.c
        ${BM_STM32_HAL_SRC}/stm32g4xx_hal_dma.c
        ${BM_STM32_HAL_SRC}/stm32g4xx_hal_dma_ex.c
        ${BM_STM32_HAL_SRC}/stm32g4xx_hal_flash.c
        ${BM_STM32_HAL_SRC}/stm32g4xx_hal_flash_ex.c
        ${BM_STM32_HAL_SRC}/stm32g4xx_hal_gpio.c
        ${BM_STM32_HAL_SRC}/stm32g4xx_hal_i2c.c
        ${BM_STM32_HAL_SRC}/stm32g4xx_hal_i2c_ex.c
        ${BM_STM32_HAL_SRC}/stm32g4xx_hal_pwr.c
        ${BM_STM32_HAL_SRC}/stm32g4xx_hal_pwr_ex.c
        ${BM_STM32_HAL_SRC}/stm32g4xx_hal_rcc.c
        ${BM_STM32_HAL_SRC}/stm32g4xx_hal_rcc_ex.c
        ${BM_STM32_HAL_SRC}/stm32g4xx_hal_uart.c
        ${BM_STM32_HAL_SRC}/stm32g4xx_hal_uart_ex.c
    )
    # one section per function so --gc-sections can drop the HAL code no
    # driver calls; scoped to the HAL so existing targets build unchanged
    target_compile_options(stm32g4_hal PRIVATE -ffunction-sections -fdata-sections)
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
    bm_add_test_executable(clock_test
        platform/cortex-m/systick/systick.c
        platform/stm32g4/rcc/rcc.c
        board/nucleo-g431rb/clock_test.c
    )

    # HAL-backed peripheral driver tests. Every one needs the clock tree,
    # the SysTick timebase, and the HAL glue that ties the HAL to both.
    set(BM_STM32_HAL_BASE
        platform/cortex-m/systick/systick.c
        platform/stm32g4/rcc/rcc.c
        platform/stm32g4/hal_glue.c
    )

    bm_add_test_executable(gpio_test
        ${BM_STM32_HAL_BASE}
        platform/${PLATFORM}/gpio/gpio.c
        board/nucleo-g431rb/gpio_test.c
    )
    target_link_libraries(gpio_test stm32g4_hal)

    bm_add_test_executable(uart_test
        ${BM_STM32_HAL_BASE}
        platform/${PLATFORM}/gpio/gpio.c
        platform/${PLATFORM}/uart/uart.c
        board/nucleo-g431rb/debug_blink.c
        board/nucleo-g431rb/uart_test.c
    )
    target_link_libraries(uart_test stm32g4_hal)

    bm_add_test_executable(i2c_test
        ${BM_STM32_HAL_BASE}
        platform/${PLATFORM}/gpio/gpio.c
        platform/${PLATFORM}/i2c/i2c.c
        platform/${PLATFORM}/uart/uart.c
        board/nucleo-g431rb/debug_print.c
        board/nucleo-g431rb/debug_blink.c
        board/nucleo-g431rb/i2c_test.c
    )
    target_link_libraries(i2c_test stm32g4_hal)

    bm_add_test_executable(bmp390_test
        ${BM_STM32_HAL_BASE}
        platform/${PLATFORM}/gpio/gpio.c
        platform/${PLATFORM}/i2c/i2c.c
        platform/${PLATFORM}/uart/uart.c
        drivers/bmp390/bmp390.c
        board/nucleo-g431rb/debug_print.c
        board/nucleo-g431rb/debug_blink.c
        board/nucleo-g431rb/bmp390_test.c
    )
    target_link_libraries(bmp390_test stm32g4_hal m)

    bm_add_test_executable(por_test
        ${BM_STM32_HAL_BASE}
        platform/${PLATFORM}/gpio/gpio.c
        platform/${PLATFORM}/i2c/i2c.c
        platform/${PLATFORM}/uart/uart.c
        board/nucleo-g431rb/debug_print.c
        board/nucleo-g431rb/debug_blink.c
        board/nucleo-g431rb/por_test.c
    )
    target_link_libraries(por_test stm32g4_hal)
endif()

# using OpenOCD for flashing instead of picotool, so not generating uf2 files