/****************************************************************************
**
** Copyright (C) 2015 - 2025 Trevor SANDY. All rights reserved.
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
#include <LDLoader/LDLModel.h>
#include <LDLoader/LDrawIni.h>
#include <LDVQt/LDVWidget.h>
#include "lpub_preferences.h"

#ifdef _MSC_VER
#include <direct.h>
#endif

bool LDPartsDirs::initLDrawSearchDirs()
{
    // set default ldraw Directory
    LDLModel::setLDrawDir(Preferences::ldrawLibPath.toLatin1().constData());

    LDLModel::setFileCaseCallback(LDVWidget::staticFileCaseCallback);

    // initialize ldrawIni and check for error
    LDLModel::lDrawDir();

    if (LDLModel::getlDrawIni())
    {
        m_searchDirsOrigin = strdup(LDLModel::getlDrawIni()->SearchDirsOrigin);
        //qDebug() << "LDrawini Initialized - m_searchDirsOrigin: " << m_searchDirsOrigin;
        return true;
    }

    //qDebug() << "Unable to initialize LDrawini: ";
    return false;
}

bool LDPartsDirs::loadLDrawSearchDirs(const char *filename) //send default arbitrary file name
{
    bool retValue = false;

    // initialize ldrawIni
    if (!initLDrawSearchDirs())
        return false;

    LDrawIniS  *lDrawIni = LDLModel::getlDrawIni();

    if (lDrawIni)
        LDrawIniComputeRealDirs(lDrawIni, 1, 0, filename);

    if (!strlen(LDLModel::lDrawDir()))
        return retValue;

    //search dirs from ldraw.ini
    if (lDrawIni && lDrawIni->nSearchDirs > 0)
    {
        int i;

        qDebug() << "";

        for (i = 0; i < lDrawIni->nSearchDirs; i++)
        {
            LDrawSearchDirS *searchDir = &lDrawIni->SearchDirs[i];

            if ((searchDir->Flags & LDSDF_SKIP) == 0)  //Add function to verifyExcludedDirs()
            {
                m_ldrawSearchDirs.push_back(searchDir->Dir);
                LDVWidget::messageSig(LOG_INFO, QString("LDRAW SEARCH DIR PUSHED: %1").arg(searchDir->Dir));
            }
        }

        qDebug() << "";

        retValue = chdir(Preferences::lpub3dPath.toLatin1().constData()) == 0;
    }

    return retValue;
}
