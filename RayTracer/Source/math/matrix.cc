#include "math/matrix.h"
#include "math/point.h"

inline void matD_x_point3D(const F64 *a, const F64 *p, F64 *res)
{
	res[0]  = a[0]*p[0]  + a[1]*p[1]  + a[2]*p[2]   + a[3];
	res[1]  = a[4]*p[0]  + a[5]*p[1]  + a[6]*p[2]   + a[7];
	res[2]  = a[8]*p[0]  + a[9]*p[1]  + a[10]*p[2]  + a[11];
}

inline void matD_x_vectorD(const F64 *m, const F64 *v, F64 *vresult)
{
   vresult[0] = m[0]*v[0] + m[1]*v[1] + m[2]*v[2];
   vresult[1] = m[4]*v[0] + m[5]*v[1] + m[6]*v[2];
   vresult[2] = m[8]*v[0] + m[9]*v[1] + m[10]*v[2];
}

inline void matD_x_point4D(const F64 *a, const F64 *p, F64 *res)
{
	res[0]  = a[0]*p[0]  + a[1]*p[1]  + a[2]*p[2]   + a[3]*p[3];
	res[1]  = a[4]*p[0]  + a[5]*p[1]  + a[6]*p[2]   + a[7]*p[3];
	res[2]  = a[8]*p[0]  + a[9]*p[1]  + a[10]*p[2]  + a[11]*p[3];
	res[3]  = a[12]*p[0] + a[13]*p[1] + a[14]*p[2]  + a[15]*p[3];
}

inline void matD_x_matD(const F64 *a, const F64 *b, F64 *res)
{
	res[0]  = a[0]*b[0]  + a[1]*b[4]  + a[2]*b[8]   + a[3]*b[12];
	res[1]  = a[0]*b[1]  + a[1]*b[5]  + a[2]*b[9]   + a[3]*b[13];
	res[2]  = a[0]*b[2]  + a[1]*b[6]  + a[2]*b[10]  + a[3]*b[14];
	res[3]  = a[0]*b[3]  + a[1]*b[7]  + a[2]*b[11]  + a[3]*b[15];

	res[4]  = a[4]*b[0]  + a[5]*b[4]  + a[6]*b[8]   + a[7]*b[12];
	res[5]  = a[4]*b[1]  + a[5]*b[5]  + a[6]*b[9]   + a[7]*b[13];
	res[6]  = a[4]*b[2]  + a[5]*b[6]  + a[6]*b[10]  + a[7]*b[14];
	res[7]  = a[4]*b[3]  + a[5]*b[7]  + a[6]*b[11]  + a[7]*b[15];

	res[8]  = a[8]*b[0]  + a[9]*b[4]  + a[10]*b[8]  + a[11]*b[12];
	res[9]  = a[8]*b[1]  + a[9]*b[5]  + a[10]*b[9]  + a[11]*b[13];
	res[10] = a[8]*b[2]  + a[9]*b[6]  + a[10]*b[10] + a[11]*b[14];
	res[11] = a[8]*b[3]  + a[9]*b[7]  + a[10]*b[11] + a[11]*b[15];

	res[12] = a[12]*b[0] + a[13]*b[4] + a[14]*b[8]  + a[15]*b[12];
	res[13] = a[12]*b[1] + a[13]*b[5] + a[14]*b[9]  + a[15]*b[13];
	res[14] = a[12]*b[2] + a[13]*b[6] + a[14]*b[10] + a[15]*b[14];
	res[15] = a[12]*b[3] + a[13]*b[7] + a[14]*b[11] + a[15]*b[15];
}

inline F64 matD_determinant(const F64 *m)
{
   return m[0] * (m[5] * m[10] - m[6] * m[9])  +
      m[4] * (m[2] * m[9]  - m[1] * m[10]) +
      m[8] * (m[1] * m[6]  - m[2] * m[5])  ;
}

