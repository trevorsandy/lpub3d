 
/****************************************************************************
**
** Copyright (C) 2007-2009 Kevin Clague. All rights reserved.
** Copyright (C) 2015 Trevor SANDY. All rights reserved.
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

#include <QGraphicsItem>

#include "ranges.h"
#include "ranges_item.h"
#include "range.h"
#include "step.h"
#include "color.h"
#include "commonmenus.h"

/****************************************************************************
 *
 * This is the graphical representation of step groups.
 *
 * Please see lpub.h for an overall description of how the files in LPub
 * make up the LPub program.
 *
 ***************************************************************************/

MultiStepRangesBackgroundItem::MultiStepRangesBackgroundItem(
  Steps *_steps,
  QRectF rect, 
  QGraphicsItem *parent, 
  Meta *_meta)
{
  meta = _meta;
  //page = dynamic_cast<Page *>(_steps);
  page = _steps;
  setRect(rect);
  setPen(Qt::NoPen);
  setBrush(Qt::NoBrush);
  setParentItem(parent);
  setToolTip("Step Group - popup menu");
  setFlag(QGraphicsItem::ItemIsSelectable,true);
  setFlag(QGraphicsItem::ItemIsMovable,true);
  setZValue(-2);
}

void MultiStepRangesBackgroundItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
  QGraphicsItem::mousePressEvent(event);
  positionChanged = false;
  position = pos();
}

void MultiStepRangesBackgroundItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
  QGraphicsRectItem::mouseMoveEvent(event);
  positionChanged = true;
}

void MultiStepRangesBackgroundItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
  QGraphicsItem::mouseReleaseEvent(event);

  if (isSelected() && (flags() & QGraphicsItem::ItemIsMovable) && positionChanged) {
    QPointF newPosition;
    // back annotate the movement of the PLI into the LDraw file.
    newPosition = pos() - position;
    PlacementData placementData = meta->LPub.multiStep.placement.value();
    placementData.offsets[0] += newPosition.x()/page->relativeToSize[0];
    placementData.offsets[1] += newPosition.y()/page->relativeToSize[1];
    meta->LPub.multiStep.placement.setValue(placementData);

    changePlacementOffset(page->topOfSteps(),&meta->LPub.multiStep.placement,StepGroupType,true,false);
  }
}

MultiStepRangeBackgroundItem::MultiStepRangeBackgroundItem(
  Steps  *_steps,
  Range  *_range,
  Meta   *_meta,
  int     _offset_x,
  int     _offset_y,
  QGraphicsItem *parent)
{
  meta = _meta;
  page = _steps;

  MultiStepMeta *multiStep = &_meta->LPub.multiStep;
  background        = NULL;
  border            = NULL;
  margin            = &multiStep->margin; 
  placement         = &multiStep->placement;
  freeform          = &multiStep->freeform;
  alloc             = &multiStep->alloc;
  subModelFont      = &multiStep->subModelFont;
  subModelFontColor = &multiStep->subModelFontColor;
  perStep           = &multiStep->pli.perStep;
  relativeType      = _steps->relativeType;
  int tx = _offset_x+_range->loc[XX];
  int ty = _offset_y+_range->loc[YY];
  setRect(tx,ty, _steps->size[XX], _steps->size[YY]);

  setPen(QPen(Qt::NoPen));
  setBrush(QBrush(Qt::NoBrush));
  setToolTip("Step Group - popup menu");
  setParentItem(parent);
  setZValue(-2);
}

