//#include <math.h>
//#include <stdio.h>

#include <array>
#include "TCVector.h"
#include "TCMacros.h"
#include "mystring.h"
#include <assert.h>

#ifdef WIN32
#if defined(_MSC_VER) && _MSC_VER >= 1400 && defined(_DEBUG)
#define new DEBUG_CLIENTBLOCK
#endif // _DEBUG
#endif // WIN32

//static int vectorCount = 0;

TCFloat TCVector::sm_identityMatrix[16] =
{
	1.0, 0.0, 0.0, 0.0,
	0.0, 1.0, 0.0, 0.0,
	0.0, 0.0, 1.0, 0.0,
	0.0, 0.0, 0.0, 1.0
};
TCFloat TCVector::sm_epsilon = 0.0;
TCFloat TCVector::sm_invEpsilon = 1.0;

// TCVector::TCVector(void) -- Default Constructor
TCVector::TCVector(void)
{
#ifdef _LEAK_DEBUG
	className[0] = 'V';
	className[1] = 'e';
	className[2] = 'c';
	className[3] = 0;
#endif
	vector[0] = (TCFloat)0.0;
	vector[1] = (TCFloat)0.0;
	vector[2] = (TCFloat)0.0;
//	vectorCount++;
}

// TCVector::TCVector(TCFloat, TCFloat, TCFloat) -- Constructor
// Expects:
//		x, y, z	: The coordinates to store in the TCVector
TCVector::TCVector(TCFloat x, TCFloat y, TCFloat z)
{
#ifdef _LEAK_DEBUG
	className[0] = 'V';
	className[1] = 'e';
	className[2] = 'c';
	className[3] = 0;
#endif
	vector[0] = x;
	vector[1] = y;
	vector[2] = z;
//	vectorCount++;
}

// TCVector::TCVector(TCFloat *) -- Constructor
// Expects:
//		v	: An array of 3 TCFloats to store in the TCVector
TCVector::TCVector(const TCFloat *v)
{
#ifdef _LEAK_DEBUG
	className[0] = 'V';
	className[1] = 'e';
	className[2] = 'c';
	className[3] = 0;
#endif
	memcpy(vector, v, sizeof(vector));
//	vectorCount++;
}

// TCVector::TCVector(const TCVector&) -- Copy Constructor
// Expects:
//		v	: The TCVector to copy.
TCVector::TCVector(const TCVector& v)
{
#ifdef _LEAK_DEBUG
	className[0] = 'V';
	className[1] = 'e';
	className[2] = 'c';
	className[3] = 0;
#endif
	vector[0] = v.vector[0];
	vector[1] = v.vector[1];
	vector[2] = v.vector[2];
//	vectorCount++;
}

TCVector::~TCVector(void)
{
//	vectorCount--;
//	if (vectorCount < 10 || vectorCount % 1000 == 0)
//	{
//		printf("~TCVector: %d\n", vectorCount);
//	}
}

// TCVector::lengthSquared(void) -- Member Function
// Returns:
//		The square of the length of the TCVector (avoids sqrt if you don't need
//		actual length, only to compare two relative lengths.
TCFloat TCVector::lengthSquared(void) const
{
	return sqr(vector[0]) + sqr(vector[1]) + sqr(vector[2]);
}

// TCVector::length(void) -- Member Function
// Returns:
//		The length of the TCVector.
TCFloat TCVector::length(void) const
{
	return (TCFloat)sqrt(sqr(vector[0]) + sqr(vector[1]) + sqr(vector[2]));
}

// TCVector::dot(const TCVector&) -- Member Function
// Expects:
//		right	: The right hand side of the dot product.
// Returns:
//		The dot product of "*this" dot "right".
TCFloat TCVector::dot(const TCVector& right) const
{
	return vector[0] * right.vector[0] + vector[1] * right.vector[1] +
			 vector[2] * right.vector[2];
}

// TCVector::operator*(const TCVector&) -- Overloaded Operator
// Expects:
//		right	: The right hand side of the cross product.
// Returns:
//		The cross product of "*this" cross "right".
TCVector TCVector::operator*(const TCVector& right) const
{
	// <a, b, c> x <d, e, f> = <bf-ce, cd-af, ae-bd>
	// <x1, y1, z1> x <x2, y2, z2> = <y1*z2-y2*z1, z1*x2-z2*x1, x1*y2-x2*y1>
	return TCVector(vector[1] * right.vector[2] - vector[2] * right.vector[1],
		vector[2] * right.vector[0] - vector[0] * right.vector[2],
		vector[0] * right.vector[1] - vector[1] * right.vector[0]);
}

