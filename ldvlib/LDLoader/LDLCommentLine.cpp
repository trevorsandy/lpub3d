#include "LDLCommentLine.h"
#include "LDLPalette.h"
#include "LDLMainModel.h"
#include <stdio.h>
#include <TCFoundation/mystring.h>
#include <TCFoundation/TCStringArray.h>
#include <TCFoundation/TCLocalStrings.h>

#ifdef WIN32
#if defined(_MSC_VER) && _MSC_VER >= 1400 && defined(_DEBUG)
#define new DEBUG_CLIENTBLOCK
#endif // _DEBUG
#endif // WIN32

LDLCommentLine::LDLCommentLine(LDLModel *parentModel, const char *line,
							   int lineNumber, const char *originalLine)
	:LDLFileLine(parentModel, line, lineNumber, originalLine),
	m_processedLine(NULL),
	m_words(NULL)
{
	setupProcessedLine();
}

LDLCommentLine::LDLCommentLine(const LDLCommentLine &other)
	:LDLFileLine(other),
	m_processedLine(copyString(other.m_processedLine)),
	m_words((TCStringArray *)TCObject::copy(other.m_words))
{
}

LDLCommentLine::~LDLCommentLine(void)
{
}

void LDLCommentLine::dealloc(void)
{
	delete[] m_processedLine;
	TCObject::release(m_words);
	LDLFileLine::dealloc();
}

bool LDLCommentLine::parse(void)
{
	if (isMovedToMeta())
	{
		if (m_parentModel)
		{
			char *newName = m_processedLine + strlen("0 ~moved to ");
			char partName[1024];
			const char *filename = m_parentModel->getFilename();
			const char *nameSpot = strrchr(filename, '/');
			const char *tmp = strrchr(filename, '\\');

			if (tmp > nameSpot)
			{
				nameSpot = tmp;
			}
			if (nameSpot == NULL)
			{
				nameSpot = newName - 1;
			}
			strcpy(partName, nameSpot + 1);
			tmp = strchr(partName, '.');
			if (tmp)
			{
				partName[tmp - partName] = 0;
			}
			UCSTR ucPartName = mbstoucstring(partName);
			UCSTR ucNewName = mbstoucstring(newName);
			setWarning(LDLEMovedTo,
				TCLocalStrings::get(_UC("LDLComPartRenamed")), ucPartName,
				ucNewName);
			delete[] ucPartName;
			delete[] ucNewName;
		}
		else
		{
			setWarning(LDLEMovedTo,
				TCLocalStrings::get(_UC("LDLComUnknownPartRenamed")));
		}
	}
	else if (isNewGeometryMeta() && m_texmapFilename.size() > 0)
	{
		// We have to be invalid in order for getReplacementLines() to be
		// called.
		m_valid = false;
	}
	else if (m_parentModel)
	{
		LDLMainModel *mainModel = m_parentModel->getMainModel();

		if (mainModel)
		{
			LDLPalette *palette = mainModel->getPalette();

			if (palette)
			{
				if (palette->isColorComment(m_processedLine))
				{
					palette->parseColorComment(m_processedLine);
				}
			}
		}
	}
	return true;
}

TCObject *LDLCommentLine::copy(void) const
{
	return new LDLCommentLine(*this);
}

bool LDLCommentLine::getMPDFilename(char *filename, int maxLength) const
{
//	if (stringHasCaseInsensitivePrefix(m_processedLine, "0 FILE ") &&
	if (stringHasPrefix(m_processedLine, "0 FILE ") &&
		strlen(m_processedLine) > 7)
	{
		if (maxLength)
		{
			strncpy(filename, m_processedLine + 7, maxLength);
			filename[maxLength - 1] = 0;
		}
		return true;
	}
	return false;
}

void LDLCommentLine::setupProcessedLine(void)
{
	if (!m_processedLine)
	{
		char* spot;
		m_processedLine = copyString(m_line);

		// First, strip all leading and trailing whitespace.
		stripLeadingWhitespace(m_processedLine);
		stripTrailingWhitespace(m_processedLine);
		// Next, convert all tabs to spaces.
		while ((spot = strchr(m_processedLine, '\t')) != NULL)
		{
			spot[0] = ' ';
		}
		// Finally, replace all multi-spaces with single spaces.
		spot = m_processedLine;
		while ((spot = strstr(spot, "  ")) != NULL)
		{
			memmove(spot, spot + 1, strlen(spot));
		}
	}
	if (strlen(m_processedLine) > 2)
	{
		int numWords;
		char **words = componentsSeparatedByString(m_processedLine + 2, " ",
			numWords);

		m_words = new TCStringArray(words, numWords);
		deleteStringArray(words, numWords);
	}
	else
	{
		m_words = new TCStringArray(0);
	}
}

