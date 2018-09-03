#include "LDLPrimitiveCheck.h"

#include <string.h>

#include <LDLoader/LDLMainModel.h>
#include <LDLoader/LDLShapeLine.h>
#include <LDLoader/LDLModelLine.h>
#include <LDLoader/LDLConditionalLineLine.h>
#include <LDLoader/LDLPalette.h>
#include <TCFoundation/mystring.h>
#include <TCFoundation/TCMacros.h>
#include <TCFoundation/TCVector.h>
#include <TCFoundation/TCLocalStrings.h>
#include <ctype.h>

#ifdef WIN32
#if defined(_MSC_VER) && _MSC_VER >= 1400 && defined(_DEBUG)
#define new DEBUG_CLIENTBLOCK
#endif // _DEBUG
#endif // WIN32

static const int LO_NUM_SEGMENTS = 8;
//static const int HI_NUM_SEGMENTS = 16;

LDLPrimitiveCheck::LDLPrimitiveCheck(void):
	m_alertSender(NULL),
	m_curveQuality(2)
{
	// Initialize all flags to false.
	memset(&m_flags, 0, sizeof(m_flags));
	m_flags.primitiveSubstitution = true;
}

LDLPrimitiveCheck::~LDLPrimitiveCheck(void)
{
}

void LDLPrimitiveCheck::dealloc(void)
{
	TCObject::dealloc();
}

TCFloat LDLPrimitiveCheck::startingFraction(const char *filename)
{
	size_t i;

	if (stringHasCaseInsensitivePrefix(filename, "48/") ||
		stringHasCaseInsensitivePrefix(filename, "48\\"))
	{
		filename += 3;
	}
	sscanf(filename, "%d", &m_filenameNumerator);
	for (i = 1; filename[i] != '-'; i++)
	{
		// Don't do anything.
	}
	sscanf(filename + i + 1, "%d", &m_filenameDenom);
	return (TCFloat)m_filenameNumerator / (TCFloat)m_filenameDenom;
}

bool LDLPrimitiveCheck::startsWithFraction(const char *filename)
{
	return isdigit(filename[0]) && filename[1] == '-' && isdigit(filename[2]) &&
		!isdigit(filename[3]);
}

bool LDLPrimitiveCheck::startsWithFraction2(const char *filename)
{
	return isdigit(filename[0]) && filename[1] == '-' && isdigit(filename[2]) &&
		isdigit(filename[3]) && !isdigit(filename[4]);
}

size_t LDLPrimitiveCheck::getStartingFractionLength(const char *filename)
{
	size_t i;

	// Note: since the string is NULL-terminated, there's no need to do a length
	// check, because the NULL terminator won't match isdigit.
	for (i = 0; isdigit(filename[i]); i++)
	{
		// Don't do anything.
	}
	if (filename[i] != '-')
	{
		return 0;
	}
	for (i = i + 1; isdigit(filename[i]); i++)
	{
		// Don't do anything.
	}
	return i;
}

bool LDLPrimitiveCheck::isPrimitive(const char *filename, const char *suffix,
								bool *is48)
{
	size_t fileLen = strlen(filename);
	size_t suffixLen = strlen(suffix);

	if (is48 != NULL)
	{
		*is48 = false;
	}
	if (((fileLen == suffixLen + 3 && startsWithFraction(filename)) ||
		(suffixLen <= 8 && fileLen == suffixLen + 4 &&
		startsWithFraction2(filename))) &&
		stringHasCaseInsensitiveSuffix(filename, suffix))
	{
		return true;
	}
	else if (is48 != NULL && (stringHasCaseInsensitivePrefix(filename, "48/") ||
		stringHasCaseInsensitivePrefix(filename, "48\\")))
	{
		*is48 = true;
		return isPrimitive(filename + 3, suffix, NULL);
	}
	return false;
}

bool LDLPrimitiveCheck::isCyli(const char *filename, bool *is48)
{
	return isPrimitive(filename, "cyli.dat", is48);
}

bool LDLPrimitiveCheck::isCyls(const char *filename, bool *is48)
{
	return isPrimitive(filename, "cyls.dat", is48);
}

