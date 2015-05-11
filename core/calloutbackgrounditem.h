 
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

/****************************************************************************
 *
 * The class described in this file is a refined version of the background
 * graphics item class that is used specifically for callouts.
 *
 * Please see lpub.h for an overall description of how the files in LPub
 * make up the LPub program.
 *
 ***************************************************************************/

#ifndef CALLOUTBACKTROUNDH
#define CALLOUTBACKTROUNDH

#include "backgrounditem.h"

class Callout;

class CalloutBackgroundItem : public PlacementBackgroundItem
{
public:
  Callout            *callout;
  CalloutMeta         calloutMeta;
  QRect               calloutRect;
  QRect               csiRect;
  BoolMeta           *perStep;
  AllocMeta          *alloc;
  PageMeta           *page;

  QGraphicsView      *view;
  QGraphicsTextItem  *cursor;

  CalloutBackgroundItem(
    Callout       *_callout,
    QRect         &_calloutRect,
    QRect         &_csiRect,
    PlacementType  parentRelativeType,
    Meta          *meta,
    int            submodelLevel,
    QString        _path,
    QGraphicsItem *parent,
    QGraphicsView *_view);

  void setPos(float x, float y)
  {
    QGraphicsPixmapItem::setPos(x,y);
  }

protected:
  void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
  void mousePressEvent(QGraphicsSceneMouseEvent *event);
  void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
  void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
  
};
#endif
