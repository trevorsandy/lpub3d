 

/****************************************************************************

**

** Copyright (C) 2007-2009 Kevin Clague. All rights reserved.

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



Paths paths;



QString Paths::lpubDir   = "LPub";

QString Paths::tmpDir    = "LPub/tmp";

QString Paths::assemDir  = "LPub/assem";

QString Paths::partsDir  = "LPub/parts";

QString Paths::viewerDir = "LPub/viewer";



void Paths::mkdirs()

{

  QDir dir;

  dir.mkdir(lpubDir);

  dir.mkdir(tmpDir);

  dir.mkdir(assemDir);

  dir.mkdir(partsDir);

  dir.mkdir(viewerDir);

}

