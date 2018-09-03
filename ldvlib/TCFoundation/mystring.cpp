#include "mystring.h"
#include "ConvertUTF.h"
#include "TCUserDefaults.h"

#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <stdarg.h>

#ifndef WIN32
#include <sys/stat.h>
#endif // !WIN32

#ifdef WIN32
#include <time.h>

#if defined(_MSC_VER) && _MSC_VER >= 1400 && defined(_DEBUG)
#define new DEBUG_CLIENTBLOCK
#endif // _DEBUG

// In Windows, we normally don't have a console.  However, we may have a
// console.
bool g_haveConsole = false;
bool g_bRealConsole = false;
HANDLE g_hStdOut = NULL;

class ConsoleBuffer
{
public:
	~ConsoleBuffer();
	void vprintf(const char *format, va_list argPtr);
	void vwprintf(const wchar_t *format, va_list argPtr);
private:
	ucstring m_buffer;
};

ConsoleBuffer g_consoleBuffer;

ConsoleBuffer::~ConsoleBuffer()
{
	if (m_buffer.size() > 0)
	{
#ifdef TC_NO_UNICODE
		MessageBoxA(NULL, m_buffer.c_str(), "NativePOVGen console output", MB_OK);
#else // TC_NO_UNICODE
		MessageBoxW(NULL, m_buffer.c_str(), L"NativePOVGen console output", MB_OK);
#endif // TC_NO_UNICODE
	}
}

void ConsoleBuffer::vprintf(const char *format, va_list argPtr)
{
	std::string temp;

#if _MSC_VER < 1400	// VC < VC 2005
	int size;
	temp.resize(65536);
	size = vsprintf(&temp[0], format, argPtr);
	temp.resize(size);
#else
	temp.resize(_vscprintf(format, argPtr));
	vsprintf(&temp[0], format, argPtr);
#endif
#ifdef TC_NO_UNICODE
	m_buffer += temp;
#else // TC_NO_UNICODE
	std::wstring wtemp;
	stringtowstring(wtemp, temp);
	m_buffer += wtemp;
#endif // TC_NO_UNICODE
}

void ConsoleBuffer::vwprintf(const wchar_t *format, va_list argPtr)
{
	std::wstring wtemp;

#if _MSC_VER < 1400	// VC < VC 2005
	int size;
	wtemp.resize(65536);
	size = vswprintf(&wtemp[0], format, argPtr);
	wtemp.resize(size);
#else
	// While std::wstring (and std::string) appear to always allocate space for
	// a terminating NULL character, their documentation indicates that they
	// might not.
	wtemp.resize(_vscwprintf(format, argPtr) + 1);
	vswprintf(&wtemp[0], wtemp.size(), format, argPtr);
	wtemp.resize(wtemp.size() - 1);
#endif
#ifdef TC_NO_UNICODE
	std::string temp;
	wstringtostring(temp, wtemp);
	m_buffer += temp;
#else // TC_NO_UNICODE
	m_buffer += wtemp;
#endif // TC_NO_UNICODE
}

#else // WIN32

#include <stdlib.h>
#include <wchar.h>

#endif

#ifdef DEBUG
static int debugLevel = 1;
#else // DEBUG
static int debugLevel = 0;
#endif // DEBUG

static size_t lastSlashIndex(const ucstring &path);
static size_t lastSlashIndex(const std::string &path);

int sucprintf(UCSTR buffer, size_t maxLen, CUCSTR format, ...)
{
	va_list argPtr;
	int retValue;

	va_start(argPtr, format);
	retValue = vsucprintf(buffer, maxLen, format, argPtr);
	va_end(argPtr);
	return retValue;
}

#if (defined(_MSC_VER) && _MSC_VER < 1400) || defined(TC_NO_UNICODE)
int vsucprintf(UCSTR buffer, size_t /*maxLen*/, CUCSTR format, va_list argPtr)
#else // Not VC, or VC 2005+
int vsucprintf(UCSTR buffer, size_t maxLen, CUCSTR format, va_list argPtr)
#endif
{
#ifdef TC_NO_UNICODE
	return vsprintf(buffer, format, argPtr);
#else // TC_NO_UNICODE
	size_t formatLen = wcslen(format);
	wchar_t *newFormat = new wchar_t[formatLen + 1];
	wchar_t *spot;
	int retValue;

	wcscpy(newFormat, format);
	while ((spot = wcsstr(newFormat, L"%s")) != NULL)
	{
		wchar_t *temp = new wchar_t[formatLen + 2];
		size_t offset = spot - newFormat;

		wcsncpy(temp, newFormat, offset);
		wcscpy(&temp[offset], L"%ls");
		wcscpy(&temp[offset + 3], &spot[2]);
		delete[] newFormat;
		newFormat = temp;
		formatLen++;
	}
#if defined(_MSC_VER) && _MSC_VER < 1400	// VC < VC 2005
	retValue = vswprintf(buffer, newFormat, argPtr);
#else // Not VC, or VC 2005+
	retValue = vswprintf(buffer, maxLen, newFormat, argPtr);
#endif
	delete[] newFormat;
	return retValue;
#endif // TC_NO_UNICODE
}

char *copyString(const char *string, size_t pad)
{
	if (string)
	{
		return strcpy(new char[strlen(string) + 1 + pad], string);
	}
	else
	{
		return NULL;
	}
}

wchar_t *copyString(const wchar_t *string, size_t pad)
{
	if (string)
	{
#ifdef __clang_analyzer__
		wchar_t *result = new wchar_t[wcslen(string) + 1 + pad];
		wcscpy(result, string);
		return result;
#else
		// The Xcode static analyzer thinks that the below leaks, even though
		// it doesn't.
		return wcscpy(new wchar_t[wcslen(string) + 1 + pad], string);
#endif
	}
	else
	{
		return NULL;
	}
}

#ifndef __APPLE__

char *strnstr(const char *s1, const char *s2, size_t n)
{
	return strnstr2(s1, s2, n, 0);
}

#ifndef __USE_GNU
char *strcasestr(const char *s1, const char *s2) __THROW
{
	char* spot;
	int len1 = (int)strlen(s1);
	int len2 = (int)strlen(s2);

	for (spot = (char*)s1; spot - s1 <= len1 - len2; spot++)
	{
		if (strncasecmp(spot, s2, len2) == 0)
		{
			return spot;
		}
	}
	return NULL;
}
#endif // !__USE_GNU

#endif // !__APPLE__

char *strnstr2(const char *s1, const char *s2, size_t n, int skipZero)
{
	char* spot;
	size_t len2 = strlen(s2);

	for (spot = (char*)s1; (*spot != 0 || skipZero) &&
		(unsigned)(spot-s1) < n; spot++)
	{
		if (strncmp(spot, s2, len2) == 0)
		{
			return spot;
		}
	}
	return NULL;
}

char *strncasestr(const char *s1, const char *s2, size_t n, int skipZero)
{
	char* spot;
	size_t len2 = strlen(s2);

	for (spot = (char*)s1; (*spot != 0 || skipZero) &&
		(unsigned)(spot - s1) < n; spot++)
	{
		if (strncasecmp(spot, s2, len2) == 0)
		{
			return spot;
		}
	}
	return NULL;
}

void printStringArray(char** array, int count)
{
	int i;

	for (i = 0; i < count; i++)
	{
		printf("<%s>\n", array[i]);
	}
}

char **copyStringArray(char** array, int count)
{
	if (array)
	{
		int i;
		char **newArray = new char*[count];

		for (i = 0; i < count; i++)
		{
			newArray[i] = copyString(array[i]);
		}
		return newArray;
	}
	else
	{
		return NULL;
	}
}

/*
template<class T>void deleteStringArray(T** array, int count)
{
	int i;

	for (i = 0; i < count; i++)
	{
		delete[] array[i];
	}
	delete[] array;
}
*/
/*
void deleteStringArray(wchar_t** array, int count)
{
	int i;

	for (i = 0; i < count; i++)
	{
		delete[] array[i];
	}
	delete[] array;
}
*/
bool arrayContainsString(char** array, int count, const char* string)
{
	int i;

	for (i = 0; i < count; i++)
	{
		if (strcmp(array[i], string) == 0)
		{
			return true;
		}
	}
	return false;
}

