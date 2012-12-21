
#include <iostream>
#include <assert.h>
#include "math/math.h"
#include "engine/raytracer.h"

#ifndef _COLOR_H_
#include "core/color.h"
#endif

#ifndef _BITMAP_H_
#include "core/bitmap.h"
#endif


#ifndef _FILE_H_
#include "core/file.h"
#endif

#ifndef _SCENE_H_
#include "scene/scene.h"
#endif

#include <time.h>
#include <iostream>

#define MAX_DEPTH (3)

//static ColorF BACKGROUND(0, 0, 0);
static ColorF BACKGROUND(0.05f, 0.05f, 0.05f);

static Scene scene;
static U8 *frameBuffer = NULL;

ColorF trace(const Ray &ray, F64 &distance, F64 refractionIndex, S32 depth);

ColorF shade(const SceneObject *obj, const Ray &ray, const Point3D &intersection, const Point3D &normal, const PointUV &uv, const F64 refractionIndex, const S32 depth)
{
	const PointLight *light;
	const Material &m = obj->getMaterial();
	// L is the vector from the light to the intersection point
	// N is the normal at the intersection point
	// R is the light rebound vector
	// V is the vector from the eye to the intersection point
	Point3D L, N, R, V;
	// ip is the intersection point
	Point3D ip = intersection;
	// Ip is the light intensity
	// fatt is the light attenuation factor
	// Temporal variables to store some dot product results
	F32 Ip, fatt, dotNL, dotRV;
	
	// Lighting
	ColorF I(0.0, 0.0, 0.0);
	ColorF Od = m.diffuseColor;
	ColorF Os = m.specularColor;
	F32 kd = m.diffusseCoefficient;			// kd is the diffuse-reflection coefficient2
	F32 ks = m.shininess;						// ks is the specular-reflection coefficient
	F32 n = m.specularReflectionExponent;  // n is the specular-reflection exponent
	F32 O1 = m.diffusiveness;
	F32 O2 = m.reflectiveness;
	F32 O3 = m.transparency;
	F64 d;											// Ligh vector magnitude
	//F64 distance;
	IntersectionList list;
    
	V = ray.getDirection() * -1;
	N = normal;
    
	
	const Texture *texture = obj->getTexture();
	const BumpMap *bumpMap = obj->getBumpMap();
    const NormalMap *normalMap = obj->getNormalMap();
	//const OpacityMap *opacityMap = obj->getOpacityMap();
    
	if (texture || bumpMap || normalMap/* || opacityMap*/)
	{
		//PointUV uv = obj->getUV(ip, N);
        
		if (texture)
		{
			U32 i = U32(texture->hTileSize * uv.u) % texture->bitmap.width;
			U32 j = U32(texture->vTileSize * uv.v) % texture->bitmap.height;
			texture->getTexel(i, j, Od);
            
            if (Od.alpha < 1.0)
            {
                O1 = Od.alpha;
                O3 = 1.0f - Od.alpha;
                Od.alpha = 1.0f;
            }
		}
        
		if (bumpMap)
		{
			U32 i = U32(bumpMap->hTileSize * uv.u) % bumpMap->width;
			U32 j = U32(bumpMap->vTileSize * uv.v) % bumpMap->height;
			F32 h = bumpMap->getHeight(i, j);
			F32 hb = bumpMap->getHeight(i-1, j);
			F32 ha = bumpMap->getHeight(i+1, j);
            
            ip = ip + N * h;
			obj->perturbNormal(N, i, j);
			//F32 h = bumpMap->getHeight(i, j);;
			//Od.set(h / 255, h / 255, h / 255);
		}
        
        if (normalMap)
        {
			U32 i = U32(normalMap->hTileSize * uv.u) % normalMap->width;
			U32 j = U32(normalMap->vTileSize * uv.v) % normalMap->height;
            
            N = normalMap->getNormal(i, j);
        }
	}
	
	for (U32 k = 0; k < scene.getLightCount(); ++k)
	{
		light = scene.getLight(k);
		Ip = light->getIntensity();
		L = light->getLocation() - ip;
		d = L.length();
		L.normalize();
        
		//distance = d;
		scene.findIntersections(Ray(ip, L), list);
		F32 S = 1.0;
		//(!obstacle || isZero(distance) || d < distance)? 1.0f : 0.0f;
		for (IntersectionList::IntersectionListNode *walk = list.getFirst(); walk != NULL; walk = walk->getNext())
		{
			if (!isZero(walk->getDistance()) && walk->getDistance() < d)
			{
				F32 kt = walk->getObject()->getMaterial().translucency;
				S *= kt;
				if (S <= EPSILON)
					break;
			}
		}
		list.clear();
        
		if (S > EPSILON)
		{
			fatt = light->getAttenuationFactor(F32(d));
            
			// Diffusse reflection
			dotNL = max(F32(dot(N, L)), 0.0f);
			
			// Specular reflection
			R = N * 2 * dot(N, L) - L;
			dotRV = max(F32(dot(R, V)), 0.0f);
			
			// Add light contribution
			I += (Od * kd * dotNL +  Os * ks * pow(dotRV, n)) * S * fatt * Ip;
		}
	}
	
	F32 Ia = m.ambientIntensity;
	I += Od * Ia;
    
    
    
	I *= O1;
	//I = Od;
    
	if (depth <= MAX_DEPTH)
	{
		if (O2 >  EPSILON)
		{
			F64 distance = F64_MAX;
			R = N * 2 * dot(N, V) - V;
            ColorF color = trace(Ray(ip, R), distance, refractionIndex, depth + 1);
			I += color * O2;
		}
        
		if (O3 > EPSILON)
		{
            F64 u1 = refractionIndex;
            F64 u2 = m.refractionIndex;
            F64 u = u1 / u2;
            F64 squaredU = u * u;
            F64 dotNV = dot(N, V);
            F64 radical = 1.0 - squaredU * (1.0 - dotNV * dotNV);
            if (radical > EPSILON)
            {
                Point3D T = N * (u * dotNV - sqrt(radical)) - V * u;
                F64 distance = F64_MAX;
                ColorF color = trace(Ray(ip, T), distance, u2, depth + 1);
                I += color * O3;
            }
		}
	}
    
	I.clamp();
	return I;
}

