/*
 * PORTIONS COPYRIGHT: 2020. Stealthy Labs LLC.
 * DATE: 2020-01-15
 * SOFTWARE: libssd1306-i2c
 * LICENSE: Refer license file
 * 
 * Modified from ssd1306_i2c.c: 2023 John Hodge
 */

#include "ssd1322_6800.h"
#include <pigpio.h>
#include <unistd.h>

#ifndef SSD1322_GET_ERRFP
#define SSD1322_GET_ERRFP(P) stderr
#endif

#ifndef SSD1306_I2C_GET_ERRFP
#define SSD1306_I2C_GET_ERRFP(P) ((P) != NULL && (P)->err != NULL && (P)->err->err_fp != NULL) ? (P)->err->err_fp : stderr
#endif // SSD1306_I2C_GET_ERRFP

// Send data to display
void write_byte(char bits, bool isCommand)
{
    gpioWrite(OLED_D0, (bits & (1 << 0)) > 0 ? 1 : 0);
    gpioWrite(OLED_D1, (bits & (1 << 1)) > 0 ? 1 : 0);
    gpioWrite(OLED_D2, (bits & (1 << 2)) > 0 ? 1 : 0);
    gpioWrite(OLED_D3, (bits & (1 << 3)) > 0 ? 1 : 0);
    gpioWrite(OLED_D4, (bits & (1 << 4)) > 0 ? 1 : 0);
    gpioWrite(OLED_D5, (bits & (1 << 5)) > 0 ? 1 : 0);
    gpioWrite(OLED_D6, (bits & (1 << 6)) > 0 ? 1 : 0);
    gpioWrite(OLED_D7, (bits & (1 << 7)) > 0 ? 1 : 0);
    if (isCommand)
    {
        gpioWrite(OLED_DC, 0);
    }
    gpioWrite(OLED_CS, 0);
    gpioWrite(OLED_CS, 1);
    if (isCommand)
    {
        gpioWrite(OLED_DC, 1);
    }
}

ssize_t write_display(uint8_t cmd_buf[8193], size_t cmd_sz)
{
    uint16_t bytesWritten = -1;

    if (cmd_sz == 0)
    {
        write_byte(cmd_buf[0], true);
        bytesWritten++;
    }
    else
    {
        for (size_t idx = 0; idx <= cmd_sz; idx++)
        {
            write_byte(cmd_buf[idx], idx == 0 ? true : false);
            bytesWritten++;
        }
    }

    return bytesWritten;
}