bool LDLCommentLine::isNoShrinkMeta(void) const
{
	int numWords = m_words->getCount();

	if (numWords >= 2 && strcasecmp((*m_words)[0], "~lsynth") == 0 &&
		strcasecmp((*m_words)[1], "constraint") == 0)
	{
		return true;
	}
	return false;
}

bool LDLCommentLine::isPrimitiveMeta(void) const
{
	int word = 0;
	int numWords = m_words->getCount();

	if (numWords == 2 && strcasecmp((*m_words)[0], "!ldraw_org") == 0 &&
		(strcasecmp((*m_words)[1], "primitive") == 0 ||
		strcasecmp((*m_words)[1], "48_primitive") == 0 ||
		strcasecmp((*m_words)[1], "unofficial_primitive") == 0 ||
		strcasecmp((*m_words)[1], "unofficial_48_primitive") == 0))
	{
		return true;
	}
	if (numWords >= 2 && (strcasecmp((*m_words)[0], "unofficial") == 0 ||
		strcasecmp((*m_words)[0], "un-official") == 0 ||
		strcasecmp((*m_words)[0], "ldraw_org") == 0 ||
		strcasecmp((*m_words)[0], "original") == 0))
	{
		if (strcasecmp((*m_words)[1], "ldraw") == 0)
		{
			word = 2;
		}
		else
		{
			word = 1;
		}
	}
	else if (numWords >= 3 && strcasecmp((*m_words)[0], "original") == 0 &&
		strcasecmp((*m_words)[1], "ldraw") == 0 &&
		strcasecmp((*m_words)[2], "primitive") == 0)
	{
		return true;
	}
	if (word && strcasecmp((*m_words)[word], "primitive") == 0)
	{
		return true;
	}
	return false;
}

bool LDLCommentLine::isOfficialPartMeta(bool partMeta) const
{
	if (partMeta)
	{
		if ((strcasecmp((*m_words)[0], "!ldraw_org") == 0 &&
			strcasecmp((*m_words)[1], "part") == 0) ||
			strcasecmp((*m_words)[0], "ldraw_org") == 0 ||
			strcasecmp((*m_words)[0], "official") == 0 ||
			strcasecmp((*m_words)[0], "original") == 0)
		{
			return true;
		}
	}
	return false;
}

bool LDLCommentLine::isPartMeta(void) const
{
	int word = 0;
	int numWords = m_words->getCount();

	if (numWords >= 2 && strcasecmp((*m_words)[0], "!ldraw_org") == 0 &&
		(strcasecmp((*m_words)[1], "part") == 0 ||
		strcasecmp((*m_words)[1], "unofficial_part") == 0 ||
		strcasecmp((*m_words)[1], "shortcut") == 0 ||
		strcasecmp((*m_words)[1], "unofficial_shortcut") == 0))
	{
		return true;
	}
	else if (numWords >= 2 && (strcasecmp((*m_words)[0], "unofficial") == 0 ||
		strcasecmp((*m_words)[0], "un-official") == 0 ||
		strcasecmp((*m_words)[0], "ldraw_org") == 0 ||
		strcasecmp((*m_words)[0], "custom") == 0))
	{
		if (strcasecmp((*m_words)[1], "ldraw") == 0)
		{
			word = 2;
		}
		else
		{
			word = 1;
		}
	}
	else if (numWords >= 3 && strcasecmp((*m_words)[0], "official") == 0 &&
		strcasecmp((*m_words)[1], "lcad") == 0)
	{
		if (strcasecmp((*m_words)[2], "update") == 0)
		{
			return true;
		}
		word = 2;
	}
	else if (numWords >= 3 && strcasecmp((*m_words)[0], "original") == 0 &&
		strcasecmp((*m_words)[1], "ldraw") == 0 &&
		strcasecmp((*m_words)[2], "part") == 0)
	{
		return true;
	}
	if (word && (strcasecmp((*m_words)[word], "part") == 0 ||
		strcasecmp((*m_words)[word], "element") == 0))
	{
		return true;
	}
	return false;
}

bool LDLCommentLine::isLDViewMeta(void) const
{
	return stringHasCaseInsensitivePrefix(m_processedLine, "0 !LDVIEW ");
}

bool LDLCommentLine::isTexmapMeta(void) const
{
	return stringHasPrefix(m_processedLine, "0 !TEXMAP ");
}

