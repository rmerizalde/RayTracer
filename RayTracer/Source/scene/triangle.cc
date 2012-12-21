#include "scene/scene.h"

void Triangle::init()
{
   mQ1      = mVertexTable[mP1Index] - mVertexTable[mP0Index];
   mQ2      = mVertexTable[mP2Index] - mVertexTable[mP0Index];
   mDotQ1Q1 = dot(mQ1, mQ1);
   mDotQ2Q2 = dot(mQ2, mQ2);
   mDotQ1Q2 = -dot(mQ1, mQ2);
   mScalar  = 1 / (mDotQ1Q1 * mDotQ1Q2 - pow(mDotQ1Q2, 2));

   if (mPlane == NULL)
   {
      Point3D normal;
      cross(mQ1, mQ2, &normal);
      normal.normalize();
      mPlane = new Plane(mVertexTable[mP0Index], normal);
   }
}

PointUV Triangle::getUV(const Point3D &point, const Point3D &normal) const
{
   return PointUV();
}

Point3D Triangle::getNormal(const Point3D &point) const
{
   return mPlane->getNormal();
}

void Triangle::perturbNormal(Point3D &normal, const U32 i, const U32 j) const
{
}

SceneObject::IntersectResult Triangle::intersect(const Ray& ray, F64& distance, IntersectionList* list) const
{
	F64 t = distance;
	SceneObject::IntersectResult intersectsPlane = mPlane->intersect(ray, t);

	if (intersectsPlane)
	{
		const Point3D &S = ray.getOrigin();
		const Point3D &V = ray.getDirection();
		Point3D P = S + V * t;
      Point3D R = P - mVertexTable[mP0Index];
      F64 dotRQ1 = dot(R, mQ1);
      F64 dotRQ2 = dot(R, mQ2);
      F64 w1     = mDotQ2Q2 * dotRQ1 + mDotQ1Q2 * dotRQ2;
      F64 w2     = mDotQ1Q2 * dotRQ1 + mDotQ1Q1 * dotRQ2;
      F64 w0     = 1 - w1 - w2;
      
      if (w0 >= 0.0 && w1 >= 0.0 && w2 >= 0.0)
      {
			if (list) list->add(this, t);
			distance = t;
			return HIT;

      }
   }

   return MISS;
}