bool LDLPrimitiveCheck::isCyls2(const char *filename, bool *is48)
{
	return isPrimitive(filename, "cyls2.dat", is48);
}

bool LDLPrimitiveCheck::isChrd(const char *filename, bool *is48)
{
	return isPrimitive(filename, "chrd.dat", is48);
}

bool LDLPrimitiveCheck::isDisc(const char *filename, bool *is48)
{
	return isPrimitive(filename, "disc.dat", is48);
}

bool LDLPrimitiveCheck::isNdis(const char *filename, bool *is48)
{
	return isPrimitive(filename, "ndis.dat", is48);
}

bool LDLPrimitiveCheck::isTang(const char *filename, bool *is48)
{
	return isPrimitive(filename, "tang.dat", is48);
}

bool LDLPrimitiveCheck::isEdge(const char *filename, bool *is48)
{
	return isPrimitive(filename, "edge.dat", is48);
}

bool LDLPrimitiveCheck::is1DigitCon(const char *filename, bool *is48)
{
	if (is48 != NULL)
	{
		*is48 = false;
	}
	if (strlen(filename) == 11 && startsWithFraction(filename) &&
		stringHasCaseInsensitivePrefix(filename + 3, "con") &&
		isdigit(filename[6]) &&
		stringHasCaseInsensitiveSuffix(filename, ".dat"))
	{
		return true;
	}
	else if (is48 != NULL && (stringHasCaseInsensitivePrefix(filename, "48/") ||
		stringHasCaseInsensitivePrefix(filename, "48\\")))
	{
		*is48 = true;
		return is1DigitCon(filename + 3, NULL);
	}
	return false;
}

bool LDLPrimitiveCheck::is2DigitCon(const char *filename, bool *is48)
{
	if (is48 != NULL)
	{
		*is48 = false;
	}
	if (strlen(filename) == 12 && startsWithFraction(filename) &&
		stringHasCaseInsensitivePrefix(filename + 3, "con") &&
		isdigit(filename[6]) && isdigit(filename[7]) &&
		stringHasCaseInsensitiveSuffix(filename, ".dat"))
	{
		return true;
	}
	else if (is48 != NULL && (stringHasCaseInsensitivePrefix(filename, "48/") ||
		stringHasCaseInsensitivePrefix(filename, "48\\")))
	{
		*is48 = true;
		return is1DigitCon(filename + 3, NULL);
	}
	return false;
}

bool LDLPrimitiveCheck::isCon(const char *filename, bool *is48)
{
	return is1DigitCon(filename, is48) || is2DigitCon(filename, is48);
}

bool LDLPrimitiveCheck::isOldRing(const char *filename, bool *is48)
{
	size_t len = strlen(filename);

	if (is48 != NULL)
	{
		*is48 = false;
	}
	if (len >= 9 && len <= 12 &&
		stringHasCaseInsensitivePrefix(filename, "ring") &&
		stringHasCaseInsensitiveSuffix(filename, ".dat"))
	{
		size_t i;

		for (i = 4; i < len - 5; i++)
		{
			if (!isdigit(filename[i]))
			{
				return false;
			}
		}
		return true;
	}
	else if (is48 != NULL && (stringHasCaseInsensitivePrefix(filename, "48/") ||
		stringHasCaseInsensitivePrefix(filename, "48\\")))
	{
		*is48 = true;
		return isOldRing(filename + 3, NULL);
	}
	else
	{
		return false;
	}
}

bool LDLPrimitiveCheck::isRing(
	const char *filename,
	int &size,
	bool &hasStartingFraction,
	bool *is48)
{
	int offset = -1;
	int rinLen;

	hasStartingFraction = true;
	if (isOldRing(m_modelName, is48))
	{
		hasStartingFraction = false;
		offset = 4;
	}
	else if (isRing(m_modelName, is48))
	{
		offset = 7;
	}
	else if (isRin(m_modelName, rinLen, is48))
	{
		int fracLen;
		int sfOffset = 0;

		if (*is48)
		{
			sfOffset = 3;
		}
		fracLen = (int)getStartingFractionLength(&filename[sfOffset]);
		offset = fracLen + rinLen;
	}
	if (offset >= 0)
	{
		if (*is48)
		{
			offset += 3;
		}
		sscanf(filename + offset, "%d", &size);
		return true;
	}
	else
	{
		return false;
	}
}

