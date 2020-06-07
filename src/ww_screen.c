#include "ww_screen.h"
#include "ww_backgrnd.h"

#include <string.h>

int ww_screen_init(ww_screen_t* const screen, uint32_t* const pixels, size_t const pitch) {
    memset(screen, 0, sizeof(*screen));
    screen->canvas.pixels = pixels;
    screen->canvas.pitch = pitch;

    return 0;
}

void ww_screen_destroy(ww_screen_t* screen) {
    (void)screen;
}
