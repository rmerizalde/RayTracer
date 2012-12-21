#include "platform/platform.h"
#include "X3DTK/X3D/scenegraph.h"
#include "X3DTK/memreleaser.h"
#include "X3DTK/kernel.h"

#include "core/file.h"
#include "math/math.h"
#include "scene/scene.h"
#include <assert.h>

using namespace X3DTK;

SceneObject::~SceneObject()
{
	while (!mCutPlaneList.empty())
	{		
		Plane *plane = mCutPlaneList.back();
		mCutPlaneList.pop_back();
		delete plane;
	}

	if (mTexture)
		delete mTexture;

	if (mBumpMap)
		delete mBumpMap;

   if (mNormalMap)
		delete mNormalMap;

	if (mOpacityMap)
		delete mOpacityMap;

}

void SceneObject::transform(const MatrixD &m)
{
	/*for (std::vector<Plane*>::iterator walk = mCutPlaneList.begin(); walk != mCutPlaneList.end(); walk++)
	{
		(*walk)->transform(m);
	}*/
}

void SceneObject::transformUV(const MatrixD &m)
{
	m.mul(mNorth);
	m.mul(mGreenwich);	
}

bool SceneObject::isInsideCutPlane(const Ray& ray, F64 distance) const
{
	Point3D ip = ray.getOrigin() + (ray.getDirection() * distance);
	
	for (std::vector<Plane*>::const_iterator walk = mCutPlaneList.begin(); walk != mCutPlaneList.end(); walk++)
	{	
		Plane *plane = *walk;
		Point3D vector = ip - plane->getAnchor();
		vector.normalize();
		if (dot(plane->getNormal(), vector) > -EPSILON)
			return false;
	}
	return true;
}

Scene::~Scene() 
{
	while (!mLightList.empty())
	{		
		PointLight *light = mLightList.back();
		mLightList.pop_back();
		delete light;
	}
}

static bool checkOpacityMap(const SceneObject *obj, PointUV &uv)
{
	const OpacityMap *opacityMap = obj->getOpacityMap();

	if (opacityMap)
	{
		U32 i = U32(opacityMap->hTileSize * uv.u) % opacityMap->width;
      U32 j = U32(opacityMap->vTileSize * uv.v) % opacityMap->height;
		bool flag = opacityMap->getFlag(i, j);

		if (!flag) 
			return false;
	}
	return true;
}

// TODO: Improve OpacityMap workaround
const SceneObject* Scene::findClosestIntersection(const Ray& ray, Point3D &intersection, Point3D &normal, PointUV &uv, F64& distance)
{	
	SceneObject::IntersectResult res;
	SceneObject *intersectedObj = NULL;	
	SceneObject *prevIntersectedObj = NULL;
	Point3D prevIntersection;
	Point3D prevNormal;
	PointUV prevUV;
	F64 prevDistance = distance;
	
	for (std::vector<SceneObject*>::const_iterator walk = mObjList.begin(); walk != mObjList.end(); walk++)
	{		
		SceneObject *obj = *walk;

		res = obj->intersect(ray, distance);

		if (res == SceneObject::HIT)
		{
			intersection = ray.getOrigin() + (ray.getDirection() * distance);
			normal = obj->getNormal(intersection);
			
			if (dot(normal, ray.getDirection()) > EPSILON) // Use correct normal
				normal *= -1;

			if (obj->getTexture() || obj->getBumpMap() || obj->getOpacityMap())
				uv = obj->getUV(intersection, normal);

			if (checkOpacityMap(obj, uv))
			{
				intersectedObj = obj;
				prevIntersectedObj = obj;
				prevIntersection = intersection;
				prevNormal = normal;
				prevUV = uv;
				prevDistance = distance;

			}
			else
			{
				intersectedObj = prevIntersectedObj;
				intersection = prevIntersection;
				normal = prevNormal;
				uv = prevUV;
				distance = prevDistance;
			}
		}
	}
	return intersectedObj;
}

