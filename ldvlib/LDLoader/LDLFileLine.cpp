#include "LDLFileLine.h"
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <TCFoundation/mystring.h>
#include <TCFoundation/TCStringArray.h>
#include <TCFoundation/TCImage.h>

#include "LDLMainModel.h"
#include "LDLCommentLine.h"
#include "LDLModelLine.h"
#include "LDLLineLine.h"
#include "LDLTriangleLine.h"
#include "LDLQuadLine.h"
#include "LDLConditionalLineLine.h"
#include "LDLEmptyLine.h"
#include "LDLUnknownLine.h"

#ifdef WIN32
#if defined(_MSC_VER) && _MSC_VER >= 1400 && defined(_DEBUG)
#define new DEBUG_CLIENTBLOCK
#endif // _DEBUG
#endif // WIN32

LDLFileLine::LDLFileLine(LDLModel *parentModel, const char *line,
						 int lineNumber, const char *originalLine)
	:m_parentModel(parentModel),
	m_line(copyString(line)),
	m_originalLine(copyString(originalLine)),
	m_formattedLine(NULL),
	m_lineNumber(lineNumber),
	m_error(NULL),
	m_valid(true),
	m_replaced(false),
	m_stepIndex(-1),
	m_texmapImage(NULL)
{
}

LDLFileLine::LDLFileLine(const LDLFileLine &other)
	:m_parentModel(other.m_parentModel),
	m_line(copyString(other.m_line)),
	m_originalLine(copyString(other.m_originalLine)),
	m_formattedLine(NULL),
	m_lineNumber(other.m_lineNumber),
	m_error(TCObject::retain(other.m_error)),
	m_valid(other.m_valid),
	m_replaced(other.m_replaced),
	m_stepIndex(other.m_stepIndex),
	m_texmapImage(TCObject::retain(other.m_texmapImage))
{
}

LDLFileLine::~LDLFileLine(void)
{
}

void LDLFileLine::dealloc(void)
{
	delete[] m_line;
	delete[] m_originalLine;
	delete[] m_formattedLine;
	TCObject::release(m_error);
	TCObject::release(m_texmapImage);
	m_error = NULL;
	TCObject::dealloc();
}

LDLFileLine::operator const char *(void) const
{
	if (m_originalLine)
	{
		return m_originalLine;
	}
	else
	{
		return m_line;
	}
}

void LDLFileLine::setErrorV(LDLErrorType type, CUCSTR format, va_list argPtr)
{
	if (m_error)
	{
		m_error->release();
	}
	m_error = m_parentModel->newError(type, *this, format, argPtr);
}

void LDLFileLine::setError(LDLErrorType type, CUCSTR format, ...)
{
	va_list argPtr;

	va_start(argPtr, format);
	setErrorV(type, format, argPtr);
	va_end(argPtr);
}

void LDLFileLine::setWarning(LDLErrorType type, CUCSTR format, ...)
{
	va_list argPtr;

	va_start(argPtr, format);
	setErrorV(type, format, argPtr);
	va_end(argPtr);
	m_error->setLevel(LDLAWarning);
}

int LDLFileLine::scanLineType(const char *line)
{
	int num;

	if (sscanf(line, "%d", &num) == 1)
	{
		return num;
	}
	return -1;
}

bool LDLFileLine::lineIsEmpty(const char *line)
{
	while (line[0])
	{
		char thisChar = line[0];

		if (thisChar != ' ' && thisChar != '\t')
		{
			return false;
		}
		line++;
	}
	return true;
}

