#include "ww_sprite.h"
#include "ww_tile.h"

void ww_sprite_clear(ww_screen_t* const screen) {
    screen->sprites.count = 0;
}

int ww_sprite_blit(ww_screen_t* const screen,
                   uint8_t const tile_num,
                   int x0,
                   int y0,
                   uint8_t const type) {

    if (screen->sprites.count == WW_MAX_SPRITES) {
        return -1;
    }

    screen->sprites.count++;
    return ww_tile_blit(screen, tile_num, x0, y0, type);
}
