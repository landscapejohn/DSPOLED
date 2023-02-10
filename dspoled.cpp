#include <iostream>
#include <stdio.h>
#include <string>
#include <unistd.h>
#include "vu_meters.h"
#include "dspoled.hpp"
#include "easywsclient.hpp"
#include "DspConnector.cpp"

#ifndef SSD1322_GRAPHICS
#define SSD1322_GRAPHICS
#include "ssd1322_graphics.h"
#endif

#ifndef SSD1322_6800
#define SSD1322_6800
#include "ssd1322_6800.h"
#endif

using easywsclient::WebSocket;

uint8_t *vu_pixel_data;

uint8_t *load_bitmap_to_memory(const char *filename)
{
    FILE *file;
    file = fopen(filename, "rb");

    // get start of actual bitmap data
    uint32_t bitmap_start_address;
    fseek(file, 10, SEEK_SET);
    fread(&bitmap_start_address, 4, 1, file);

    uint8_t *pixel_data = static_cast<uint8_t *>(malloc(8192));
    if (pixel_data)
    {
        fseek(file, bitmap_start_address, SEEK_SET);
        fread(pixel_data, 1, 8192, file);
    }

    return pixel_data;
}

void display_start_screen(ssd1322_t *oled)
{
    ssd1322_err_t *errp = NULL;
    ssd1322_framebuffer_t *start_screen_framebuffer = ssd1322_framebuffer_create(256, 64, errp);
    if (!start_screen_framebuffer) {
        printf("Framebuffer FAIL!");
    }

    uint8_t *logo_pixel_data = load_bitmap_to_memory(START_BITMAP);
    copy_bitmap_to_framebuffer(logo_pixel_data, start_screen_framebuffer);
    memset(logo_pixel_data, 0, sizeof(*logo_pixel_data));
    ssd1306_display_update(oled, start_screen_framebuffer);
    sleep(2);
    ssd1322_framebuffer_clear(start_screen_framebuffer);
    ssd1306_display_update(oled, start_screen_framebuffer);

    if (start_screen_framebuffer)
    {
        ssd1322_framebuffer_destroy(start_screen_framebuffer);
    }
    if (errp)
    {
        ssd1322_err_destroy(errp);
    }
}

int screensaver_x = 0;
int screensaver_y = 3;
int screensaver_x_direction = 3;
int screensaver_y_direction = 1;

