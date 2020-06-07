#include "ww_backgrnd.h"
#include "ww_tile.h"

#include <string.h>

void ww_backgrnd_clear(ww_screen_t* const screen, uint8_t const tile_num) {
    memset(screen->background.tiles, tile_num, sizeof(screen->background.tiles));
}

void ww_backgrnd_set(ww_screen_t* const screen,
                     unsigned const x,
                     unsigned const y,
                     uint8_t const tile) {

    screen->background.tiles[y][x] = tile;
}

void ww_backgrnd_scroll_left(ww_screen_t* const screen, uint8_t const tile_num) {
    uint8_t* const ptr = (uint8_t*)screen->background.tiles;
    memmove(ptr, ptr + 1, sizeof(screen->background.tiles) - 1);

    for (unsigned y = 0; y < WW_BACKGRND_HEIGHT; y++) {
        screen->background.tiles[y][WW_BACKGRND_WIDTH - 1] = tile_num;
    }
}

void ww_backgrnd_scroll_right(ww_screen_t* const screen, uint8_t const tile_num) {
    uint8_t* const ptr = (uint8_t*)screen->background.tiles;
    memmove(ptr + 1, ptr, sizeof(screen->background.tiles) - 1);

    for (unsigned y = 0; y < WW_BACKGRND_HEIGHT; y++) {
        screen->background.tiles[y][0] = tile_num;
    }
}

void ww_backgrnd_scroll_up(ww_screen_t* const screen, uint8_t const tile_num) {
    uint8_t* const ptr = (uint8_t*)screen->background.tiles;
    memmove(ptr, ptr + WW_BACKGRND_WIDTH, sizeof(screen->background.tiles) - WW_BACKGRND_WIDTH);

    for (unsigned x = 0; x < WW_BACKGRND_WIDTH; x++) {
        screen->background.tiles[WW_BACKGRND_HEIGHT - 1][x] = tile_num;
    }
}

void ww_backgrnd_scroll_down(ww_screen_t* const screen, uint8_t const tile_num) {
    uint8_t* const ptr = (uint8_t*)screen->background.tiles;
    memmove(ptr + WW_BACKGRND_WIDTH, ptr, sizeof(screen->background.tiles) - WW_BACKGRND_WIDTH);

    for (unsigned x = 0; x < WW_BACKGRND_WIDTH; x++) {
        screen->background.tiles[0][x] = tile_num;
    }
}

void ww_backgrnd_render(ww_screen_t* const screen, unsigned const dx, unsigned const dy) {
    int y0 = -(dy % WW_TILE_SIZE);

    for (unsigned y = 0; y < WW_BACKGRND_HEIGHT; y++, y0 += WW_TILE_SIZE) {
        int x0 = -(dx % WW_TILE_SIZE);

        for (unsigned x = 0; x < WW_BACKGRND_WIDTH; x++, x0 += WW_TILE_SIZE) {
            ww_tile_blit(screen, screen->background.tiles[y][x], x0, y0);
        }
    }
}
