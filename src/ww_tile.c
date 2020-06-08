#include "ww_tile.h"
#include "ww_filesys.h"
#include "ww_config.h"

#include <png.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

typedef uint8_t ww_tile_t[WW_TILE_SIZE][WW_TILE_SIZE];

static ww_tile_t ww_tiles[WW_MAX_TILES];

// https://lospec.com/palette-list/aap-64
static uint32_t const ww_tile_palette[] = {
    0x000000, // Palette entry 0 is transparent
    0x060608, 0x141013, 0x3b1725, 0x73172d, 0xb4202a, 0xdf3e23, 0xfa6a0a, 0xf9a31b,
    0xffd541, 0xfffc40, 0xd6f264, 0x9cdb43, 0x59c135, 0x14a02e, 0x1a7a3e, 0x24523b,
    0x122020, 0x143464, 0x285cc4, 0x249fde, 0x20d6c7, 0xa6fcdb, 0xffffff, 0xfef3c0,
    0xfad6b8, 0xf5a097, 0xe86a73, 0xbc4a9b, 0x793a80, 0x403353, 0x242234, 0x221c1a,
    0x322b28, 0x71413b, 0xbb7547, 0xdba463, 0xf4d29c, 0xdae0ea, 0xb3b9d1, 0x8b93af,
    0x6d758d, 0x4a5462, 0x333941, 0x422433, 0x5b3138, 0x8e5252, 0xba756a, 0xe9b5a3,
    0xe3e6ff, 0xb9bffb, 0x849be4, 0x588dbe, 0x477d85, 0x23674e, 0x328464, 0x5daf8d,
    0x92dcba, 0xcdf7e2, 0xe4d2aa, 0xc7b08b, 0xa08662, 0x796755, 0x5a4e44, 0x423934
};

int ww_tile_init(void) {
    return 0;
}

void ww_tile_destroy(void) {
}

static void ww_tile_png_error(png_structp const png, png_const_charp const error) {
    (void)png;
    (void)error;
}

typedef struct {
    uint8_t const* buffer;
    size_t remaining;
}
ww_tile_reader_t;

static void ww_tile_png_read(png_structp const png, png_bytep const buffer, size_t const size) {
    ww_tile_reader_t* const reader = png_get_io_ptr(png);
    size_t const to_read = size <= reader->remaining ? size : reader->remaining;

    memcpy(buffer, reader->buffer, to_read);
    reader->buffer += to_read;
    reader->remaining -= to_read;
}

