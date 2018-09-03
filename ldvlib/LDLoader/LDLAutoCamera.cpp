#include "LDLAutoCamera.h"
#include "LDLModel.h"
#include "LDLMainModel.h"
#include <TCFoundation/TCMacros.h>

#ifdef WIN32
#if defined(_MSC_VER) && _MSC_VER >= 1400 && defined(_DEBUG)
#define new DEBUG_CLIENTBLOCK
#endif // _DEBUG
#endif // WIN32

LDLAutoCamera::LDLAutoCamera(void):
m_model(NULL),
m_haveGlobeRadius(false),
m_distanceMultiplier(1.0f),
m_width(0.0f),
m_height(0.0f),
m_margin(0.0f),
m_step(-1),
m_cameraData(NULL)
{
}

LDLAutoCamera::~LDLAutoCamera(void)
{
}

void LDLAutoCamera::dealloc(void)
{
	delete m_cameraData;
	TCObject::dealloc();
}

void LDLAutoCamera::setModel(LDLModel *value)
{
	m_model = value;
}

void LDLAutoCamera::setModelCenter(const TCVector &value)
{
	m_modelCenter = value;
}

void LDLAutoCamera::setRotationMatrix(const TCFloat *value)
{
	memcpy(m_rotationMatrix, value, sizeof(value[0]) * 16);
}

void LDLAutoCamera::setCamera(const LDLCamera &value)
{
	m_camera = value;
}

void LDLAutoCamera::setCameraGlobe(const char *value)
{
	if (value && sscanf(value, "%*f,%*f,%f", &m_globeRadius) == 1)
	{
		m_haveGlobeRadius = true;
	}
	else
	{
		m_haveGlobeRadius = false;
	}
}

void LDLAutoCamera::setDistanceMultiplier(TCFloat value)
{
	m_distanceMultiplier = value;
}

void LDLAutoCamera::setWidth(TCFloat value)
{
	m_width = value;
}

void LDLAutoCamera::setHeight(TCFloat value)
{
	m_height = value;
}

void LDLAutoCamera::setMargin(TCFloat value)
{
	m_margin = value;
}

void LDLAutoCamera::setFov(TCFloat value)
{
	m_fov = value;
}

