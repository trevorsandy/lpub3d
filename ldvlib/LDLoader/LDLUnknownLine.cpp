#include "LDLUnknownLine.h"
#include <TCFoundation/TCLocalStrings.h>

#ifdef WIN32
#if defined(_MSC_VER) && _MSC_VER >= 1400 && defined(_DEBUG)
#define new DEBUG_CLIENTBLOCK
#endif // _DEBUG
#endif // WIN32

LDLUnknownLine::LDLUnknownLine(LDLModel *parentModel, const char *line,
							   int lineNumber, const char *originalLine)
	:LDLFileLine(parentModel, line, lineNumber, originalLine)
{
}

LDLUnknownLine::LDLUnknownLine(const LDLUnknownLine &other)
	:LDLFileLine(other)
{
}

LDLUnknownLine::~LDLUnknownLine(void)
{
}

bool LDLUnknownLine::parse(void)
{
	setError(LDLEParse, TCLocalStrings::get(_UC("LDLUnknownLineParse")));
	return false;
}

TCObject *LDLUnknownLine::copy(void) const
{
	return new LDLUnknownLine(*this);
}
