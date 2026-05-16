# Bare-Metal Drivers

A from-scratch bare-metal driver library in C, targeting ARM Cortex-M microcontrollers. Initial target is the Raspberry Pi RP2040, with portability to STM32 planned as a follow-on.

The goal is a layered driver architecture that separates MCU-specific peripheral access from device logic, so the same sensor drivers can run on different host MCUs. No vendor HAL — all peripheral configuration is written directly against the reference manual.

This is a personal learning project. Active development starts June 2026.

---

## Drivers

| Driver | Status | Notes |
|---|---|---|
| GPIO | In progress | Header stubbed, implementation pending |
| UART | Planned | |
| I2C | Planned | |
| SysTick | Planned | |
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
└── GPIO/
    ├── gpio.h
    ├── gpio.c
    └── gpio_test.c
```

Each driver lives in its own folder with a header, implementation, and test file. Test files contain `main()` and produce a standalone flashable binary.

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
make gpio_test
```

### Flash

Using OpenOCD with a CMSIS-DAP debugger:

```bash
openocd -f interface/cmsis-dap.cfg -f target/rp2040.cfg \
  -c "adapter speed 5000" \
  -c "program build/gpio_test.elf verify reset exit"
```

---

## License

MIT. See `LICENSE`.