void MultiStepRangeBackgroundItem::contextMenuEvent(
  QGraphicsSceneContextMenuEvent *event)
{
  QMenu menu;

  PlacementData placementData = meta->LPub.multiStep.placement.value();

  QString name = "Move These Steps";
  QAction *placementAction = menu.addAction(name);
  placementAction->setWhatsThis(commonMenus.naturalLanguagePlacementWhatsThis(
    StepGroupType,placementData,name));

  QAction *perStepAction = menu.addAction(
    meta->LPub.multiStep.pli.perStep.value() ?
      "No Parts List per Step" :
      "Parts List per Step");

  QAction *allocAction;
  
  if (page->allocType() == Vertical) {
    allocAction = menu.addAction("Display as Rows");
    allocAction->setWhatsThis(
      "Display as Rows:\n"
      "  Change this whole set of steps from columns of steps\n"
      "  to rows of steps");
  } else {
    allocAction = menu.addAction("Display as Columns");
    allocAction->setWhatsThis(
      "Display as Columns:\n"
      "  Change this whole set of steps from rows of steps\n"
      "  to columns of steps");
  }

  QAction *marginAction = menu.addAction("Change Step Group Margins");

  marginAction->setWhatsThis("Change Step Group Margins:\n"
                             "  Margins are the empty space around this group of steps.\n"
                             "  You can change the margins if things are too close together,\n"
                             "  or too far apart. You can change the things in this group of\n"
                             "  steps by putting your mouse cursor over them, and clicking\n"
                             "  the menu button\n");

  QAction *selectedAction = menu.exec(event->screenPos());

  if (selectedAction == placementAction) {
    changePlacement(PageType,
                    relativeType,
                    "Step Group Placement",
                    page->topOfSteps(),
                    page->bottomOfSteps(),
                    &meta->LPub.multiStep.placement,
                    true,1,false);
  } else if (selectedAction == perStepAction) {
    changeBool(page->topOfSteps(),
               page->bottomOfSteps(),
              &meta->LPub.multiStep.pli.perStep,true,1,false,false);
  } else if (selectedAction == marginAction) {
    changeMargins("Step Group Margins",
                  page->topOfSteps(),
                  page->bottomOfSteps(),
                  margin);
  } else if (selectedAction == allocAction) {
    changeAlloc(page->topOfSteps(),
                page->bottomOfSteps(),
                page->allocMeta());
  }
}

DividerItem::DividerItem(
  Step  *_step,
  Meta  *_meta,
  int    offsetX,
  int    offsetY)
{
  step = _step;
  AllocEnc allocEnc;
  Steps  *steps = step->grandparent();
  Range  *range = step->range();
  parentRelativeType = steps->relativeType;
  
  if (parentRelativeType == CalloutType) {
    allocEnc = steps->meta.LPub.callout.alloc.value();
  } else {
    allocEnc = steps->meta.LPub.multiStep.alloc.value();
  }

  SepData sepData = range->sepMeta.valuePixels();

  /* Size the rectangle around the divider */

  if (allocEnc == Vertical) {
    setRect(offsetX,
            offsetY,
           2*sepData.margin[XX]
           +sepData.thickness,
            range->size[YY]);
  } else {
    setRect(offsetX,
            offsetY,
            range->size[XX],
           2*sepData.margin[YY]
           +sepData.thickness);
  }

  setPen(QPen(Qt::NoPen));
  setBrush(QBrush(Qt::NoBrush));
  setToolTip("Divider - popup menu");
  lineItem = new DividerLine(this);

  BorderData borderData;

  if (steps->relativeType == CalloutType) {
    borderData = _meta->LPub.callout.border.valuePixels();
  } else {
    borderData.margin[0] = 0;
    borderData.margin[1] = 0;
    borderData.thickness = 0;
  }

  if (sepData.thickness > 0.5) {
    if (allocEnc == Vertical) {

      int left = offsetX+sepData.margin[XX]+sepData.thickness/2;

      lineItem->setLine(left,
                        offsetY,
                        left,
                        offsetY
                       +range->size[YY]);
    } else {
      int top = offsetY+sepData.margin[YY]+sepData.thickness/2;
  
      lineItem->setLine(offsetX,
                        top,
                        offsetX
                       +range->size[XX],
                        top);
    }
    QPen pen(LDrawColor::color(sepData.color));
    pen.setWidth(sepData.thickness);
    pen.setCapStyle(Qt::RoundCap);

    lineItem->setPen(pen);
    lineItem->setZValue(100);
    setZValue(99);
  }
}

void DividerItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
  QMenu menu;
  QAction *editAction     = menu.addAction("Change Divider");
  QAction *deleteAction   = menu.addAction("Delete Divider");

  editAction->setWhatsThis("Put the steps in this row together with the steps after this");
  
  QAction *selectedAction = menu.exec(event->screenPos());

  if (selectedAction == NULL) {
    return;
  }
  
  Step *nextStep = step->nextStep();
  if ( ! nextStep) {
    return;
  }
  
  Where topOfStep    = nextStep->topOfStep();
  Where bottomOfStep = nextStep->bottomOfStep();
  Range *range = step->range();

  if (selectedAction == editAction) {
    changeDivider("Divider",topOfStep,bottomOfStep,&range->sepMeta,1,false);
  } else if (selectedAction == deleteAction) {
    deleteDivider(parentRelativeType,topOfStep);
  }
}
void DividerLine::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
  DividerItem *dividerItem = dynamic_cast<DividerItem *>(parentItem());
  dividerItem->contextMenuEvent(event);
}