bool arrayContainsPrefix(char** array, int count, const char* prefix)
{
	int i;
	size_t prefixLength = strlen(prefix);

	for (i = 0; i < count; i++)
	{
		if (strncmp(array[i], prefix, prefixLength) == 0)
		{
			return true;
		}
	}
	return false;
}

char** componentsSeparatedByString(const char* string, const char* separator,
								   int &count)
{
	int i;
	char* spot = (char*)string;
	char* tokenEnd = NULL;
	size_t separatorLength = strlen(separator);
	char** components;
	char* stringCopy;

	if (strlen(string) == 0)
	{
		count = 0;
		return NULL;
	}
	for (i = 0; (spot = strstr(spot, separator)) != NULL; i++)
	{
		spot += separatorLength;
	}
	count = i + 1;
	components = new char*[count];
	stringCopy = new char[strlen(string) + 1];
	strcpy(stringCopy, string);
	tokenEnd = strstr(stringCopy, separator);
	if (tokenEnd)
	{
		*tokenEnd = 0;
	}
	spot = stringCopy;
	for (i = 0; i < count; i++)
	{
		if (spot)
		{
			components[i] = new char[strlen(spot) + 1];
			strcpy(components[i], spot);
		}
		else
		{
			components[i] = new char[1];
			components[i][0] = 0;
		}
		if (tokenEnd)
		{
			spot = tokenEnd + separatorLength;
		}
		else
		{
			spot = NULL;
		}
		if (spot)
		{
			tokenEnd = strstr(spot, separator);
			if (tokenEnd)
			{
				*tokenEnd = 0;
			}
		}
		else
		{
			tokenEnd = NULL;
		}
	}
	delete[] stringCopy;
	return components;
}

wchar_t** componentsSeparatedByString(const wchar_t* string,
									  const wchar_t* separator,
									  int &count)
{
	int i;
	wchar_t* spot = (wchar_t*)string;
	wchar_t* tokenEnd = NULL;
	size_t separatorLength = wcslen(separator);
	wchar_t** components;
	wchar_t* stringCopy;

	if (wcslen(string) == 0)
	{
		count = 0;
		return NULL;
	}
	for (i = 0; (spot = wcsstr(spot, separator)) != NULL; i++)
	{
		spot += separatorLength;
	}
	count = i + 1;
	components = new wchar_t*[count];
	stringCopy = copyString(string);
	tokenEnd = wcsstr(stringCopy, separator);
	if (tokenEnd)
	{
		*tokenEnd = 0;
	}
	spot = stringCopy;
	for (i = 0; i < count; i++)
	{
		if (spot)
		{
			components[i] = new wchar_t[wcslen(spot) + 1];
			wcscpy(components[i], spot);
		}
		else
		{
			components[i] = new wchar_t[1];
			components[i][0] = 0;
		}
		if (tokenEnd)
		{
			spot = tokenEnd + separatorLength;
		}
		else
		{
			spot = NULL;
		}
		if (spot)
		{
			tokenEnd = wcsstr(spot, separator);
			if (tokenEnd)
			{
				*tokenEnd = 0;
			}
		}
		else
		{
			tokenEnd = NULL;
		}
	}
	delete[] stringCopy;
	return components;
}

char* componentsJoinedByString(char** array, int count, const char* separator)
{
	size_t length = 0;
	int i;
	size_t separatorLength = strlen(separator);
	char* string;

	for (i = 0; i < count; i++)
	{
		length += strlen(array[i]);
		if (i < count - 1)
		{
			length += separatorLength;
		}
	}
	string = new char[length + 1];
	string[0] = 0;
	for (i = 0; i < count - 1; i++)
	{
		strcat(string, array[i]);
#ifndef __clang_analyzer__
		// The Xcode static analyzer has a bug where it thinks we can get here
		// if count is 0. Since both i and count are signed, that's not
		// possible.
		strcat(string, separator);
#endif
	}
	if (count)
	{
		strcat(string, array[count - 1]);
	}
	return string;
}

bool stringHasCaseInsensitivePrefix(const char* string, const char* prefix)
{
	int i;

	for (i = 0; string[i] && prefix[i] &&
		toupper(string[i]) == toupper(prefix[i]); i++)
	{
		// Do nothing
	}
	return prefix[i] == 0;
}

bool stringHasCaseInsensitivePrefix(const wchar_t* string,
	const wchar_t* prefix)
{
	int i;

	for (i = 0; string[i] && prefix[i] &&
		toupper(string[i]) == toupper(prefix[i]); i++)
	{
		// Do nothing
	}
	return prefix[i] == 0;
}

bool stringHasPrefix(const char* string, const char* prefix)
{
	int i;

	for (i = 0; string[i] && prefix[i] && string[i] == prefix[i]; i++)
	{
		// Do nothing
	}
	return prefix[i] == 0;
}

TCExport bool stringHasPrefix(const wchar_t *string, const wchar_t *prefix)
{
	int i;

	for (i = 0; string[i] && prefix[i] && string[i] == prefix[i]; i++)
	{
		// Do nothing
	}
	return prefix[i] == 0;
}

bool stringHasCaseInsensitiveSuffix(const char* string, const char* suffix)
{
	size_t i;
	size_t len1 = strlen(string);
	size_t len2 = strlen(suffix);

	for (i = 0; i < len1 && i < len2 &&
		toupper(string[len1 - i - 1]) == toupper(suffix[len2 - i - 1]); i++)
	{
		// Do nothing
	}
	return i == len2;
}

bool stringHasCaseInsensitiveSuffix(
	const wchar_t* string,
	const wchar_t* suffix)
{
	size_t i;
	size_t len1 = wcslen(string);
	size_t len2 = wcslen(suffix);

	for (i = 0; i < len1 && i < len2 &&
		toupper(string[len1 - i - 1]) == toupper(suffix[len2 - i - 1]); i++)
	{
		// Do nothing
	}
	return i == len2;
}

bool stringHasSuffix(const char* string, const char* suffix)
{
	size_t i;
	size_t len1 = strlen(string);
	size_t len2 = strlen(suffix);

	for (i = 0; i < len1 && i < len2 &&
		string[len1 - i - 1] == suffix[len2 - i - 1]; i++)
	{
		// Do nothing
	}
	return i == len2;
}

bool stringHasSuffix(const wchar_t* string, const wchar_t* suffix)
{
	size_t i;
	size_t len1 = wcslen(string);
	size_t len2 = wcslen(suffix);

	for (i = 0; i < len1 && i < len2 &&
		string[len1 - i - 1] == suffix[len2 - i - 1]; i++)
	{
		// Do nothing
	}
	return i == len2;
}

std::string lowerCaseString(const std::string &src)
{
	std::string retValue = src;

	convertStringToLower(&retValue[0]);
	return retValue;
}

std::string upperCaseString(const std::string &src)
{
	std::string retValue = src;

	convertStringToUpper(&retValue[0]);
	return retValue;
}

char* convertStringToUpper(char* string)
{
	size_t length = strlen(string);
	size_t i;

	for (i = 0; i < length; i++)
	{
		string[i] = (char)toupper(string[i]);
	}
	return string;
}

char* convertStringToLower(char* string)
{
	size_t length = strlen(string);
	size_t i;

	for (i = 0; i < length; i++)
	{
		string[i] = (char)tolower(string[i]);
	}
	return string;
}

char* findExecutable(const char* executable)
{
	char *path = getenv("PATH");
	char *retValue = NULL;
	int pathCount;
	char **pathComponents = componentsSeparatedByString(path, ":", pathCount);
	int i;

	for (i = 0; i < pathCount && retValue == NULL; i++)
	{
		FILE *file;

		retValue = copyString(pathComponents[i], 7);
		strcat(retValue, "/");
		strcat(retValue, executable);
		file = ucfopen(retValue, "r");
		if (file)
		{
			fclose(file);
		}
		else
		{
			delete[] retValue;
			retValue = NULL;
		}
	}
	deleteStringArray(pathComponents, pathCount);
	return retValue;
}

bool isDirectoryPath(const char* path)
{
	size_t len = strlen(path);
	
	if (len > 0)
	{
#ifdef WIN32
		if (path[len - 1] == '\\')
		{
			return true;
		}
#endif // WIN32
		if (path[len - 1] == '/')
		{
			return true;
		}
	}
	return false;
}

