# current version of CMake is 3.28.3, but was advised 3.13 was more standard for embedded development
cmake_minimum_required(VERSION 3.13)

# avoid manually setting toolchain file when running cmake
set(PICO_SDK_PATH $ENV{PICO_SDK_PATH})
set(CMAKE_TOOLCHAIN_FILE ${PICO_SDK_PATH}/cmake/preload/toolchains/pico_arm_cortex_m0plus_gcc.cmake)

# device specific for now, may make more generic later
project(RP2040_Bare_Metal_Drivers)

# 11 and 17 should both work so going with 11 since it's more widely supported
set(CMAKE_C_STANDARD 11)

# include directories for all tests — allows test .c files to include headers from platform/ without needing relative paths
# change this to point to the correct platform if switching targets, or make more generic if supporting multiple platforms in the future
set(PLATFORM "rp2040")
include_directories(${CMAKE_SOURCE_DIR}/platform/${PLATFORM})



include(pico_sdk_import.cmake)
pico_sdk_init() #must be after project call

# cmake allows one main() per executible
# will need separate executibles per test
# executibles are elf/uf2 etc. type files
# remember to add all required .c files to the executible, including dependencies(Ex: i2c.c for bmp390 executable)
add_executable(gpio_test
    GPIO/gpio.c
    SysTick/systick.c
    GPIO/gpio_test.c # main() here
)

# add_executable(systick_test
#     SysTick/systick.c
#     SysTick/systick_test.c # main() here
# )

add_executable(uart_test
    GPIO/gpio.c
    SysTick/systick.c
    UART/uart.c 
    UART/uart_test.c # main() here
)

# ensures an elf is made with the name gpio_test.elf instead of just gpio_test
set_target_properties(gpio_test PROPERTIES OUTPUT_NAME "gpio_test.elf")
# set_target_properties(systick_test PROPERTIES OUTPUT_NAME "systick_test.elf")
set_target_properties(uart_test PROPERTIES OUTPUT_NAME "uart_test.elf")



# set up linker
# linker stitches together .o files(object) and .a files(libraries) to create executable
target_link_libraries(gpio_test pico_runtime)
# target_link_libraries(systick_test pico_runtime)
target_link_libraries(uart_test pico_runtime)




# generate uf2 file for flashing since working w/ pico
# pico_add_extra_outputs(gpio_test)
# using openOCD for flashing instead of picotool, so not generating uf2 files


