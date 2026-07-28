cmake_minimum_required(VERSION 3.18)

project(Host_Bare_Metal_Drivers)

set(CMAKE_C_STANDARD 11)

# Warning flags applied to project source files only (never the SDK) —
set(BM_WARNING_FLAGS -Wall -Wextra -Werror)



add_executable(bmp390_test_host
    bmp390_test_host.c
    i2c_fake.c
    systick_fake.c
    unity/unity.c
)

target_compile_options(bmp390_test_host PRIVATE ${BM_WARNING_FLAGS})

target_include_directories(bmp390_test_host
    PRIVATE ../platform/rp2040
)

target_link_libraries(bmp390_test_host
    PRIVATE m
)

add_test(NAME tier1_bmp390 COMMAND bmp390_test_host)

enable_testing()
