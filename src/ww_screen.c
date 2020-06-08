#include "ww_screen.h"
#include "ww_backgrnd.h"
#include "ww_sprite.h"
#include "ww_tile.h"

#include <string.h>

int ww_screen_init(ww_screen_t* const screen, uint16_t* const pixels, size_t const pitch) {
    memset(screen, 0, sizeof(*screen));
    screen->canvas.pixels = pixels;
    screen->canvas.pitch = pitch;

    return 0;
}

void ww_screen_destroy(ww_screen_t* const screen) {
    (void)screen;
}

void ww_screen_clear(ww_screen_t* const screen) {
    ww_backgrnd_clear(screen, 0);
    ww_tile_clear(screen);
    ww_sprite_clear(screen);
}