// TCVector::operator*(TCFloat) -- Overloaded Operator
// Expects:
//		right	: The right hand side of the scalar multiply.
// Returns:
//		The TCVector multiplied by the scalar "right".
TCVector TCVector::operator*(TCFloat right) const
{
	return TCVector(vector[0]*right, vector[1]*right, vector[2]*right);
}

// TCVector::operator/(TCFloat) -- Overloaded Operator
// Expects:
//		right	: The right hand side of the scalar divide.
// Returns:
//		The TCVector divided by the scalar "right".
TCVector TCVector::operator/(TCFloat right) const
{
	// Note: this is done so we can perform 3 multiplies and a divide instead of
	// three divides.  My understanding is that FP divides can take
	// significantly longer than FP multiplies.
	TCFloat mult = 1.0f / right;

	return TCVector(vector[0]*mult, vector[1]*mult, vector[2]*mult);
}

// TCVector::operator+(const TCVector&) -- Overloaded Operator
// Expects:
//		right	: The right hand side of the addition
//	Returns:
//		The TCVector added value "*this" + "right".
TCVector TCVector::operator+(const TCVector& right) const
{
	return TCVector(vector[0] + right.vector[0], vector[1] + right.vector[1],
		vector[2] + right.vector[2]);
}

// TCVector::operator-(const TCVector&) -- Overloaded Operator
// Expects:
//		right	: The right hand side of the subtraction
//	Returns:
//		The TCVector subtracted value "*this" - "right".
TCVector TCVector::operator-(const TCVector& right) const
{
	return TCVector(vector[0] - right.vector[0], vector[1] - right.vector[1],
		vector[2] - right.vector[2]);
}

// TCVector::operator-(void) -- Overloaded Operator
// Returns:
//		The TCVector -"*this".
// Results:
//		This is the unary minus operator
TCVector TCVector::operator-(void) const
{
	return TCVector(-vector[0], -vector[1], -vector[2]);
}

// TCVector::operator*=(const TCVector&) -- Overloaded Operator
// Expects:
//		right	: The right hand side of the cross product *=
// Returns:
//		*this, after it has been set to be the cross product of "*this" cross
//		"right"
TCVector& TCVector::operator*=(const TCVector& right)
{
	vector[0] = vector[1] * right.vector[2] - vector[2] * right.vector[1];
	vector[1] = vector[2] * right.vector[0] - vector[0] * right.vector[2];
	vector[2] = vector[0] * right.vector[1] - vector[1] * right.vector[0];
	return *this;
}

// TCVector::operator+=(const TCVector&) -- Overloaded Operator
// Expects:
//		right	: The right hand side of the +=
// Returns:
//		*this, after "right" has been added to it.
TCVector& TCVector::operator+=(const TCVector& right)
{
	vector[0] += right.vector[0];
	vector[1] += right.vector[1];
	vector[2] += right.vector[2];
	return *this;
}

// TCVector::operator-=(const TCVector&) -- Overloaded Operator
// Expects:
//		right	: The right hand side of the -=
// Returns:
//		*this, after "right" has been subtracted from it.
TCVector& TCVector::operator-=(const TCVector& right)
{
	vector[0] -= right.vector[0];
	vector[1] -= right.vector[1];
	vector[2] -= right.vector[2];
	return *this;
}

// TCVector::operator*=(const TCVector&) -- Overloaded Operator
// Expects:
//		right	: The right hand side of the scalar *=
// Returns:
//		*this, after it has been multiplied by the scalar "right".
TCVector& TCVector::operator*=(TCFloat right)
{
	vector[0] *= right;
	vector[1] *= right;
	vector[2] *= right;
	return *this;
}

// TCVector::operator/=(const TCVector&) -- Overloaded Operator
// Expects:
//		right	: The right hand side of the scalar /=
// Returns:
//		*this, after it has been divided by the scalar "right".
TCVector& TCVector::operator/=(TCFloat right)
{
	// Note: this is done so we can perform 3 multiplies and a divide instead of
	// three divides.  My understanding is that FP divides can take
	// significantly longer than FP multiplies.
	TCFloat mult = 1.0f / right;

	vector[0] *= mult;
	vector[1] *= mult;
	vector[2] *= mult;
	return *this;
}