static size_t ssd1322_internal_get_cmd_bytes(ssd1322_cmd_t cmd, uint8_t *data, size_t dlen, uint8_t *cmdbuf, size_t cmd_buf_max)
{
    size_t size = 1; // default
    if (!cmdbuf || cmd_buf_max < 16 || (data != NULL && dlen == 0)) {
        return 0;//error
    }

    // fill it up
    for (size_t idx = 0; idx < cmd_buf_max; ++idx)
    {
        cmdbuf[idx] = 0xE3; // NOP by default
    }
    switch (cmd)
    {
        case SSD1322_CMD_ENABLE_GRAY_SCALE_TABLE:
            cmdbuf[0] = 0x00;
            break;
        case SSD1322_CMD_SET_COLUMN_ADDRESS:
            cmdbuf[0] = 0x15; // set column address
            if (data && dlen >= 2)
            {
                cmdbuf[1] = data[0] & 0x7F;
                cmdbuf[2] = data[1] & 0x7F;
            }
            else
            {
                cmdbuf[1] = 0x00; // RESET
                cmdbuf[2] = 0x77; // RESET
            }
            size = 3;
            break;
        case SSD1322_CMD_WRITE_RAM:
            cmdbuf[0] = 0x5C;
            break;
        case SSD1322_CMD_READ_RAM:
            cmdbuf[0] = 0x5D;
            break;
        case SSD1322_CMD_SET_ROW_ADDRESS:
            cmdbuf[0] = 0x75; // set row address
            if (data && dlen >= 2)
            {
                cmdbuf[1] = data[0] & 0x7F;
                cmdbuf[2] = data[1] & 0x7F;
            }
            else
            {
                cmdbuf[1] = 0x00; // RESET
                cmdbuf[2] = 0x7F; // RESET
            }
            size = 3;
            break;
        case SSD1322_CMD_SET_REMAP_AND_DUAL_COM_LINE_MODE:
            cmdbuf[0] = 0xA0;
            cmdbuf[1] = data[0] & 0x37;
            cmdbuf[2] = data[1] & (0x10 | 0x01);
            size = 3;
            break;
        case SSD1322_CMD_SET_DISPLAY_START_LINE:
            cmdbuf[0] = 0xA1;
            cmdbuf[1] = data[0];
            size = 2;
            break;
        case SSD1322_CMD_SET_DISPLAY_OFFSET:
            cmdbuf[0] = 0xA2;
            cmdbuf[1] = data[0];
            size = 2;
            break;
        case SSD1322_CMD_ENTIRE_DISPLAY_OFF:
            cmdbuf[0] = 0xA4;
            break;
        case SSD1322_CMD_ENTIRE_DISPLAY_ON:
            cmdbuf[0] = 0xA5;
            break;
        case SSD1322_CMD_NORMAL_DISPLAY_MODE:
            cmdbuf[0] = 0xA6;
            break;
        case SSD1322_CMD_INVERSE_DISPLAY_MODE:
            cmdbuf[0] = 0xA7;
            break;
        case SSD1322_CMD_ENABLE_PARTIAL_DISPLAY:
            cmdbuf[0] = 0xA8;
            cmdbuf[1] = data[0] & 0x7F;
            cmdbuf[2] = data[1] & 0x7F;
            size = 3;
            break;
        case SSD1322_CMD_EXIT_PARTIAL_DISPLAY:
            cmdbuf[0] = 0xA9;
            break;
        case SSD1322_CMD_VDD_REGULATOR_SELECTION:
            cmdbuf[0] = 0xAB;
            cmdbuf[1] = data[0] & 0x01;
            break;
        case SSD1322_CMD_SET_SLEEP_MODE_ON:
            cmdbuf[0] = 0xAE;
            break;
        case SSD1322_CMD_SET_SLEEP_MODE_OFF:
            cmdbuf[0] = 0xAF;
            break;
        case SSD1322_CMD_SET_PHASE_LENGTH:
            cmdbuf[0] = 0xB1;
            cmdbuf[1] = data[0];
            size = 2;
            break;
        case SSD1322_CMD_SET_FRONT_CLOCK_DIVIDER_OSCILLATOR_FREQUENCY:
            cmdbuf[0] = 0xB3;
            cmdbuf[1] = data[0];
            size = 2;
            break;
        case SSD1322_CMD_DISPLAY_ENHANCEMENT_A:
            cmdbuf[0] = 0xB4;
            cmdbuf[1] = data[0] & (0xA3 | 0xA0);
            cmdbuf[2] = data[1] & (0xFD | 0x05);
            size = 3;
            break;
        case SSD1322_CMD_SET_GPIO:
            cmdbuf[0] = 0xB5;
            cmdbuf[1] = data[0];
            size = 2;
            break;
        case SSD1322_CMD_SET_SECOND_PRECHARGE_PERIOD:
            cmdbuf[0] = 0xB6;
            cmdbuf[1] = data[0];
            size = 2;
            break;
        case SSD1322_CMD_SET_GRAY_SCALE_TABLE:
            cmdbuf[0] = 0xB8;
            cmdbuf[1] = data[0];
            cmdbuf[2] = data[1];
            cmdbuf[3] = data[2];
            cmdbuf[4] = data[3];
            cmdbuf[5] = data[4];
            cmdbuf[6] = data[5];
            cmdbuf[7] = data[6];
            cmdbuf[8] = data[7];
            cmdbuf[9] = data[8];
            cmdbuf[10] = data[9];
            cmdbuf[11] = data[10];
            cmdbuf[12] = data[11];
            cmdbuf[13] = data[12];
            cmdbuf[14] = data[13];
            cmdbuf[15] = data[14];
            size = 16;
            break;
        case SSD1322_CMD_SELECT_DEFAULT_LINEAR_GRAY_SCALE_TABLE:
            cmdbuf[0] = 0xB9;
            break;
        case SSD1322_CMD_SET_PRECHARGE_VOLTAGE:
            cmdbuf[0] = 0xBB;
            cmdbuf[1] = data[0];
            size = 2;
            break;
        case SSD1322_CMD_SET_VCOMH:
            cmdbuf[0] = 0xBE;
            cmdbuf[1] = data[0] & 0x07;
            size = 2;
            break;
        case SSD1322_CMD_SET_CONTRAST_CURRENT:
            cmdbuf[0] = 0xC1;
            cmdbuf[1] = data[0];
            size = 2;
            break;
        case SSD1322_CMD_MASTER_CONTRAST_CURRENT_CONTROL:
            cmdbuf[0] = 0xC7;
            cmdbuf[1] = data[0];
            size = 2;
            break;
        case SSD1322_CMD_SET_MUX_RATIO:
            cmdbuf[0] = 0xCA;
            cmdbuf[1] = data[0];
            break;
        case SSD1322_CMD_DISPLAY_ENHANCEMENT_B:
            cmdbuf[0] = 0xD1;
            cmdbuf[1] = data[0] & 0xB2;
            cmdbuf[2] = 0x20;
            size = 3;
            break;
        case SSD1322_CMD_SET_COMMAND_LOCK:
            cmdbuf[0] = 0xFD;
            cmdbuf[1] = data[0] & 0x16;
            size = 2;
            break;
        default:
            cmdbuf[0] = 0xE3; // NOP
            break;
    }
    return size;
}

