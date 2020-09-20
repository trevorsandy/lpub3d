
/****************************************************************************
**
** Copyright (C) 2007-2009 Kevin Clague. All rights reserved.
** Copyright (C) 2016 - 2019 Trevor SANDY. All rights reserved.
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
  Meta           *_meta,
  QGraphicsView  *_view)
{
  relativeType  = PagePointerType;
  meta          = *_meta;
  view          =  _view;
  margin.setValues(0,0);
}

PagePointer::~PagePointer()
{
  for (int i = 0; i < pointerList.size(); i++) {
    Pointer *p = pointerList[i];
    delete p;
  }
  pointerList.clear();
}


void PagePointer::appendPointer(
  const Where       &here,
  PointerMeta       &pointerMeta,
  PointerAttribMeta &pointerAttrib)
{
  int pid = pointerList.size() + 1;
  PointerAttribMeta pam = pointerAttrib;
  Pointer *pointer = new Pointer(pid,here,pointerMeta);
  pam.setDefaultColor(meta.LPub.page.border.value().color);
  pointer->setPointerAttribInches(pam);
  pointerList.append(pointer);
}

void PagePointer::sizeIt()
{
  int dim = 1;

  BorderData borderData = meta.LPub.pointerBase.border.valuePixels();

  size[XX] += int(borderData.margin[XX]);
  size[YY] += int(borderData.margin[YY]);

  size[XX] += int(borderData.thickness*0);
  size[YY] += int(borderData.thickness*0);

  if (placement.value().placement == Top ||
      placement.value().placement == Bottom) {
      size[XX] += gui->pageSize(meta.LPub.page, 0);
      size[YY] += dim;
  } else {
      size[YY] += gui->pageSize(meta.LPub.page, 1);
      size[XX] += dim;
  }
}

// If we make the pointer start at the center of the corner rather than the edge,
// then the pagePointer hides the starting point of the pointer, and the
// pointer always appears to start right at the edge of the pagepointer
// (no matter the shape of the corner's curve.
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

  // This is the pagePointer origin rectangle - a tiny rect
  // to anchor the pointer on the choosen perimeter of the page

  QRect pagePointerRect(newLoc[XX],newLoc[YY],size[XX],size[YY]);

  // This is the background for the pagePointer origin rectangle.

  background = new PagePointerBackgroundItem(
                     pagePointerRect,
                     parentRelativeType,
                    &meta,
                     parent);
  background->setPos(newLoc[XX],newLoc[YY]);
  background->setFlag(QGraphicsItem::ItemIsMovable, movable);
  background->setFlag(QGraphicsItem::ItemIsSelectable, movable);
}

void PagePointer::addGraphicsPointerItem(
  Pointer *pointer)
{
  PagePointerItem *t =
    new PagePointerItem(
          this,
          pointer,
          background,
          view);
  graphicsPagePointerList.append(t);
}

void PagePointer::updatePointers(QPoint &delta, QGraphicsItem *target)
{
  for (int i = 0; i < graphicsPagePointerList.size(); i++) {
    PagePointerItem *pointerItem = graphicsPagePointerList[i];
    foreach (QGraphicsItem *item, pointerItem->collidingItems(Qt::IntersectsItemBoundingRect)) {
        if (item == target)
            pointerItem->updatePointer(delta);
    }
  }
}

void PagePointer::drawTips(QPoint &delta, QGraphicsItem *target, int type)
{
  for (int i = 0; i < graphicsPagePointerList.size(); i++) {
    PagePointerItem *pointerItem = graphicsPagePointerList[i];
    int initiator = type ? type : PageType;
    foreach (QGraphicsItem *item, pointerItem->collidingItems(Qt::IntersectsItemBoundingRect)) {
        if (item == target)
            pointerItem->drawTip(delta,initiator);
    }
  }
}
