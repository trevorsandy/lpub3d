#if defined(WIN32) && !defined(_QT)
#if _MSC_VER < 1400	// VC < VC 2005
#pragma warning(disable: 4503) // Decorated name truncated
#endif // VC < VC 2005
#endif // WIN32 && NOT _QT
#include "TCUserDefaults.h"
#include "TCStringArray.h"
#include "TCAlert.h"
#include "TCAlertManager.h"
#include "mystring.h"

#ifdef COCOA
#import <Foundation/Foundation.h>
#endif // COCOA
#include <stdio.h>

#define MAX_INI_LINE_LEN 65536

#ifdef WIN32
#include <direct.h>
#if defined(_MSC_VER) && _MSC_VER >= 1400 && defined(_DEBUG)
#define new DEBUG_CLIENTBLOCK
#endif
#else // WIN32
#include <unistd.h>
#endif // WIN32

#ifdef _QT
#include "../LDVQt/misc.h"
#include <QtCore/qstringlist.h>
#endif // _QT

#define APP_PATH_PREFIX "::AppDir::"

TCUserDefaults* TCUserDefaults::currentUserDefaults = NULL;

TCUserDefaults::TCUserDefaultsCleanup TCUserDefaults::userDefaultsCleanup;
char *TCUserDefaults::argv0 = NULL;
std::string TCUserDefaults::appPath;

TCUserDefaults::TCUserDefaultsCleanup::~TCUserDefaultsCleanup(void)
{
	delete[] argv0;
	argv0 = NULL;
	if (currentUserDefaults != NULL)
	{
		currentUserDefaults->release();
		currentUserDefaults = NULL;
	}
}

void TCUserDefaults::TCUserDefaultsFlusher::dealloc(void)
{
	if (currentUserDefaults != NULL)
	{
		TCUserDefaults::flush();
		getCurrentUserDefaults()->flushRequested = false;
	}
	TCObject::dealloc();
}

TCUserDefaults::TCUserDefaults(void)
	:appName(NULL),
	sessionName(NULL),
	commandLine(NULL),
	useIni(false),
	flushRequested(false)
#if defined(WIN32) && !defined(_QT)
	,hAppDefaultsKey(NULL),
	hSessionKey(NULL)
#endif // WIN32 && NOT _QT
#ifdef TCUD_INI_SUPPORT
	,iniChanges(false)
#endif // TCUD_INI_SUPPORT
#ifdef COCOA
	,sessionDict(nil)
#endif // COCOA
{
#ifdef _LEAK_DEBUG
	strcpy(className, "TCUserDefaults");
#endif
#ifdef _QT
	//qSettings = new QSettings("LDView","LDView");
	qSettings = new QSettings();
#endif // _QT
#ifdef COCOA
	appName = copyString([[[NSBundle mainBundle] bundleIdentifier]
		cStringUsingEncoding: NSASCIIStringEncoding]);
#endif // COCOA
}

void TCUserDefaults::dealloc(void)
{
#ifdef TCUD_INI_SUPPORT
	if (useIni)
	{
		iniFlush();
	}
#endif // TCUD_INI_SUPPORT
#ifdef _QT
	delete qSettings;
#endif // _QT
#ifdef COCOA
	[sessionDict release];
#endif // COCOA
	delete[] appName;
	delete[] sessionName;
	appName = NULL;
	if (commandLine)
	{
		commandLine->release();
		commandLine = NULL;
	}
#if defined(WIN32) && !defined(_QT)
	if (hSessionKey && hSessionKey != hAppDefaultsKey)
	{
		RegCloseKey(hSessionKey);
	}
	if (hAppDefaultsKey)
	{
		RegCloseKey(hAppDefaultsKey);
	}
#endif // WIN32 && NOT _QT
	TCObject::dealloc();
}

TCUserDefaults::~TCUserDefaults(void)
{
}

TCUserDefaults* TCUserDefaults::getCurrentUserDefaults(void)
{
	if (!currentUserDefaults)
	{
		currentUserDefaults = new TCUserDefaults;
	}
	return currentUserDefaults;
}

const TCStringArray* TCUserDefaults::getProcessedCommandLine(void)
{
	return getCurrentUserDefaults()->defGetProcessedCommandLine();
}

TCStringArray* TCUserDefaults::getUnhandledCommandLineArgs(void)
{
	return getCurrentUserDefaults()->defGetUnhandledCommandLineArgs();
}

TCStringArray* TCUserDefaults::getAllKeys(void)
{
	return getCurrentUserDefaults()->defGetAllKeys();
}

TCStringArray* TCUserDefaults::getAllSessionNames(void)
{
	return getCurrentUserDefaults()->defGetAllSessionNames();
}

void TCUserDefaults::saveSessionNameInKey(const char* key)
{
	getCurrentUserDefaults()->defSaveSessionNameInKey(key);
}

char* TCUserDefaults::getSavedSessionNameFromKey(const char* key)
{
	return getCurrentUserDefaults()->defGetSavedSessionNameFromKey(key);
}

void TCUserDefaults::removeSession(const char* value)
{
	getCurrentUserDefaults()->defRemoveSession(value);
}

void TCUserDefaults::initAppPath(void)
{
#ifdef WIN32
	// In Windows, there's an easy way to get the true path of the app.
	// Use it.
	HMODULE hModule = GetModuleHandle(NULL);
	char programPath[2048];

	if (GetModuleFileNameA(hModule, programPath, sizeof(programPath)) > 0)
	{
		char *spot = strrchr(programPath, '\\');

		if (islower(programPath[0]))
		{
			// Windows sucks: GetModuleFileName uses a lower case letter for the
			// drive letter.  GetOpenFilename uses an upper case letter.  People
			// expect this to be upper case.  When running in INI mode, the
			// drive letter from appPath can show up in the recent files list,
			// since any files loaded from that drive go into the INI file with
			// a tag indicating that they're relative to the program path.
			programPath[0] = (char)toupper(programPath[0]);
		}
		if (spot)
		{
			spot[1] = 0;
			appPath = programPath;
		}
	}
#else // WIN32
	char currentDir[2048];
	if (getcwd(currentDir, sizeof(currentDir)))
	{
		if (argv0)
		{
			char *dirPart = directoryFromPath(argv0);
			const char *pathSeparator = "/";

			if (isRelativePath(dirPart))
			{
				appPath = currentDir;
				if (dirPart[0])
				{
					appPath += pathSeparator;
					appPath += dirPart;
				}
			}
			else
			{
				appPath = dirPart;
			}
			appPath += pathSeparator;
			delete[] dirPart;
		}
	}
#endif // WIN32
}

#ifdef __APPLE__

static std::set<std::string> appleSystemArguments;

// Cocoa apps support a bunch of Mac-specific command line arguments. As I see
// them show up (causing file load errors, generally), I will add them to the
// list below. Note that each one has a parameter that goes with it, so the
// argument itself is ignored, along with the following argument.
bool shouldIgnoreArgument(char *argv[], int i)
{
	if (appleSystemArguments.empty())
	{
		appleSystemArguments.insert("-AppleLanguages");
		appleSystemArguments.insert("-AppleTextDirection");
		appleSystemArguments.insert("-AppleLocale");
		appleSystemArguments.insert("-NSForceRightToLeftWritingDirection");
		appleSystemArguments.insert("-NSDoubleLocalizedStrings");
		appleSystemArguments.insert("-NSConstraintBasedLayoutVisualizeMutuallyExclusiveConstraints");
		appleSystemArguments.insert("-NSSurroundLocalizedStrings");
	}
	if (appleSystemArguments.find(argv[i]) != appleSystemArguments.end() ||
		(i > 1 && appleSystemArguments.find(argv[i - 1]) != appleSystemArguments.end()))
	{
		debugPrintf("Ignoring system argument: %s\n", argv[i]);
		return true;
	}
	debugPrintf("Not ignoring argument: %s\n", argv[i]);
	return false;
}
#endif // __APPLE__

void TCUserDefaults::setCommandLine(char *argv[])
{
	TCStringArray *argArray = new TCStringArray;
	int i;

	delete[] argv0;
	argv0 = copyString(argv[0]);
	for (i = 0; argv[i]; i++)
	{
		if (i > 0)
		{
#ifdef __APPLE__
			if (shouldIgnoreArgument(argv, i))
			{
				continue;
			}
#endif // __APPLE__
			argArray->addString(argv[i]);
		}
	}
	getCurrentUserDefaults()->defSetCommandLine(argArray);
	argArray->release();
	initAppPath();
}

void TCUserDefaults::addCommandLineArg(const char *arg)
{
	getCurrentUserDefaults()->defAddCommandLineArg(arg);
}

void TCUserDefaults::setCommandLine(const char *args)
{
	TCStringArray *argArray = new TCStringArray;
	const char *tmpString = args;
	char tmpBuf[1024];

	while (tmpString[0])
	{
		const char *end = NULL;

		if (tmpString[0] == '"')
		{
			end = strchr(tmpString + 1, '"');
			tmpString++;
		}
		else
		{
			end = strchr(tmpString, ' ');
			if (tmpString[0] == '-')
			{
				const char *equals = strchr(tmpString, '=');

				if (equals && equals < end && equals[1] == '"')
				{
					end = strchr(equals + 2, '"');
					if (end[0] == '"')
					{
						end++;
					}
				}
			}
		}
		if (end)
		{
			int length = (int)(end - tmpString);

			strncpy(tmpBuf, tmpString, length);
			tmpBuf[length] = 0;
			if (length)
			{
				argArray->addString(tmpBuf);
			}
			tmpString += length + 1;
			if (end[0] == '"' && tmpString[0])
			{
				tmpString++;
			}
		}
		else
		{
			if (strlen(tmpString))
			{
				argArray->addString(tmpString);
			}
			tmpString += strlen(tmpString);
		}
	}
	getCurrentUserDefaults()->defSetCommandLine(argArray);
	argArray->release();
	initAppPath();
}

