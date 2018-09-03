#include "LDLEmptyLine.h"

#ifdef WIN32
#if defined(_MSC_VER) && _MSC_VER >= 1400 && defined(_DEBUG)
#define new DEBUG_CLIENTBLOCK
#endif // _DEBUG
#endif // WIN32

LDLEmptyLine::LDLEmptyLine(LDLModel *parentModel, const char *line,
						   int lineNumber, const char *originalLine)
	:LDLFileLine(parentModel, line, lineNumber, originalLine)
{
}

LDLEmptyLine::LDLEmptyLine(const LDLEmptyLine &other)
	:LDLFileLine(other)
{
}

LDLEmptyLine::~LDLEmptyLine(void)
{
}

bool LDLEmptyLine::parse(void)
{
	return true;
}

TCObject *LDLEmptyLine::copy(void) const
{
	return new LDLEmptyLine(*this);
}
