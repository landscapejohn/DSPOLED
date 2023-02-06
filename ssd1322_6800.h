/*
 * PORTIONS COPYRIGHT: 2020. Stealthy Labs LLC.
 * DATE: 2020-01-15
 * SOFTWARE: libssd1306-i2c
 * LICENSE: Refer license file
 * 
 * Modified from ssd1306_i2c.h: 2023 John Hodge
 */
#include <stdio.h>
#include "ssd1322_graphics.h"

#define LIBSSD1322_PACKAGE_VERSION "0.1.0"

 // Define RPi GPIO to display mapping for 8 bit 6800 parallel
#define OLED_DC 16
#define OLED_E 23
#define OLED_D0 26
#define OLED_D1 13
#define OLED_D2 6
#define OLED_D3 5
#define OLED_D4 22
#define OLED_D5 27
#define OLED_D6 17
#define OLED_D7 18
#define OLED_RESET 12
#define OLED_CS 25

typedef struct {
    uint16_t width; // default 128
    uint16_t height;  // default 64
    uint8_t *gddram_buffer; // buffer for GDDRAM size (height x width) + 1 bytes
    size_t gddram_buffer_len; // value = (height x width) + 1
    ssd1322_err_t *err; // for re-entrant error handling
} ssd1322_t;

typedef enum {
    SSD1322_CMD_ENABLE_GRAY_SCALE_TABLE,
    SSD1322_CMD_SET_COLUMN_ADDRESS, //Set Column start and end address
    SSD1322_CMD_WRITE_RAM, //Enable MCU to write Data into RAM
    SSD1322_CMD_READ_RAM, //Enable MCU to read Data from RAM
    SSD1322_CMD_SET_ROW_ADDRESS, //Set Row start and end address
    SSD1322_CMD_SET_REMAP_AND_DUAL_COM_LINE_MODE,
    SSD1322_CMD_SET_DISPLAY_START_LINE, //Set display RAM start line register from 0-127
    SSD1322_CMD_SET_DISPLAY_OFFSET, //Set vertical scroll by COM from 0-127
    SSD1322_CMD_ENTIRE_DISPLAY_OFF,
    SSD1322_CMD_ENTIRE_DISPLAY_ON,
    SSD1322_CMD_NORMAL_DISPLAY_MODE,
    SSD1322_CMD_INVERSE_DISPLAY_MODE,
    SSD1322_CMD_ENABLE_PARTIAL_DISPLAY, //This command turns ON partial mode, start row address, end row address.
    SSD1322_CMD_EXIT_PARTIAL_DISPLAY,
    SSD1322_CMD_VDD_REGULATOR_SELECTION,
    SSD1322_CMD_SET_SLEEP_MODE_ON,
    SSD1322_CMD_SET_SLEEP_MODE_OFF,
    SSD1322_CMD_SET_PHASE_LENGTH,
    SSD1322_CMD_SET_FRONT_CLOCK_DIVIDER_OSCILLATOR_FREQUENCY,
    SSD1322_CMD_DISPLAY_ENHANCEMENT_A,
    SSD1322_CMD_SET_GPIO,
    SSD1322_CMD_SET_SECOND_PRECHARGE_PERIOD,
    SSD1322_CMD_SET_GRAY_SCALE_TABLE,
    SSD1322_CMD_SELECT_DEFAULT_LINEAR_GRAY_SCALE_TABLE,
    SSD1322_CMD_SET_PRECHARGE_VOLTAGE,
    SSD1322_CMD_SET_VCOMH, //Set COM deselect voltage level [reset = 04h]
    SSD1322_CMD_SET_CONTRAST_CURRENT,
    SSD1322_CMD_MASTER_CONTRAST_CURRENT_CONTROL,
    SSD1322_CMD_SET_MUX_RATIO,
    SSD1322_CMD_DISPLAY_ENHANCEMENT_B,
    SSD1322_CMD_SET_COMMAND_LOCK
} ssd1322_cmd_t;

#ifdef __cplusplus
extern "C" {
#endif 
int ssd1322_run_cmd(ssd1322_t *oled, ssd1322_cmd_t cmd, uint8_t *data, size_t dlen);

void ssd1322_close(ssd1322_t *oled);

int ssd1306_display_update(ssd1322_t *oled, const ssd1322_framebuffer_t *fbp);

int ssd1306_display_clear(ssd1322_t *oled);

const char *ssd1306_version(void);

ssd1322_t *ssd1322_6800_open(uint16_t width, uint16_t height, FILE *logerr);
#ifdef __cplusplus
}
#endif
