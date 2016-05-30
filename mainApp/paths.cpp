 

/****************************************************************************

**

** Copyright (C) 2007-2009 Kevin Clague. All rights reserved.

** Copyright (C) 2016 Trevor SANDY. All rights reserved.

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
QString Paths::viewerDir = "LPub3D/viewer";

QString Paths::fadePartDir;
QString Paths::fadeSubDir;
QString Paths::fadePrimDir;
QString Paths::fadePrim8Dir;
QString Paths::fadePrim48Dir;


void Paths::mkdirs(){

    QDir dir;
    dir.mkdir(lpubDir);
    dir.mkdir(tmpDir);
    dir.mkdir(assemDir);
    dir.mkdir(partsDir);
    dir.mkdir(viewerDir);

}

void Paths::mkfadedirs(){

  QDir dir;
  fadePartDir = QDir::toNativeSeparators(Preferences::ldrawPath + "/Unofficial/fade");
  if(! dir.exists(fadePartDir)) {
      dir.mkdir(fadePartDir);
      fadePartDir   = QDir::toNativeSeparators(Preferences::ldrawPath + "/Unofficial/fade/parts");
      dir.mkdir(fadePartDir);
      fadeSubDir    = QDir::toNativeSeparators(Preferences::ldrawPath + "/Unofficial/fade/parts/s");
      dir.mkdir(fadeSubDir);
      fadePrimDir   = QDir::toNativeSeparators(Preferences::ldrawPath + "/Unofficial/fade/p");
      dir.mkdir(fadePrimDir);
      fadePrim8Dir  = QDir::toNativeSeparators(Preferences::ldrawPath + "/Unofficial/fade/p/8");
      dir.mkdir(fadePrim8Dir);
      fadePrim48Dir = QDir::toNativeSeparators(Preferences::ldrawPath + "/Unofficial/fade/p/48");
      dir.mkdir(fadePrim48Dir);
    } else {
      fadePartDir   = QDir::toNativeSeparators(Preferences::ldrawPath + "/Unofficial/fade/parts");
      fadeSubDir    = QDir::toNativeSeparators(Preferences::ldrawPath + "/Unofficial/fade/parts/s");
      fadePrimDir   = QDir::toNativeSeparators(Preferences::ldrawPath + "/Unofficial/fade/p");
      fadePrim8Dir  = QDir::toNativeSeparators(Preferences::ldrawPath + "/Unofficial/fade/p/8");
      fadePrim48Dir = QDir::toNativeSeparators(Preferences::ldrawPath + "/Unofficial/fade/p/48");
    }
}