bool TCUserDefaults::setIniFile(const char* value)
{
	return getCurrentUserDefaults()->defSetIniFile(value);
}

const char *TCUserDefaults::getIniPath(void)
{
	return getCurrentUserDefaults()->defGetIniPath();
}

void TCUserDefaults::setAppName(const char* value)
{
	getCurrentUserDefaults()->defSetAppName(value);
}

const char* TCUserDefaults::getAppName(void)
{
	return getCurrentUserDefaults()->defGetAppName();
}

void TCUserDefaults::setSessionName(const char* value, const char* saveKey,
									bool copyCurrent)
{
	getCurrentUserDefaults()->defSetSessionName(value, saveKey, copyCurrent);
}

const char* TCUserDefaults::getSessionName(void)
{
	return getCurrentUserDefaults()->defGetSessionName();
}

void TCUserDefaults::setStringForKey(const char* value, const char* key,
				     bool sessionSpecific)
{
	getCurrentUserDefaults()->defSetStringForKey(value, key, sessionSpecific);
}

#ifndef TC_NO_UNICODE
void TCUserDefaults::setStringForKey(CUCSTR value, const char* key,
									 bool sessionSpecific)
{
	getCurrentUserDefaults()->defSetStringForKey(value, key, sessionSpecific);
}
#endif // TC_NO_UNICODE

std::string TCUserDefaults::commandLineStringForKey(const char *key)
{
	char *value = getCurrentUserDefaults()->defCommandLineStringForKey(key);
	std::string retValue;

	if (value)
	{
		retValue = value;
		delete[] value;
	}
	return retValue;
}

char* TCUserDefaults::stringForKey(const char* key, const char* defaultValue,
								   bool sessionSpecific)
{
	return getCurrentUserDefaults()->defStringForKey(key, sessionSpecific,
		defaultValue);
}

const char* TCUserDefaults::defaultStringForKey(const char* key)
{
	return getCurrentUserDefaults()->defDefaultStringForKey(key);
}

CUCSTR TCUserDefaults::defaultStringForKeyUC(const char* key)
{
	return getCurrentUserDefaults()->defDefaultStringForKeyUC(key);
}

long TCUserDefaults::defaultLongForKey(const char* key)
{
	return getCurrentUserDefaults()->defDefaultLongForKey(key);
}

bool TCUserDefaults::defaultBoolForKey(const char* key)
{
	return getCurrentUserDefaults()->defDefaultLongForKey(key) ? true : false;
}

float TCUserDefaults::defaultFloatForKey(const char* key)
{
	return getCurrentUserDefaults()->defDefaultFloatForKey(key);
}

const LongVector& TCUserDefaults::defaultLongVectorForKey(const char* key)
{
	return getCurrentUserDefaults()->defDefaultLongVectorForKey(key);
}

const StringVector& TCUserDefaults::defaultStringVectorForKey(const char* key)
{
	return getCurrentUserDefaults()->defDefaultStringVectorForKey(key);
}

void TCUserDefaults::setPathForKey(const char* value, const char* key,
								   bool sessionSpecific)
{
	getCurrentUserDefaults()->defSetPathForKey(value, key, sessionSpecific);
}

char* TCUserDefaults::pathForKey(const char* key, const char* defaultValue,
								 bool sessionSpecific)
{
	return getCurrentUserDefaults()->defPathForKey(key, sessionSpecific,
		defaultValue);
}

UCSTR TCUserDefaults::stringForKeyUC(const char* key, CUCSTR defaultValue,
									 bool sessionSpecific)
{
	return getCurrentUserDefaults()->defStringForKeyUC(key, sessionSpecific,
		defaultValue);
}

void TCUserDefaults::setLongForKey(long value, const char* key,
								   bool sessionSpecific)
{
	getCurrentUserDefaults()->defSetLongForKey(value, key,
		sessionSpecific);
}

long TCUserDefaults::longForKey(const char* key, long defaultValue,
								bool sessionSpecific)
{
	return getCurrentUserDefaults()->defLongForKey(key, sessionSpecific,
		defaultValue);
}

void TCUserDefaults::setBoolForKey(bool value, const char* key,
								   bool sessionSpecific)
{
	getCurrentUserDefaults()->defSetLongForKey(value ? 1 : 0, key,
		sessionSpecific ? 1 : 0);
}

bool TCUserDefaults::boolForKey(const char* key, bool defaultValue,
								bool sessionSpecific)
{
	long value = getCurrentUserDefaults()->defLongForKey(key, sessionSpecific,
		defaultValue ? 1 : 0);
	return value != 0;
}

void TCUserDefaults::setLongVectorForKey(
	const LongVector &value,
	const char* key,
	bool sessionSpecific,
	int keyDigits /*= 2*/,
	int startIndex /*= 0*/)
{
	getCurrentUserDefaults()->defSetLongVectorForKey(value, key,
		sessionSpecific, keyDigits, startIndex);
}

LongVector TCUserDefaults::longVectorForKey(
	const char* key,
	const LongVector &defaultValue,
	bool sessionSpecific,
	int keyDigits /*= 2*/,
	int startIndex /*= 0*/)
{
	return getCurrentUserDefaults()->defLongVectorForKey(key, sessionSpecific,
		defaultValue, keyDigits, startIndex);
}

void TCUserDefaults::setStringVectorForKey(
	const StringVector &value,
	const char* key,
	bool sessionSpecific,
	bool isPath, /*= false*/
	int keyDigits /*= 2*/,
	int startIndex /*= 0*/)
{
	getCurrentUserDefaults()->defSetStringVectorForKey(value, key,
		sessionSpecific, isPath, keyDigits, startIndex);
}

StringVector TCUserDefaults::stringVectorForKey(
	const char* key,
	const StringVector &defaultValue,
	bool sessionSpecific,
	bool isPath, /*= false*/
	int keyDigits /*= 2*/,
	int startIndex /*= 0*/)
{
	return getCurrentUserDefaults()->defStringVectorForKey(key, sessionSpecific,
		defaultValue, isPath, keyDigits, startIndex);
}

void TCUserDefaults::setFloatForKey(float value, const char* key,
									bool sessionSpecific)
{
	char stringValue[128];

	sprintf(stringValue, "%.24g", value);
	setStringForKey(stringValue, key, sessionSpecific);
}

float TCUserDefaults::floatForKey(const char* key, float defaultValue,
								  bool sessionSpecific)
{
	return getCurrentUserDefaults()->defFloatForKey(key, sessionSpecific,
		defaultValue);
}

void TCUserDefaults::removeValue(const char* key, bool sessionSpecific)
{
	getCurrentUserDefaults()->defRemoveValue(key, sessionSpecific);
}

void TCUserDefaults::removeValueGroup(const char* key, bool sessionSpecific)
{
	getCurrentUserDefaults()->defRemoveValueGroup(key, sessionSpecific);
}

void TCUserDefaults::flush(void)
{
	getCurrentUserDefaults()->defFlush();
}

void TCUserDefaults::defSaveSessionNameInKey(const char* key)
{
	char *savedSessionName = copyString(sessionName);

	if (savedSessionName)
	{
//		defSetSessionName(NULL);
		defSetStringForKey(savedSessionName, key, false);
//		defSetSessionName(savedSessionName);
		delete[] savedSessionName;
	}
	else
	{
		defRemoveValue(key, false);
	}
}

char* TCUserDefaults::defGetSavedSessionNameFromKey(const char* key)
{
	char *savedSessionName;
//	char *currentSessionName = copyString(sessionName);

//	defSetSessionName(NULL);
	savedSessionName = defStringForKey(key, false);
//	defSetSessionName(currentSessionName);
//	delete[] currentSessionName;
	return savedSessionName;
}

void TCUserDefaults::defRemoveSession(const char *value)
{
#ifdef TCUD_INI_SUPPORT
	if (useIni)
	{
		iniRemoveSession(value);
		return;
	}
#endif // TCUD_INI_SUPPORT
#ifdef _QT
	char sessionKey[1024];

	sprintf(sessionKey, "/%s/Sessions/%s", appName, value);
	deleteSubkeys(sessionKey);
#endif // _QT
#ifdef COCOA
	[[NSUserDefaults standardUserDefaults] removePersistentDomainForName:
		getSessionKey(value)];
	if (strcmp(sessionName, value) == 0)
	{
		[sessionDict removeAllObjects];
	}
#endif // COCOA
#if defined(WIN32) && !defined(_QT)
	HKEY hSessionsKey = openKeyPathUnderKey(hAppDefaultsKey, "Sessions");

	if (hSessionsKey)
	{
		HKEY hDelKey = openKeyPathUnderKey(hSessionsKey, value);

		if (hDelKey)
		{
			deleteSubKeys(hDelKey);
			RegCloseKey(hDelKey);
			RegDeleteKeyA(hSessionsKey, value);
		}
	}
	RegCloseKey(hSessionsKey);
#endif // WIN32 && NOT _QT
}

void TCUserDefaults::defSetStringForKey(const char* value, const char* key,
										bool sessionSpecific)
{
	if (matchesCommandLine(key, value))
	{
		// We're being asked to store a value that matches one provided on the
		// command line.
		return;
	}
#ifdef TCUD_INI_SUPPORT
	if (useIni)
	{
		ucstring valueUC;
		utf8toucstring(valueUC, value);
		iniSetStringForKey(valueUC.c_str(), key, sessionSpecific);
	}
	else
	{
#endif // TCUD_INI_SUPPORT
#ifdef _QT
	qSettings->setValue(qKeyForKey(key, sessionSpecific), value);
	requestFlush();
#endif // _QT
#ifdef COCOA
	NSString *nsKey = [NSString stringWithCString: key encoding:
		NSASCIIStringEncoding];

	if (sessionDict && sessionSpecific)
	{
		[sessionDict setObject: [NSString stringWithCString: value encoding:
			NSASCIIStringEncoding] forKey: nsKey];
		[[NSUserDefaults standardUserDefaults] setPersistentDomain: sessionDict
			forName: getSessionKey()];
	}
	else
	{
		[[NSUserDefaults standardUserDefaults] setObject:
			[NSString stringWithCString: value encoding: NSASCIIStringEncoding]
			forKey: nsKey];
	}
#endif // COCOA
#if defined(WIN32) && !defined(_QT)
	defSetValueForKey((LPBYTE)value, (int)strlen(value) + 1, REG_SZ, key,
		sessionSpecific);
#endif // WIN32 && NOT _QT
#ifdef TCUD_INI_SUPPORT
	}
#endif // TCUD_INI_SUPPORT
	sendValueChangedAlert(key);
}

