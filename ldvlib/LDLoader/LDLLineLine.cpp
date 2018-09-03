#include "LDLLineLine.h"
#include "LDLMainModel.h"
#include <TCFoundation/TCLocalStrings.h>
#include <sstream>

#ifdef WIN32
#if defined(_MSC_VER) && _MSC_VER >= 1400 && defined(_DEBUG)
#define new DEBUG_CLIENTBLOCK
#endif // _DEBUG
#endif // WIN32

LDLLineLine::LDLLineLine(LDLModel *parentModel, const char *line,
						 int lineNumber, const char *originalLine)
	:LDLShapeLine(parentModel, line, lineNumber, originalLine)
{
}

LDLLineLine::LDLLineLine(const LDLLineLine &other)
	:LDLShapeLine(other)
{
}

bool LDLLineLine::parse(void)
{
	float x1, y1, z1;
	float x2, y2, z2;
	int lineType;

	if (sscanf(m_line, "%d %i %f %f %f %f %f %f", &lineType, &m_colorNumber,
		&x1, &y1, &z1, &x2, &y2, &z2) == 8)
	{
		std::string prefix = getTypeAndColorPrefix();
		std::stringstream ss;
		if (!prefix.empty())
		{
			ss << prefix << "  ";
		}
		else
		{
			ss << "2 " << m_colorNumber << "  ";
		}
		ss << x1 << " " << y1 << " " << z1 << "  ";
		ss << x2 << " " << y2 << " " << z2 << "  ";
		m_formattedLine = copyString(ss.str().c_str());
		m_points = new TCVector[2];
		m_points[0] = TCVector(x1, y1, z1);
		m_points[1] = TCVector(x2, y2, z2);
		if (!getMainModel()->getSkipValidation())
		{
			getMatchingPoints();
		}
		return true;
	}
	else
	{
		m_valid = false;
		setError(LDLEParse, TCLocalStrings::get(_UC("LDLLineParse")));
		return false;
	}
}

TCObject *LDLLineLine::copy(void) const
{
	return new LDLLineLine(*this);
}
