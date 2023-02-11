/*
 * COPYRIGHT: 2020. Stealthy Labs LLC.
 * DATE: 2020-01-15
 * SOFTWARE: libssd1322-i2c
 * LICENSE: Refer license file
 * 
 * Modified from ssd1306_graphics.h: 2023 John Hodge
 */

#ifndef __LIB_ssd1322_GFX_H__
#define __LIB_ssd1322_GFX_H__

#include <errno.h>
#include <stdbool.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

#define LIBSSD1322_PACKAGE_VERSION "0.1.0"

#define SSD1322_ATOMIC_INCREMENT(A) __atomic_add_fetch((A), 1, __ATOMIC_SEQ_CST)
#define SSD1322_ATOMIC_DECREMENT(A) __atomic_sub_fetch((A), 1, __ATOMIC_SEQ_CST)
#define SSD1322_ATOMIC_ZERO(A) __atomic_clear((A), __ATOMIC_SEQ_CST)
#define SSD1322_ATOMIC_SET(A,B) __atomic_store_n((A),(B), __ATOMIC_SEQ_CST)
#define SSD1322_ATOMIC_IS_EQUAL(A,B) __atomic_compare_exchange_n((A),(B),*(A),1,__ATOMIC_SEQ_CST,__ATOMIC_SEQ_CST)

    const char *ssd1322_fb_version(void);

    typedef struct {
        int errnum; // store the errno here
        char *errbuf; // error string buffer, allocated on the heap
        size_t errlen; // size of the error string buffer
        FILE *err_fp; // err file pointer for messages. default is stderr
        volatile int _ref; // reference counted
    } ssd1322_err_t;

    ssd1322_err_t *ssd1322_err_create(FILE *fp); // if fp is NULL, stderr is used
    void ssd1322_err_destroy(ssd1322_err_t *err);
#define SSD1322_ERR_REF_INC(A) if ((A) != NULL) SSD1322_ATOMIC_INCREMENT(&((A)->_ref))

    typedef struct ssd1322_font_ ssd1322_font_t;

