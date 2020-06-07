#ifndef WW_BACKGRND_H
#define WW_BACKGRND_H

#include <stdint.h>

int  ww_backgrnd_init(void);
void ww_backgrnd_destroy(void);

int  ww_backgrnd_clear(unsigned player, uint8_t tile);
int  ww_backgrnd_set(unsigned player, unsigned x, unsigned y, uint8_t tile);

int  ww_backgrnd_scroll_left(unsigned player, uint8_t tile);
int  ww_backgrnd_scroll_right(unsigned player, uint8_t tile);
int  ww_backgrnd_scroll_up(unsigned player, uint8_t tile);
int  ww_backgrnd_scroll_down(unsigned player, uint8_t tile);

void ww_backgrnd_render(uint32_t* canvas, unsigned player, unsigned dx, unsigned dy);

#endif // WW_BACKGRND_H
