#if !defined( BITMAP_H )

struct Bitmap
{
    uint32 width;
    uint32 height;
    uint32 * bits;
};

#define BITMAP_H
#endif
