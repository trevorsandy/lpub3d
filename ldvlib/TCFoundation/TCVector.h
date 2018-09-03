/*
 ******************************************************************************
 * SimStarIV
 * Travis Cobbs, Chris Connley, Rusty Larner
 * CSC 480
 *
 * FILE:  TCVector.h
 ******************************************************************************
 */
#ifndef __TCVECTOR_H__
#define __TCVECTOR_H__

#include <TCFoundation/TCDefines.h>
#include <TCFoundation/mystring.h>

#include <stdio.h>
//#include <windows.h>
//#include <GL/gl.h>

// All overload * operators with 2 vectors are cross product.
// dot product is not an overloaded operator, simply dot.

typedef TCFloat* GlPt;
typedef const TCFloat *ConstGlPt;

class TCVector
{
public:
	// Constructors
	TCVector(void);
	TCVector(TCFloat, TCFloat, TCFloat);
	TCVector(const TCFloat *);
	TCVector(const TCVector&);

	// Destructor
	~TCVector(void);

	// Member Functions
	void print(FILE* = stdout) const;
	void print(char* buffer, int precision = 3) const;
#ifndef TC_NO_UNICODE
	void print(UCSTR buffer, size_t bufferSize, int precision = 3) const;
#endif // TC_NO_UNICODE
	std::string string(int precision = 6) const;
	::ucstring ucstring(int precision = 6) const;
	TCFloat length(void) const;
	TCFloat lengthSquared(void) const;
	TCFloat dot(const TCVector&) const;
	TCVector& normalize(void);
	TCFloat get(int i) const { return vector[i];}
	bool approxEquals(const TCVector &right, TCFloat epsilon) const;
	bool exactlyEquals(const TCVector &right) const;

	// Overloaded Operators
	TCVector operator*(const TCVector&) const;
	TCVector operator*(TCFloat) const;
	TCVector operator/(TCFloat) const;
	TCVector operator+(const TCVector&) const;
	TCVector operator-(const TCVector&) const;
	TCVector operator-(void) const;
	TCVector& operator*=(const TCVector&);
	TCVector& operator*=(TCFloat);
	TCVector& operator/=(TCFloat);
	TCVector& operator+=(const TCVector&);
	TCVector& operator-=(const TCVector&);
	TCVector& operator=(const TCVector&);
	int operator==(const TCVector&) const;
	int operator!=(const TCVector&) const;
	bool operator<=(const TCVector& right) const;
	bool operator>=(const TCVector& right) const;
	bool operator<(const TCVector& right) const;
	bool operator>(const TCVector& right) const;
	//int operator>(const TCVector& right) const;
	//int operator<=(const TCVector& right) const;
	//int operator>=(const TCVector& right) const;
	TCFloat& operator[](int i) {return vector[i]; }
	const TCFloat& operator[](int i) const { return vector[i]; }
	operator GlPt(void) {return vector;}
	operator ConstGlPt(void) const {return vector;}
	TCVector mult(TCFloat* matrix) const;
	TCVector mult2(TCFloat* matrix) const;
	void transformPoint(const TCFloat *matrix, TCVector &newPoint) const;
	TCVector transformPoint(const TCFloat *matrix) const;
	void transformNormal(const TCFloat *matrix, TCVector& newNormal,
		bool shouldNormalize = true) const;
	TCVector transformNormal(const TCFloat *matrix,
		bool shouldNormalize = true) const;
	TCVector rearrange(int x, int y, int z) const;
	void upConvert(double *doubleVector);
	TCFloat distToLine(const TCVector &p1, const TCVector &p2,
		bool bSegment = false);

	static TCFloat determinant(const TCFloat *matrix);
	static void multMatrix(const TCFloat *left, const TCFloat *right,
		TCFloat *result);
	static void multMatrixd(const double *left, const double *right,
		double *result);
	static TCFloat invertMatrix(const TCFloat *matrix, TCFloat *inverseMatrix);
	static void initIdentityMatrix(TCFloat*);
	static const TCFloat *getIdentityMatrix(void);
	static void doubleNormalize(double *v);
	static void doubleCross(const double *v1, const double *v2, double *v3);
	static void doubleAdd(const double *v1, const double *v2, double *v3);
	static void doubleMultiply(const double *v1, double *v2, double n);
	static double doubleLength(const double *v);
	static void fixPerpendicular(const double *v1, double *v2);
	static void calcScaleMatrix(TCFloat amount, TCFloat *scaleMatrix,
		const TCVector &boundingMin, const TCVector &boundingMax);
	static void calcRotationMatrix(TCFloat latitude, TCFloat longitude,
		TCFloat *rotationMatrix);
	static TCFloat getEpsilon(void) { return sm_epsilon; }
	static void setEpsilon(TCFloat value)
	{
		sm_epsilon = value;
		if (sm_epsilon != 0.0)
		{
			sm_invEpsilon = 1.0f / sm_epsilon;
		}
	}
protected:
	static TCFloat epRound(TCFloat value);
#ifdef _LEAK_DEBUG
	char className[4];
#endif
	TCFloat vector[3];
	static TCFloat sm_identityMatrix[16];
	static TCFloat sm_epsilon;
	static TCFloat sm_invEpsilon;
};

// Overloaded Operator with non-TCVector as first argument
TCVector operator*(TCFloat, const TCVector&);

#endif // __TCVECTOR_H__
