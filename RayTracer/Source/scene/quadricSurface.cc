#include "scene/scene.h"

QuadricSurface::QuadricSurface(F64 a, F64 b, F64 c, F64 d, F64 e, F64 f, F64 g, F64 h, F64 j, F64 k) :
    mTexturePoly(NULL),
    mWidthLeft(0.0f),
    mWidthRight(0.0f),
    mHeightTop(0.0f),
    mHeightBottom(0.0f)
{
    Point4D row;
    
    row.set(a, d / 2.0, f / 2.0, g / 2.0);
    mMatrix.setRow(0, row);
    row.set(d / 2.0, b, e / 2.0, h / 2.0);
    mMatrix.setRow(1, row);
    row.set(f / 2.0, e / 2.0, c, j / 2.0);
    mMatrix.setRow(2, row);
    row.set(g / 2.0, h / 2.0, j / 2.0, k);
    mMatrix.setRow(3, row);
    
    for (U32 i = 0; i < 4; i++)
        mCutPlanes[i] = NULL;
}

QuadricSurface::~QuadricSurface()
{
    if (mTexturePoly)
        delete mTexturePoly;
}

void QuadricSurface::setTexture(Texture *texture)
{
    Parent::setTexture(texture);
    if (mTexturePoly)
        mTexturePoly->setTexture(texture);
}

void QuadricSurface::setBumpMap(BumpMap *bumpMap)
{
    Parent::setBumpMap(bumpMap);
    if(mTexturePoly)
        mTexturePoly->setBumpMap(bumpMap);
}

void QuadricSurface::setNormalMap(NormalMap *normalMap)
{
    Parent::setNormalMap(normalMap);
    if(mTexturePoly)
        mTexturePoly->setNormalMap(normalMap);
}

void QuadricSurface::setOpacityMap(OpacityMap *opacityMap)
{
    Parent::setOpacityMap(opacityMap);
    if(mTexturePoly)
        mTexturePoly->setOpacityMap(opacityMap);
}

void QuadricSurface::setBounds(F32 widhtLeft, F32 widthRight, F32 heightBottom, F32 heightTop)
{
    mTexturePoly = new PolygonD();
    
    
    // Wrap the surface in a rectangle
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

PointUV QuadricSurface::getUV(const Point3D &point, const Point3D &normal) const
{
    if (mTexturePoly)
        return mTexturePoly->getUV(point, normal);
    return PointUV(0.0, 0.0); // Mmmm :(
}

Point3D QuadricSurface::getNormal(const Point3D &point) const
{
    F64 *m = mMatrix;
    F64 A = m[0];
    F64 B = m[5];
    F64 C = m[10];
    F64 D = m[1];
    F64 E = m[6];
    F64 F = m[2];
    F64 G = m[3];
    F64 H = m[7];
    F64 J = m[11];
    F64 K = m[15];
    
    //Point3D normal((dot(point, ADF) + G), (dot(point, DBE)) + H, (dot(point, FCE) + J));
    F64 xi = point.x;
    F64 yi = point.y;
    F64 zi = point.z;
    Point3D normal((A * xi + D * yi + F * zi) + G,
                   (D * xi + B * yi + E * zi) + H,
                   (F * xi + E * yi + C * zi) + J);
    normal.normalize();
    return normal;
}

SceneObject::IntersectResult QuadricSurface::intersect(const Ray& ray, F64 &distance, IntersectionList *list) const
{
    F64 *m = mMatrix;
    F64 A = m[0];
    F64 B = m[5];
    F64 C = m[10];
    F64 D = m[1];
    F64 E = m[6];
    F64 F = m[2];
    F64 G = m[3];
    F64 H = m[7];
    F64 J = m[11];
    F64 K = m[15];
    
    const Point3D &S = ray.getOrigin();
    const Point3D &V = ray.getDirection();
    F64 xe = S.x;
    F64 ye = S.y;
    F64 ze = S.z;
    F64 xd = V.x;
    F64 yd = V.y;
    F64 zd = V.z;
    
    F64 a = A * xd * xd + B * yd * yd + C * zd * zd +
    2 * (D * xd * yd + E * yd * zd + F * xd * zd);
    F64 b = 2 * (A * xe * xd + B * ye * yd + C * ze * zd +
                 D * xe * yd + D * ye * xd + E * ye * zd + E * ze * yd + F * ze * xd +
                 F * xe * zd + G * xd + H * yd + J * zd);
    F64 c = A * xe * xe + B * ye * ye + C * ze * ze + 2 * (D * xe * ye + E * ye * ze + F *ze * xe +
                                                           G * xe + H * ye + J * ze) + K;
    
    F64 disc = (b * b) - 4 * a * c;
    IntersectResult res = MISS;
    
    if (disc > EPSILON)
    {
        F64 sqrtD = sqrt(disc);
        F64 t1 = (-b - sqrtD) / (2 * a);
        F64 t2 = (-b + sqrtD) / (2 * a);
        
        processIntersection(ray, t1, res, distance, list);
        processIntersection(ray, t2, res, distance, list);
    }
    else if (isZero(disc))
    {
        F64 t = -b / (2 * a);
        processIntersection(ray, t, res, distance, list);
    }
    
    return res;
}

void QuadricSurface::perturbNormal(Point3D &normal, const U32 i, const U32 j) const
{
    if (mTexturePoly)
        perturbNormal(normal, i, j);
}

void QuadricSurface::transform(const MatrixD &m)
{
    MatrixD W = m;
    W.inverse();
    MatrixD Wt = W;
    Wt.transpose();
    MatrixD Q = mMatrix;
    
    mMatrix.mul(Wt, Q);
    mMatrix.mul(W);
}

void QuadricSurface::transformUV(const MatrixD &m)
{
}