#ifndef TC_NO_UNICODE
void TCUserDefaults::defSetStringForKey(CUCSTR value, const char* key,
										bool sessionSpecific)
{
	std::string valuea;

	ucstringtoutf8(valuea, value);
	if (matchesCommandLine(key, valuea.c_str()))
	{
		// We're being asked to store a value that matches one provided on the
		// command line.
		return;
	}
#ifdef TCUD_INI_SUPPORT
	if (useIni)
	{
		iniSetStringForKey(value, key, sessionSpecific);
	}
	else
	{
#endif // TCUD_INI_SUPPORT
#ifdef _QT
	QString qvalue;
	ucstringtoqstring(qvalue, value);
	qSettings->setValue(qKeyForKey(key, sessionSpecific), qvalue);
	requestFlush();
#endif // _QT
#ifdef COCOA
	NSString *nsKey = [NSString stringWithCString: key encoding:
		NSASCIIStringEncoding];
	unichar *ucValue = NULL;
	size_t valueLen = wcslen(value);

	if (sizeof(wchar_t) == sizeof(unichar))
	{
		ucValue = (unichar *)value;
	}
	else
	{
		ucValue = new unichar[valueLen];
		
		for (size_t i = 0; i < valueLen; i++)
		{
			ucValue[i] = (unichar)value[i];
		}
	}
	if (sessionDict && sessionSpecific)
	{
		[sessionDict setObject: [NSString stringWithCharacters: ucValue
			length: valueLen] forKey: nsKey];
		[[NSUserDefaults standardUserDefaults] setPersistentDomain: sessionDict
			forName: getSessionKey()];
	}
	else
	{
		[[NSUserDefaults standardUserDefaults] setObject:
			[NSString stringWithCharacters: ucValue length: valueLen]
			forKey: nsKey];
	}
	if (ucValue != (unichar *)value)
	{
		delete[] ucValue;
	}
#endif // COCOA
#if defined(WIN32) && !defined(_QT)
	defSetValueForKey((LPBYTE)value,
		((int)ucstrlen(value) + 1) * sizeof(UCCHAR), REG_SZ, key,
		sessionSpecific, true);
#endif // WIN32 && NOT _QT
#ifdef TCUD_INI_SUPPORT
	}
#endif // TCUD_INI_SUPPORT
	sendValueChangedAlert(key);
}
#endif // TC_NO_UNICODE

void TCUserDefaults::defSetPathForKey(const char* value, const char* key,
									  bool sessionSpecific)
{
#ifdef TCUD_INI_SUPPORT
	if (useIni && appPath.size() > 0)
	{
		char *relativePath = findRelativePath(appPath.c_str(), value);

		if (relativePath)
		{
			char *pathValue = copyString(APP_PATH_PREFIX,
				(int)strlen(relativePath));

			strcat(pathValue, relativePath);
			defSetStringForKey(pathValue, key, sessionSpecific);
			delete[] pathValue;
			delete[] relativePath;
			return;
		}
	}
#endif // TCUD_INI_SUPPORT
	defSetStringForKey(value, key, sessionSpecific);
}

char* TCUserDefaults::defPathForKey(const char* key, bool sessionSpecific,
									const char* defaultValue)
{
#ifdef TCUD_INI_SUPPORT
	if (useIni && appPath.size() > 0)
	{
		char *stringValue = defStringForKey(key, sessionSpecific, defaultValue);

		if (stringValue && stringHasPrefix(stringValue, APP_PATH_PREFIX))
		{
			size_t prefixLength = strlen(APP_PATH_PREFIX);
			char *pathValue = copyString(appPath.c_str(),
				strlen(stringValue) - prefixLength);
			char *retValue;

			strcat(pathValue, &stringValue[prefixLength]);
			retValue = cleanedUpPath(pathValue);
			delete[] stringValue;
			delete[] pathValue;
			return retValue;
		}
		return stringValue;
	}
#endif // TCUD_INI_SUPPORT
	return defStringForKey(key, sessionSpecific, defaultValue);
}

int TCUserDefaults::defCommandLineIndexForKey(const char *key)
{
	if (commandLine)
	{
		int i;
		int count = commandLine->getCount();
		size_t keyLength = strlen(key);
		char *keyEquals = new char[keyLength + 3];
		int returnValue = -1;

		strcpy(keyEquals, "-");
		strcat(keyEquals, key);
		strcat(keyEquals, "=");
		for (i = 0; i < count && returnValue == -1; i++)
		{
			char *arg = commandLine->stringAtIndex(i);

			if (stringHasCaseInsensitivePrefix(arg, keyEquals))
			{
				returnValue = i;
			}
		}
		delete[] keyEquals;
		return returnValue;
	}
	return -1;
}

char* TCUserDefaults::defCommandLineStringForKey(const char* key)
{
	if (commandLine)
	{
		int index = defCommandLineIndexForKey(key);

		if (index >= 0)
		{
			char *arg = commandLine->stringAtIndex(index);

			arg += strlen(key) + 2;
			if (strlen(arg) > 0)
			{
				int argLen = (int)strlen(arg);

				if (arg[0] == '"' && arg[argLen - 1] == '"')
				{
					char *retValue = copyString(arg + 1);

					retValue[argLen - 2] = 0;
					return retValue;
				}
				else
				{
					return copyString(arg);
				}
			}
		}
	}
	return NULL;
}

const char* TCUserDefaults::defDefaultStringForKey(const char* key)
{
	return defaultStrings[key].c_str();
}

CUCSTR TCUserDefaults::defDefaultStringForKeyUC(const char* key)
{
	return defaultUCStrings[key].c_str();
}

long TCUserDefaults::defDefaultLongForKey(const char* key)
{
	return defaultLongs[key];
}

float TCUserDefaults::defDefaultFloatForKey(const char* key)
{
	return defaultFloats[key];
}

const LongVector& TCUserDefaults::defDefaultLongVectorForKey(const char* key)
{
	return defaultLongVectors[key];
}

const StringVector& TCUserDefaults::defDefaultStringVectorForKey(
	const char* key)
{
	return defaultStringVectors[key];
}

UCSTR TCUserDefaults::defStringForKeyUC(const char* key, bool sessionSpecific,
	CUCSTR defaultValue)
{
	char *commandLineValue = defCommandLineStringForKey(key);

	if (defaultValue != NULL)
	{
		defaultUCStrings[key] = defaultValue;
	}
	if (commandLineValue)
	{
		UCSTR retValue = utf8toucstring(commandLineValue);

		delete[] commandLineValue;
		return retValue;
	}
#ifdef TCUD_INI_SUPPORT
	if (useIni)
	{
		UCSTR value = iniStringForKey(key, sessionSpecific);

		if (value)
		{
			return value;
		}
		else
		{
			return copyString(defaultValue);
		}
	}
#endif // TCUD_INI_SUPPORT
#ifdef _QT
	QString qDefaultValue;

	ucstringtoqstring(qDefaultValue, defaultValue);
	QString qKey = qKeyForKey(key, sessionSpecific);
	QString string;
	if (!qSettings->contains(qKey))
	{
		if (qDefaultValue == QString::null)
		{
			return NULL;
		}
		string = qDefaultValue;
	}
	else
	{
		string = qSettings->value(qKey, qDefaultValue).toString();
	}
	UCSTR returnValue = new UCCHAR[string.length() + 1];
	int i;

	for (i = 0; i < (int)string.length(); i++)
	{
		QChar qchar = string.at(i);

		returnValue[i] = (wchar_t)qchar.unicode();
	}
	returnValue[i] = 0;
	return returnValue;
#endif // _QT
#ifdef COCOA
	NSString *returnString;
	NSString *nsKey = [NSString stringWithCString: key encoding:
		NSASCIIStringEncoding];

	if (sessionDict && sessionSpecific)
	{
		returnString = [sessionDict objectForKey: nsKey];
	}
	else
	{
		returnString = [[NSUserDefaults standardUserDefaults] objectForKey:
			nsKey];
	}
	if ([returnString isKindOfClass: [NSString class]])
	{
		return utf8toucstring([returnString UTF8String]);
	}
	else
	{
		return copyString(defaultValue);
	}
#endif // COCOA
#if defined(WIN32) && !defined(_QT)
	DWORD size;
	LPBYTE value = defValueForKey(size, REG_SZ, key, sessionSpecific, true);

	if (value)
	{
		return (UCSTR)value;
	}
	else
	{
		return copyString(defaultValue);
	}
#endif // WIN32 && NOT _QT
#ifdef _OSMESA
	// _OSMESA requires INI handling, so hopefully we won't get here.
	return copyString(defaultValue);
#endif // _OSMESA
}