bool isDirectoryPath(const std::string &path)
{
	size_t len = path.length();
	
	if (len > 0)
	{
#ifdef WIN32
		if (path[len - 1] == '\\')
		{
			return true;
		}
#endif // WIN32
		if (path[len - 1] == '/')
		{
			return true;
		}
	}
	return false;
}

bool isRelativePath(const char* path)
{
#ifdef WIN32
	if (strlen(path) >= 3 && isalpha(path[0]) && path[1] == ':' &&
		(path[2] == '\\' || path[2] == '/'))
	{
		return false;
	}
	return true;
#else // WIN32
	if (path[0] == '/')
	{
		return false;
	}
	return true;
#endif // WIN32
}

char* findRelativePath(const char* cwd, const char* path)
{
	size_t lastSlash = 0;
	size_t cwdLen = strlen(cwd);
	size_t pathLen = strlen(path);
	char *fixedCwd;
	char *fixedPath;
	char **cwdComponents;
	int cwdCount;
	int dotDotCount;
	char *retValue;
	const char *diffSection;
	size_t i;

	if (isRelativePath(cwd))
	{
		// cwd must be a full path.
		return NULL;
	}
	if (isRelativePath(path))
	{
#ifdef WIN32
		if (path[0] == '/' || path[0] == '\\')
		{
			// In Windows, it's not an absolute path if it doesn't start with a
			// drive letter followed by a colon.  However, it's not truly a
			// relative path either, since you can't tag it on to cwd and have
			// it work.
			return NULL;
		}
#endif // WIN32
		// If path is already relative, it should be relative to cwd; return it.
		return copyString(path);
	}
#ifdef WIN32
	if (strncasecmp(cwd, path, 2) != 0)
	{
		// The two paths are on different drives; relative path is impossible.
		return NULL;
	}
#endif // WIN32
	fixedCwd = copyString(cwd);
	fixedPath = copyString(path);
#ifdef WIN32
	replaceStringCharacter(fixedCwd, '\\', '/');
	convertStringToLower(fixedCwd);
	replaceStringCharacter(fixedPath, '\\', '/');
	convertStringToLower(fixedPath);
#endif // WIN32
	for (i = 0; i < cwdLen && i < pathLen; i++)
	{
		if (fixedCwd[i] == '/' && fixedPath[i] == '/')
		{
			lastSlash = i;
		}
		else if (fixedCwd[i] != fixedPath[i])
		{
			break;
		}
	}
	cwdComponents = componentsSeparatedByString(&fixedCwd[lastSlash], "/",
		cwdCount);
	dotDotCount = cwdCount - 2;	// There's a / at the beginning and end.
	diffSection = &path[lastSlash + 1];
	retValue = new char[dotDotCount * 3 + strlen(diffSection) + 1];
	for (i = 0; i < (size_t)dotDotCount; i++)
	{
		strcpy(&retValue[i * 3], "../");
	}
	strcpy(&retValue[i * 3], diffSection);
	deleteStringArray(cwdComponents, cwdCount);
	delete[] fixedCwd;
	delete[] fixedPath;
	return retValue;
}

char* directoryFromPath(const char* path)
{
	if (path)
	{
		const char* slashSpot = strrchr(path, '/');
#ifdef WIN32
		const char* backslashSpot = strrchr(path, '\\');

		if (backslashSpot > slashSpot)
		{
			slashSpot = backslashSpot;
		}
#endif // WIN32
		if (slashSpot)
		{
			size_t length = slashSpot - path;
			char* directory = new char[length + 1];

			strncpy(directory, path, length);
			directory[length] = 0;
			return directory;
		}
		else
		{
			return copyString("");
		}
	}
	else
	{
		return NULL;
	}
}

TCExport void combinePath(
	const char *left,
	const char *right,
	std::string &combinedPath)
{
	std::string leftStr;
	std::string rightStr;
	
	if (left != NULL)
	{
		leftStr = left;
	}
	if (right != NULL)
	{
		rightStr = right;
	}
	combinePath(leftStr, rightStr, combinedPath);
}

void combinePath(
	const std::string &left,
	const std::string &right,
	std::string &combinedPath)
{
	if (left.length() > 0)
	{
		combinedPath = left + "/";
		combinedPath += right;
		char *tempPath = cleanedUpPath(combinedPath.c_str());
		combinedPath = tempPath;
		delete[] tempPath;
	}
	else
	{
		combinedPath = right;
	}
}

void removeExtenstion(std::string& path)
{
	size_t slashSpot = lastSlashIndex(path);
	size_t dotSpot = path.rfind('.');
	if (dotSpot < path.size())
	{
		if (slashSpot < path.size() && slashSpot > dotSpot)
		{
			// The last dot comes before the last slash, so don't strip.
			return;
		}
		path.resize(dotSpot);
	}
}

char* cleanedUpPath(const char* path)
{
	char *newPath = copyString(path);

#ifdef WIN32
	replaceStringCharacter(newPath, '\\', '/');
#endif // WIN32
	if (strstr(newPath, "../"))
	{
		char **pathComponents;
		int pathCount;
		//int newCount;
		std::stack<std::string> pathStack;
		std::list<std::string> pathList;
		std::list<std::string>::const_iterator it;
		size_t len = 1;	// The terminating NULL.
		size_t offset = 0;

		pathComponents = componentsSeparatedByString(newPath, "/", pathCount);
		// Note that we're intentionally skipping the first component.  That's
		// either empty (for a Unix path), or the drive letter followed by a
		// colon (for a Windows path).  We'll put it back later, though.
		for (int i = 1; i < pathCount; i++)
		{
			if (strcmp(pathComponents[i], "..") == 0)
			{
				if (pathStack.size() > 0)
				{
					pathStack.pop();
				}
			}
			else
			{
				pathStack.push(pathComponents[i]);
			}
		}
		while (pathStack.size() > 0)
		{
			pathList.push_front(pathStack.top());
			pathStack.pop();
			len += pathList.front().size() + 1;
		}
		len += strlen(pathComponents[0]);
		delete[] newPath;
		newPath = new char[len];
		strcpy(newPath, pathComponents[0]);
		offset = strlen(newPath);
		for (it = pathList.begin(); it != pathList.end(); ++it)
		{
			// The following line leaves the string without a NULL terminator,
			// but the line after that puts one back.
			newPath[offset++] = '/';
			strcpy(&newPath[offset], it->c_str());
			offset += it->size();
		}
		deleteStringArray(pathComponents, pathCount);
	}
	if (strstr(newPath, "//") != NULL)
	{
		char *tempPath = stringByReplacingSubstring(newPath, "//", "/");
		delete[] newPath;
		newPath = tempPath;
	}
#ifdef WIN32
	replaceStringCharacter(newPath, '/', '\\');
#endif // WIN32
	return newPath;
}

static size_t lastSlashIndex(const ucstring &path)
{
	size_t slashSpot = path.rfind('/');
#ifdef WIN32
	size_t backslashSpot = path.rfind('\\');
	
	if (slashSpot >= path.size() ||
		(backslashSpot < path.size() && backslashSpot > slashSpot))
	{
		slashSpot = backslashSpot;
	}
#endif // WIN32
	return slashSpot;
}

static size_t lastSlashIndex(const std::string &path)
{
	size_t slashSpot = path.rfind('/');
#ifdef WIN32
	size_t backslashSpot = path.rfind('\\');

	if (slashSpot >= path.size() ||
		(backslashSpot < path.size() && backslashSpot > slashSpot))
	{
		slashSpot = backslashSpot;
	}
#endif // WIN32
	return slashSpot;
}

TCExport ucstring filenameFromPath(const ucstring &path)
{
	size_t slashSpot = lastSlashIndex(path);
	if (slashSpot < path.size())
	{
		return path.substr(slashSpot + 1);
	}
	else
	{
		return path;
	}
}

TCExport ucstring directoryFromPath(const ucstring &path)
{
	size_t slashSpot = lastSlashIndex(path);
	if (slashSpot < path.size())
	{
		return path.substr(0, slashSpot);
	}
	else
	{
		return _UC("");
	}
}

char* filenameFromPath(const char* path)
{
	if (path)
	{
		const char* slashSpot = strrchr(path, '/');
#ifdef WIN32
		const char* backslashSpot = strrchr(path, '\\');

		if (backslashSpot > slashSpot)
		{
			slashSpot = backslashSpot;
		}
#endif // WIN32
		if (slashSpot)
		{
			slashSpot++;
			return copyString(slashSpot);
		}
		else
		{
			return copyString(path);
		}
	}
	else
	{
		return NULL;
	}
}