int ssd1322_run_cmd(ssd1322_t *oled, ssd1322_cmd_t cmd, uint8_t *data, size_t dlen)
{
    FILE *err_fp = SSD1322_GET_ERRFP(oled);
    if (dlen > 0 && !data) {
        fprintf(err_fp, "WARN: data pointer is NULL but dlen is %zu. Ignoring\n", dlen);
        dlen = 0;
        data = NULL;
    }

    uint8_t cmd_buf[8193] = { 0 };
    const size_t cmd_buf_max = 8193;
    size_t cmd_sz = ssd1322_internal_get_cmd_bytes(cmd, data, dlen, cmd_buf, cmd_buf_max);
    if (cmd_sz == 0 || cmd_sz > cmd_buf_max) {
        fprintf(err_fp, "WARN: Unknown cmd given %d of size %ld\n", cmd, cmd_sz);
        return -1;
    }
    size_t nb = write_display(cmd_buf, cmd_sz);
    if (nb < 0) {
        oled->err->errnum = errno;
        //strerror_r(oled->err->errnum, oled->err->errbuf, oled->err->errlen);
        fprintf(err_fp, "ERROR: Failed to write cmd ");
        for (size_t idx = 0; idx < cmd_sz; ++idx) {
            fprintf(err_fp, "%c0x%02x%c", (idx == 0) ? '[' : ' ', cmd_buf[idx], (idx == (cmd_sz - 1)) ? ']' : ',');
        }
        fprintf(err_fp, " to device: %s\n", oled->err->errbuf);
        return -1;
    }
    //fprintf(err_fp, "INFO: Wrote %zd bytes of cmd ", nb);
    //for (size_t idx = 0; idx < cmd_sz; ++idx) {
    //    fprintf(err_fp, "%c0x%02x%c", (idx == 0) ? '[' : ' ', cmd_buf[idx], (idx == (cmd_sz - 1)) ? ']' : ',');
    //}
    //fprintf(err_fp, " to device\n");
    return 0;
}

