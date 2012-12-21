#ifndef _MATH_H_
#define _MATH_H_

#ifndef _POINT_H_
#include "math/point.h"
#endif

#ifndef _MATHFN_H_
#include "math/mathFn.h"
#endif

#ifndef _MATRIX_H_
#include "math/matrix.h"
#endif

#ifndef _RAY_H_
#include "math/ray.h"
#endif

#ifdef PI
#undef PI
#endif

#define PI (3.141592653589793238462643)
#define PI2 (3.141592653589793238462643 * 2)
#define EPSILON (0.000001)
//  #define EPSILON (0.00000000000000000000000000000001)
#define F64_MAX (1.7976931348623157e+308)
#define F64_MIN (4.9e-324)

#define isZero(x) (x >= -EPSILON && x <= EPSILON)

#endif