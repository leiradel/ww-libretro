#ifndef WW_IMAGE_H
#define WW_IMAGE_H

#include <stdint.h>

void ww_image_init(void);
void ww_image_destroy(void);

void ww_image_blit(uint32_t* canvas, uint8_t tile, int x0, int y0);

#endif /* WW_IMAGE_H */
