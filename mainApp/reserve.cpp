/****************************************************************************
**
** Copyright (C) 2020 Trevor SANDY. All rights reserved.
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
 * Please see lpub.h for an overall description of how the files in LPub
 * make up the LPub program.
 *
 ***************************************************************************/

#include "meta.h"
#include "where.h"
#include "range_element.h"
#include "lpub.h"
#include "reserve.h"

Reserve::Reserve(
  Where    &_here,
  LPubMeta &meta)
{
  top  = _here;
  top.setModelIndex(gui->getSubmodelIndex(_here.modelName));

  alloc = meta.multiStep.alloc.value();

  float space = meta.reserve.value();

  relativeType = ReserveType;

  if (alloc == Horizontal) {
    setSize(int(gui->pageSize(meta.page, 0) * space + 0.5),0);
  } else {
    setSize(0,int(gui->pageSize(meta.page, 1) * space + 0.5));
  }
}

void Reserve::addGraphicsItems(
    int             offsetX,
    int             offsetY,
    int             sizeX,
    int             sizeY,
    PlacementType   parentRelativeType,
    QGraphicsItem  *parent)
{
  offsetX += loc[XX];
  offsetY += loc[YY];
  if (alloc == Horizontal) {
    setSize(size[XX],sizeY);
  } else {
    setSize(sizeX,size[YY]);
  }
  if (parentRelativeType == StepGroupType || parentRelativeType == CalloutType) {
    background = new ReserveBackgroundItem(this, parent);
    background->setPos(offsetX, offsetY);
  }
}

ReserveBackgroundItem::ReserveBackgroundItem(
  Reserve       *_reserve,
  QGraphicsItem *parent)
{
  reserve       = _reserve;
  top           = _reserve->top;

  setRect(0,0,reserve->size[XX],reserve->size[YY]);
  setPen(Qt::NoPen);
  setBrush(Qt::NoBrush);
  setParentItem(parent);
  setToolTip(QString("Reserve Rectangle [%1 x %2 px]")
             .arg(boundingRect().width())
             .arg(boundingRect().height()));
  setFlag(QGraphicsItem::ItemIsMovable,false);
  setData(ObjectId, StepBackgroundObj);
  setZValue(RESERVE_BACKGROUND_ZVALUE_DEFAULT);
}
