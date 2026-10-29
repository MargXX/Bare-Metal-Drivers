#include "i2c.h"
#include "gpio.h"
#include "rcc.h"
#include "systick.h"
#include "uart.h"
#include "i2c_platform.h"
#include "gpio_platform.h"
#include "bmp390_reg.h"
#include "debug_print.h"
#include "debug_blink.h"

/*
 * por_test — cold power-on register state of the BMP390 (or BMP388).
 *
 * Records CHIP_ID and REV_ID, then measures OSR (0x1C) and PWR_CTRL (0x1B)
 * at a true power-on, before anything is written to the sensor, then again
 * after a soft reset. drivers/bmp390 is
 * NOT used: bm_bmp390_init soft-resets internally and rejects any CHIP_ID
 * other than 0x60, so only raw bm_i2c reads and writes appear here.
 *
 * Control: as its last step this test writes POR_SENTINEL_OSR into OSR. That
 * value survives an MCU reset or reflash but not a sensor power loss, so if
 * the first OSR read of a run returns it, the sensor was never power-cycled
 * and the "cold" reads are stale. EVENT.por_detected is printed as well, but
 * it is not a sufficient control alone: a soft reset from an earlier image
 * sets it too, and it stays set until something reads EVENT.
 *
 * Procedure: flash once, then remove ALL power (Nucleo USB and the debug
 * probe, which can back-power the board through its SWD and UART lines),
 * wait ~10 s, reconnect, and read the output. Do not reflash in between.
 *
 * Sensor on I2C1, PB9 (SDA, D14) / PB8 (SCL, D15). Output on USART1,
 * PC4 (TX) / PC5 (RX), at 115200 8N1.
 *
 * LED solid on: valid cold-POR capture. Slow blink: sensor was not
 * power-cycled, cold reads are invalid. Fast blink: init or bus failure.
 */

#define SENSOR_ADDR_SDO_LOW  0x76
#define SENSOR_ADDR_SDO_HIGH 0x77
#define BMP388_CHIP_ID       0x50
#define POR_SENTINEL_OSR     0x15   // osr_t = x4, osr_p = x32: matches neither 0x00 nor 0x02
#define POISON               0xA5

#define UART_TX_PIN BM_GPIO_PIN(BM_GPIO_PORT_C, 4)
#define UART_RX_PIN BM_GPIO_PIN(BM_GPIO_PORT_C, 5)
#define I2C_SDA_PIN BM_GPIO_PIN(BM_GPIO_PORT_B, 9)
#define I2C_SCL_PIN BM_GPIO_PIN(BM_GPIO_PORT_B, 8)

// set by the address probe in main; 0x77 unless only 0x76 answers
static uint8_t sensor_addr = SENSOR_ADDR_SDO_HIGH;


// raw single-register read, buffer poisoned so a short read cannot pass as data
static bool read_reg(uint8_t i2c_num, uint8_t uart_num, uint8_t reg, const char *label, uint8_t *out) {
    *out = POISON;
    bool result = bm_i2c_write_read(i2c_num, sensor_addr, reg, out, 1);
    bm_debug_print_result(uart_num, result, label);
    if (result) {
        bm_debug_print_labeled_hex8(uart_num, label, *out);
    } else {
        // I2C_ISR and HAL error code, e.g. 0x04 = NAK (HAL_I2C_ERROR_AF)
        uint32_t status, error;
        bm_i2c_get_status(i2c_num, &status, &error);
        bm_debug_print_hex32(uart_num, status);
        bm_debug_print_hex32(uart_num, error);
    }
    return result;
}

static bool write_reg(uint8_t i2c_num, uint8_t uart_num, uint8_t reg, uint8_t value, const char *label) {
    uint8_t buf[2] = {reg, value};
    bool result = bm_i2c_write(i2c_num, sensor_addr, buf, 2);
    bm_debug_print_result(uart_num, result, label);
    return result;
}


