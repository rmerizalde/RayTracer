#ifndef _COLOR_H_
#define _COLOR_H_

#ifndef _PLATFORM_H_
#include "platform/platform.h"
#endif

#define INV255 1.0f/255.0f

class ColorF
{
public:
    F32 red;
    F32 green;
    F32 blue;
    F32 alpha;
    
    ColorF() { }
    ColorF(const ColorF& copy);
    ColorF(const F32 r, const F32 g, const F32 b, const F32 a = 1.0f);
    
    void set(const F32 r, const F32 g, const F32 b, const F32 a = 1.0f);
    
    ColorF& operator+=(const ColorF& add);
    ColorF operator+(const ColorF& add) const;
    
    ColorF& operator*=(F32 mul);
    ColorF operator*(F32 mul) const;
    
    void clamp();
};

// Inlines

inline ColorF::ColorF(const F32 r, const F32 g, const F32 b, const F32 a)
{
    set(r, g, b, a);
}

inline ColorF::ColorF(const ColorF& copy)
{
    red   = copy.red;
    green = copy.green;
    blue  = copy.blue;
    alpha = copy.alpha;
}

inline void ColorF::set(const F32 r, const F32 g, const F32 b, const F32 a)
{
    red   = r;
    green = g;
    blue  = b;
    alpha = a;
}

inline ColorF& ColorF::operator+=(const ColorF& add)
{
    red += add.red;
    green += add.green;
    blue += add.blue;
    alpha += add.alpha;
    return *this;
}

inline ColorF ColorF::operator+(const ColorF& add) const
{
    return ColorF(red + add.red,
                  green + add.green,
                  blue  + add.blue,
                  alpha + add.alpha);
}

inline ColorF& ColorF::operator*=(F32 mul)
{
    red *= mul;
    green *= mul;
    blue *= mul;
    //alpha *= mul;
    return *this;
}

inline ColorF ColorF::operator*(F32 mul) const
{
    return ColorF(red * mul,
                  green * mul,
                  blue  * mul,
                  alpha);
    
}

inline void ColorF::clamp()
{
    if (red > 1.0f)
        red = 1.0f;
    else if (red < 0.0f)
        red = 0.0f;
    
    if (green > 1.0f)
        green = 1.0f;
    else if (green < 0.0f)
        green = 0.0f;
    
    if (blue > 1.0f)
        blue = 1.0f;
    else if (blue < 0.0f)
        blue = 0.0f;
    
    if (alpha > 1.0f)
        alpha = 1.0f;
    else if (alpha < 0.0f)
        alpha = 0.0f;
}

#endif