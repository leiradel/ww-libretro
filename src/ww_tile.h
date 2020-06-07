#ifndef WW_TILE_H
#define WW_TILE_H

#include <stdint.h>

int  ww_tile_init(void);
void ww_tile_destroy(void);

int  ww_tile_load(uint8_t tile_num, char const* file_path);
void ww_tile_blit(uint32_t* canvas, uint8_t tile_num, int x0, int y0);

#endif // WW_TILE_H
