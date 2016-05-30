/****************************************************************************
 
**

** Copyright (C) 2007-2008 Kevin Clague. All rights reserved.

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



#include "rx.h"

#include <QString>

#include <QStringList>


//  30/11/2014 Generating "multiple definition of split" compile error:
inline void split(const QString &line, QStringList &argv)

{

  QString     chopped = line;



  while (chopped[0] == ' ') {

    chopped.remove(0,1);

  }

  argv << chopped.split(" ",QString::SkipEmptyParts);



  if (argv.size() >= 2 && argv[0] == "0" && argv[1] == "GHOST") {

    chopped.remove(0,2);

    while (chopped[0] == ' ') {

      chopped.remove(0,1);

    }

    argv.removeFirst();

    chopped.remove(0,6);

    while (chopped[0] == ' ') {

      chopped.remove(0,1);

    }

    argv.removeFirst();

  }

}