void stripCRLF(char* line)
{
	if (line)
	{
		size_t length = strlen(line);

		while (length > 0 && (line[length-1] == '\r' || line[length-1] == '\n'))
		{
			line[--length] = 0;
		}
	}
}

void stripCRLF(wchar_t* line)
{
	if (line)
	{
		size_t length = wcslen(line);

		while (length > 0 && (line[length-1] == '\r' || line[length-1] == '\n'))
		{
			line[--length] = 0;
		}
	}
}

void stripLeadingWhitespace(char* string)
{
	char *spot;

	for (spot = string; spot[0] == ' ' || spot[0] == '\t'; spot++)
		;
	if (spot[0])
	{
		if (spot != string)
		{
			memmove(string, spot, strlen(spot) + 1);
		}
	}
	else
	{
		string[0] = 0;
	}
}

void stripLeadingWhitespace(wchar_t* string)
{
	wchar_t *spot;

	for (spot = string; spot[0] == ' ' || spot[0] == '\t'; spot++)
		;
	if (spot[0])
	{
		if (spot != string)
		{
			memmove(string, spot, (wcslen(spot) + 1) * sizeof(wchar_t));
		}
	}
	else
	{
		string[0] = 0;
	}
}

void stripTrailingWhitespace(char* string)
{
	if (string)
	{
		size_t length = strlen(string);

		while (length > 0 && (string[length - 1] == ' ' ||
			string[length - 1] == '\t'))
		{
			string[--length] = 0;
		}
	}
}

void stripTrailingWhitespace(wchar_t* string)
{
	if (string)
	{
		size_t length = wcslen(string);

		while (length > 0 && (string[length - 1] == ' ' ||
			string[length - 1] == '\t'))
		{
			string[--length] = 0;
		}
	}
}

void stripTrailingPathSeparators(char* path)
{
	if (path)
	{
		size_t length = strlen(path);

#ifdef WIN32
		while (length > 0 && (path[length-1] == '\\' || path[length-1] == '/'))
#else
		while (length > 0 && path[length-1] == '/')
#endif
		{
			path[--length] = 0;
		}
	}
}

void stripTrailingPathSeparators(wchar_t* path)
{
	if (path)
	{
		size_t length = wcslen(path);

#ifdef WIN32
		while (length > 0 && (path[length - 1] == L'\\' || path[length - 1] == L'/'))
#else
		while (length > 0 && path[length - 1] == '/')
#endif
		{
			path[--length] = 0;
		}
	}
}

void replaceStringCharacter(
	char* string,
	char oldChar,
	char newChar,
	int repeat)
{
	char* spot;

	if ((spot = strchr(string, oldChar)) != NULL)
	{
		*spot = newChar;
		if (repeat)
		{
			while ((spot = strchr(spot, oldChar)) != NULL)
			{
				*spot = newChar;
			}
		}
	}
}

void replaceStringCharacter(
	wchar_t* string,
	wchar_t oldChar,
	wchar_t newChar,
	int repeat)
{
	UCSTR spot;

	if ((spot = ucstrchr(string, oldChar)) != NULL)
	{
		*spot = newChar;
		if (repeat)
		{
			while ((spot = ucstrchr(spot, oldChar)) != NULL)
			{
				*spot = newChar;
			}
		}
	}
}

char *stringByReplacingSubstring(const char* string, const char* oldSubstring,
				 const char* newSubstring, bool repeat)
{
	char *newString = NULL;

	if (repeat)
	{
		int count;
		char **components = componentsSeparatedByString(string, oldSubstring,
			count);

		newString = componentsJoinedByString(components, count,
			newSubstring);
		deleteStringArray(components, count);
	}
	else
	{
		const char *oldSpot = strstr(string, oldSubstring);

		if (oldSpot)
		{
			size_t oldSubLength = strlen(oldSubstring);
			size_t newSubLength = strlen(newSubstring);
			size_t preLength = oldSpot - string;

			newString = new char[strlen(string) + newSubLength - oldSubLength +
				1];
			strncpy(newString, string, preLength);
			strcpy(newString + preLength, newSubstring);
			strcpy(newString + preLength + newSubLength,
				string + preLength + oldSubLength);
		}
		else
		{
			newString = new char[strlen(string) + 1];
			strcpy(newString, string);
		}
	}
	return newString;
}

int countStringLines(const char* string)
{
	size_t len = strlen(string);
	int count = 0;
	char *spot = (char*)string;

	if (len > 0 && string[len - 1] != '\n')
	{
		count++;
	}
	while ((spot = strchr(spot, '\n')) != NULL)
	{
		count++;
		spot++;
	}
	return count;
}

void setDebugLevel(int value)
{
	debugLevel = value;
}

int getDebugLevel(void)
{
	return debugLevel;
}

void indentPrintf(int indent, const char *format, ...)
{
	int i;
	va_list argPtr;

	for (i = 0; i < indent; i++)
	{
		printf(" ");
	}
	va_start(argPtr, format);
	vprintf(format, argPtr);
	va_end(argPtr);
}

void consoleVPrintf(const char *format, va_list argPtr)
{
#ifdef WIN32
	if (g_haveConsole)
	{
#ifdef TC_NO_UNICODE
		vprintf(format, argPtr);
		fflush(stdout);
#else // TC_NO_UNICODE
		static std::string temp;
		std::wstring wtemp;
		DWORD bytesWritten;

#if _MSC_VER < 1400	// VC < VC 2005
		int size;
		temp.resize(65536);
		size = vsprintf(&temp[0], format, argPtr);
		temp.resize(size);
#else
		temp.resize(_vscprintf(format, argPtr));
		vsprintf(&temp[0], format, argPtr);
#endif
		utf8towstring(wtemp, temp);
		if (g_bRealConsole)
		{
			// g_bRealConsole means we're running from an actual console app,
			// not a child process of the launcher app.
			WriteConsoleW(g_hStdOut, wtemp.c_str(), (DWORD)wtemp.size(),
				&bytesWritten, NULL);
		}
		else
		{
			WriteFile(g_hStdOut, wtemp.c_str(), (DWORD)wtemp.size() * 2,
				&bytesWritten, NULL);
			FlushFileBuffers(g_hStdOut);
		}
#endif // TC_NO_UNICODE
	}
	else
	{
		g_consoleBuffer.vprintf(format, argPtr);
	}
#else // WIN32
	vprintf(format, argPtr);
#endif
}

void consoleVPrintf(const wchar_t *format, va_list argPtr)
{
#ifdef WIN32
	if (g_haveConsole)
	{
#ifdef TC_NO_UNICODE
		vwprintf(format, argPtr);
		fflush(stdout);
#else // TC_NO_UNICODE
		std::wstring wtemp;
		DWORD bytesWritten;

#if _MSC_VER < 1400	// VC < VC 2005
		int size;
		wtemp.resize(65536);
		size = vswprintf(&wtemp[0], format, argPtr);
		wtemp.resize(size);
#else
		wtemp.resize(_vscwprintf(format, argPtr));
		// Note: second arg is size of buffer.  Buffer in std::string has one
		// extra character for the terminating NULL.
		vswprintf(&wtemp[0], wtemp.size() + 1, format, argPtr);
#endif
		if (g_bRealConsole)
		{
			// g_bRealConsole means we're running from an actual console app,
			// not a child process of the launcher app.
			WriteConsoleW(g_hStdOut, wtemp.c_str(), (DWORD)wtemp.size(),
				&bytesWritten, NULL);
		}
		else
		{
			WriteFile(g_hStdOut, wtemp.c_str(), (DWORD)wtemp.size() * 2,
				&bytesWritten, NULL);
			FlushFileBuffers(g_hStdOut);
		}
#endif // TC_NO_UNICODE
	}
	else
	{
		g_consoleBuffer.vwprintf(format, argPtr);
	}
#else // WIN32
#ifdef NO_WSTRING
	printf("wprintf attempted.\n");
#else // NO_WSTRING
	vwprintf(format, argPtr);
#endif // NO_WSTRING
#endif
}

void consolePrintf(const char *format, ...)
{
	va_list argPtr;

	va_start(argPtr, format);
	consoleVPrintf(format, argPtr);
	va_end(argPtr);
}

