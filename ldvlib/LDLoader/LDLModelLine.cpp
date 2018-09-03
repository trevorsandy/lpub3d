#include "LDLModelLine.h"
#include "LDLPalette.h"
#include "LDLMainModel.h"
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <sstream>
#include <TCFoundation/TCMacros.h>
#include <TCFoundation/TCVector.h>
#include <TCFoundation/TCLocalStrings.h>
#include <TCFoundation/mystring.h>

#ifdef WIN32
#if defined(_MSC_VER) && _MSC_VER >= 1400 && defined(_DEBUG)
#define new DEBUG_CLIENTBLOCK
#endif // _DEBUG
#endif // WIN32

LDLModelLine::LDLModelLine(LDLModel *parentModel, const char *line,
						   int lineNumber, const char *originalLine)
	:LDLActionLine(parentModel, line, lineNumber, originalLine),
	m_highResModel(NULL),
	m_lowResModel(NULL)
{
	m_flags.nonUniform = true;
}

LDLModelLine::LDLModelLine(const LDLModelLine &other)
	:LDLActionLine(other),
	m_highResModel(NULL),
	m_lowResModel(NULL),
	m_flags(other.m_flags)
{
	if (other.m_highResModel)
	{
		m_highResModel = (LDLModel *)other.m_highResModel->copy();
	}
	if (other.m_lowResModel)
	{
		m_lowResModel = (LDLModel *)other.m_lowResModel->copy();
	}
	memcpy(m_matrix, other.m_matrix, sizeof(m_matrix));
}

LDLModelLine::~LDLModelLine(void)
{
}

void LDLModelLine::dealloc(void)
{
	TCObject::release(m_highResModel);
	TCObject::release(m_lowResModel);
	LDLActionLine::dealloc();
}

TCObject *LDLModelLine::copy(void) const
{
	return new LDLModelLine(*this);
}

// This function does the following:
// *  Strips out trailing and leading spaces
// *  Converts all whitespace characters prior to the filename into spaces.
//    (For example, tabs are replaced with spaces.)
// *  Makes it so that there are not contiguous spaces prior to the filename.
//    (In other words, n whitespace characters in a row are replaced by a
//    single space.)
void LDLModelLine::fixLine(void)
{
	if (m_line != NULL)
	{
		size_t i, j, k;
		size_t newLen = 0;
		size_t diff;
		bool done = false;
		// The extra char is needed below if line is truncated.
		size_t len = strlen(m_line) + 1;
		char *newLine = new char[len + 1];

		memset(newLine, 0, len + 1);
		for (i = 0; isspace(m_line[i]); i++)
		{
			// Don't do anything
			// We're skipping over any leading spaces here.
		}
		for (j = 0; j < 14 && !done; j++)
		{
			for (k = 0; m_line[i + k] != 0 &&
				!isspace(m_line[i + k]); k++)
			{
				// Don't do anything
				// We're skipping over all non-whitespace characters here.
			}
			diff = k;
			if (diff)
			{
				// If we got here, we haven't yet reached the end of our string.
				// The for loop above will always start with
				// originalLine[i + k] as a non-whitespace character, so if it
				// doesn't loop at all, then the end of the string was reached.
				strncpy(&newLine[newLen], &m_line[i], diff);
				newLine[newLen + diff] = ' ';
				newLen += diff + 1;
				done = true;
				for (i = i + diff; isspace(m_line[i]); i++)
				{
					// We're skipping over whitespace characters here.  If we
					// get in here at all, then we're at the end of the string.
					// Note that done will be set to false repeatedly, but
					// that's not the main focus of this loop.
					done = false;
				}
			}
			else
			{
				done = true;
			}
		}
		strcpy(&newLine[newLen], &m_line[i]);
		stripTrailingWhitespace(&newLine[0]);
		m_processedLine = newLine;
		delete[] newLine;
	}
}

