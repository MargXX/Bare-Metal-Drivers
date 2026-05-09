# Bare-Metal Drivers

A from-scratch bare-metal driver library in C, targeting ARM Cortex-M microcontrollers. Initial target is the Raspberry Pi RP2040, with portability to STM32 planned as a follow-on.

The goal is a layered driver architecture that separates MCU-specific peripheral access from device logic, so the same sensor drivers can run on different host MCUs. No vendor HAL; all peripheral configuration is written directly against the reference manual.

This is a personal learning project. Active development starts May 2026.

## License

MIT. See `LICENSE`.