// TODO: Improve OpacityMap workaround
void Scene::findIntersections(const Ray &ray, IntersectionList &list)
{
	Point3D intersection;
	Point3D normal;
	PointUV uv;
	F64	  ignoredDistance = F64_MAX;
   IntersectionList::IntersectionListNode *prevFirst = NULL;

	for (std::vector<SceneObject*>::const_iterator walk = mObjList.begin(); walk != mObjList.end(); walk++)
	{
		SceneObject::IntersectResult res = (*walk)->intersect(ray, ignoredDistance, &list);
      IntersectionList::IntersectionListNode *first = list.getFirst();

      if (prevFirst != first)
		{			
			const SceneObject *obj = first->getObject();
			intersection = ray.getOrigin() + (ray.getDirection() * ignoredDistance);
			normal = obj->getNormal(intersection);

			if (dot(normal, ray.getDirection()) > EPSILON) // Use correct normal
				normal *= -1;

			if (obj->getOpacityMap() && !checkOpacityMap(obj, obj->getUV(intersection, normal)))
			{
				list.pop();
			}
         prevFirst = list.getFirst();
		}
	}
}

IntersectionList::~IntersectionList()
{
	clear();
}

void IntersectionList::clear()
{
	for (IntersectionListNode *walk = mFirst; walk != NULL;)
	{
		IntersectionListNode *deleteMe = walk;
		walk = walk->mNext;
		delete deleteMe;
	}
	mFirst = NULL;
}

void IntersectionList::pop()
{
	IntersectionListNode *deleteMe = mFirst;

	mFirst = mFirst->mNext;
	delete deleteMe;
}

void IntersectionList::add(const SceneObject *obj, F64 distance)
{
	IntersectionListNode *node = new IntersectionListNode();
	node->mObj = obj;
	node->mDistance = distance;
	node->mNext = mFirst;
	mFirst = node;
}

class SceneProcessor : public X3DOnePassProcessor
{
public:
	SceneProcessor();
	
	void process(X3D::Scene *scene);
};

class MyVisitor : public X3DComponentVisitor
{
public:
	Scene*      scene;
	Material    material;
	Texture*    texture;
	BumpMap*    bumpMap;
	NormalMap*  normalMap;
	OpacityMap* opacityMap;
	Point3D*    north;
	Point3D*    greenwich;

	std::vector<MatrixD*> matrixList;
	std::vector<MatrixD*> textureMatrixList;
	std::vector<Plane*> planeList;

	void addCutPlanes(SceneObject *obj);
	void tranformObject(SceneObject *obj);	
private:
	void loadTexture(Texture *texture, const X3D::ImageTexture *textureNode, const char *url);	
public:
	MyVisitor();

	void addObject(SceneObject *obj)
	{
		obj->setMaterial(material);
		if (texture)
			obj->setTexture(texture);
		if (bumpMap)
			obj->setBumpMap(bumpMap);
		if (normalMap)
			obj->setNormalMap(normalMap);
		if (opacityMap)
			obj->setOpacityMap(opacityMap);
		if (north)
			obj->setNorth(*north);
		if (greenwich)
			obj->setGeenwich(*greenwich);
		addCutPlanes(obj);
		tranformObject(obj);
		scene->addObject(obj);
	}

public:
	static void enterX3DViewpointNode(X3D::Viewpoint*);
	static void enterX3DTransformNode(X3D::Transform*);
	static void leaveX3DTransformNode(X3D::Transform*);
	static void enterX3DPointLightNode(X3D::PointLight*);
	static void leaveX3DShapeNode(X3D::Shape*);
	static void enterX3DMaterialNode(X3D::Material*);
	static void enterX3DImageTextureNode(X3D::ImageTexture*);
	static void enterX3DConeNode(X3D::Cone*);
	static void enterX3DCylinderNode(X3D::Cylinder*);
	static void enterX3DSphereNode(X3D::Sphere*);
	static void enterX3DCutPlaneNode(X3D::CutPlane*);
	static void enterX3DPolygonNode(X3D::Polygon*);
   static void enterX3DQuadricSurfaceNode(X3D::QuadricSurface*);
	static void enterX3DDiskNode(X3D::Disk*);
	
} gVisitor;