bool LDLModelLine::parse(void)
{
	float x, y, z;
	float a, b, c, d, e, f, g, h, i;
	size_t k;
	char subModelName[1024] = "";
	int lineType;
	const char *spaceSpot;

	fixLine();
	spaceSpot = strchr(m_processedLine.c_str(), ' ');
	for (k = 0; k < 13 && spaceSpot != NULL; k++)
	{
		spaceSpot = strchr(spaceSpot + 1, ' ');
	}
	if (spaceSpot != NULL)
	{
		size_t len;

		spaceSpot++;
		strcpy(subModelName, spaceSpot);
		len = strlen(subModelName);
		for (k = 0; k < len; k++)
		{
			// Without the typecast to TCByte below, extended characters trigger
			// an assert in debug mode in Visual Studio 2005.  The assert code
			// line appears to have been written by someone who didn't know what
			// they were doing.  They add one to the input (an int), then
			// typecast to an unsigned, then check to see if that's less than or
			// equal to 256.  A negagive input results in a huge number,
			// (billions), triggering the assert.
			if (isspace((TCByte)subModelName[k]))
			{
				UCSTR ucSubModelName = mbstoucstring(subModelName);

				setWarning(LDLEWhitespace,
					TCLocalStrings::get(_UC("LDLModelLineWhitespace")),
					ucSubModelName);
				delete[] ucSubModelName;
				break;
			}
		}
	}
	if (sscanf(m_processedLine.c_str(),
		"%d %i %f %f %f %f %f %f %f %f %f %f %f %f", &lineType, &m_colorNumber,
		&x, &y, &z, &a, &b, &c, &d, &e, &f, &g, &h, &i) == 14 &&
		subModelName[0])
	{
		std::stringstream ss;
		std::string prefix = getTypeAndColorPrefix();
		if (!prefix.empty())
		{
			ss << prefix << "  ";
		}
		else
		{
			ss << "1 " << m_colorNumber << "  ";
		}
		ss << x << " " << y << " " << z << "  ";
		ss << a << " " << b << " " << c << "  ";
		ss << d << " " << e << " " << f << "  ";
		ss << g << " " << h << " " << i << "  ";
		ss << subModelName;
		m_formattedLine = copyString(ss.str().c_str());
		//int red, green, blue, alpha;
		m_highResModel = m_parentModel->subModelNamed(subModelName, false,
			false, this);
		if (m_highResModel)
		{
			m_highResModel->retain();
		}
		else
		{
			m_valid = false;
			if (getMainModel()->ancestorCheck(subModelName))
			{
				setError(LDLEModelLoop,
					TCLocalStrings::get(_UC("LDLEModelLoop")));
			}
			else
			{
				UCSTR ucSubModelName = mbstoucstring(subModelName);

				setError(LDLEFileNotFound,
					TCLocalStrings::get(_UC("LDLModelLineFNF")),
					ucSubModelName);
				delete[] ucSubModelName;
			}
			return false;
		}
		m_lowResModel = m_parentModel->subModelNamed(subModelName, true, false,
			this);
		if (m_lowResModel)
		{
			m_lowResModel->retain();
		}
		//m_parentModel->getRGBA(m_colorNumber, red, green, blue, alpha);
		setTransformation(x, y, z, a, b, c, d, e, f, g, h, i);
		if (!getMainModel()->getSkipValidation())
		{
			TCFloat determinant = TCVector::determinant(m_matrix);

			if (determinant == 0.0f)
			{
				// The determinant is zero.  We will try to fix the
				// transformation matrix if the part only contains points in an
				// XZ-plane with Y == 0.0. Many part authors are lazy or
				// ignorant and specify zero Y values.
				if (isXZPlanar())
				{
					// This is an XZ-planar model, so try to fix the singular
					// matrix.
					determinant = tryToFixPlanarMatrix();
					if (determinant == 0.0f)
					{
						// If it's still zero, we failed to fix it.
						setError(LDLEMatrix,
							TCLocalStrings::get(_UC("LDLModelLineSingular")));
					}
				}
				else
				{
					// We don't want to even try to fix if the sub-model isn't
					// XZ-planar at Y == 0.
					setError(LDLEMatrix,
						TCLocalStrings::get(
						_UC("LDLModelLineSingularNonFlat")));
				}
				if (determinant == 0.0f)
				{
					m_valid = false;
					return false;
				}
			}
			// Note that if we get this far, determinant still holds the current
			// determinant of the matrix, even if the matrix got adjusted above.
			if (m_highResModel->isPart() && !m_parentModel->isPart())
			{
				if (!fEq2(determinant, 1.0f, 0.05f) &&
					!fEq2(determinant, -1.0f, 0.05f))
				{
					// If the determinant is not either 1 or -1, they applied a
					// non-uniform scale.  Note that we are being EXTREMELY
					//  loose with this "equality" check (within 0.05).
					setWarning(LDLEPartDeterminant,
						TCLocalStrings::get(_UC("LDLModelLineNonUniformPart")));
					m_flags.nonUniform = true;
					m_highResModel->setNoShrinkFlag(true);
					if (m_lowResModel != NULL)
					{
						m_lowResModel->setNoShrinkFlag(true);
					}
				}
			}
		}
		return true;
	}
	else
	{
		m_valid = false;
		setError(LDLEParse, TCLocalStrings::get(_UC("LDLModelLineParse")));
		return false;
	}
}

