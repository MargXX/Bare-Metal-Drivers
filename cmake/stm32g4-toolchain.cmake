

#compile flags, turns .c and .S files into .o files, but does not link them into a binary

set(CMAKE_SYSTEM_NAME Generic) #skip os specific assumptions
set(CMAKE_SYSTEM_PROCESSOR arm)

#point at cross compiler, assumes on path
set(CMAKE_C_COMPILER arm-none-eabi-gcc)
set(CMAKE_CXX_COMPILER arm-none-eabi-g++)
set(CMAKE_ASM_COMPILER arm-none-eabi-gcc)

#stop cmake from tryin to link test binary
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

#apply cpu and fpu flags globally
set(CPU_FLAGS "-mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard")
set(CMAKE_C_FLAGS "${CPU_FLAGS}" CACHE STRING "" FORCE)
set(CMAKE_ASM_FLAGS "${CPU_FLAGS}" CACHE STRING "" FORCE)

#linkers turn .o files into a binary, and need to know where to find the startup code and linker script

#cpu/fpu flags at link time
set(CMAKE_EXE_LINKER_FLAGS "${CPU_FLAGS}" CACHE STRING "" FORCE)

#point to linker script, assumes in project root
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -T${CMAKE_SOURCE_DIR}/platform/stm32g4/startup/stm32g431rb.ld" CACHE STRING "" FORCE)

set(LINKER_FLAGS "-nostartfiles -Wl,--gc-sections")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${LINKER_FLAGS}" CACHE STRING "" FORCE)