SceneProcessor::SceneProcessor()
{
	setGraphTraversal(new DFSGraphTraversal());
	setComponentVisitor(new MyVisitor());
}

void SceneProcessor::process(X3D::Scene *scene)
{
	traverse(scene);
}

MyVisitor::MyVisitor() : X3DComponentVisitor(), texture(NULL), bumpMap(NULL)
{


	//X3D::Transform
	defineComponentName("MyVisitor", "MyVisitor");
	define(Recorder<X3D::Viewpoint>::getEnterFunction(&MyVisitor::enterX3DViewpointNode));
	define(Recorder<X3D::Transform>::getEnterFunction(&MyVisitor::enterX3DTransformNode));
	define(Recorder<X3D::Transform>::getLeaveFunction(&MyVisitor::leaveX3DTransformNode));
	define(Recorder<X3D::PointLight>::getEnterFunction(&MyVisitor::enterX3DPointLightNode));
	define(Recorder<X3D::Shape>::getLeaveFunction(&MyVisitor::leaveX3DShapeNode)); 	
	define(Recorder<X3D::Cone>::getEnterFunction(&MyVisitor::enterX3DConeNode));
	define(Recorder<X3D::Cylinder>::getEnterFunction(&MyVisitor::enterX3DCylinderNode));
	define(Recorder<X3D::Sphere>::getEnterFunction(&MyVisitor::enterX3DSphereNode));
	define(Recorder<X3D::Material>::getEnterFunction(&MyVisitor::enterX3DMaterialNode)); 
	define(Recorder<X3D::ImageTexture>::getEnterFunction(&MyVisitor::enterX3DImageTextureNode)); 
	define(Recorder<X3D::CutPlane>::getEnterFunction(&MyVisitor::enterX3DCutPlaneNode));
	define(Recorder<X3D::Polygon>::getEnterFunction(&MyVisitor::enterX3DPolygonNode));
   define(Recorder<X3D::QuadricSurface>::getEnterFunction(&MyVisitor::enterX3DQuadricSurfaceNode));
	define(Recorder<X3D::Disk>::getEnterFunction(&MyVisitor::enterX3DDiskNode));
}


void MyVisitor::enterX3DViewpointNode(X3D::Viewpoint* viewpointNode)
{
	SFVec3f pos = viewpointNode->getPosition();
	gVisitor.scene->setViewpoint(Point3D(pos.x, pos.y, pos.z));
}

inline static void mat_rotateX(MatrixD &mat, const F64 theta)
{
	Point4D row;
	F64 cosTheta = cos(theta);
	F64 sinTheta = sin(theta);

	row.set(1.0, 0.0, 0.0,0.0);
	mat.setRow(0, row);
	row.set(0.0, cosTheta, -sinTheta, 0.0);
	mat.setRow(1, row);
	row.set(0.0, sinTheta, cosTheta, 0.0);
	mat.setRow(2, row);
	row.set(0.0, 0.0, 0.0, 1.0);
	mat.setRow(3, row);
}

inline static void mat_rotateY(MatrixD &mat, const F64 theta)
{
	Point4D row;
	F64 cosTheta = cos(theta);
	F64 sinTheta = sin(theta);

	row.set(cosTheta, 0.0, sinTheta, 0.0);
	mat.setRow(0, row);
	row.set(0.0, 1.0, 0.0, 0.0);
	mat.setRow(1, row);
	row.set(-sinTheta, 0.0, cosTheta, 0.0);
	mat.setRow(2, row);
	row.set(0.0, 0.0, 0.0, 1.0);
	mat.setRow(3, row);
}

inline static void mat_rotateZ(MatrixD &mat, const F64 theta)
{
	Point4D row;
	F64 cosTheta = cos(theta);
	F64 sinTheta = sin(theta);

	row.set(cosTheta, -sinTheta, 0.0, 0.0);
	mat.setRow(0, row);
	row.set(sinTheta, cosTheta, 0.0, 0.0);
	mat.setRow(1, row);
	row.set(0.0, 0.0, 1.0, 0.0);
	mat.setRow(2, row);
	row.set(0.0, 0.0, 0.0, 1.0);
	mat.setRow(3, row);
}

