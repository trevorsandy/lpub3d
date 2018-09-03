 
/****************************************************************************
**
** Copyright (C) 2007-2009 Kevin Clague. All rights reserved.
** Copyright (C) 2016 - 2018 Trevor SANDY. All rights reserved.
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
 * graphics item class that is used specifically for pages.
 *
 * Please see lpub.h for an overall description of how the files in LPub
 * make up the LPub program.
 *
 ***************************************************************************/

#include <QtWidgets>
#include "pagepointerbackgrounditem.h"
#include <QGraphicsView>

PagePointerBackgroundItem::PagePointerBackgroundItem(
  QRect         &_pagePointerRect,
  PlacementType  _parentRelativeType,
  Meta          *_meta,
  QGraphicsItem *parent)
{
  meta               = _meta;
  parentRelativeType = _parentRelativeType;
  pagePointerRect    = _pagePointerRect;
  background         = &_meta->LPub.pagePointer.background;
  border             = &_meta->LPub.pagePointer.border;
  placement          = &_meta->LPub.pagePointer.placement;
  margin             = &_meta->LPub.pagePointer.margin;

  setRect(0,0,_pagePointerRect.width(),_pagePointerRect.height());
  setToolTip("");
  setPen(QPen(Qt::NoPen));
  setBrush(QBrush(Qt::NoBrush));
  setParentItem(parent);
  setZValue(98);
  setFlag(QGraphicsItem::ItemIsSelectable,false);
  setFlag(QGraphicsItem::ItemIsMovable,false);
}