// TCVector::operator=(const TCVector&) -- Overloaded Operator
// Expects:
//		right	: The vector copy the value of.
// Returns:
//		*this, which has the value of "right".
TCVector& TCVector::operator=(const TCVector& right)
{
	vector[0] = right.vector[0];
	vector[1] = right.vector[1];
	vector[2] = right.vector[2];
	return *this;
}

TCFloat TCVector::epRound(TCFloat value)
{
	if (sm_epsilon == 0.0)
	{
		return value;
	}
	else
	{
		int whole = (int)value;
		int frac = (int)((value - whole + sm_epsilon / 2.0f) * sm_invEpsilon);

		TCFloat retVal = whole + frac / sm_invEpsilon;
		return retVal;
		//return whole + frac / sm_invEpsilon;
		//return (int)((value + sm_epsilon / 2.0f) * sm_invEpsilon) /
		//	sm_invEpsilon;
	}
}

// TCVector::operator==(const TCVector&) -- Overloaded Operator
// Expects:
//		right	: The right hand side of the equality test.
// Returns:
//		If the contents of "*this" equal the contents of "right", then 1.
//		Otherwise, 0.  The contents are defined to be equal if they are within a
//		small distance from each other (based on the default epsilon).
int TCVector::operator==(const TCVector& right) const
{
	return fEq(epRound(vector[0]), epRound(right.vector[0])) &&
	       fEq(epRound(vector[1]), epRound(right.vector[1])) &&
	       fEq(epRound(vector[2]), epRound(right.vector[2]));
	//if (sm_epsilon == 0.0)
	//{
	//	return fEq(vector[0], right.vector[0]) &&
	//		fEq(vector[1], right.vector[1]) &&
	//		fEq(vector[2], right.vector[2]);
	//}
	//else
	//{
	//	return fEq2(vector[0], right.vector[0], sm_epsilon) &&
	//		fEq2(vector[1], right.vector[1], sm_epsilon) &&
	//		fEq2(vector[2], right.vector[2], sm_epsilon);
	//}
}

// TCVector::operator!=(const TCVector&) -- Overloaded Operator
// Expects:
//		right	: The right hand side of the inequality test.
// Returns:
//		If the contents of "*this" do not equal the contents of "right", then 1.
//		Otherwise, 0.  The contents are defined to be inequal if they are within
//		a very small distance from each other.
int TCVector::operator!=(const TCVector& right) const
{
	return !(*this == right);
}

// TCVector::operator<(const TCVector&) -- Overloaded Operator
// Expects:
//		right	: The right hand side of the < test.
// Returns:
//		If the contents of "*this" < the contents of "right", then 1.
//		Otherwise, 0.
bool TCVector::operator<(const TCVector& right) const
{
	if (sm_epsilon == 0.0)
	{
		if (vector[0] < right.vector[0])
		{
			return true;
		}
		else if (vector[0] > right.vector[0])
		{
			return false;
		}
		else if (vector[1] < right.vector[1])
		{
			return true;
		}
		else if (vector[1] > right.vector[1])
		{
			return false;
		}
		else if (vector[2] < right.vector[2])
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		if (epRound(vector[0]) < epRound(right.vector[0]))
		{
			return true;
		}
		else if (epRound(vector[0]) > epRound(right.vector[0]))
		{
			return false;
		}
		else if (epRound(vector[1]) < epRound(right.vector[1]))
		{
			return true;
		}
		else if (epRound(vector[1]) > epRound(right.vector[1]))
		{
			return false;
		}
		else if (epRound(vector[2]) < epRound(right.vector[2]))
		{
			return true;
		}
		else
		{
			return false;
		}
		//if (vector[0] < right.vector[0] - sm_epsilon)
		//{
		//	return true;
		//}
		//else if (vector[0] > right.vector[0] + sm_epsilon)
		//{
		//	return false;
		//}
		//else if (vector[1] < right.vector[1] - sm_epsilon)
		//{
		//	return true;
		//}
		//else if (vector[1] > right.vector[1] + sm_epsilon)
		//{
		//	return false;
		//}
		//else if (vector[2] < right.vector[2] - sm_epsilon)
		//{
		//	return true;
		//}
		//else
		//{
		//	return false;
		//}
	}
}

