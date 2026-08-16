cmake_minimum_required(VERSION 3.18)

project(Host_Bare_Metal_Drivers)

set(CMAKE_C_STANDARD 11)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Warning flags applied to project source files only (never the SDK) —
set(BM_WARNING_FLAGS -Wall -Wextra -Werror)

include_directories(${CMAKE_SOURCE_DIR}/../GPIO)
include_directories(${CMAKE_SOURCE_DIR}/../SysTick)
include_directories(${CMAKE_SOURCE_DIR}/../UART)
include_directories(${CMAKE_SOURCE_DIR}/../I2C)
include_directories(${CMAKE_SOURCE_DIR}/../BMP390)
include_directories(${CMAKE_SOURCE_DIR}/../Debug)



add_executable(bmp390_test_host
    bmp390_test_host.c
    bmp390_i2c_fake.c
    systick_fake.c
    unity/unity.c
)

target_compile_options(bmp390_test_host PRIVATE ${BM_WARNING_FLAGS})

target_include_directories(bmp390_test_host
    PRIVATE ../platform/rp2040
)

target_include_directories(bmp390_test_host
    PRIVATE ../platform/rp2040/i2c
)

target_include_directories(bmp390_test_host
    PRIVATE ../I2C
)

target_link_libraries(bmp390_test_host
    PRIVATE m
)

enable_testing()

add_test(NAME bmp390 COMMAND bmp390_test_host)


