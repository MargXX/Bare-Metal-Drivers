cmake_minimum_required(VERSION 3.18)

project(Host_Bare_Metal_Drivers)

set(CMAKE_C_STANDARD 11)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Warning flags applied to project source files only (never the SDK) —
set(BM_WARNING_FLAGS -Wall -Wextra -Werror)

# Header search paths, in dependency order. The host suite is a third
# implementation of the portable contract, so it sees include/ and drivers/
# exactly as an MCU target does — plus the RP2040 platform headers, which the
# portable headers reach for init, get_status, and the speed constants.

# Tier 1: portable contracts and portable device logic.
include_directories(${CMAKE_SOURCE_DIR}/../include)
include_directories(${CMAKE_SOURCE_DIR}/../drivers/bmp390)

# Tier 2: RP2040 platform headers.
include_directories(${CMAKE_SOURCE_DIR}/../platform/rp2040/i2c)
include_directories(${CMAKE_SOURCE_DIR}/../platform/rp2040/uart)
include_directories(${CMAKE_SOURCE_DIR}/../platform/rp2040/systick)


add_executable(bmp390_test_host
    bmp390_test_host.c
    bmp390_i2c_fake.c
    systick_fake.c
    unity/unity.c
)

target_compile_options(bmp390_test_host PRIVATE ${BM_WARNING_FLAGS})

target_link_libraries(bmp390_test_host
    PRIVATE m
)

enable_testing()

add_test(NAME bmp390 COMMAND bmp390_test_host)