int main()
{
    float capture_left_vu, capture_right_vu;
    float playback_1_vu, playback_2_vu, playback_3_vu, playback_4_vu;

    ssd1322_err_t *errp = NULL;
    ssd1322_t *oled = ssd1322_6800_open(256, 64, NULL);

    ssd1322_framebuffer_t *screensaver_framebuffer = ssd1322_framebuffer_create(256, 64, errp);
    ssd1322_framebuffer_t *vu_framebuffer = ssd1322_framebuffer_create(256, 64, errp);
    ssd1322_framebuffer_t *volume_framebuffer = ssd1322_framebuffer_create(256, 64, errp);

    if (!screensaver_framebuffer || !vu_framebuffer || !vu_framebuffer) {
        printf("Framebuffer FAIL!");
    }

    string current_config_name, new_config_name;
    string current_capture_rate, new_capture_rate;
    string current_processing_state, new_processing_state;
    bool current_muted, new_muted;

    float current_volume, new_volume;

    display_start_screen(oled);

    vu_pixel_data = load_bitmap_to_memory(VU_BACKGROUND_BITMAP);

    int loop_counter = 0;

    do
    {
        DspConnector::UpdateConfigName();
        new_config_name = DspConnector::ConfigName;

        DspConnector::UpdateCaptureRate();
        new_capture_rate = DspConnector::CaptureRate;

        DspConnector::UpdateVolume();
        new_volume = DspConnector::Volume;

        DspConnector::UpdateState();
        new_processing_state = DspConnector::State;

        DspConnector::UpdateMuted();
        new_muted = DspConnector::Muted;

        if (new_config_name != current_config_name
            || new_capture_rate != current_capture_rate
            || new_volume != current_volume
            || new_processing_state != current_processing_state
            || new_muted != current_muted)
        {
            if (loop_counter > 0)
            {
                if (current_processing_state == "Paused")
                {
                    display_start_screen(oled);
                }
            }
            loop_counter = 0;

            current_config_name = new_config_name;
            current_capture_rate = new_capture_rate;
            current_volume = new_volume;
            current_processing_state = new_processing_state;
            current_muted = new_muted;
        }

        DspConnector::UpdateCaptureSignalPeak();
        capture_left_vu = DspConnector::CaptureSignalPeak[0];
        capture_right_vu = DspConnector::CaptureSignalPeak[1];

        DspConnector::UpdatePlaybackSignalPeak();
        playback_1_vu = DspConnector::PlaybackSignalPeak[0];
        playback_2_vu = DspConnector::PlaybackSignalPeak[1];
        playback_3_vu = DspConnector::PlaybackSignalPeak[2];
        playback_4_vu = DspConnector::PlaybackSignalPeak[3];

        ssd1322_framebuffer_clear(vu_framebuffer);
        copy_bitmap_to_framebuffer(vu_pixel_data, vu_framebuffer);
        draw_vu_meters(vu_framebuffer, capture_left_vu, capture_right_vu, playback_1_vu, playback_2_vu, playback_3_vu, playback_4_vu, VU_NEEDLE_COLOR);

        if (loop_counter > 40) // ~4 seconds
        {
            // If nothing has changed in over four seconds then display the VU meters if the system isn't paused, otherwise go into screensaver mode

            if (current_processing_state != "Paused" && !current_muted)
            {
                ssd1306_display_update(oled, vu_framebuffer);
            }
            else
            {
                ssd1322_framebuffer_clear(screensaver_framebuffer);
                ssd1322_framebuffer_put_pixel(screensaver_framebuffer, screensaver_x, screensaver_y, 0xFF);
		        ssd1322_framebuffer_put_pixel(screensaver_framebuffer, screensaver_x, screensaver_y + 1, 0xFF);
                ssd1322_framebuffer_put_pixel(screensaver_framebuffer, screensaver_x + 1, screensaver_y, 0xFF);
                ssd1322_framebuffer_put_pixel(screensaver_framebuffer, screensaver_x + 1, screensaver_y + 1, 0xFF);
                ssd1322_framebuffer_put_pixel(screensaver_framebuffer, screensaver_x + 2, screensaver_y, 0xFF);
                ssd1322_framebuffer_put_pixel(screensaver_framebuffer, screensaver_x + 2, screensaver_y + 1, 0xFF);

                ssd1306_display_update(oled, screensaver_framebuffer);
                screensaver_x += screensaver_x_direction;
                screensaver_y += screensaver_y_direction;
                
                if (screensaver_x > 254)
                {
                    screensaver_x_direction = -screensaver_x_direction;
                }
                else if (screensaver_x < 1)
                {
                    screensaver_x_direction = -screensaver_x_direction;
                }

                if (screensaver_y > 62)
                {
                    screensaver_y_direction = -screensaver_y_direction;
                }
                else if (screensaver_y < 0)
                {
                    screensaver_y_direction = -screensaver_y_direction;
                }
            }
        }
        else
        {
            // If any state changes then switch to the data display framebuffer and show the current state

            char buf[64] = { 0 };
            ssd1322_framebuffer_box_t bbox;
            ssd1322_framebuffer_clear(volume_framebuffer);

            readlink(current_config_name.c_str(), buf, sizeof(buf) - 1);
            string current_config_buffer(buf);

            snprintf(buf, sizeof(buf) - 1, "%s", current_config_buffer.substr(30, 30).c_str());
            ssd1322_framebuffer_draw_text(volume_framebuffer, buf, 0, 5, 20, SSD1322_FONT_OPENSANS_LIGHT, 18, &bbox);

            snprintf(buf, sizeof(buf) - 1, "%s", current_capture_rate.c_str());
            ssd1322_framebuffer_draw_text(volume_framebuffer, buf, 0, 5, 39, SSD1322_FONT_OPENSANS_LIGHT, 18, &bbox);

            snprintf(buf, sizeof(buf) - 1, "%s", current_muted ? "Muted" : current_processing_state.c_str());
            ssd1322_framebuffer_draw_text(volume_framebuffer, buf, 0, 5, 58, SSD1322_FONT_OPENSANS_LIGHT, 18, &bbox);

            snprintf(buf, sizeof(buf) - 1, "%.0f", current_volume);
            ssd1322_framebuffer_draw_text(volume_framebuffer, buf, 0, 100, 62, SSD1322_FONT_OPENSANS_BOLD, 72, &bbox);

            snprintf(buf, sizeof(buf) - 1, "dB");
            ssd1322_framebuffer_draw_text(volume_framebuffer, buf, 0, 232, 40, SSD1322_FONT_OPENSANS_LIGHT, 14, &bbox);

            ssd1306_display_update(oled, volume_framebuffer);

            loop_counter++;
        }

        usleep(100000);
    } while (1);

    if (oled)
    {
        ssd1322_close(oled);
    }
    if (vu_framebuffer)
    {
        ssd1322_framebuffer_destroy(vu_framebuffer);
    }
    if (errp)
    {
        ssd1322_err_destroy(errp);
    }

    return 0;
}
