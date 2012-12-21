#ifndef _SCENE_H_
#define _SCENE_H_

#include <vector>

#ifndef _PLATFORM_H_
   #include "platform/platform.h"
#endif

#ifndef _BITMAP_H_
#include "core/bitmap.h"
#endif

#ifndef _COLOR_H_
#include "core/color.h"
#endif

#ifndef _LIGHT_H_
#include "scene/light.h"
#endif

#include "math/math.h"

class Bitmap;
class Ray;
class Plane;
class PolygonD;
class IntersectionList;

class Texture
{
public:
	void getTexel(U32 i, U32 j, ColorF &color) const;
   void getTexel(U32 i, U32 j, U8* red, U8* blue, U8* green, U8* alpha) const;

	Bitmap bitmap;
	U32 hTile;
	U32 vTile;
	U32 hTileSize;
	U32 vTileSize;
};

class Map
{
public:
	U32 height;
	U32 width;
	U32 hTile;
	U32 vTile;
	U32 hTileSize;
	U32 vTileSize;
};

class BumpMap : public Map
{
public:
	
	BumpMap();
	virtual ~BumpMap();

	void init(Texture *texture, F32 minWidth, F32 maxHeight);
	F32 getHeight(U32 i, U32 j) const;
	void setHeight(U32 i, U32 j, F32 height);

private:
	F32 mMinHeight;
	F32 mMaxHeight;	
	F32 *mHeights;	
};

class NormalMap : public Map
{
public:
	
	NormalMap();
	virtual ~NormalMap();

	void init(Texture *texture);
	const Point3D& getNormal(U32 i, U32 j) const;
	void setNormal(U32 i, U32 j, const Point3D &normal);

private:
	Point3D *mNormals;	
};

class OpacityMap : public Map
{
public:
	OpacityMap();
	virtual ~OpacityMap();

	void init(Texture *texture, F32 tolerance);
	bool getFlag(U32 i, U32 j) const;
	void setFlag(U32 i, U32 j, bool flag);

private:
	U8* mFlags;
};

class Material
{
public:
	Material()
	{
		ambientIntensity = 0.2f;
		diffuseColor.set(0.8f, 0.8f, 0.8f);
		diffusseCoefficient = 0.45f;
		shininess = 0.2f;
		specularReflectionExponent = 3.0f;
		diffusiveness = 1.0f;
		reflectiveness = 0.0f;
		transparency = 0.0f;
		translucency = 0.0f;
      refractionIndex = 1.0f;
	}

	F32 ambientIntensity;
	ColorF diffuseColor;	
	F32 diffusseCoefficient;
	ColorF specularColor;
	F32 shininess;
	F32 specularReflectionExponent;
	F32 diffusiveness;
	F32 reflectiveness;
	F32 transparency;
	F32 translucency;
   F32 refractionIndex;
};


class SceneObject
{
public:
	enum IntersectResult { MISS, HIT };	

   SceneObject() : mTexture(NULL), mBumpMap(NULL), mNormalMap(NULL), mOpacityMap(NULL)
	{
		mNorth.set(0.0, -1.0, 0.0);
		mGreenwich.set(0.0, 0.0, -1.0);
	}

	SceneObject(const Material &material) : mTexture(NULL), mBumpMap(NULL), mNormalMap(NULL), mOpacityMap(NULL), mMaterial(material) {}
	~SceneObject();

	S32 getCutPlaneCount() { return (S32) mCutPlaneList.size(); }
	void addCutPlane(Plane *plane) { mCutPlaneList.push_back(plane); }
	Plane* getCutPlane(S32 index) { mCutPlaneList[index]; }
	
	void setMaterial(const Material &material) { mMaterial = material; }
	const Material &getMaterial() const { return mMaterial; }

	virtual void setTexture(Texture *texture) { mTexture = texture; }
	const Texture* getTexture() const { return mTexture; }
		
	virtual void setBumpMap(BumpMap *bumpMap) { mBumpMap = bumpMap; }
	const BumpMap* getBumpMap() const { return mBumpMap; }

	virtual void setNormalMap(NormalMap *normalMap) { mNormalMap = normalMap; }
	const NormalMap* getNormalMap() const { return mNormalMap; }

	virtual void setOpacityMap(OpacityMap *opacityMap) { mOpacityMap = opacityMap; }
	const OpacityMap* getOpacityMap() const { return mOpacityMap; }

	void setNorth(const Point3D &north) { mNorth = north; }
	const Point3D& getNorth() const { return mNorth; }

