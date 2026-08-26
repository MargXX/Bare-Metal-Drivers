# Bare-Metal Drivers

![CI](https://github.com/MargXX/Bare-Metal-Drivers/actions/workflows/build.yml/badge.svg)

A from-scratch bare-metal driver library in C for ARM Cortex-M. No vendor HAL is used; all peripheral configuration is written directly against the chip datasheet.

The RP2040 driver stack is complete and hardware-verified. A second target, the STM32G431RB (Cortex-M4F), is in progress: its startup infrastructure is hand-written and boots on hardware, with peripheral drivers still to come.

This driver stack is the foundation layer of a larger flight computer project: a custom PCB with a full application layer. The repository is organized into portable, platform-specific, and board-specific tiers so that a second MCU target can be added without rewriting driver logic or public API contracts.

---

## Drivers

| Driver | Status | Notes |
|---|---|---|
| GPIO | Complete | Verified on hardware. Platform-specific by design; see [Portability](#portability). |
| SysTick | Complete | ISR-driven millisecond timing. Verified portable across ARMv6-M and ARMv7-M; see [Portability](#portability). |
| UART | Complete | Verified on hardware. TX confirmed via logic analyzer and serial monitor. |
| I2C | Complete | All transaction functions verified on hardware. First sensor read (BMP390 CHIP_ID) confirmed. |
| BMP390 (device) | Complete | Verified on hardware: init, configure, soft reset, data-ready, forced and normal reads, float compensation. Ambient readings 100,764 Pa and 22 °C. Full host-side unit test coverage. |
| SPI | Deferred | Moved to the flight computer project, with onboard flash logging. |
| W25Q128 (device) | Deferred | Depends on SPI; moved with it. |

All of the above target the RP2040.

### STM32G431RB port

| Component | Status | Notes |
|---|---|---|
| Linker script | Complete | Memory map written from the datasheet: 128K flash, SRAM1/SRAM2/CCM. |
| Vector table and reset handler | Complete | Hand-written in C. All 102 device IRQ slots stubbed with weak aliases; ordering cross-checked against ST's own startup file. |
| CMake toolchain file | Complete | Cortex-M4F, hard float ABI, separate from the RP2040 build. |
| Peripheral drivers | Not started | — |

Verified end to end: build, flash over SWD, and an LED driven by direct RCC and GPIO register writes on real hardware. Clock tree configuration is not yet written; the chip runs on its HSI16 reset default.

---

## Project Structure

```
Bare-Metal-Drivers/
├── .github/
│   └── workflows/
│       └── build.yml
├── .gitignore
├── CMakeLists.txt              ARM project; PLATFORM selection + validation
├── pico_sdk_import.cmake
├── LICENSE
├── README.md
│
├── cmake/
│   └── stm32g4-toolchain.cmake     cross-compile settings for the STM32 target
│
├── include/                    Tier 1: portable public API
│   ├── systick.h
│   ├── uart.h
│   ├── i2c.h
│   └── bmp390.h
│
├── drivers/                    Tier 1: portable device logic
│   └── bmp390/
│       ├── bmp390.c            byte-identical across MCU targets
│       ├── bmp390_reg.h        chip register map (internal)
│       └── bmp390_chip.h       caller-facing chip facts
│
├── platform/                   Tier 2: vendor-specific implementation
│   ├── cortex-m/
│   │   └── systick/
│   │       ├── systick.c       shared across any Cortex-M
│   │       └── systick_reg.h
│   ├── rp2040/
│   │   ├── resets_reg.h        shared across RP2040 peripherals
│   │   ├── gpio/
│   │   │   ├── gpio.h          no portable contract; see Portability
│   │   │   ├── gpio.c
│   │   │   ├── gpio_reg.h
│   │   │   └── gpio_platform.h
│   │   ├── uart/
│   │   │   ├── uart.c
│   │   │   └── uart_reg.h
│   │   ├── i2c/
│   │   │   ├── i2c.c
│   │   │   ├── i2c_reg.h
│   │   │   └── i2c_platform.h
│   │   └── systick/
│   │       └── systick_platform.h    SYSTICK_TICKS_PER_MS
│   └── stm32g4/
│       ├── cmsis/              vendored, unmodified vendor headers
│       │   ├── core/Include/       ARM CMSIS-Core
│       │   └── device/             ST CMSIS device headers
│       └── startup/
│           ├── stm32g431rb.ld
│           └── startup_stm32g431rb.c
│
├── board/                      Tier 3: physical wiring
│   ├── pico-devboard/
│   │   ├── gpio_test.c
│   │   ├── uart_test.c
│   │   ├── i2c_test.c
│   │   ├── bmp390_test.c
│   │   ├── debug_blink.{h,c}
│   │   └── debug_print.{h,c}
│   └── nucleo-g431rb/
│       └── blink_test.c
│
└── tests/                      Off-target suite; separate CMake project
    ├── CMakeLists.txt
    ├── bmp390_test_host.c
    ├── bmp390_i2c_fake.c
    ├── systick_fake.c
    └── unity/
        ├── unity.h
        ├── unity.c
        └── unity_internals.h
```

### The three tiers

The layout separates code by *what would have to change on a port*, not by peripheral name.

**`include/` and `drivers/` are portable.** Nothing here references a specific MCU. `include/` holds only the public API contracts that keep the same signature on every target. `drivers/bmp390/` holds device logic that reaches hardware exclusively through those contracts, which is what makes it portable and what makes off-target unit testing cheap.

**`platform/` is vendor-specific.** Selected at link time via `-DPLATFORM=<mcu>`. The `cortex-m/` subtree holds code shared by any Cortex-M core regardless of silicon vendor; `rp2040/` and `stm32g4/` hold code specific to those chips. Peripherals get their own subfolders so that a second target does not produce a flat directory of thirty files.

**`board/` is wiring-specific.** Which pin drives the LED, which I2C bus the sensor sits on, what baud the serial monitor expects. These are facts about one physical assembly, not about the MCU family. A second board running the same MCU gets its own sibling directory.

A `startup/` subfolder exists under `stm32g4/` and not under `rp2040/`, because that layer is hand-written on STM32 and supplied by `pico_runtime` on the RP2040. The folder is present where the code is.

Vendored vendor headers sit under `platform/<mcu>/cmsis/`, split by upstream (ARM for the core headers, ST for the device headers) and kept unmodified. They provide register addresses and bitfield names only; every register value written by this project is chosen against the reference manual.

### Header conventions

| File | Audience | Contents |
|---|---|---|
| `include/xxx.h` | Anyone using the driver | Portable public API |
| `platform/<mcu>/xxx/xxx_reg.h` | `xxx.c` only | Register structs, base addresses, masks, peripheral pointer tables |
| `platform/<mcu>/xxx/xxx_platform.h` | Callers | Constants callers pass into driver functions |
| `platform/<mcu>/startup/*` | Nothing else | Boot infrastructure; runs before `main` |
| `drivers/<device>/xxx_reg.h` | `xxx.c` only | Chip register map, host-independent |
| `drivers/<device>/xxx_chip.h` | Callers | Caller-facing chip facts (I2C address, chip ID) |
| `board/<board>/*` | Nothing else | On-target mains and presentation helpers |
| `tests/*` | Host compiler only | Off-target unit tests and fakes; never cross-compiled, never flashed |

`_platform.h` files exist only where there are constants callers must pass in. Drivers with no caller-facing platform constants do not have one.

`resets_reg.h` sits flat in `platform/rp2040/` rather than in a peripheral subfolder, because both UART and I2C write to the RESETS block. It is the single source for that block and is included directly by each `.c` that needs it.

Device `_reg.h` files live in `drivers/<device>/` rather than under `platform/`, because they describe the sensor's internal register map, which is identical regardless of host MCU or bus. Only the transport layer underneath changes on a port.

Implementation files include every header whose symbols they use directly, rather than relying on transitive includes through a public header. This costs nothing at compile time (header guards) and prevents a silent break when a public header's own includes change.

---

## Portability

Findings from evaluating what actually ports between the two targets.

**GPIO has no portable contract.** RP2040 uses flat pin numbering with a function-select field; STM32 uses port + pin + alternate-function. Not unifiable behind one signature, so `gpio.h` lives in `platform/rp2040/gpio/`, not `include/`, unlike `uart.h`, `i2c.h`, and `systick.h`.

**SysTick is portable, verified against ARM DDI 0419E (ARMv6-M) and ARM DDI 0403E (ARMv7-M).** `SYST_CSR.CLKSOURCE` (bit 2) has the same definition on both architectures, but the "external reference clock" it selects is vendor-wired differently: RP2040 ties it to a fixed 1 MHz watchdog tick, STM32 ties it to AHB/8. This driver always sets `CLKSOURCE=1`, so it never touches that divergence. `SYST_CVR.CURRENT` is bits[23:0] on ARMv6-M and bits[31:0] on ARMv7-M, a real difference, but `SYST_RVR.RELOAD` is bits[23:0] on both, so `CURRENT` never holds more than 24 bits in practice either way. On this basis `systick.c`/`systick_reg.h` sit in `platform/cortex-m/systick/`; the one chip-specific value, `SYSTICK_TICKS_PER_MS`, is split into `platform/rp2040/systick/systick_platform.h`.

**Startup infrastructure does not port and is not meant to.** The RP2040 build uses the Pico SDK's second-stage bootloader, linker script, and `pico_runtime`. The STM32 build uses a hand-written linker script, vector table, and reset handler, because no equivalent is being taken from ST. Both are `platform/` concerns and neither is claimed as shared.

**Platform selection is validated, not assumed.** `PLATFORM` is a CMake cache variable checked against a whitelist (`rp2040`, `stm32g4`); an unrecognized value fails the configure with `FATAL_ERROR` rather than silently building against an empty include path.

**Each target gets its own build directory.** `CMAKE_TOOLCHAIN_FILE` must be set before `project()` and is fixed in a build directory's cache at first configure, so one directory cannot serve two toolchains.

---

## Building

### Prerequisites

```bash
sudo apt install -y git cmake gcc-arm-none-eabi libnewlib-arm-none-eabi \
    build-essential libstdc++-arm-none-eabi-newlib python3
```

For the RP2040 target, clone the Pico SDK as a sibling to this repo. Submodules are not initialized: only the second-stage bootloader, the linker script, and `pico_runtime` are used, so TinyUSB and btstack are not required. The clone is pinned to a release tag to match CI.

```bash
cd ~/Desktop/Projects   # or wherever this repo lives
git clone --depth 1 --branch 2.3.0 https://github.com/raspberrypi/pico-sdk.git
```

Set the SDK path in your shell environment:

```bash
echo 'export PICO_SDK_PATH=/path/to/pico-sdk' >> ~/.bashrc
source ~/.bashrc
```

The STM32 target needs no SDK. Its CMSIS headers are vendored in this repository.

### Compile firmware: RP2040

```bash
cmake -B build-rp2040 -DPICO_BOARD=pico -DPLATFORM=rp2040
cmake --build build-rp2040
```

Targets: `gpio_test`, `uart_test`, `i2c_test`, `bmp390_test`. To build a single one:

```bash
cmake --build build-rp2040 --target uart_test
```

Output is one `.elf` per target. UF2 generation is disabled; flashing is done over SWD with OpenOCD.

### Compile firmware: STM32G431RB

```bash
cmake -B build-stm32g4 -DPLATFORM=stm32g4
cmake --build build-stm32g4
```

Target: `blink_test`.

### Build and run host tests

The host suite is a separate CMake project under `tests/` with no toolchain file, so it configures with the system compiler. It does not require the Pico SDK or the ARM toolchain.

```bash
cmake -S tests -B tests/build
cmake --build tests/build
ctest --test-dir tests/build --output-on-failure
```

`-S` and `--test-dir` are required. Without them, CMake and CTest resolve paths against the current working directory and pick up the root ARM project instead.

### Flash

Using OpenOCD with a CMSIS-DAP debugger, from the project root.

RP2040:

```bash
openocd -f interface/cmsis-dap.cfg -f target/rp2040.cfg \
  -c "adapter speed 5000" \
  -c "program build-rp2040/bmp390_test.elf verify reset exit"
```

STM32G431RB:

```bash
openocd -f interface/cmsis-dap.cfg -f target/stm32g4x.cfg \
  -c "adapter speed 5000" \
  -c "program build-stm32g4/blink_test.elf verify reset exit"
```

On a Nucleo-64 board, using an external probe requires JP1 (STLK_RST) fitted, which holds the onboard ST-LINK in reset. SWDIO and SWCLK are on CN7 pins 13 and 15.

### Monitor serial output

```bash
screen /dev/ttyACM0 115200
```

---

## Testing

Verification is structured across three layers.

| Layer | Status | Scope |
|---|---|---|
| 1. Host-side unit tests | Complete | BMP390 driver logic, compiled and run natively. No hardware. |
| 2. Hardware-in-the-loop (HIL) | Deferred | Automated pytest harness moved to the flight computer project. |
| 3. Continuous integration | Complete | Cross-compile and host test suite on every push. |

Peripheral drivers (GPIO, UART, I2C) are verified on hardware through their on-target test binaries in `board/pico-devboard/`, observed manually with a logic analyzer and serial monitor. Layer 1 covers the BMP390 device driver.

Manual hardware verification is not the same thing as Layer 2. Layer 2 refers specifically to an automated harness that drives the board and asserts on its output without a human watching an LED. That remains reassigned.

### SysTick

SysTick has no on-target test binary. Absolute tick accuracy requires an external time reference, measured as a pin toggle on the logic analyzer, rather than firmware reporting on its own timing. The rollover-safe elapsed-time arithmetic, `(now - start) >= timeout`, is exercised by `tests/systick_fake.c` in the host suite, and SysTick is exercised transitively by the delay and timeout paths in the I2C and BMP390 on-target suites.

### Layer 1: host-side unit tests

`tests/bmp390_test_host.c` includes `bmp390.c` directly, placing both in a single translation unit so the file's internal `static` helpers are reachable from the test. It links against [Unity](https://github.com/ThrowTheSwitch/Unity), vendored under `tests/unity/`.

Because the driver source is pulled in by `#include`, it must not also appear in the target's source list. Listing it in both places produces a multiple-definition link error for every public function in the driver.

The driver reaches hardware only through the `bm_i2c_*` and `bm_systick_*` public APIs, which makes those APIs link-time substitution points. The host build supplies its own implementations in place of the real drivers.

Every function in `bmp390.c` is covered: the compensation math, calibration parsing, configuration, data-ready polling, status reads, soft reset, and both read paths. Each is tested for its calculation or side effects, its input guards, and its behaviour when an I2C transaction fails. The suite runs in under a second and is registered with CTest.

**Test doubles.** `tests/bmp390_i2c_fake.c` models the sensor as a 256-byte register array. Reads serve consecutive registers from that array, matching the real burst auto-increment; writes land `{register, value}` pairs back into it. It validates the device address and bounds-checks every access. It intercepts the soft-reset command and restores the power-on defaults using the named constants in `bmp390_reg.h` that the driver checks against, so the fake and the driver share one definition.

Two mechanisms make failure paths reachable. A transaction counter lets a test specify that the Nth I2C transaction should fail, which is how NAK handling is verified at each step of a multi-write sequence. Two independent flags control whether a simulated reset signals completion and whether it restores register defaults, which is how the reset timeout and the reset verification step are tested.

`tests/systick_fake.c` provides a settable millisecond counter with rollover-safe elapsed-time arithmetic. Each simulated I2C transaction advances that clock, so polling loops reach a timeout rather than spinning indefinitely.

**Test design.** Expected values are derived independently rather than by re-running the code under test: compensation results are hand-computed, and calibration coefficients are transcribed separately from the datasheet quantization table. Two calibration fixtures are used. The primary one is synthetic, with byte values chosen so that an index slip, a byte-order swap, or a missing sign extension produces a visibly wrong coefficient. The second is a 21-byte blob captured from the physical sensor. Floating-point comparisons use a relative tolerance, since the coefficients span roughly twenty orders of magnitude.

The fake's model of device behaviour is itself derived from the datasheet. The reset-verification test therefore covers the mechanism, that the driver detects a reset which fails to restore defaults, but not the value, since it passes for any value the fake and the driver agree on. Values encoded in the fake are confirmed against hardware separately.

### Layer 3: continuous integration

`.github/workflows/build.yml` runs two jobs on every push:

- **RP2040 Build**: installs the ARM toolchain, clones the pinned SDK, and cross-compiles every target.
- **Host Unit Tests**: configures the `tests/` project with the native compiler and runs the Layer 1 suite through CTest.

The jobs use different toolchains and do not depend on each other, so they run in parallel and report separately. The STM32 target is not yet built in CI.

### Flight simulation and replay

Planned for the flight computer stage: a replay harness that feeds 25 hours of logged flight data, including a 16+ G ascent, back through the stack for fault injection.

---

## Datasheet Discrepancies

Values observed on hardware that do not match the reference documentation.

### BMP390 `OSR` (0x1C) power-on value

| | Value | Decode |
|---|---|---|
| Datasheet Table 25, Default Value | `0x02` | `osr_p` ×4, `osr_t` ×1 |
| Observed on hardware | `0x00` | `osr_p` ×1, `osr_t` ×1 |

Reference: Bosch BMP390 datasheet, BST-BMP390-DS002-07, Revision 1.7 (03/2021), Table 25 "BMP390 memory map".

The reset restores the register; only the documented value differs. Two observations establish this:

- `bm_bmp390_configure` had written `OSR = 0x03`. After the soft reset the register read `0x00`, so the reset acted on it.
- `PWR_CTRL` went `0x13` to `0x00` across the same reset. Forced mode auto-returns to sleep after one conversion, which clears the mode bits but leaves `press_en` and `temp_en` set, so an un-reset device reads `0x03`. Clearing the enables requires a reset.

Both destination bytes were poisoned before the reads and both `bm_i2c_write_read` return values were checked. `OSR` also reads `0x00` immediately after `bm_bmp390_init`, which soft-resets internally. Both observations follow a soft reset, so this is a replication rather than an independent confirmation, but it rules out an artifact of the register history in the reset test specifically.

The observed values are named constants in `bmp390_reg.h`, annotated with their provenance and shared by the driver, the on-target test, and the host-side fake.

The discrepancy surfaced during hardware re-verification after the reset read-back was added to `bm_bmp390_soft_reset`. Because `bm_bmp390_init` calls that function, the wrong constant failed initialization and every function guarded on `initialized`. The host suite did not detect it: the fake restored the same documented value the driver asserted.

---

## Known Limitations

- `bm_bmp390_configure` writes four registers in sequence and is not atomic. If a transaction fails partway through, the handle correctly reports the device as unconfigured, but the sensor may hold a mix of old and new register values. Callers should re-run configuration after a failure rather than assuming the previous configuration survived.
- `bm_bmp390_soft_reset` polls `STATUS.cmd_rdy`, then reads `PWR_CTRL` and `OSR` back to confirm the register file returned to its power-on state. This check is inside the driver, so a wrong expected value fails initialization and every function guarded on `initialized`. The expected values are hardware-derived named constants in `bmp390_reg.h`, shared with the host-side fake. See [Datasheet Discrepancies](#datasheet-discrepancies).
- The polling loops in `bm_bmp390_soft_reset` and `bm_bmp390_read_forced` do not distinguish a persistent bus fault from a slow sensor. Either resolves as a timeout rather than an immediate failure.
- `bm_bmp390_init` does not populate `dev->cfg`. That field is indeterminate until `bm_bmp390_configure` returns `true`, and `configured` is its validity flag. Every driver path that reads `cfg` checks `configured` first, so no driver code can observe an indeterminate value, but callers must not read it before configuring.
- SysTick's absolute tick accuracy is not asserted by any automated test. See [SysTick](#systick).
- `include/systick.h` includes `systick_platform.h`, which is platform-specific, so the portable header is not currently free of platform dependencies. `SYSTICK_TICKS_PER_MS` is a build-configuration constant rather than something callers pass in, so it does not fit the stated `_platform.h` rule. Open.
- The STM32G431RB target has startup infrastructure and a verified boot path, but no peripheral drivers. Nothing under `include/` or `drivers/` has been exercised on it yet.
- The STM32 build runs on the HSI16 reset clock. No clock tree configuration is written, so `SYSTICK_TICKS_PER_MS` has no STM32 equivalent yet.

---

## Notes

- `bm_` prefix is used on all driver functions to avoid linker collisions with the Pico SDK
- `pico_runtime` is used instead of `pico_stdlib` to avoid name conflicts with driver functions
- The Pico SDK remaps `SysTick_Handler` to `isr_systick`, so `isr_systick` is the handler name
- `pico_runtime` sets the system clock to 125 MHz on startup, so `SYSTICK_TICKS_PER_MS` is set to `125000`
- `pico_runtime` also configures `clk_peri` to 125 MHz at startup, so no explicit clock enable is needed in peripheral init
- On the STM32 target there is no equivalent runtime, so the reset handler copies `.data`, zeroes `.bss`, and calls `main` directly
- STM32 exception and interrupt handlers are declared as weak aliases to a common default handler, so a real implementation replaces one by definition alone, with no edit to the vector table
- Static memory allocation is used throughout; no `malloc`/`free`
- BMP390 compensation uses `float` rather than `double`, matching Bosch's reference implementation. The smallest coefficient scale (`2^-65`) is within float's range, and the terms it feeds are multiplied by large raw values before contributing to the result
- The I2C test double is named for the device rather than the bus. It encodes BMP390 addressing and reset behaviour, so a second device driver gets its own fake rather than sharing a generalized one
- `bm_bmp390_read_forced` rejects a call made while `cfg.mode` is `NORMAL`. Triggering a forced conversion drops the device into sleep afterward, which would leave the handle claiming a mode the sensor is no longer in

---

## License

MIT. See `LICENSE`.