bool LDLPrimitiveCheck::isRing(const char *filename, bool *is48)
{
	if (is48 != NULL)
	{
		*is48 = false;
	}
	if (strlen(filename) == 12 && startsWithFraction(filename) &&
		stringHasCaseInsensitivePrefix(filename + 3, "ring") &&
		isdigit(filename[7]) &&
		stringHasCaseInsensitiveSuffix(filename, ".dat"))
	{
		return true;
	}
	else if (is48 != NULL && (stringHasCaseInsensitivePrefix(filename, "48/") ||
		stringHasCaseInsensitivePrefix(filename, "48\\")))
	{
		*is48 = true;
		return isRing(filename + 3, NULL);
	}
	else
	{
		return false;
	}
}

bool LDLPrimitiveCheck::isRin(const char *filename, int &rinLen, bool *is48)
{
	rinLen = 0;
	if (is48 != NULL)
	{
		*is48 = false;
	}
	if (is48 != NULL && (stringHasCaseInsensitivePrefix(filename, "48/") ||
		stringHasCaseInsensitivePrefix(filename, "48\\")))
	{
		*is48 = true;
		return isRin(filename + 3, rinLen, NULL);
	}
	else
	{
		size_t fracLen = getStartingFractionLength(filename);
		size_t len = strlen(filename);

		if (len >= 12 && fracLen > 0 &&
			stringHasCaseInsensitiveSuffix(filename, ".dat"))
		{
			if (stringHasCaseInsensitivePrefix(filename + fracLen, "ring"))
			{
				rinLen = 4;
			}
			else if (stringHasCaseInsensitivePrefix(filename + fracLen, "rin"))
			{
				rinLen = 3;
			}
			else if (stringHasCaseInsensitivePrefix(filename + fracLen, "ri"))
			{
				rinLen = 2;
			}
			else if (stringHasCaseInsensitivePrefix(filename + fracLen, "r"))
			{
				rinLen = 1;
			}
			if (rinLen > 0)
			{
				size_t i;

				for (i = fracLen + rinLen; isdigit(filename[i]); i++)
				{
					// Don't do anything
				}
				if (i > fracLen + rinLen && i == len - 4)
				{
					return true;
				}
			}
		}
		return false;
	}
}

bool LDLPrimitiveCheck::isTorus(
	const char *filename,
	bool allowR,
	bool &isMixed,
	bool *is48)
{
	if (is48 != NULL)
	{
		*is48 = false;
	}
	size_t len = strlen(filename);
	size_t expectedLen = 12;
	size_t prefixSize = 1;
	isMixed = false;
	if (len == 13 && toupper(filename[0]) == 'T' && toupper(filename[1]) == 'M')
	{
		expectedLen = 13;
		prefixSize = 2;
		isMixed = true;
	}
	if (len == expectedLen && (toupper(filename[0]) == 'T' ||
		(toupper(filename[0]) == 'R' && allowR)) &&
		isdigit(filename[prefixSize]) &&
		isdigit(filename[prefixSize + 1]) &&
		isdigit(filename[prefixSize + 3]) &&
		isdigit(filename[prefixSize + 4]) &&
		isdigit(filename[prefixSize + 5]) &&
		isdigit(filename[prefixSize + 6]) &&
		stringHasCaseInsensitiveSuffix(filename, ".dat"))
	{
		return true;
	}
	else if (is48 != NULL && (stringHasCaseInsensitivePrefix(filename, "48/") ||
		stringHasCaseInsensitivePrefix(filename, "48\\")))
	{
		*is48 = true;
		return isTorus(filename + 3, allowR, isMixed, NULL);
	}
	else
	{
		return false;
	}
}

