 
/****************************************************************************
**
** Copyright (C) 2007-2009 Kevin Clague. All rights reserved.
** Copyright (C) 2015 - 2018 Trevor SANDY. All rights reserved.
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
  setToolTip("Step Group - right-click to modify");
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
  background        = nullptr;
  border            = nullptr;
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
  setToolTip("Step Group - right-click to modify");
  setParentItem(parent);
  setZValue(-2);
}

void MultiStepRangeBackgroundItem::contextMenuEvent(
  QGraphicsSceneContextMenuEvent *event)
{
  QMenu menu;
  PlacementData placementData = meta->LPub.multiStep.placement.value();
  QString name = "Steps";

  QAction *placementAction;
  placementAction = commonMenus.placementMenu(menu,name,
                                              commonMenus.naturalLanguagePlacementWhatsThis(StepGroupType,placementData,name));

  QAction *perStepAction;
  if (meta->LPub.multiStep.pli.perStep.value()) {
    perStepAction = commonMenus.noPartsList(menu,"Step");
  } else {
    perStepAction = commonMenus.partsList(menu,"Step");
  }

  QAction *allocAction;
  if (page->allocType() == Vertical) {
    allocAction = commonMenus.displayRowsMenu(menu,name);
  } else {
    allocAction = commonMenus.displayColumnsMenu(menu, name);
  }

  QAction *marginAction;
  marginAction = commonMenus.marginMenu(menu,name);

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

/******************************************************************************
 * Divider pointer item routines
 *****************************************************************************/

#include "dividerpointeritem.h"

DividerItem::DividerItem(
  Step       *_step,
  Meta       *_meta,
  int         _offsetX,
  int         _offsetY)
{
  meta               = *_meta;
  parentStep         =  _step;
  Range  *range      =  _step->range();
  Steps  *steps      =  _step->grandparent();
  parentRelativeType =  steps->relativeType;
  placement          =  steps->placement;
  SepData sepData    =  range->sepMeta.valuePixels();

  AllocEnc allocEnc;
  if (parentRelativeType == CalloutType) {
    allocEnc = meta.LPub.callout.alloc.value();
  } else {
    allocEnc = meta.LPub.multiStep.alloc.value();
  }

  /* Size the divider length */

  int separatorLength = sepData.length;
  if (sepData.type != SepData::LenCustom) {
    if(parentStep->dividerType == StepDivider) { // Step divider
      separatorLength = allocEnc == Vertical ? range->size[XX] : range->size[YY];
    } else {                                    // Range divider
      separatorLength = allocEnc == Vertical ? range->size[YY] : range->size[XX];
    }
  }

  /* Size the rectangle around the divider */

  if(parentStep->dividerType == StepDivider) { // Step divider
      if (allocEnc == Vertical) {
          setRect(_offsetX,
                  _offsetY,
                  separatorLength,
                  2*sepData.margin[YY]+sepData.thickness);
      } else {
          setRect(_offsetX,
                  _offsetY,
                  2*sepData.margin[XX]+sepData.thickness,
                  separatorLength);
      }
  } else {                        // Range divider
      if (allocEnc == Vertical) {
        setRect(_offsetX,
                _offsetY,
                2*sepData.margin[XX]+sepData.thickness,
                separatorLength);
      } else {
        setRect(_offsetX,
                _offsetY,
                separatorLength,
                2*sepData.margin[YY]+sepData.thickness);
      }
  }

  setPen(QPen(Qt::NoPen));
  setBrush(QBrush(Qt::NoBrush));
  setToolTip("Divider - right-click to modify");
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
    // determine the position of the divider
    if(parentStep->dividerType == StepDivider) {     // Step divider
        if (allocEnc == Vertical) {
            int separatorWidth = sepData.margin[XX]+sepData.thickness/2;
            int spacingHeight  = (sepData.margin[YY]+sepData.thickness+range->stepSpacing)/2;
            lineItem->setLine(_offsetX+separatorWidth,
                              _offsetY-spacingHeight,    // top
                              _offsetX+separatorLength+separatorWidth,
                              _offsetY-spacingHeight);   // top
        } else {
            int separatorHeight = sepData.margin[YY]+sepData.thickness/2;
            int spacingWidth    = (sepData.margin[XX]+sepData.thickness+range->stepSpacing)/2;
            lineItem->setLine(_offsetX+spacingWidth,     // left
                              _offsetY-separatorHeight,
                              _offsetX+spacingWidth,     // left
                              _offsetY+separatorLength-separatorHeight);
        }
    } else {                            // Range divider
        if (allocEnc == Vertical) {
          int separatorWidth = sepData.margin[XX]+sepData.thickness/2;
          int spacingHeight  = (sepData.margin[YY]+sepData.thickness+range->stepSpacing)/2;
          lineItem->setLine(_offsetX+separatorWidth,   // right
                            _offsetY-spacingHeight,
                            _offsetX+separatorWidth,   // right
                            _offsetY+separatorLength-spacingHeight);
        } else {
          int separatorHeight = sepData.margin[YY]+sepData.thickness/2;
          int spacingWidth    = (sepData.margin[XX]+sepData.thickness+range->stepSpacing)/2;
          lineItem->setLine(_offsetX-spacingWidth,
                            _offsetY+separatorHeight,   // top
                            _offsetX+separatorLength-spacingWidth,
                            _offsetY+separatorHeight);  // top
        }
    }

    loc[XX]  = lineItem->boundingRect().x();
    loc[YY]  = lineItem->boundingRect().y();
    size[XX] = lineItem->boundingRect().size().width();
    size[YY] = lineItem->boundingRect().size().height();

    QPen pen(LDrawColor::color(sepData.color));
    pen.setWidth(sepData.thickness);
    pen.setCapStyle(Qt::RoundCap);

    lineItem->setPen(pen);
    lineItem->setZValue(100);
    setZValue(99);
  }
}