int ww_tile_load(uint8_t const tile_num, char const* const file_path) {
    if (tile_num >= WW_MAX_TILES) {
        return -1;
    }

    void* buffer;
    ww_tile_reader_t reader;
    reader.buffer = buffer = ww_filesys_read_all(file_path, &reader.remaining);

    if (reader.buffer == NULL) {
        return -1;
    }

    png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, ww_tile_png_error, ww_tile_png_error);

    if (png == NULL) {
error1:
        free(buffer);
        return -1;
    }

    png_infop info = png_create_info_struct(png);

    if (info == NULL) {
        png_destroy_read_struct(&png, NULL, NULL);
        goto error1;
    }

    if (setjmp(png_jmpbuf(png))) {
error2:
        png_destroy_read_struct(&png, &info, NULL);
        goto error1;
    }

    png_set_read_fn(png, &reader, ww_tile_png_read);
    png_read_info(png, info);

    png_uint_32 width = png_get_image_width(png, info);
    png_uint_32 height = png_get_image_height(png, info);

    if (width != WW_TILE_SIZE || height != WW_TILE_SIZE) {
        goto error2;
    }

    // Make sure we always get RGBA pixels.
    png_byte color_type = png_get_color_type(png, info);
    png_byte bit_depth = png_get_bit_depth(png, info);

    if (bit_depth == 16) {
        png_set_strip_16(png);
    }

    if (color_type == PNG_COLOR_TYPE_PALETTE) {
        png_set_palette_to_rgb(png);
    }

    if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8) {
        png_set_expand_gray_1_2_4_to_8(png);
    }

    // Transform transparent color to alpha.
    if (png_get_valid(png, info, PNG_INFO_tRNS)) {
        png_set_tRNS_to_alpha(png);
    }

    // Set alpha to opaque if non-existent.
    if (color_type == PNG_COLOR_TYPE_RGB ||
        color_type == PNG_COLOR_TYPE_GRAY ||
        color_type == PNG_COLOR_TYPE_PALETTE) {

        png_set_filler(png, 0xff, PNG_FILLER_AFTER);
    }

    if (color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA) {
        png_set_gray_to_rgb(png);
    }

    png_read_update_info(png, info);

    uint32_t const pixels[WW_TILE_SIZE][WW_TILE_SIZE];
    png_bytep rows[WW_TILE_SIZE];
    
    for (size_t i = 0; i < WW_TILE_SIZE; i++) {
        rows[i] = (png_bytep)pixels[i];
    }
    
    png_read_image(png, rows);
    free((void*)buffer);
    png_destroy_read_struct(&png, &info, NULL);

    ww_tile_t* tile = ww_tiles + tile_num;

    for (unsigned y = 0; y < WW_TILE_SIZE; y++) {
        for (unsigned x = 0; x < WW_TILE_SIZE; x++) {
            uint32_t const color = pixels[y][x];

            if (((color >> 24) & 0xff) == 0) {
                // Transparent.
                (*tile)[y][x] = 0;
                continue;
            }

            float const r = color & 0xff;
            float const g = (color >> 8) & 0xff;
            float const b = (color >> 16) & 0xff;

            float dr = r - ((ww_tile_palette[1] >> 16) & 0xff);
            float dg = g - ((ww_tile_palette[1] >> 8) & 0xff);
            float db = b - (ww_tile_palette[1] & 0xff);

            float min_distance = dr * dr + dg * dg + db * db;
            size_t min_index = 1;

            for (size_t i = 2; i < sizeof(ww_tile_palette) / sizeof(ww_tile_palette[0]); i++) {
                dr = r - ((ww_tile_palette[i] >> 16) & 0xff);
                dg = g - ((ww_tile_palette[i] >> 8) & 0xff);
                db = b - (ww_tile_palette[i] & 0xff);

                float distance = dr * dr + dg * dg + db * db;

                if (distance < min_distance) {
                    min_distance = distance;
                    min_index = i;
                }
            }

            (*tile)[y][x] = (uint8_t)min_index;
        }
    }

    return 0;
}

void ww_tile_clear(ww_screen_t* const screen) {
    memset(&screen->collision, 0, sizeof(screen->collision));
}

void ww_tile_blit(ww_screen_t* const screen, uint8_t const tile_num, int x0, int y0) {
    int width = WW_TILE_SIZE;
    int ox = 0;

    if (x0 < 0) {
        width += x0;
        ox -= x0;
        x0 = 0;
    }
    else if (x0 + WW_TILE_SIZE > WW_SCREEN_WIDTH) {
        width -= x0 + WW_TILE_SIZE - WW_SCREEN_WIDTH;
    }

    if (width <= 0) {
        return;
    }

    int height = WW_TILE_SIZE;
    int oy = 0;

    if (y0 < 0) {
        height += y0;
        oy -= y0;
        y0 = 0;
    }
    else if (y0 + WW_TILE_SIZE > WW_SCREEN_HEIGHT) {
        height -= y0 + WW_TILE_SIZE - WW_SCREEN_HEIGHT;
    }

    if (height <= 0) {
        return;
    }

    size_t const pitch = screen->canvas.pitch / 4;
    uint32_t* pixels = screen->canvas.pixels + y0 * pitch + x0;
    ww_tile_t* const tile = ww_tiles + tile_num;

    //uint32_t const tile_bit = UINT32_C(1) << tile_num;
    //uint32_t collided = 0;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++, pixels++) {
            uint8_t const index = (*tile)[oy + y][ox + x];

            if (index != 0) {
                *pixels = ww_tile_palette[index];

                //uint32_t const collisions = screen->collision.tiles[y0 + y][x0 + x] | tile_bit;
                //collided |= screen->collision.tiles[y0 + y][x0 + x] = collisions;
            }
        }

        pixels += pitch - width;
    }
}

int ww_tile_collided(ww_screen_t* screen, uint8_t tile1_num, uint8_t tile2_num) {
    uint32_t const tile1_bit = UINT32_C(1) << tile1_num;
    return (screen->collision.collided[tile2_num] & tile1_bit) != 0;
}
