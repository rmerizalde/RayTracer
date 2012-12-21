#ifndef _LIGHT_H_
#define _LIGHT_H_

#ifndef _COLOR_H_
#include "core/color.h"
#endif

#ifndef _POINT_H_
#include "math/point.h"
#endif

class PointLight
{
public:
	PointLight() : mIntensity(1.0)
	{
		mLocation.set(0.0, 0.0, 0.0);
		mColor.set(1.0, 1.0, 1.0);
		mC1 = 1.0;
		mC2 = 0.0;
		mC3 = 0.0;		
	}

	PointLight(F32 intensity, const Point3D &location, const ColorF color) : mIntensity(intensity), mLocation(location), mColor(color)
	{}

	F32 getIntensity() const { return mIntensity; }
	const Point3D& getLocation() const { return mLocation; }
	void setAttenuationConstants(F32 c1, F32 c2, F32 c3) { mC1 = c1; mC2 = c2; mC3 = c3; }
	F32 getAttenuationFactor(F32 distance) const { return min(1 / (mC1 + mC2 * distance + mC3 * (distance * distance)), 1.0f); }
private:
	F32 mIntensity;
	Point3D mLocation;
	ColorF mColor;
	F32 mC1, mC2, mC3;	
};

#endif