#include "LDLConditionalLineLine.h"
#include "LDLMainModel.h"
#include <TCFoundation/TCLocalStrings.h>
#include <sstream>

#ifdef WIN32
#if defined(_MSC_VER) && _MSC_VER >= 1400 && defined(_DEBUG)
#define new DEBUG_CLIENTBLOCK
#endif // _DEBUG
#endif // WIN32

LDLConditionalLineLine::LDLConditionalLineLine(LDLModel *parentModel,
											   const char *line, int lineNumber,
											   const char *originalLine)
	:LDLLineLine(parentModel, line, lineNumber, originalLine),
	m_controlPoints(NULL)
{
}

LDLConditionalLineLine::LDLConditionalLineLine(const LDLConditionalLineLine
											   &other)
	:LDLLineLine(other),
	m_controlPoints(NULL)
{
	if (other.m_controlPoints)
	{
		int i;
		int count = other.getNumControlPoints();

		m_controlPoints = new TCVector[count];
		for (i = 0; i < count; i++)
		{
			m_controlPoints[i] = other.m_controlPoints[i];
		}
	}
}

bool LDLConditionalLineLine::parse(void)
{
	float x1, y1, z1;
	float x2, y2, z2;
	float x3, y3, z3;
	float x4, y4, z4;
	int lineType;

	if (sscanf(m_line, "%d %i %f %f %f %f %f %f %f %f %f %f %f %f", &lineType,
		&m_colorNumber, &x1, &y1, &z1, &x2, &y2, &z2, &x3, &y3, &z3,
		&x4, &y4, &z4) == 14)
	{
		std::string prefix = getTypeAndColorPrefix();
		std::stringstream ss;
		if (!prefix.empty())
		{
			ss << prefix << "  ";
		}
		else
		{
			ss << "4 " << m_colorNumber << "  ";
		}
		ss << x1 << " " << y1 << " " << z1 << "  ";
		ss << x2 << " " << y2 << " " << z2 << "  ";
		ss << x3 << " " << y3 << " " << z3 << "  ";
		ss << x4 << " " << y4 << " " << z4 << "  ";
		m_formattedLine = copyString(ss.str().c_str());
		m_points = new TCVector[2];
		m_points[0] = TCVector(x1, y1, z1);
		m_points[1] = TCVector(x2, y2, z2);
		m_controlPoints = new TCVector[2];
		m_controlPoints[0] = TCVector(x3, y3, z3);
		m_controlPoints[1] = TCVector(x4, y4, z4);
		if (getMatchingPoints())
		{
			setError(LDLEMatchingPoints,
				TCLocalStrings::get(_UC("LDLCondMatchingVertices")));
			return false;
		}
		if (getMatchingPoints(m_controlPoints, getNumControlPoints()))
		{
			setError(LDLEMatchingPoints,
				TCLocalStrings::get(_UC("LDLCondMatchingControlPts")));
			return false;
		}
		return true;
	}
	else
	{
		m_valid = false;
		setError(LDLEParse, TCLocalStrings::get(_UC("LDLCondParse")));
		return false;
	}
}

void LDLConditionalLineLine::dealloc(void)
{
	delete[] m_controlPoints;
	LDLLineLine::dealloc();
}

TCObject *LDLConditionalLineLine::copy(void) const
{
	return new LDLConditionalLineLine(*this);
}

void LDLConditionalLineLine::scanPoints(
	TCObject *scanner,
	LDLScanPointCallback scanPointCallback,
	const TCFloat *matrix,
	bool watchBBoxIgnore) const
{
	if (m_valid)
	{
		int i;
		TCVector point;

		LDLShapeLine::scanPoints(scanner, scanPointCallback, matrix,
			watchBBoxIgnore);
		if (m_parentModel->getMainModel()->getScanConditionalControlPoints())
		{
			for (i = 0; i < 2; i++)
			{
				m_controlPoints[i].transformPoint(matrix, point);
				((*scanner).*scanPointCallback)(point, this);
			}
		}
	}
}
