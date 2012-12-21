#ifndef _MATHFN_H_
#define _MATHFN_H_

//--------------------------------------
#ifndef _POINT_H_
#include "math/point.h"
#endif

inline void cross(const Point3D &v1, const Point3D &v2, Point3D *res)
{
    res->x = (v1.y * v2.z) - (v1.z * v2.y);
    res->y = (v1.z * v2.x) - (v1.x * v2.z);
    res->z = (v1.x * v2.y) - (v1.y * v2.x);
}

inline F64 dot(const Point3D &p1, const Point3D &p2)
{
    return (p1.x * p2.x + p1.y * p2.y + p1.z * p2.z);
}

#endif