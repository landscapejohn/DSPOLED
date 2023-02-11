#ifndef SSD1322_GRAPHICS
#define SSD1322_GRAPHICS
#include "ssd1322_graphics.h"
#endif

#ifndef SSD1322_6800
#define SSD1322_6800
#include "ssd1322_6800.h"
#endif

#include <stdio.h>
#include <unistd.h>
#include "vu_meters.h"

float left_level_indicated_index = 0;
float right_level_indicated_index = 0;

int get_vu_needle_index(float decibels)
{
    int i = 0;

    while (vu_needle_lookup[i] < decibels && i < 104)
    {
        i++;
    }

    return i;
}

int get_vu_linear_index(float decibels)
{
    int i = 0;

    while (vu_linear_lookup[i] < decibels && i < 56)
    {
        i++;
    }

    return i;
}

/// <summary>
/// Draws a vertical bar starting at (left_x, VU_LINEAR_BOTTOM_Y) three pixels wide up to y
/// </summary>
/// <param name="framebuffer"></param>
/// <param name="left_x"></param>
/// <param name="y"></param>
void draw_linear_meter(ssd1322_framebuffer_t *framebuffer, int left_x, int y)
{
    int x2 = left_x + 1;
    int x3 = left_x + 2;

    ssd1322_framebuffer_draw_line(framebuffer, left_x, VU_LINEAR_BOTTOM_Y, left_x, y, 0xFF);
    ssd1322_framebuffer_draw_line(framebuffer, x2, VU_LINEAR_BOTTOM_Y, x2, y, 0xFF);
    ssd1322_framebuffer_draw_line(framebuffer, x3, VU_LINEAR_BOTTOM_Y, x3, y, 0xFF);
}

/// <summary>
/// Having the VU meter indicator directly indicate the signal level results in a jumpy and disjointed appearance when audio levels
/// change drastically. This function instead aims to move the needle smoothly between levels but only moving the needle from its current
/// location by a specified amount in the direction of the target location.
/// </summary>
/// <param name="target">This is the index to the lookup array corresponding to the current decibel level of the signal to be displayed</param>
/// <param name="current">This is the index to the lookup array corresponding to the current location of the VU meter indicator</param>
/// <param name="amount">How much to move the needle towards the target direction</param>
/// <returns>New index location of the indicator after adjustment</returns>
int approach_target(int target_index, int current_index, int amount)
{
    if (target_index > current_index)
    {
        current_index += amount;
    }
    else if (target_index < current_index)
    {
        current_index -= amount;
    }

    if (current_index < 0)
    {
        current_index = 0;
    }
    else if (current_index > 103)
    {
        current_index = 103;
    }

    return current_index;
}

/// <summary>
/// Having the VU meter indicator directly indicate the signal level results in a jumpy and disjointed appearance when audio levels
/// change drastically. This function aims to adjust the speed of the indicator so that its speed is proportional to how close to the target
/// level it is currently indicating.
/// </summary>
/// <param name="current_index">This is the index to the lookup array corresponding to the current location of the VU meter indicator</param>
/// <param name="new_level">Decibel level of the signal to be indicated</param>
/// <returns>New index location of the indicator after adjustment</returns>
int get_index(int current_index, float new_level)
{
    int target_index = get_vu_needle_index(new_level);
    int difference = abs(target_index - current_index);

    if (difference < 2)
    {
        current_index = approach_target(target_index, current_index, 1);
    }
    if (difference < 3)
    {
        current_index = approach_target(target_index, current_index, 2);
    }
    if (difference < 7)
    {
        current_index = approach_target(target_index, current_index, 3);
    }
    else if (difference < 10)
    {
        current_index = approach_target(target_index, current_index, 6);
    }
    else
    {
        current_index = approach_target(target_index, current_index, 8);
    }

    return current_index;
}

void draw_vu_meters(ssd1322_framebuffer_t *framebuffer,
    float main_left,
    float main_right,
    float center_left_left,
    float center_left,
    float center_right,
    float center_right_right,
    char color)
{
    left_level_indicated_index = get_index(left_level_indicated_index, main_left);
    right_level_indicated_index = get_index(right_level_indicated_index, main_right);

    int vu_needle_left_index = left_level_indicated_index;
    int vu_needle_right_index = right_level_indicated_index;
    int vu_center_left_left_index = get_vu_linear_index(center_left_left);
    int vu_center_left_index = get_vu_linear_index(center_left);
    int vu_center_right_index = get_vu_linear_index(center_right);
    int vu_center_right_right_index = get_vu_linear_index(center_right_right);

    // Left VU meter
    ssd1322_framebuffer_draw_aa_line(framebuffer,
        vu_left_needle_bottom_x[vu_needle_left_index],
        vu_left_needle_bottom_y[vu_needle_left_index],
        vu_left_needle_top_x[vu_needle_left_index],
        vu_left_needle_top_y[vu_needle_left_index],
        color);

    // Right VU meter, +143 pixels from the left VU meter
    ssd1322_framebuffer_draw_aa_line(framebuffer,
        vu_left_needle_bottom_x[vu_needle_right_index] + 143,
        vu_left_needle_bottom_y[vu_needle_right_index],
        vu_left_needle_top_x[vu_needle_right_index] + 143,
        vu_left_needle_top_y[vu_needle_right_index],
        color);

    draw_linear_meter(framebuffer, 117, vu_linear_top_y[vu_center_left_left_index]);
    draw_linear_meter(framebuffer, 123, vu_linear_top_y[vu_center_left_index]);
    draw_linear_meter(framebuffer, 129, vu_linear_top_y[vu_center_right_index]);
    draw_linear_meter(framebuffer, 135, vu_linear_top_y[vu_center_right_right_index]);
}