bool TCVector::operator>(const TCVector& right) const
{
	return right < *this;
}

bool TCVector::operator<=(const TCVector& right) const
{
	return !(*this > right);
}

bool TCVector::operator>=(const TCVector& right) const
{
	return !(*this < right);
}

// TCVector::operator<(const TCVector&) -- Overloaded Operator
// Expects:
//		right	: The right hand side of the < test.
// Returns:
//		If the contents of "*this" < the contents of "right", then 1.
//		Otherwise, 0.
//int TCVector::operator<(const TCVector& right) const
//{
//	return length() < right.length();
//}
//
// TCVector::operator>(const TCVector&) -- Overloaded Operator
// Expects:
//		right	: The right hand side of the > test.
// Returns:
//		If the contents of "*this" > the contents of "right", then 1.
//		Otherwise, 0.
//int TCVector::operator>(const TCVector& right) const
//{
//	return length() > right.length();
//}
//
// TCVector::operator<=(const TCVector&) -- Overloaded Operator
// Expects:
//		right	: The right hand side of the <= test.
// Returns:
//		If the contents of "*this" <= the contents of "right", then 1.
//		Otherwise, 0.
//int TCVector::operator<=(const TCVector& right) const
//{
//	return length() <= right.length();
//}
//
// TCVector::operator>=(const TCVector&) -- Overloaded Operator
// Expects:
//		right	: The right hand side of the >= test.
// Returns:
//		If the contents of "*this" >= the contents of "right", then 1.
//		Otherwise, 0.
//int TCVector::operator>=(const TCVector& right) const
//{
//	return length() >= right.length();
//}

// TCVector::print(FILE*) -- Member Function
// Expects:
//		outFile	: File to output to.
// Results:
//		Prints the TCVector.  Does NOT print a trailing line feed.
void TCVector::print(FILE* outFile) const
{
	fprintf(outFile, "%f %f %f", vector[0], vector[1], vector[2]);
}

void TCVector::print(char* buffer, int precision) const
{
	sprintf(buffer, "%s %s %s", ftostr(vector[0], precision).c_str(),
		ftostr(vector[1], precision).c_str(),
		ftostr(vector[2], precision).c_str());
}

void TCVector::print(UCSTR buffer, size_t bufferSize, int precision) const
{
	sucprintf(buffer, bufferSize, _UC("%s %s %s"), ftoucstr(vector[0], precision).c_str(),
		ftoucstr(vector[1], precision).c_str(),
		ftoucstr(vector[2], precision).c_str());
}

std::string TCVector::string(int precision) const
{
	char buf[1024];

	print(buf, precision);
	return buf;
}

::ucstring TCVector::ucstring(int precision) const
{
#ifdef TC_NO_UNICODE
	return string(precision);
#else // TC_NO_UNICODE
	std::string astring = string(precision);
	std::wstring wstring;

	stringtowstring(wstring, astring);
	return wstring;
#endif // TC_NO_UNICODE
}

// operator*(TCFloat, TCVector&) -- Overloaded Non-Member Operator
// Expects:
//		left	: The left hand side of the scalar multiply.
//		right	: The right hand side of the scalar multiply.
// Returns:
//		The TCVector resulting from "right" being multiplied by the scalar
//		"left".
TCVector operator*(TCFloat left, const TCVector& right)
{
	return right * left;
}

TCVector TCVector::mult2(TCFloat* matrix) const
{
	TCVector tempVec;

	tempVec[0] = vector[0]*matrix[0] + vector[0]*matrix[1] +
	 vector[0]*matrix[2] + matrix[3];
	tempVec[1] = vector[1]*matrix[4] + vector[1]*matrix[5] +
	 vector[1]*matrix[6] + matrix[7];
	tempVec[2] = vector[2]*matrix[8] + vector[2]*matrix[9] +
	 vector[2]*matrix[10] + matrix[11];
	return tempVec;
}


TCVector TCVector::mult(TCFloat* matrix) const
{
	TCVector tempVec;

	tempVec[0] = vector[0]*matrix[0] + vector[1]*matrix[1] +
	 vector[2]*matrix[2] + matrix[3];
	tempVec[1] = vector[0]*matrix[4] + vector[1]*matrix[5] +
	 vector[2]*matrix[6] + matrix[7];
	tempVec[2] = vector[0]*matrix[8] + vector[1]*matrix[9] +
	 vector[2]*matrix[10] + matrix[11];
	return tempVec;
}

