
/****************************************************************************

**

** Copyright (C) 2007-2009 Kevin Clague. All rights reserved.

** Copyright (C) 2015 - 2023 Trevor SANDY. All rights reserved.

**

** This file may be used under the terms of the GNU General Public

** License version 2.0 as published by the Free Software Foundation

** and appearing in the file LICENSE.GPL included in the packaging of

** this file.  Please review the following information to ensure GNU

** General Public Licensing requirements will be met:

** http://www.trolltech.com/products/qt/opensource.html

**

** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE

** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.

**

****************************************************************************/

#include <QDir>
#include "paths.h"
#include "lpub_preferences.h"
#include "version.h"

Paths paths;

QString Paths::lpubDir          = QLatin1String(VER_PRODUCTNAME_STR);
QString Paths::tmpDir           = QString("%1%2tmp").arg(VER_PRODUCTNAME_STR).arg(QDir::separator());
QString Paths::assemDir         = QString("%1%2assem").arg(VER_PRODUCTNAME_STR).arg(QDir::separator());
QString Paths::partsDir         = QString("%1%2parts").arg(VER_PRODUCTNAME_STR).arg(QDir::separator());
QString Paths::bomDir           = QString("%1%2bom").arg(VER_PRODUCTNAME_STR).arg(QDir::separator());
QString Paths::submodelDir      = QString("%1%2submodels").arg(VER_PRODUCTNAME_STR).arg(QDir::separator());
QString Paths::povrayRenderDir  = QString("%1%2povray").arg(VER_PRODUCTNAME_STR).arg(QDir::separator());
QString Paths::blenderRenderDir = QString("%1%2blender").arg(VER_PRODUCTNAME_STR).arg(QDir::separator());
QString Paths::htmlStepsDir     = QString("%1%2htmlsteps").arg(VER_PRODUCTNAME_STR).arg(QDir::separator());
QString Paths::logsDir          = QLatin1String("logs");
QString Paths::extrasDir        = QLatin1String("extras");
QString Paths::libraryDir       = QLatin1String("libraries");

QString Paths::customDir        = QString("%1custom").arg(Preferences::validLDrawLibrary);
QString Paths::customPartDir    = QString("%1custom%2parts").arg(Preferences::validLDrawLibrary).arg(QDir::separator());
QString Paths::customSubDir     = QString("%1custom%2parts%2s").arg(Preferences::validLDrawLibrary).arg(QDir::separator());
QString Paths::customTextureDir = QString("%1custom%2parts%2textures").arg(Preferences::validLDrawLibrary).arg(QDir::separator());
QString Paths::customPrimDir    = QString("%1custom%2p").arg(Preferences::validLDrawLibrary).arg(QDir::separator());
QString Paths::customPrim8Dir   = QString("%1custom%2p%28").arg(Preferences::validLDrawLibrary).arg(QDir::separator());
QString Paths::customPrim48Dir  = QString("%1custom%2p%248").arg(Preferences::validLDrawLibrary).arg(QDir::separator());

QStringList Paths::customDirs;

void Paths::mkPovrayDir(){

    QDir dir;
    dir.mkdir(povrayRenderDir);

}

void Paths::mkBlenderDir(){

    QDir dir;
    dir.mkdir(blenderRenderDir);

}

void Paths::mkDirs(){

    QDir dir;
    dir.mkdir(lpubDir);
    dir.mkdir(tmpDir);
    dir.mkdir(bomDir);
    dir.mkdir(assemDir);
    dir.mkdir(partsDir);
    dir.mkdir(submodelDir);

}

void Paths::mkCustomDirs() {

  QDir dir;

  QString dp = Preferences::lpubDataPath;
  if(! dir.exists(dp + QDir::separator() + customDir))
    dir.mkdir(dp + QDir::separator() + customDir);

  if (! dir.exists(dp + QDir::separator() + customPartDir))
    dir.mkdir(dp + QDir::separator() + customPartDir);

  if (! dir.exists(dp + QDir::separator() + customSubDir))
    dir.mkdir(dp + QDir::separator() + customSubDir);

  if (! dir.exists(dp + QDir::separator() + customTextureDir))
    dir.mkdir(dp + QDir::separator() + customTextureDir);

  if (! dir.exists(dp + QDir::separator() + customPrimDir))
    dir.mkdir(dp + QDir::separator() + customPrimDir);

  if (! dir.exists(dp + QDir::separator() + customPrim8Dir))
    dir.mkdir(dp + QDir::separator() + customPrim8Dir);

  if (! dir.exists(dp + QDir::separator() + customPrim48Dir))
    dir.mkdir(dp + QDir::separator() + customPrim48Dir);

  customDirs << dp + QDir::separator() + customPartDir
             << dp + QDir::separator() + customSubDir
             << dp + QDir::separator() + customTextureDir
             << dp + QDir::separator() + customPrimDir
             << dp + QDir::separator() + customPrim8Dir
             << dp + QDir::separator() + customPrim48Dir;
}
