#include "LDLFacing.h"
#include <TCFoundation/TCMacros.h>
#include <string.h>
#include <TCFoundation/mystring.h>

#ifdef WIN32
#if defined(_MSC_VER) && _MSC_VER >= 1400 && defined(_DEBUG)
#define new DEBUG_CLIENTBLOCK
#endif // _DEBUG
#endif // WIN32

TCFloat LDLFacing::glMatrix[16] =
{
	(TCFloat)1.0, (TCFloat)0.0, (TCFloat)0.0, (TCFloat)0.0,
	(TCFloat)0.0, (TCFloat)1.0, (TCFloat)0.0, (TCFloat)0.0,
	(TCFloat)0.0, (TCFloat)0.0, (TCFloat)1.0, (TCFloat)0.0,
	(TCFloat)0.0, (TCFloat)0.0, (TCFloat)0.0, (TCFloat)1.0
};
/*
static TCFloat emptyMatrix[] =
{
	(TCFloat)0.0, (TCFloat)0.0, (TCFloat)0.0, (TCFloat)0.0,
	(TCFloat)0.0, (TCFloat)0.0, (TCFloat)0.0, (TCFloat)0.0,
	(TCFloat)0.0, (TCFloat)0.0, (TCFloat)0.0, (TCFloat)0.0,
	(TCFloat)0.0, (TCFloat)0.0, (TCFloat)0.0, (TCFloat)0.0
};
static TCFloat identityMatrix[] =
{
	(TCFloat)1.0, (TCFloat)0.0, (TCFloat)0.0, (TCFloat)0.0,
	(TCFloat)0.0, (TCFloat)1.0, (TCFloat)0.0, (TCFloat)0.0,
	(TCFloat)0.0, (TCFloat)0.0, (TCFloat)1.0, (TCFloat)0.0,
	(TCFloat)0.0, (TCFloat)0.0, (TCFloat)0.0, (TCFloat)1.0
};
*/

LDLFacing::LDLFacing(void)
	:TCVector((TCFloat)0.0, (TCFloat)0.0, (TCFloat)0.0)
{
	//TCVector tempVec(1, 0, 0);

	//setFacing(tempVec, 0);
	rotation = (TCFloat)1.0;
}

LDLFacing::LDLFacing(TCVector &a, TCFloat phi)
{
	setFacing(a, phi);
}

LDLFacing LDLFacing::operator+(const LDLFacing& otherFacing) 
{
	return mult(otherFacing);
}

TCFloat LDLFacing::angleBetween(const LDLFacing &f2)
{
  //t = (q[0]*r[0])+(q[1]*r[1])+(q[2]*r[2])+(q[3]*r[3]);
  return (this->vector[0]*f2.vector[0]) +
	  (this->vector[1]*f2.vector[1]) +
	  (this->vector[2]*f2.vector[2]) +
	  (this->rotation*f2.rotation);
}

LDLFacing LDLFacing::inverse(void)
{
	LDLFacing newFacing;

	newFacing.vector[0] = -vector[0];
	newFacing.vector[1] = -vector[1];
	newFacing.vector[2] = -vector[2];
	newFacing.rotation = rotation;

	return newFacing;
}

LDLFacing LDLFacing::negate(void)
{
	LDLFacing newFacing;

	newFacing.vector[0] = -vector[0];
	newFacing.vector[1] = -vector[1];
	newFacing.vector[2] = -vector[2];
//	newFacing.rotation = -rotation;

	return newFacing;
}

LDLFacing LDLFacing::mult(const LDLFacing& f2)
{
	LDLFacing temp;

	temp.vector[0] = this->rotation*f2.vector[0] +
		this->vector[0]*f2.rotation +
		this->vector[1]*f2.vector[2] -
		this->vector[2]*f2.vector[1];
	temp.vector[1] = this->rotation*f2.vector[1] +
		this->vector[1]*f2.rotation +
		this->vector[2]*f2.vector[0] -
		this->vector[0]*f2.vector[2];
	temp.vector[2] = this->rotation*f2.vector[2] +
		this->vector[2]*f2.rotation +
		this->vector[0]*f2.vector[1] -
		this->vector[1]*f2.vector[0];
	temp.rotation = this->rotation*f2.rotation -
		this->vector[0]*f2.vector[0] -
		this->vector[1]*f2.vector[1] -
		this->vector[2]*f2.vector[2];

	return temp;
}

