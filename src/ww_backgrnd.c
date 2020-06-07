#include "ww_backgrnd.h"
#include "ww_tile.h"

#include <string.h>

int ww_backgrnd_init(ww_backgrnd_t* const bgnd) {
    ww_backgrnd_clear(bgnd, 0);
    return 0;
}

void ww_backgrnd_destroy(ww_backgrnd_t* const bgnd) {
    (void)bgnd;
}

void ww_backgrnd_clear(ww_backgrnd_t* const bgnd, uint8_t const tile) {
    memset(bgnd, tile, sizeof(*bgnd));
}

void ww_backgrnd_set(ww_backgrnd_t* bgnd, unsigned const x, unsigned const y, uint8_t const tile) {
    bgnd->tiles[y][x] = tile;
}

void ww_backgrnd_scroll_left(ww_backgrnd_t* const bgnd, uint8_t const tile) {
    uint8_t* const ptr = (uint8_t*)bgnd->tiles;
    memmove(ptr, ptr + 1, sizeof(*bgnd) - 1);

    for (unsigned y = 0; y < WW_BACKGRND_HEIGHT; y++) {
        bgnd->tiles[y][WW_BACKGRND_WIDTH - 1] = tile;
    }
}

void ww_backgrnd_scroll_right(ww_backgrnd_t* const bgnd, uint8_t const tile) {
    uint8_t* const ptr = (uint8_t*)bgnd->tiles;
    memmove(ptr + 1, ptr, sizeof(*bgnd) - 1);

    for (unsigned y = 0; y < WW_BACKGRND_HEIGHT; y++) {
        bgnd->tiles[y][0] = tile;
    }
}

void ww_backgrnd_scroll_up(ww_backgrnd_t* const bgnd, uint8_t const tile) {
    uint8_t* const ptr = (uint8_t*)bgnd->tiles;
    memmove(ptr, ptr + WW_BACKGRND_WIDTH, sizeof(*bgnd) - WW_BACKGRND_WIDTH);

    for (unsigned x = 0; x < WW_BACKGRND_WIDTH; x++) {
        bgnd->tiles[WW_BACKGRND_HEIGHT - 1][x] = tile;
    }
}

void ww_backgrnd_scroll_down(ww_backgrnd_t* const bgnd, uint8_t const tile) {
    uint8_t* const ptr = (uint8_t*)bgnd->tiles;
    memmove(ptr + WW_BACKGRND_WIDTH, ptr, sizeof(*bgnd) - WW_BACKGRND_WIDTH);

    for (unsigned x = 0; x < WW_BACKGRND_WIDTH; x++) {
        bgnd->tiles[0][x] = tile;
    }
}

void ww_backgrnd_render(ww_backgrnd_t const* const bgnd,
                        uint16_t* const canvas,
                        size_t const pitch,
                        unsigned const dx,
                        unsigned const dy) {

    int y0 = -(dy % WW_TILE_SIZE);

    for (unsigned y = 0; y < WW_BACKGRND_HEIGHT; y++, y0 += WW_TILE_SIZE) {
        int x0 = -(dx % WW_TILE_SIZE);

        for (unsigned x = 0; x < WW_BACKGRND_WIDTH; x++, x0 += WW_TILE_SIZE) {
            ww_tile_blit(canvas, pitch, bgnd->tiles[y][x], x0, y0);
        }
    }
}
