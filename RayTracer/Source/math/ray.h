#ifndef _RAY_H_
#define _RAY_H_

class Point3D;

class Ray
{
private:
    Point3D mOrigin;
    Point3D mDirection;
public:
    
    Ray(const Point3D &origin, const Point3D &direction) : mOrigin(origin), mDirection(direction)
    {}
    
    const Point3D& getOrigin() const { return mOrigin; }
    const Point3D& getDirection() const { return mDirection; }
};

#endif