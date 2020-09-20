
/****************************************************************************
**
** Copyright (C) 2007-2009 Kevin Clague. All rights reserved.
** Copyright (C) 2015 - 2019 Trevor SANDY. All rights reserved.
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
#include "lpub.h"

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
  Meta *_meta):
    isHovered(false),
    mouseIsDown(false)
{
  meta = _meta;
  //page = dynamic_cast<Page *>(_steps);
  page = _steps;
  setRect(rect);
  setPen(Qt::NoPen);
  setBrush(Qt::NoBrush);
  setParentItem(parent);
  setToolTip(QString("Steps Group [%1 x %2 px] - right-click to modify")
             .arg(boundingRect().width())
             .arg(boundingRect().height()));
  setFlag(QGraphicsItem::ItemIsSelectable,true);
  setFlag(QGraphicsItem::ItemIsFocusable, true);
  setAcceptHoverEvents(true);
  setFlag(QGraphicsItem::ItemIsMovable,true);
  setData(ObjectId, MultiStepsBackgroundObj);
  setZValue(MULTISTEPSBACKGROUND_ZVALUE_DEFAULT);
}

void MultiStepRangesBackgroundItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
  QGraphicsItem::mousePressEvent(event);
  mouseIsDown = true;
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
  mouseIsDown = false;
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

void MultiStepRangesBackgroundItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    isHovered = !this->isSelected() && !mouseIsDown;
    QGraphicsItem::hoverEnterEvent(event);
}

void MultiStepRangesBackgroundItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    isHovered = false;
    QGraphicsItem::hoverLeaveEvent(event);
}

void MultiStepRangesBackgroundItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QPen pen;
    pen.setColor(isHovered ? QColor(Preferences::sceneGuideColor) : Qt::black);
    pen.setWidth(0/*cosmetic*/);
    pen.setStyle(isHovered ? Qt::PenStyle(Preferences::sceneGuidesLine) : Qt::NoPen);
    painter->setPen(pen);
    painter->setBrush(Qt::transparent);
    painter->drawRect(this->boundingRect());
    QGraphicsRectItem::paint(painter,option,widget);
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
  setToolTip(QString("Step Group [%1 x %2 px] - right-click to modify")
             .arg(boundingRect().width())
             .arg(boundingRect().height()));
  setParentItem(parent);
  setData(ObjectId, MultiStepBackgroundObj);
  setZValue(MULTISTEPBACKGROUND_ZVALUE_DEFAULT);
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

  if ( ! selectedAction ) {
      return;
  }

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
  int         _offsetY):
    isHovered(false),
    mouseIsDown(false)
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

  QString dividerType = QString("%1").arg(parentStep->dividerType == StepDivider ? "Step" : "Range");
  int separatorLength, separatorWidthX, separatorHeightY, spacingWidthX, spacingHeightY;
  if(parentStep->dividerType == StepDivider) {// Step divider
    if (sepData.type != SepData::Default)
      separatorLength = int(sepData.length);
    else
      separatorLength = allocEnc == Vertical ? range->size[XX] : range->size[YY];
  } else {                                    // Range divider
    separatorLength = allocEnc == Vertical ? range->size[YY] : range->size[XX];
  }

  /* Size the rectangle around the divider */

