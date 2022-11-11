
/****************************************************************************

**

** Copyright (C) 2007-2009 Kevin Clague. All rights reserved.

** Copyright (C) 2015 - 2022 Trevor SANDY. All rights reserved.

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
QString Paths::tmpDir           = QString("%1/tmp").arg(VER_PRODUCTNAME_STR);
QString Paths::assemDir         = QString("%1/assem").arg(VER_PRODUCTNAME_STR);
QString Paths::partsDir         = QString("%1/parts").arg(VER_PRODUCTNAME_STR);
QString Paths::submodelDir      = QString("%1/submodels").arg(VER_PRODUCTNAME_STR);
QString Paths::povrayRenderDir  = QString("%1/povray").arg(VER_PRODUCTNAME_STR);
QString Paths::blenderRenderDir = QString("%1/blender").arg(VER_PRODUCTNAME_STR);
QString Paths::htmlStepsDir     = QString("%1/htmlsteps").arg(VER_PRODUCTNAME_STR);
QString Paths::logsDir          = QLatin1String("logs");
QString Paths::extrasDir        = QLatin1String("extras");
QString Paths::libraryDir       = QLatin1String("libraries");

QString Paths::customDir       = QString();
QString Paths::customPartDir   = QString();
QString Paths::customSubDir    = QString();
QString Paths::customPrimDir   = QString();
QString Paths::customPrim8Dir  = QString();
QString Paths::customPrim48Dir = QString();

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
    dir.mkdir(assemDir);
    dir.mkdir(partsDir);
    dir.mkdir(submodelDir);

}

void Paths::mkCustomDirs(){

  customDir       = Preferences::validLDrawLibrary+"custom";
  customPartDir   = Preferences::validLDrawLibrary+"custom/parts";
  customSubDir    = Preferences::validLDrawLibrary+"custom/parts/s";
  customPrimDir   = Preferences::validLDrawLibrary+"custom/p";
  customPrim8Dir  = Preferences::validLDrawLibrary+"custom/p/8";
  customPrim48Dir = Preferences::validLDrawLibrary+"custom/p/48";

  QDir dir;

  QString dp = Preferences::lpubDataPath;
  if(! dir.exists(dp + "/" + customDir))
    dir.mkdir(dp + "/" + customDir);

  if (! dir.exists(dp + "/" + customPartDir))
    dir.mkdir(dp + "/" + customPartDir);

  if (! dir.exists(dp + "/" + customSubDir))
    dir.mkdir(dp + "/" + customSubDir);

  if (! dir.exists(dp + "/" + customPrimDir))
    dir.mkdir(dp + "/" + customPrimDir);

  if (! dir.exists(dp + "/" + customPrim8Dir))
    dir.mkdir(dp + "/" + customPrim8Dir);

  if (! dir.exists(dp + "/" + customPrim48Dir))
    dir.mkdir(dp + "/" + customPrim48Dir);

  customDirs << dp + "/" + customPartDir
             << dp + "/" + customSubDir
             << dp + "/" + customPrimDir
             << dp + "/" + customPrim8Dir
             << dp + "/" + customPrim48Dir;
}
