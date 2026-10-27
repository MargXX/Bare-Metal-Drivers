#include <string.h>
#include "i2c.h"
#include "i2c_reg.h"
#include "i2c_platform.h"
#include "gpio.h"
#include "systick.h"
#include "hal_glue.h"
#include "stm32g4xx_hal.h"


// functions returns false on error, true on success unless said otherwise in function comment

// one HAL handle per entry in i2c_instances; State stays RESET until init
static I2C_HandleTypeDef i2c_handles[NUM_I2C_PERIPHERALS];
// HAL error code from the most recent transaction, kept across the reset a
// failure triggers so bm_i2c_get_status can still report why it failed
static uint32_t i2c_last_error[NUM_I2C_PERIPHERALS];

static bool i2c_wait_bus_free(uint8_t i2c_num);
static bool i2c_finish(uint8_t i2c_num, HAL_StatusTypeDef status);

// gate the peripheral clock — single consumer, so kept inline with the driver
static void i2c_clock_enable(uint8_t peripheral) {
    switch (peripheral) {
        case 0: __HAL_RCC_I2C1_CLK_ENABLE(); break;
        case 1: __HAL_RCC_I2C2_CLK_ENABLE(); break;
        case 2: __HAL_RCC_I2C3_CLK_ENABLE(); break;
        default: break;
    }
}

// initialize I2C peripheral in master mode — must be called before any other I2C functions
// configures GPIO pins, timing, and enables peripheral
bool bm_i2c_init(uint8_t *i2c_num, uint32_t clock_hz, uint8_t sda_pin, uint8_t scl_pin) {
    if (i2c_num == NULL) { return false; }
    // every HAL timeout below reads the SysTick millisecond count
    if (!bm_hal_timebase_ready()) { return false; }
    // TIMINGR values assume the 144 MHz PCLK1 bm_rcc_clock_init sets up
    if (!bm_hal_clock_ready()) { return false; }

    // validate pin pair and determine peripheral
    const i2c_pin_pair *pair = NULL;
    for (size_t i = 0; i < NUM_I2C_PAIRS; i++) {
        if (i2c_valid_pairs[i].sda == sda_pin && i2c_valid_pairs[i].scl == scl_pin) {
            pair = &i2c_valid_pairs[i];
            break;
        }
    }
    if (pair == NULL) { return false; } //invalid pin pair

    // verify clock speed and select timing
    uint32_t timing;
    if (clock_hz == 0) {
        return false; //invalid clock speed
    } else if (clock_hz <= I2C_MODE_STANDARD) {
        timing = I2C_TIMINGR_STANDARD;
    } else if (clock_hz <= I2C_MODE_FAST) {
        timing = I2C_TIMINGR_FAST;
    } else {
        return false; //invalid clock speed
    }

    // peripheral clock source is left at its reset value (RCC_CCIPR = PCLK1)
    i2c_clock_enable(pair->peripheral);

    // configure GPIO pins: open drain is required by the bus; the internal
    // pull-up is weak (~40k) and only backs up the board's external pull-ups
    if (!bm_gpio_set_function(sda_pin, pair->af, BM_GPIO_OPEN_DRAIN, BM_GPIO_PULL_UP)) { return false; }
    if (!bm_gpio_set_function(scl_pin, pair->af, BM_GPIO_OPEN_DRAIN, BM_GPIO_PULL_UP)) { return false; }

    I2C_HandleTypeDef *h = &i2c_handles[pair->peripheral];
    memset(h, 0, sizeof(*h));
    h->Instance = i2c_instances[pair->peripheral];
    h->Init.Timing = timing;
    h->Init.OwnAddress1 = 0;
    h->Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    h->Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    h->Init.OwnAddress2 = 0;
    h->Init.OwnAddress2Masks = I2C_OA2_NOMASK;
    h->Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    h->Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;

    // HAL_I2C_Init rewrites CR1 with ANFOFF = 0 and DNF = 0, so the analog
    // filter the TIMINGR values assume is on without a separate call
    if (HAL_I2C_Init(h) != HAL_OK) {
        memset(h, 0, sizeof(*h));
        return false;
    }
    i2c_last_error[pair->peripheral] = HAL_I2C_ERROR_NONE;

    *i2c_num = pair->peripheral; //output the peripheral number for use in other functions
    return true; // success
}

// transmit len bytes from buf to device at 7-bit addr
// generates START, sends address + write bit, sends data, generates STOP
bool bm_i2c_write(uint8_t i2c_num, uint8_t addr, const uint8_t *buf, size_t len) {
    if (len == 0) { return true; } //nothing to write, likely error on users part, the function itself succeeded since there are no bytes to write, so return true
    if (buf == NULL) { return false; }
    if (!bm_i2c_is_valid(i2c_num)) { return false; }
    if (addr > 0x7F) { return false; } //not a 7-bit address
    if (len > UINT16_MAX) { return false; } //HAL transfer size is 16-bit

    //wait for bus to be free before starting transaction to avoid collisions with other transactions
    if (!i2c_wait_bus_free(i2c_num)) { return false; }

    // HAL takes a non-const pointer for transmit but only reads from it
    HAL_StatusTypeDef status = HAL_I2C_Master_Transmit(
        &i2c_handles[i2c_num], (uint16_t)(addr << 1), (uint8_t *)buf, (uint16_t)len, I2C_TIMEOUT_MS);
    return i2c_finish(i2c_num, status);
}