//#ifdef QT_DEBUG_MODE
//          logDebug() << "\n" << dividerType << " Divider Attributes for Step [" << _step->stepNumber.number << "]:"
//                     << "\nseparatorLength    [" << separatorLength << "]"
//                     << "\nallocEnc           [" << (allocEnc == Vertical ? "Vertical" : "Horizontal") << "]"
//                     << "\nsepData.margin[XX] [" << sepData.margin[XX] << "]"
//                     << "\nsepData.margin[YY] [" << sepData.margin[YY] << "]"
//                     << "\nsepData.thickness  [" << sepData.thickness << "]"
//                     << "\nrange->stepSpacing [" << range->stepSpacing << "]"
//                     << "\nseparatorHeightX   [" << sepData.margin[XX]+sepData.thickness*2 << "]  sepData.margin[XX] + sepData.thickness * 2"
//                     << "\nseparatorHeightY   [" << sepData.margin[YY]+sepData.thickness*2 << "]  sepData.margin[YY] + sepData.thickness * 2"
//                     << "\nseparatorWidthX V  [" << sepData.margin[XX]+sepData.thickness/2 << "] sepData.margin[XX] + sepData.thickness / 2"
//                     << "\nseparatorWidthY H  [" << sepData.margin[YY]+sepData.thickness/2 << "] sepData.margin[YY] + sepData.thickness / 2"
//                     << "\nspacingHeightY  V  [" << (sepData.margin[YY]+sepData.thickness+range->stepSpacing)/2 << "] (sepData.margin[YY]+sepData.thickness+range->stepSpacing) / 2"
//                     << "\nspacingHeightX  H  [" << (sepData.margin[XX]+sepData.thickness+range->stepSpacing)/2 << "] (sepData.margin[XX]+sepData.thickness+range->stepSpacing) / 2"
//                        ;
//#endif

  if(parentStep->dividerType == StepDivider) { // Step divider
      separatorHeightY = int(sepData.margin[YY]+sepData.thickness)*2;
      separatorWidthX  = int(sepData.margin[XX]+sepData.thickness)*2;
      if (allocEnc == Vertical) {
          if (sepData.type == SepData::LenPage) // Manually adjust page size
              separatorLength = gui->pageSize(meta.LPub.page,XX) - (_offsetX + separatorWidthX);
          setRect(_offsetX,
                  _offsetY,
                  separatorLength,
                  separatorHeightY);
//#ifdef QT_DEBUG_MODE
//          logDebug() << "\nStep Divider Vertical Rectangle for Step [" << _step->stepNumber.number << "]:"
//                     << "\nrectangle::(x1) [" << _offsetX << "] offsetX"
//                     << "\nrectangle::(y1) [" << _offsetY << "] offsetY"
//                     << "\nrectangle::(x2) [" << separatorLength << "] separatorLength"
//                     << "\nrectangle::(y2) [" << separatorHeightY << "] separatorHeightY"
//                        ;
//#endif
      } else {
          if (sepData.type == SepData::LenPage) // // Manually adjust page size
              separatorLength = gui->pageSize(meta.LPub.page,YY) - (_offsetY + separatorHeightY);
          setRect(_offsetX,
                  _offsetY,
                  separatorWidthX,
                  separatorLength);
//#ifdef QT_DEBUG_MODE
//          logDebug() << "\nStep Divider Horizontal Rectangle for Step [" << _step->stepNumber.number << "]:"
//                     << "\nrectangle::(x1) [" << _offsetX << "] offsetX"
//                     << "\nrectangle::(y1) [" << _offsetY << "] offsetY"
//                     << "\nrectangle::(x2) [" << separatorWidthX << "] separatorWidthX"
//                     << "\nrectangle::(y2) [" << separatorLength << "] separatorLength"
//                        ;
// #endif
      }
  } else {                        // Range divider
      separatorHeightY = int(sepData.margin[YY])+int(sepData.thickness)*2;
      separatorWidthX  = int(sepData.margin[XX])+int(sepData.thickness)*2;
      if (allocEnc == Vertical) {
        setRect(_offsetX,
                _offsetY,
                separatorWidthX,
                separatorLength);
//#ifdef QT_DEBUG_MODE
//        logDebug() << "\nRange Divider Horizontal Rectangle for Step [" << _step->stepNumber.number << "]:"
//                   << "\nrectangle::(x1) [" << _offsetX << "] offsetX"
//                   << "\nrectangle::(y1) [" << _offsetY << "] offsetY"
//                   << "\nrectangle::(x2) [" << separatorWidthX << "] separatorWidthX"
//                   << "\nrectangle::(y2) [" << separatorLength << "] separatorLength"
//                      ;
//#endif
      } else {
        setRect(_offsetX,
                _offsetY,
                separatorLength,
                separatorHeightY);
//#ifdef QT_DEBUG_MODE
//        logDebug() << "\nRange Divider Vertical Rectangle for Step [" << _step->stepNumber.number << "]:"
//                   << "\nrectangle::(x1) [" << _offsetX << "] offsetX"
//                   << "\nrectangle::(y1) [" << _offsetY << "] offsetY"
//                   << "\nrectangle::(x2) [" << separatorLength << "] separatorLength"
//                   << "\nrectangle::(y2) [" << separatorHeightY << "] separatorHeightY"
//                      ;
//#endif
      }
  }

  setPen(QPen(Qt::NoPen));
  setBrush(QBrush(Qt::NoBrush));
  setToolTip(QString("Divider [%1 x %2 px] - right-click to modify")
             .arg(boundingRect().width())
             .arg(boundingRect().height()));

  lineItem = new DividerLine(this);
  lineItem->stepNumber = parentStep->stepNumber.number;
  lineItem->top = parentStep->topOfStep();
  lineItem->bottom = parentStep->bottomOfStep();

  BorderData borderData;

  if (steps->relativeType == CalloutType) {
    borderData = _meta->LPub.callout.border.valuePixels();
  } else {
    borderData.margin[0] = 0;
    borderData.margin[1] = 0;
    borderData.thickness = 0;
  }

  if (double(sepData.thickness) > 0.5) {
    // determine the position of the divider
    if(parentStep->dividerType == StepDivider) {     // Step divider
        if (allocEnc == Vertical) {
            separatorWidthX = int(sepData.margin[XX])+int(sepData.thickness)/2;
            spacingHeightY  = int(sepData.margin[YY])+int(sepData.thickness+range->stepSpacing)/2;
            lineItem->setLine(_offsetX-separatorWidthX,
                              _offsetY-spacingHeightY,    // top
                              _offsetX+separatorLength+separatorWidthX,
                              _offsetY-spacingHeightY);   // top
//#ifdef QT_DEBUG_MODE
//            logDebug() << "\nStep Divider Line Vertical Position points for Step [" << _step->stepNumber.number << "]:"
//                       << "\nlineItem::(x1) [" << _offsetX-separatorWidthX << "] offsetX - separatorWidthX"
//                       << "\nlineItem::(y1) [" << _offsetY-spacingHeightY << "] offsetY - spacingHeightY"
//                       << "\nlineItem::(x2) [" << _offsetX+separatorLength-separatorWidthX << "] offsetX + separatorLength - separatorWidthX"
//                       << "\nlineItem::(y2) [" << _offsetY-spacingHeightY << "] offsetY - (sepData.margin[YY] + spacingHeightY"
//                          ;
//#endif
        } else {
            separatorHeightY = int(sepData.margin[YY])+int(sepData.thickness)/2;
            spacingWidthX    = /*sepData.margin[XX]+*/int(sepData.thickness+range->stepSpacing)/2;
            lineItem->setLine(_offsetX-spacingWidthX,     // left
                              _offsetY-separatorHeightY,
                              _offsetX-spacingWidthX,     // left
                              _offsetY+separatorLength-separatorHeightY);
//#ifdef QT_DEBUG_MODE
//            logDebug() << "\nStep Divider Line Horizontal Position points for Step [" << _step->stepNumber.number << "]:"
//                       << "\nlineItem::(x1) [" << _offsetX-spacingWidthX << "] offsetX - spacingWidthX"
//                       << "\nlineItem::(y1) [" << _offsetY-separatorHeightY << "] offsetY - separatorHeightY"
//                       << "\nlineItem::(x2) [" << _offsetX-spacingWidthX << "] offsetX - spacingWidthX"
//                       << "\nlineItem::(y2) [" << _offsetY+separatorLength-separatorHeightY << "] offsetY + separatorLength - separatorHeightY"
//                          ;
//#endif
        }
    } else {                            // Range divider
        if (allocEnc == Vertical) {
          separatorWidthX = int(sepData.margin[XX])+int(sepData.thickness)/2;
          spacingHeightY =  0.0; //= (sepData.margin[YY]+sepData.thickness+range->stepSpacing)/2;
          lineItem->setLine(_offsetX+separatorWidthX,   // right
                            _offsetY-spacingHeightY,
                            _offsetX+separatorWidthX,   // right
                            _offsetY+separatorLength-spacingHeightY);
//#ifdef QT_DEBUG_MODE
//          logDebug() << "\nRange Divider Line Vertical Position points for Step [" << _step->stepNumber.number << "]:"
//                     << "\nlineItem::(x1) [" << _offsetX+separatorWidthX << "] offsetX + separatorWidth"
//                     << "\nlineItem::(y1) [" << _offsetY << "] offsetY"
//                     << "\nlineItem::(x2) [" << _offsetX+separatorWidthX << "] offsetX + separatorWidth"
//                     << "\nlineItem::(y2) [" << _offsetY+separatorLength << "] offsetY + separatorLength"
//                        ;
//#endif
        } else {
          separatorHeightY = int(sepData.margin[YY])+int(sepData.thickness)/2;
          spacingWidthX    = 0.0; //(sepData.margin[XX]+sepData.thickness+range->stepSpacing)/2;
          lineItem->setLine(_offsetX-spacingWidthX,
                            _offsetY+separatorHeightY,   // top
                            _offsetX+separatorLength-spacingWidthX,
                            _offsetY+separatorHeightY);  // top
//#ifdef QT_DEBUG_MODE
//          logDebug() << "\nRange Divider Line Horizontal Position points for Step [" << _step->stepNumber.number << "]:"
//                     << "\nlineItem::(x1) [" << _offsetX-spacingWidthX << "] offsetX - spacingWidthX"
//                     << "\nlineItem::(y1) [" << _offsetY+separatorHeightY << "] offsetY + separatorHeightY"
//                     << "\nlineItem::(x2) [" << _offsetX+separatorLength-spacingWidthX << "] offsetX + separatorLength - spacingWidthX"
//                     << "\nlineItem::(y2) [" << _offsetY+separatorHeightY << "] offsetY + separatorHeightY"
//                        ;
//#endif
        }
    }

    loc[XX]  = int(lineItem->boundingRect().x());
    loc[YY]  = int(lineItem->boundingRect().y());
    size[XX] = int(lineItem->boundingRect().size().width());
    size[YY] = int(lineItem->boundingRect().size().height());

//#ifdef QT_DEBUG_MODE
//    logDebug() << "\nFinal " << dividerType << " Divider Dimensions for Step [" << _step->stepNumber.number << "]:"
//               << "\nlineItem::loc XX  [" << loc[XX] << "]"
//               << "\nlineItem::loc YY  [" << loc[YY] << "]"
//               << "\nlineItem::size XX [" << size[XX] << "]"
//               << "\nlineItem::size YY [" << size[YY] << "]"
//               << "\nlineItem::size    [" << lineItem->boundingRect().size() << "]"
//                  ;
//#endif

    QPen pen(LDrawColor::color(sepData.color));
    pen.setWidth(int(sepData.thickness));
    pen.setCapStyle(Qt::RoundCap);
    lineItem->setPen(pen);
  }

  setData(ObjectId, DividerObj);
  setZValue(DIVIDER_ZVALUE_DEFAULT);

  setFlag(QGraphicsItem::ItemIsSelectable,true);
  setFlag(QGraphicsItem::ItemIsFocusable, true);
  setAcceptHoverEvents(true);
}