	void setGeenwich(const Point3D &greenwich) { mGreenwich = greenwich; }
	const Point3D& getGreenwich() const { return mGreenwich; }

	virtual PointUV getUV(const Point3D &point, const Point3D &normal) const { return PointUV(); }
	virtual Point3D getNormal(const Point3D &point) const = 0 ;
	virtual IntersectResult intersect(const Ray& ray, F64 &distance, IntersectionList *list = NULL) const = 0;
	virtual void perturbNormal(Point3D &normal, const U32 i, const U32 j) const { }

	virtual void transform(const MatrixD &m);	
	virtual void transformUV(const MatrixD &m);
protected:
	void processIntersection(const Ray& ray, F64 t, IntersectResult &res, F64 &distance, IntersectionList *list) const;
	bool isInsideCutPlane(const Ray& ray, F64 distance) const;
private:
	Material mMaterial;
	Texture *mTexture;
	BumpMap *mBumpMap;
   NormalMap *mNormalMap;
	OpacityMap *mOpacityMap;
	Point3D mNorth;
	Point3D mGreenwich;

	std::vector<Plane*> mCutPlaneList;
};

class Sphere : public SceneObject
{
public:
	typedef SceneObject Parent;

	Sphere(F64 radius);	

	PointUV getUV(const Point3D &point, const Point3D &normal) const;
	Point3D getNormal(const Point3D &point) const;
	IntersectResult intersect(const Ray& ray, F64 &distance, IntersectionList *list = NULL) const;
	void perturbNormal(Point3D &normal, const U32 i, const U32 j) const;

	void transform(const MatrixD &m);
private:
	Point3D mCenter;
	F64 mRadius;
	F64 mSquaredRadius;
};

class Plane : public SceneObject
{
public:
	typedef SceneObject Parent;

	Plane(const Point3D &anchor, const Point3D &normal);
	
	// Normal is the same for every point in the plane
	Point3D getAnchor() const;
	Point3D getNormal() const;
	Point3D getNormal(const Point3D &point) const;
	IntersectResult intersect(const Ray& ray, F64 &distance, IntersectionList *list = NULL) const;

	void transform(const MatrixD &m);

	void calculateDistance(const Ray& ray, F64 &distance);
private:
	Point3D mAnchor;
	Point3D mNormal;
};

class Disk : public SceneObject
{
public:
	typedef SceneObject Parent;

	Disk(F64 radius, bool anti = false);
	Disk(const Point3D &anchor, const Point3D &normal, F64 radius, bool anti = false);
	virtual ~Disk();

	void setTexture(Texture *texture);
	void setBumpMap(BumpMap *bumpMap);
   void setNormalMap(NormalMap *normalMap);
   void setOpacityMap(OpacityMap *opacityMap);   
	void setBounds(F32 widhtLeft, F32 widthRight, F32 heightBottom, F32 heightTop);

	virtual PointUV getUV(const Point3D &point, const Point3D &normal) const;
	virtual Point3D getNormal(const Point3D &point) const;
	virtual IntersectResult intersect(const Ray& ray, F64 &distance, IntersectionList *list = NULL) const;
	virtual void perturbNormal(Point3D &normal, const U32 i, const U32 j) const;

	void transform(const MatrixD &m);
	void transformUV(const MatrixD &m);
	
private:
	Plane mPlane;
	F64 mRadius;
	F64 mSquaredRadius;
	bool mAnti;
	// Use for anti-disk texturing only
	PolygonD *mTexturePoly;
	F32 mWidthLeft;
	F32 mWidthRight;
	F32 mHeightTop;
	F32 mHeightBottom;
	Plane* mCutPlanes[4];
};

class Cylinder : public SceneObject
{
public:
	typedef SceneObject Parent;

	Cylinder(F64 radius);
	Cylinder(const Point3D &anchor, const Point3D &direction, F64 radius);
	Cylinder(F64 radius, F64 height);

	void createTop(Disk **top);
	void createBottom(Disk **bottom);

	PointUV getUV(const Point3D &point, const Point3D &normal) const;
	Point3D getNormal(const Point3D &point) const;
	IntersectResult intersect(const Ray& ray, F64 &distance, IntersectionList *list = NULL) const;
	void perturbNormal(Point3D &normal, const U32 i, const U32 j) const;