void consolePrintf(const wchar_t *format, ...)
{
	va_list argPtr;

	va_start(argPtr, format);
	consoleVPrintf(format, argPtr);
	va_end(argPtr);
}

#if defined(WIN32) && !defined(_QT)
#pragma warning(disable : 4996) // suppress MS deprecated warning for vsnprintf
#endif

std::string formatString(const char *format, ...)
{
    va_list argPtr;

    va_start(argPtr, format);
    std::vector<char> buf;
    buf.resize(vsnprintf(NULL, 0, format, argPtr) + 1); // Extra space for '\0'
    va_end(argPtr);
    va_start(argPtr, format);
    vsnprintf(&buf[0], buf.size(), format, argPtr);
    va_end(argPtr);

    return &buf[0];
}

void debugVLog(const char *udKey, const char *format, va_list argPtr)
{
	char *logFilename = TCUserDefaults::pathForKey(udKey);

	if (logFilename != NULL)
	{
		FILE *logFile = ucfopen(logFilename, "a+");

		if (logFile != NULL)
		{
#ifdef WIN32
			struct tm *timeStruct;
			time_t timeV;
			char timeString[1024];

			time(&timeV);
			timeStruct = localtime(&timeV);
			strncpy(timeString, asctime(timeStruct), sizeof(timeString));
			timeString[sizeof(timeString) - 1] = 0;
			stripCRLF(timeString);
			fprintf(logFile, "%s: ", timeString);
#endif
			vfprintf(logFile, format, argPtr);
			fclose(logFile);
		}
		delete[] logFilename;
	}
}

void debugLog(const char *udKey, const char *format, ...)
{
	va_list argPtr;

	va_start(argPtr, format);
	debugVLog(udKey, format, argPtr);
	va_end(argPtr);
}

void debugLog1s(
	const char *udKey,
	const char *format,
	const char *value)
{
	if (value != NULL)
	{
		debugLog(udKey, format, value);
	}
	else
	{
		debugLog(udKey, format, "!!NULL!!");
	}
}

#ifdef WIN32
void debugVLog(const char *udKey, const wchar_t *format, va_list argPtr)
{
	char *logFilename = TCUserDefaults::pathForKey(udKey);

	if (logFilename != NULL)
	{
		FILE *logFile = ucfopen(logFilename, "a+b");

		if (logFile != NULL)
		{
			wchar_t buf[10240];
			struct tm *timeStruct;
			time_t timeV;
			char timeString[1024];

			time(&timeV);
			timeStruct = localtime(&timeV);
			strncpy(timeString, asctime(timeStruct), sizeof(timeString));
			timeString[sizeof(timeString) - 1] = 0;
			stripCRLF(timeString);
			swprintf(buf, 10240, L"%hs: ", timeString);
			fwrite(buf, wcslen(buf) * 2, 1, logFile);
			vswprintf(buf, 10240, format, argPtr);
			fwrite(buf, wcslen(buf) * 2, 1, logFile);
			fclose(logFile);
		}
		delete[] logFilename;
	}
}
#else
// Sorry: not sure how to deal with wide characters in OSX/Linux.
void debugVLog(const char *, const wchar_t *, va_list)
{
}
#endif

void debugLog(const char *udKey, const wchar_t *format, ...)
{
	va_list argPtr;

	va_start(argPtr, format);
	debugVLog(udKey, format, argPtr);
	va_end(argPtr);
}

void debugLog1s(
	const char *udKey,
	const wchar_t *format,
	const wchar_t *value)
{
	if (value != NULL)
	{
		debugLog(udKey, format, value);
	}
	else
	{
		debugLog(udKey, format, L"!!NULL!!");
	}
}

void debugVPrintf(int level, const char *format, va_list argPtr)
{
	if (debugLevel >= level)
	{
#ifdef WIN32
		if (g_haveConsole)
		{
			consoleVPrintf(format, argPtr);
			return;
		}
#endif // WIN32
		vprintf(format, argPtr);
		fflush(stdout);
	}
}

void debugPrintf(const char *format, ...)
{
	va_list argPtr;

	va_start(argPtr, format);
	debugVPrintf(1, format, argPtr);
	va_end(argPtr);
}

void debugPrintf(int level, const char *format, ...)
{
	va_list argPtr;

	va_start(argPtr, format);
	debugVPrintf(level, format, argPtr);
	va_end(argPtr);
}

#ifndef WIN32

char *prettyLongLongString(long long num)
{
	char backwards[256];
	char *forwards;
	int i, j;

	for (i = 0; num; i++)
	{
		if (!((i + 1) % 4))
		{
			backwards[i++] = ',';
		}
		backwards[i] = (num % 10) + '0';
		num /= 10;
	}
	if (i == 0)
	{
		backwards[0] = '0';
		i = 1;
	}
	backwards[i] = 0;
	forwards = new char[(i + 1) * sizeof(char)];
	for (j = 0, i--; i >= 0; i--, j++)
	{
		forwards[j] = backwards[i];
	}
	forwards[j] = 0;
	return forwards;
}

long long longLongFromString(char* string)
{
	long long val;
	char* tmpString = NULL;
	size_t length = strlen(string);
	char* spot;

	while ((spot = strchr(string, ',')) != NULL)
	{
		size_t diff = spot - string;
		int needsDelete = 0;

		if (tmpString)
		{
			needsDelete = 1;
		}
		tmpString = new char[length];
		memcpy(tmpString, string, diff);
		tmpString[diff] = 0;
		strcat(tmpString, string + diff + 1);
		if (needsDelete)
		{
			delete[] string;
		}
		string = tmpString;
	}
	sscanf(string, "%lld", &val);
	if (tmpString)
	{
		delete[] tmpString;
	}
	return val;
}

#endif // WIN32

static const char *getEscapeString(char ch)
{
	switch (ch)
	{
	case '\a':
		return "\\a";
		break;
	case '\b':
		return "\\b";
		break;
	case '\f':
		return "\\f";
		break;
	case '\n':
		return "\\n";
		break;
	case '\r':
		return "\\r";
		break;
	case '\t':
		return "\\t";
		break;
	case '\v':
		return "\\v";
		break;
	case '\?':
		return "\\?";
		break;
	case '\\':
		return "\\\\";
		break;
	}
	return NULL;
}

static int escapeReplacement(char ch)
{
	switch (ch)
	{
	case 'a':
		return '\a';
		break;
	case 'b':
		return '\b';
		break;
	case 'f':
		return '\f';
		break;
	case 'n':
		return '\n';
		break;
	case 'r':
		return '\r';
		break;
	case 't':
		return '\t';
		break;
	case 'v':
		return '\v';
		break;
	case '?':
		return '\?';
		break;
	case '\'':
		return '\'';
		break;
	case '"':
		return '"';
		break;
	case '\\':
		return '\\';
		break;
	case '0':
		return '\0';
		break;
	}
	return -1;
}

static int escapeReplacement(wchar_t wch)
{
	return escapeReplacement((char)wch);
}

char *createEscapedString(const char *string)
{
	size_t i;
	size_t len = strlen(string);
	size_t tmpLen = 0;
	bool bFound = false;

	for (i = 0; i < len; i++)
	{
		const char *escapeString = getEscapeString(string[i]);

		if (escapeString)
		{
			tmpLen += strlen(escapeString);
			bFound = true;
		}
		else
		{
			tmpLen += 1;
		}
	}
	if (bFound)
	{
		char *retValue = new char[tmpLen + 1];

		tmpLen = 0;
		for (i = 0; i < len; i++)
		{
			const char *escapeString = getEscapeString(string[i]);

			if (escapeString)
			{
				strcpy(&retValue[tmpLen], escapeString);
				tmpLen += strlen(escapeString);
			}
			else
			{
				retValue[tmpLen] = string[i];
				tmpLen += 1;
			}
		}
		retValue[tmpLen] = 0;
		return retValue;
	}
	else
	{
		return copyString(string);
	}
}

