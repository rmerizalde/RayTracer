#include "scene/scene.h"
#include <assert.h>

NormalMap::NormalMap() : mNormals(NULL)
{
}

NormalMap::~NormalMap()
{
    if (mNormals)
        free(mNormals);
}

void NormalMap::init(Texture *texture)
{
    if (mNormals)
        free(mNormals);
    
    width = texture->bitmap.width;
    height = texture->bitmap.height;
    U32 size = width * height * sizeof(Point3D);
    mNormals = new Point3D[size];
    memset(mNormals, 0, size);
    ColorF c;
    Point3D normal;
    
    MatrixD mat;
    F64 theta = 3.1415926535897932384626433832795;
    Point4D row;
    F64 cosTheta = cos(theta);
    F64 sinTheta = sin(theta);
    
    row.set(cosTheta, -sinTheta, 0.0, 0.0);
    mat.setRow(0, row);
    row.set(sinTheta, cosTheta, 0.0, 0.0);
    mat.setRow(1, row);
    row.set(0.0, 0.0, 1.0, 0.0);
    mat.setRow(2, row);
    row.set(0.0, 0.0, 0.0, 1.0);
    mat.setRow(3, row);
    U8 red;
    U8 green;
    U8 blue;
    U8 alpha;
    
    F64 inv255 = 1.0 / 255.0;
    for (U32 i = 0; i < width; i++)
    {
        for (U32 j = 0; j < height; j++)
        {
            texture->getTexel(i, j, &red, &green, &blue, &alpha);
            normal.x = (red * 2.0 * inv255) -1;
            normal.y = (green * 2.0 * inv255) -1;
            normal.z = (blue * 2.0 * inv255) -1;
            
            F64 length = normal.length();
            //normal *= -1;
            //mat.mul(normal);
            //normal.normalize();
            //assert(isZero(normal.length() - 1));
            setNormal(i, j, normal);
        }
    }
}
