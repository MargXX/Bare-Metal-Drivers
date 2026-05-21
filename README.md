# Bare-Metal Drivers

A from-scratch bare-metal driver library in C, targeting ARM Cortex-M microcontrollers. Initial target is the Raspberry Pi RP2040, with portability to STM32 planned as a follow-on.

The goal is a layered driver architecture that separates MCU-specific peripheral access from device logic, so the same sensor drivers can run on different host MCUs. No vendor HAL — all peripheral configuration is written directly against the reference manual.

This is a personal learning project.

---

## Drivers

| Driver | Status | Notes |
|---|---|---|
| GPIO | Complete | Verified on hardware. LED blink on GP25. |
| SysTick | Complete | ISR-driven millisecond timing. 125MHz processor clock. |
| UART | Complete | Verified on hardware. TX confirmed via logic analyzer and serial monitor. |
| I2C | Planned | |
| BMP390 (device) | Planned | Depends on I2C |
| LSM9DS1 (device) | Planned | Depends on I2C, fall target |

---

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
│       └── uart_reg.h
├── GPIO/
│   ├── gpio.h
│   ├── gpio.c
│   └── gpio_test.c
├── SysTick/
│   ├── systick.h
│   ├── systick.c
│   └── systick_test.c
└── UART/
    ├── uart.h
    ├── uart.c
    └── uart_test.c
```

Each driver lives in its own folder with a portable public API header and implementation file. Hardware-specific register maps and constants live under `platform/rp2040/` and are included only by the driver's own `.c` file. Test files contain `main()` and produce a standalone flashable binary.

`_platform.h` files exist only where there are public constants that callers need to pass into driver functions. Drivers with no caller-facing platform constants (SysTick, UART) do not have one. The asymmetry is intentional.

---

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

---

## Notes

- `bm_` prefix used on all driver functions to avoid linker collisions with the Pico SDK
- `pico_runtime` used instead of `pico_stdlib` to avoid name conflicts with driver functions
- The Pico SDK remaps `SysTick_Handler` to `isr_systick` — use `isr_systick` as the handler name
- `pico_runtime` sets the system clock to 125MHz on startup — set `SYSTICK_TICKS_PER_MS` to `125000` accordingly
- `pico_runtime` also configures `clk_peri` to 125MHz at startup — no explicit clock enable needed in UART init
- Static memory allocation preferred throughout — no `malloc`/`free`
- Switching target platforms requires only a one-line CMake change (`set(PLATFORM "rp2040")`) and a new `platform/` directory

---

## License

MIT. See `LICENSE`.