void processEscapedString(char *string)
{
	size_t len = strlen(string);

	if (len > 0)
	{
		size_t i;
		size_t tmpLen = 0;
		char *tmpString = new char[len + 1];
		size_t lastSpot = 0;

		// Note we skip the last character, because even if it's a backslash, we
		// can't do anything with it.
		for (i = 0; i < len - 1; i++)
		{
			if (string[i] == '\\')
			{
				int replacement = escapeReplacement(string[i + 1]);

				if (replacement != -1)
				{
					if (i > lastSpot)
					{
						size_t count = i - lastSpot;

						strncpy(&tmpString[tmpLen], &string[lastSpot], count);
						tmpLen += count;
						lastSpot += count;
					}
					lastSpot += 2;
					tmpString[tmpLen++] = (char)replacement;
					i++;
				}
			}
		}
		strcpy(&tmpString[tmpLen], &string[lastSpot]);
		strcpy(string, tmpString);
		delete[] tmpString;
	}
}

void processEscapedString(wchar_t *string)
{
	size_t len = wcslen(string);

	if (len > 0)
	{
		size_t i;
		size_t tmpLen = 0;
		wchar_t *tmpString = new wchar_t[len + 1];
		size_t lastSpot = 0;

		// Note we skip the last character, because even if it's a backslash, we
		// can't do anything with it.
		for (i = 0; i < len - 1; i++)
		{
			if (string[i] == '\\')
			{
				int replacement = escapeReplacement(string[i + 1]);

				if (replacement != -1)
				{
					if (i > lastSpot)
					{
						size_t count = i - lastSpot;

						wcsncpy(&tmpString[tmpLen], &string[lastSpot], count);
						tmpLen += count;
						lastSpot += count;
					}
					lastSpot += 2;
					tmpString[tmpLen++] = (wchar_t)replacement;
					i++;
				}
			}
		}
		wcscpy(&tmpString[tmpLen], &string[lastSpot]);
		wcscpy(string, tmpString);
		delete[] tmpString;
	}
}

#ifdef TC_NO_UNICODE
void mbstoucstring(ucstring &dst, const char *src, int /*length*/)
#else
void mbstoucstring(ucstring &dst, const char *src, int length /*= -1*/)
#endif
{
#ifdef TC_NO_UNICODE
	dst = src;
#else
	mbstowstring(dst, src, length);
#endif
}

void mbstowstring(std::wstring &dst, const char *src, int length /*= -1*/)
{
	dst.erase();
	if (src)
	{
#ifndef NO_WSTRING
		mbstate_t state;// = { 0 };
		memset(&state, 0, sizeof(state));
#endif // !NO_WSTRING
		size_t newLength;

		if (length == -1)
		{
			length = (int)strlen(src);
		}
		if (length > 0)
		{
			dst.resize(length);
			// Even though we don't check, we can't pass NULL instead of &state
			// and still be thread-safe.
#ifdef NO_WSTRING
			newLength = mbstowcs(&dst[0], src, length + 1);
#else // NO_WSTRING
			newLength = mbsrtowcs(&dst[0], &src, length + 1, &state);
#endif // NO_WSTRING
			if (newLength == (size_t)-1)
			{
				dst.resize(wcslen(&dst[0]));
			}
			else
			{
				dst.resize(newLength);
			}
		}
	}
}

void stringtoucstring(ucstring &dst, const std::string &src)
{
#ifdef TC_NO_UNICODE
	dst = src;
#else // TC_NO_UNICODE
	stringtowstring(dst, src);
#endif // TC_NO_UNICODE
}

ucstring stringtoucstring(const std::string &src)
{
#ifdef TC_NO_UNICODE
	return src;
#else // TC_NO_UNICODE
	ucstring dst;
	stringtowstring(dst, src);
	return dst;
#endif // TC_NO_UNICODE
}

#ifdef TC_NO_UNICODE
UCSTR mbstoucstring(const char *src, int /*length*/ /*= -1*/)
#else // TC_NO_UNICODE
UCSTR mbstoucstring(const char *src, int length /*= -1*/)
#endif // TC_NO_UNICODE
{
	if (src)
	{
#ifdef TC_NO_UNICODE
		return copyString(src);
#else // TC_NO_UNICODE
		std::wstring dst;
		mbstowstring(dst, src, length);
		return copyString(dst.c_str());
#endif // TC_NO_UNICODE
	}
	else
	{
		return NULL;
	}
}

#ifdef TC_NO_UNICODE
char *ucstringtombs(CUCSTR src, int /*length*/ /*= -1*/)
#else // TC_NO_UNICODE
char *ucstringtombs(CUCSTR src, int length /*= -1*/)
#endif // TC_NO_UNICODE
{
	if (src)
	{
#ifdef TC_NO_UNICODE
		return copyString(src);
#else // TC_NO_UNICODE
		std::string dst;
		wcstostring(dst, src, length);
		return copyString(dst.c_str());
#endif // TC_NO_UNICODE
	}
	else
	{
		return NULL;
	}
}

#ifdef TC_NO_UNICODE
char *ucstringtoutf8(CUCSTR src, int /*length*/ /*= -1*/)
#else // TC_NO_UNICODE
char *ucstringtoutf8(CUCSTR src, int length /*= -1*/)
#endif // TC_NO_UNICODE
{
	if (src == NULL)
	{
		return NULL;
	}
#ifdef TC_NO_UNICODE
	// This isn't 100% accurate, but we don't have much choice.
	return copyString(src);
#else // TC_NO_UNICODE
	UTF8 *dst;
	UTF8 *dstDup;
	UTF16 *src16;
	const UTF16 *src16Dup;
	size_t utf8Length;
	char *retValue = NULL;

	if (length == -1)
	{
		length = (int)wcslen(src);
	}
	// I think every UTF-16 character can fit in 4 UTF-8 characters.
	// (Actually, hopefully it's less than that, but I'm trying to be safe.
	utf8Length = length * 4 + 1;
	dst = new UTF8[utf8Length];
	if (sizeof(wchar_t) == sizeof(UTF16))
	{
		src16 = (UTF16 *)src;
	}
	else
	{
		int i;

		src16 = new UTF16[length + 1];
		for (i = 0; i < length; i++)
		{
			src16[i] = (UTF16)src[i];
		}
		src16[length] = 0;
	}
	src16Dup = src16;
	dstDup = dst;
	// Note: length really is correct for end below, not length - 1.
	ConversionResult result = ConvertUTF16toUTF8(&src16Dup, &src16[length],
		&dstDup, &dst[utf8Length], lenientConversion);
	if (result == conversionOK)
	{
		utf8Length = dstDup - dst;
		retValue = new char[utf8Length + 1];
		memcpy(retValue, dst, utf8Length);
		retValue[utf8Length] = 0;
	}
	delete[] dst;
	if (src16 != (UTF16 *)src)
	{
		delete[] src16;
	}
	return retValue;
#endif // TC_NO_UNICODE
}

bool utf8towstring(std::wstring& dst, const std::string& src)
{
	return utf8towstring(dst, src.c_str(), (int)src.length());
}

bool utf8towstring(std::wstring& dst, const char *src, int length /*= -1*/)
{
	dst.clear();
	if (src == NULL)
	{
		return false;
	}
	if (length == 0)
	{
		return true;
	}
	const UTF8 *src8;
	const UTF8 *src8Dup;
	if (length == -1)
	{
		length = (int)strlen(src);
	}
	std::vector<UTF8> srcBuffer;
	// I'm going to assume that the wide string has no more characters than the
	// UTF-8 one.
	size_t dstLength = length + 1;
	if (sizeof(UTF8) == sizeof(char))
	{
		src8 = src8Dup = (UTF8 *)src;
	}
	else
	{
		srcBuffer.reserve(length + 1);
		for (size_t i = 0; i < length; ++i)
		{
			srcBuffer.push_back((UTF8)src[i]);
		}
		srcBuffer.push_back(0);
		src8 = src8Dup = &srcBuffer[0];
	}
	if (sizeof(wchar_t) == sizeof(UTF16))
	{
		UTF16 *dst16;
		UTF16 *dst16Dup;
		dst.resize(dstLength);
		dst16 = dst16Dup = (UTF16 *)&dst[0];
		// Note: length really is correct for end below, not length - 1.
		ConversionResult result = ConvertUTF8toUTF16(&src8Dup, &src8[length],
			&dst16Dup, &dst16[dstLength], lenientConversion);
		if (result == conversionOK)
		{
			dstLength = dst16Dup - dst16;
			dst.resize(dstLength);
			return true;
		}
	}
	else if (sizeof(wchar_t) == sizeof(UTF32))
	{
		UTF32 *dst32;
		UTF32 *dst32Dup;
		dst.resize(dstLength);
		dst32 = dst32Dup = (UTF32 *)&dst[0];
		// Note: length really is correct for end below, not length - 1.
		ConversionResult result = ConvertUTF8toUTF32(&src8Dup, &src8[length],
			&dst32Dup, &dst32[dstLength], lenientConversion);
		if (result == conversionOK)
		{
			dstLength = dst32Dup - dst32;
			dst.resize(dstLength);
			return true;
		}
	}
	dst.clear();
	return false;
}

