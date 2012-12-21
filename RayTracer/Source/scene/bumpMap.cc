#include "math/math.h"
#include "scene/scene.h"
#include <assert.h>

BumpMap::BumpMap() : mHeights(NULL)
{
}

BumpMap::~BumpMap()
{
	if (mHeights)
		free(mHeights);
}

void BumpMap::init(Texture *texture, F32 minHeight, F32 maxHeight)
{
	if (mHeights)
		delete mHeights;
    
	mMinHeight = minHeight;
	mMaxHeight = maxHeight;
	width = texture->bitmap.width;
	height = texture->bitmap.height;
	U32 size = width * height * sizeof(F32);
	mHeights = new F32[size];
	memset(mHeights, 0, size);
	ColorF c;
	F32 delta = mMaxHeight - mMinHeight;
    
	for (U32 i = 0; i < width; i++)
	{
		for (U32 j = 0; j < height; j++)
		{
			texture->getTexel(i, j, c);
			F32 r = c.red;
			F32 g = c.green;
			F32 b = c.blue;
            
			F32 factor = c.red + c.green + c.blue;
			F32 height = (delta / 3.0f) * factor + mMinHeight;
			assert(height >= mMinHeight);
			assert(height <= mMaxHeight);
			setHeight(i, j, height);
		}
	}
}