TCFloat TCVector::determinant(const TCFloat *matrix)
{
	TCFloat det;

	det = matrix[0] * (matrix[5] * matrix[10] - matrix[6] * matrix[9]);
	det -= matrix[4] * (matrix[1] * matrix[10] - matrix[2] * matrix[9]);
	det += matrix[8] * (matrix[1] * matrix[6] - matrix[2] * matrix[5]);
	return det;
}

TCFloat TCVector::invertMatrix(const TCFloat *matrix, TCFloat *inverseMatrix)
{
	TCFloat det = determinant(matrix);

	if (fEq2(det, 0.0f, 1e-8))
	{
		memset(inverseMatrix, 0, sizeof(TCFloat) * 16);
		debugPrintf("Error: Matrix inversion failed.\n");
	}
	else
	{
		det = 1.0f / det;

		inverseMatrix[0]  =  (matrix[5] * matrix[10] - matrix[6] * matrix[9]) *
			det;
		inverseMatrix[1]  = -(matrix[1] * matrix[10] - matrix[2] * matrix[9]) *
			det;
		inverseMatrix[2]  =  (matrix[1] * matrix[6]  - matrix[2] * matrix[5]) *
			det;
		inverseMatrix[4]  = -(matrix[4] * matrix[10] - matrix[6] * matrix[8]) *
			det;
		inverseMatrix[5]  =  (matrix[0] * matrix[10] - matrix[2] * matrix[8]) *
			det;
		inverseMatrix[6]  = -(matrix[0] * matrix[6]  - matrix[2] * matrix[4]) *
			det;
		inverseMatrix[8]  =  (matrix[4] * matrix[9]  - matrix[5] * matrix[8]) *
			det;
		inverseMatrix[9]  = -(matrix[0] * matrix[9]  - matrix[1] * matrix[8]) *
			det;
		inverseMatrix[10] =  (matrix[0] * matrix[5]  - matrix[1] * matrix[4]) *
			det;

		inverseMatrix[12] = -(matrix[12] * matrix[0] + matrix[13] * matrix[4] +
							matrix[14] * matrix[8]);
		inverseMatrix[13] = -(matrix[12] * matrix[1] + matrix[13] * matrix[5] +
							matrix[14] * matrix[9]);
		inverseMatrix[14] = -(matrix[12] * matrix[2] + matrix[13] * matrix[6] +
							matrix[14] * matrix[10]);

		inverseMatrix[3] = inverseMatrix[7] = inverseMatrix[11] = 0.0;
		inverseMatrix[15] = 1.0;
	}
	return det;
}

void TCVector::multMatrix(const TCFloat* left, const TCFloat* right, TCFloat* result)
{
	result[0] = left[0] * right[0] + left[4] * right[1] +
		left[8] * right[2] + left[12] * right[3];
	result[1] = left[1] * right[0] + left[5] * right[1] +
		left[9] * right[2] + left[13] * right[3];
	result[2] = left[2] * right[0] + left[6] * right[1] +
		left[10] * right[2] + left[14] * right[3];
	result[3] = left[3] * right[0] + left[7] * right[1] +
		left[11] * right[2] + left[15] * right[3];
	result[4] = left[0] * right[4] + left[4] * right[5] +
		left[8] * right[6] + left[12] * right[7];
	result[5] = left[1] * right[4] + left[5] * right[5] +
		left[9] * right[6] + left[13] * right[7];
	result[6] = left[2] * right[4] + left[6] * right[5] +
		left[10] * right[6] + left[14] * right[7];
	result[7] = left[3] * right[4] + left[7] * right[5] +
		left[11] * right[6] + left[15] * right[7];
	result[8] = left[0] * right[8] + left[4] * right[9] +
		left[8] * right[10] + left[12] * right[11];
	result[9] = left[1] * right[8] + left[5] * right[9] +
		left[9] * right[10] + left[13] * right[11];
	result[10] = left[2] * right[8] + left[6] * right[9] +
		left[10] * right[10] + left[14] * right[11];
	result[11] = left[3] * right[8] + left[7] * right[9] +
		left[11] * right[10] + left[15] * right[11];
	result[12] = left[0] * right[12] + left[4] * right[13] +
		left[8] * right[14] + left[12] * right[15];
	result[13] = left[1] * right[12] + left[5] * right[13] +
		left[9] * right[14] + left[13] * right[15];
	result[14] = left[2] * right[12] + left[6] * right[13] +
		left[10] * right[14] + left[14] * right[15];
	result[15] = left[3] * right[12] + left[7] * right[13] +
		left[11] * right[14] + left[15] * right[15];
}