void ssd1322_close(ssd1322_t *oled)
{
    if (oled) {
        if (oled->gddram_buffer) {
            free(oled->gddram_buffer);
        }
        oled->gddram_buffer = NULL;

        ssd1322_err_destroy(oled->err);
        oled->err = NULL;
        memset(oled, 0, sizeof(*oled));
        free(oled);
        oled = NULL;
    }
}

int ssd1306_display_update(ssd1322_t *oled, const ssd1322_framebuffer_t *fbp)
{
    FILE *err_fp = SSD1322_GET_ERRFP(oled);
    if (!oled || !oled->gddram_buffer || oled->gddram_buffer_len == 0) {
        fprintf(err_fp, "ERROR: Invalid ssd1306 I2C object\n");
        return -1;
    }
    int rc = 0;
    uint8_t x[2] = { 0x1C, 0x5B };
    rc |= ssd1322_run_cmd(oled, SSD1322_CMD_SET_COLUMN_ADDRESS, x, 2);

    x[0] = 0x00;
    x[1] = 0x3F;
    rc |= ssd1322_run_cmd(oled, SSD1322_CMD_SET_ROW_ADDRESS, x, 2);

    if (rc != 0) {
        fprintf(err_fp, "WARN: Unable to update display, exiting from earlier errors\n");
        return -1;
    }

    oled->gddram_buffer[0] = 0x5C; // Enable write to RAM
    // if the framebuffer pointer is provided, we copy it to GDDRAM, otherwise
    // we just display the GDDRAM
    if (fbp) {
        // invalid data in pointer
        if (!(fbp->buffer) || fbp->len == 0 || (fbp->len != (oled->gddram_buffer_len - 1))) {
            fprintf(err_fp, "ERROR: Invalid ssd1322 framebuffer object. FB: %ld GD: %ld\n", fbp->len, oled->gddram_buffer_len);
            return -1;
        }
        memcpy(&(oled->gddram_buffer[1]), fbp->buffer, fbp->len);
    }
    // the rest is framebuffer data for the GDDRAM as in section 8.1.5.2
    size_t nb = write_display(oled->gddram_buffer, oled->gddram_buffer_len);
    if (nb < 0) {
        oled->err->errnum = errno;
        fprintf(err_fp, "ERROR: Failed to write %zu bytes of screen buffer to device: %s\n", oled->gddram_buffer_len, oled->err->errbuf);
        return -1;
    }

    return 0;
}

int ssd1306_display_clear(ssd1322_t *oled)
{
    if (oled != NULL && oled->gddram_buffer != NULL && oled->gddram_buffer_len > 0) {
        memset(oled->gddram_buffer, 0, sizeof(uint8_t) * oled->gddram_buffer_len);
        return ssd1306_display_update(oled, NULL);
    }
    else {
        FILE *err_fp = SSD1322_GET_ERRFP(oled);
        fprintf(err_fp, "ERROR: Invalid OLED object. Failed to clear display\n");
        return -1;
    }
}

const char *ssd1306_version(void)
{
    return LIBSSD1322_PACKAGE_VERSION;
}

int ssd1322_set_brightness(ssd1322_t* oled, char brightness)
{
    int rc = 0;

    uint8_t data[2];
    data[0] = brightness;
    rc |= ssd1322_run_cmd(oled, SSD1322_CMD_MASTER_CONTRAST_CURRENT_CONTROL, data, 1);

    return rc;
}

