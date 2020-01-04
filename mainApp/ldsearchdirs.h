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

#ifndef LDSEARCHDIRS_H
#define LDSEARCHDIRS_H

#include <LDLoader/LDLModel.h>

#define LP3DExport

typedef std::map<std::string, bool> StringBoolMap;

class PartWorker;
class LDLModel;

class LDPartsDirs : public LDLModel
{
public:
  LDPartsDirs(void);
  virtual const char *getSearchDirsOrigin(void) const { return m_searchDirsOrigin; }
  virtual bool        loadLDrawSearchDirs(const char *filename);             //send default arbitrary file name
  virtual bool        initLDrawSearchDirs();                                 //initialize ldrawini and check for errors
  StringList          getLDrawSearchDirs(void) { return m_ldrawSearchDirs; }
  static bool         verifyExtraDir(const char *value);

protected:
  StringList        m_ldrawSearchDirs;
  StringList 	    m_extraSearchDirs;
  char             *m_searchDirsOrigin;
};

#endif // LDSEARCHDIRS_H
