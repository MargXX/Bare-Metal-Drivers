/*SRAM1 currently storing one all used SRAM for now, update as needed, SRAM is contiguous blobk */


ENTRY(Reset_Handler)

/* Section 2.4 of RM0440, STM32G431 should be STM32G4 Series category 2 */
MEMORY 
{
    FLASH (rx)  : ORIGIN = 0x08000000, LENGTH = 128K
    SRAM1   (rwx) : ORIGIN = 0x20000000, LENGTH = 16K
    SRAM2   (rwx) : ORIGIN = 0x20004000, LENGTH = 6K
    CCMRAM   (rwx) : ORIGIN = 0x10000000, LENGTH = 10K
}

SECTIONS
{

    .isr_vector :
    {
        . = ALIGN(4);
        /*KEEP to prevent "uncalled" c code from being discarded*/
        KEEP(*(.isr_vector)) 
        . = ALIGN(4);
    } > FLASH

    .text : {
        . = ALIGN(4);
    *(.text)
    *(.text*)
    . = ALIGN(4);
    } > FLASH

    /* store load address so reset hadnler knows where source bytes begin, LOADADDR()/_si* separate for virtual and load addresses */
    _sidata = LOADADDR(.data);

    /* _s and _e designates start and end so reset handler knows what to copy or zero*/
    .data : 
    {
        . = ALIGN(4);
        _sdata = .;
        *(.data)
        *(.data*)
        . = ALIGN(4);
        _edata = .;
        /* AT> sets virtual address to RAM but loads from flash */
    } > SRAM1 AT> FLASH

    .bss :
    {
    . = ALIGN(4);
    _sbss = .;
    *(.bss)
    *(.bss*)
    /* designates where to put uninitialized global variables */
    *(COMMON)
    . = ALIGN(4);
    _ebss = .;
    } > SRAM1

    /* for permanently stored constants */
    .rodata :
    {
    . = ALIGN(4);
    *(.rodata)
    *(.rodata*)
    . = ALIGN(4);
    } > FLASH

    _estack = ORIGIN(SRAM1) + LENGTH(SRAM1);
}