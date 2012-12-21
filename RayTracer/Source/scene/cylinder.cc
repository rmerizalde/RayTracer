#include <assert.h>
#include "math/math.h"
#include "scene/scene.h"

Cylinder::Cylinder(F64 radius) : mHeight(0.0), mRadius(radius)
{
	mAnchor.set(0.0, 0.0, 0.0);
	mDirection.set(0.0, -1.0, 0.0);
	mDirection.normalize();
	mSquaredRadius = radius * radius;
	mTopPlane = NULL;
	mBottomPlane = NULL;
}

Cylinder::Cylinder(const Point3D &anchor, const Point3D &direction, F64 radius) :
	mAnchor(anchor),	
	mDirection(direction),
	mHeight(0.0),
	mRadius(radius)
{
	mDirection.normalize();
	mSquaredRadius = radius * radius;
	mTopPlane = NULL;
	mBottomPlane = NULL;
}

Cylinder::Cylinder(F64 radius, F64 height) :
	mHeight(height),
	mRadius(radius)
{
	mAnchor.set(0.0, 0.0, 0.0);
	mDirection.set(0, -1, 0);
	mDirection.normalize();

	mSquaredRadius = radius * radius;
	mBottomPlane = new Plane(Point3D(0, mAnchor.y - height, 0), Point3D(0, -1, 0));
	addCutPlane(mBottomPlane);
	mTopPlane = new Plane(mAnchor, Point3D(0, 1, 0));
	addCutPlane(mTopPlane);
}

void Cylinder::createTop(Disk **top)
{
	if (mHeight)
	{
		Point3D anchor = Point3D(0, mAnchor.y - mHeight, 0);
		Point3D normal = Point3D(0, -1, 0);
		*top = new Disk(anchor, normal, mRadius);
	}
}

void Cylinder::createBottom(Disk **bottom)
{
	if (mHeight)
	{
		Point3D normal = Point3D(0, 1, 0);
		*bottom = new Disk(mAnchor, normal, mRadius);
	}
}

PointUV Cylinder::getUV(const Point3D &point, const Point3D &normal) const
{
	assert(getTexture());
	Point3D N = normal;
	Point3D G = getGreenwich();
	F64 d = dot(point - mAnchor, mDirection);
	F64 u = acos(dot(N, G)) / PI2;
	F64 v = 1 - d / mHeight;

	Point3D planeNormal;
   cross(G, mDirection, &planeNormal);
	F64 D = -dot(planeNormal, mAnchor);
   F64 val = dot(point, planeNormal) + D;
	
	if (val < 0.0f)
		u = 1 - u;

	return PointUV(u, v);
}


Point3D Cylinder::getNormal(const Point3D &point) const
{
	const Point3D &Q = mDirection;
	Point3D H = point - mAnchor;
	F64 d = dot(Q, H);	
	Point3D m = mAnchor + Q * d;
	
	return (point - m) / mRadius;
}

SceneObject::IntersectResult Cylinder::intersect(const Ray& ray, F64 &distance, IntersectionList *list) const
{
	const Point3D &S = ray.getOrigin();
	const Point3D &V = ray.getDirection();
	const Point3D &P = mAnchor;
	const Point3D &Q = mDirection;
	
	F64 dVQ = dot(V, Q);
	F64 dQQ = dot(Q, Q);
	F64 dPQ = dot(P, Q);
	F64 dSQ = dot(S, Q);

	F64 a = dot(V, V) - 2 * (dVQ * dVQ) + dQQ * (dVQ * dVQ);
	F64 b = 2 * dPQ * dVQ - 2 * dSQ * dVQ - 2 * dVQ * dPQ * dQQ + 2 * dVQ * dSQ * dQQ
			+ 2 * dot(S, V) - 2 * dot(V, P) - 2 * dVQ * dSQ + 2 * dVQ * dPQ;
	F64 c = dQQ * (dPQ * dPQ - 2 * dPQ * dSQ + dSQ * dSQ)
			+ dot(S, S) - 2 * dot(S, P) + dot(P, P)
			+ 2 * dPQ * (dSQ - dPQ) - 2 * dSQ * (dSQ - dPQ)
			- mSquaredRadius;
	F64 D = (b * b) - 4 * a * c;
	IntersectResult res = MISS;

	if (D > 0.0)
	{
		F64 sqrtD = sqrt(D);
		F64 t1 = (-b - sqrtD) / (2 * a);
		F64 t2 = (-b + sqrtD) / (2 * a);

		processIntersection(ray, t1, res, distance, list);
		processIntersection(ray, t2, res, distance, list);

		/*if (t1 > EPSILON && t1 < distance && isInsideCutPlane(ray, t1))
		{
			distance = t1;
			res = HIT;
		}
		
		if (t2 > EPSILON && t2 < distance && isInsideCutPlane(ray, t2))
		{
			distance = t2;
			res = HIT;
		}*/
	}
	else if (isZero(D))
	{
		F64 t = -b / (2 * a);
		processIntersection(ray, t, res, distance, list);
		/*if (t > 0 && t < distance && isInsideCutPlane(ray, t))
		{
			distance = t;
			res = HIT;
		}*/
	}

	return res;
}

void Cylinder::perturbNormal(Point3D &normal, const U32 i, const U32 j) const
{
	const BumpMap *bumpMap = getBumpMap();
	const Point3D &Q = mDirection;
	Point3D U;
	cross(Q, normal, &U);
	const Point3D &V = Q; 
	F32 k1 =  (bumpMap->getHeight(i-1, j) - bumpMap->getHeight(i+1, j)) / 2;
	F32 k2 =  (bumpMap->getHeight(i, j-1) - bumpMap->getHeight(i, j+1)) / 2;
	normal = normal + U * k2 + V * k1;
	normal.normalize();
}

void Cylinder::transform(const MatrixD &m)
{
	Point3D point = mAnchor + mDirection * 2;
	m.mul(point);
	m.mul(mAnchor);
	mDirection = point - mAnchor;
	mDirection.normalize();
	
	if (mTopPlane)
		mTopPlane->transform(m);
	if (mBottomPlane)
		mBottomPlane->transform(m);
	Parent::transform(m);
}