bool LDLCommentLine::isDataMeta(void) const
{
	return stringHasPrefix(m_processedLine, "0 !DATA ");
}

bool LDLCommentLine::isNewGeometryMeta(void) const
{
	return stringHasPrefix(m_processedLine, "0 !: ");
}

bool LDLCommentLine::isDataRowMeta(void) const
{
	return stringHasPrefix(m_processedLine, "0 !:");
}

bool LDLCommentLine::isBBoxIgnoreMeta(void) const
{
	return isLDViewMeta() && containsCommand("BBOX_IGNORE", 1, true, 1);
	//// Note: second part is to verify that BBOX_IGNORE has a space after it.
	//return stringHasPrefix(m_processedLine, "0 !LDVIEW BBOX_IGNORE") &&
	//	strcasecmp((*m_words)[1], "BBOX_IGNORE") == 0;
}

bool LDLCommentLine::containsCommand(
	const char *command,
	int startWord,
	bool caseSensitive /*= false*/,
	int endWord /*= -1*/) const
{
	int i;
	if (endWord == -1)
	{
		endWord = m_words->getCount() - 1;
	}
	for (i = startWord; i <= endWord; i++)
	{
		if ((caseSensitive && strcmp((*m_words)[i], command) == 0) ||
			(!caseSensitive && strcasecmp((*m_words)[i], command) == 0))
		{
			return true;
		}
	}
	return false;
}

bool LDLCommentLine::containsBBoxIgnoreCommand(const char *command) const
{
	if (isBBoxIgnoreMeta())
	{
		return containsCommand(command, 2, true);
	}
	return false;
}

bool LDLCommentLine::isBFCMeta(void) const
{
//	if (stringHasCaseInsensitivePrefix(m_processedLine, "0 BFC "))
	if (stringHasPrefix(m_processedLine, "0 BFC "))
	{
		return true;
	}
	return false;
}

bool LDLCommentLine::isOBIMeta(void) const
{
	return(stringHasPrefix(m_processedLine, "0 !OBI "));
}

// OBI

// 0 !OBI SET <token>
// 0 !OBI UNSET <token>
// 0 !OBI NEXT <color> [IFSET <token>|IFNSET <token>] 
// 0 !OBI START <color> [IFSET <token>|IFNSET <token>]
// 0 !OBI END
LDLCommentLine::OBICommand LDLCommentLine::getOBICommand(void) const
{
	OBICommand result = OBICERROR;

	if (m_words->getCount() > 1)
	{
		if (strcasecmp((*m_words)[1], "set") == 0)
		{
			result = OBICSet;
		}
		else if (strcasecmp((*m_words)[1], "unset") == 0)
		{
			result = OBICUnset;
		}
		else if (strcasecmp((*m_words)[1], "next") == 0)
		{
			result = OBICNext;
		}
		else if (strcasecmp((*m_words)[1], "start") == 0)
		{
			result = OBICStart;
		}
		else if (strcasecmp((*m_words)[1], "end") == 0)
		{
			result = OBICEnd;
		}
	}
	return result;
}

bool LDLCommentLine::hasOBIConditional(void) const
{
	OBICommand cmd = getOBICommand();

	if (cmd == OBICNext || cmd == OBICStart)
	{
		// 0 !OBI NEXT <color> <IFcommand> <token>
		// 0 !OBI START <color> <IFcommand> <token>
		if (m_words->getCount() > 3)
		{
			return (strcasecmp((*m_words)[3], "ifset") == 0)
				|| (strcasecmp((*m_words)[3], "ifnset") == 0);
		}
	}
	return false;
}

bool LDLCommentLine::getOBIConditional(void) const
{
	OBICommand cmd = getOBICommand();

	if (cmd == OBICNext || cmd == OBICStart)
	{
		// 0 !OBI NEXT <color> <IFcommand>
		// 0 !OBI START <color> <IFcommand>
		if (m_words->getCount() > 3)
		{
			if (strcasecmp((*m_words)[3], "ifset") == 0)
				return true;
			else if (strcasecmp((*m_words)[3], "ifnset") == 0)
				return false;
		}
	}
	// error condition here
	return true;
}

bool LDLCommentLine::hasOBIToken(void) const
{
	OBICommand cmd = getOBICommand();

	if ((cmd == OBICSet || cmd == OBICUnset) && m_words->getCount() > 2)
	{
		return true;
	}
	if ((cmd == OBICNext || cmd == OBICStart) && m_words->getCount() > 3)
	{
		return true;
	}
	return false;
}