ColorF trace(const Ray &ray, F64 &distance, F64 refractionIndex, S32 depth)
{
	Point3D intersection;
	Point3D normal;
	PointUV uv;
	const SceneObject *intersectedObj = scene.findClosestIntersection(ray, intersection, normal, uv, distance);
    
	if (intersectedObj)
	{
		return shade(intersectedObj, ray, intersection, normal, uv, refractionIndex, depth);
	}
	else
	{
		return BACKGROUND;
	}
}

void render(const Point3D &eye, const Point3D &wMin, const Point3D &wMax, U32 hRes, U32 vRes)
{
	for (U32 i = 0; i < hRes; ++i)
	{
		for (U32 j = 0; j < vRes; ++j)
		{
			// Get the point in the projection plane
            ColorF sample(0.0f, 0.0f, 0.0f);
            /*F32 sampling[][2] = {{0.0, 0.0},
             {0.5, 0.0},
             {1.0, 0.0},
             {0.5, 0.5},
             {0.0, 1.0},
             {0.5, 0.5},
             {1.0, 1.0},
             {0.0, 0.5},
             {1.0, 0.5},
             {0.25, 0.25},
             {0.75, 0.25},
             {0.25, 0.75},
             {0.75, 0.75},
             };*/
            F32 sampling[][2] = {{0.5, 0.5}};
			Point3D w;
            U32 size = sizeof(sampling) / sizeof(F32[2]);
            
            for (U32 k = 0; k < size; ++k)
            {
                w.x = wMin.x + (i + sampling[k][0]) * (wMax.x - wMin.x) / hRes;
                w.y = wMin.y + (j + sampling[k][1]) * (wMax.y - wMin.y) / vRes;
                w.z = 0.0;
                
                // Calculate the distance between the eye and the projection plane
                Point3D direction = w - eye;
                direction.normalize();
                Ray ray(eye, direction);
                F64 distance = F64_MAX;
                sample += trace(ray, distance, 1.0f, 1);
            }
            
            sample.red /= size;
            sample.green /= size;
            sample.blue /= size;
            sample.alpha = 1.0;
            
            U32 pos = 4 * (hRes * j + i);
			frameBuffer[pos] = U8(255 * sample.alpha);
			frameBuffer[pos + 1] = U8(255 * sample.red);
			frameBuffer[pos + 2] = U8(255 * sample.green);
			frameBuffer[pos + 3] = U8(255 * sample.blue);
		}
	}
}