inline void matD_inverse(F64 *m)
{
   F64 det = matD_determinant(m);

   F64 invDet = 1.0f/det;
   F64 temp[16];

   temp[0] = (m[5] * m[10]- m[6] * m[9]) * invDet;
   temp[1] = (m[9] * m[2] - m[10]* m[1]) * invDet;
   temp[2] = (m[1] * m[6] - m[2] * m[5]) * invDet;

   temp[4] = (m[6] * m[8] - m[4] * m[10])* invDet;
   temp[5] = (m[10]* m[0] - m[8] * m[2]) * invDet;
   temp[6] = (m[2] * m[4] - m[0] * m[6]) * invDet;

   temp[8] = (m[4] * m[9] - m[5] * m[8]) * invDet;
   temp[9] = (m[8] * m[1] - m[9] * m[0]) * invDet;
   temp[10]= (m[0] * m[5] - m[1] * m[4]) * invDet;

   m[0] = temp[0];
   m[1] = temp[1];
   m[2] = temp[2];

   m[4] = temp[4];
   m[5] = temp[5];
   m[6] = temp[6];

   m[8] = temp[8];
   m[9] = temp[9];
   m[10] = temp[10];

   // invert the translation
   temp[0] = -m[3];
   temp[1] = -m[7];
   temp[2] = -m[11];
   matD_x_vectorD(m, temp, &temp[4]);
   m[3] = temp[4];
   m[7] = temp[5];
   m[11]= temp[6];
}

inline void swap(F64 &a, F64 &b)
{
   F64 temp = a;
   a = b;
   b = temp;
}

inline void matD_transpose(F64 *m)
{
   swap(m[1], m[4]);
   swap(m[2], m[8]);
   swap(m[3], m[12]);
   swap(m[6], m[9]);
   swap(m[7], m[13]);
   swap(m[11],m[14]);
}

MatrixD::MatrixD()
{
	for (int i = 0; i < 16; ++i)
		mData[i] = 0.0;
}

bool MatrixD::isIdentity() const
{
   return
   mData[0]  == 1.0f &&
   mData[1]  == 0.0f &&
   mData[2]  == 0.0f &&
   mData[3]  == 0.0f &&
   mData[4]  == 0.0f &&
   mData[5]  == 1.0f &&
   mData[6]  == 0.0f &&
   mData[7]  == 0.0f &&
   mData[8]  == 0.0f &&
   mData[9]  == 0.0f &&
   mData[10] == 1.0f &&
   mData[11] == 0.0f &&
   mData[12] == 0.0f &&
   mData[13] == 0.0f &&
   mData[14] == 0.0f &&
   mData[15] == 1.0f;
}

MatrixD& MatrixD::identity()
{
   mData[0]  = 1.0f;
   mData[1]  = 0.0f;
   mData[2]  = 0.0f;
   mData[3]  = 0.0f;
   mData[4]  = 0.0f;
   mData[5]  = 1.0f;
   mData[6]  = 0.0f;
   mData[7]  = 0.0f;
   mData[8]  = 0.0f;
   mData[9]  = 0.0f;
   mData[10] = 1.0f;
   mData[11] = 0.0f;
   mData[12] = 0.0f;
   mData[13] = 0.0f;
   mData[14] = 0.0f;
   mData[15] = 1.0f;
   return (*this);
}

void MatrixD::setColumn(S32 col, const Point4D &p)
{
	mData[col] = p.x; 
	mData[col + 4] = p.y;
	mData[col + 8] = p.z;
	mData[col + 12] = p.w;
}

void MatrixD::setRow(S32 row, const Point4D &p)
{
	row *= 4;
	mData[row] = p.x; 
	mData[row + 1] = p.y;
	mData[row + 2] = p.z;
	mData[row + 3] = p.w;
}

void MatrixD::mul(Point3D &p) const
{
	Point3D tmp(p);
	matD_x_point3D(*this, &p.x, &tmp.x);
	p = tmp;
}

void MatrixD::mul(Point4D &p)  const
{
	Point4D tmp(p);
	matD_x_point4D(*this, &p.x, &tmp.x);
	p = tmp;
}

MatrixD& MatrixD::mul(const MatrixD &a)
{
	MatrixD tmp(*this);
	matD_x_matD(tmp, a, *this);
	return  *this;
}

MatrixD& MatrixD::mul(const MatrixD &a, const MatrixD &b)
{
	matD_x_matD(a, b, *this);
	return  *this;
}

MatrixD& MatrixD::inverse()
{
   matD_inverse(mData);
   return (*this);
}

MatrixD& MatrixD::transpose()
{
   matD_transpose(mData);
   return (*this);
}