void TCVector::multMatrixd(const double *left, const double *right,
						   double *result)
{
	result[0] = left[0] * right[0] + left[4] * right[1] +
		left[8] * right[2] + left[12] * right[3];
	result[1] = left[1] * right[0] + left[5] * right[1] +
		left[9] * right[2] + left[13] * right[3];
	result[2] = left[2] * right[0] + left[6] * right[1] +
		left[10] * right[2] + left[14] * right[3];
	result[3] = left[3] * right[0] + left[7] * right[1] +
		left[11] * right[2] + left[15] * right[3];
	result[4] = left[0] * right[4] + left[4] * right[5] +
		left[8] * right[6] + left[12] * right[7];
	result[5] = left[1] * right[4] + left[5] * right[5] +
		left[9] * right[6] + left[13] * right[7];
	result[6] = left[2] * right[4] + left[6] * right[5] +
		left[10] * right[6] + left[14] * right[7];
	result[7] = left[3] * right[4] + left[7] * right[5] +
		left[11] * right[6] + left[15] * right[7];
	result[8] = left[0] * right[8] + left[4] * right[9] +
		left[8] * right[10] + left[12] * right[11];
	result[9] = left[1] * right[8] + left[5] * right[9] +
		left[9] * right[10] + left[13] * right[11];
	result[10] = left[2] * right[8] + left[6] * right[9] +
		left[10] * right[10] + left[14] * right[11];
	result[11] = left[3] * right[8] + left[7] * right[9] +
		left[11] * right[10] + left[15] * right[11];
	result[12] = left[0] * right[12] + left[4] * right[13] +
		left[8] * right[14] + left[12] * right[15];
	result[13] = left[1] * right[12] + left[5] * right[13] +
		left[9] * right[14] + left[13] * right[15];
	result[14] = left[2] * right[12] + left[6] * right[13] +
		left[10] * right[14] + left[14] * right[15];
	result[15] = left[3] * right[12] + left[7] * right[13] +
		left[11] * right[14] + left[15] * right[15];
}

bool TCVector::exactlyEquals(const TCVector &right) const
{
	return vector[0] == right.vector[0] && vector[1] == right.vector[1] &&
		vector[2] == right.vector[2];
}

bool TCVector::approxEquals(const TCVector &right, TCFloat epsilon) const
{
	return fEq2(vector[0], right.vector[0], epsilon) &&
		fEq2(vector[1], right.vector[1], epsilon) &&
		fEq2(vector[2], right.vector[2], epsilon);
}

void TCVector::initIdentityMatrix(TCFloat *matrix)
{
	memcpy(matrix, sm_identityMatrix, sizeof(sm_identityMatrix));
/*
	memset(matrix, 0, 16 * sizeof(TCFloat));
	matrix[0] = 1.0f;
	matrix[5] = 1.0f;
	matrix[10] = 1.0f;
	matrix[15] = 1.0f;
*/
}

void TCVector::transformPoint(const TCFloat *matrix, TCVector &newPoint) const
{
	TCFloat x = vector[0];
	TCFloat y = vector[1];
	TCFloat z = vector[2];

//	x' = a*x + b*y + c*z + X
//	y' = d*x + e*y + f*z + Y
//	z' = g*x + h*y + i*z + Z
	newPoint[0] = matrix[0]*x + matrix[4]*y + matrix[8]*z + matrix[12];
	newPoint[1] = matrix[1]*x + matrix[5]*y + matrix[9]*z + matrix[13];
	newPoint[2] = matrix[2]*x + matrix[6]*y + matrix[10]*z + matrix[14];
}

TCVector TCVector::transformPoint(const TCFloat *matrix) const
{
	TCVector newPoint;

	transformPoint(matrix, newPoint);
	return newPoint;
}