//S32 PASCAL WinMain( HINSTANCE hInstance, HINSTANCE, LPSTR lpszCmdLine, int)
S32 main(S32 argc, const char **argv)
{
	//U32 hRes = 1440;
	//U32 vRes = 1080;
    //U32 hRes = 1024;
	//U32 vRes = 768;
    //U32 hRes = 800;
	//U32 vRes = 600;
    U32 hRes = 640;
	U32 vRes = 480;
    //U32 hRes = 320;
	//U32 vRes = 240;
	U32 frameBufferSize = hRes * vRes * 4;
	frameBuffer = new U8[frameBufferSize];
    
    std::cout << "Loading scene ... \n";
    clock_t start = clock();
    //scene.load("sceneWater.xml");
    scene.load("scenetmp.xml");
    F32 seconds = (clock()-start)/(F32)CLOCKS_PER_SEC;
    printf("%d cones\n", scene.coneCount);
    printf("%d cross sections\n", scene.cutPlaneCount);
    printf("%d cylinders\n", scene.cylinderCount);
    printf("%d disks\n", scene.diskCount);
    printf("%d polygon\n", scene.polygonCount);
    printf("%d quadric surfaces\n", scene.quadricCount);
    printf("%d spheres\n", scene.sphereCount);
    printf("%d transformations\n", scene.transformationCount);
    printf("%d lights\n", scene.getLightCount());
    printf("Scene loaded in %d minutes and %d seconds (%f seconds)\n", (S32) (seconds/60), ((S32) seconds%60), seconds);
    Point3D wMin(-6.0, -4, 0.0);
    Point3D wMax(6.0, 4, 0.0);
    //Point3D wMin(-4.0, -3, 0.0);
    //Point3D wMax(4.0, 3, 0.0);
	//Point3D wMin(-12.0, -9, 0.0);
    //Point3D wMax(12.0, 9, 0.0);
    //Point3D wMin(-5.0, -3, 0.0);
	//Point3D wMax(19.0, 15, 0.0);
    //Point3D wMin(-19.5, -14.625, 0.0);
    //Point3D wMax(19.5, 14.625, 0.0);
    std::cout << "Rendering scene ... \n";
    start = clock();
    render(scene.getViewpoint(), wMin, wMax, hRes, vRes);
    seconds = (clock()-start)/(F32)CLOCKS_PER_SEC;
    printf("Scene rendered in %d minutes and %d seconds (%f seconds)\n", (S32) (seconds/60), ((S32) seconds%60), seconds);
    
    std::cout << "Saving image ... \n";
    
	File file;
	file.open("c:/temp/image.avs", File::Write);
	U32 width = convertLEndianToBEndian(hRes);
	U32 height = convertLEndianToBEndian(vRes);
	U32 bytesWritten = 0;
	file.write(4, (char*) &width, &bytesWritten);
	file.write(4, (char*) &height, &bytesWritten);
	file.write(frameBufferSize, (char*) frameBuffer, &bytesWritten);
	file.flush();
	file.close();
	delete[] frameBuffer;
    
    std::cout << "Press any key to continiue ... \n";
    std::cin.get();
	return 0;
}