LDLFacing LDLFacing::dot(LDLFacing& f2)
{
	LDLFacing answer;
	double temp;
	int i;

	answer.rotation = (this->rotation * f2.rotation) -
		((this->vector[0] * f2.vector[0]) +
		(this->vector[1] * f2.vector[1]) +
		(this->vector[2] * f2.vector[2]));
	answer.vector[0] = (this->rotation * f2.vector[0]) +
		(f2.rotation * this->vector[0]) +
		((this->vector[1] * f2.vector[2]) -
		(this->vector[2] * f2.vector[1]));
	answer.vector[1] = (this->rotation * f2.vector[1]) +
		(f2.rotation * this->vector[1]) +
		((this->vector[2] * f2.vector[0]) -
		(this->vector[0] * f2.vector[2]));
	answer.vector[2] = (this->rotation * f2.vector[2]) +
		(f2.rotation * this->vector[2]) +
		((this->vector[0] * f2.vector[1]) -
		(this->vector[1] * f2.vector[0]));

	/* make sure the resulting quaternion is a unit quaternion */
	temp = sqrt((double)(answer.vector[0]*answer.vector[0] +
		answer.vector[1]*answer.vector[1] +
		answer.vector[2]*answer.vector[2] +
		answer.rotation*answer.rotation));
	for (i=0; i<4; i++) 
	{
		answer[i] = (TCFloat)(answer[i]/temp);
	}
	return answer;
}

void LDLFacing::setFacing(const TCVector &a, TCFloat phi)
{
	double phiOver2 = phi / 2.0;

	//a.normalize(); /* Normalize axis */
	//(TCVector)(*this) = a;
	(*this)[0] = a[0];
	(*this)[1] = a[1];
	(*this)[2] = a[2];
	TCVector::normalize();

	//vscale(e, sin(phi/2.0));
	(*this)[0] *= (TCFloat)sin(phiOver2);
	(*this)[1] *= (TCFloat)sin(phiOver2);
	(*this)[2] *= (TCFloat)sin(phiOver2);

	this->rotation = (TCFloat)cos(phiOver2);
	//printf("LDLFacing set to %f, %f, %f, %f\n", (*this)[0], (*this)[1], 
	// (*this)[2], this->rotation);
}

TCFloat& LDLFacing::operator[](int i)
{
	if (i == 3)
	{
		return rotation;
	}
	else
	{
		return vector[i];
	}
}

LDLFacing& LDLFacing::normalize(void)
{
	int which, i;
	TCFloat gr;

	which = 0;
	gr = (*this)[which];
	for (i = 1 ; i < 4 ; i++)
	{
		if (fabs((*this)[i]) > fabs(gr))
		{
			gr = (*this)[i];
			which = i;
		}
	}
	(*this)[which] = (TCFloat)0.0;   /* So it doesn't affect next operation */

	(*this)[which] = (TCFloat)(sqrt(1.0 - ((*this)[0]*(*this)[0] +
		(*this)[1]*(*this)[1] +
		(*this)[2]*(*this)[2] +
		this->rotation*this->rotation)));

	/* Check to see if we need negative square root */
	if (gr < 0.0)
	{
		(*this)[which] = -(*this)[which];
	}

	return *this;

}

TCFloat* LDLFacing::getMatrix(void)
{
//	return glMatrix;

	glMatrix[0] = (TCFloat)(1 - 2.0 * ((*this)[1] * (*this)[1] + (*this)[2] * (*this)[2]));
	glMatrix[1] = (TCFloat)(2.0 * ((*this)[0] * (*this)[1] - (*this)[2] * this->rotation));
	glMatrix[2] = (TCFloat)(2.0 * ((*this)[2] * (*this)[0] + (*this)[1] * this->rotation));
	glMatrix[3] = (TCFloat)0.0;

	glMatrix[4] = (TCFloat)(2.0 * ((*this)[0] * (*this)[1] + (*this)[2] * this->rotation));
	glMatrix[5] = (TCFloat)(1 - 2.0 * ((*this)[2] * (*this)[2] + (*this)[0] * (*this)[0]));
	glMatrix[6] = (TCFloat)(2.0 * ((*this)[1] * (*this)[2] - (*this)[0] * this->rotation));
	glMatrix[7] = (TCFloat)0.0;

	glMatrix[8] = (TCFloat)(2.0 * ((*this)[2] * (*this)[0] - (*this)[1] * this->rotation));
	glMatrix[9] = (TCFloat)(2.0 * ((*this)[1] * (*this)[2] + (*this)[0] * this->rotation));
	glMatrix[10] = (TCFloat)(1 - 2.0 * ((*this)[1] * (*this)[1] + (*this)[0] * (*this)[0]));
	glMatrix[11] = (TCFloat)0.0;

	glMatrix[12] = (TCFloat)0.0;
	glMatrix[13] = (TCFloat)0.0;
	glMatrix[14] = (TCFloat)0.0;
	glMatrix[15] = (TCFloat)1.0;

	return glMatrix;
}

