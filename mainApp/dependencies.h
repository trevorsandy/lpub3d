 
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

/*
 * The GUI knows about the LDraw files, but the Step class is the one that
 * knows when it wants to render a new image.  It is in the right place
 * to decide if a CSI is out of date.  I don't want to expose all of the
 * LPub GUI to Step, so these routines provide an interface
 */

#ifndef DEPENDENCIES
#define DEPENDENCIES

#include <QDateTime>

bool isMpd();
bool isOlder(const QStringList &, const QDateTime &);

#endif