// receive len bytes from device at 7-bit addr into buf
// generates START, sends address + read bit, clocks in data, generates STOP
bool bm_i2c_read(uint8_t i2c_num, uint8_t addr, uint8_t *buf, size_t len) {
    if (len == 0) { return true; } //nothing to read, likely error on users part, the function itself succeeded since there are no bytes to read, so return true
    if (buf == NULL) { return false; }
    if (!bm_i2c_is_valid(i2c_num)) { return false; }
    if (addr > 0x7F) { return false; } //not a 7-bit address
    if (len > UINT16_MAX) { return false; } //HAL transfer size is 16-bit

    //wait for bus to be free before starting transaction to avoid collisions with other transactions
    if (!i2c_wait_bus_free(i2c_num)) { return false; }

    HAL_StatusTypeDef status = HAL_I2C_Master_Receive(
        &i2c_handles[i2c_num], (uint16_t)(addr << 1), buf, (uint16_t)len, I2C_TIMEOUT_MS);
    return i2c_finish(i2c_num, status);
}

// write reg_addr, then read len bytes into buf without releasing the bus
// issues a repeated START between write and read phases
// pattern for sensor register reads (BMP390, LSM9DS1)
bool bm_i2c_write_read(uint8_t i2c_num, uint8_t addr, uint8_t reg_addr, uint8_t *buf, size_t len) {
    if (!bm_i2c_is_valid(i2c_num)) { return false; }
    if (len == 0) { return true; } //nothing to read, likely error on users part, the function itself succeeded since there are no bytes to read, so return true
    if (buf == NULL) { return false; }
    if (addr > 0x7F) { return false; } //not a 7-bit address
    if (len > UINT16_MAX) { return false; } //HAL transfer size is 16-bit

    //wait for bus to be free before starting transaction to avoid collisions with other transactions
    if (!i2c_wait_bus_free(i2c_num)) { return false; }

    // HAL_I2C_Mem_Read sends reg_addr with a software-end write (no STOP),
    // then re-issues START with the read bit: a repeated START
    HAL_StatusTypeDef status = HAL_I2C_Mem_Read(
        &i2c_handles[i2c_num], (uint16_t)(addr << 1), reg_addr, I2C_MEMADD_SIZE_8BIT,
        buf, (uint16_t)len, I2C_TIMEOUT_MS);
    return i2c_finish(i2c_num, status);
}

// returns true if the I2C bus or peripheral is currently busy
// check before initiating a transfer to avoid collisions
// returns false for an uninitialized i2c_num
bool bm_i2c_is_busy(uint8_t i2c_num) {
    if (!bm_i2c_is_valid(i2c_num)) { return false; }
    I2C_HandleTypeDef *h = &i2c_handles[i2c_num];
    return
        __HAL_I2C_GET_FLAG(h, I2C_FLAG_BUSY) != RESET ||
        HAL_I2C_GetState(h) != HAL_I2C_STATE_READY;
}

// reads I2C_ISR and the HAL error code of the most recent transaction into
// status_out and error_out
// useful for diagnosing NAK, arbitration loss, or timeout conditions
bool bm_i2c_get_status(uint8_t i2c_num, uint32_t *status_out, uint32_t *error_out) {
    if (status_out == NULL) { return false; }
    if (error_out == NULL) { return false; }
    if (!bm_i2c_is_valid(i2c_num)) { return false; }
    *status_out = i2c_handles[i2c_num].Instance->ISR;
    *error_out = i2c_last_error[i2c_num];
    return true; //success
}

// disable and re-enable the I2C peripheral to recover from a hung bus
// use if SDA or SCL lines are stuck and normal transactions are not completing
bool bm_i2c_reset(uint8_t i2c_num) {
    if (!bm_i2c_is_valid(i2c_num)) { return false; }
    I2C_HandleTypeDef *h = &i2c_handles[i2c_num];
    // DeInit clears PE, which resets the peripheral's internal state machine
    // and flags (RM0440 §37.4.6). Init is kept in the handle across the call.
    if (HAL_I2C_DeInit(h) != HAL_OK) { return false; }
    if (HAL_I2C_Init(h) != HAL_OK) { return false; }
    return true; //success
}

//return false if i2c not initialized and true if it is
bool bm_i2c_is_valid(uint8_t i2c_num){
    if (i2c_num >= NUM_I2C_PERIPHERALS) {return false;}
    if (i2c_handles[i2c_num].Instance == NULL) {return false;}
    if (HAL_I2C_GetState(&i2c_handles[i2c_num]) == HAL_I2C_STATE_RESET) {return false;}
    return true;
}

// wait up to I2C_TIMEOUT_MS for the bus to go idle
// no reset on timeout since the transaction hasnt started and someone else might be using the bus
static bool i2c_wait_bus_free(uint8_t i2c_num) {
    uint32_t start_ms;
    if (!bm_systick_get_ms(&start_ms)) { return false; }
    while (bm_i2c_is_busy(i2c_num)) {
        if (bm_systick_timeout_elapsed(start_ms, I2C_TIMEOUT_MS)) { return false; }
    }
    return true;
}

// map a HAL transfer result to the driver's return convention
// HAL_BUSY means the handle was locked and nothing went on the bus, so no reset.
// HAL_ERROR (NAK, arbitration loss, bus error) and HAL_TIMEOUT happen after
// START, so the peripheral is reset to leave it in a known state.
static bool i2c_finish(uint8_t i2c_num, HAL_StatusTypeDef status) {
    i2c_last_error[i2c_num] = HAL_I2C_GetError(&i2c_handles[i2c_num]);
    if (status == HAL_OK) { return true; }
    if (status != HAL_BUSY) {
        // HAL already generated STOP for a NAK, so the bus itself is released
        bm_i2c_reset(i2c_num);
    }
    return false;
}
