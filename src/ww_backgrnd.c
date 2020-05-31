#include "ww_backgrnd.h"
#include "ww_config.h"
#include "ww_image.h"

#include <string.h>

#define WW_BACKGRND_WIDTH  (((WW_SCREEN_WIDTH + WW_TILE_SIZE - 1) / WW_TILE_SIZE) + 1)
#define WW_BACKGRND_HEIGHT (((WW_SCREEN_HEIGHT + WW_TILE_SIZE - 1) / WW_TILE_SIZE) + 1)

static uint8_t ww_tiles[WW_MAX_PLAYERS][WW_BACKGRND_HEIGHT][WW_BACKGRND_WIDTH];

int ww_backgrnd_init(void) {
    for (unsigned player = 0; player < WW_MAX_PLAYERS; player++) {
        ww_backgrnd_clear(player, 0);
    }

    return 0;
}

void ww_backgrnd_destroy(void) {
}

int ww_backgrnd_clear(unsigned const player, uint8_t const tile) {
    if (player < WW_MAX_PLAYERS) {
        memset(ww_tiles, tile, sizeof(ww_tiles));
        return 0;
    }

    return -1;
}

int ww_backgrnd_set(unsigned const player, unsigned const x, unsigned const y, uint8_t const tile) {
    if (player < WW_MAX_PLAYERS &&  x < WW_BACKGRND_WIDTH && y < WW_BACKGRND_HEIGHT && tile < 32) {
        ww_tiles[player][y][x] = tile;
        return 0;
    }

    return -1;
}

int ww_backgrnd_scroll_left(unsigned const player, uint8_t const tile) {
    if (player < WW_MAX_PLAYERS) {
        uint8_t* const ptr = (uint8_t*)ww_tiles[player];
        memmove(ptr, ptr + 1, sizeof(ww_tiles) - 1);

        for (unsigned y = 0; y < WW_BACKGRND_HEIGHT; y++) {
            ww_tiles[player][y][WW_BACKGRND_WIDTH - 1] = tile;
        }

        return 0;
    }

    return -1;
}

int ww_backgrnd_scroll_right(unsigned const player, uint8_t const tile) {
    if (player < WW_MAX_PLAYERS) {
        uint8_t* const ptr = (uint8_t*)ww_tiles[player];
        memmove(ptr + 1, ptr, sizeof(ww_tiles) - 1);

        for (unsigned y = 0; y < WW_BACKGRND_HEIGHT; y++) {
            ww_tiles[player][y][0] = tile;
        }

        return 0;
    }

    return -1;
}

int ww_backgrnd_scroll_up(unsigned const player, uint8_t const tile) {
    if (player < WW_MAX_PLAYERS) {
        uint8_t* const ptr = (uint8_t*)ww_tiles[player];
        memmove(ptr, ptr + WW_BACKGRND_WIDTH, sizeof(ww_tiles) - WW_BACKGRND_WIDTH);

        for (unsigned x = 0; x < WW_BACKGRND_WIDTH; x++) {
            ww_tiles[player][WW_BACKGRND_HEIGHT - 1][x] = tile;
        }

        return 0;
    }

    return -1;
}

int ww_backgrnd_scroll_down(unsigned const player, uint8_t const tile) {
    if (player < WW_MAX_PLAYERS) {
        uint8_t* const ptr = (uint8_t*)ww_tiles[player];
        memmove(ptr + WW_BACKGRND_WIDTH, ptr, sizeof(ww_tiles) - WW_BACKGRND_WIDTH);

        for (unsigned x = 0; x < WW_BACKGRND_WIDTH; x++) {
            ww_tiles[player][0][x] = tile;
        }

        return 0;
    }

    return -1;
}

void ww_backgrnd_render(uint32_t* const canvas, unsigned const player, unsigned const dx, unsigned const dy) {
    if (player < WW_MAX_PLAYERS) {
        int y0 = -(dy % WW_TILE_SIZE);

        for (unsigned y = 0; y < WW_BACKGRND_HEIGHT; y++, y0 += WW_TILE_SIZE) {
            int x0 = -(dx % WW_TILE_SIZE);

            for (unsigned x = 0; x < WW_BACKGRND_WIDTH; x++, x0 += WW_TILE_SIZE) {
                ww_image_blit(canvas, ww_tiles[player][y][x], x0, y0);
            }
        }
    }
}