char* TCUserDefaults::defStringForKey(const char* key, bool sessionSpecific,
				      const char* defaultValue)
{
	char *commandLineValue = defCommandLineStringForKey(key);

	if (defaultValue != NULL)
	{
		defaultStrings[key] = defaultValue;
	}
	if (commandLineValue)
	{
		return commandLineValue;
	}
#ifdef TCUD_INI_SUPPORT
	if (useIni)
	{
		UCSTR valueUC = iniStringForKey(key, sessionSpecific);

		if (valueUC)
		{
			char *value = ucstringtoutf8(valueUC);

			delete[] valueUC;
			return value;
		}
		else
		{
			return copyString(defaultValue);
		}
	}
#endif // TCUD_INI_SUPPORT
#ifdef _QT
	QString qKey = qKeyForKey(key, sessionSpecific);
	QString string;
	if (!qSettings->contains(qKey))
	{
		if (defaultValue == NULL)
		{
			return NULL;
		}
		string = defaultValue;
	}
	else
	{
		string = qSettings->value(qKey, defaultValue).toString();
	}
	char *returnValue = new char[string.length() + 1];

	strcpy(returnValue, (const char *)string.toLatin1().constData());
	return returnValue;
#endif // _QT
#ifdef COCOA
	NSString *returnString;
	NSString *nsKey = [NSString stringWithCString: key encoding:
		NSASCIIStringEncoding];

	if (sessionDict && sessionSpecific)
	{
		returnString = [sessionDict objectForKey: nsKey];
	}
	else
	{
		returnString = [[NSUserDefaults standardUserDefaults] objectForKey:
			nsKey];
	}
	if ([returnString isKindOfClass: [NSString class]])
	{
		return copyString([returnString cStringUsingEncoding:
			NSASCIIStringEncoding]);
	}
	else
	{
		return copyString(defaultValue);
	}
#endif // COCOA
#if defined(WIN32) && !defined(_QT)
	DWORD size;
	LPBYTE value = defValueForKey(size, REG_SZ, key, sessionSpecific);

	if (value)
	{
		return (char*)value;
	}
	else
	{
		return copyString(defaultValue);
	}
#endif // WIN32 && NOT _QT
#ifdef _OSMESA
	// _OSMESA requires INI handling, so hopefully we won't get here.
	return copyString(defaultValue);
#endif // _OSMESA
}

void TCUserDefaults::sendValueChangedAlert(const char *key)
{
	TCAlert *alert = new TCAlert(TCUserDefaults::alertClass(), key);

	TCAlertManager::sendAlert(alert);
	alert->release();
}

bool TCUserDefaults::matchesCommandLine(const char *key, long value)
{
	// Don't just convert to string and do a string compare with the command
	// line value.  If they give -someValue=32st on the command line, that will
	// result in a longValueForKey("someValue") returning 32, but "32" won't
	// match "32st".
	bool matches = false;
	char *commandLineValue = defCommandLineStringForKey(key);

	if (commandLineValue)
	{
		long temp;

		if (sscanf(commandLineValue, "%ld", &temp) == 1 && temp == value)
		{
			matches = true;
		}
		delete[] commandLineValue;
	}
	return matches;
}

bool TCUserDefaults::matchesCommandLine(const char *key, const char *value)
{
	bool matches = false;
	char *commandLineValue = defCommandLineStringForKey(key);

	if (commandLineValue)
	{
		if (strcmp(commandLineValue, value) == 0)
		{
			matches = true;
		}
		delete[] commandLineValue;
	}
	return matches;
}

void TCUserDefaults::defSetLongForKey(long value, const char* key,
									  bool sessionSpecific)
{
	if (matchesCommandLine(key, value))
	{
		// We're being asked to store a value that matches one provided on the
		// command line.
		return;
	}
#ifdef TCUD_INI_SUPPORT
	if (useIni)
	{
		UCCHAR valueStr[128];

		sucprintf(valueStr, sizeof(valueStr), _UC("%d"), value);
		iniSetStringForKey(valueStr, key, sessionSpecific);
	}
	else
	{
#endif // TCUD_INI_SUPPORT
#ifdef _QT
	qSettings->setValue(qKeyForKey(key, sessionSpecific), (int)value);
	requestFlush();
#endif // _QT
#ifdef COCOA
	NSString *nsKey = [NSString stringWithCString: key encoding:
		NSASCIIStringEncoding];
	NSNumber *numberValue = [NSNumber numberWithLong: value];

	if (sessionDict && sessionSpecific)
	{
		[sessionDict setObject: numberValue forKey: nsKey];
		[[NSUserDefaults standardUserDefaults] setPersistentDomain: sessionDict
			forName: getSessionKey()];
	}
	else
	{
		[[NSUserDefaults standardUserDefaults] setObject: numberValue forKey:
			nsKey];
	}
#endif // COCOA
#if defined(WIN32) && !defined(_QT)
	defSetValueForKey((LPBYTE)&value, sizeof value, REG_DWORD, key,
		sessionSpecific);
#endif // WIN32 && NOT _QT
#ifdef TCUD_INI_SUPPORT
	}
#endif // TCUD_INI_SUPPORT
	sendValueChangedAlert(key);
}

float TCUserDefaults::defFloatForKey(
	const char* key,
	bool sessionSpecific,
	float defaultValue)
{
	char *stringValue = defStringForKey(key, sessionSpecific, NULL);
	float returnValue = defaultValue;

	defaultFloats[key] = defaultValue;
	if (stringValue)
	{
		if (sscanf(stringValue, "%g", &returnValue) != 1)
		{
			returnValue = defaultValue;
		}
		delete[] stringValue;
	}
	return returnValue;
}

long TCUserDefaults::defLongForKey(const char* key, bool sessionSpecific,
								   long defaultValue, bool *found)
{
	char *commandLineValue = defCommandLineStringForKey(key);

	defaultLongs[key] = defaultValue;
	if (found)
	{
		*found = false;
	}
	if (commandLineValue)
	{
		long returnValue;

		if (sscanf(commandLineValue, "%li", &returnValue) == 1)
		{
			delete[] commandLineValue;
			if (found)
			{
				*found = true;
			}
			return returnValue;
		}
		delete[] commandLineValue;
	}
#ifdef TCUD_INI_SUPPORT
	if (useIni)
	{
		UCSTR value = iniStringForKey(key, sessionSpecific);

		if (value)
		{
			long returnValue;

			if (sucscanf(value, _UC("%li"), &returnValue) == 1)
			{
				if (found)
				{
					*found = true;
				}
			}
			else
			{
				returnValue = defaultValue;
			}
			delete[] value;
			return returnValue;
		}
		return defaultValue;
	}
#endif // TCUD_INI_SUPPORT
#ifdef _QT
	QString qKey = qKeyForKey(key, sessionSpecific);
	if (qSettings->contains(qKey))
	{
		if (found != NULL)
		{
			*found = true;
		}
		return qSettings->value(qKeyForKey(key, sessionSpecific),
			(int)defaultValue).toInt();
	}
	else
	{
		if (found != NULL)
		{
			*found = false;
		}
		return defaultValue;
	}
#endif // _QT
#ifdef COCOA
	NSNumber *returnNumber;
	NSString *nsKey = [NSString stringWithCString: key encoding:
		NSASCIIStringEncoding];

	if (sessionDict && sessionSpecific)
	{
		returnNumber = [sessionDict objectForKey: nsKey];
	}
	else
	{
		returnNumber = [[NSUserDefaults standardUserDefaults] objectForKey:
			nsKey];
	}
	if ([returnNumber isKindOfClass: [NSNumber class]])
	{
		if (found)
		{
			*found = true;
		}
		return [returnNumber longValue];		
	}
	else
	{
		return defaultValue;
	}
#endif // COCOA
#if defined(WIN32) && !defined(_QT)
	DWORD size;
	LPBYTE value = defValueForKey(size, REG_DWORD, key, sessionSpecific);

	if (value)
	{
		long returnValue;

		memcpy(&returnValue, value, sizeof returnValue);
		delete[] value;
		if (found)
		{
			*found = true;
		}
		return returnValue;
	}
	else
	{
		return defaultValue;
	}
#endif // WIN32 && NOT _QT
#ifdef _OSMESA
	// _OSMESA requires INI handling, so hopefully we won't get here.
	return defaultValue;
#endif // _OSMESA
}

// Note: static function
std::string TCUserDefaults::arrayKey(const char *key, int index, int digits)
{
	char indexString[16];
	char formatString[16];

	sprintf(formatString, "%%0%dd", digits);
	sprintf(indexString, formatString, index);
	return (std::string)key + indexString;
}

void TCUserDefaults::defSetLongVectorForKey(
	const LongVector &value,
	const char* key,
	bool sessionSpecific,
	int keyDigits /*= 2*/,
	int startIndex /*= 0*/)
{
	int i;

	for (i = 0; i < (int)value.size(); i++)
	{
		defSetLongForKey(value[i],
			arrayKey(key, i + startIndex, keyDigits).c_str(),
			sessionSpecific);
	}
	// Remove the next value after the ones given.  Note that we don't have to
	// remove all subsequent values, just the one, because when we read the
	// array back, we stop when he hit the first missing one.
	defRemoveValue(arrayKey(key, i + startIndex, keyDigits).c_str(),
		sessionSpecific);
}

LongVector TCUserDefaults::defLongVectorForKey(
	const char* key,
	bool sessionSpecific,
	const LongVector &defaultValue,
	int keyDigits /*= 2*/,
	int startIndex /*= 0*/)
{
	bool found;

	defaultLongVectors[key] = defaultValue;
	defLongForKey(arrayKey(key, startIndex, keyDigits).c_str(), sessionSpecific,
		0, &found);
	if (found)
	{
		int i;
		LongVector retValue;

		for (i = 0; found; i++)
		{
			long value = defLongForKey(
				arrayKey(key, i + startIndex, keyDigits).c_str(),
				sessionSpecific, 0, &found);

			if (found)
			{
				retValue.push_back(value);
			}
		}
		return retValue;
	}
	else
	{
		return defaultValue;
	}
}

