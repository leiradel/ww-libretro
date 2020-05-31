#include "ww_image.h"
#include "ww_config.h"

#include <stdlib.h>
#include <string.h>

typedef uint8_t ww_image[16][16];

static ww_image ww_images[32];

#if 0
void ww_image_init(void) {
  tslt_x = tslt_y = 0;
}

void ww_image_translate( int x, int y )
{
  tslt_x = x;
  tslt_y = y;
}

int ww_image_create( ww_image_t* image, const ww_pixelsrc_t* pixelsrc, int check_transp, uint16_t transparent )
{
  size_t size;
  const void* data = ww_pixelsrc_encode( &size, pixelsrc, check_transp, transparent );
  
  if ( data )
  {
    union
    {
      const void*     v;
      const uint8_t*  u8;
      const uint16_t* u16;
      const uint32_t* u32;
    }
    ptr;
    
    image->rle = ptr.v = data;
    
    image->width  = *ptr.u16++;
    image->height = *ptr.u16++;
    image->used   = *ptr.u32++;
    image->rows   = ptr.u32;
    
    return 0;
  }
  
  free( (void*)data );
  return -1;
}

void ww_image_blit_nobg( const ww_image_t* image, int x, int y )
{
  x += tslt_x;
  y += tslt_y;
  
  int x0 = 0;
  int y0 = 0;
  int x1 = image->width;
  int y1 = image->height;
  
  int width, height;
  uint16_t* fb = ww_backgrnd_fb( &width, &height );
  
  if ( x < -WW_BACKGRND_MARGIN )
  {
    x   = -x;
    x0 += x & ~( WW_BACKGRND_MARGIN - 1 );
    x1 -= x & ~( WW_BACKGRND_MARGIN - 1 );
    x   = -( x & ( WW_BACKGRND_MARGIN - 1 ) );
  }
  
  if ( x + x1 > width )
  {
    x1 -= x + x1 - width;
  }
  
  if ( y < 0 )
  {
    y0 -= y;
    y1 += y;
    y   = 0;
  }
  
  if ( y + y1 > height )
  {
    y1 -= y + y1 - height;
  }
  
  if ( y1 > 0 && x1 > 0 )
  {
    int       pitch  = width + WW_BACKGRND_MARGIN;
    uint16_t* save   = fb + y * pitch + x;
    
#if WW_BACKGRND_MARGIN == 0
    int runcnt = 1;
#else
    int runcnt = ( x1 + ( WW_BACKGRND_MARGIN - 1 ) ) / WW_BACKGRND_MARGIN;
    x0         = ( x0 + ( WW_BACKGRND_MARGIN - 1 ) ) / WW_BACKGRND_MARGIN;
#endif
    
    do
    {
      const uint16_t* restrict rle = (uint16_t*)( (const uint8_t*)image->rows + image->rows[ y0++ ] );
      rle += rle[ x0 ];
      
      uint16_t* restrict dest = save;
      save += pitch;
      
      int runs = runcnt;
      
      do
      {
        /* number of rle encodings in this chunk */
        int numrle = *rle++;
        
        do
        {
          /* decode */
          int code   = *rle++;
          int count  = code & 0x1fff;
          code >>= 13;
          
          uint32_t c1, c2;
          
          switch ( code & 7 )
          {
          case 4: /* opaque */
            memcpy( (void*)dest, (void*)rle, count * 2 );
            rle += count, dest += count;
            break;
          
          case 0: /* transparent */
            dest += count;
            break;
          
          case 2: /* 50% */
            do
            {
              c1      = *dest & 0xf7de;
              c2      = *rle++ & 0xf7de;
              *dest++ = ( c1 + c2 ) >> 1;
            }
            while ( --count );
            break;
          
          case 1: /* 25% */
            do
            {
              c1      = *dest & 0xe79c;
              c2      = *rle++ & 0xe79c;
              *dest++ = ( 3 * c1 + c2 ) >> 2;
            }
            while ( --count );
            break;
          
          case 3: /* 75% */
            do
            {
              c1      = *dest & 0xe79c;
              c2      = *rle++ & 0xe79c;
              *dest++ = ( c1 + 3 * c2 ) >> 2;
            }
            while ( --count );
            break;
          
          /* these are just to avoid gcc adding a cmp + ja */
          case 5:
            rle++;
          case 6:
            rle++;
          case 7:
            rle++;
            break;
          }
        }
        while ( --numrle );
      }
      while ( --runs );
    }
    while ( --y1 );
  }
}