int ssd1306_display_initialize(ssd1322_t *oled)
{
    int rc = 0;
    uint8_t data[2];
    FILE *err_fp = SSD1322_GET_ERRFP(oled);
    if (!oled) {
        fprintf(err_fp, "ERROR: Invalid ssd1322 object\n");
        return -1;
    }
    do {
        data[0] = 0x12;
        rc |= ssd1322_run_cmd(oled, SSD1322_CMD_SET_COMMAND_LOCK, data, 1);
        if (rc < 0) break;

        rc |= ssd1322_run_cmd(oled, SSD1322_CMD_SET_SLEEP_MODE_ON, 0, 0);
        if (rc < 0) break;

        data[0] = 0xA0;
        rc |= ssd1322_run_cmd(oled, SSD1322_CMD_SET_FRONT_CLOCK_DIVIDER_OSCILLATOR_FREQUENCY, data, 1);
        if (rc < 0) break;

        data[0] = 0x3F;
        rc |= ssd1322_run_cmd(oled, SSD1322_CMD_SET_MUX_RATIO, data, 1);
        if (rc < 0) break;

        data[0] = 0x00;
        rc |= ssd1322_run_cmd(oled, SSD1322_CMD_SET_DISPLAY_OFFSET, data, 1);
        if (rc < 0) break;

        data[0] = 0x01;
        rc |= ssd1322_run_cmd(oled, SSD1322_CMD_VDD_REGULATOR_SELECTION, data, 1);
        if (rc < 0) break;

        data[0] = 0x00;
        rc |= ssd1322_run_cmd(oled, SSD1322_CMD_SET_DISPLAY_START_LINE, data, 1);
        if (rc < 0) break;

        data[0] = 0x14;
        data[1] = 0x11;
        rc |= ssd1322_run_cmd(oled, SSD1322_CMD_SET_REMAP_AND_DUAL_COM_LINE_MODE, data, 2);
        if (rc < 0) break;
        
        data[0] = 0xA0;
        data[1] = 0xFD;
        rc |= ssd1322_run_cmd(oled, SSD1322_CMD_DISPLAY_ENHANCEMENT_A, data, 2);
        if (rc < 0) break;

        data[0] = 0xA6;
        rc |= ssd1322_run_cmd(oled, SSD1322_CMD_NORMAL_DISPLAY_MODE, data, 1);
        if (rc < 0) break;
        
        rc |= ssd1322_run_cmd(oled, SSD1322_CMD_EXIT_PARTIAL_DISPLAY, 0, 0);
        if (rc < 0) break;
        
        data[0] = 0xA2;
        rc |= ssd1322_run_cmd(oled, SSD1322_CMD_DISPLAY_ENHANCEMENT_B, data, 1);
        if (rc < 0) break;

        data[0] = 0x00;
        rc |= ssd1322_run_cmd(oled, SSD1322_CMD_SET_GPIO, data, 1);
        if (rc < 0) break;
        
        rc |= ssd1322_run_cmd(oled, SSD1322_CMD_SELECT_DEFAULT_LINEAR_GRAY_SCALE_TABLE, 0, 0);
        if (rc < 0) break;
        
        data[0] = 0xAF;
        rc |= ssd1322_run_cmd(oled, SSD1322_CMD_SET_SLEEP_MODE_OFF, data, 1);
        if (rc < 0) break;

        uint8_t data[2];
        data[0] = 0x1F;
        rc |= ssd1322_run_cmd(oled, SSD1322_CMD_SET_PRECHARGE_VOLTAGE, data, 1);
        if (rc < 0) break;

        data[0] = 0x07;
        rc |= ssd1322_run_cmd(oled, SSD1322_CMD_SET_VCOMH, data, 1);
        if (rc < 0) break;

        data[0] = 0xFF;
        rc |= ssd1322_run_cmd(oled, SSD1322_CMD_SET_CONTRAST_CURRENT, data, 1);
        if (rc < 0) break;

        data[0] = 0x04;
        rc |= ssd1322_run_cmd(oled, SSD1322_CMD_SET_SECOND_PRECHARGE_PERIOD, data, 1);
        if (rc < 0) break;

        data[0] = 0xFF;
        rc |= ssd1322_run_cmd(oled, SSD1322_CMD_SET_PHASE_LENGTH, data, 1);
        if (rc < 0) break;

        data[0] = 0x0F;
        rc |= ssd1322_run_cmd(oled, SSD1322_CMD_MASTER_CONTRAST_CURRENT_CONTROL, data, 1);
        if (rc < 0) break;
    } while (0);

    return rc;
}

