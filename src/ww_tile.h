#ifndef WW_TILE_H
#define WW_TILE_H

#include "ww_screen.h"

#include <stdint.h>
#include <stddef.h>

extern uint32_t const ww_tile_palette[];

int  ww_tile_init(void);
void ww_tile_destroy(void);

int  ww_tile_load(uint8_t tile_num, char const* file_path);

void ww_tile_clear(ww_screen_t* screen);
int  ww_tile_blit(ww_screen_t* screen, uint8_t tile_num, int x0, int y0, uint8_t type);

int  ww_tile_collided(ww_screen_t const* screen, uint8_t type1, uint8_t type2);

#endif // WW_TILE_H