DividerItem::~DividerItem()
{
  graphicsPointerList.clear();
};

void DividerItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
  QMenu menu;
  QAction *editAction;

  editAction = menu.addAction("Edit Divider");
  editAction->setIcon(QIcon(":/resources/editdivider.png"));
  editAction->setWhatsThis("Edit this divider margin, thickness and color");

  QAction *deleteAction;

  deleteAction = menu.addAction("Delete Divider");
  deleteAction->setIcon(QIcon(":/resources/deletedivider.png"));
  deleteAction->setWhatsThis("Delete this divider from the model");
  
  QAction *selectedAction = menu.exec(event->screenPos());

  if (selectedAction == nullptr) {
    return;
  }
  
  Step *nextStep = nullptr;
  nextStep = parentStep->nextStep();
  if ( ! nextStep)
      return;
  Where topOfStep    = nextStep->topOfStep();
  Where bottomOfStep = nextStep->bottomOfStep();
  Range *range       = parentStep->range();

  if (selectedAction == editAction) {

    changeDivider("Divider",topOfStep,bottomOfStep,&range->sepMeta,1,false);

  } else if (selectedAction == deleteAction) {

    enum Type { Attribute, MetaCmd };

    Where undefined;
    QString modelName;
    QList<int> lineNumbers;
    QHash<int, Type> hash;

    beginMacro("deleteDividerMetas");

    // first we delete any divider pointers
    bool stepList = parentStep->dividerType == StepDivider;
    int listSize  = stepList ? range->stepDividerPointerList.size() :
                               range->rangeDividerPointerList.size();
    // capture list of pointers and their attirbutes if any
    for (int j = 0; j < listSize; j++) {
      Pointer *pointer        = stepList ? range->stepDividerPointerList[j] :
                                           range->rangeDividerPointerList[j];
      if (pointer->stepNum   == parentStep->stepNumber.number) {
        modelName             = pointer->here.modelName;
        PointerAttribData pad = pointer->pointerAttrib.value();
        Where lineAttribTop   = Where(pad.lineWhere.modelName,pad.lineWhere.lineNumber);
        Where borderAttribTop = Where(pad.borderWhere.modelName,pad.borderWhere.lineNumber);
        // for each pointer load the pointer...
        hash.insert(pointer->here.lineNumber,MetaCmd);
        lineNumbers << pointer->here.lineNumber;
        // load pointer attributes if any...
        if (lineAttribTop != undefined){
            hash.insert(lineAttribTop.lineNumber,Attribute);
            lineNumbers << lineAttribTop.lineNumber;
        }
        if (borderAttribTop != undefined){
            hash.insert(borderAttribTop.lineNumber,Attribute);
            lineNumbers << borderAttribTop.lineNumber;
        }
      }
    }
    // sort the line numbers...
    qSort(lineNumbers.begin(),lineNumbers.end());
    // process from last to first to preserve line numbers
    int lastLineNumber = -1;
    for (int k = lineNumbers.size() - 1; k >= 0; --k) {
      int lineNumber  = lineNumbers[k];
      if (lineNumber != lastLineNumber) {
        Where here(modelName,lineNumber);
        QHash<int, Type>::iterator i = hash.find(lineNumber);
        while (i != hash.end() && i.key() == lineNumber) {
          if (i.value() == Attribute)
            deletePointerAttribute(here,true);
          else
            deleteMeta(here);
          ++i;
        }
        lastLineNumber = lineNumber;
      }
    }
    // delete divider
    deleteDivider(parentRelativeType,topOfStep);
    endMacro();
  }
}

void DividerItem::addGraphicsPointerItem(Pointer *pointer, QGraphicsView *view)
{
    // This is the background for the dividerPointer line.
    QRect dividerRect(loc[XX],loc[YY],size[XX],size[YY]);
    background = new DividerBackgroundItem (
                &meta,
                dividerRect,
                parentItem());
    background->setPos(loc[XX],loc[YY]);
    background->setFlag(QGraphicsItem::ItemIsMovable, false);
    background->setFlag(QGraphicsItem::ItemIsSelectable, false);

    DividerPointerItem *pi =
            new DividerPointerItem(
                this,
                pointer,
                background,
                view);
    graphicsPointerList.append(pi);
}

void DividerItem::updatePointers(QPoint &delta)
{
  for (int i = 0; i < graphicsPointerList.size(); i++) {
    DividerPointerItem *pointerItem = graphicsPointerList[i];
    pointerItem->updatePointer(delta);
  }
}

void DividerItem::drawTips(QPoint &delta)
{
  for (int i = 0; i < graphicsPointerList.size(); i++) {
    DividerPointerItem *pointerItem = graphicsPointerList[i];
    pointerItem->drawTip(delta);
  }
}

/******************************************************************************
 * Divider line routines
 *****************************************************************************/

void DividerLine::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
  DividerItem *dividerItem = dynamic_cast<DividerItem *>(parentItem());
  dividerItem->contextMenuEvent(event);
}

/******************************************************************************
 * Divider background item routine
 *****************************************************************************/

DividerBackgroundItem::DividerBackgroundItem(
  Meta          *_meta,
  QRect         &_dividerRect,
  QGraphicsItem *parent)
{
  border = &_meta->LPub.pointerBase.border;

  setRect(0,0,_dividerRect.width(),_dividerRect.height());
  setToolTip(QString());
  setPen(QPen(Qt::NoPen));
  setBrush(QBrush(Qt::NoBrush));
  setParentItem(parent);
  setZValue(98);
}