#define SSD1322_FONT_DEFAULT SSD1322_FONT_VERA
#define SSD1322_FONT_MAX SSD1322_FONT_CUSTOM

    typedef enum {
        // bitstream-vera
        SSD1322_FONT_VERA = 0,
        SSD1322_FONT_VERA_BOLD,
        SSD1322_FONT_VERA_ITALIC,
        SSD1322_FONT_VERA_BOLDITALIC,
        // freefont Mono
        SSD1322_FONT_FREEMONO,
        SSD1322_FONT_FREEMONO_BOLD,
        SSD1322_FONT_FREEMONO_ITALIC,
        SSD1322_FONT_FREEMONO_BOLDITALIC,
        // OpenSans
        SSD1322_FONT_OPENSANS_LIGHT,
        SSD1322_FONT_OPENSANS_REGULAR,
        SSD1322_FONT_OPENSANS_SEMIBOLD,
        SSD1322_FONT_OPENSANS_BOLD,
        SSD1322_FONT_OPENSANS_EXTRABOLD,
        // OpenSans Italic
        SSD1322_FONT_OPENSANS_LIGHTITALIC,
        SSD1322_FONT_OPENSANS_ITALIC,
        SSD1322_FONT_OPENSANS_SEMIBOLDITALIC,
        SSD1322_FONT_OPENSANS_BOLDITALIC,
        SSD1322_FONT_OPENSANS_EXTRABOLDITALIC,
        // OpenSans Condensed
        SSD1322_FONT_OPENSANS_CONDLIGHT,
        SSD1322_FONT_OPENSANS_CONDBOLD,
        SSD1322_FONT_OPENSANS_CONDLIGHTITALIC,
        // last font
        SSD1322_FONT_CUSTOM              // use the ssd1322_graphics_options_t with this
    } ssd1322_fontface_t;

    [[maybe_unused]]
    static const char *ssd1322_fontface_paths[SSD1322_FONT_MAX + 1] = {
    "/usr/share/fonts/truetype/ttf-bitstream-vera/Vera.ttf",
    "/usr/share/fonts/truetype/ttf-bitstream-vera/VeraBd.ttf",
    "/usr/share/fonts/truetype/ttf-bitstream-vera/VeraIt.ttf",
    "/usr/share/fonts/truetype/ttf-bitstream-vera/VeraBI.ttf",

    "/usr/share/fonts/truetype/freefont/FreeMono.ttf",
    "/usr/share/fonts/truetype/freefont/FreeMonoBold.ttf",
    "/usr/share/fonts/truetype/freefont/FreeMonoOblique.ttf",
    "/usr/share/fonts/truetype/freefont/FreeMonoBoldOblique.ttf",

    "/usr/share/fonts/truetype/open-sans/OpenSans-Light.ttf",
    "/usr/share/fonts/truetype/open-sans/OpenSans-Regular.ttf",
    "/usr/share/fonts/truetype/open-sans/OpenSans-Semibold.ttf",
    "/usr/share/fonts/truetype/open-sans/OpenSans-Bold.ttf",
    "/usr/share/fonts/truetype/open-sans/OpenSans-ExtraBold.ttf",

    "/usr/share/fonts/truetype/open-sans/OpenSans-LightItalic.ttf",
    "/usr/share/fonts/truetype/open-sans/OpenSans-Italic.ttf",
    "/usr/share/fonts/truetype/open-sans/OpenSans-SemiboldItalic.ttf",
    "/usr/share/fonts/truetype/open-sans/OpenSans-BoldItalic.ttf",
    "/usr/share/fonts/truetype/open-sans/OpenSans-ExtraBoldItalic.ttf",

    "/usr/share/fonts/truetype/open-sans/OpenSans-CondLight.ttf",
    "/usr/share/fonts/truetype/open-sans/OpenSans-CondBold.ttf",
    "/usr/share/fonts/truetype/open-sans/OpenSans-CondLightItalic.ttf",
    NULL
    };

    [[maybe_unused]]
    static const char *ssd1322_fontface_names[SSD1322_FONT_MAX + 1] = {
        "SSD1322_FONT_VERA",
        "SSD1322_FONT_VERA_BOLD",
        "SSD1322_FONT_VERA_ITALIC",
        "SSD1322_FONT_VERA_BOLDITALIC",
        "SSD1322_FONT_FREEMONO",
        "SSD1322_FONT_FREEMONO_BOLD",
        "SSD1322_FONT_FREEMONO_ITALIC",
        "SSD1322_FONT_FREEMONO_BOLDITALIC",
        // OpenSans
        "SSD1322_FONT_OPENSANS_LIGHT",
        "SSD1322_FONT_OPENSANS_REGULAR",
        "SSD1322_FONT_OPENSANS_SEMIBOLD",
        "SSD1322_FONT_OPENSANS_BOLD",
        "SSD1322_FONT_OPENSANS_EXTRABOLD",
        // OpenSans Italic
        "SSD1322_FONT_OPENSANS_LIGHTITALIC",
        "SSD1322_FONT_OPENSANS_ITALIC",
        "SSD1322_FONT_OPENSANS_SEMIBOLDITALIC",
        "SSD1322_FONT_OPENSANS_BOLDITALIC",
        "SSD1322_FONT_OPENSANS_EXTRABOLDITALIC",
        // OpenSans Condensed
        "SSD1322_FONT_OPENSANS_CONDLIGHT",
        "SSD1322_FONT_OPENSANS_CONDBOLD",
        "SSD1322_FONT_OPENSANS_CONDLIGHTITALIC",
        "SSD1322_FONT_CUSTOM"
    };

    typedef struct {
        uint16_t width; // width of the framebuffer
        uint16_t height; // height of the framebuffer
        uint8_t *buffer; // buffer pointer
        size_t len; // length of the buffer
        ssd1322_err_t *err; // pointer to an optional error object
        ssd1322_font_t *font; // pointer to an opaque font library implementation - default is freetype
    } ssd1322_framebuffer_t;

    // ssd1322 has 1:1 correspondence between pixel to bit
    ssd1322_framebuffer_t *ssd1322_framebuffer_create(
        uint16_t width, // width of the screen in pixels
        uint16_t height, // height of the screen in pixels
        ssd1322_err_t *err // err object to be re-used. increments errstr_ref
    );

    void ssd1322_framebuffer_destroy(ssd1322_framebuffer_t *fbp);

    // clear the contents of the framebuffer
    int ssd1322_framebuffer_clear(ssd1322_framebuffer_t *fbp);
    // a debug function to dump the GDDRAM buffer to the FILE * err_fp in the
    // fb->err object. This dumps the data in the widthxheight format so the
    // developer can see how the bits in the RAM are being set 
    // this is called framebuffer_hexdump because this is not the actual GDDRAM dump
    // from the device, but the dump of the data stored in gddram_buffer pointer
    int ssd1322_framebuffer_hexdump(ssd1322_framebuffer_t *fbp);

    // this dump function dumps the width x height as bits. so that you can really
    // see deeply. set char zerobit to the character that represents the 0 bit. by
    // default it is '.' if it is not printable or is the number 0.
    // set char onebit to the character that represents the 1 bit. by default it is
    // '|' if it not printable or is the number 0 or 1.
    // set use_space to true if you want a space every byte, or false otherwise.
    // We also use terminal color codes to draw the text. red for '|' and green for '.'
    // the boolean use_color needs to be set to use this

    int ssd1322_framebuffer_bitdump_custom(ssd1322_framebuffer_t *fbp,
        char zerobit, char onebit, bool use_space, bool use_color);