LDLFileLine *LDLFileLine::initFileLine(
	LDLModel *parentModel,
	const char *line,
	int lineNumber,
	const char *originalLine /*= NULL*/)
{
	if (strlen(line))
	{
		switch (scanLineType(line))
		{
		case 0:
			return new LDLCommentLine(parentModel, line, lineNumber,
				originalLine);
			break;
		case 1:
			return new LDLModelLine(parentModel, line, lineNumber,
				originalLine);
			break;
		case 2:
			return new LDLLineLine(parentModel, line, lineNumber, originalLine);
			break;
		case 3:
			return new LDLTriangleLine(parentModel, line, lineNumber,
				originalLine);
			break;
		case 4:
			return new LDLQuadLine(parentModel, line, lineNumber,
				originalLine);
			break;
		case 5:
			return new LDLConditionalLineLine(parentModel, line, lineNumber,
				originalLine);
			break;
		}
	}
	if (lineIsEmpty(line))
	{
		return new LDLEmptyLine(parentModel, line, lineNumber, originalLine);
	}
	else
	{
		return new LDLUnknownLine(parentModel, line, lineNumber);
	}
}

void LDLFileLine::print(int /*indent*/) const
{
//	printf("%d: %s\n", m_lineNumber, m_line);
}

const char *LDLFileLine::getFormattedLine(void) const
{
	if (m_formattedLine)
	{
		return m_formattedLine;
	}
	else if (m_originalLine)
	{
		return m_originalLine;
	}
	else if (m_line)
	{
		return m_line;
	}
	return NULL;
}

LDLFileLineArray *LDLFileLine::getReplacementLines(void)
{
	return NULL;
}

bool LDLFileLine::isXZPlanar(void) const
{
	// This returns true if the all the points in this file line have Y == 0.
	// It is implemented in sub-classes that care.
	return true;
}

bool LDLFileLine::isXZPlanar(const TCFloat * /*matrix*/) const
{
	// This returns true if the all the points in this file line have Y == 0,
	// after being transformed by matrix.
	// It is implemented in sub-classes that care.
	return true;
}

LDLMainModel *LDLFileLine::getMainModel(void)
{
	return m_parentModel->getMainModel();
}

const LDLMainModel *LDLFileLine::getMainModel(void) const
{
	return m_parentModel->getMainModel();
}

const char *LDLFileLine::findWord(int index) const
{
	int n;
	int i = 0;

	while (isspace(m_line[i]) && m_line[i] != 0)
	{
		i++;
	}
	for (n = 0; n < index; n++)
	{
		while (!isspace(m_line[i]) && m_line[i] != 0)
		{
			i++;
		}
		while (isspace(m_line[i]) && m_line[i] != 0)
		{
			i++;
		}
	}
	return &m_line[i];
}

TCObject *LDLFileLine::getAlertSender(void)
{
	return m_parentModel->getMainModel()->getAlertSender();
}

void LDLFileLine::setParentModel(LDLModel *value)
{
	// m_parentModel isn't retained.
	m_parentModel = value;
}

void LDLFileLine::setTexmapSettings(
	TexmapType type,
	const std::string &filename,
	TCImage *image,
	const TCVector *points,
	const TCFloat *extra)
{
	m_texmapType = type;
	if (image != m_texmapImage)
	{
		TCObject::release(m_texmapImage);
		m_texmapImage = TCObject::retain(image);
	}
	m_texmapFilename = filename;
	m_texmapPoints[0] = points[0];
	m_texmapPoints[1] = points[1];
	m_texmapPoints[2] = points[2];
	memcpy(m_texmapExtra, extra, sizeof(m_texmapExtra));
}

static size_t skipSpace(const char* input)
{
	size_t i;
	for (i = 0; input[i] && isspace(input[i]); ++i)
	{
		// do nothing.
	}
	return i;
}

static size_t skipNonSpace(const char* input)
{
	size_t i;
	for (i = 0; input[i] && !isspace(input[i]); ++i)
	{
		// do nothing.
	}
	return i;
}

std::string LDLFileLine::getTypeAndColorPrefix(void) const
{
	std::string prefix;
	if (m_line)
	{
		size_t prefixIndex = skipSpace(m_line);
		prefixIndex += skipNonSpace(&m_line[prefixIndex]);
		prefixIndex += skipSpace(&m_line[prefixIndex]);
		prefixIndex += skipNonSpace(&m_line[prefixIndex]);
		if (m_line[prefixIndex])
		{
			prefix.insert(prefix.begin(), &m_line[0], &m_line[prefixIndex]);
		}
	}
	return prefix;
}