const char *LDLCommentLine::getOBIToken(void) const
{
	if (hasOBIToken())
	{
		OBICommand cmd = getOBICommand();

		// 0 !OBI SET <token>
		if (cmd == OBICSet || cmd == OBICUnset)
		{
			return (*m_words)[2];
		}
		// 0 !OBI START <color> IFSET <token>
		// 0 !OBI START <color> IFNSET <token>
		// 0 !OBI NEXT <color> IFSET <token>
		// 0 !OBI NEXT <color> IFNSET <token>
		if (cmd == OBICNext || cmd == OBICStart)
		{
			return (*m_words)[4];
		}
	}
	return 0;
}

int LDLCommentLine::getOBIColorNumber(void) const
{
	int result = 0;

	// 0 !OBI NEXT <color> [IF...]
	if (m_words->getCount() > 2)
	{
		char *word = (*m_words)[2];

		if (isdigit(word[0]))
		{
			result = atoi(word);
		}
		else
		{
			if (m_parentModel != NULL)
			{
				LDLMainModel *mainModel = m_parentModel->getMainModel();

				if (mainModel != NULL)
				{
					LDLPalette *palette = mainModel->getPalette();

					if (palette != NULL)
					{
						int colorNumber = palette->getColorNumberForName(word);

						if (colorNumber != -1)
						{
							result = colorNumber;
						}
					}
				}
			}
		}
	}
	return result;
}


/* bool LDLCommentLine::isOBICommandConditional(void) const
{
	bool result = false;

	OBICommand cmd = getOBICommand();

	if (OBICommand == OBICNext || OBICommand == OBICStart)
	{
		// 0 !OBI NEXT <color> <IFcommand>
		// 0 !OBI START <color> <IFcommand>
		if (m_words->getCount() > 3)
		{
			result = (strcasecmp((*m_words[3], "ifset") == 0)
				|| (strcasecmp((*m_words[3], "ifnset") == 0));
		}
	}

	return result;
}
*/

// /OBI

bool LDLCommentLine::isMovedToMeta(void) const
{
	if (stringHasCaseInsensitivePrefix(m_processedLine, "0 ~moved to "))
	{
		return true;
	}
	return false;
}

bool LDLCommentLine::isStepMeta(void) const
{
	if (stringHasCaseInsensitivePrefix(m_processedLine, "0 step") ||
		stringHasCaseInsensitivePrefix(m_processedLine, "0 rotstep"))
	{
		return true;
	}
	return false;
}

bool LDLCommentLine::getAuthor(char *author, int maxLength) const
{
	const char *authorSpot = NULL;
	if (stringHasCaseInsensitivePrefix(m_processedLine, "0 author "))
	{
		authorSpot = &m_processedLine[strlen("0 author ")];
	}
	else if (stringHasCaseInsensitivePrefix(m_processedLine, "0 author: "))
	{
		authorSpot = &m_processedLine[strlen("0 author: ")];
	}
	if (authorSpot)
	{
		strncpy(author, authorSpot, maxLength);
		author[maxLength - 1] = 0;
		return true;
	}
	return false;
}

bool LDLCommentLine::containsBFCCommand(const char *command) const
{
	if (isBFCMeta())
	{
		return containsCommand(command, 1, true);
	}
	return false;
}

bool LDLCommentLine::containsTexmapCommand(const char *command) const
{
	if (isTexmapMeta())
	{
		return containsCommand(command, 1, true, 1);
	}
	return false;
}

bool LDLCommentLine::containsDataCommand(const char *command) const
{
	if (isDataMeta())
	{
		return containsCommand(command, 1, true, 1);
	}
	return false;
}

const char *LDLCommentLine::getWord(int index) const
{
	return m_words->stringAtIndex(index);
}

int LDLCommentLine::getNumWords(void) const
{
	return m_words->getCount();
}

LDLFileLineArray *LDLCommentLine::getReplacementLines(void)
{
	LDLFileLineArray *fileLineArray = NULL;

	// Note: if m_texmapFilename.size() is 0 and this is a 0 !: line, that means
	// that either the !: is bogus, or the texture map image couldn't be found.
	// Either way, we do NOT want to turn the !: line into geometry.
	if (m_texmapFilename.size() > 0 && isNewGeometryMeta())
	{
		std::string newLine = &m_processedLine[5];
		LDLFileLine *fileLine = LDLFileLine::initFileLine(m_parentModel,
			newLine.c_str(), m_lineNumber, m_line);
		if (fileLine != NULL)
		{
			if (fileLine->isActionLine())
			{
				fileLineArray = new LDLFileLineArray(1);
				fileLineArray->addObject(fileLine);
			}
			fileLine->release();
		}
	}
	return fileLineArray;
}