DividerItem::~DividerItem()
{
  graphicsDividerPointerList.clear();
};

void DividerItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
  QMenu menu;
  QAction *editAction;
  QString pl = "Divider";

  editAction = menu.addAction("Edit " + pl);
  editAction->setIcon(QIcon(":/resources/editdivider.png"));
  editAction->setWhatsThis("Edit this divider margin, thickness, length, and color");

  QAction *deleteAction;

  deleteAction = menu.addAction("Delete " + pl);
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
    background = new DividerBackgroundItem(
                &meta,
                dividerRect,
                parentItem());
    background->stepNumber = parentStep->stepNumber.number;
    background->top        = parentStep->topOfStep();
    background->bottom     = parentStep->bottomOfStep();
    background->setPos(loc[XX],loc[YY]);
    background->setFlag(QGraphicsItem::ItemIsMovable, false);
    background->setFlag(QGraphicsItem::ItemIsSelectable, false);

    DividerPointerItem *pi =
            new DividerPointerItem(
                this,
                pointer,
                background,
                view);
    graphicsDividerPointerList.append(pi);
}

void DividerItem::updatePointers(QPoint &delta)
{
  for (int i = 0; i < graphicsDividerPointerList.size(); i++) {
    DividerPointerItem *pointerItem = graphicsDividerPointerList[i];
    pointerItem->updatePointer(delta);
  }
}

