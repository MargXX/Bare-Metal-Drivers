# Bare-Metal Drivers

![CI](https://github.com/MargXX/Bare-Metal-Drivers/actions/workflows/build.yml/badge.svg)

A from-scratch bare-metal driver library in C for the Raspberry Pi RP2040. No vendor HAL is used; all peripheral configuration is written directly against the RP2040 datasheet.

This driver stack is the foundation layer of a larger flight computer project: a custom PCB with a full application layer. Hardware-specific register details are isolated from the public API.

-----

## Drivers

|Driver          |Status     |Notes                                                                                        |
|----------------|-----------|---------------------------------------------------------------------------------------------|
|GPIO            |Complete   |Verified on hardware. LED blink on GP25.                                                     |
|SysTick         |Complete   |ISR-driven millisecond timing. 125MHz processor clock.                                       |
|UART            |Complete   |Verified on hardware. TX confirmed via logic analyzer and serial monitor.                    |
|I2C             |Complete   |All transaction functions verified on hardware. First sensor read (BMP390 CHIP_ID) confirmed.|
|BMP390 (device) |Complete   |Verified on hardware: init, configure, soft reset, data-ready, forced and normal reads, float compensation. Ambient readings 101,150 Pa and 23 °C. Full host-side unit test coverage.|
|LSM9DS1 (device)|Optional    |Depends on I2C                                                                               |
|SPI             |Planned   |Needed if onboard flash logging is added                                                     |
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
    ├── bmp390_i2c_fake.c
    ├── systick_fake.c
    └── unity/
        ├── unity.h
        ├── unity.c
        └── unity_internals.h
```

Each driver lives in its own folder with a portable public API header and an implementation file. Hardware-specific register maps and constants live under `platform/rp2040/` and are included only by the driver's own `.c` file. On-target test files contain `main()` and produce a standalone flashable binary.

`_platform.h` files exist only where there are public constants that callers need to pass into driver functions. Drivers with no caller-facing platform constants do not have one.

`resets_reg.h` is shared across peripheral drivers. It is the single source for the RP2040 RESETS block and is included directly by each driver's `.c` file as needed.

Device drivers (BMP390, LSM9DS1) place their `_reg.h` in the device folder rather than `platform/rp2040/`, because it describes the chip's internal register map, which is the same over any host MCU or bus. Only the I2C/SPI layer underneath changes on a port.

`tests/` holds the off-target test suite. It is a separate CMake project built with the native compiler rather than the ARM cross-compiler. See [Testing](#testing).

-----

## Building

### Prerequisites

Install dependencies:

```bash
sudo apt install -y git cmake gcc-arm-none-eabi libnewlib-arm-none-eabi \
    build-essential libstdc++-arm-none-eabi-newlib python3
```

Clone the Pico SDK as a sibling to this repo. Submodules are not initialized: only the second-stage bootloader, the linker script, and `pico_runtime` are used, so TinyUSB and btstack are not required. The clone is pinned to a release tag to match CI.

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

Output is one `.elf` per target in `build/`. UF2 generation is disabled; flashing is done over SWD with OpenOCD.

### Build and run host tests

The host suite is a separate CMake project under `tests/` with no toolchain file, so it configures with the system compiler. It does not require the Pico SDK or the ARM toolchain.

```bash
cmake -S tests -B tests/build
cmake --build tests/build
ctest --test-dir tests/build --output-on-failure
```

`-S` and `--test-dir` are required. Without them, CMake and CTest resolve paths against the current working directory and pick up the root ARM project instead.

To see per-test output, run the binary directly:

```bash
./tests/build/bmp390_test_host
```

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

Verification is structured across three layers, which differ in feedback speed and in how much of the real system they exercise.

|Layer                         |Status     |Scope                                                                          |
|------------------------------|-----------|-------------------------------------------------------------------------------|
|1. Host-side unit tests       |Complete   |BMP390 driver logic, compiled and run natively. No hardware.                   |
|2. Hardware-in-the-loop (HIL) |Out of scope|Moved to the flight computer project.                                         |
|3. Continuous integration     |Complete   |Cross-compile and host test suite on every push.                               |

Peripheral drivers (GPIO, SysTick, UART, I2C) are verified on hardware through their on-target test binaries rather than by host-side unit tests. Layer 1 covers the BMP390 device driver, which is where the logic worth testing off-target lives.

### Layer 1: host-side unit tests

`tests/bmp390_test_host.c` includes `bmp390.c` directly, placing both in a single translation unit so the file's internal `static` helpers are reachable from the test. It links against [Unity](https://github.com/ThrowTheSwitch/Unity), vendored under `tests/unity/`.

The driver reaches hardware only through the `bm_i2c_*` and `bm_systick_*` public APIs, which makes those APIs link-time substitution points. The host build supplies its own implementations in place of the real drivers.

Every function in `bmp390.c` is covered: the compensation math, calibration parsing, configuration, data-ready polling, status reads, soft reset, and both read paths. Each is tested for its calculation or side effects, its input guards, and its behaviour when an I2C transaction fails. The suite runs in well under a second and is registered with CTest.

**Test doubles.** `tests/bmp390_i2c_fake.c` models the sensor as a 256-byte register array. Reads serve consecutive registers from that array, matching the real burst auto-increment; writes land `{register, value}` pairs back into it. It validates the device address and bounds-checks every access, so a driver bug that talks to the wrong device or walks off the register map fails the test rather than passing silently. It also intercepts the soft-reset command and restores the power-on defaults observed on hardware, which are not in every case the values printed in the datasheet memory map. See [Datasheet Discrepancies](#datasheet-discrepancies).

Two mechanisms make failure paths reachable. A transaction counter lets a test specify that the Nth I2C transaction should fail, which is how NAK handling is verified at each step of a multi-write sequence. Two independent flags control whether a simulated reset signals completion and whether it actually restores register defaults, which is how the reset timeout and the reset verification step are tested.

**Test design.** Expected values are derived independently rather than by re-running the code under test: compensation results are hand-computed, and calibration coefficients are transcribed separately from the datasheet quantization table. Two calibration fixtures are used. The primary one is synthetic, with byte values chosen so that an index slip, a byte-order swap, or a missing sign extension produces an obviously wrong coefficient rather than a plausible one. The second is a 21-byte blob captured from the physical sensor, kept as a differently-shaped input. Floating-point comparisons use a relative tolerance, since the coefficients span roughly twenty orders of magnitude.

Independently derived expected values close one gap but not all of them. Where the fake models device *behaviour* rather than supplying input data, that model is itself derived from the datasheet, so a fake and a driver written from the same wrong page will agree with each other and pass. Device behaviour encoded in the fake is treated as an assumption to be confirmed against hardware, not as ground truth.

### Layer 2: hardware-in-the-loop

A `pytest` harness driving the RP2040 over UART was scoped for this repo and has been moved to the flight computer project, where a full application layer gives it more to exercise.

### Layer 3: continuous integration

`.github/workflows/build.yml` runs two jobs on every push:

- **RP2040 Build**: installs the ARM toolchain, clones the pinned SDK, and cross-compiles every target.
- **Host Unit Tests**: configures the `tests/` project with the native compiler and runs the Layer 1 suite through CTest.

The jobs use different toolchains and do not depend on each other, so they run in parallel and report separately.

### Flight simulation and replay

Planned for the flight computer stage: a replay harness that feeds 23+ hours of logged flight data, including a 12 G ascent, back through the stack for fault injection.

-----

## Datasheet Discrepancies

Values observed on hardware that do not match the reference documentation. Recorded here because a driver written strictly to the datasheet will disagree with the part.

### BMP390 `OSR` (0x1C) power-on value

|                                    | Value  | Decode                                     |
|------------------------------------|--------|--------------------------------------------|
| Datasheet Table 25, Default Value  | `0x02` | `osr_p` ×4, `osr_t` ×1                     |
| Observed on hardware               | `0x00` | `osr_p` ×1, `osr_t` ×1 (ultra low power)   |

Reference: Bosch BMP390 datasheet, BST-BMP390-DS002-07, Revision 1.7 (03/2021), Table 25 "BMP390 memory map".

The reset itself is functioning correctly; only the documented value is wrong. Two independent observations establish this:

- `bm_bmp390_configure` had written `OSR = 0x03`. After the soft reset the register read `0x00`, so the reset did act on it.
- `PWR_CTRL` went `0x13` → `0x00` across the same reset. Forced mode auto-returns to sleep after one conversion, which clears the mode bits but leaves `press_en` and `temp_en` set, so an un-reset device reads `0x03`. Clearing the enables requires a reset.
- `OSR` also reads `0x00` immediately after `bm_bmp390_init`, which soft-resets internally, so the value is consistent at two separate points in the run and on two separate handles. Both observations follow a soft reset, so this is a replication rather than an independent confirmation, but it rules out the value being an artifact of the register history in the reset test specifically.

Both reads were confirmed live rather than assumed: the destination bytes were poisoned before the reads, and both `bm_i2c_write_read` return values were checked.

`0x00` is also the more plausible power-on state on its own terms, being the lowest-power oversampling setting rather than ×4 pressure oversampling.

This was found by re-verifying the driver on hardware after a reset read-back was added. Because the read-back sat inside `bm_bmp390_soft_reset`, which `bm_bmp390_init` calls, a single wrong constant failed initialization and cascaded through every function guarded on `initialized`. The host-side suite did not catch it: the fake restored the same documented defaults the driver asserted, so both agreed and both were wrong.

-----

## Known Limitations

- `bm_bmp390_configure` writes four registers in sequence and is not atomic. If a transaction fails partway through, the handle correctly reports the device as unconfigured, but the sensor may hold a mix of old and new register values. Callers should re-run configuration after a failure rather than assuming the previous configuration survived.
- `bm_bmp390_soft_reset` confirms the device has returned by polling `STATUS.cmd_rdy`. Verification that the register file came back to its power-on state lives in the on-target test rather than in the driver, so a wrong expected value fails one test instead of cascading through every function guarded on `initialized`.
- The polling loops in `bm_bmp390_soft_reset` and `bm_bmp390_read_forced` do not distinguish a persistent bus fault from a slow sensor. Either resolves as a timeout rather than an immediate failure.
- `bm_bmp390_init` does not populate `dev->cfg`. That field is indeterminate until `bm_bmp390_configure` returns `true`, and `configured` is its validity flag. Every driver path that reads `cfg` checks `configured` first, so no driver code can observe an indeterminate value, but callers must not read it before configuring.

-----

## Notes

- `bm_` prefix is used on all driver functions to avoid linker collisions with the Pico SDK
- `pico_runtime` is used instead of `pico_stdlib` to avoid name conflicts with driver functions
- The Pico SDK remaps `SysTick_Handler` to `isr_systick`, so `isr_systick` is the handler name
- `pico_runtime` sets the system clock to 125MHz on startup, so `SYSTICK_TICKS_PER_MS` is set to `125000`
- `pico_runtime` also configures `clk_peri` to 125MHz at startup, so no explicit clock enable is needed in peripheral init
- Static memory allocation is used throughout; no `malloc`/`free`
- The `platform/rp2040/` structure keeps register-level details out of driver logic, so switching targets requires a new `platform/` directory and a one-line CMake change (`set(PLATFORM "rp2040")`)
- Device driver `_reg.h` files (BMP390, LSM9DS1) live in the device folder rather than `platform/rp2040/`, because the sensor register map is chip-specific rather than host-specific and does not change on a port
- BMP390 compensation uses `float` rather than `double`, matching Bosch's reference implementation. The smallest coefficient scale (`2^-65`) is within float's range, and the terms it feeds are multiplied by large raw values before contributing to the result
- The I2C test double is named for the device rather than the bus. It encodes BMP390 addressing and reset behaviour, so a second device driver gets its own fake rather than sharing a generalized one
- `bm_bmp390_read_forced` rejects a call made while `cfg.mode` is `NORMAL`. Triggering a forced conversion drops the device into sleep afterward, which would leave the handle claiming a mode the sensor is no longer in

-----

## License

MIT. See `LICENSE`.