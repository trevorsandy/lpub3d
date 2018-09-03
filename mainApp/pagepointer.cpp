
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
 * This file describes the data structure that represents an LPub pagepointer
 * (the steps of a submodel packed together and displayed next to the
 * assembly where the submodel is used).
 *
 * Please see lpub.h for an overall description of how the files in LPub
 * make up the LPub program.
 *
 ***************************************************************************/

#include "lpub.h"
#include <QtWidgets>
#include "pagepointer.h"
#include "pointer.h"
#include "pagepointeritem.h"
#include "pagepointerbackgrounditem.h"
#include "numberitem.h"
#include "ranges.h"
#include "range.h"
#include "step.h"
#include "placementdialog.h"
#include "commonmenus.h"
#include "paths.h"
#include "render.h"

//---------------------------------------------------------------------------

PagePointer::PagePointer(
  Meta                 &_meta,
  QGraphicsView        *view)
  : view(view)
{
  relativeType  = PagePointerType;
  meta = _meta;
}

PagePointer::~PagePointer()
{
  pointerList.clear();
}


void PagePointer::appendPointer(const Where &here, PointerMeta &pointerMeta)
{
  Pointer *pointer = new Pointer(here,pointerMeta);
  pointerList.append(pointer);
}

void PagePointer::sizeIt()
{
  int dim = 5;

  BorderData borderData = meta.LPub.pagePointer.border.valuePixels();

  size[XX] += int(borderData.margin[XX]);
  size[YY] += int(borderData.margin[YY]);

  size[XX] += int(borderData.thickness*0);
  size[YY] += int(borderData.thickness*0);

  margin.setValues(0,0);

  if (placement.value().placement == Top ||
      placement.value().placement == Bottom) {
      size[XX] += gui->pageSize(meta.LPub.page, 0);
      size[YY] += dim;
    } else {
      size[XX] += dim;
      size[YY] += gui->pageSize(meta.LPub.page, 1);
    }
}

// PagePointers that have round corners are tricky, trying to get the pointer to start/end on the
// rounded corner.  To avoid trying to know the shape of the curve, we make sure the pointer
// is below (think zDepth) the pagepointer.  If we make the pointer start at the center of the curved
// corner rather than the edge, then the pagepointer hides the starting point of the arrow, and the
// arrow always appears to start right at the edge of the pagepointer (no matter the shape of the
// corner's curve.
void PagePointer::addGraphicsItems(
  int            offsetX,
  int            offsetY,
  QGraphicsItem *parent,
  bool           movable)
{
  PlacementData placementData = placement.value();
  if (placementData.relativeTo == PageType ||
      placementData.relativeTo == StepGroupType ||
      placementData.relativeTo == CalloutType) {
    offsetX = 0;
    offsetY = 0;
  }
  int newLoc[2] = { offsetX + loc[XX], offsetY + loc[YY] };
  QRect pagePointerRect(newLoc[XX],newLoc[YY],size[XX],size[YY]);

  // This is the background for the entire pagePointer.

  background = new PagePointerBackgroundItem(
                     pagePointerRect,
                     parentRelativeType,
                    &meta,
                     parent);
  background->setPos(newLoc[XX],newLoc[YY]);
  background->setFlag(QGraphicsItem::ItemIsMovable, movable);
}

void PagePointer::addGraphicsPointerItem(
  Pointer *pointer)
{
  PagePointerItem *t =
    new PagePointerItem(
          this,
         &meta,
          pointer,
          background,
          view);
  graphicsPointerList.append(t);
}

void PagePointer::updatePointers(QPoint &delta)
{
  for (int i = 0; i < graphicsPointerList.size(); i++) {
    PagePointerItem *pointer = graphicsPointerList[i];
    pointer->updatePointer(delta);
  }
}

void PagePointer::drawTips(QPoint &delta)
{
  for (int i = 0; i < graphicsPointerList.size(); i++) {
    PagePointerItem *pointer = graphicsPointerList[i];
    pointer->drawTip(delta);
  }
}