void LDLFacing::getInverseMatrix(TCFloat *inverseMatrix)
{
	TCVector::invertMatrix(getMatrix(), inverseMatrix);

//	glMatrix[0]=-(1-2.0 * ((*this)[1] * (*this)[1] + (*this)[2] * (*this)[2]));
//	glMatrix[1]=-(2.0*((*this)[0] * (*this)[1] - (*this)[2] * this->rotation));
//	glMatrix[2]=-(2.0*((*this)[2] * (*this)[0] + (*this)[1] * this->rotation));
//	glMatrix[3]= 0.0;
//
//	glMatrix[4]=-(2.0*((*this)[0] * (*this)[1] + (*this)[2] * this->rotation));
//	glMatrix[5]=-(1-2.0 * ((*this)[2] * (*this)[2] + (*this)[0] * (*this)[0]));
//	glMatrix[6]=-(2.0*((*this)[1] * (*this)[2] - (*this)[0] * this->rotation));
//	glMatrix[7]= 0.0;
//
//	glMatrix[8]=-(2.0*((*this)[2] * (*this)[0] - (*this)[1] * this->rotation));
//	glMatrix[9]=-(2.0*((*this)[1] * (*this)[2] + (*this)[0] * this->rotation));
//	glMatrix[10]=-(1-2.0 * ((*this)[1] * (*this)[1] + (*this)[0] * (*this)[0]));
//	glMatrix[11]= 0.0;
//
//	glMatrix[12] = 0.0;
//	glMatrix[13] = 0.0;
//	glMatrix[14] = 0.0;
//	glMatrix[15] = 1.0;
//
//	return glMatrix;
}

TCVector LDLFacing::getVector(void) const
{
	return TCVector(
	 (TCFloat)(2.0*((*this)[2] * (*this)[0] - (*this)[1] * this->rotation)),
	 (TCFloat)(2.0*((*this)[1] * (*this)[2] + (*this)[0] * this->rotation)),
	 (TCFloat)(1-2.0 * ((*this)[1] * (*this)[1] + (*this)[0] * (*this)[0]))).normalize();
	//vec[0] = (*this)[0];
	//vec[1] = (*this)[1];
	//vec[2] = (*this)[2];

	//return vec;
}

void LDLFacing::pointAt(const TCVector &v2)
{
	TCVector tempVec = v2;
	tempVec.normalize();
	TCVector axis = v2 * this->getVector();
	TCFloat radians = (TCFloat)acos(tempVec.dot(this->getVector()));
	this->setFacing(axis, radians);
}

void LDLFacing::print(FILE* outFile)
{
	TCVector::print(outFile);
	fprintf(outFile, " %f", rotation);
}

TCVector LDLFacing::difference(const LDLFacing &from)
{
	TCVector newVec(from.getVector());
	newVec.mult(getMatrix());
	return newVec;
}

void LDLFacing::swapMatrixRows(TCFloat* m, int r1, int r2)
{
	TCFloat tmpRow[4];

	memcpy(tmpRow, m + r1*4, 4*sizeof(TCFloat));
	memmove(m + r1*4, m + r2*4, 4*sizeof(TCFloat));
	memmove(m + r2*4, tmpRow, 4*sizeof(TCFloat));
}

