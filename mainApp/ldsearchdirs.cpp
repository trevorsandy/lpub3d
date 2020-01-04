/****************************************************************************
**
** Copyright (C) 2015 - 2020 Trevor SANDY. All rights reserved.
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

#include "ldsearchdirs.h"
#include <TCFoundation/mystring.h>
#include <LDLoader/LDrawIni.h>
#include <LDVQt/LDVWidget.h>
#include "threadworkers.h"
#include "lpub_preferences.h"

#include <string.h>

#ifdef _MSC_VER
#include <direct.h>
#endif

LDPartsDirs::LDPartsDirs(){}

bool LDPartsDirs::initLDrawSearchDirs()
{
  // set default ldraw Directory
  setLDrawDir(Preferences::ldrawLibPath.toLatin1().constData());

  LDLModel::setFileCaseCallback(LDVWidget::staticFileCaseCallback);

  // initialize ldrawIni and check for error
  lDrawDir();

  if (sm_lDrawIni)
    {
      m_searchDirsOrigin = strdup(sm_lDrawIni->SearchDirsOrigin);
      //qDebug() << "LDrawini Initialized - m_searchDirsOrigin: " << m_searchDirsOrigin;
      return true;
    } else {
      //qDebug() << "Unable to initialize LDrawini: ";
      return false;
    }
}

// NOTE: static function. (NOT USED)
bool LDPartsDirs::verifyExtraDir(const char *value)
{
  char currentDir[1024];
  bool retValue = false;

  if (value && getcwd(currentDir, sizeof(currentDir)))
    {
      if (chdir(value) == 0)
        {
            retValue = true;
        }
      if (chdir(currentDir) != 0)
        {
          qDebug("Error going back to original directory.\n");
          qDebug("currentDir before: <%s>\n", currentDir);
          if (getcwd(currentDir, sizeof(currentDir)) != nullptr)
            {
              qDebug("currentDir  after: <%s>\n", currentDir);
            }
        }
    }
  return retValue;
}

bool LDPartsDirs::loadLDrawSearchDirs(const char *filename) //send default arbitrary file name
{
  bool retValue = false;

  if (!initLDrawSearchDirs()) {
      qDebug() << "Could not initialize LDrawini ";
      return retValue;
    }

  if (sm_lDrawIni)
    {
      LDrawIniComputeRealDirs(sm_lDrawIni, 1, 0, filename);
    }

  if (!strlen(lDrawDir()))
    {
      return retValue;
    }
  // end initialize

  //search dirs from ldraw.ini
  if (sm_lDrawIni && sm_lDrawIni->nSearchDirs > 0)
    {
      int i;

      qDebug() << "";

      for (i = 0; i < sm_lDrawIni->nSearchDirs; i++)
        {
          LDrawSearchDirS *searchDir = &sm_lDrawIni->SearchDirs[i];

          if ((searchDir->Flags & LDSDF_SKIP) == 0)  //Add function to verifyExcludedDirs()
            {
              m_ldrawSearchDirs.push_back(searchDir->Dir);
              logInfo() << "LDRAW SEARCH DIR PUSHED: " << searchDir->Dir;
            }
        }

      qDebug() << "";

      //process extra seach directories (NOT USED)
      bool found = false;
      for (StringList::const_iterator it = m_extraSearchDirs.begin(); !found &&
           it != m_extraSearchDirs.end(); it++)
        {
          const char *dir = it->c_str();

          if (verifyLDrawDir(dir))
            {
              m_ldrawSearchDirs.push_back(dir);
              qDebug() << "LDRAW EXTRA SEARCH DIR PUSHED: " << dir;
              found = true;
            }
        }
      retValue = chdir(Preferences::lpub3dPath.toLatin1().constData()) == 0;
    }
  return retValue;
}
