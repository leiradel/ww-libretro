#ifndef WW_SCREEN_H
#define WW_SCREEN_H

#include "ww_config.h"

#include <stdint.h>
#include <stddef.h>

#define WW_BACKGRND_WIDTH  (((WW_SCREEN_WIDTH + WW_TILE_SIZE - 1) / WW_TILE_SIZE) + 1)
#define WW_BACKGRND_HEIGHT (((WW_SCREEN_HEIGHT + WW_TILE_SIZE - 1) / WW_TILE_SIZE) + 1)

typedef struct {
    uint8_t tiles[WW_BACKGRND_HEIGHT][WW_BACKGRND_WIDTH];
}
ww_backgrnd_t;

typedef struct {
    uint8_t tiles[WW_SCREEN_HEIGHT][WW_SCREEN_WIDTH];
    uint32_t collided[WW_MAX_SPRITES];
    size_t count;
}
ww_sprites_t;

typedef struct {
    uint32_t* pixels;
    size_t pitch;
}
ww_canvas_t;

typedef struct {
    ww_backgrnd_t background;
    ww_sprites_t sprites;
    ww_canvas_t canvas;
}
ww_screen_t;

int  ww_screen_init(ww_screen_t* screen, uint32_t* pixels, size_t pitch);
void ww_screen_destroy(ww_screen_t* screen);

#endif // WW_SCREEN_H
