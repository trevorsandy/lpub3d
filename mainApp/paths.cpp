 

/****************************************************************************

**

** Copyright (C) 2007-2009 Kevin Clague. All rights reserved.

** Copyright (C) 2015 Trevor SANDY. All rights reserved.

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

#include <QtGui>
#include <QSettings>
#include <QDir>
#include "paths.h"
#include "lpub_preferences.h"

Paths paths;

QString Paths::lpubDir   = "LPub";
QString Paths::tmpDir    = "LPub/tmp";
QString Paths::assemDir  = "LPub/assem";
QString Paths::partsDir  = "LPub/parts";
QString Paths::viewerDir = "LPub/viewer";

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
  fadePartDir = Preferences::ldrawPath + "/Unofficial/fade";
  //    if(! dir.exists(fadePartDir)) {
  //        fadePartDir       = Preferences::ldrawPath + "/Unofficial/parts/fade";
  //        dir.mkdir(fadePartDir);
  //        fadeSubDir      = Preferences::ldrawPath + "/Unofficial/parts/fade/s";
  //        dir.mkdir(fadeSubDir);
  //    }

  if(! dir.exists(fadePartDir)) {
      dir.mkdir(fadePartDir);
      fadePartDir   = Preferences::ldrawPath + "/Unofficial/fade/parts";
      dir.mkdir(fadePartDir);
      fadeSubDir    = Preferences::ldrawPath + "/Unofficial/fade/parts/s";
      dir.mkdir(fadeSubDir);
      fadePrimDir   = Preferences::ldrawPath + "/Unofficial/fade/p";
      dir.mkdir(fadePrimDir);
      fadePrim8Dir  = Preferences::ldrawPath + "/Unofficial/fade/p/8";
      dir.mkdir(fadePrim8Dir);
      fadePrim48Dir = Preferences::ldrawPath + "/Unofficial/fade/p/48";
      dir.mkdir(fadePrim48Dir);
    } else {
      fadePartDir   = Preferences::ldrawPath + "/Unofficial/fade/parts";
      fadeSubDir    = Preferences::ldrawPath + "/Unofficial/fade/parts/s";
      fadePrimDir   = Preferences::ldrawPath + "/Unofficial/fade/p";
      fadePrim8Dir  = Preferences::ldrawPath + "/Unofficial/fade/p/8";
      fadePrim48Dir = Preferences::ldrawPath + "/Unofficial/fade/p/48";
    }

  //    if(! dir.exists(fadePrimDir)) {
  //        fadePrimDir      = Preferences::ldrawPath + "/Unofficial/p/fade/";
  //        dir.mkdir(fadePrimDir);
  //        fadePrim8Dir     = Preferences::ldrawPath + "/Unofficial/p/fade/8";
  //        dir.mkdir(fadePrim8Dir);
  //        fadePrim48Dir    = Preferences::ldrawPath + "/Unofficial/p/fade/48";
  //        dir.mkdir(fadePrim48Dir);
  //    }

}

