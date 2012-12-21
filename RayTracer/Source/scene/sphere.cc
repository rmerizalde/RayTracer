#include <assert.h>
#include "math/math.h"
#include "scene/scene.h"

Sphere::Sphere(F64 radius) : mRadius(radius)
{
	mCenter.set(0.0, 0.0, 0.0);
	mSquaredRadius = radius * radius;
}

PointUV Sphere::getUV(const Point3D &point, const Point3D &normal) const
{
	const Texture *texture = getTexture();
	assert(texture);
	Point3D N = normal;
	const Point3D &North = getNorth();
	const Point3D &G = getGreenwich();
	F64 d = dot(North, point - mCenter);
	Point3D m = point - (North * d);
	Point3D M = m - mCenter;
	M.normalize();
	F64 u = acos(dot(M, G)) / PI2;
	F64 v = acos(dot(North, N)) / PI;

	/*Point3D planeNormal;
	cross(G, North, &planeNormal);
	F64 D = -dot(planeNormal, point);*/

	Point3D planeNormal;
   cross(G, North, &planeNormal);
	F64 D = -dot(planeNormal, mCenter);
   F64 val = dot(point, planeNormal) + D;
	
	if (val < 0.0)
		u = 1 - u;

	return PointUV(u, v);
}

Point3D Sphere::getNormal(const Point3D &point) const
{
	return (point - mCenter) / mRadius;
}

SceneObject::IntersectResult Sphere::intersect(const Ray& ray, F64 &distance, IntersectionList *list) const
{
	const Point3D &S = ray.getOrigin();
	const Point3D &V = ray.getDirection();
	F64 b = dot(V * 2, (S - mCenter));
	F64 c = dot(S, S) - (dot(S, mCenter) * 2) + dot(mCenter, mCenter) - mSquaredRadius;
	F64 D = (b * b) - 4 * c;
	IntersectResult res = MISS;

	if (D > 0)
	{
		F64 sqrtD = sqrt(D);
		F64 t1 = (-b - sqrtD) / 2;
		F64 t2 = (-b + sqrtD) / 2;

		processIntersection(ray, t1, res, distance, list);		
		processIntersection(ray, t2, res, distance, list);
	}
	else if (isZero(D))
	{
		F64 t = -b / 2;
		processIntersection(ray, t, res, distance, list);
	}
	return res;
}

void Sphere::perturbNormal(Point3D &normal, const U32 i, const U32 j) const
{
	const BumpMap *bumpMap = getBumpMap();
	Point3D U;
	cross(normal, getNorth(), &U);
	Point3D V; 
	cross(normal, U, &V);
	F32 k1 =  (bumpMap->getHeight(i-1, j) - bumpMap->getHeight(i+1, j)) / 2;
	F32 k2 =  (bumpMap->getHeight(i, j-1) - bumpMap->getHeight(i, j+1)) / 2;
	normal = normal + U * k2 + V * k1;
	normal.normalize();
}

void Sphere::transform(const MatrixD &m)
{
	m.mul(mCenter);
	Parent::transform(m);
}