void MyVisitor::enterX3DTransformNode(X3D::Transform *transformNode)
{
	// P' = T * C * R * SR * S * -SR * -C * P
	SFVec3f C = transformNode->getCenter();
	SFVec3f S = transformNode->getScale();
	SFRotation R = transformNode->getRotation();
	SFVec3f T = transformNode->getTranslation();
	
	Point4D row;
	// Center translation matrix
	MatrixD cMat;
	row.set(1.0, 0.0, 0.0, C.x);
	cMat.setRow(0, row);
	row.set(0.0, 1.0, 0.0, C.y);
	cMat.setRow(1, row);
	row.set(0.0, 0.0, 1.0, C.y);
	cMat.setRow(2, row);
	row.set(0.0, 0.0, 0.0, 1.0);
	cMat.setRow(3, row);
	// Scale matrix
	/*MatrixD sMat;
	row.set(S.x, 0.0, 0.0, 0.0);
	sMat.setRow(0, row);
	row.set(0.0, S.y, 0.0, 0.0);
	sMat.setRow(1, row);
	row.set(0.0, 0.0, S.z, 0.0);
	sMat.setRow(2, row);
	row.set(0.0, 0.0, 0.0, 1.0);
	sMat.setRow(3, row);*/
	// Rotation matrix	
	MatrixD rMat;	
	SFRotation rotation = transformNode->getRotation();
	F64 theta = rotation.angle;

	if (rotation.x)
		mat_rotateX(rMat, theta);
	else if (rotation.y)
		mat_rotateY(rMat, theta);
	else
		mat_rotateZ(rMat, theta);

	/*F64 theta = rotation.angle;
	F64 c = cos(theta);
	F64 s = sin(theta);
	F64 t = 1 - c;
	F64 x = rotation.x;
	F64 y = rotation.y;
	F64 z = rotation.z;

	row.set(t * x * x + c, t * x * y + s * z, t * x * z - s * y, 0.0);
	rMat.setRow(0, row);
	row.set(t * x * y - s * z, t * y * y + c, t * y * z + s * z, 0.0);
	rMat.setRow(0, row);
	row.set(t * x * y + s * y, t * y * z - s * x, t * z * z + c, 0.0);
	rMat.setRow(0, row);
	row.set(0.0, 0.0, 0.0, 1.0);
	rMat.setRow(0, row);*/
	// Translation matrix
	MatrixD tMat;
	row.set(1.0, 0.0, 0.0, T.x);
	tMat.setRow(0, row);
	row.set(0.0, 1.0, 0.0, T.y);
	tMat.setRow(1, row);
	row.set(0.0, 0.0, 1.0, T.z);
	tMat.setRow(2, row);
	row.set(0.0, 0.0, 0.0, 1.0);
	tMat.setRow(3, row);

	// P' = T * C * R * SR * S * -SR * -C * P
	MatrixD *m = new MatrixD();
	m->mul(tMat, cMat);	
	m->mul(rMat);
	//m->mul(sMat);

	row.set(1.0, 0.0, 0.0, -C.x);
	cMat.setRow(0, row);
	row.set(0.0, 1.0, 0.0, -C.y);
	cMat.setRow(1, row);
	row.set(0.0, 0.0, 1.0, -C.z);
	cMat.setRow(2, row);
	row.set(0.0, 0.0, 0.0, 1.0);
	cMat.setRow(3, row);

	m->mul(cMat);	
	gVisitor.matrixList.push_back(m);
	//m = new  MatrixD();
	gVisitor.textureMatrixList.push_back(new MatrixD(rMat));
}

void MyVisitor::leaveX3DTransformNode(X3D::Transform*)
{
	MatrixD* matrix = gVisitor.matrixList.back();
	gVisitor.matrixList.pop_back();
	delete matrix;

	matrix = gVisitor.textureMatrixList.back();
	gVisitor.textureMatrixList.pop_back();
	delete matrix;

}

