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

/// <summary>
/// Defines decibel levels for the points that will be defined for the VU meter needles themselves
/// </summary>
static const float vu_needle_lookup[104] =
{
    -32.00, -31.00, -30.00, -29.00, -28.00, -27.00, -26.00, -25.00, -24.00, -23.00, -22.00, -21.00, -20.00, -19.26,
    -18.55, -17.84, -17.13, -16.42, -15.71, -15.00, -14.26, -13.55, -12.84, -12.13, -11.42, -10.71, -10.00,  -9.84,
     -9.70,  -9.56,  -9.42,  -9.28,  -9.14,  -9.00,  -8.80,  -8.60,  -8.40,  -8.20,  -8.00,  -7.84,  -7.70,  -7.56,
     -7.42,  -7.28,  -7.14,  -7.00,  -6.90,  -6.80,  -6.70,  -6.60,  -6.50,  -6.40,  -6.30,  -6.20,  -6.10,  -6.00,
     -5.94,  -5.85,  -5.77,  -5.68,  -5.60,  -5.51,  -5.43,  -5.34,  -5.26,  -5.17,  -5.08,  -5.00,  -4.90,  -4.81,
     -4.72,  -4.63,  -4.54,  -4.45,  -4.36,  -4.27,  -4.18,  -4.09,  -4.00,  -3.88,  -3.77,  -3.66,  -3.55,  -3.44,
     -3.33,  -3.22,  -3.11,  -3.00,  -2.85,  -2.68,  -2.51,  -2.34,  -2.17,  -2.00,  -1.85,  -1.68,  -1.51,  -1.34,
     -1.17,  -1.00,  -0.75,  -0.50,  -0.25,   0.00
};

/// <summary>
/// Defines decibel levels for the points that will be defined for the linear VU meters themselves
/// </summary>
static const float vu_linear_lookup[56] =
{
    -99  -98.4, -97.8, -97.2, -96.6, -96, -93.6, -91.2, -88.8, -86.4, -84, -81.6, -79.2, -76.8, -74.4, -72, -70.4, -68.8, -67.2, -65.6,
    -64, -62.4, -60.8, -59.2, -57.6, -56, -52.8, -49.6, -46.4, -43.2, -40, -36.8, -33.6, -30.4, -27.2, -24, -22.8, -21.6, -20.4, -19.2,
    -18, -16.8, -15.6, -14.4, -13.2, -12, -10.8,  -9.6,  -8.4,  -7.2,  -6,  -4.8,  -3.6,  -2.4,  -1.2,  -0
};

// VU_LINEAR_BOTTOM_Y defines the bottom Y coordinate for the bottom end of the linear VU meter(s)
#define VU_LINEAR_BOTTOM_Y  60

/// <summary>
/// vu_linear_top_y defines the Y coordinate for the top end of the linear VU meter(s) for each decibel level in the
/// vu_linear_lookup array
/// </summary>
static const int vu_linear_top_y[56] =
{
    61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49, 48, 47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34,
    33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10,  9,  8,  7,  6
};

/// <summary>
/// vu_left_needle_bottom_x and vu_left_needle_bottom_y define the coordinates for the borrom end of the VU meter needle
/// for each decibel level in the vu_needle_lookup array.
/// </summary>
static const int vu_left_needle_bottom_x[104] =
{
    30, 31, 31, 32, 32, 32, 33, 33, 34, 34, 34, 35, 35, 36, 37, 37, 38, 38, 39, 39, 40, 40, 41, 41, 42, 42, 43, 43,
    44, 44, 45, 45, 46, 46, 47, 47, 47, 48, 48, 49, 49, 50, 50, 50, 51, 51, 52, 52, 53, 53, 53, 54, 54, 55, 55, 56,
    56, 56, 57, 57, 58, 58, 59, 59, 59, 60, 60, 61, 61, 62, 62, 62, 63, 63, 64, 64, 65, 65, 65, 66, 66, 67, 67, 68,
    68, 69, 69, 70, 70, 71, 71, 72, 72, 73, 73, 74, 74, 75, 75, 76, 76, 77, 78, 78
};
static const int vu_left_needle_bottom_y[104] =
{
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64
};

/// <summary>
/// vu_left_needle_top_x and vu_left_needle_top_y define the coordinates for the top end of the VU meter needle
/// for each decibel level in the vu_needle_lookup array.
/// </summary>
static const int vu_left_needle_top_x[104] =
{
     4,  4,  5,  6,  6,  7,  8,  8,  9,  9,  10, 11,  12,  13,  14,  15,  16,  17,  18,  19,  20,  21,
    22, 23, 24, 25, 26, 27, 28, 29, 30, 31,  32, 33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,
    44, 45, 46, 47, 48, 49, 50, 51, 52, 53,  54, 55,  56,  57,  58,  59,  60,  61,  62,  63,  64,  65,
    66, 67, 68, 69, 70, 71, 72, 73, 74, 75,  76, 77,  78,  79,  80,  81,  82,  83,  84,  85,  86,  87,
    88, 89, 90, 91, 92, 93, 94, 95, 96, 97,  98, 99, 100, 101, 102, 103,
};
static const int vu_left_needle_top_y[104] =
{
    21, 20, 20, 20, 19, 19, 19, 18, 18, 17, 17, 17, 16, 16, 15, 15, 14, 14, 13, 13, 13, 12, 12, 11, 11,
    11, 11, 10, 10,  9,  9,  9,  9,  8,  8,  8,  8,  7,  7,  7,  7,  7,  7,  7,  7,  6,  7,  6,  6,  6,
    6,   6,  6,  6,  6,  5,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  7,  7,  7,  7,  7,  7,  7,  7,  7,
    8,   8,  8,  8,  8,  9,  9,  9,  9, 10, 10, 10, 11, 11, 11, 12, 12, 13, 13, 13, 14, 14, 14, 15, 15,
    15, 16,  17, 17
};

#ifdef __cplusplus
extern "C" {
#endif 
    /// <summary>
    /// Draws VU meters to the framebuffer corresponding to the decibel levels passed in for each meter
    /// </summary>
    /// <param name="framebuffer"></param>
    /// <param name="main_left"></param>
    /// <param name="main_right"></param>
    /// <param name="center_left_left"></param>
    /// <param name="center_left"></param>
    /// <param name="center_right"></param>
    /// <param name="center_right_right"></param>
    /// <param name="color">True for white needle, False for black needle</param>
    void draw_vu_meters(ssd1322_framebuffer_t *framebuffer,
        float main_left,
        float main_right,
        float center_left_left,
        float center_left,
        float center_right,
        float center_right_right,
        bool color);
#ifdef __cplusplus
}
#endif
