#ifndef __LDLFACING_H__
#define __LDLFACING_H__

#include <TCFoundation/TCVector.h>

typedef enum Axis {XAxis, YAxis, ZAxis} SimAxis;

class LDLFacing: public TCVector
{
private:
	static TCFloat glMatrix[16];
public:
	LDLFacing(void);
	LDLFacing(TCVector& a, TCFloat phi);
	~LDLFacing(void) {}

	// this creates a gl-compatible 4x4 matrix that you can use to
	// do rotations
	TCFloat* getMatrix(void);

	// this gets the inverse matrix -- it _should_ rotate away for
	//		the object.  (This might be used in camera translations)
	void getInverseMatrix(TCFloat *inverseMatrix);

	// setFacing sets a vector to be a rotation around vector v by phi
	//		radians.
	void setFacing(const TCVector& v, TCFloat phi);

	// this multiplies two facings together.
	LDLFacing operator+(const LDLFacing& otherFacing);

	// normalize a facing.  Facings should automatically be
	//		normalized, but the user might want to call this every
	//		so often to get rid of floating point round off.
	LDLFacing& normalize(void);

	// getVector returns a vector pointing in the direction of the facing.
	//		< 0, 0, 1 > is asumed to be a rotation of 0.
	TCVector getVector(void) const;

	TCFloat getRotation(void) { return rotation; }

	// this gets the angle between two facings, ignoring the top of head.
	//		Not fully tested at this time.
	TCFloat angleBetween(const LDLFacing &f2);

	// this returns the inverse facing.
	LDLFacing inverse(void);

	// mult multiplies two facings together.  You get the same
	//		result as if you multiplied the two rotation matricies
	//		together.
	LDLFacing mult(const LDLFacing& f2);

	// I dont' know why dot returns a facing -- this was in
	//		the code I grabbed from graphicsGems.  Don't try this
	//		before checking the answer.
	LDLFacing dot(LDLFacing& f2);

	// This negates a facing.  I don't know the difference geometrically
	//		between a negate and an inverse -- but there is a difference.
	LDLFacing negate(void);

	// this THEORETICALLY works.  Give it a vector, and this SHOULD
	//		make the current rotation point at the vector, maintaining
	//		the top of head.  Note, SHOULD.  I tried testing, and
	//		this seemed to work.
	void pointAt(const TCVector &v2);

	// this function returns the difference between two facings
	TCVector difference(const LDLFacing &from);

	// this prints a facing's data.  Use mostly for bug testing.
	void print(FILE* = stdout);

	// Note: NOT virtual.  TCVector doesn't have any virtual member
	// functions, and adding one would create a vtable, which we don't
	// want.  Right now, TCVector looks the same in memory as an array of
	// 3 floats.
	TCFloat& operator[](int i);

protected:
	TCFloat* invertMatrix(TCFloat*);
	void swapMatrixRows(TCFloat*, int, int);
	// this is the 4th number in the 4 TCFloat array of a quaternion.
	// (the other 3 are in the private vector superclass.
	TCFloat rotation;
};

#endif // __LDLFACING_H__
