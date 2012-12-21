#include <assert.h>
#include "scene/scene.h"

PolygonD::PolygonD() : mPlane(NULL), mDropCoord(NONE)
{
	mMaxX = -F64_MAX;
	mMinX = F64_MAX;
	mMaxY = -F64_MAX;
	mMinY = F64_MAX;
}

PolygonD::~PolygonD()
{
	if (mPlane)
		delete mPlane;

	while (!mVertexList.empty())
	{		
		Point3D *vertex = mVertexList.back();
		mVertexList.pop_back();
		delete vertex;
	}

	while (!mUVVertexList.empty())
	{		
		PointUV *vertex = mUVVertexList.back();
		mUVVertexList.pop_back();
		delete vertex;
	}

}

// Fix me: move texture initialization somewhere else
void PolygonD::addVertex(const Point3D &vertex)
{
	assert(vertex.z == 0.0);
	mVertexList.push_back(new Point3D(vertex.x, vertex.y, vertex.z));
	mMaxX = max(mMaxX, vertex.x);
	mMinX = min(mMinX, vertex.x);
	mMaxY = max(mMaxY, vertex.y);
	mMinY = min(mMinY, vertex.y);
}

void PolygonD::preInitialize()
{
	Point3D p0(mMinX, mMinY, 0.0);
	Point3D p1(mMaxX, mMinY, 0.0);
	Point3D p3(mMinX, mMaxY, 0.0);

	U = p1 - p0;
	V = p3 - p0;
	mWidth = sqrt(dot(U, U));
	mHeight = sqrt(dot(V, V));
	V.normalize();
	U.normalize();
	mPoint0 = p0;
}

void PolygonD::initialize()
{
	calculatePlane();
	assert(mPlane);
	if (mPlane)
		project();	
}

bool PolygonD::isInitialized()
{ 
	return mPlane != NULL && mDropCoord != NONE; 
}

void PolygonD::calculatePlane()
{
	if (mVertexList.size() < 3)
		return;

	const Point3D *first = mVertexList.front();
	for (std::vector<Point3D*>::const_iterator walk = mVertexList.begin() + 1; walk != mVertexList.end() - 1; walk++)
	{
		Point3D* second = *walk;
		Point3D* third = *(walk + 1);
		Point3D v1 = *second - *first;
		Point3D v2 = *third - *first;
		Point3D normal;
		cross(v1, v2, &normal);
		
		if (normal.length() != 0)
		{
			normal.normalize();
			mPlane = new Plane(*first, normal);
			return;
		}
	}
	return;
}

void PolygonD::project()
{
	Point3D normal = mPlane->getNormal(); 
	F64 A = normal.x;
	F64 B = normal.y;
	F64 C = normal.z;
	F64 m = max(max(abs(A), abs(B)), abs(C));	
	mDropCoord = (abs(A) == m)? X : (abs(B) == m)? Y : Z;

	for (std::vector<Point3D*>::const_iterator walk = mVertexList.begin(); walk != mVertexList.end(); walk++)
	{
		Point3D *point = *walk;
		PointUV *uvPoint = new PointUV();

		projectPoint(*point, uvPoint);
		mUVVertexList.push_back(uvPoint);
	}
}

inline void PolygonD::projectPoint(const Point3D &point, PointUV *uvPoint) const
{
	switch(mDropCoord)
	{
	case X:
		uvPoint->set(point.y, point.z); break;
	case Y:
		uvPoint->set(point.x, point.z); break;
	case Z:
		uvPoint->set(point.x, point.y); break;
	default:
		assert(false); // Should not happen;
	}
}

PointUV PolygonD::getUV(const Point3D &point, const Point3D &normal) const
{
	Point3D D = point - mPoint0;
	F64 l1 = dot(D, U);
	F64 l2 = dot(D, V);
	PointUV p;
	
	p.u = l1 / mWidth;
	p.v = l2 / mHeight;

//	assert(p.u >= 0.0 && p.u <= 1.0);
//	assert(p.v >= 0.0 && p.v <= 1.0);
	
	return p;
}

Point3D PolygonD::getNormal(const Point3D &point) const
{
	return mPlane->getNormal();
}

// Translates the given point p to the origin. The origin is according the intersection point ip
static inline PointUV translate(const PointUV &p, const PointUV &ip)
{
	return p - ip;
}

static bool intersectsEdge(const PointUV &p1, const PointUV &p2)
{
	if (p1.v >= 0.0 && p2.v >= 0.0
		|| p1.v < 0.0 && p2.v < 0.0		
		|| p1.u < 0.0 && p2.u < 0.0)
	{
		return false;
	}

	// vertical line
   if (p1.u == p2.u && p1.u == 0.0)
		return (p1.v < 0.0) ^ (p2.v < 0.0);

	// horizontal line
	if (p1.v == p2.v && p1.v == 0.0)
      return (p1.u < 0.0) ^ (p2.u < 0.0);

   if ((p1.u >= 0.0 && p2.u >= 0.0) && ((p1.v < 0.0) ^ (p2.v < 0.0)))
		return true;

	F64 m = (p2.v - p1.v) / (p2.u - p1.u);
	// y = 0.0
	F64 x = p2.u - p2.v / m;
	return (x >= 0.0);
}

SceneObject::IntersectResult PolygonD::intersect(const Ray& ray, F64 &distance, IntersectionList *list) const
{
	F64 t = distance;
	SceneObject::IntersectResult intersectsPlane = mPlane->intersect(ray, t);

	if (intersectsPlane)
	{
		const Point3D &S = ray.getOrigin();
		const Point3D &V = ray.getDirection();
		PointUV ip;
		S32 n = 0;

		projectPoint(S + V * t, &ip);
		std::vector<PointUV*>::const_iterator walk = mUVVertexList.begin();
		for (; walk != mUVVertexList.end() - 1; )
		{
			if (intersectsEdge(translate(**walk, ip), translate(**(walk+1), ip)))
				n++;
			walk++;
		}

		if (intersectsEdge(translate(*mUVVertexList.back(), ip), translate(*mUVVertexList.front(), ip)))
			n++;

      if (n % 2 && isInsideCutPlane(ray, t))
		{
			if (list)
				list->add(this, t);
			distance = t;
			return HIT;
		}
	}
	return MISS;
}

void PolygonD::perturbNormal(Point3D &normal, const U32 i, const U32 j) const
{
	const BumpMap *bumpMap = getBumpMap();
	F32 k1 =  (bumpMap->getHeight(i-1, j) - bumpMap->getHeight(i+1, j)) / 2;
	F32 k2 =  (bumpMap->getHeight(i, j-1) - bumpMap->getHeight(i, j+1)) / 2;
	normal = normal + U * k2 * 8 + V * k1 * 8;
	normal.normalize();
}

void PolygonD::transform(const MatrixD &m)
{
   for (std::vector<Point3D*>::const_iterator walk = mVertexList.begin(); walk != mVertexList.end() ; walk++)
	{
		m.mul(**walk);
	}
	m.mul(mPoint0);
}

void PolygonD::transformUV(const MatrixD &m)
{
   Parent::transformUV(m);
	m.mul(U);
	m.mul(V);
}