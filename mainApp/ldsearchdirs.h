/****************************************************************************
**
** Copyright (C) 2015 - 2023 Trevor SANDY. All rights reserved.
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

#include <TCFoundation/mystring.h>

#define LP3DExport

class LDPartsDirs
{
public:
    LDPartsDirs(void){};
    virtual const char *getSearchDirsOrigin(void) const { return m_searchDirsOrigin; }
    virtual bool        loadLDrawSearchDirs(const char *filename);             //send default arbitrary file name
    virtual bool        initLDrawSearchDirs();                                 //initialize ldrawini and check for errors
    StringList          getLDrawSearchDirs(void) { return m_ldrawSearchDirs; }

protected:
    char       *m_searchDirsOrigin;
    StringList  m_ldrawSearchDirs;
};

#endif // LDSEARCHDIRS_H
