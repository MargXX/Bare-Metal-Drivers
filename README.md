# Bare-Metal Drivers

![CI](https://github.com/MargXX/Bare-Metal-Drivers/actions/workflows/build.yml/badge.svg)

A from-scratch bare-metal driver library in C for the Raspberry Pi RP2040. No vendor HAL, all peripheral configuration is written directly against the RP2040 datasheet.

This driver stack is the foundation layer of a larger flight computer project: a custom PCB with a full application layer, all built from scratch. The drivers are written to be clean and self-contained, with hardware-specific register details isolated from the public API.

-----

## Drivers

|Driver          |Status     |Notes                                                                                        |
|----------------|-----------|---------------------------------------------------------------------------------------------|
|GPIO            |Complete   |Verified on hardware. LED blink on GP25.                                                     |
|SysTick         |Complete   |ISR-driven millisecond timing. 125MHz processor clock.                                       |
|UART            |Complete   |Verified on hardware. TX confirmed via logic analyzer and serial monitor.                    |
|I2C             |Complete   |All transaction functions verified on hardware. First sensor read (BMP390 CHIP_ID) confirmed.|
|BMP390 (device) |Complete   |Verified on hardware: init, configure, soft reset, data-ready, forced and normal reads, float compensation. Ambient readings physically plausible (~101,150 Pa, 23 °C).|
|LSM9DS1 (device)|Planned    |Depends on I2C                                                                               |
|SPI             |Optional   |Needed if onboard flash logging is added                                                     |
|W25Q128 (device)|Optional   |Depends on SPI                                                                               |
|STM32 port      |Stretch    |Low priority follow-on                                                                       |

-----

## Project Structure

```
Bare-Metal-Drivers/
├── .github/
│   └── workflows/
│       └── build.yml
├── .gitignore
├── CMakeLists.txt
├── pico_sdk_import.cmake
├── LICENSE
├── README.md
├── Debug/
│   ├── debug_blink.h
│   ├── debug_blink.c
│   ├── debug_print.h
│   └── debug_print.c
├── platform/
│   └── rp2040/
│       ├── gpio_platform.h
│       ├── gpio_reg.h
│       ├── systick_reg.h
│       ├── uart_platform.h
│       ├── uart_reg.h
│       ├── resets_reg.h
│       ├── i2c_platform.h
│       └── i2c_reg.h
├── GPIO/
│   ├── gpio.h
│   ├── gpio.c
│   └── gpio_test.c
├── SysTick/
│   ├── systick.h
│   ├── systick.c
│   └── systick_test.c
├── UART/
│   ├── uart.h
│   ├── uart.c
│   └── uart_test.c
├── I2C/
│   ├── i2c.h
│   ├── i2c.c
│   └── i2c_test.c
├── BMP390/
│   ├── bmp390.h
│   ├── bmp390_reg.h
│   ├── bmp390_platform.h
│   ├── bmp390.c
│   └── bmp390_test.c
└── tests/
    ├── CMakeLists.txt
    ├── bmp390_test_host.c
    ├── stubs.c
    └── unity/
        ├── unity.h
        ├── unity.c
        └── unity_internals.h
```

Each driver lives in its own folder with a portable public API header and implementation file. Hardware-specific register maps and constants live under `platform/rp2040/` and are included only by the driver's own `.c` file. On-target test files contain `main()` and produce a standalone flashable binary.

`_platform.h` files exist only where there are public constants that callers need to pass into driver functions. Drivers with no caller-facing platform constants do not have one. The asymmetry is intentional.

`resets_reg.h` is shared across peripheral drivers. It is the single authoritative source for the RP2040 RESETS block and is included directly by each driver's `.c` file as needed.

Device drivers (BMP390, LSM9DS1) follow a different convention from peripheral drivers: their `_reg.h` lives in the device folder rather than `platform/rp2040/`, because it describes the chip's internal register map, identical over any host MCU or bus. Only the I2C/SPI layer underneath changes on a port.

`tests/` holds the host-side (off-target) test suite, which is a separate CMake project built with the native compiler rather than the ARM cross-compiler. See [Testing](#testing).

-----

## Building

### Prerequisites

Install dependencies:

```bash
sudo apt install -y git cmake gcc-arm-none-eabi libnewlib-arm-none-eabi \
    build-essential libstdc++-arm-none-eabi-newlib python3
```

Clone the Pico SDK as a sibling to this repo. Submodules are not initialized: only the second-stage bootloader, the linker script, and `pico_runtime` are used, so TinyUSB and btstack are not needed. The clone is pinned to a release tag to match CI.

```bash
cd ~/Desktop/Projects   # or wherever this repo lives
git clone --depth 1 --branch 2.3.0 https://github.com/raspberrypi/pico-sdk.git
```

Set the SDK path in your shell environment:

```bash
echo 'export PICO_SDK_PATH=/path/to/pico-sdk' >> ~/.bashrc
source ~/.bashrc
```

### Compile firmware (ARM)

```bash
cmake -B build -DPICO_BOARD=pico
cmake --build build
```

Targets: `gpio_test`, `uart_test`, `i2c_test`, `bmp390_test`. To build a single one:

```bash
cmake --build build --target uart_test
```

Output is a `.elf` per target in `build/`. UF2 generation is disabled; flashing is done over SWD with OpenOCD.

### Build and run host tests

The host suite is a separate CMake project under `tests/` with no toolchain file, so it configures with the system compiler. It does not require the Pico SDK or the ARM toolchain.

```bash
cmake -S tests -B tests/build
cmake --build tests/build
ctest --test-dir tests/build --output-on-failure
```

The explicit `-S` and `--test-dir` flags matter: without them, CMake and CTest resolve paths against the current working directory and will pick up the root (ARM) project instead.

### Flash

Using OpenOCD with a CMSIS-DAP debugger, from the project root:

```bash
openocd -f interface/cmsis-dap.cfg -f target/rp2040.cfg \
  -c "adapter speed 5000" \
  -c "program build/uart_test.elf verify reset exit"
```

### Monitor serial output

```bash
screen /dev/ttyACM0 115200
```

-----

## Testing

Verification is structured across three layers, chosen to trade off feedback speed against fidelity to real hardware.

|Layer                         |Status     |What it covers                                                                 |
|------------------------------|-----------|-------------------------------------------------------------------------------|
|1. Host-side unit tests       |Partial    |Pure driver logic, compiled and run natively. No hardware, no board.           |
|2. Hardware-in-the-loop (HIL) |Planned    |The driver against the real sensor over a real bus.                            |
|3. Continuous integration     |Complete   |Cross-compile and host test suite on every push.                               |

### Layer 1: host-side unit tests

The BMP390 compensation math is separated from bus I/O inside the driver, so it can be exercised without any I2C at all. `tests/bmp390_test_host.c` includes `bmp390.c` directly into a single translation unit (giving the test access to the file's internal `static` helpers) and links against [Unity](https://github.com/ThrowTheSwitch/Unity), vendored under `tests/unity/`. Symbols that the rest of the driver references but these tests never call (`bm_i2c_*`, `bm_systick_*`) are satisfied by explicit stubs in `tests/stubs.c`.

**Implemented.** Coverage of `power_of_two`, `compensate_temperature`, and `compensate_pressure`, using hand-computed expected values. Each formula group is tested in isolation (all other coefficients zeroed) so a failure localizes to a single term, then again in combination to verify the groups sum correctly. Negative coefficients and cross-contamination between the temperature and pressure coefficient sets are covered. The suite is registered with CTest and runs in well under a second.

**Planned.** A fake I2C backend returning canned register contents, which unlocks tests for calibration parsing and the read-and-compensate path. The fixture will be the 21-byte calibration blob captured from the physical BMP390, used as a golden oracle.

### Layer 2: hardware-in-the-loop

Planned. A `pytest` harness driving the physical RP2040 over UART via a lightweight ASCII command protocol, asserting on register contents, configuration round-trips, and physically plausible sensor output. This is where beliefs about the hardware get checked against the hardware.

### Layer 3: continuous integration

`.github/workflows/build.yml` runs two independent jobs on every push:

- **RP2040 Build**: installs the ARM toolchain, clones the pinned SDK, and cross-compiles every target.
- **Host Unit Tests**: configures the `tests/` project with the native compiler and runs the Layer 1 suite through CTest.

The two jobs use different toolchains and have no dependency on each other, so they run in parallel and a logic regression is distinguishable from a compile break at a glance.

### Flight simulation and replay

Planned for the flight computer stage: a replay harness that feeds 23+ hours of logged flight data, including a 12 G ascent, back through the stack for fault injection.

-----

## Notes

- `bm_` prefix used on all driver functions to avoid linker collisions with the Pico SDK
- `pico_runtime` used instead of `pico_stdlib` to avoid name conflicts with driver functions
- The Pico SDK remaps `SysTick_Handler` to `isr_systick`, so use `isr_systick` as the handler name
- `pico_runtime` sets the system clock to 125MHz on startup, so `SYSTICK_TICKS_PER_MS` is set to `125000` accordingly
- `pico_runtime` also configures `clk_peri` to 125MHz at startup, so no explicit clock enable is needed in peripheral init
- Static memory allocation preferred throughout, no `malloc`/`free`
- The `platform/rp2040/` structure keeps register-level details isolated from driver logic, so switching targets requires only a new `platform/` directory and a one-line CMake change (`set(PLATFORM "rp2040")`)
- Device driver `_reg.h` files (BMP390, LSM9DS1) live in the device folder, not `platform/rp2040/`, because the sensor register map is chip-specific rather than host-specific and does not change on a port
- BMP390 compensation uses `float` rather than `double`, matching Bosch's reference implementation. The smallest coefficient scale (`2^-65`) is well within float's range, and the terms it feeds are multiplied by large raw values before contributing to the result

-----

## License

MIT. See `LICENSE`.