void DividerItem::drawTips(QPoint &delta, int type)
{
  for (int i = 0; i < graphicsDividerPointerList.size(); i++) {
    DividerPointerItem *pointerItem = graphicsDividerPointerList[i];
    int initiator = type ? type : 102 /*DividerType*/;
    pointerItem->drawTip(delta,initiator);
  }
}

void DividerItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    isHovered = !this->isSelected() && !mouseIsDown;
    QGraphicsItem::hoverEnterEvent(event);
}

void DividerItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    isHovered = false;
    QGraphicsItem::hoverLeaveEvent(event);
}

void DividerItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    mouseIsDown = true;
    QGraphicsItem::mousePressEvent(event);
    update();
}

void DividerItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    mouseIsDown = false;
    QGraphicsItem::mouseReleaseEvent(event);
    update();
}

void DividerItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QPen pen;
    pen.setColor(isHovered ? QColor(Preferences::sceneGuideColor) : Qt::black);
    pen.setWidth(0/*cosmetic*/);
    pen.setStyle(isHovered ? Qt::PenStyle(Preferences::sceneGuidesLine) : Qt::NoPen);
    painter->setPen(pen);
    painter->setBrush(Qt::transparent);
    painter->drawRect(this->boundingRect());
    QGraphicsRectItem::paint(painter,option,widget);
}

/******************************************************************************
 * Divider line routines
 *****************************************************************************/

DividerLine::DividerLine(DividerItem *parent)
{
  setParentItem(parent);
  setData(ObjectId, DividerLineObj);
  setZValue(DIVIDERLINE_ZVALUE_DEFAULT);
}

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
    :QGraphicsRectItem(parent)
{
  border = &_meta->LPub.pointerBase.border;

  setRect(0,0,_dividerRect.width(),_dividerRect.height());
  setToolTip(QString());
  setPen(QPen(Qt::NoPen));
  setBrush(QBrush(Qt::NoBrush));
  setParentItem(parent);
  setData(ObjectId, DividerBackgroundObj);
  setZValue(DIVIDERBACKGROUND_ZVALUE_DEFAULT);
}