void TCVector::transformNormal(const TCFloat *matrix, TCVector& newNormal,
							   bool shouldNormalize) const
{
	TCFloat inverseMatrix[16];
	TCFloat x = vector[0];
	TCFloat y = vector[1];
	TCFloat z = vector[2];

	invertMatrix(matrix, inverseMatrix);
//	x' = a*x + b*y + c*z + X
//	y' = d*x + e*y + f*z + Y
//	z' = g*x + h*y + i*z + Z
//	newNormal[0] = inverseMatrix[0]*x + inverseMatrix[4]*y + inverseMatrix[8]*z;
//	newNormal[1] = inverseMatrix[1]*x + inverseMatrix[5]*y + inverseMatrix[9]*z;
//	newNormal[2] = inverseMatrix[2]*x + inverseMatrix[6]*y + inverseMatrix[10]*z;
	newNormal[0] = inverseMatrix[0]*x + inverseMatrix[1]*y + inverseMatrix[2]*z;
	newNormal[1] = inverseMatrix[4]*x + inverseMatrix[5]*y + inverseMatrix[6]*z;
	newNormal[2] = inverseMatrix[8]*x + inverseMatrix[9]*y + inverseMatrix[10]*z;
	if (shouldNormalize)
	{
		newNormal.normalize();
	}
}

TCVector TCVector::transformNormal(
	const TCFloat *matrix,
	bool shouldNormalize) const
{
	TCVector newNormal;

	transformNormal(matrix, newNormal, shouldNormalize);
	return newNormal;
}

TCVector TCVector::rearrange(int x, int y, int z) const
{
	return TCVector(get(x), get(y), get(z));
}

const TCFloat *TCVector::getIdentityMatrix(void)
{
	return sm_identityMatrix;
}

// Convert ourselves into a double array.
void TCVector::upConvert(double *doubleVector)
{
	doubleVector[0] = (double)vector[0];
	doubleVector[1] = (double)vector[1];
	doubleVector[2] = (double)vector[2];
}

// Take two double-precision arrays, and adjust the second one slightly so that
// it is perpendicular to the first.  This assumes that v1 are nearly
// perpendicular to begin with.  Otherwise, it might not work.
void TCVector::fixPerpendicular(const double *v1, double *v2)
{
	double epsilon = 1e-15;
	double pairs[3];
	double dot = 0.0;
	int i;

	// The dot product of perpendicular vectors is zero, so first calculate
	// the dot product.
	for (i = 0; i < 3; i++)
	{
		pairs[i] = v1[i] * v2[i];
		dot += pairs[i];
	}
	// If it's already zero, then we don't have to do anything.
	if (!fEq2(dot, 0.0f, epsilon))
	{
		int index0;
		int index1;
		int index2;

		// If the vectors were nearly perpendicular, then either two of the
		// pairs will be negative and one positive, or two of the pairs of the
		// pairs will be positive and one negative.  Assuming that none of the
		// pairs is exactly zero, then find the odd pair.  If one of the pairs
		// is zero, find either one of the other pairs.
		if ((pairs[0] < 0.0 && pairs[1] >= 0.0 && pairs[2] >= 0) ||
			(pairs[0] > 0.0 && pairs[1] <= 0.0 && pairs[2] <= 0.0))
		{
			index0 = 0;
		}
		else if ((pairs[1] < 0.0 && pairs[0] >= 0.0 && pairs[2] >= 0) ||
			(pairs[1] > 0.0 && pairs[0] <= 0.0 && pairs[2] <= 0.0))
		{
			index0 = 1;
		}
		else
		{
			index0 = 2;
		}
		index1 = (index0 + 1) % 3;
		index2 = (index0 + 2) % 3;
		// Adjust the component we decided upon so that the dot product will
		// equal zero again.
		v2[index0] = (pairs[index1] + pairs[index2]) / -v1[index0];
		// Re-normalize our adjusted vector.  Remember that normalizing a vector
		// doesn't have any effect on the angle between it and another vector,
		// so the dot product will remain zero in this case.  This wouldn't work
		// with any other angles, since the dot product would then be dependent
		// on the lengths of the vectors, but with perpendicular vectors, the
		// lengths of the vectors have no effect on the dot product.
		doubleNormalize(v2);
	}
}

void TCVector::doubleNormalize(double *v)
{
	double invLength = 1.0 / sqrt(sqr(v[0]) + sqr(v[1]) + sqr(v[2]));

	v[0] *= invLength;
	v[1] *= invLength;
	v[2] *= invLength;
}