void LDLAutoCamera::zoomToFit(void)
{
	if (m_haveGlobeRadius)
	{
		TCVector location;
		location[0] = location[1] = 0.0;
		if (m_globeRadius >= 0)
		{
			location[2] = m_globeRadius;
		}
		else
		{
			location[2] *= 1.0f - m_globeRadius / 100.0f;
		}
		m_camera.setPosition(location);
	}
	else if (m_model)
	{
		TCFloat d;
		TCFloat a[6][6];
		TCFloat b[6];
		TCFloat x[6];
		TCVector tmpVec;
		TCVector location;
		TCVector cameraDir;
		TCFloat tmpMatrix[16];
		TCFloat tmpMatrix2[16];
		TCFloat transformationMatrix[16];
		LDLMainModel *mainModel = m_model->getMainModel();
		bool origScanControlPoints =
			mainModel->getScanConditionalControlPoints();

		TCVector::initIdentityMatrix(tmpMatrix);
		tmpMatrix[12] = m_modelCenter[0];
		tmpMatrix[13] = m_modelCenter[1];
		tmpMatrix[14] = m_modelCenter[2];
		TCVector::multMatrix(tmpMatrix, m_rotationMatrix, tmpMatrix2);
		tmpMatrix[12] = -m_modelCenter[0];
		tmpMatrix[13] = -m_modelCenter[1];
		tmpMatrix[14] = -m_modelCenter[2];
		TCVector::multMatrix(tmpMatrix2, tmpMatrix, transformationMatrix);
		preCalcCamera();
#ifdef _DEBUG
		m_numPoints = 0;
#endif // _DEBUG
		mainModel->setScanConditionalControlPoints(
			m_scanConditionalControlPoints);
		m_model->scanPoints(this,
			(LDLScanPointCallback)&LDLAutoCamera::scanCameraPoint,
			transformationMatrix, m_step, true);
		mainModel->setScanConditionalControlPoints(origScanControlPoints);
#ifdef _DEBUG
		debugPrintf("num points: %d\n", m_numPoints);
#endif // _DEBUG
		memset(a, 0, sizeof(a));
		memset(b, 0, sizeof(b));
		a[0][0] = m_cameraData->normal[0][0];
		a[0][1] = m_cameraData->normal[0][1];
		a[0][2] = m_cameraData->normal[0][2];
		b[0] = m_cameraData->dMin[0];
		a[1][0] = m_cameraData->normal[1][0];
		a[1][1] = m_cameraData->normal[1][1];
		a[1][2] = m_cameraData->normal[1][2];
		b[1] = m_cameraData->dMin[1];
		a[2][3] = m_cameraData->normal[2][0];
		a[2][4] = m_cameraData->normal[2][1];
		a[2][5] = m_cameraData->normal[2][2];
		b[2] = m_cameraData->dMin[2];
		a[3][3] = m_cameraData->normal[3][0];
		a[3][4] = m_cameraData->normal[3][1];
		a[3][5] = m_cameraData->normal[3][2];
		b[3] = m_cameraData->dMin[3];
		if (m_cameraData->direction[0] == 0.0)
		{
			a[4][1] = -m_cameraData->direction[2];
			a[4][2] = m_cameraData->direction[1];
			a[4][4] = m_cameraData->direction[2];
			a[4][5] = -m_cameraData->direction[1];
			if (m_cameraData->direction[1] == 0.0)
			{
				a[5][0] = -m_cameraData->direction[2];
				a[5][2] = m_cameraData->direction[0];
				a[5][3] = m_cameraData->direction[2];
				a[5][5] = -m_cameraData->direction[0];
			}
			else
			{
				a[5][0] = -m_cameraData->direction[1];
				a[5][1] = m_cameraData->direction[0];
				a[5][3] = m_cameraData->direction[1];
				a[5][4] = -m_cameraData->direction[0];
			}
		}
		else
		{
			a[4][0] = -m_cameraData->direction[2];
			a[4][2] = m_cameraData->direction[0];
			a[4][3] = m_cameraData->direction[2];
			a[4][5] = -m_cameraData->direction[0];
			if (m_cameraData->direction[1] == 0.0 && m_cameraData->direction[2]
				!= 0.0)
			{
				a[5][1] = -m_cameraData->direction[2];
				a[5][2] = m_cameraData->direction[1];
				a[5][4] = m_cameraData->direction[2];
				a[5][5] = -m_cameraData->direction[1];
			}
			else
			{
				a[5][0] = -m_cameraData->direction[1];
				a[5][1] = m_cameraData->direction[0];
				a[5][3] = m_cameraData->direction[1];
				a[5][4] = -m_cameraData->direction[0];
			}
		}
		if (!L3Solve6(x, a, b))
		{
			// Singular matrix; can't work
			// (We shouldn't ever get here, so I'm not going to bother with an
			// error message.  I'd have to first come up with some mechanism
			// for communicating the error with the non-portable part of the
			// app so that it could be displayed to the user.)
			return;
		}
		tmpVec = TCVector(x[3], x[4], x[5]) - TCVector(x[0], x[1], x[2]);
		cameraDir = TCVector(m_cameraData->direction[0], m_cameraData->direction[1],
			m_cameraData->direction[2]);
		d = cameraDir.dot(tmpVec);
		if (d > 0.0)
		{
			location[0] = x[0];
			location[1] = x[1];
			location[2] = x[2] * (m_height + m_margin) / m_height;
		}
		else
		{
			location[0] = x[3];
			location[1] = x[4];
			location[2] = x[5] * (m_width + m_margin) / m_width;
		}
		location[2] *= m_distanceMultiplier;
		m_camera.setPosition(location);
	}
}

void LDLAutoCamera::scanCameraPoint(
	const TCVector &point,
	const LDLFileLine * /*pFileLine*/)
{
	TCFloat d;
	int i;

	for (i = 0; i < 4; i++)
	{
		d = m_cameraData->normal[i].dot(point - m_modelCenter);
		if (d < m_cameraData->dMin[i])
		{
			m_cameraData->dMin[i] = d;
		}
	}
#ifdef _DEBUG
	m_numPoints++;
#endif // _DEBUG
}

void LDLAutoCamera::preCalcCamera(void)
{
	TCFloat d;
	int i;

	delete m_cameraData;
	m_cameraData = new CameraData;
	if (m_width > m_height)
	{
		m_cameraData->fov = (TCFloat)(2.0 * rad2deg(atan(tan(deg2rad(m_fov / 2.0)) *
			(double)m_width / (double)m_height)));
	}
	else
	{
		m_cameraData->fov = m_fov;
	}
	d = (TCFloat)(1.0 / tan(deg2rad(m_cameraData->fov / 2.0)));
	m_cameraData->normal[2] = m_cameraData->direction -
		(m_cameraData->horizontal * d);
	m_cameraData->normal[3] = m_cameraData->direction +
		(m_cameraData->horizontal * d);
	d *= m_width / m_height;
	m_cameraData->normal[0] = m_cameraData->direction -
		(m_cameraData->vertical * d);
	m_cameraData->normal[1] = m_cameraData->direction +
		(m_cameraData->vertical * d);
	for (i = 0; i < 4; i++)
	{
		m_cameraData->normal[i].normalize();
	}
}