bool LDLPrimitiveCheck::isTorusO(
	const char *filename,
	bool &isMixed,
	bool &is48)
{
	if (isTorus(filename, allowRTori(), isMixed, &is48))
	{
		size_t spot = isMixed ? 4 : 3;
		if (is48)
		{
			spot += 3;
		}
		return toupper(filename[spot]) == 'O';
	}
	else
	{
		return false;
	}
}

bool LDLPrimitiveCheck::isTorusI(
	const char *filename,
	bool &isMixed,
	bool &is48)
{
	if (isTorus(filename, false, isMixed, &is48))
	{
		size_t spot = isMixed ? 4 : 3;
		if (is48)
		{
			spot += 3;
		}
		return toupper(filename[spot]) == 'I';
	}
	else
	{
		return false;
	}
}

bool LDLPrimitiveCheck::isTorusQ(
	const char *filename,
	bool &isMixed,
	bool &is48)
{
	if (isTorus(filename, false, isMixed, &is48))
	{
		size_t spot = isMixed ? 4 : 3;
		if (is48)
		{
			spot += 3;
		}
		return toupper(filename[spot]) == 'Q';
	}
	else
	{
		return false;
	}
}

TCFloat LDLPrimitiveCheck::getTorusFraction(int size)
{
	if (size < 0)
	{
		int i;
		int sizeFrac = -size % 1000;

		size = -size;
		for (i = 0; i < 10; i++)
		{
			if (sizeFrac == i + i * 10 + i * 100)
			{
				return (TCFloat)i / 9.0f + (size / 1000);
			}
		}
		return (TCFloat)size / 1000.0f;
	}
	else
	{
		int i;

		for (i = 0; i < 10; i++)
		{
			if (size == i + i * 10 + i * 100 + i * 1000)
			{
				return (TCFloat)i / 9.0f;
			}
		}
		return (TCFloat)size / 10000.0f;
	}
}

int LDLPrimitiveCheck::getUsedCircleSegments(int numSegments, TCFloat fraction)
{
	return (int)(numSegments * fraction + 0.000001);
}

int LDLPrimitiveCheck::getNumCircleSegments(TCFloat fraction, bool is48)
{
	int retValue = m_curveQuality * LO_NUM_SEGMENTS;

	if (is48 && retValue < 48)
	{
		retValue = 48;
	}
	if (fraction != 0.0f)
	{
		int i;
		
		for (i = m_curveQuality; !fEq(fraction * retValue,
			(TCFloat)getUsedCircleSegments(retValue, fraction)) && i < 12; i++)
		{
			int newValue = (i + 1) * LO_NUM_SEGMENTS;

			if (newValue > retValue)
			{
				retValue = newValue;
			}
		}
	}
	return retValue;
}