uint16_t* ww_image_blit( const ww_image_t* image, int x, int y, uint16_t* bg_ )
{
  x += tslt_x;
  y += tslt_y;
  
  int x0 = 0;
  int y0 = 0;
  int x1 = image->width;
  int y1 = image->height;
  
  int width, height;
  uint16_t* fb = ww_backgrnd_fb( &width, &height );
  
  if ( x < -WW_BACKGRND_MARGIN )
  {
    x   = -x;
    x0 += x & ~( WW_BACKGRND_MARGIN - 1 );
    x1 -= x & ~( WW_BACKGRND_MARGIN - 1 );
    x   = -( x & ( WW_BACKGRND_MARGIN - 1 ) );
  }
  
  if ( x + x1 > width )
  {
    x1 -= x + x1 - width;
  }
  
  if ( y < 0 )
  {
    y0 -= y;
    y1 += y;
    y   = 0;
  }
  
  if ( y + y1 > height )
  {
    y1 -= y + y1 - height;
  }
  
  uint16_t* restrict bg = bg_;
  
  if ( y1 > 0 && x1 > 0 )
  {
    int       pitch  = width + WW_BACKGRND_MARGIN;
    uint16_t* save   = fb + y * pitch + x;
    
#if WW_BACKGRND_MARGIN == 0
    int runcnt = 1;
#else
    int runcnt = ( x1 + ( WW_BACKGRND_MARGIN - 1 ) ) / WW_BACKGRND_MARGIN;
    x0         = ( x0 + ( WW_BACKGRND_MARGIN - 1 ) ) / WW_BACKGRND_MARGIN;
#endif
    
    do
    {
      const uint16_t* restrict rle = (uint16_t*)( (const uint8_t*)image->rows + image->rows[ y0++ ] );
      rle += rle[ x0 ];
      
      uint16_t* restrict dest = save;
      save += pitch;
      
      int runs = runcnt;
      
      do
      {
        /* number of rle encodings in this chunk */
        int numrle = *rle++;
        
        do
        {
          /* decode */
          int code   = *rle++;
          int count  = code & 0x1fff;
          code >>= 13;
          
          uint32_t c1, c2;
          
          switch ( code & 7 )
          {
          case 4: /* opaque */
            memcpy( (void*)bg, (void*)dest, count * 2 );
            memcpy( (void*)dest, (void*)rle, count * 2 );
            rle += count, dest += count, bg += count;
            break;
          
          case 0: /* transparent */
            dest += count;
            break;
          
          case 2: /* 50% */
            do
            {
              c1      = *dest;
              *bg++   = c1;
              c1     &= 0xf7de;
              c2      = *rle++ & 0xf7de;
              *dest++ = ( c1 + c2 ) >> 1;
            }
            while ( --count );
            break;
          
          case 1: /* 25% */
            do
            {
              c1      = *dest;
              *bg++   = c1;
              c1     &= 0xe79c;
              c2      = *rle++ & 0xe79c;
              *dest++ = ( 3 * c1 + c2 ) >> 2;
            }
            while ( --count );
            break;
          
          case 3: /* 75% */
            do
            {
              c1      = *dest;
              *bg++   = c1;
              c1     &= 0xe79c;
              c2      = *rle++ & 0xe79c;
              *dest++ = ( c1 + 3 * c2 ) >> 2;
            }
            while ( --count );
            break;
          
          /* these are just to avoid gcc adding a cmp + ja */
          case 5:
            rle++;
          case 6:
            rle++;
          case 7:
            rle++;
            break;
          }
        }
        while ( --numrle );
      }
      while ( --runs );
    }
    while ( --y1 );
  }
  
  return bg;
}

void ww_image_unblit( const ww_image_t* image, int x, int y, const uint16_t* bg_ )
{
  x += tslt_x;
  y += tslt_y;
  
  int x0 = 0;
  int y0 = 0;
  int x1 = image->width;
  int y1 = image->height;
  
  int width, height;
  uint16_t* fb = ww_backgrnd_fb( &width, &height );
  
  if ( x < -WW_BACKGRND_MARGIN )
  {
    x   = -x;
    x0 += x & ~( WW_BACKGRND_MARGIN - 1 );
    x1 -= x & ~( WW_BACKGRND_MARGIN - 1 );
    x   = -( x & ( WW_BACKGRND_MARGIN - 1 ) );
  }
  
  if ( x + x1 > width )
  {
    x1 -= x + x1 - width;
  }
  
  if ( y < 0 )
  {
    y0 -= y;
    y1 += y;
    y   = 0;
  }
  
  if ( y + y1 > height )
  {
    y1 -= y + y1 - height;
  }
  
  const uint16_t* restrict bg = bg_;
  
  if ( y1 > 0 && x1 > 0 )
  {
    int       pitch  = width + WW_BACKGRND_MARGIN;
    uint16_t* save   = fb + y * pitch + x;
    
#if WW_BACKGRND_MARGIN == 0
    int runcnt = 1;
#else
    int runcnt = ( x1 + ( WW_BACKGRND_MARGIN - 1 ) ) / WW_BACKGRND_MARGIN;
    x0         = ( x0 + ( WW_BACKGRND_MARGIN - 1 ) ) / WW_BACKGRND_MARGIN;
#endif
    
    do
    {
      const uint16_t* restrict rle = (uint16_t*)( (const uint8_t*)image->rows + image->rows[ y0++ ] );
      rle += rle[ x0 ];
      
      uint16_t* restrict dest = save;
      save += pitch;
      
      int runs = runcnt;
      
      do
      {
        /* number of rle encodings in this chunk */
        int numrle = *rle++;
        
        do
        {
          /* decode */
          int code   = *rle++;
          int count  = code & 0x1fff;
          
          if ( code & 0xe000 )
          {
            memcpy( (void*)dest, (void*)bg, count * 2 );
            rle += count, dest += count, bg += count;
          }
          else
          {
            dest += count;
          }
        }
        while ( --numrle );
      }
      while ( --runs );
    }
    while ( --y1 );
  }
}
#endif
