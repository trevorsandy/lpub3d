 
/****************************************************************************
**
** Copyright (C) 2007-2009 Kevin Clague. All rights reserved.
** Copyright (C) 2015 - 2024 Trevor SANDY. All rights reserved.
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
 * This derived class allows the user to create empty spaces within
 * callouts and multi-steps.
 *
 * Please see lpub.h for an overall description of how the files in LPub3D
 * make up the LPub3D program.
 *
 ***************************************************************************/

#ifndef reserveH
#define reserveH

#include "range_element.h"

class Where;
class LPubMeta;
class ReserveBackgroundItem;
class Reserve : public AbstractRangeElement {
  public:
    Where here;
    AllocEnc alloc;
    ReserveBackgroundItem *background;

    Reserve(
      Where    &_here,
      LPubMeta &meta);

    virtual void addGraphicsItems(
      int ox,
      int oy,
      int sx,
      int sy,
      PlacementType parentRelativeType,
      QGraphicsItem *);
};

class ReserveBackgroundItem : public QGraphicsRectItem, public MetaItem
{
  Reserve *reserve;

public:
  ReserveBackgroundItem(
    Reserve       *reserve,
    QGraphicsItem *parent);
  Where top;

protected:
  virtual void hoverEnterEvent(QGraphicsSceneHoverEvent* event);
  virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent* event);
  virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
  virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
  virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
  bool isHovered;
  bool mouseIsDown;
};

#endif
