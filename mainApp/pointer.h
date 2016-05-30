 
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

/****************************************************************************
 *
 * This class describes the graphics independent representation of a pointer
 * extended from a callout to the assembly image where the called out submodel
 * is used.
 *
 * Please see lpub.h for an overall description of how the files in LPub
 * make up the LPub program.
 *
 ***************************************************************************/

#ifndef pointerH
#define pointerH

#include <QGraphicsItem>
#include <QGraphicsScene>
#include "where.h"
#include "meta.h"

class Pointer {
  public:
    PointerMeta pointerMeta;
    Where       here;

    Pointer() {}
    Pointer(
      Where        _here,
      CalloutMeta &attrib)
    {
      here       = _here;
      pointerMeta = attrib.pointer;
    };

    virtual ~Pointer()
    {
    }
};
#endif
