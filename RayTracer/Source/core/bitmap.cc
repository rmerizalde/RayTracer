#include <assert.h>
#include "core/bitmap.h"
#include "core/file.h"

Bitmap::Bitmap() : internalFormat(RGBA),
    pBits(NULL),
    byteSize(0),
    width(0),
    height(0),
    bytesPerPixel(0)
{
}

Bitmap::~Bitmap()
{
	if (pBits != NULL)
		delete[] pBits;
}

void Bitmap::allocateBitmap(const U32 in_width, const U32 in_height, const BitmapFormat in_format)
{
	//--------------------------------------
	if (pBits != NULL)
		delete[] pBits;
    //-------------------------------------- Some debug checks...
    //U32 svByteSize = byteSize;
    //U8 *svBits = pBits;
    
    assert(in_width != 0 && in_height != 0);
    
    internalFormat = in_format;
	width          = in_width;
    height         = in_height;
	
    bytesPerPixel = 1;
    switch (internalFormat)
	{
        case RGBA:
            bytesPerPixel = 4;
			break;
        default:
            assert(false);
            break;
    }
    
    // Set up the mip levels, if necessary...
    U32 allocPixels = in_width * in_height * bytesPerPixel;
    
    // Set up the memory...
    byteSize = allocPixels;
    pBits    = new U8[byteSize];
	memset(pBits, 0xFF, byteSize);
    
    //if(svBits != NULL)
    //{
    //   dMemcpy(pBits, svBits, getMin(byteSize, svByteSize));
    //   delete[] svBits;
    //}
}

bool Bitmap::read(File &file)
{
	file.read(4, &width);
	width = convertBEndianToLEndian(width);
	file.read(4, &height);
	height = convertBEndianToLEndian(height);
	allocateBitmap(width, height, Bitmap::RGBA);
    file.read(byteSize, pBits);
	
	// change format from ARGB to RGBA
	U8 *begin = pBits;
	U8 *end = pBits + byteSize;
	for (U8 *itr = begin; itr != end; itr += 4)
	{
		U32 alpha = itr[0];
		itr[0] = itr[1];
		itr[1] = itr[2];
		itr[2] = itr[3];
		itr[3] = alpha;
	}	
	return true;
}