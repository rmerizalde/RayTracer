#include "scene/scene.h"

Plane::Plane(const Point3D &anchor, const Point3D &normal) : mAnchor(anchor), mNormal(normal)
{
	//
}

Point3D Plane::getAnchor() const
{
	return mAnchor;
}

Point3D Plane::getNormal() const
{
	return mNormal;
}

Point3D Plane::getNormal(const Point3D &point) const
{
	return getNormal();
}

void Plane::calculateDistance(const Ray& ray, F64 &distance)
{
	const Point3D &S = ray.getOrigin();
	const Point3D &V = ray.getDirection();
	const Point3D &P = mAnchor;
	const Point3D &N = mNormal;

	F64 D = -dot(N, P);
	F64 dNV = dot(N, V);
	distance = -(dot(N, S) + D) / dNV;
}

SceneObject::IntersectResult Plane::intersect(const Ray& ray, F64 &distance, IntersectionList *list) const
{
	const Point3D &S = ray.getOrigin();
	const Point3D &V = ray.getDirection();
	const Point3D &P = mAnchor;
	const Point3D &N = mNormal;

	F64 D = -dot(N, P);
	F64 dNV = dot(N, V);
	IntersectResult res = MISS;

	if (dNV > EPSILON || dNV < -EPSILON)
	{
		F64 t = -(dot(N, S) + D) / dNV;

		if (t > EPSILON && t < distance)
		{
			if (list)
				list->add(this, t);

			distance = t;
			res = HIT;
		}
	}
	return res;
}

void Plane::transform(const MatrixD &m)
{
	Point3D point = mAnchor + mNormal * 2;
	m.mul(point);
	m.mul(mAnchor);
	mNormal = point - mAnchor;
	mNormal.normalize();
}
