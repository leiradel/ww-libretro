#ifndef WW_BACKGRND_H
#define WW_BACKGRND_H

#include "ww_screen.h"

#include <stdint.h>

void ww_backgrnd_clear(ww_screen_t* screen, uint8_t tile_num);
void ww_backgrnd_set(ww_screen_t* screen, unsigned x, unsigned y, uint8_t tile_num);

void ww_backgrnd_scroll_left(ww_screen_t* screen, uint8_t tile_num);
void ww_backgrnd_scroll_right(ww_screen_t* screen, uint8_t tile_num);
void ww_backgrnd_scroll_up(ww_screen_t* screen, uint8_t tile_num);
void ww_backgrnd_scroll_down(ww_screen_t* screen, uint8_t tile_num);

void ww_backgrnd_render(ww_screen_t* screen, unsigned dx, unsigned dy);

#endif // WW_BACKGRND_H