void LDLModelLine::setTransformation(TCFloat x, TCFloat y, TCFloat z,
									 TCFloat a, TCFloat b, TCFloat c,
									 TCFloat d, TCFloat e, TCFloat f,
									 TCFloat g, TCFloat h, TCFloat i)
{
	m_matrix[3] = 0.0f;
	m_matrix[7] = 0.0f;
	m_matrix[11] = 0.0f;
	m_matrix[15] = 1.0f;
	m_matrix[0] = a;
	m_matrix[4] = b;
	m_matrix[8] = c;
	m_matrix[1] = d;
	m_matrix[5] = e;
	m_matrix[9] = f;
	m_matrix[2] = g;
	m_matrix[6] = h;
	m_matrix[10] = i;
	m_matrix[12] = x;
	m_matrix[13] = y;
	m_matrix[14] = z;
}

void LDLModelLine::print(int indent) const
{
	LDLActionLine::print(indent);
	if (getModel())
	{
		getModel()->print(indent);
	}
}

LDLModel *LDLModelLine::getModel(bool forceHighRes /*= false*/)
{
	const LDLModel *model =
		((const LDLModelLine *)this)->getModel(forceHighRes);
	return const_cast<LDLModel *>(model);
}

const LDLModel *LDLModelLine::getModel(bool forceHighRes /*= false*/) const
{
	if (m_parentModel)
	{
		if (m_lowResModel && m_parentModel->getLowResStuds() && !forceHighRes)
		{
			return m_lowResModel;
		}
		else
		{
			return m_highResModel;
		}
	}
	else
	{
		return NULL;
	}
}

bool LDLModelLine::isXZPlanar(const TCFloat *matrix) const
{
	// This returns true if the all the points in this file line have Y == 0,
	// after being transformed by matrix.
	TCFloat newMatrix[16];
	int i;
	int count = m_highResModel->getActiveLineCount();
	LDLFileLineArray *fileLines = m_highResModel->getFileLines();

	TCVector::multMatrix(matrix, m_matrix, newMatrix);
	for (i = 0; i < count; i++)
	{
		LDLFileLine *fileLine = (*fileLines)[i];

		if (!fileLine->isXZPlanar(newMatrix))
		{
			return false;
		}
	}
	return true;
}

bool LDLModelLine::isXZPlanar(void) const
{
	// This returns true if the all the points in this file line have Y == 0.
	int i;
	int count = m_highResModel->getActiveLineCount();
	LDLFileLineArray *fileLines = m_highResModel->getFileLines();

	for (i = 0; i < count; i++)
	{
		LDLFileLine *fileLine = (*fileLines)[i];

		if (fileLine->getLineType() == LDLLineTypeModel)
		{
			LDLModelLine *modelLine = (LDLModelLine *)fileLine;

			if (!modelLine->isXZPlanar(TCVector::getIdentityMatrix()))
			{
				return false;
			}
			else if (!fileLine->isXZPlanar())
			{
				return false;
			}
		}
	}
	return true;
}