void TCUserDefaults::defSetStringVectorForKey(
	const StringVector &value,
	const char* key,
	bool sessionSpecific,
	bool isPath,
	int keyDigits,
	int startIndex)
{
	int i;

	for (i = 0; i < (int)value.size(); i++)
	{
		if (isPath)
		{
			defSetPathForKey(value[i].c_str(),
				arrayKey(key, i + startIndex, keyDigits).c_str(),
				sessionSpecific);
		}
		else
		{
			defSetStringForKey(value[i].c_str(),
				arrayKey(key, i + startIndex, keyDigits).c_str(),
				sessionSpecific);
		}
	}
	// Remove the next value after the ones given.  Note that we don't have to
	// remove all subsequent values, just the one, because when we read the
	// array back, we stop when he hit the first missing one.
	defRemoveValue(arrayKey(key, i + startIndex, keyDigits).c_str(),
		sessionSpecific);
}

StringVector TCUserDefaults::defStringVectorForKey(
	const char* key,
	bool sessionSpecific,
	const StringVector &defaultValue,
	bool isPath,
	int keyDigits,
	int startIndex)
{
	char *value = defStringForKey(arrayKey(key, startIndex, keyDigits).c_str(),
		sessionSpecific, NULL);

	defaultStringVectors[key] = defaultValue;
	if (value)
	{
		int i;
		StringVector retValue;

		delete[] value;
		for (i = 0; true; i++)
		{
			if (isPath)
			{
				value = defPathForKey(
					arrayKey(key, i + startIndex, keyDigits).c_str(),
					sessionSpecific, NULL);
			}
			else
			{
				value = defStringForKey(
					arrayKey(key, i + startIndex, keyDigits).c_str(),
					sessionSpecific, NULL);
			}
			if (value)
			{
				retValue.push_back(value);
				delete[] value;
			}
			else
			{
				break;
			}
		}
		return retValue;
	}
	else
	{
		return defaultValue;
	}
}

void TCUserDefaults::defRemoveValue(const char* key, bool sessionSpecific)
{
#ifdef TCUD_INI_SUPPORT
	if (useIni)
	{
		iniRemoveValue(key, sessionSpecific);
		return;
	}
#endif // TCUD_INI_SUPPORT
#ifdef _QT
	qSettings->remove(qKeyForKey(key, sessionSpecific));
#endif // _QT
#ifdef COCOA
	NSString *nsKey = [NSString stringWithCString: key encoding:
		NSASCIIStringEncoding];

	if (sessionDict && sessionSpecific)
	{
		[sessionDict removeObjectForKey: nsKey];
		[[NSUserDefaults standardUserDefaults] setPersistentDomain: sessionDict
			forName: getSessionKey()];
	}
	else
	{
		[[NSUserDefaults standardUserDefaults] removeObjectForKey: nsKey];
	}
#endif // COCOA
#if defined(WIN32) && !defined(_QT)
	HKEY hParentKey;

	if (sessionSpecific)
	{
		hParentKey = hSessionKey;
	}
	else
	{
		hParentKey = hAppDefaultsKey;
	}
	if (hParentKey)
	{
		const char* spot;

		if ((spot = strrchr(key, '/')) != NULL)
		{
			size_t subKeyLength = spot - key;
			char* keyPath = strncpy(new char[subKeyLength + 1], key, subKeyLength);

			keyPath[subKeyLength] = 0;
			hParentKey = openKeyPathUnderKey(hParentKey, keyPath, true);
			delete[] keyPath;
			spot++;
		}
		else
		{
			spot = key;
		}
		RegDeleteValueA(hParentKey, spot);
		if (hParentKey != hSessionKey && hParentKey != hAppDefaultsKey)
		{
			RegCloseKey(hParentKey);
		}
	}
#endif // WIN32 && NOT _QT
}

void TCUserDefaults::defRemoveValueGroup(const char* key, bool sessionSpecific)
{
#ifdef TCUD_INI_SUPPORT
	if (useIni)
	{
		iniRemoveValueGroup(key, sessionSpecific);
		return;
	}
#endif // TCUD_INI_SUPPORT
#ifdef _QT
	deleteSubkeys(qKeyForKey(key, sessionSpecific));
#endif // _QT
#ifdef COCOA
	NSString *nsKey = [NSString stringWithCString: key encoding:
		NSASCIIStringEncoding];

	if (sessionDict && sessionSpecific)
	{
		[sessionDict removeObjectForKey: nsKey];
		[[NSUserDefaults standardUserDefaults] setPersistentDomain: sessionDict
			forName: getSessionKey()];
	}
	else
	{
		nsKey = [nsKey stringByAppendingString:@"/"];
		NSUserDefaults *ud = [NSUserDefaults standardUserDefaults];
		NSArray *allKeys = [[ud dictionaryRepresentation] allKeys];
		for (NSString *udKey in allKeys)
		{
			if ([udKey hasPrefix:nsKey])
			{
				[ud removeObjectForKey: udKey];
			}
		}
	}
#endif // COCOA
#if defined(WIN32) && !defined(_QT)
	HKEY hParentKey;
	
	if (sessionSpecific)
	{
		hParentKey = hSessionKey;
	}
	else
	{
		hParentKey = hAppDefaultsKey;
	}
	if (hParentKey)
	{
		HKEY hDelKey = openKeyPathUnderKey(hParentKey, key, true);
		deleteSubKeys(hDelKey);
		RegCloseKey(hDelKey);
	}
#endif // WIN32 && NOT _QT
}

void TCUserDefaults::defFlush(void)
{
#ifdef TCUD_INI_SUPPORT
	if (useIni)
	{
		iniFlush();
	}
#endif // TCUD_INI_SUPPORT
#ifdef _QT
	// QSettings only writes to disk when the object is destroyed.  If LDView
	// crashes, all settings that were set are lost.
//	delete qSettings;
//	qSettings = new QSettings("LDView","LDView");
	qSettings->sync();
#endif // _QT
}

TCStringArray* TCUserDefaults::defGetUnhandledCommandLineArgs(void)
{
	if (commandLine)
	{
		TCStringArray *unhandledArgs = new TCStringArray;
		int count = commandLine->getCount();

		for (int i = 0; i < count; i++)
		{
			char *arg = commandLine->stringAtIndex(i);
			
			if (arg[0] != '-' || strchr(arg, '=') == NULL)
			{
				unhandledArgs->addString(arg);
			}
		}
		if (unhandledArgs->getCount() > 0)
		{
			return unhandledArgs;
		}
		else
		{
			unhandledArgs->release();
		}
	}
	return NULL;
}

const TCStringArray* TCUserDefaults::defGetProcessedCommandLine(void)
{
	return commandLine;
}

TCStringArray* TCUserDefaults::defGetAllKeys(void)
{
	TCStringArray *allKeys = new TCStringArray;

#ifdef TCUD_INI_SUPPORT
	if (useIni)
	{
		iniGetAllKeys(allKeys);
		return allKeys;
	}
#endif // TCUD_INI_SUPPORT
#ifdef _QT
	defGetAllKeysUnderKey(qKeyForKey("", true), allKeys);
#endif // _QT
#ifdef COCOA
	NSArray *nsAllKeys;
	int i;
	int count;
	
	if (sessionDict)
	{
		nsAllKeys = [sessionDict allKeys];
	}
	else
	{
		// We have to synchronize before we read things, because
		// otherwise the non-session values won't get flushed into the
		// main app's persistent domain.
		[[NSUserDefaults standardUserDefaults] synchronize];
		nsAllKeys = [[[NSUserDefaults standardUserDefaults]
			persistentDomainForName: [[NSBundle mainBundle] bundleIdentifier]] allKeys];
	}
	count = (int)[nsAllKeys count];
	for (i = 0; i < count; i++)
	{
		allKeys->addString([[nsAllKeys objectAtIndex: i]
			cStringUsingEncoding: NSASCIIStringEncoding]);
	}
#endif // COCOA
#if defined(WIN32) && !defined(_QT)
	defGetAllKeysUnderKey(hSessionKey, "", allKeys);
#endif // WIN32 && NOT _QT
	return allKeys;
}

TCStringArray* TCUserDefaults::defGetAllSessionNames(void)
{
	TCStringArray *allSessionNames = new TCStringArray;

#ifdef TCUD_INI_SUPPORT
	if (useIni)
	{
		iniGetAllSessionNames(allSessionNames);
		return allSessionNames;
	}
#endif // TCUD_INI_SUPPORT
#ifdef _QT
	char key[1024];
	QStringList subkeyList;
	
	sprintf(key, "/%s/Sessions/", appName);
	qSettings->beginGroup(key);
	subkeyList = qSettings->childGroups();
	for (QStringList::const_iterator it = subkeyList.begin();
		it != subkeyList.end(); ++it)
	{
		allSessionNames->addString((const char *)it->toLatin1().constData());
	}
	qSettings->endGroup();
#endif // _QT
#ifdef COCOA
	NSArray *domainNames = [[NSUserDefaults standardUserDefaults]
		persistentDomainNames];
	NSString *prefix = getSessionKey("");
	int prefixLength = (int)[prefix length];
	int i;
	int count;

	count = (int)[domainNames count];
	for (i = 0; i < count; i++)
	{
		NSString *domainName = [domainNames objectAtIndex: i];
		if (![domainName isEqualToString: prefix] &&
			[domainName hasPrefix: prefix])
		{
			NSString *sessionName = [domainName substringFromIndex:
				prefixLength];
			allSessionNames->addString([sessionName cStringUsingEncoding:
				NSASCIIStringEncoding]);
		}
	}
#endif // COCOA
#if defined(WIN32) && !defined(_QT)
	HKEY hSessionsKey = openKeyPathUnderKey(hAppDefaultsKey, "Sessions");
	if (hSessionsKey)
	{
		int i;
		int status = ERROR_SUCCESS;

		for (i = 0; status == ERROR_SUCCESS; i++)
		{
			char keyName[1024];
			DWORD keyNameLen = sizeof(keyName);

			if ((status = RegEnumKeyA(hSessionsKey, i, keyName, keyNameLen)) ==
				ERROR_SUCCESS)
			{
				HKEY hSubKey;

				if (RegOpenKeyA(hSessionsKey, keyName, &hSubKey) == ERROR_SUCCESS)
				{
					allSessionNames->addString(keyName);
					RegCloseKey(hSubKey);
				}
			}
		}
		RegCloseKey(hSessionsKey);
	}
#endif // WIN32 && NOT _QT
	return allSessionNames;
}

