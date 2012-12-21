#ifndef _POINT_H_
#define _POINT_H_

#ifndef _PLATFORM_H_
#include "platform/platform.h"
#endif

#include <math.h>

class Point3D
{
public:
    F64 x;
    F64 y;
    F64 z;
    
    Point3D();
    Point3D(const Point3D&);
    Point3D(const F64 _x, const F64 _y, const F64 _z);
    
    void set(const F64 _x, const F64 _y, const F64 _z);
    
    F64 length() const;
    
    void normalize();
    
    Point3D  operator+(const Point3D&) const;
    Point3D  operator-(const Point3D&) const;
    Point3D& operator+=(const Point3D&);
    Point3D& operator-=(const Point3D&);
    
    Point3D  operator*(const F64) const;
    Point3D  operator/(const F64) const;
    Point3D& operator*=(const F64);
    Point3D& operator/=(const F64);
};

class Point4D
{
public:
    F64 x;
    F64 y;
    F64 z;
    F64 w;
    
    Point4D();
    Point4D(const Point4D&);
    Point4D(const F64 _x, const F64 _y, const F64 _z, const F64 _w);
    
    void set(const F64 _x, const F64 _y, const F64 _z, const F64 _w);
};

class PointUV
{
public:
    F64 u;
    F64 v;
    
    PointUV();
    PointUV(const PointUV&);
    PointUV(const F64 _u, const F64 _v);
    
    void set(const F64 _u, const F64 _v);
    
    PointUV  operator-(const PointUV&) const;
};

// Inlines Point3D

inline Point3D::Point3D()
{}


inline Point3D::Point3D(const Point3D& copy) : x(copy.x), y(copy.y), z(copy.z)
{}


inline Point3D::Point3D(const F64 _x, const F64 _y, const F64 _z) : x(_x), y(_y), z(_z)
{}

inline F64 Point3D::length() const
{
    return sqrt(x*x + y*y + z*z);
}

inline void Point3D::set(const F64 _x, const F64 _y, const F64 _z)
{
    x = _x;
    y = _y;
    z = _z;
}

inline void Point3D::normalize()
{
    F64 l = 1.0 / length();
    x *= l;
    y *= l;
    z *= l;
}

inline Point3D Point3D::operator+(const Point3D& add) const
{
    return Point3D(x + add.x, y + add.y, z + add.z);
}

inline Point3D Point3D::operator-(const Point3D& sub) const
{
    return Point3D(x - sub.x, y - sub.y, z - sub.z);
}

inline Point3D& Point3D::operator+=(const Point3D& add)
{
    x += add.x;
    y += add.y;
    z += add.z;
    return *this;
}

inline Point3D& Point3D::operator-=(const Point3D& sub)
{
    x -= sub.x;
    y -= sub.y;
    z -= sub.z;
    return *this;
}

inline Point3D Point3D::operator*(const F64 mul) const
{
    return Point3D(x * mul, y * mul, z * mul);
}

inline Point3D Point3D::operator/(const F64 div) const
{
    return Point3D(x / div, y / div, z / div);
}

inline Point3D& Point3D::operator*=(const F64 mul)
{
    x *= mul;
    y *= mul;
    z *= mul;
    return *this;
}

inline Point3D& Point3D::operator/=(const F64 div)
{
    x /= div;
    y /= div;
    z /= div;
    return *this;
}

// Inlines Point4D

inline Point4D::Point4D()
{}


inline Point4D::Point4D(const Point4D& copy) : x(copy.x), y(copy.y), z(copy.z), w(copy.w)
{}


inline Point4D::Point4D(const F64 _x, const F64 _y, const F64 _z, const F64 _w) : x(_x), y(_y), z(_z), w(_w)
{}

inline void Point4D::set(const F64 _x, const F64 _y, const F64 _z, const F64 _w)
{
    x = _x;
    y = _y;
    z = _z;
    w = _w;
}

inline PointUV::PointUV()
{
    //
}

inline PointUV::PointUV(const PointUV& copy)
{
    u = copy.u;
    v = copy.v;
}

inline PointUV::PointUV(const F64 _u, const F64 _v)
{
    u = _u;
    v = _v;
}

inline void PointUV::set(const F64 _u, const F64 _v)
{
    u = _u;
    v = _v;
}

inline PointUV  PointUV::operator-(const PointUV& sub) const
{
    return PointUV(u - sub.u, v - sub.v);
}

#endif