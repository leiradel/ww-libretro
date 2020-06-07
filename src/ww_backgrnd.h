#ifndef WW_BACKGRND_H
#define WW_BACKGRND_H

#include "ww_config.h"

#include <stdint.h>
#include <stddef.h>

#define WW_BACKGRND_WIDTH  (((WW_SCREEN_WIDTH + WW_TILE_SIZE - 1) / WW_TILE_SIZE) + 1)
#define WW_BACKGRND_HEIGHT (((WW_SCREEN_HEIGHT + WW_TILE_SIZE - 1) / WW_TILE_SIZE) + 1)

typedef struct {
    uint8_t tiles[WW_BACKGRND_HEIGHT][WW_BACKGRND_WIDTH];
}
ww_backgrnd_t;

int  ww_backgrnd_init(ww_backgrnd_t* bgnd);
void ww_backgrnd_destroy(ww_backgrnd_t* bgnd);

void ww_backgrnd_clear(ww_backgrnd_t* bgnd, uint8_t tile);
void ww_backgrnd_set(ww_backgrnd_t* bgnd, unsigned x, unsigned y, uint8_t tile);

void ww_backgrnd_scroll_left(ww_backgrnd_t* bgnd, uint8_t tile);
void ww_backgrnd_scroll_right(ww_backgrnd_t* bgnd, uint8_t tile);
void ww_backgrnd_scroll_up(ww_backgrnd_t* bgnd, uint8_t tile);
void ww_backgrnd_scroll_down(ww_backgrnd_t* bgnd, uint8_t tile);

void ww_backgrnd_render(uint32_t* canvas, size_t pitch, ww_backgrnd_t const* bgnd, unsigned dx, unsigned dy);

#endif // WW_BACKGRND_H
