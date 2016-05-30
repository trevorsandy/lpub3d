 
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

#ifndef PATHS_H
#define PATHS_H

#include <QString>

class Paths {
public:

    static void mkdirs();
    static void mkfadedirs();
    static QString lpubDir;
    static QString tmpDir;
    static QString assemDir;
    static QString partsDir;
    static QString viewerDir;
    static QString fadePartDir;
    static QString fadeSubDir;
    static QString fadePrimDir;
    static QString fadePrim8Dir;
    static QString fadePrim48Dir;
};

#endif