bool LDLPrimitiveCheck::performPrimitiveSubstitution(
	LDLModel *ldlModel,
	bool bfc)
{
	m_modelName = ldlModel->getName();

	if (getPrimitiveSubstitutionFlag())
	{
		bool is48;
		bool isMixed;
		int size;
		bool hasStartingFraction;

		if (!m_modelName)
		{
			return false;
		}
		if (strcasecmp(m_modelName, "LDL-LOWRES:stu2.dat") == 0)
		{
			return substituteStu2();
		}
		else if (strcasecmp(m_modelName, "LDL-LOWRES:stu22.dat") == 0)
		{
			return substituteStu22(false, bfc);
		}
		else if (strcasecmp(m_modelName, "LDL-LOWRES:stu22a.dat") == 0)
		{
			return substituteStu22(true, bfc);
		}
		else if (strcasecmp(m_modelName, "LDL-LOWRES:stu23.dat") == 0)
		{
			return substituteStu23(false, bfc);
		}
		else if (strcasecmp(m_modelName, "LDL-LOWRES:stu23a.dat") == 0)
		{
			return substituteStu23(true, bfc);
		}
		else if (strcasecmp(m_modelName, "LDL-LOWRES:stu24.dat") == 0)
		{
			return substituteStu24(false, bfc);
		}
		else if (strcasecmp(m_modelName, "LDL-LOWRES:stu24a.dat") == 0)
		{
			return substituteStu24(true, bfc);
		}
		else if (strcasecmp(m_modelName, "stud.dat") == 0)
		{
			return substituteStud();
		}
		else if (strcasecmp(m_modelName, "1-8sphe.dat") == 0)
		{
			return substituteEighthSphere(bfc);
		}
		else if (strcasecmp(m_modelName, "48/1-8sphe.dat") == 0 ||
			strcasecmp(m_modelName, "48\\1-8sphe.dat") == 0)
		{
			return substituteEighthSphere(bfc, true);
		}
		else if (strcasecmp(m_modelName, "1-8sphc.dat") == 0)
		{
			return substituteEighthSphereCorner(bfc);
		}
		else if (strcasecmp(m_modelName, "48/1-8sphc.dat") == 0 ||
			strcasecmp(m_modelName, "48\\1-8sphc.dat") == 0)
		{
			return substituteEighthSphereCorner(bfc, true);
		}
		else if (isCyli(m_modelName, &is48))
		{
			return substituteCylinder(startingFraction(m_modelName),
				bfc, is48);
		}
		else if (isCyls(m_modelName, &is48))
		{
			return substituteSlopedCylinder(startingFraction(m_modelName), bfc,
				is48);
		}
		else if (isCyls2(m_modelName, &is48))
		{
			return substituteSlopedCylinder2(startingFraction(m_modelName), bfc,
				is48);
		}
		else if (isChrd(m_modelName, &is48))
		{
			return substituteChrd(startingFraction(m_modelName), bfc,
				is48);
		}
		else if (isDisc(m_modelName, &is48))
		{
			return substituteDisc(startingFraction(m_modelName), bfc,
				is48);
		}
		else if (isNdis(m_modelName, &is48))
		{
			return substituteNotDisc(startingFraction(m_modelName),
									 bfc, is48);
		}
		else if (isTang(m_modelName, &is48))
		{
			return substituteTangent(startingFraction(m_modelName),
				bfc, is48);
		}
		else if (isEdge(m_modelName, &is48))
		{
			return substituteCircularEdge(startingFraction(m_modelName), is48);
		}
		else if (isCon(m_modelName, &is48))
		{
			int offset = 0;

			if (is48)
			{
				offset = 3;
			}
			sscanf(m_modelName + 6 + offset, "%d", &size);
			return substituteCone(startingFraction(m_modelName), size,
				bfc, is48);
		}
		else if (isRing(m_modelName, size, hasStartingFraction, &is48))
		{
			if (hasStartingFraction)
			{
				return substituteRing(startingFraction(m_modelName), size,
					bfc, is48);
			}
			else
			{
				return substituteRing(1.0f, size, bfc, is48, true);
			}
		}
		else if (isTorus(m_modelName, true, isMixed, &is48))
		{
			TCFloat fraction;
			int offset = 0;
			const char *name = m_modelName;

			if (isMixed)
			{
				++name;
			}
			if (is48)
			{
				name += 3;
			}
			m_filenameNumerator = 1;
			sscanf(name + 1 + offset, "%d", &m_filenameDenom);
			sscanf(name + 4 + offset, "%d", &size);
			fraction = 1.0f / (TCFloat)m_filenameDenom;
			if (isTorusO(m_modelName, isMixed, is48))
			{
				int rOffset = 0;

				if (is48)
				{
					rOffset = 3;
				}
				if (toupper(m_modelName[rOffset]) == 'R')
				{
					size = -size;
				}
				return substituteTorusIO(false, fraction, size, bfc, isMixed,
					is48);
			}
			else if (isTorusI(m_modelName, isMixed, is48))
			{
				return substituteTorusIO(true, fraction, size, bfc, isMixed,
					is48);
			}
			else if (isTorusQ(m_modelName, isMixed, is48))
			{
				return substituteTorusQ(fraction, size, bfc, isMixed, is48);
			}
		}
	}
	if (getNoLightGeomFlag())
	{
		if (m_modelName && strcasecmp(m_modelName, "light.dat") == 0)
		{
			// Don't draw any geometry for light.dat.
			return true;
		}
	}
	return false;
}