	void transform(const MatrixD &m);

private:
	Point3D mAnchor;
	Point3D mDirection;
	F64 mHeight;
	F64 mRadius;
	F64 mSquaredRadius;
	Plane *mTopPlane;
	Plane *mBottomPlane;
};

class Cone : public SceneObject
{
public:
	typedef SceneObject Parent;

	Cone(F64 bottomRadius);
	Cone(F64 bottomRadius, F64 height);
	//Cone(F64 angle, F64 height);

	PointUV getUV(const Point3D &point, const Point3D &normal) const;
	Point3D getNormal(const Point3D &point) const;
	IntersectResult intersect(const Ray& ray, F64 &distance, IntersectionList *list = NULL) const;
	void perturbNormal(Point3D &normal, const U32 i, const U32 j) const;

	void transform(const MatrixD &m);
private:
	Point3D mAnchor;
	Point3D mDirection;
	F64 mHeight;
	//F64 mAngle;
	F64 mCosAngle;
	Plane *mTopPlane;
	Plane *mBottomPlane;
};

class QuadricSurface : public SceneObject
{
public:
	typedef SceneObject Parent;

   enum {A = 0, B, C, D, E, F, G, H, I, J, K};

   QuadricSurface(F64 a, F64 b, F64 c, F64 d, F64 e, F64 f, F64 g, F64 h, F64 j, F64 k);
   ~QuadricSurface();

	void setTexture(Texture *texture);
	void setBumpMap(BumpMap *bumpMap);
   void setNormalMap(NormalMap *normalMap);
   void setOpacityMap(OpacityMap *opacityMap);   
	void setBounds(F32 widhtLeft, F32 widthRight, F32 heightBottom, F32 heightTop);
   
   PointUV getUV(const Point3D &point, const Point3D &normal) const;
	Point3D getNormal(const Point3D &point) const;
	IntersectResult intersect(const Ray& ray, F64 &distance, IntersectionList *list = NULL) const;
	void perturbNormal(Point3D &normal, const U32 i, const U32 j) const;

	void transform(const MatrixD &m);
	void transformUV(const MatrixD &m);
private:
   MatrixD mMatrix;
	PolygonD *mTexturePoly;
	F32 mWidthLeft;
	F32 mWidthRight;
	F32 mHeightTop;
	F32 mHeightBottom;
	Plane* mCutPlanes[4];
};
     
class PolygonD : public SceneObject
{
public:
	typedef SceneObject Parent;

	PolygonD();

	virtual ~PolygonD();

	void addVertex(const Point3D &vertex);
	
	void preInitialize();
	void initialize();
	bool isInitialized(); 

	PointUV getUV(const Point3D &point, const Point3D &normal) const;
	Point3D getNormal(const Point3D &point) const;
	IntersectResult intersect(const Ray& ray, F64 &distance, IntersectionList *list = NULL) const;
	void perturbNormal(Point3D &normal, const U32 i, const U32 j) const;

	void transform(const MatrixD &m);
	void transformUV(const MatrixD &m);
private:
	void calculatePlane();
	void project();
	void projectPoint(const Point3D &point, PointUV *uvPoint) const;

private:
	enum DropCoord { NONE, X, Y, Z } mDropCoord;

	std::vector<Point3D*> mVertexList;
	std::vector<PointUV*> mUVVertexList;
	Plane *mPlane;
	// Used for texture mapping
	F64 mMaxX;
	F64 mMinX;
	F64 mMaxY;
	F64 mMinY;
	Point3D mPoint0;
	Point3D U;
	Point3D V;
	F64 mWidth;
	F64 mHeight;
};

class Triangle : public SceneObject
{
public:
   Triangle(Point3D *vertexTable, U32 p0Index, U32 p1Index, U32 p2Index) :
      mPlane(NULL),
      mVertexTable(vertexTable),
      mP0Index(p0Index),
      mP1Index(p1Index),
      mP2Index(p2Index)
   {
      init();
   }

   virtual ~Triangle()
   {
      if (mPlane) delete mPlane;
   }

	virtual PointUV getUV(const Point3D &point, const Point3D &normal) const;
	virtual Point3D getNormal(const Point3D &point) const;
	virtual void perturbNormal(Point3D &normal, const U32 i, const U32 j) const;
   virtual IntersectResult intersect(const Ray& ray, F64 &distance, IntersectionList *list = NULL) const;

private:
   void init();

private:
   Plane*   mPlane;
   Point3D* mVertexTable;
   U32      mP0Index;
   U32      mP1Index;
   U32      mP2Index;   
   Point3D  mQ1;
   Point3D  mQ2;
   F64      mDotQ1Q1;
   F64      mDotQ2Q2;
   F64      mDotQ1Q2;
   F64      mScalar;
};