void MyVisitor::addCutPlanes(SceneObject *obj)
{
	std::vector<Plane*> *planeList = &gVisitor.planeList;
	for (std::vector<Plane*>::const_iterator walk = planeList->begin(); walk != planeList->end(); walk++)
		obj->addCutPlane(*walk);
}

void MyVisitor::tranformObject(SceneObject *obj)
{
	std::vector<MatrixD*>* vector = &gVisitor.matrixList;

	if (obj && vector->size() > 0)
	{
		for (std::vector<MatrixD*>::const_reverse_iterator walk = vector->rbegin(); walk != vector->rend(); walk++)
		{
			obj->transform(**walk);
         gVisitor.scene->transformationCount++;
		}
	}

	vector = &gVisitor.textureMatrixList;

	if (obj && vector->size() > 0)
	{
		for (std::vector<MatrixD*>::const_reverse_iterator walk = vector->rbegin(); walk != vector->rend(); walk++)
		{
			obj->transformUV(**walk);
         gVisitor.scene->transformationCount++;
		}
	}
}

void MyVisitor::enterX3DPointLightNode(X3D::PointLight* pointLightNode)
{
	SFVec3f location = pointLightNode->getLocation();	
	SFColor color = pointLightNode->getColor();
	PointLight *pointLight = new PointLight(pointLightNode->getIntensity(), 
														 Point3D(location.x, location.y, location.z),
														 ColorF(color.r, color.g, color.b));
	SFVec3f att = pointLightNode->getAttenuation();
	pointLight->setAttenuationConstants(att.x, att.y, att.z);
	gVisitor.scene->addLight(pointLight);
}

void MyVisitor::enterX3DMaterialNode(X3D::Material *materialNode)
{
	Material &material = gVisitor.material;
	material.ambientIntensity = materialNode->getAmbientIntensity();
	const SFColor &diffusiveColor = materialNode->getDiffuseColor();
	material.diffusseCoefficient = materialNode->getDiffuseCoefficient();
	material.diffuseColor.set(diffusiveColor.r, diffusiveColor.g, diffusiveColor.b);
	const SFColor &specularColor = materialNode->getSpecularColor();	
	material.shininess = materialNode->getShininess();
	material.specularColor.set(specularColor.r, specularColor.g, specularColor.b);
   material.specularReflectionExponent = materialNode->getSpecularReflectionExponent();
	material.diffusiveness = materialNode->getDiffusiveness();
	material.reflectiveness = materialNode->getReflectiveness();
	material.transparency = materialNode->getTransparency();
	material.translucency = materialNode->getTranslucency();
   material.refractionIndex = materialNode->getRefractionIndex();
	assert(isZero(material.diffusiveness + material.reflectiveness + material.transparency - 1.0f));
}

void MyVisitor::loadTexture(Texture *texture, const X3D::ImageTexture *textureNode, const char *url)
{
	File file;

	file.open(url, File::Read);
	texture->bitmap.read(file);
	SFVec3f north = textureNode->getNorth();
	SFVec3f greenwich = textureNode->getGreenwich();
	gVisitor.north = new Point3D();
	gVisitor.north->set(north.x, north.y, north.z);
	gVisitor.north->normalize();
	gVisitor.greenwich = new Point3D();	
	gVisitor.greenwich->set(greenwich.x, greenwich.y, greenwich.z);
	gVisitor.greenwich->normalize();
	texture->hTile = textureNode->getHTile();
	texture->vTile = textureNode->getVTile();
	texture->hTileSize = texture->hTile * texture->bitmap.width;
	texture->vTileSize = texture->vTile * texture->bitmap.height;
}


