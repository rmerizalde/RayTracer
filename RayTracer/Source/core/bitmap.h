#ifndef _BITMAP_H_
#define _BITMAP_H_

#ifndef _PLATFORM_H_
#include "platform/platform.h"
#endif

class File;

class Bitmap
{
public:
    enum BitmapFormat
    {
        RGBA = 0
    };
    
    Bitmap();
    ~Bitmap();
    
    bool read(File &file);
    
private:
    void allocateBitmap(const U32 in_width, const U32 in_height, const BitmapFormat in_format);
    
private:
    BitmapFormat internalFormat;
public:
    U8* pBits;            // Master bytes
    U32 byteSize;
    U32 width;            // Top level w/h
    U32 height;
    U32 bytesPerPixel;
};

#endif