int main() {
    bool result = bm_rcc_clock_init();
    result = result && bm_systick_init();

    // LED setup
    bm_gpio_set_direction(DEBUG_LED_PIN, GPIO_DIR_OUTPUT);
    bm_gpio_put(DEBUG_LED_PIN, false);
    if (!result) { blink_loop(50); }

    // 3 blinks — signals start of test
    blink_n_times(3);

    uint8_t uart_num;
    bm_uart_init(&uart_num, 115200, UART_TX_PIN, UART_RX_PIN);

    uint8_t i2c_num;
    result = bm_i2c_init(&i2c_num, I2C_MODE_STANDARD, I2C_SDA_PIN, I2C_SCL_PIN);
    bm_debug_print_result(uart_num, result, "I2C init");
    if (!result) { blink_loop(50); }

    // ---- address probe: CHIP_ID at both SDO addresses, read-only ----
    // A NAK on every read says nothing about why; this separates "answers at
    // the other address" from "not answering at all".
    uint8_t probe_buf;
    bool ack_low = bm_i2c_write_read(i2c_num, SENSOR_ADDR_SDO_LOW, BMP390_REG_CHIP_ID, &probe_buf, 1);
    bm_debug_print_result(uart_num, ack_low, "ACK at 0x76 (SDO low)");
    bool ack_high = bm_i2c_write_read(i2c_num, SENSOR_ADDR_SDO_HIGH, BMP390_REG_CHIP_ID, &probe_buf, 1);
    bm_debug_print_result(uart_num, ack_high, "ACK at 0x77 (SDO high)");
    if (ack_low && !ack_high) { sensor_addr = SENSOR_ADDR_SDO_LOW; }

    // ---- phase 1: cold reads, nothing written to the sensor yet ----
    // OSR and PWR_CTRL first, before EVENT, so no read of ours precedes them
    // that could plausibly disturb state.
    uint8_t chip_id, rev_id, cold_osr, cold_pwr_ctrl, cold_event, cold_err;
    result = read_reg(i2c_num, uart_num, BMP390_REG_CHIP_ID, "CHIP_ID", &chip_id);
    result = result && read_reg(i2c_num, uart_num, BMP390_REG_REV_ID, "REV_ID", &rev_id);
    result = result && read_reg(i2c_num, uart_num, BMP390_REG_OSR, "cold OSR", &cold_osr);
    result = result && read_reg(i2c_num, uart_num, BMP390_REG_PWR_CTRL, "cold PWR_CTRL", &cold_pwr_ctrl);
    result = result && read_reg(i2c_num, uart_num, BMP390_REG_ERR_REG, "cold ERR_REG", &cold_err);
    result = result && read_reg(i2c_num, uart_num, BMP390_REG_EVENT, "cold EVENT", &cold_event);
    if (!result) { blink_loop(50); }

    if (chip_id == BMP390_CHIP_ID_VALUE) {
        bm_debug_print_result(uart_num, true, "part is BMP390");
    } else if (chip_id == BMP388_CHIP_ID) {
        bm_debug_print_result(uart_num, true, "part is BMP388");
    } else {
        bm_debug_print_result(uart_num, false, "part is unrecognised");
    }

    bool power_cycled = (cold_osr != POR_SENTINEL_OSR);
    bm_debug_print_result(uart_num, power_cycled, "sentinel absent (sensor was power-cycled)");
    bm_debug_print_result(uart_num, (cold_event & BMP390_EVENT_POR_DETECTED_Msk) != 0, "cold EVENT.por_detected set");

    // ---- phase 2: dirty OSR, then soft reset ----
    // Writing a non-default value first means a post-reset read of 0x00 or
    // 0x02 shows the reset actually changed the register.
    uint8_t dirty_osr, reset_osr, reset_pwr_ctrl, reset_event, reset_err, status;
    result = write_reg(i2c_num, uart_num, BMP390_REG_OSR, POR_SENTINEL_OSR, "write OSR sentinel");
    result = result && read_reg(i2c_num, uart_num, BMP390_REG_OSR, "OSR before reset", &dirty_osr);
    result = result && write_reg(i2c_num, uart_num, BMP390_REG_CMD, BMP390_CMD_SOFTRESET, "soft reset");
    if (!result) { blink_loop(50); }

    bm_systick_delay_ms(10);

    result = read_reg(i2c_num, uart_num, BMP390_REG_STATUS, "reset STATUS", &status);
    result = result && read_reg(i2c_num, uart_num, BMP390_REG_OSR, "reset OSR", &reset_osr);
    result = result && read_reg(i2c_num, uart_num, BMP390_REG_PWR_CTRL, "reset PWR_CTRL", &reset_pwr_ctrl);
    result = result && read_reg(i2c_num, uart_num, BMP390_REG_ERR_REG, "reset ERR_REG", &reset_err);
    result = result && read_reg(i2c_num, uart_num, BMP390_REG_EVENT, "reset EVENT", &reset_event);
    if (!result) { blink_loop(50); }

    bm_debug_print_result(uart_num, dirty_osr == POR_SENTINEL_OSR, "OSR write took");
    bm_debug_print_result(uart_num, reset_osr != POR_SENTINEL_OSR, "soft reset changed OSR");
    bm_debug_print_result(uart_num, (reset_event & BMP390_EVENT_POR_DETECTED_Msk) != 0, "reset EVENT.por_detected set");
    bm_debug_print_result(uart_num, reset_osr == cold_osr, "reset OSR matches cold OSR");

    // ---- phase 3: arm the control for the next run ----
    write_reg(i2c_num, uart_num, BMP390_REG_OSR, POR_SENTINEL_OSR, "arm OSR sentinel");

    if (power_cycled) {
        // solid on — cold reads are valid
        bm_gpio_put(DEBUG_LED_PIN, true);
        while (1) {}
    } else {
        // slow blink — sensor kept state from a previous run
        blink_loop(1000);
    }
}