void MyVisitor::enterX3DImageTextureNode(X3D::ImageTexture* textureNode)
{
	MFString strVect = textureNode->getUrl();
   std::string texturePath("C:\\Documents and Settings\\merizald\\My Documents\\Visual Studio 2005\\Projects\\RayTracer\\RayTracer\\textures\\");

	// Texture
	if (strVect.size() > 0 && strVect[0].length() > 0)
	{
		Texture *texture = new Texture();		
      std::string url = texturePath + strVect[0];
		gVisitor.loadTexture(texture, textureNode, url.data());
		gVisitor.texture = texture;
	}

	if (strVect.size() > 1 && strVect[1].length() > 0)
	{
		Texture *texture = new Texture();
      std::string url = texturePath + strVect[1];
		gVisitor.loadTexture(texture, textureNode, url.data());
		BumpMap *bumpMap = new BumpMap();
		bumpMap->init(texture, textureNode->getMinHeight(), textureNode->getMaxHeight());
		bumpMap->hTile = texture->hTile;
		bumpMap->vTile = texture->vTile;
		bumpMap->hTileSize = texture->hTileSize;
		bumpMap->vTileSize = texture->vTileSize;
		gVisitor.bumpMap = bumpMap;
		delete texture;
	}

	if (strVect.size() > 2 && strVect[2].length() > 0)
	{
		Texture *texture = new Texture();
      std::string url = texturePath + strVect[2];
		gVisitor.loadTexture(texture, textureNode, url.data());
		OpacityMap *opacityMap = new OpacityMap();
		opacityMap->init(texture, textureNode->getTolerance());
		opacityMap->hTile = texture->hTile;
		opacityMap->vTile = texture->vTile;
		opacityMap->hTileSize = texture->hTileSize;
		opacityMap->vTileSize = texture->vTileSize;
		gVisitor.opacityMap = opacityMap;
		delete texture;
	}

	if (strVect.size() > 3 && strVect[3].length() > 0)
	{
		Texture *texture = new Texture();
      std::string url = texturePath + strVect[3];
		gVisitor.loadTexture(texture, textureNode, url.data());
		NormalMap *normalMap = new NormalMap();
		normalMap->init(texture);
		normalMap->hTile = texture->hTile;
		normalMap->vTile = texture->vTile;
		normalMap->hTileSize = texture->hTileSize;
		normalMap->vTileSize = texture->vTileSize;
		gVisitor.normalMap = normalMap;
		delete texture;
	}
}

void MyVisitor::enterX3DSphereNode(X3D::Sphere *sphereNode)
{
	Sphere *sphere = new Sphere(sphereNode->getRadius());
	gVisitor.addObject(sphere);
   gVisitor.scene->sphereCount++;
}

void MyVisitor::enterX3DConeNode(X3D::Cone *coneNode)
{
	F64 height = coneNode->getHeight();
	Cone *cone;

	if (height < EPSILON)
		cone = new Cone(coneNode->getBottomRadius());
	else
		cone = new Cone(coneNode->getBottomRadius(), coneNode->getHeight());
	gVisitor.addObject(cone);
   gVisitor.scene->coneCount++;
}

void MyVisitor::enterX3DCylinderNode(X3D::Cylinder *cylinderNode)
{
	F64 height = cylinderNode->getHeight();
	Cylinder *cylinder;
	bool isFinite = height >= EPSILON;

	if (isFinite)
		cylinder = new Cylinder(cylinderNode->getRadius(), height);
	else
		cylinder = new Cylinder(cylinderNode->getRadius());
	
	if (isFinite && cylinderNode->getTop())
	{
		// Fix me
		Disk *top = NULL;
		cylinder->createTop(&top);
		top->setMaterial(gVisitor.material);
		gVisitor.tranformObject(top);
		gVisitor.scene->addObject(top);
      gVisitor.scene->diskCount++;
	}

	if (isFinite && cylinderNode->getBottom())
	{
		// Fix me
		Disk *bottom = NULL;
		cylinder->createBottom(&bottom);
		bottom->setMaterial(gVisitor.material);
		gVisitor.tranformObject(bottom);
		gVisitor.scene->addObject(bottom);
      gVisitor.scene->diskCount++;
	}
	gVisitor.addObject(cylinder);
   gVisitor.scene->cylinderCount++;
}