TCFloat* LDLFacing::invertMatrix(TCFloat* inM)
{
	TCFloat* inv;
	TCFloat m[16];
	int i, j;
	TCFloat tmp;

//	inv = new TCFloat[16];
	memcpy(m, inM, sizeof(m));
	inv = glMatrix;
	memcpy(inv, sm_identityMatrix, sizeof(sm_identityMatrix));
	if (fEq(m[0], 0.0))
	{
		for (i = 1; i < 4 && fEq(m[i*4], 0); i++)
			;
		if (i == 4)
		{
			debugPrintf("Error: Matrix inversion failed.\n");
//			exit(1);
		}
		swapMatrixRows(m, 0, i);
		swapMatrixRows(inv, 0, i);
	}
	tmp = m[0];
	if (!fEq(tmp, 1.0))
	{
		for (i = 0; i < 4; i++)
		{
			m[i] /= tmp;
			inv[i] /= tmp;
		}
	}
	for (i = 1; i < 4; i++)
	{
		if (!fEq(m[i*4], 0.0))
		{
			tmp = m[i*4];
			for (j = 0; j < 4; j++)
			{
				m[i*4+j] -= tmp * m[j];
				inv[i*4+j] -= tmp * inv[j];
			}
		}
	}
	if (fEq(m[5], 0.0))
	{
		for (i = 2; i < 4 && fEq(m[i*4+1], 0); i++)
			;
		if (i == 4)
		{
			debugPrintf("Error: Matrix inversion failed.\n");
//			exit(1);
		}
		swapMatrixRows(m, 1, i);
		swapMatrixRows(inv, 1, i);
	}
	tmp = m[5];
	if (!fEq(tmp, 1.0))
	{
		for (i = 4; i < 8; i++)
		{
			m[i] /= tmp;
			inv[i] /= tmp;
		}
	}
	for (i = 2; i < 4; i++)
	{
		if (!fEq(m[i*4+1], 0.0))
		{
			tmp = m[i*4+1];
			for (j = 0; j < 4; j++)
			{
				m[i*4+j] -= tmp * m[j+4];
				inv[i*4+j] -= tmp * inv[j+4];
			}
		}
	}
	if (fEq(m[10], 0.0))
	{
		if (fEq(m[14], 0.0))
		{
			debugPrintf("Error: Matrix inversion failed.\n");
//			exit(1);
		}
		swapMatrixRows(m, 2, 3);
		swapMatrixRows(inv, 2, 3);
	}
	tmp = m[10];
	if (!fEq(tmp, 1.0))
	{
		for (i = 8; i < 12; i++)
		{
			m[i] /= tmp;
			inv[i] /= tmp;
		}
	}
	if (!fEq(m[14], 0.0))
	{
		tmp = m[14];
		for (j = 0; j < 4; j++)
		{
			m[12+j] -= tmp * m[j+8];
			inv[12+j] -= tmp * inv[j+8];
		}
	}
	if (fEq(m[15], 0.0))
	{
		debugPrintf("Error: Matrix inversion failed.\n");
//		exit(1);
	}
	tmp = m[15];
	if (!fEq(tmp, 1.0))
	{
		for (i = 12; i < 16; i++)
		{
			m[i] /= tmp;
			inv[i] /= tmp;
		}
	}
	tmp = m[1];
	for (i = 0; i < 4; i++)
	{
		m[i] -= m[i+4]*tmp;
		inv[i] -= inv[i+4]*tmp;
	}
	tmp = m[2];
	for (i = 0; i < 4; i++)
	{
		m[i] -= m[i+8]*tmp;
		inv[i] -= inv[i+8]*tmp;
	}
	tmp = m[3];
	for (i = 0; i < 4; i++)
	{
		m[i] -= m[i+12]*tmp;
		inv[i] -= inv[i+12]*tmp;
	}
	tmp = m[6];
	for (i = 0; i < 4; i++)
	{
		m[i+4] -= m[i+8]*tmp;
		inv[i+4] -= inv[i+8]*tmp;
	}
	tmp = m[7];
	for (i = 0; i < 4; i++)
	{
		m[i+4] -= m[i+12]*tmp;
		inv[i+4] -= inv[i+12]*tmp;
	}
	tmp = m[11];
	for (i = 0; i < 4; i++)
	{
		m[i+8] -= m[i+12]*tmp;
		inv[i+8] -= inv[i+12]*tmp;
	}
	return inv;
}