/* The following L3Solve6 was extracted by Lars C. Hassing in May 2005
  from jama_lu.h, part of the "JAMA/C++ Linear Algebra Package" (JAva MAtrix)
  found together with the "Template Numerical Toolkit (TNT)"
  at http://math.nist.gov/tnt/download.html
  Their disclaimer:
  "This software was developed at the National Institute of Standards and
   Technology (NIST) by employees of the Federal Government in the course
   of their official duties. Pursuant to title 17 Section 105 of the
   United States Code this software is not subject to copyright protection
   and is in the public domain. NIST assumes no responsibility whatsoever
   for its use by other parties, and makes no guarantees, expressed or
   implied, about its quality, reliability, or any other characteristic."
*/

/* Solve A*x=b, returns 1 if OK, 0 if A is singular */
int LDLAutoCamera::L3Solve6(TCFloat x[L3ORDERN],
            const TCFloat A[L3ORDERM][L3ORDERN],
            const TCFloat b[L3ORDERM])
{
  TCFloat          LU_[L3ORDERM][L3ORDERN];
  int            pivsign;
  int            piv[L3ORDERM];/* pivot permutation vector                  */
  int            i;
  int            j;
  int            k;
  int            p;
  TCFloat         *LUrowi;
  TCFloat          LUcolj[L3ORDERM];
  int            kmax;
  double         s;
  TCFloat          t;

  /** LU Decomposition.
  For an m-by-n matrix A with m >= n, the LU decomposition is an m-by-n
  unit lower triangular matrix L, an n-by-n upper triangular matrix U,
  and a permutation vector piv of length m so that A(piv,:) = L*U.
  If m < n, then L is m-by-m and U is m-by-n.

  The LU decompostion with pivoting always exists, even if the matrix is
  singular, so the constructor will never fail.  The primary use of the
  LU decomposition is in the solution of square systems of simultaneous
  linear equations.  This will fail if isNonsingular() returns false.
                                                                            */
  memcpy(LU_, A, sizeof(LU_));

  /* Use a "left-looking", dot-product, Crout/Doolittle algorithm. */
  for (i = 0; i < L3ORDERM; i++)
     piv[i] = i;
  pivsign = 1;

  /* Outer loop. */
  for (j = 0; j < L3ORDERN; j++)
  {
     /* Make a copy of the j-th column to localize references. */
     for (i = 0; i < L3ORDERM; i++)
        LUcolj[i] = LU_[i][j];

     /* Apply previous transformations. */
     for (i = 0; i < L3ORDERM; i++)
     {
        LUrowi = LU_[i];
        /* Most of the time is spent in the following dot product. */
        kmax = i < j ? i : j;  /* min(i, j)                                 */
        s = 0.0;
        for (k = 0; k < kmax; k++)
           s += LUrowi[k] * LUcolj[k];
        LUrowi[j] = LUcolj[i] -= (TCFloat)s;
     }

     /* Find pivot and exchange if necessary. */
     p = j;
     for (i = j + 1; i < L3ORDERM; i++)
     {
        if (fabs(LUcolj[i]) > fabs(LUcolj[p]))
           p = i;
     }
     if (p != j)
     {
        for (k = 0; k < L3ORDERN; k++)
        {
           t = LU_[p][k];
           LU_[p][k] = LU_[j][k];
           LU_[j][k] = t;
        }
        k = piv[p];
        piv[p] = piv[j];
        piv[j] = k;
        pivsign = -pivsign;
     }

     /* Compute multipliers. */
     if ((j < L3ORDERM) && (LU_[j][j] != 0.0))
     {
        for (i = j + 1; i < L3ORDERM; i++)
           LU_[i][j] /= LU_[j][j];
     }
  }

  /* LCH: This was LU Decomposition. Now solve: */

  /** Solve A*x = b, where x and b are vectors of length equal
        to the number of rows in A.

  @param  b   a vector (Array1D> of length equal to the first dimension of A.
  @return x   a vector (Array1D> so that L*U*x = b(piv), if B is nonconformant,
              returns 0x0 (null) array.
                                                                            */
  /* Is the matrix nonsingular? I.e. is upper triangular factor U (and hence
     A) nonsingular (isNonsingular())                                       */
  for (j = 0; j < L3ORDERN; j++)
  {
     if (LU_[j][j] == 0)
        return 0;
  }

  for (i = 0; i < L3ORDERN; i++)
     x[i] = b[piv[i]];

  /* Solve L*Y = B(piv) */
  for (k = 0; k < L3ORDERN; k++)
  {
     for (i = k + 1; i < L3ORDERN; i++)
        x[i] -= x[k] * LU_[i][k];
  }

  /* Solve U*X = Y; */
  for (k = L3ORDERN - 1; k >= 0; k--)
  {
     x[k] /= LU_[k][k];
     for (i = 0; i < k; i++)
        x[i] -= x[k] * LU_[i][k];
  }

  return 1;
}                               /* Solve6                                    */