void MyVisitor::enterX3DCutPlaneNode(X3D::CutPlane *cutPlaneNode)
{	
	SFVec3f a = cutPlaneNode->getAnchor();
	SFVec3f d = cutPlaneNode->getNormal();
	Point3D anchor(a.x, a.y, a.z);
	Point3D normal(d.x,d.y, d.z);
	Plane *plane = new Plane(anchor, normal);
	gVisitor.tranformObject(plane);
	gVisitor.planeList.push_back(plane);
   gVisitor.scene->cutPlaneCount++;
}

Point3D gVertexTable[3];

void MyVisitor::enterX3DPolygonNode(X3D::Polygon *polygonNode)
{
   gVertexTable[0].set(0.0, 0.0, 0.0);
   gVertexTable[1].set(1.0, 0.0, 0.0);
   gVertexTable[2].set(1.0, -1.0, 0.0);
	//PolygonD *poly = new PolygonD();
   Triangle *poly = new Triangle(gVertexTable, 0, 1, 2);
	const MFVec3f& points = polygonNode->getPoints();

	/*for (MFVec3f::const_iterator walk = points.begin(); walk != points.end(); walk++)
	{
		const SFVec3f p = *walk;
		poly->addVertex(Point3D(p.x, p.y, p.z));
	}*/
	//poly->preInitialize();
	gVisitor.addObject(poly);
	//poly->initialize();
	//assert(poly->isInitialized());
   gVisitor.scene->polygonCount++;
   
/*   poly->preInitialize();
	std::vector<MatrixD*>* vector = &gVisitor.textureMatrixList;

	if (poly && vector->size() > 0)
	{
		for (std::vector<MatrixD*>::const_reverse_iterator walk = vector->rbegin(); walk != vector->rend(); walk++)
		{
			poly->transformUV(**walk);
		}
	}*/
   }

void MyVisitor::enterX3DQuadricSurfaceNode(X3D::QuadricSurface* quadricSurfaceNode)
{
   QuadricSurface *qSurface = new QuadricSurface(
      quadricSurfaceNode->A,
      quadricSurfaceNode->B,
      quadricSurfaceNode->C,
      quadricSurfaceNode->D,
      quadricSurfaceNode->E,
      quadricSurfaceNode->F,
      quadricSurfaceNode->G,
      quadricSurfaceNode->H,
      quadricSurfaceNode->J,
      quadricSurfaceNode->K);
   //disk->setBounds(diskNode->getWidthLeft(), diskNode->getWidthRight(), diskNode->getHeightBottom(), diskNode->getHeightTop());
   qSurface->setBounds(11.5, 11.5, 11.5, 11.5);
   gVisitor.addObject(qSurface);
   gVisitor.scene->quadricCount++;
}


void MyVisitor::enterX3DDiskNode(X3D::Disk* diskNode)
{
	Disk * disk = new Disk(diskNode->getOuterRadius(), diskNode->getAnti() != 0.0);
	disk->setBounds(diskNode->getWidthLeft(), diskNode->getWidthRight(), diskNode->getHeightBottom(), diskNode->getHeightTop());
	gVisitor.addObject(disk);
   gVisitor.scene->diskCount++;
}

void MyVisitor::leaveX3DShapeNode(X3D::Shape*)
{
	gVisitor.planeList.clear();
	gVisitor.texture = NULL;
	gVisitor.bumpMap = NULL;
	gVisitor.opacityMap = NULL;
	if (gVisitor.north)
	{
		delete gVisitor.north;
		gVisitor.north = NULL;
	}
	if (gVisitor.greenwich)
	{
		delete gVisitor.greenwich;
		gVisitor.greenwich = NULL;
	}
}

void Scene::load(const char *filename)
{
	X3D::Loader loader;
	SceneProcessor sp;
	MemReleaser releaser;
//	GraphTester tester;

	gVisitor.scene = this;
	X3D::Scene *s = loader.load(filename, false);
	sp.process(s);
//	X3D::Scene *s = loader.load("c:/dino.x3d", false);	
//	SceneWalker *myWalker = new SceneWalker();
//	tester.setWalker(myWalker);
//	tester.test(s);
	releaser.release(s);
}
