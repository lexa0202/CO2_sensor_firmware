#include "degree.h"

static const uint8_t degreeBitmap[] =
{
    0x30, 0x78, 0xCC, 0xCC, 0x78, 0x30, 
};

const Bitmap_t BitmapDegree6 =
{
    .width = 6,
    .height = 6,
    .bitmap = degreeBitmap
};
