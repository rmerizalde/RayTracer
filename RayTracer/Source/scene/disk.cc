#include <assert.h>
#include "math/math.h"
#include "scene/scene.h"

Disk::Disk(F64 radius, bool anti) :
    mPlane(Point3D(0.0, 0.0, 0.0), Point3D(0.0, 0.0, -1.0)),
    mRadius(radius),
    mTexturePoly(NULL),
    mWidthLeft(0.0f),
    mWidthRight(0.0f),
    mHeightTop(0.0f),
    mHeightBottom(0.0f),
    mAnti(anti)
{
    mSquaredRadius = radius * radius;
    
    for (U32 i = 0; i < 4; i++)
        mCutPlanes[i] = NULL;
}

Disk::Disk(const Point3D &anchor, const Point3D &normal, F64 radius, bool anti) :
    mPlane(Plane(anchor, normal)),
    mRadius(radius),
    mTexturePoly(NULL),
    mWidthLeft(0.0f),
    mWidthRight(0.0f),
    mHeightTop(0.0f),
    mHeightBottom(0.0f),
    mAnti(anti)
{
    mSquaredRadius = radius * radius;
    
    for (U32 i = 0; i < 4; i++)
        mCutPlanes[i] = NULL;
}

Disk::~Disk()
{
    if (mTexturePoly)
        delete mTexturePoly;
}

void Disk::setTexture(Texture *texture)
{
    Parent::setTexture(texture);
    if (mTexturePoly)
        mTexturePoly->setTexture(texture);
}

void Disk::setBumpMap(BumpMap *bumpMap)
{
    Parent::setBumpMap(bumpMap);
    if(mTexturePoly)
        mTexturePoly->setBumpMap(bumpMap);
}

void Disk::setNormalMap(NormalMap *normalMap)
{
    Parent::setNormalMap(normalMap);
    if(mTexturePoly)
        mTexturePoly->setNormalMap(normalMap);
}

void Disk::setOpacityMap(OpacityMap *opacityMap)
{
    Parent::setOpacityMap(opacityMap);
    if(mTexturePoly)
        mTexturePoly->setOpacityMap(opacityMap);
}

void Disk::setBounds(F32 widhtLeft, F32 widthRight, F32 heightBottom, F32 heightTop)
{
    mTexturePoly = new PolygonD();
    
    
    // Wrap the disk in a rectangle
    Point3D p0(-widhtLeft, -heightBottom, 0.0);
    Point3D p1(widthRight, -heightBottom, 0.0);
    Point3D p3(widthRight, heightTop, 0.0);
    Point3D p2(-widhtLeft, heightTop, 0.0);
    
    mTexturePoly->addVertex(p0);
    mTexturePoly->addVertex(p1);
    mTexturePoly->addVertex(p2);
    mTexturePoly->addVertex(p3);
    mTexturePoly->preInitialize();
    mWidthLeft = widhtLeft;
    mWidthRight = widthRight;
    mHeightTop = heightTop;
    mHeightBottom = heightBottom;
    
    mCutPlanes[0] = new Plane(Point3D(0.0, heightTop, 0.0), Point3D(0.0, 1.0, 0.0));
    mCutPlanes[1] = new Plane(Point3D(-widhtLeft, 0.0, 0.0), Point3D(-1.0, 0.0, 0.0));
    mCutPlanes[2] = new Plane(Point3D(widthRight, 0.0, 0.0), Point3D(1.0, 0.0, 0.0));
    mCutPlanes[3] = new Plane(Point3D(0.0, -heightBottom, 0.0), Point3D(0.0, -1.0, 0.0));
    
    for (U32 i = 0; i < 4; i++)
        if (mCutPlanes[i])
            addCutPlane(mCutPlanes[i]);
}

PointUV Disk::getUV(const Point3D &point, const Point3D &normal) const
{
    return mTexturePoly->getUV(point, normal);
}

Point3D Disk::getNormal(const Point3D &point) const
{
    return mPlane.getNormal();
}

SceneObject::IntersectResult Disk::intersect(const Ray& ray, F64 &distance, IntersectionList *list) const
{
    F64 t = distance;
    SceneObject::IntersectResult intersectsPlane = mPlane.intersect(ray, t);
    
    if (intersectsPlane)
    {
        const Point3D &S = ray.getOrigin();
        const Point3D &V = ray.getDirection();
        const Point3D &C = mPlane.getAnchor();
        Point3D ip = S + V * t;
        F64 f1 = pow(ip.x - C.x, 2) + pow(ip.y - C.y, 2) + pow(ip.z - C.z, 2) - mSquaredRadius;
        F64 f2 = pow(ip.x - C.x, 2) + pow(ip.y - C.y, 2) + pow(ip.z - C.z, 2) - mSquaredRadius;
        bool intersects = (mAnti)? f1 >= EPSILON : f1 <= EPSILON /*&& f2 > EPSILON*/;
        
        if (intersects && isInsideCutPlane(ray, t))
        {
            if (list)
                list->add(this, t);
            distance = t;
            return HIT;
        }
    }
    return MISS;
}

void Disk::perturbNormal(Point3D &normal, const U32 i, const U32 j) const
{
    mTexturePoly->perturbNormal(normal, i, j);
}

void Disk::transform(const MatrixD &m)
{
    mPlane.transform(m);
    for (U32 i = 0; i < 4; i++)
        if (mCutPlanes[i])
            mCutPlanes[i]->transform(m);
    if (mTexturePoly)
        mTexturePoly->transform(m);
}

void Disk::transformUV(const MatrixD &m)
{
    if (mTexturePoly)
        mTexturePoly->transformUV(m);
}