// This code is inspired by an old (2000) version of L3Input that Lars Hassing
// wrote and then released for use in LDGLite.
//
// This method attempts to fix a singular matrix.  It's only called if the model
// is XZ-planar at 0.0 (all Y values equal 0.0).  Typically one of the rows are
// all zeros, and it doesn't affect the transformation to set the Y value to
// 1.0 (what the part author should have done).  Also, the second column may be
// all zeros.  Here we may set one of the Y values to 1.0 at a time.  Setting
// all three Y values to 1.0 at the same time may give two rows that are
// linear dependent (Travis Cobbs: not sure what that means).
TCFloat LDLModelLine::tryToFixPlanarMatrix(void)
{
	TCFloat determinant = 0.0f;
	int i;

	// First, check the rows.
	for (i = 0; i < 3; i++)
	{
		if (m_matrix[i] == 0.0f && m_matrix[4 + i] == 0.0f &&
			m_matrix[8 + i] == 0.0f)
		{
			// Row i all zeros, try fixing by setting the Y value
			m_matrix[4 + i] = 1.0f;
			determinant = TCVector::determinant(m_matrix);
			if (determinant != 0.0f)
			{
				setWarning(LDLEMatrix,
					TCLocalStrings::get(_UC("LDLModelLineZeroMatrixRow")), i);
				return determinant;
			}
		}
	}
	// That didn't work, so check the Y column.
	if (m_matrix[4] == 0.0f && m_matrix[5] == 0.0f && m_matrix[6] == 0.0f)
	{
		for (i = 0; i < 3; i++)
		{
			// Y column all zeros, try fixing one Y at a time.
			m_matrix[4 + i] = 1.0f;
			determinant = TCVector::determinant(m_matrix);
			if (determinant != 0.0f)
			{
				setWarning(LDLEMatrix,
					TCLocalStrings::get(_UC("LDLModelLineZeroMatrixCol")), i);
				return determinant;
			}
		}
	}
	return determinant;
}

void LDLModelLine::scanPoints(
	TCObject *scanner,
	LDLScanPointCallback scanPointCallback,
	const TCFloat *matrix,
	bool watchBBoxIgnore) const
{
	const LDLModel *model = getModel();
	if (model)
	{
		TCFloat newMatrix[16];
		float seamWidth = model->getMainModel()->getSeamWidth();

		if (seamWidth > 0.0f && model->hasBoundingBox() && model->isPart() &&
			(m_parentModel == NULL || !m_parentModel->isPart()))
		{
			TCFloat scaleMatrix[16];
			TCFloat tempMatrix[16];
			TCVector boundingMin, boundingMax;

			model->getBoundingBox(boundingMin, boundingMax);
			TCVector::calcScaleMatrix(seamWidth, scaleMatrix, boundingMin,
				boundingMax);
			TCVector::multMatrix(m_matrix, scaleMatrix, tempMatrix);
			TCVector::multMatrix(matrix, tempMatrix, newMatrix);
		}
		else
		{
			TCVector::multMatrix(matrix, m_matrix, newMatrix);
		}
		model->scanPoints(scanner, scanPointCallback, newMatrix, -1,
			watchBBoxIgnore);
	}
}

void LDLModelLine::setMatrix(const TCFloat *value)
{
	memcpy(m_matrix, value, sizeof(m_matrix));
}

void LDLModelLine::createLowResModel(LDLMainModel *mainModel, const char *name)
{
	m_lowResModel = new LDLModel;
	m_lowResModel->setMainModel(mainModel);
	m_lowResModel->setName(name);
}

void LDLModelLine::createHighResModel(LDLMainModel *mainModel, const char *name)
{
	m_highResModel = new LDLModel;
	m_highResModel->setMainModel(mainModel);
	m_highResModel->setName(name);
}


int LDLModelLine::getColorNumber(void) const
{
	const LDLMainModel *mainModel = getMainModel();

	if (mainModel->getRandomColors() && m_highResModel != NULL &&
		m_highResModel->isPart() && !m_parentModel->isPart())
	{
		return getRandomColorNumber();
	}
	else
	{
		return LDLActionLine::getColorNumber();
	}
}