class IntersectionList
{
public:
	class IntersectionListNode
	{
	public:
		friend class IntersectionList;

		const SceneObject* getObject() { return mObj; }
		F64 getDistance() { return mDistance; }
	
		IntersectionListNode* getNext() { return mNext; }

	private:
		IntersectionListNode * mNext;
		const SceneObject *mObj;
		F64 mDistance;
	};

	IntersectionList() : mFirst(NULL) {};
	~IntersectionList();

	IntersectionListNode* getFirst() { return mFirst; }		
	void add(const SceneObject *obj, F64 distance);	
	void pop();
	void clear();
	
private:
	IntersectionListNode *mFirst;
};

class Scene
{
public:
	Scene() 
   {
      transformationCount = 0;
      polygonCount        = 0;
      cutPlaneCount       = 0;
      cylinderCount       = 0;
      diskCount           = 0;
      sphereCount         = 0;
      quadricCount        = 0;
   }
	virtual ~Scene();

	void addLight(PointLight *light) { mLightList.push_back(light); }
	const PointLight* getLight(S32 index) { return mLightList[index]; }
	size_t getLightCount() { return mLightList.size(); }

	void addObject(SceneObject *object) { mObjList.push_back(object); }
	
	const SceneObject* findClosestIntersection(const Ray &ray, Point3D &intersection, Point3D &normal, PointUV &uv, F64 &distance);
	void findIntersections(const Ray &ray, IntersectionList &list);

	void load(const char* filename);

	void setViewpoint(const Point3D &viewpoint) { mViewpoint = viewpoint; }
	const Point3D& getViewpoint() { return mViewpoint; }
public:
   S32 transformationCount;
   S32 polygonCount;
   S32 cutPlaneCount;
   S32 cylinderCount;
   S32 diskCount;
   S32 sphereCount;
   S32 coneCount;
   S32 quadricCount;

private:
	std::vector<SceneObject*> mObjList;
	std::vector<PointLight*> mLightList;
	Point3D mViewpoint;
};

// Inlines

// Fix me
inline void SceneObject::processIntersection(const Ray& ray, F64 t, IntersectResult &res, F64 &distance, IntersectionList *list) const
{
	if (t > EPSILON && isInsideCutPlane(ray, t))
	{
		if (list) list->add(this, t);

		if (t < distance)
		{
			distance = t;
			res = HIT;
		}
	}
}

// Texture inlines

inline void Texture::getTexel(U32 i, U32 j, ColorF &color) const
{
	U8 *pBits = bitmap.pBits;
	U8 *texel = pBits + (j * (bitmap.width * 4)) + (i * 4);	
	color.set(texel[0] * INV255, texel[1] * INV255, texel[2] * INV255, texel[3] * INV255);
}

inline void Texture::getTexel(U32 i, U32 j, U8* red, U8* blue, U8* green, U8* alpha) const
{
	U8 *pBits = bitmap.pBits;
	U8 *texel = pBits + (j * (bitmap.width * 4)) + (i * 4);	
   *red   = texel[0];
   *green = texel[1];
   *blue  = texel[2];
   *alpha = texel[3];
}


// BumpMap inlines

inline F32 BumpMap::getHeight(U32 i, U32 j) const
{
	if (i < 0)
		i = 0;
	if (i >= width)
		i = width - 1;
	if (j < 0)
		j = 0;
	if (j >= height)
		j = height - 1;
	//return *(mHeights + (j * (width * 4)) + (i * 4));
	return *(mHeights + j * width + i);
}

inline void BumpMap::setHeight(U32 i, U32 j, F32 height)
{
	*(mHeights + j * width + i) = height;
}

// OpacityMap inlines

inline bool OpacityMap::getFlag(U32 i, U32 j) const
{
	return *(mFlags + j * width + i) != 0;
}

inline void OpacityMap::setFlag(U32 i, U32 j, bool flag)
{
	*(mFlags + j * width + i) = (flag)? 1 : 0;
}

// NormalMap inlines

inline const Point3D& NormalMap::getNormal(U32 i, U32 j) const
{
	return *(mNormals + j * width + i);
}

inline void NormalMap::setNormal(U32 i, U32 j, const Point3D &normal)
{
   *(mNormals + j * width + i) = normal;
}



#endif