bool wstringtoutf8(std::string& dst, const std::wstring& src)
{
	return wstringtoutf8(dst, src.c_str(), (int)src.length());
}

template <typename SrcChar>
bool wstringtoutf8Helper(std::string& dst, CUCSTR src, int length /*= -1*/)
{
	dst.clear();
	if (src == NULL)
	{
		return false;
	}
	if (length == 0)
	{
		return true;
	}
	const SrcChar *srcU;
	const SrcChar *srcUDup;
	if (length == -1)
	{
		length = (int)wcslen(src);
	}
	// Every Unicode character can fit in 4 UTF-8 characters.
	// (Actually, hopefully it's less than that, but I'm trying to be safe.)
	size_t dstLength = (length * 4) + 1;
	srcU = srcUDup = (const SrcChar *)src;
	UTF8 *dst8;
	UTF8 *dst8Dup;
	dst.resize(dstLength);
	dst8 = dst8Dup = (UTF8 *)&dst[0];
	ConversionResult result;
	if (sizeof(SrcChar) == sizeof(UTF16))
	{
		// Note: length really is correct for end below, not length - 1.
		result = ConvertUTF16toUTF8((const UTF16 **)&srcUDup, (UTF16 *)&srcU[length],
			&dst8Dup, &dst8[dstLength], lenientConversion);
	}
	else if (sizeof(SrcChar) == sizeof(UTF32))
	{
		// Note: length really is correct for end below, not length - 1.
		result = ConvertUTF32toUTF8((const UTF32 **)&srcUDup, (UTF32 *)&srcU[length],
			&dst8Dup, &dst8[dstLength], lenientConversion);
	}
	if (result == conversionOK)
	{
		dstLength = dst8Dup - dst8;
		dst.resize(dstLength);
		return true;
	}
	dst.clear();
	return false;
}

bool wstringtoutf8(std::string& dst, const wchar_t* src, int length /*= -1*/)
{
	if (sizeof(wchar_t) == sizeof(UTF16))
	{
		return wstringtoutf8Helper<UTF16>(dst, src, length);
	}
	else if (sizeof(wchar_t) == sizeof(UTF32))
	{
		return wstringtoutf8Helper<UTF32>(dst, src, length);
	}
	else
	{
		return false;
	}
}

#ifdef TC_NO_UNICODE
bool ucstringtoutf8(std::string& dst, CUCSTR src, int /*length = -1*/)
{
	dst = src;
	return true;
}
#else
bool ucstringtoutf8(std::string& dst, CUCSTR src, int length /*= -1*/)
{
	return wstringtoutf8(dst, src, length);
}
#endif

TCExport bool ucstringtoutf8(std::string& dst, const ucstring& src)
{
#ifdef TC_NO_UNICODE
	dst = src;
	return true;
#else
	return wstringtoutf8(dst, src);
#endif
}

TCExport bool utf8toucstring(ucstring& dst, const std::string &src)
{
#ifdef TC_NO_UNICODE
	dst = src;
	return true;
#else // TC_NO_UNICODE
	return utf8towstring(dst, src);
#endif // TC_NO_UNICODE
}

#ifdef TC_NO_UNICODE
TCExport bool utf8toucstring(ucstring& dst, const char *src, int /*length = -1*/)
{
	dst = src;
	return true;
}
#else // TC_NO_UNICODE
TCExport bool utf8toucstring(ucstring& dst, const char *src, int length /*= -1*/)
{
	return utf8towstring(dst, src, length);
}
#endif // TC_NO_UNICODE

#ifdef TC_NO_UNICODE
UCSTR utf8toucstring(const char *src, int /*length*/ /*= -1*/)
#else // TC_NO_UNICODE
UCSTR utf8toucstring(const char *src, int length /*= -1*/)
#endif // TC_NO_UNICODE
{
	if (src == NULL)
	{
		return NULL;
	}
#ifdef TC_NO_UNICODE
	// This isn't 100% accurate, but we don't have much choice.
	return copyString(src);
#else // TC_NO_UNICODE
	UTF16 *dst;
	UTF16 *dstDup;
	UTF8 *src8;
	const UTF8 *src8Dup;
	size_t utf16Length;
	wchar_t *retValue = NULL;

	if (length == -1)
	{
		length = (int)strlen(src);
	}
	// I'm going to assume that the UTF-16 string has no more characters
	// than the UTF-8 one.
	utf16Length = length + 1;
	dst = new UTF16[utf16Length];
	if (sizeof(char) == sizeof(UTF8))
	{
		src8 = (UTF8 *)src;
	}
	else
	{
		int i;

		src8 = new UTF8[length + 1];
		for (i = 0; i < length; i++)
		{
			src8[i] = (UTF8)src[i];
		}
		src8[length] = 0;
	}
	src8Dup = src8;
	dstDup = dst;
	// Note: length really is correct for end below, not length - 1.
	ConversionResult result = ConvertUTF8toUTF16(&src8Dup, &src8[length],
		&dstDup, &dst[utf16Length], lenientConversion);
	if (result == conversionOK)
	{
		utf16Length = dstDup - dst;
		retValue = new wchar_t[utf16Length + 1];
		if (sizeof(wchar_t) == sizeof(UTF16))
		{
			memcpy(retValue, dst, utf16Length * sizeof(wchar_t));
		}
		else
		{
			size_t i;

			for (i = 0; i < utf16Length; i++)
			{
				retValue[i] = dst[i];
			}
		}
		retValue[utf16Length] = 0;
	}
	delete[] dst;
	if (src8 != (UTF8 *)src)
	{
		delete[] src8;
	}
	return retValue;
#endif // TC_NO_UNICODE
}

void wcstostring(std::string &dst, const wchar_t *src, int length /*= -1*/)
{
	dst.erase();
	if (src)
	{
#ifndef NO_WSTRING
		mbstate_t state;// = { 0 };
		memset(&state, 0, sizeof(state));
#endif // !NO_WSTRING
		size_t newLength;

		if (length == -1)
		{
			length = (int)wcslen(src);
		}
		length *= 2;
		dst.resize(length);
		// Even though we don't check, we can't pass NULL instead of &state and
		// still be thread-safe.
#ifdef NO_WSTRING
		newLength = wcstombs(&dst[0], src, length);
#else // NO_WSTRING
		newLength = wcsrtombs(&dst[0], &src, length, &state);
#endif // NO_WSTRING
		if (newLength == (size_t)-1)
		{
			dst.resize(strlen(&dst[0]));
		}
		else
		{
			dst.resize(newLength);
		}
	}
}

void wstringtostring(std::string &dst, const std::wstring &src)
{
#ifndef NO_WSTRING
	wcstostring(dst, src.c_str(), (int)src.length());
#endif // NO_WSTRING
}

void stringtowstring(std::wstring &dst, const std::string &src)
{
	mbstowstring(dst, src.c_str(), (int)src.length());
}

#ifdef WIN32

void runningWithConsole(bool bRealConsole /*= false*/)
{
	g_haveConsole = true;
	g_bRealConsole = bRealConsole;
	g_hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
}

bool haveConsole(void)
{
	return g_haveConsole;
}

#endif // WIN32

#ifdef NO_WSTRING

unsigned long wcstoul(const wchar_t *start, wchar_t **end, int base)
{
	std::string temp;
	char *tempEnd;
	wcstostring(temp, start);
	unsigned long retValue = strtoul(temp.c_str(), &tempEnd, base);
	if (end != NULL)
	{
		*end = const_cast<wchar_t *>(&start[tempEnd - temp.c_str()]);
	}
	return retValue;
}

#endif // NO_WSTRING

ucstring ftoucstr(double value, int precision /*= 6*/)
{
#ifdef TC_NO_UNICODE
	return ftostr(value, precision);
#else // TC_NO_UNICODE
	std::string string = ftostr(value, precision);
	std::wstring wstring;

	stringtowstring(wstring, string);
	return wstring;
#endif // TC_NO_UNICODE
}

