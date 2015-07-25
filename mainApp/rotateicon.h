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

#ifndef ROTATEICON_H
#define ROTATEICON_H

#include <QGraphicsScene>
#include <QGraphicsItem>
#include <QGraphicsSceneMouseEvent>
#include "backgrounditem.h"
#include "placement.h"
#include "metaitem.h"
#include "meta.h"

#include "QsLog.h"


class RotateIconBackgroundItem;
class RotateIcon : public Placement
{
public:
  Meta                     *meta;
  InsertMeta                insMeta;
  RotateIconMeta            rotateIconMeta;
  PlacementType             parentRelativeType;
  bool                      positionChanged;
  QPointF                   position;
  RotateIconBackgroundItem *background;
  //RotateIconArrowsItem     *rotateArrows;

  RotateIcon(/*Meta          *meta,
             InsertMeta     insMeta,
             PlacementType  parentRelativeType,
             QGraphicsItem *parent*/); /*:
    meta(meta),
    insMeta(insMeta),
    rotateIconMeta(meta->LPub.rotateIconMeta),
    parentRelativeType(parentRelativeType)
  {
    addBackground(0,parent);
  } */
  ~RotateIcon();

  int  addBackground (int, QGraphicsItem *);
};

class RotateIcon;
class RotateIconBackgroundItem : public BackgroundItem, public Placement
{
public:
  RotateIcon    *rotateIcon;
  PlacementType  parentRelativeType;
  bool           positionChanged;
  QPointF        position;

  RotateIconBackgroundItem(
    RotateIcon   *_rotateIcon,
    int            width,
    int            height,
    PlacementType _parentRelativeType,
    int            submodelLevel,
    QGraphicsItem *parent);

  void setPos(float x, float y)
  {
    QGraphicsPixmapItem::setPos(x,y);
  }
  void setFlag(GraphicsItemFlag flag, bool value)
  {
    QGraphicsItem::setFlag(flag,value);
  }

  virtual QGraphicsItem *myParentItem()
  {
    return parentItem();
  }

protected:
  virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
  virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
  virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
  void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);

};
#endif // ROTATEICON_H