// It didn't even enter my mind what this method would be named when I
// implemented doubleNormalize.  At that point, I didn't even think I'd NEED a
// cross product for double vectors. :-)
void TCVector::doubleCross(const double *v1, const double *v2, double *v3)
{
	v3[0] = v1[1] * v2[2] - v1[2] * v2[1];
	v3[1] = v1[2] * v2[0] - v1[0] * v2[2];
	v3[2] = v1[0] * v2[1] - v1[1] * v2[0];
}

void TCVector::doubleAdd(const double *v1, const double *v2, double *v3)
{
	v3[0] = v1[0] + v2[0];
	v3[1] = v1[1] + v2[1];
	v3[2] = v1[2] + v2[2];
}

double TCVector::doubleLength(const double *v)
{
	return sqrt(sqr(v[0]) + sqr(v[1]) + sqr(v[2]));
}

void TCVector::doubleMultiply(const double *v1, double *v2, double n)
{
	v2[0] = v1[0] * n;
	v2[1] = v1[1] * n;
	v2[2] = v1[2] * n;
}

/*
Let *this == p0:
    |(p2 - p1) x (p2 - p0)|
d = -----------------------
          |p2 - p1|
*/
TCFloat TCVector::distToLine(
	const TCVector &p1,
	const TCVector &p2,
	bool bSegment /*= true*/)
{
	TCFloat dist = ((p2 - p1) * (p2 - *this)).length() / (p2 - p1).length();
	if (bSegment)
	{
		// Not supported
		assert(false);
	}
	return dist;
}

// Thanks go to Lars again here for the simplified positioning-after-shrinkage
// algorithm.
// Note: static method
void TCVector::calcScaleMatrix(
	TCFloat amount,
	TCFloat *scaleMatrix,
	const TCVector &boundingMin,
	const TCVector &boundingMax)
{
	TCVector center;
	TCVector delta;
	int i;
	
	TCVector::initIdentityMatrix(scaleMatrix);
	delta = boundingMax - boundingMin;
	center = (boundingMin + boundingMax) / 2.0f;
	for (i = 0; i < 3; i++)
	{
		if (delta[i] > amount)
		{
			scaleMatrix[i * 4 + i] = 1.0f - amount / delta[i];
			if (center[i])
			{
				scaleMatrix[12 + i] = amount / delta[i] * center[i];
			}
		}
	}
}

// Note: static method
void TCVector::calcRotationMatrix(
	TCFloat latitude,
	TCFloat longitude,
	TCFloat *rotationMatrix)
{
	TCFloat leftMatrix[16];
	TCFloat rightMatrix[16];
	//TCFloat resultMatrix[16];
	TCFloat cosTheta;
	TCFloat sinTheta;
	TCVector::initIdentityMatrix(leftMatrix);
	TCVector::initIdentityMatrix(rightMatrix);
	latitude = (TCFloat)deg2rad(latitude);
	longitude = (TCFloat)deg2rad(longitude);

	// First, apply latitude by rotating around X.
	cosTheta = (TCFloat)cos(latitude);
	sinTheta = (TCFloat)sin(latitude);
	rightMatrix[5] = cosTheta;
	rightMatrix[6] = sinTheta;
	rightMatrix[9] = -sinTheta;
	rightMatrix[10] = cosTheta;
	TCVector::multMatrix(leftMatrix, rightMatrix, rotationMatrix);

	memcpy(leftMatrix, rotationMatrix, sizeof(leftMatrix));
	TCVector::initIdentityMatrix(rightMatrix);

	// Next, apply longitude by rotating around Y.
	cosTheta = (TCFloat)cos(longitude);
	sinTheta = (TCFloat)sin(longitude);
	rightMatrix[0] = cosTheta;
	rightMatrix[2] = -sinTheta;
	rightMatrix[8] = sinTheta;
	rightMatrix[10] = cosTheta;
	TCVector::multMatrix(leftMatrix, rightMatrix, rotationMatrix);
}

TCVector& TCVector::normalize(void)
{
#ifdef _DEBUG
	if (lengthSquared() == 0.0f)
	{
		assert(lengthSquared() > 0.0f);
	}
#endif // _DEBUG
	return *this *= 1.0f / length();
}
