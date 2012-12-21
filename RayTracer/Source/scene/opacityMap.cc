#include "scene/scene.h"

OpacityMap::OpacityMap() : mFlags(NULL)
{
}

OpacityMap::~OpacityMap()
{
	if (mFlags)
		free(mFlags);
}

void OpacityMap::init(Texture *texture, F32 tolerance)
{
	if (mFlags)
		free(mFlags);

	width = texture->bitmap.width;
	height = texture->bitmap.height;
	U32 size = width * height * sizeof(U8);
	mFlags = new U8[size];	
	memset(mFlags, 0, size);
	ColorF c;
	//Point3D p;

	for (U32 i = 0; i < width; i++)
	{
		for (U32 j = 0; j < height; j++)
		{
			texture->getTexel(i, j, c);
			setFlag(i, j, c.alpha > 0.0);
		}
	}
}
