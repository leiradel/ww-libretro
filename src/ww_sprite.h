#ifndef WW_SPRITE_H
#define WW_SPRITE_H

#include "ww_screen.h"

#include <stdint.h>

void ww_sprite_clear(ww_screen_t* screen);
int  ww_sprite_blit(ww_screen_t* screen, uint8_t tile_num, int x0, int y0);

#endif // WW_TILE_H
