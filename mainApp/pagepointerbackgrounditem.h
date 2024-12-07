 
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
 * graphics item class that is used specifically for pages.
 *
 * Please see lpub.h for an overall description of how the files in LPub3D
 * make up the LPub3D program.
 *
 ***************************************************************************/

#ifndef PAGEPOINTERBACKTROUNDH
#define PAGEPOINTERBACKTROUNDH

#include "backgrounditem.h"

class PagePointerBackgroundItem : public QGraphicsRectItem, public MetaItem /* public PlacementBackgroundItem */
{
public:
  PlacementType   parentRelativeType;
  QRect           pagePointerRect;
  Meta           *meta;
  BackgroundMeta *background;
  BorderMeta     *border;
  PlacementMeta  *placement;
  MarginsMeta    *margin;

  PagePointerBackgroundItem(
    QRect        &_pagePointerRect,
    PlacementType  parentRelativeType,
    Meta          *meta,
    QGraphicsItem *parent);

  void setPos(float x, float y)
  {
    QGraphicsRectItem::setPos(x,y);
  }
};
#endif
