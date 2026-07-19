# Bare-Metal Drivers

A from-scratch bare-metal driver library in C for the Raspberry Pi RP2040. No vendor HAL — all peripheral configuration is written directly against the RP2040 datasheet.

This driver stack is the foundation layer of a larger flight computer project: a custom PCB with a full application layer, all built from scratch. The drivers are written to be clean and self-contained, with hardware-specific register details isolated from the public API.

-----

## Drivers

|Driver          |Status     |Notes                                                                                        |
|----------------|-----------|---------------------------------------------------------------------------------------------|
|GPIO            |Complete   |Verified on hardware. LED blink on GP25.                                                     |
|SysTick         |Complete   |ISR-driven millisecond timing. 125MHz processor clock.                                       |
|UART            |Complete   |Verified on hardware. TX confirmed via logic analyzer and serial monitor.                    |
|I2C             |Complete   |All transaction functions verified on hardware. First sensor read (BMP390 CHIP_ID) confirmed.|
|BMP390 (device) |Complete|`bmp390.c` logic complete (init, configure, soft reset, data-ready, forced/normal reads, float compensation). Build and hardware verification pending.|
|LSM9DS1 (device)|Planned    |Depends on I2C                                                                               |
|SPI             |Optional   |Needed if onboard flash logging is added                                                     |
|W25Q128 (device)|Optional   |Depends on SPI                                                                               |
|STM32 port      |Stretch    |Low priority follow-on                                                                       |

-----

## Project Structure

```
Bare-Metal-Drivers/
├── CMakeLists.txt
├── pico_sdk_import.cmake
├── LICENSE
├── README.md
├── platform/
│   └── rp2040/
│       ├── gpio_platform.h
│       ├── gpio_reg.h
│       ├── systick_reg.h
│       ├── uart_reg.h
│       ├── resets_reg.h
│       ├── i2c_reg.h
│       └── i2c_platform.h
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
└── BMP390/
    ├── bmp390.h
    ├── bmp390_reg.h
    ├── bmp390_platform.h
    ├── bmp390.c            (logic complete, build/verify pending)
    └── bmp390_test.c       (in progress)
```

Each driver lives in its own folder with a portable public API header and implementation file. Hardware-specific register maps and constants live under `platform/rp2040/` and are included only by the driver’s own `.c` file. Test files contain `main()` and produce a standalone flashable binary.

`_platform.h` files exist only where there are public constants that callers need to pass into driver functions. Drivers with no caller-facing platform constants (SysTick, UART) do not have one. The asymmetry is intentional.

`resets_reg.h` is shared across peripheral drivers — it is the single authoritative source for the RP2040 RESETS block and is included directly by each driver’s `.c` file as needed.

Device drivers (BMP390, LSM9DS1) follow a different convention from peripheral drivers: their `_reg.h` lives in the device folder rather than `platform/rp2040/`, because it describes the chip’s internal register map — identical over any host MCU or bus. Only the I2C/SPI layer underneath changes on a port.

-----

## Building

### Prerequisites

Install dependencies:

```bash
sudo apt install -y git cmake gcc-arm-none-eabi libnewlib-arm-none-eabi \
    build-essential libstdc++-arm-none-eabi-newlib python3
```

Clone the Pico SDK as a sibling to this repo:

```bash
cd ~/Desktop/Projects   # or wherever this repo lives
git clone https://github.com/raspberrypi/pico-sdk.git
cd pico-sdk
git submodule update --init --recursive
```

Set the SDK path in your shell environment:

```bash
echo 'export PICO_SDK_PATH=/path/to/pico-sdk' >> ~/.bashrc
source ~/.bashrc
```

### Compile

```bash
mkdir build && cd build
cmake .. -DPICO_BOARD=pico
make
```

To build a single target:

```bash
make uart_test
```

### Flash

Using OpenOCD with a CMSIS-DAP debugger (from project root):

```bash
openocd -f interface/cmsis-dap.cfg -f target/rp2040.cfg \
  -c "adapter speed 5000" \
  -c "program build/uart_test.elf verify reset exit"
```

Or from the build directory:

```bash
openocd -f interface/cmsis-dap.cfg -f target/rp2040.cfg \
  -c "adapter speed 5000" \
  -c "program uart_test.elf verify reset exit"
```

### Monitor Serial Output

```bash
screen /dev/ttyACM0 115200
```
## Sensor Driver Testing (In Progress)

The verification stack is structured across three distinct layers to optimize feedback speed and test coverage:

1. **Host-Side Unit Tests:** Driver logic (such as compensation formulas) is compiled for the host architecture and executed against mocked I2C register files and fake SysTick implementations. Tests use a captured, real-world BMP390 calibration data blob as a fixture to catch pure-logic regressions in under a second with no hardware attached.
2. **Hardware-in-the-Loop (HIL):** A `pytest` harness drives the physical RP2040 board over UART via a lightweight ASCII command protocol. This layer validates the driver against the actual physical sensor hardware to verify real-world timing and bus behavior.
3. **Continuous Integration (CI):** A GitHub Actions workflow cross-compiles every test and application target on every `push` and `pull_request` to catch compilation failures and toolchain regressions automatically.

### Flight Simulation & Replay
Planned for the flight computer stage: a replay harness that feeds 23+ hours of logged
flight data, including a 12 G ascent, back through the stack for fault injection.


-----

## Notes

- `bm_` prefix used on all driver functions to avoid linker collisions with the Pico SDK
- `pico_runtime` used instead of `pico_stdlib` to avoid name conflicts with driver functions
- The Pico SDK remaps `SysTick_Handler` to `isr_systick` — use `isr_systick` as the handler name
- `pico_runtime` sets the system clock to 125MHz on startup — set `SYSTICK_TICKS_PER_MS` to `125000` accordingly
- `pico_runtime` also configures `clk_peri` to 125MHz at startup — no explicit clock enable needed in peripheral init
- Static memory allocation preferred throughout — no `malloc`/`free`
- The `platform/rp2040/` structure keeps register-level details isolated from driver logic — switching targets requires only a new `platform/` directory and a one-line CMake change (`set(PLATFORM "rp2040")`)
- Device driver `_reg.h` files (BMP390, LSM9DS1) live in the device folder, not `platform/rp2040/` — the sensor register map is chip-specific, not host-specific, and does not change on a port

-----

## License

MIT. See `LICENSE`.
