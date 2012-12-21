#ifndef _MATRIX_H_
#define _MATRIX_H_

#ifndef _PLATFORM_H_
#include "platform/platform.h"
#endif

class Point3D;
class Point4D;

class MatrixD
{
public:
	MatrixD();
    
	bool isIdentity() const;
	MatrixD& identity();
    
	void setColumn(S32 col, const Point4D &p);
	void setRow(S32 row, const Point4D &p);
    
    operator F64*() { return mData; }
    operator F64*() const { return  (F64*) mData; }
    
	void mul(Point3D &p) const;
	void mul(Point4D &p) const;
    
	MatrixD& mul(const MatrixD &a);
	MatrixD& mul(const MatrixD &a, const MatrixD &b);
    
    MatrixD& inverse();
    MatrixD& transpose();
private:
	F64 mData[16];
};

#endif


