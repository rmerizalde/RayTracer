#include <assert.h>
#include "math/math.h"
#include "scene/scene.h"

Cone::Cone(F64 bottomRadius) //: mCosAngle(cos(angle))
{
	mAnchor.set(0.0, 0.0, 0.0);
	// Calculate angle
	Point3D axis = Point3D(0.0, -5, 0.0) - mAnchor;
	Point3D rightEdge = Point3D(bottomRadius, -5, 0.0) - mAnchor;
	axis.normalize();
	rightEdge.normalize();
	mCosAngle = dot(axis, rightEdge);
	mDirection.set(0, -1, 0);
	mDirection.normalize();
	mTopPlane = NULL;
	mBottomPlane = NULL;
}

/*Cone::Cone(F64 angle, F64 height) : mAngle(angle)
{
	mAnchor.set(0.0, 0.0, 0.0);
	mDirection.set(0, -1, 0);
	mDirection.normalize();
	Plane* plane = new Plane(Point3D(0.0, -height, 0.0), Point3D(0.0, -1.0, 0.0));
	addCutPlane(plane);
	plane = new Plane(mAnchor, Point3D(0.0, 1.0, 0.0));
	addCutPlane(plane);
}*/

Cone::Cone(F64 bottomRadius, F64 height)
{
	mAnchor.set(0.0, 0.0, 0.0);
	// Calculate angle
	Point3D axis = Point3D(0.0, -height, 0.0) - mAnchor;
	Point3D rightEdge = Point3D(bottomRadius, -height, 0.0) - mAnchor;
	axis.normalize();
	rightEdge.normalize();
	mCosAngle = dot(axis, rightEdge);
	mHeight = height;
	mDirection.set(0, -1, 0);
	mDirection.normalize();
	mBottomPlane = new Plane(Point3D(0.0, -height, 0.0), Point3D(0.0, -1.0, 0.0));
	addCutPlane(mBottomPlane);
	mTopPlane = new Plane(mAnchor, Point3D(0.0, 1.0, 0.0));
	addCutPlane(mTopPlane);
}

PointUV Cone::getUV(const Point3D &point, const Point3D &normal) const
{
	assert(getTexture());
	const Point3D &N = normal;
	F64 d = dot(point - mAnchor, mDirection);
	F64 v = d / mHeight;

	const Point3D &North = getNorth();
	const Point3D &G = getGreenwich();

	//F64 u = acos(dot(N, G)) / PI2;
	d = dot(North, point - mAnchor);
	Point3D m = point - (North * d);
	Point3D M = m - mAnchor;
	M.normalize();
	F64 u = acos(dot(M, G)) / PI2;

	Point3D planeNormal;
   cross(G, mDirection, &planeNormal);
	F64 D = -dot(planeNormal, mAnchor);
   F64 val = dot(point, planeNormal) + D;
	
	if (val < 0.0f)
		u = 1 - u;

	return PointUV(u, v);
}


Point3D Cone::getNormal(const Point3D &point) const
{
	Point3D H = point - mAnchor;
	//F64 d = H.length() / cos(mAngle);
	F64 d = H.length() / mCosAngle;
	Point3D m = mAnchor + mDirection * d;

	if (dot(point - mAnchor, mDirection) < -EPSILON)
		m = mAnchor + mDirection * -d;
	
	Point3D normal = (point - m);
	normal.normalize();
	return normal;
}

SceneObject::IntersectResult Cone::intersect(const Ray& ray, F64 &distance, IntersectionList *list) const
{
	const Point3D &S = ray.getOrigin();
	const Point3D &V = ray.getDirection();
	const Point3D &P = mAnchor;
	const Point3D &Q = mDirection;
	F64 cos2 = mCosAngle * mCosAngle;

	F64 dQV = dot(Q, V);
	F64 dQP = dot(Q, P);
	F64 dQS = dot(Q, S);
	
	F64 a = (dQV * dQV) - (dot(V, V) * cos2);
	F64 b = 2 * (dQS * dQV - dQV * dQP - dot(S, V) * cos2 + dot(V, P) * cos2);
	F64 c = (dQS * dQS) + (dQP * dQP) - (dQS * dQP * 2) - (dot(S, S) * cos2) - (dot(P, P) * cos2) + (dot(S, P) * cos2 * 2);
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

void Cone::perturbNormal(Point3D &normal, const U32 i, const U32 j) const
{
	const BumpMap *bumpMap = getBumpMap();
	Point3D U;
	cross(normal, getNorth(), &U);
	const Point3D &V = mDirection; 
	F32 k1 =  (bumpMap->getHeight(i-1, j) - bumpMap->getHeight(i+1, j)) / 2;
	F32 k2 =  (bumpMap->getHeight(i, j-1) - bumpMap->getHeight(i, j+1)) / 2;
	normal = normal + U * k2 + V * k1;
	normal.normalize();
}

void Cone::transform(const MatrixD &m)
{
	Point3D point = mAnchor + mDirection * 2;
	m.mul(point);
	m.mul(mAnchor);
	mDirection = point - mAnchor;
	mDirection.normalize();
	//Parent::transform(m);
	if (mTopPlane)
		mTopPlane->transform(m);
	if (mBottomPlane)
		mBottomPlane->transform(m);
	Parent::transform(m);
}