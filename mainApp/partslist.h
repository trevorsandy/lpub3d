 

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



/****************************************************************************

 *

 * This class reads and internalizes the part titles described in PARTS.LST

 * distributed with the LDraw parts library.

 *

 * Please see lpub.h for an overall description of how the files in LPub

 * make up the LPub program.

 *

 ***************************************************************************/



#ifndef PARTSLIST_H

#define PARTSLIST_H



#include <QHash>

#include <QString>

#include <QStringList>



class PartsList {

  private:

    static QHash<QString, QString> list;

    static QString     empty;

    static QStringList partialPaths;

  public:

    PartsList();

    static const QString &title(QString part);

    static bool  isKnownPart(QString &part);

};

#endif

