/****************************************************************************
**
** Copyright (C) 2015 Trevor SANDY. All rights reserved.
**
** This file may be used under the terms of the
** GNU General Public Liceense (GPL) version 3.0
** which accompanies this distribution, and is
** available at http://www.gnu.org/licenses/gpl.html
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef LDSEARCHDIRS_H
#define LDSEARCHDIRS_H

//#ifdef WIN32
//#define TCExport
//#endif // WIN32

#define TCExport

#if defined (__APPLE__)
#include <wchar.h>
#endif // __APPLE__


#ifndef WIN32
#include <unistd.h>
#include <sys/stat.h>
#endif // !WIN32

#ifdef _QT
#include <QtCore/qstring.h>
#endif // _QT


// STRING UTILITIES
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <stdarg.h>

#include <stack>

#include "QsLog.h"

TCExport char *copyString(const char *string, size_t pad = 0);
TCExport char *cleanedUpPath(const char* path);
TCExport void  replaceStringCharacter(char*, char, char, int = 1);
TCExport void  stripTrailingPathSeparators(char*);
TCExport char *directoryFromPath(const char*);
TCExport char *componentsJoinedByString(char** array, int count,
					const char* separator);
TCExport char **componentsSeparatedByString(const char* string,
					    const char* separator, int& count);
TCExport char *stringByReplacingSubstring(const char* string,
					      const char* oldSubstring,
					      const char* newSubstring,
					      bool repeat = true);
template<class T> inline TCExport void deleteStringArray(T** array, int count)
{
	int i;

	for (i = 0; i < count; i++)
	{
		delete array[i];
	}
	delete array;
}
// END STRING UTILITIES


class PartWorker;
class LDPartsDirs;

typedef bool (*LDLFileCaseCallback)(char *filename);
typedef std::list<std::string> StringList;

struct LDrawIniS;

class LDSearchDirs
{
public:
  LDSearchDirs(void);
  static const char *lDrawDir(bool defaultValue = false);
  static       void  setLDrawDir(const char *value);
  static void setFileCaseCallback(LDLFileCaseCallback value);
  static LDLFileCaseCallback getFileCaseCallback(void)
  {
          return fileCaseCallback;
  }

  static       bool  verifyLDrawDir(const char *value);
  static       bool  verifyExtraDir(const char *value);
  static       bool  verifyArchiveDir(const char *value);
  static       void  initCheckDirs();

protected:
  LDPartsDirs         *m_partsDirs;
  static StringList    sm_checkDirs;
  static       char   *sm_systemLDrawDir;
  static       char   *sm_defaultLDrawDir;
  static LDrawIniS    *sm_lDrawIni;

  static LDLFileCaseCallback fileCaseCallback;
  static class LDLPartsCleanup
  {
  public:
          ~LDLPartsCleanup(void);
  } sm_cleanup;
  friend class LDLPartsCleanup;
};

typedef std::map<std::string, bool> StringBoolMap;

class LDPartsDirs : public LDSearchDirs
{
public:
  LDPartsDirs(void);
  virtual bool      loadLDrawSearchDirs(const char *filename);   //send default arbitrary file name
  virtual void      setExtraSearchDirs(const char *value);
  StringList        getExtraSearchDirs(void) { return m_extraSearchDirs; } //this is not used
  StringList        getLDrawSearchDirs(void) { return m_ldrawSearchDirs; }

protected:
  StringList        m_ldrawSearchDirs;
  StringList 	    m_extraSearchDirs;
  StringBoolMap     m_ancestorMap;
};

#endif // LDSEARCHDIRS_H