void TCUserDefaults::defAddCommandLineArg(const char *arg)
{
	int count = commandLine->getCount();

	if (count > 0 && strcmp(arg, "=") == 0)
	{
		commandLine->appendString("=", count - 1);
	}
	else if (count > 0 && stringHasSuffix(commandLine->stringAtIndex(count - 1),
		"="))
	{
		commandLine->appendString(arg, count - 1);
	}
	else
	{
		commandLine->addString(arg);
	}
}

void TCUserDefaults::defSetCommandLine(TCStringArray *argArray)
{
	int i;
	int count = argArray->getCount();

	if (commandLine)
	{
		commandLine->release();
	}
	commandLine = new TCStringArray;
	for (i = 0; i < count; i++)
	{
		defAddCommandLineArg(argArray->stringAtIndex(i));
	}
#ifdef TCUD_INI_SUPPORT
	const char *iniFile = defCommandLineStringForKey("IniFile");
	if (iniFile != NULL)
	{
		setIniFile(iniFile);
	}
#endif // TCUD_INI_SUPPORT
}

#ifdef TCUD_INI_SUPPORT

bool TCUserDefaults::iniSetSessionName(const char *value, bool copyCurrent)
{
	bool isNewSession = false;

	if (copyCurrent && value)
	{
		IniKey &sessionsKey = rootIniKey.children["Sessions"];
		IniKeyMap::iterator it1 = sessionsKey.children.find(value);

		if (it1 == sessionsKey.children.end())
		{
			if (sessionName)
			{
				sessionsKey.children[value] = sessionsKey.children[sessionName];
			}
			else
			{
				IniKey &newSession = sessionsKey.children[value];
				newSession.values = rootIniKey.values;
				for (IniKeyMap::iterator it2 = rootIniKey.children.begin();
					it2 != rootIniKey.children.end(); ++it2)
				{
					if (it2->first != "Sessions")
					{
						newSession.children[it2->first] = it2->second;
					}
				}
			}
			isNewSession = true;
			iniChanged();
		}
	}
	delete[] sessionName;
	sessionName = copyString(value);
	return isNewSession;
}

void TCUserDefaults::iniGetAllKeys(TCStringArray *allKeys)
{
	char *newKey = iniKeyString("", true);
	char *pathPart = newKey;
	IniKey *pKey = findIniKey(&rootIniKey, pathPart);

	if (pKey)
	{
		for (StringStringMap::const_iterator it = pKey->values.begin();
			it != pKey->values.end(); ++it)
		{
			allKeys->addString(it->first.c_str());
		}
	}
	delete[] newKey;
}

void TCUserDefaults::iniGetAllSessionNames(TCStringArray *allSessionNames)
{
	IniKeyMap::const_iterator it1 = rootIniKey.children.find("Sessions");

	if (it1 != rootIniKey.children.end())
	{
		const IniKey &sessionsKey = it1->second;

		for (IniKeyMap::const_iterator it2 = sessionsKey.children.begin();
			it2 != sessionsKey.children.end(); ++it2)
		{
			allSessionNames->addString(it2->first.c_str());
		}
	}
}

void TCUserDefaults::iniWriteValues(
	FILE *iniFile,
	const IniKey &iniKey,
	const char *keyPrefix)
{
	for (StringStringMap::const_iterator it = iniKey.values.begin();
		it != iniKey.values.end(); ++it)
	{
		char *value = createEscapedString(it->second.c_str());

		fprintf(iniFile, "%s%s=%s\n", keyPrefix, it->first.c_str(), value);
		delete[] value;
	}
}

void TCUserDefaults::iniWriteKey(
	FILE *iniFile,
	const IniKey &iniKey,
	const char *keyPrefix)
{
	std::string keyPrefixStr(keyPrefix);

	iniWriteValues(iniFile, iniKey, keyPrefix);
	for (IniKeyMap::const_iterator it = iniKey.children.begin();
		it != iniKey.children.end(); ++it)
	{
		std::string newPrefix = keyPrefixStr + it->first + "/";
		iniWriteKey(iniFile, it->second, newPrefix.c_str());
	}
}

void TCUserDefaults::iniFlush(void)
{
	if (iniChanges)
	{
		FILE *iniFile = ucfopen(iniPath.c_str(), "w");

		if (iniFile)
		{
			fprintf(iniFile, "[General]\n");
			iniWriteValues(iniFile, rootIniKey, "");
			fprintf(iniFile, "\n");
			for (IniKeyMap::const_iterator it = rootIniKey.children.begin();
				it != rootIniKey.children.end(); ++it)
			{
				fprintf(iniFile, "[%s]\n", it->first.c_str());
				iniWriteKey(iniFile, it->second, "");
				fprintf(iniFile, "\n");
			}
			fclose(iniFile);
		}
		iniChanges = false;
	}
}

char *TCUserDefaults::iniKeyString(const char *key, bool sessionSpecific)
{
	char *newKey;
	size_t keyLen = strlen(key);

	if (sessionSpecific && sessionName)
	{
		newKey = new char[keyLen + strlen(sessionName) + 20];

		sprintf(newKey, "Sessions/%s/%s", sessionName, key);
	}
	else
	{
		newKey = copyString(key);
	}
	return newKey;
}

void TCUserDefaults::iniRemoveSession(const char *value)
{
	char *newKey = copyString("Sessions/", strlen(value));
	char *pathPart = newKey;
	IniKey *pKey;

	strcat(newKey, value);
	pKey = findIniKey(&rootIniKey, pathPart);
	if (pKey)
	{
		pKey->children.erase(value);
	}
}

void TCUserDefaults::iniRemoveValue(const char *key, bool sessionSpecific)
{
	char *newKey = iniKeyString(key, sessionSpecific);
	char *pathPart = newKey;
	IniKey *pKey = findIniKey(&rootIniKey, pathPart);

	pKey->values.erase(pathPart);
	delete[] newKey;
}

void TCUserDefaults::iniRemoveValueGroup(const char *key, bool sessionSpecific)
{
	char *newKey = iniKeyString(key, sessionSpecific);
	char *pathPart = newKey;
	IniKey *pKey = findIniKey(&rootIniKey, pathPart);
	
	if (pKey != NULL)
	{
		pKey->children.erase(pathPart);
	}
	delete[] newKey;
}

void TCUserDefaults::iniSetStringForKey(
	CUCSTR value,
	const char *key,
	bool sessionSpecific)
{
	char *newKey = iniKeyString(key, sessionSpecific);
	char *pathPart = newKey;
	IniKey *pKey = findIniKey(&rootIniKey, pathPart);
	std::string utf8Value;

	ucstringtoutf8(utf8Value, value);
	pKey->values[pathPart] = utf8Value;
	iniChanged();
	delete[] newKey;
}

void TCUserDefaults::requestFlush(void)
{
	if (!flushRequested)
	{
		TCUserDefaultsFlusher *flusher = new TCUserDefaultsFlusher;

		// When the flusher gets released by the autorelease pool, it will
		// trigger a flush, which will write our settings to disk.  This way we
		// can make a bunch of settings changes, and only have to write to disk
		// once.
		flusher->autorelease();
		flushRequested = true;
	}
}

void TCUserDefaults::iniChanged(void)
{
	iniChanges = true;
	requestFlush();
}

UCSTR TCUserDefaults::iniStringForKey(const char *key, bool sessionSpecific)
{
	char *newKey = iniKeyString(key, sessionSpecific);
	char *pathPart = newKey;
	IniKey *pKey = findIniKey(&rootIniKey, pathPart);
	StringStringMap::const_iterator it;

	if ((it = pKey->values.find(pathPart)) != pKey->values.end())
	{
		delete[] newKey;
		return utf8toucstring(it->second.c_str(), (int)it->second.size());
	}
	else
	{
		delete[] newKey;
		return NULL;
	}
}

TCUserDefaults::IniKey *TCUserDefaults::findIniKey(IniKey *pKey, char *&pathPart)
{
	char *slashSpot;

	while ((slashSpot = strchr(pathPart, '/')) != NULL)
	{
		slashSpot[0] = 0;
		pKey = &pKey->children[pathPart];
		pathPart = &slashSpot[1];
	}
	return pKey;
}

