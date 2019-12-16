
/****************************************************************************

**

** Copyright (C) 2007-2009 Kevin Clague. All rights reserved.

** Copyright (C) 2015 - 2019 Trevor SANDY. All rights reserved.

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

Paths paths;

QString Paths::lpubDir         = "LPub3D";
QString Paths::tmpDir          = QDir::toNativeSeparators("LPub3D/tmp");
QString Paths::assemDir        = QDir::toNativeSeparators("LPub3D/assem");
QString Paths::partsDir        = QDir::toNativeSeparators("LPub3D/parts");
QString Paths::submodelDir     = QDir::toNativeSeparators("LPub3D/submodels");
QString Paths::povrayRenderDir = QDir::toNativeSeparators("LPub3D/povray");
QString Paths::logsDir         = "logs";
QString Paths::extrasDir       = "extras";
QString Paths::libraryDir      = "libraries";

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
  customPartDir   = QDir::toNativeSeparators(Preferences::validLDrawLibrary+"custom/parts");
  customSubDir    = QDir::toNativeSeparators(Preferences::validLDrawLibrary+"custom/parts/s");
  customPrimDir   = QDir::toNativeSeparators(Preferences::validLDrawLibrary+"custom/p");
  customPrim8Dir  = QDir::toNativeSeparators(Preferences::validLDrawLibrary+"custom/p/8");
  customPrim48Dir = QDir::toNativeSeparators(Preferences::validLDrawLibrary+"custom/p/48");

  QDir dir;

  QString dp = Preferences::lpubDataPath;
  if(! dir.exists(dp + QDir::separator() + customDir))
    dir.mkdir(dp + QDir::separator() + customDir);

  if (! dir.exists(dp + QDir::separator() + customPartDir))
    dir.mkdir(dp + QDir::separator() + customPartDir);

  if (! dir.exists(dp + QDir::separator() + customSubDir))
    dir.mkdir(dp + QDir::separator() + customSubDir);

  if (! dir.exists(dp + QDir::separator() + customPrimDir))
    dir.mkdir(dp + QDir::separator() + customPrimDir);

  if (! dir.exists(dp + QDir::separator() + customPrim8Dir))
    dir.mkdir(dp + QDir::separator() + customPrim8Dir);

  if (! dir.exists(dp + QDir::separator() + customPrim48Dir))
    dir.mkdir(dp + QDir::separator() + customPrim48Dir);

  customDirs << QDir::toNativeSeparators(dp + "/" + customPartDir)
             << QDir::toNativeSeparators(dp + "/" + customSubDir)
             << QDir::toNativeSeparators(dp + "/" + customPrimDir)
             << QDir::toNativeSeparators(dp + "/" + customPrim8Dir)
             << QDir::toNativeSeparators(dp + "/" + customPrim48Dir);
}
