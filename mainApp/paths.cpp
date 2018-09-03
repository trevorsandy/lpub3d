 

/****************************************************************************

**

** Copyright (C) 2007-2009 Kevin Clague. All rights reserved.

** Copyright (C) 2015 - 2018 Trevor SANDY. All rights reserved.

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

#include <QSettings>
#include <QDir>
#include "paths.h"
#include "lpub_preferences.h"

Paths paths;

QString Paths::lpubDir   = "LPub3D";
QString Paths::tmpDir    = "LPub3D/tmp";
QString Paths::assemDir  = "LPub3D/assem";
QString Paths::partsDir  = "LPub3D/parts";

QString Paths::logsDir    = "logs";
QString Paths::extrasDir  = "extras";
QString Paths::libraryDir = "libraries";

QString Paths::customDir       = "custom";
QString Paths::customPartDir   = "custom/parts";
QString Paths::customSubDir    = "custom/parts/s";
QString Paths::customPrimDir   = "custom/p";
QString Paths::customPrim8Dir  = "custom/p/8";
QString Paths::customPrim48Dir = "custom/p/48";

QStringList Paths::customDirs;

void Paths::mkDirs(){

    QDir dir;
    dir.mkdir(lpubDir);
    dir.mkdir(tmpDir);
    dir.mkdir(assemDir);
    dir.mkdir(partsDir);

}

void Paths::mkCustomDirs(){

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
