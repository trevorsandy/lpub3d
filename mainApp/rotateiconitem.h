/****************************************************************************
**
** Copyright (C) 2015 Trevor SANDY. All rights reserved.
**
** This file may be used under the terms of the
** GNU General Public Liceense (GPL) version 3.0
** which accompanies this distribution, and is
** available at http://www.gnu.org/licenses/gpl.html
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

/****************************************************************************
 *
 * This class provides a simple mechanism for displaying a rotation icon
 * on the page.
 *
 ***************************************************************************/

#ifndef ROTATEICONITEM_H
#define ROTATEICONITEM_H

#include <QGraphicsPixmapItem>
#include <QSize>
#include <QRect>
#include "meta.h"
#include "metaitem.h"
#include "resize.h"

#include "QsLog.h"
class Page;
class RotateIconItem : public ResizePixmapItem
{
  Page                     *page;
public:

  InsertMeta                insMeta;
  RotateIconMeta            rotateIconMeta;
  PlacementType             parentRelativeType;

  FloatMeta                 picScale;

  RotateIconItem(
    Page          *_page,
    InsertMeta     _insMeta,
    QGraphicsItem  *parent = 0);

  ~RotateIconItem()
  {

  }
protected:
  virtual void change();
  void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
};


#endif // ROTATEICONITEM_H