#define ssd1322_framebuffer_bitdump(A) ssd1322_framebuffer_bitdump_custom((A), 0, 0, true, true)
#define ssd1322_framebuffer_bitdump_nospace(A) ssd1322_framebuffer_bitdump_custom((A), 0, 0, false, true)

    // framebuffer or graphics functions that edit the framebuffer to perform
    // drawing. the user must call the ssd1322_i2c_display_update() function every
    // time they want to update the display on the screen.
    // this is useful since the user may want to update the framebuffer several
    // times to create a layered image (such as first draw bricks, then draw a
    // person, then clip scenes that are unnecessary) before performing a display
    // update.
    int ssd1322_framebuffer_draw_bricks(ssd1322_framebuffer_t *fbp);

    // the x,y coordinates are based on the screen widthxheight. color if true means
    // color the pixel that was in that x,y location. false will clear the pixel.
    // since the height and width are uint8_t the x,y are also the same. For a
    // screen size of 128x64, the below diagram should display how to think about
    // pixels and arrangement. Each pixel is a bit in the GDDRAM.
    //  (0,0)   x ---->    (127,0)
    //  y
    //   |
    //   V
    //  (63,0)  x ---->    (127,63)
    //
    // rotation_flag is either 1 for 90 degrees, 2 for 180 degrees, 3 for 270
    // degrees or 0 for 0 degrees/360 degrees or any other.

    int ssd1322_framebuffer_put_pixel_rotation(ssd1322_framebuffer_t *fbp,
        uint16_t x, uint16_t y, char color, uint8_t rotation_flag);
#define ssd1322_framebuffer_put_pixel(fbp,x,y,color) ssd1322_framebuffer_put_pixel_rotation((fbp),(x),(y),(color),0)

    // invert the color of the pixel at position (x,y). This is the same as getting
    // the pixel color using the ssd1322_framebuffer_get_pixel() call and changing
    // the color using the ssd1322_framebuffer_put_pixel() call, but twice as fast.
    int ssd1322_framebuffer_invert_pixel(ssd1322_framebuffer_t *fbp,
        uint16_t x, uint16_t y);
    // returns the value at the pixel. is 0 if pixel is clear, is 1 if the pixel is
    // colored and is -1 if the fbp pointer is bad or the pixel is not found
    int8_t ssd1322_framebuffer_get_pixel(ssd1322_framebuffer_t *fbp,
        uint16_t x, uint16_t y);

    typedef struct {
        enum {
            SSD1322_OPT_FONT_FILE,
            SSD1322_OPT_ROTATE_FONT,    // rotate the font rendering
            SSD1322_OPT_ROTATE_PIXEL    // rotate the pixel location: valid values are multiples of 90 for rotation_degrees
        } type;
        union {
            const char *font_file;      // NULL terminated path of the font file to use
            int16_t rotation_degrees;   // rotation angle in degrees, examples: 90, 180, 270, 360, -90, -180, 45 etc.
            void *ptr;                  // pointer to something in the future
        } value;
    } ssd1322_graphics_options_t;

    typedef struct {
        uint8_t top;
        uint8_t left;
        uint8_t bottom;
        uint8_t right;
    } ssd1322_framebuffer_box_t;

    // These functions are for pure ASCII text only.
    // returns 0 if the task succeeded and sets the bounding box pixel values
    // returns -1 if error occurred loading fonts or anything else
    // if the bounding_box is set, it returns the pixel coordinates of the drawing
    // of the font. This can be used by the developer to know where to draw the next string of text vertically or horizontally.
    // the box is always within the framebuffer height and width
    ssize_t ssd1322_framebuffer_draw_text(ssd1322_framebuffer_t *fbp,
        const char *str, size_t slen,
        uint16_t x, uint16_t y,
        ssd1322_fontface_t fontface, uint8_t font_size,
        ssd1322_framebuffer_box_t *bounding_box);
    // if you want to draw text with a custom font or rotate the text you need extra
    // options and this function allows you to do that. the above function is the
    // same as the below function with the options as NULL
    ssize_t ssd1322_framebuffer_draw_text_extra(ssd1322_framebuffer_t *fbp,
        const char *str, size_t slen,
        uint16_t x, uint16_t y,
        ssd1322_fontface_t fontface, uint8_t font_size,
        const ssd1322_graphics_options_t *opts, size_t num_opts, ssd1322_framebuffer_box_t *bounding_box);

    // draw a line using Bresenham's line algorithm. returns 0 on success and -1 on
    // failure. if the (x0,y0) or (x1,y1) coordinates are outside the width and
    // height of the screen, the coordinates get clipped automatically
    // coordinates look like below as shown for a 128x64 size screen
    //  (0,0)   x ---->    (127,0)
    //  y
    //   |
    //   V
    //  (63,0)  x ---->    (127,63)
    int ssd1322_framebuffer_draw_line(ssd1322_framebuffer_t *fbp,
        uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, char color);

    // Draw an anti - aliased line based on Xiaolin Wu's line algorithm (https://en.wikipedia.org/wiki/Xiaolin_Wu%27s_line_algorithm)
    int ssd1322_framebuffer_draw_aa_line(ssd1322_framebuffer_t* fbp, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, bool invert);

    /// <summary>
    /// Copies a bitmap in memory to the framebuffer for display, 1 bit per pixel
    /// </summary>
    /// <param name="bitmap"></param>
    /// <param name="framebuffer"></param>
    void copy_bitmap_to_framebuffer(uint8_t *bitmap, ssd1322_framebuffer_t *framebuffer);
#ifdef __cplusplus
}  // extern "C"
#endif

#endif /* __LIB_SSD1322_GFX_H__ */