bool TCUserDefaults::defSetIniFile(const char* value)
#else // TCUD_INI_SUPPORT
bool TCUserDefaults::defSetIniFile(const char* /*value*/)
#endif // TCUD_INI_SUPPORT
{
	bool retValue = false;

#ifdef TCUD_INI_SUPPORT
#ifdef NO_WSTRING
	iniPath.resize(0);
#else // NO_WSTRING
	iniPath.clear();
#endif // NO_WSTRING
	rootIniKey.children.clear();
	rootIniKey.values.clear();
	if (isRelativePath(value))
	{
		if (appPath.size() > 0)
		{
			iniPath = appPath + value;
		}
	}
	else
	{
		iniPath = value;
	}
	if (iniPath.size() > 0)
	{
		FILE *iniFile = ucfopen(iniPath.c_str(), "r+b");

		if (iniFile)
		{
			char *line = new char[MAX_INI_LINE_LEN];
			bool haveGeneralSection = false;
			std::string sectionName;

			while (1)
			{
				if (fgets(line, MAX_INI_LINE_LEN, iniFile) == NULL)
				{
					break;
				}
				stripCRLF(line);
				stripLeadingWhitespace(line);
				int length = (int)strlen(line);
				char *equalsSpot;

				if (line[0] == '[' && line[length - 1] == ']')
				{
					line[length - 1] = 0;
					sectionName = &line[1];
					if (!haveGeneralSection &&
						(strcasecmp(sectionName.c_str(), "General") == 0 ||
						strcasecmp(sectionName.c_str(), "LDView") == 0))
					{
						haveGeneralSection = true;
#ifdef NO_WSTRING
						sectionName.resize(0);
#else // NO_WSTRING
						sectionName.clear();
#endif // NO_WSTRING
					}
				}
				else if (line[0] != ';' && (haveGeneralSection ||
					sectionName.size() > 0) &&
					(equalsSpot = strchr(line, '=')) != NULL)
				{
					IniKey *pKey;

					if (sectionName.size() > 0)
					{
						pKey = &rootIniKey.children[sectionName];
					}
					else
					{
						pKey = &rootIniKey;
					}
					equalsSpot[0] = 0;
					char *pathPart = line;
					pKey = findIniKey(pKey, pathPart);
					processEscapedString(&equalsSpot[1]);
					pKey->values[pathPart] = &equalsSpot[1];
				}
			}
			delete[] line;
			fclose(iniFile);
			retValue = true;
		}
		else
		{
			retValue = false;
//			iniFile = ucfopen(iniPath.c_str(), "w");
//			if (iniFile)
//			{
//				if (fprintf(iniFile, "[General]\n") >= 10)
//				{
//					retValue = true;
//				}
//				fclose(iniFile);
//				unlink(iniPath.c_str());
//			}
		}
	}
	if (retValue)
	{
		useIni = true;
	}
#endif // TCUD_INI_SUPPORT
	return retValue;
}

void TCUserDefaults::defSetAppName(const char* value)
{
	if (appName != value)
	{
		delete[] appName;
		appName = copyString(value);
		defSetSessionName(NULL, NULL, false);
#if defined(WIN32) && !defined(_QT)
		if (!useIni)
		{
			if (hAppDefaultsKey)
			{
				RegCloseKey(hAppDefaultsKey);
			}
			hAppDefaultsKey = openAppDefaultsKey();
			hSessionKey = hAppDefaultsKey;
		}
#endif // WIN32 && NOT _QT
#ifdef COCOA
		initSessionDict();
#endif // COCOA
	}
}

void TCUserDefaults::defSetSessionName(const char* value, const char *saveKey,
									   bool copyCurrent)
{
	if (value != sessionName)
	{
		bool isNewSession = false;

#ifdef TCUD_INI_SUPPORT
		if (useIni)
		{
			isNewSession = iniSetSessionName(value, copyCurrent);
		}
		else
		{
#endif // TCUD_INI_SUPPORT
#ifdef _QT
		char key[1024];
		QStringList sessionNames;
		
		sprintf(key, "/%s/Sessions/", appName);
		qSettings->beginGroup(key);
		sessionNames = qSettings->childGroups();
		if (value && sessionNames.indexOf(value) == -1)
		{
			char srcKey[1024];
			char dstKey[1024];

			sprintf(dstKey, "%s%s", key, value);
			if (sessionName)
			{
				sprintf(srcKey, "%s%s", key, sessionName);
			}
			else
			{
				sprintf(srcKey, "/%s/", appName);
			}
			copyTree(dstKey, srcKey, key);
			isNewSession = true;
		}
		qSettings->endGroup();
		delete[] sessionName;
		sessionName = copyString(value);
#endif // _QT
#ifdef COCOA
		delete[] sessionName;
		sessionName = copyString(value);
		if ([[NSUserDefaults standardUserDefaults]
			persistentDomainForName: getSessionKey()] == nil)
		{
			// The new session doesn't exist yet, so copy the current session
			// into it.  Note that if the current session is already in
			// sessionDict, we can just continue to use that.  Otherwise,
			// we need to create a new sessionDict.
			if (!sessionDict)
			{
				// We have to synchronize before we read things, because
				// otherwise the non-session values won't get flushed into the
				// main app's persistent domain.
				[[NSUserDefaults standardUserDefaults] synchronize];
				sessionDict = [[[NSUserDefaults standardUserDefaults] persistentDomainForName: [[NSBundle mainBundle] bundleIdentifier]] mutableCopy];
				if (!sessionDict)
				{
					sessionDict = [[NSMutableDictionary alloc] init];
				}
			}
			[[NSUserDefaults standardUserDefaults] setPersistentDomain:
				sessionDict forName: getSessionKey()];
			isNewSession = true;
		}
		else
		{
			initSessionDict();
		}
#endif // COCOA
#if defined(WIN32) && !defined(_QT)
		HKEY hOldSessionKey = hSessionKey;

		delete[] sessionName;
		sessionName = copyString(value);
		if (sessionName && appName)
		{
			hSessionKey = openSessionKey();
			if (!hSessionKey)
			{
				hSessionKey = hOldSessionKey;
				if (copyCurrent)
				{
					TCStringArray *allKeys;
					int i;
					int count;
					char *sessionPrefix = new char[strlen(sessionName) + 128];

					sprintf(sessionPrefix, "Sessions/%s/", sessionName);
					allKeys = defGetAllKeys();
					count = allKeys->getCount();
					for (i = 0; i < count; i++)
					{
						char *key = allKeys->stringAtIndex(i);
						char *newKey = new char[strlen(sessionPrefix) +
							strlen(key) + 4];

						sprintf(newKey, "%s%s", sessionPrefix, key);
						if (defIsLongKey(key, true))
						{
							long longValue = defLongForKey(key, true);

							hSessionKey = hAppDefaultsKey;
							setLongForKey(longValue, newKey);
						}
						else
						{
							char *stringValue = defStringForKey(key, true);

							hSessionKey = hAppDefaultsKey;
							setStringForKey(stringValue, newKey);
							delete[] stringValue;
						}
						hSessionKey = hOldSessionKey;
						delete[] newKey;
					}
					allKeys->release();
					delete[] sessionPrefix;
				}
				else
				{
					char *dummyKeyName = new char[strlen(sessionName) + 128];

					sprintf(dummyKeyName, "Sessions/%s/dummy", sessionName);
					setLongForKey(1, dummyKeyName);
					removeValue(dummyKeyName);
					delete[] dummyKeyName;
				}
				hSessionKey = openSessionKey();
				isNewSession = true;
			}
		}
		else
		{
			hSessionKey = hAppDefaultsKey;
		}
		if (hOldSessionKey && hOldSessionKey != hAppDefaultsKey &&
			hOldSessionKey != hSessionKey)
		{
			RegCloseKey(hOldSessionKey);
		}
#endif // WIN32 && NOT _QT
#ifdef TCUD_INI_SUPPORT
		}
#endif // TCUD_INI_SUPPORT
		if (isNewSession)
		{
			defSetStringForKey("DO NOT DELETE.", "_SessionPlaceholder", true);
		}
	}
	if (saveKey)
	{
		defSaveSessionNameInKey(saveKey);
	}
}

#if defined(WIN32) && !defined(_QT)

void TCUserDefaults::defSetValueForKey(const LPBYTE value, int length,
									   DWORD type, const char* key,
									   bool sessionSpecific, bool unicode)
{
	int index = defCommandLineIndexForKey(key);
	HKEY hParentKey;

	if (index >= 0)
	{
		commandLine->removeStringAtIndex(index);
	}
	if (sessionSpecific)
	{
		hParentKey = hSessionKey;
	}
	else
	{
		hParentKey = hAppDefaultsKey;
	}
	if (hParentKey)
	{
		const char* spot;

		if ((spot = strrchr(key, '/')) != NULL)
		{
			size_t subKeyLength = spot - key;
			char* keyPath = strncpy(new char[subKeyLength + 1], key, subKeyLength);

			keyPath[subKeyLength] = 0;
			hParentKey = openKeyPathUnderKey(hParentKey, keyPath, true);
			spot++;
			delete[] keyPath;
		}
		else
		{
			spot = key;
		}
#ifdef TC_NO_UNICODE
		unicode = false;
#endif // TC_NO_UNICODE
		if (unicode)
		{
#ifndef TC_NO_UNICODE
			ucstring spotUC;
			utf8toucstring(spotUC, spot);
			RegSetValueExW(hParentKey, spotUC.c_str(), 0, type, value, length);
#endif //TC_NO_UNICODE
		}
		else
		{
			RegSetValueExA(hParentKey, spot, 0, type, value, length);
		}
		if (hParentKey != hSessionKey && hParentKey != hAppDefaultsKey)
		{
			RegCloseKey(hParentKey);
		}
	}
}

LPBYTE TCUserDefaults::defValueForKey(DWORD& size, DWORD type, const char* key,
									  bool sessionSpecific, bool unicode)
{
	HKEY hParentKey;

	if (sessionSpecific)
	{
		hParentKey = hSessionKey;
	}
	else
	{
		hParentKey = hAppDefaultsKey;
	}
	if (hParentKey)
	{
		DWORD valueType;
		const char* spot;
		LPBYTE value = NULL;

		if ((spot = strrchr(key, '/')) != NULL)
		{
			size_t subKeyLength = spot - key;
			char* keyPath = strncpy(new char[subKeyLength + 1], key, subKeyLength);

			keyPath[subKeyLength] = 0;
			hParentKey = openKeyPathUnderKey(hParentKey, keyPath, true);
			delete[] keyPath;
			spot++;
		}
		else
		{
			spot = key;
		}
		size = 0;
#ifdef TC_NO_UNICODE
		unicode = false;
#endif // TC_NO_UNICODE
		ucstring spotUC;
		if (unicode)
		{
			utf8toucstring(spotUC, spot);
		}
#ifdef TC_NO_UNICODE
		if (RegQueryValueExA(hParentKey, spot, 0, &valueType, NULL, &size) ==
			ERROR_SUCCESS)
#else // TC_NO_UNICODE
		if ((!unicode && RegQueryValueExA(hParentKey, spot, 0, &valueType, NULL,
			&size) == ERROR_SUCCESS) ||
			(unicode && RegQueryValueExW(hParentKey, spotUC.c_str(), 0,
			&valueType, NULL, &size) == ERROR_SUCCESS))
#endif //TC_NO_UNICODE
		{
			if (valueType == type)
			{
				value = new BYTE[size];
#ifdef TC_NO_UNICODE
				if (RegQueryValueExA(hParentKey, spot, 0, &valueType, value,
					&size) != ERROR_SUCCESS)
#else // TC_NO_UNICODE
				if ((!unicode && RegQueryValueExA(hParentKey, spot, 0,
					&valueType, value, &size) != ERROR_SUCCESS) ||
					(unicode && RegQueryValueExW(hParentKey, spotUC.c_str(), 0,
					&valueType, value, &size) != ERROR_SUCCESS))
#endif // TC_NO_UNICODE
				{
					delete[] value;
					value = NULL;
				}
			}
		}
		if (hParentKey != hSessionKey && hParentKey != hAppDefaultsKey)
		{
			RegCloseKey(hParentKey);
		}
		return value;
	}
	return NULL;
}

