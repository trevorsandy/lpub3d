#include <string.h>
#include "LDLCamera.h"
#include <TCFoundation/TCMacros.h>
#include <TCFoundation/mystring.h>

#ifdef WIN32
#if defined(_MSC_VER) && _MSC_VER >= 1400 && defined(_DEBUG)
#define new DEBUG_CLIENTBLOCK
#endif // _DEBUG
#endif // WIN32

LDLCamera::LDLCamera(void)
	:name(NULL)
{
}

LDLCamera::LDLCamera(const LDLCamera &other)
	:name(NULL)
{
	duplicate(other);
}

LDLCamera::~LDLCamera(void)
{
	delete[] name;
}

LDLCamera& LDLCamera::duplicate(const LDLCamera& copyFrom)
{
	position = copyFrom.position;
	facing = copyFrom.facing;
	setName(copyFrom.name);
	return *this;
}

void LDLCamera::setName(char* n)
{
	delete[] name;
	name = copyString(n);
}

void LDLCamera::rotate(const TCVector &rotation)
{
	TCFloat inverseMatrix[16];

	facing.getInverseMatrix(inverseMatrix);
	if (!fEq(rotation.get(1), 0.0))
	{
		facing = facing +
			LDLFacing(TCVector(-1,0,0).mult(inverseMatrix).normalize(),
			rotation.get(1));
	}
	if (!fEq(rotation.get(0), 0.0))
	{
		facing = facing +
			LDLFacing(TCVector(0,1,0).mult(inverseMatrix).normalize(),
			rotation.get(0));
	}
	if (!fEq(rotation.get(2), 0.0))
	{
		facing = facing +
			LDLFacing(TCVector(0,0,1).mult(inverseMatrix).normalize(),
			rotation.get(2));
	}
}

void LDLCamera::move(const TCVector &distance)
{
	TCFloat inverseMatrix[16];
//	float *matrix;
	TCVector v = distance;

//	matrix = facing.getMatrix();
	facing.getInverseMatrix(inverseMatrix);
	position += v.mult(inverseMatrix);
}