std::string ftostr(double value, int precision /*= 6*/)
{
	char buf[128];
	char format[128];
	size_t len;

	sprintf(format, "%%.%df", precision);
	sprintf(buf, format, value);
	len = strlen(buf);
	while (len > 0 && buf[len - 1] == '0')
	{
		buf[--len] = 0;
	}
	if (len > 0 && buf[len - 1] == '.')
	{
		buf[--len] = 0;
	}
	if (strcmp(buf, "-0") == 0)
	{
		strcpy(buf, "0");
	}
	return buf;
}

ucstring ltoucstr(long value)
{
#ifdef TC_NO_UNICODE
	return ltostr(value);
#else // TC_NO_UNICODE
	std::string string = ltostr(value);
	std::wstring wstring;

	stringtowstring(wstring, string);
	return wstring;
#endif // TC_NO_UNICODE
}

std::string ltostr(long value)
{
	char buf[32];

	sprintf(buf, "%ld", value);
	return buf;
}

bool getCurrentDirectory(std::string &dir)
{
#ifdef WIN32
#ifdef TC_NO_UNICODE
	dir.resize(2048);
	DWORD len = GetCurrentDirectory((DWORD)dir.length(), &dir[0]);
	dir.resize(len);
#else // TC_NO_UNICODE
	std::wstring wDir;
	wDir.resize(2048);
	DWORD len = GetCurrentDirectoryW((DWORD)wDir.length(), &wDir[0]);
	wDir.resize(len);
	ucstringtoutf8(dir, wDir);
#endif // !TC_NO_UNICODE
	return len > 0;
#else // WIN32
	char *temp = getcwd(NULL, 0);
	if (temp == NULL)
	{
		dir.clear();
		return false;
	}
	else
	{
		dir = temp;
		free(temp);
		return true;
	}
#endif // !WIN32
}

bool setCurrentDirectory(const std::string &dir)
{
#ifdef WIN32
#ifdef TC_NO_UNICODE
	if (!SetCurrentDirectory(dir.c_str()))
	{
		return false;
	}
#else // TC_NO_UNICODE
	std::wstring wDir;
	utf8towstring(wDir, dir);
	if (!SetCurrentDirectoryW(wDir.c_str()))
	{
		return false;
	}
#endif // !TC_NO_UNICODE
#else // WIN32
	if (chdir(dir.c_str()) == -1)
	{
		return false;
	}
#endif // !WIN32
	return true;
}

bool createDirectory(const std::string &dir)
{
#ifdef WIN32
#ifdef TC_NO_UNICODE
	if (!CreateDirectory(dir.c_str(), NULL))
	{
		return false;
	}
#else // TC_NO_UNICODE
	std::wstring wDir;
	utf8towstring(wDir, dir);
	if (!CreateDirectoryW(wDir.c_str(), NULL))
	{
		return false;
	}
#endif // !TC_NO_UNICODE
#else // WIN32
	if (mkdir(dir.c_str(), 0777) == -1)
	{
		return false;
	}
#endif // !WIN32
	return true;
}

TCExport bool ensurePath(const std::string &path)
{
	std::string origDir;

	if (!getCurrentDirectory(origDir))
	{
		return false;
	}
	if (setCurrentDirectory(path))
	{
		setCurrentDirectory(origDir);
		return true;
	}
	int count;
	char *tempPath = copyString(path.c_str());
	replaceStringCharacter(tempPath, '\\', '/');
	char **components = componentsSeparatedByString(tempPath, "/", count);
	delete[] tempPath;
	bool retValue = false;
	if (count > 0)
	{
		int i = 0;
		retValue = true;
		
		if (!isRelativePath(path.c_str()))
		{
#ifdef WIN32
			std::string drive(components[0]);
			drive += '\\';
			retValue = setCurrentDirectory(drive);
#else // WIN32
			retValue = setCurrentDirectory("/");
#endif // !WIN32
			i = 1;
		}
		if (retValue)
		{
			// Note: it's impossible to create a new drive under Windows, and
			// if / doesn't work elsewhere then things are screwed.
			for (; i < count && retValue; i++)
			{
				if (components[i][0] != 0 &&
					!setCurrentDirectory(components[i]))
				{
					retValue = createDirectory(components[i]);
				}
			}
		}
	}
	deleteStringArray(components, count);
	setCurrentDirectory(origDir);
	return retValue;
}

// Adapted from Public Domain base64Decode from:
// https://en.wikibooks.org/wiki/Algorithm_Implementation/Miscellaneous/Base64
const static char base64Charset[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";
static std::set<char> base64CharsetSet;
const static char padCharacter = '=';

bool base64Decode(const std::string& input, std::vector<TCByte>& decodedBytes)
{
	decodedBytes.clear();
	if (input.size() % 4) //Sanity check
	{
		return false;
	}
	size_t padding = 0;
	if (input.length())
	{
		if (input[input.length()-1] == padCharacter)
		{
			padding++;
		}
		if (input[input.length()-2] == padCharacter)
		{
			padding++;
		}
	}
	//Setup a vector to hold the result
	decodedBytes.reserve(((input.length()/4)*3) - padding);
	TCULong temp=0; //Holds decoded quanta
	std::string::const_iterator cursor = input.begin();
	while (cursor < input.end())
	{
		for (size_t quantumPosition = 0; quantumPosition < 4; ++quantumPosition)
		{
			temp <<= 6;
			if (*cursor >= 0x41 && *cursor <= 0x5A) // This area will need tweaking if
			{		                                // you are using an alternate alphabet
				temp |= *cursor - 0x41;
			}
			else if (*cursor >= 0x61 && *cursor <= 0x7A)
			{
				temp |= *cursor - 0x47;
			}
			else if (*cursor >= 0x30 && *cursor <= 0x39)
			{
				temp |= *cursor + 0x04;
			}
			else if (*cursor == 0x2B)
			{
				temp |= 0x3E; //change to 0x2D for URL alphabet
			}
			else if (*cursor == 0x2F)
			{
				temp |= 0x3F; //change to 0x5F for URL alphabet
			}
			else if (*cursor == padCharacter) //pad
			{
				switch (input.end() - cursor)
				{
					case 1: //One pad character
						decodedBytes.push_back((temp >> 16) & 0x000000FF);
						decodedBytes.push_back((temp >> 8 ) & 0x000000FF);
						return true;
					case 2: //Two pad characters
						decodedBytes.push_back((temp >> 10) & 0x000000FF);
						return true;
					default:
						decodedBytes.clear();
						return false;
				}
			}
			else
			{
				decodedBytes.clear();
				return false;
			}
			cursor++;
		}
		decodedBytes.push_back((temp >> 16) & 0x000000FF);
		decodedBytes.push_back((temp >> 8 ) & 0x000000FF);
		decodedBytes.push_back((temp      ) & 0x000000FF);
	}
	return true;
}

bool isInBase64Charset(char character)
{
	if (base64CharsetSet.empty())
	{
		for (const char *charsetChar = base64Charset; *charsetChar; ++charsetChar)
		{
			base64CharsetSet.insert(*charsetChar);
		}
	}
	return base64CharsetSet.find(character) != base64CharsetSet.end();
}

FILE *ucfopen(const char *filename, const char *mode)
{
#ifdef _MSC_VER
	std::wstring wFilename;
	std::wstring wMode;

	if (utf8towstring(wFilename, filename) && utf8towstring(wMode, mode))
	{
		return _wfopen(wFilename.c_str(), wMode.c_str());
	}
#endif // !_MSC_VER
	return fopen(filename, mode);
}

bool skipUtf8BomIfPresent(std::istream &stream)
{
	std::streampos origPos = stream.tellg();
	unsigned char bomBuf[3] = { 0, 0, 0 };
	bool hasBom = false;
	stream.read((char *)bomBuf, 3);
	if (stream)
	{
		hasBom = bomBuf[0] == 0xEF && bomBuf[1] == 0xBB &&
		bomBuf[2] == 0xBF;
	}
	if (!hasBom)
	{
		stream.seekg(origPos);
	}
	return hasBom;
}

bool isLittleEndian()
{
	short int number = 0x1;
	char *numPtr = (char*)&number;
	return (numPtr[0] == 1);
}