void TCUserDefaults::defGetAllKeysUnderKey(HKEY parentKey, const char* keyPath,
										   TCStringArray* allKeys)
{
	int i;
	long status = ERROR_SUCCESS;

	for (i = 0; status == ERROR_SUCCESS; i++)
	{
		char valueName[1024];
		DWORD valueNameLen = sizeof(valueName);
 
		if ((status = RegEnumValueA(parentKey, i, valueName, &valueNameLen, NULL,
			NULL, NULL, NULL)) == ERROR_SUCCESS)
		{
			char *keyName = new char[strlen(valueName) + strlen(keyPath) + 16];

			sprintf(keyName, "%s%s", keyPath, valueName);
			allKeys->addString(keyName);
			delete[] keyName;
		}
	}
	status = ERROR_SUCCESS;
	for (i = 0; status == ERROR_SUCCESS; i++)
	{
		char keyName[1024];
		DWORD keyNameLen = sizeof(keyName);

		if ((status = RegEnumKeyA(parentKey, i, keyName, keyNameLen)) ==
			ERROR_SUCCESS)
		{
			HKEY hSubKey;

			if (RegOpenKeyA(parentKey, keyName, &hSubKey) == ERROR_SUCCESS)
			{
				char *newPath = new char[strlen(keyName) + strlen(keyPath) + 4];

				sprintf(newPath, "%s%s/", keyPath, keyName);
				if (strcmp(newPath, "Sessions/") != 0)
				{
					// Don't return session keys.
					defGetAllKeysUnderKey(hSubKey, newPath, allKeys);
				}
				delete[] newPath;
				RegCloseKey(hSubKey);
			}
		}
	}
}

bool TCUserDefaults::defIsLongKey(const char *key, bool sessionSpecific)
{
	DWORD size;
	LPBYTE value = defValueForKey(size, REG_DWORD, key, sessionSpecific);

	if (value)
	{
		delete[] value;
		return true;
	}
	return false;
}

HKEY TCUserDefaults::openKeyPathUnderKey(HKEY parentKey, const char* keyPath,
										 bool create)
{
	int count;
	char** components = componentsSeparatedByString(keyPath, "/", count);
	HKEY currentKey = parentKey;
	DWORD disposition;
	int i;
	bool failed = false;

	for (i = 0; i < count && !failed; i++)
	{
		char* component = components[i];

		if (strlen(component) > 0)
		{
			HKEY newKey;

			if (create)
			{
				if (RegCreateKeyExA(currentKey, component, 0, NULL, 0,
					KEY_WRITE | KEY_READ, NULL, &newKey, &disposition) !=
					ERROR_SUCCESS)
				{
					failed = true;
				}
			}
			else
			{
				if (RegOpenKeyExA(currentKey, component, 0, KEY_WRITE | KEY_READ,
					&newKey) != ERROR_SUCCESS)
				{
					failed = true;
				}
			}
			if (currentKey != parentKey)
			{
				RegCloseKey(currentKey);
			}
			currentKey = newKey;
		}
	}
	deleteStringArray(components, count);
	if (failed)
	{
		return NULL;
	}
	else
	{
		return currentKey;
	}
}

HKEY TCUserDefaults::openAppDefaultsKey(void)
{
//	HKEY softwareKey;
//	DWORD disposition;
//	HKEY returnValue = NULL;
	char* keyPath = new char[strlen(appName) + 128];
	HKEY retValue;

	sprintf(keyPath, "Software/%s", appName);
	retValue = openKeyPathUnderKey(HKEY_CURRENT_USER, keyPath, true);
	delete[] keyPath;
	return retValue;
}

HKEY TCUserDefaults::openSessionKey(void)
{
	char* keyPath = new char[strlen(appName) + strlen(sessionName) + 128];
	HKEY retValue;

	sprintf(keyPath, "Software/%s/Sessions/%s", appName, sessionName);
	retValue = openKeyPathUnderKey(HKEY_CURRENT_USER, keyPath, false);
	delete[] keyPath;
	return retValue;
}

void TCUserDefaults::deleteSubKeys(HKEY hKey)
{
	while (1)
	{
		char name[1024];
		HKEY hSubKey;

		// Since we're deleting all sub-keys, we always just want the first
		// one.
		if (RegEnumKeyA(hKey, 0, name, sizeof(name)) != ERROR_SUCCESS)
		{
			break;
		}
		if (RegOpenKeyA(hKey, name, &hSubKey) == ERROR_SUCCESS)
		{
			deleteSubKeys(hSubKey);
			RegCloseKey(hSubKey);
			RegDeleteKeyA(hKey, name);
		}
	}
}

#endif // WIN32 && NOT _QT


#ifdef _QT

char *TCUserDefaults::qKeyForKey(const char *key, bool sessionSpecific)
{
	if (sessionSpecific && sessionName)
	{
		sprintf(qKey, "/%s/Sessions/%s/%s", appName, sessionName, key);
	}
	else
	{
		sprintf(qKey, "/%s/%s", appName, key);
	}
	return qKey;
}

void TCUserDefaults::deleteSubkeys(const char *key)
{
	qSettings->beginGroup(key);
	QStringList subkeyList = qSettings->childGroups();
	QStringList entryList = qSettings->childKeys();
	int i;
	int count = subkeyList.count();

	for (i = 0; i < count; i++)
	{
		char subkey[1024];

		sprintf(subkey, "%s/%s", key,
			(const char *)subkeyList[i].toLatin1().constData());
		deleteSubkeys(subkey);
	}
	count = entryList.count();
	for (i = 0; i < count; i++)
	{
		char subkey[1024];

		sprintf(subkey, "%s/%s", key,
			(const char *)entryList[i].toLatin1().constData());
		qSettings->remove(subkey);
	}
	qSettings->endGroup();
	qSettings->remove(key);
}

void TCUserDefaults::defGetAllKeysUnderKey(const char *key,
										   TCStringArray *allKeys)
{
	qSettings->beginGroup(key);
	QStringList subkeyList = qSettings->childGroups();
	QStringList entryList = qSettings->childKeys();
	int i;
	int count = subkeyList.count();

	for (i = 0; i < count; i++)
	{
		char subkey[1024];

		sprintf(subkey, "%s/%s", key,
			(const char *)subkeyList[i].toLatin1().constData());
		defGetAllKeysUnderKey(subkey, allKeys);
	}
	qSettings->endGroup();
	count = entryList.count();
	for (i = 0; i < count; i++)
	{
		allKeys->addString((const char *)subkeyList[i].toLatin1().constData());
	}
}

void TCUserDefaults::copyTree(const char *dstKey, const char *srcKey,
							  const char *skipKey)
{
	qSettings->beginGroup(srcKey);
	QStringList subkeyList = qSettings->childGroups();
	QStringList entryList = qSettings->childKeys();
	QStringList::const_iterator it;

	if (strcmp(dstKey, skipKey) == 0)
	{
		return;
	}
	for (it = subkeyList.begin(); it != subkeyList.end(); ++it)
	{
		char srcSubkey[1024];
		char dstSubkey[1024];

		sprintf(srcSubkey, "%s/%s", srcKey,
			(const char *)it->toLatin1().constData());
		sprintf(dstSubkey, "%s/%s", dstKey,
			(const char *)it->toLatin1().constData());
		copyTree(dstSubkey, srcSubkey, skipKey);
	}
	for (it = entryList.begin(); it != entryList.end(); ++it)
	{
		char srcSubKey[1024];
		char dstSubKey[1024];

		sprintf(srcSubKey, "%s/%s", srcKey,
			(const char *)it->toLatin1().constData());
		sprintf(dstSubKey, "%s/%s", dstKey,
			(const char *)it->toLatin1().constData());
		qSettings->setValue(dstSubKey, qSettings->value(srcSubKey));
	}
	qSettings->endGroup();
	requestFlush();
}

#endif // _QT

#ifdef COCOA

NSString *TCUserDefaults::getSessionKey(const char *key)
{
	if (key == NULL)
	{
		key = sessionName;
	}
	if (key)
	{
		return [NSString stringWithFormat: @"%@.Session.%s", [[NSBundle mainBundle] bundleIdentifier], key];
	}
	else
	{
		return [NSString stringWithFormat: @"%@", [[NSBundle mainBundle] bundleIdentifier]];
	}
}

void TCUserDefaults::initSessionDict(void)
{
	if (sessionDict)
	{
		[sessionDict release];
		sessionDict = nil;
	}
	if (sessionName)
	{
		sessionDict = [[[NSUserDefaults standardUserDefaults]
			persistentDomainForName: getSessionKey()] mutableCopy];
		if (!sessionDict)
		{
			sessionDict = [[NSMutableDictionary alloc] init];
		}
	}
}

#endif // COCOA