ssd1322_t *ssd1322_6800_open(uint16_t width, uint16_t height, FILE *logerr)
{
    ssd1322_t *oled = NULL;
    int rc = 0;
    FILE *err_fp = logerr == NULL ? stderr : logerr;
    do {
        oled = calloc(1, sizeof(*oled));
        if (!oled) {
            fprintf(err_fp, "ERROR: Failed to allocate memory of size %zu bytes\n", sizeof(*oled));
            rc = -1;
            break;
        }
        oled->err = ssd1322_err_create(err_fp);
        if (!oled->err) {
            rc = -1;
            break;
        }

        oled->err->errnum = 0;
        memset(oled->err->errbuf, 0, oled->err->errlen);

        if (width == 256)
        {
            oled->width = 256;
        }
        else if (width == 128 || width == 0) {
            oled->width = 128;
        }
        else if (width == 96) {
            oled->width = 96;
        }
        else {
            fprintf(err_fp, "WARN: OLED screen width cannot be %d. has to be either 96, 128, or 256. Using 128\n", width);
            oled->width = 128;
        }

        if (height == 64 || height == 0) {
            oled->height = 64;
        }
        else if (height == 32) {
            oled->height = 32;
        }
        else if (height == 16) {
            oled->height = 16;
        }
        else {
            fprintf(err_fp, "WARN: OLED screen height cannot be %d. has to be either 16, 32, or 64. Using %d\n", height, (oled->width == 96) ? 16 : 64);
            oled->height = (oled->width == 96) ? 16 : 64;
        }
        // this is width x height nibbles of GDDRAM, 1 byte for every two pixels
        oled->gddram_buffer_len = sizeof(uint8_t) * (oled->width * oled->height / 2) + 1;
        oled->gddram_buffer = calloc(sizeof(uint8_t), oled->gddram_buffer_len);
        if (!oled->gddram_buffer) {
            oled->err->errnum = errno;
            fprintf(err_fp, "ERROR: Out of memory allocating %zu bytes for screen buffer\n",
                oled->gddram_buffer_len);
            rc = -1;
            break;
        }

        if (gpioInitialise() < 0)
        {
            oled->err->errnum = errno;
            fprintf(err_fp, "ERROR: Failed to initialize GPIO.\n");
            rc = -1;
            break;
        }

        gpioSetMode(OLED_DC, PI_OUTPUT);
        gpioSetMode(OLED_E, PI_OUTPUT);
        gpioSetMode(OLED_D0, PI_OUTPUT);
        gpioSetMode(OLED_D1, PI_OUTPUT);
        gpioSetMode(OLED_D2, PI_OUTPUT);
        gpioSetMode(OLED_D3, PI_OUTPUT);
        gpioSetMode(OLED_D4, PI_OUTPUT);
        gpioSetMode(OLED_D5, PI_OUTPUT);
        gpioSetMode(OLED_D6, PI_OUTPUT);
        gpioSetMode(OLED_D7, PI_OUTPUT);
        gpioSetMode(OLED_RESET, PI_OUTPUT);
        gpioSetMode(OLED_CS, PI_OUTPUT);

        gpioWrite(OLED_RESET, 0);
        sleep(0.15);
        gpioWrite(OLED_RESET, 1);
        sleep(0.15);

        gpioWrite(OLED_DC, 1);
        gpioWrite(OLED_E, 1);

        ssd1306_display_initialize(oled);
    } while (0);
    if (rc < 0) {
        ssd1322_close(oled);
        oled = NULL;
    }
    return oled;
}
