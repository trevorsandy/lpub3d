 
/****************************************************************************
**
** Copyright (C) 2007-2009 Kevin Clague. All rights reserved.
** Copyright (C) 2015 - 2025 Trevor SANDY. All rights reserved.
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
 * This file describes the graphical representation of a number displayed
 * in the graphics scene that is used to describe a building instruction
 * page.
 *
 * Please see lpub.h for an overall description of how the files in LPub3D
 * make up the LPub3D program.
 *
 ***************************************************************************/

#include "numberitem.h"
#include "metatypes.h"
#include <QColor>
#include <QPixmap>
#include <QAction>
#include <QMenu>
#include <QGraphicsSceneContextMenuEvent>
#include "color.h"
#include "declarations.h"
#include "placementdialog.h"
#include "commonmenus.h"

#include "ranges.h"
#include "step.h"
#include "lpub.h"
#include "declarations.h"

void NumberItem::setAttributes(
  PlacementType  _relativeType,
  PlacementType  _parentRelativeType,
  Meta          *_meta,
  NumberMeta    &_number,
  const char    *_format,
  int            _value,
  QString       &toolTip,
  QGraphicsItem *_parent,
  QString        _name)
{
  relativeType = _relativeType;
  parentRelativeType = _parentRelativeType;
  meta         =  _meta;
  font         = &_number.font;
  color        = &_number.color;
  margin       = &_number.margin;
  value        = _value;
  name         = _name;

  QFont qfont;
  qfont.fromString(_number.font.valueFoo());
  setFont(qfont);

  QString foo;
  foo.sprintf(_format,_value);
  setPlainText(foo);
  setDefaultTextColor(LDrawColor::color(color->value()));
  setToolTip(toolTip);
  setParentItem(_parent);
}

NumberItem::NumberItem()
{
  relativeType = PageNumberType;
  meta = nullptr;
  font = nullptr;
  color = nullptr;
  margin = nullptr;
}

NumberItem::NumberItem(
  PlacementType  _relativeType,
  PlacementType  _parentRelativeType,
  Meta          *_meta,
  NumberMeta    &_number,
  const char    *_format,
  int            _value,
  QString       &_toolTip,
  QGraphicsItem *_parent,
  QString        _name)
{
  setAttributes(_relativeType,
                _parentRelativeType,
                _meta,
                _number,
                _format,
                _value,
                _toolTip,
                _parent,
                _name);
}

NumberPlacementItem::NumberPlacementItem()
{
  value = 0;
  relativeType = PageNumberType;
  setFlag(QGraphicsItem::ItemIsMovable,true);
  setFlag(QGraphicsItem::ItemIsSelectable,true);
}

NumberPlacementItem::NumberPlacementItem(
  PlacementType        _relativeType,
  PlacementType        _parentRelativeType,
  NumberPlacementMeta &_number,
  const char          *_format,
  int                  _value,
  QString             &_toolTip,
  QGraphicsItem       *_parent,
  QString              _name)
{
  setAttributes(_relativeType,
                _parentRelativeType,
                _number,
                _format,
                _value,
                _toolTip,
                _parent,
                _name);
}

void NumberPlacementItem::setAttributes(
  PlacementType        _relativeType,
  PlacementType        _parentRelativeType,
  NumberPlacementMeta &_number,
  const char          *_format,
  int                  _value,
  QString             &_toolTip,
  QGraphicsItem       *_parent,
  QString              _name)
{
  relativeType       = _relativeType;
  parentRelativeType = _parentRelativeType;
  font               = _number.font;
  color              = _number.color;
  margin             = _number.margin;
  placement          = _number.placement;
  value              = _value;
  name               = _name;

  QFont qfont;
  qfont.fromString(_number.font.valueFoo());
  setFont(qfont);

  QString foo;
  foo.sprintf(_format,_value);
  setPlainText(foo);
  setDefaultTextColor(LDrawColor::color(color.value()));

  setToolTip(_toolTip);
  setParentItem(_parent);
}

void NumberPlacementItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{     
  QGraphicsItem::mousePressEvent(event);
  positionChanged = false;
  position = pos();
} 
  
void NumberPlacementItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{ 
  QGraphicsItem::mouseMoveEvent(event);
  if (isSelected() && (flags() & QGraphicsItem::ItemIsMovable)) {
    positionChanged = true;
  }   
}     
      
void NumberPlacementItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{     
  QGraphicsItem::mouseReleaseEvent(event);
} 

// Group Step Number - used exclusively when placing the step number for pli per page

GroupStepNumberItem::GroupStepNumberItem(
  Page                *_page,
  NumberPlacementMeta &_number,
  const char          *_format,
  int                  _value,
  QGraphicsItem       *_parent) :
  isHovered(false),
  mouseIsDown(false)
{
  page = _page;
  QString toolTip(tr("Group Step Number - right click to modify"));
  setAttributes(StepNumberType,
                page->relativeType,
                _number,
                _format,
                _value,
                toolTip,
                _parent);
  setData(ObjectId, StepNumberObj);
  setZValue(STEPNUMBER_ZVALUE_DEFAULT);
  setFlag(QGraphicsItem::ItemIsSelectable,true);
  setFlag(QGraphicsItem::ItemIsFocusable, true);
  setAcceptHoverEvents(true);
}

void GroupStepNumberItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
  QMenu menu;
  QString name = tr("Group Step Number");

  QAction *placementAction    = lpub->getAct("placementAction.1");
  PlacementData placementData = placement.value();
  placementAction->setWhatsThis(commonMenus.naturalLanguagePlacementWhatsThis(relativeType,placementData,name));
  commonMenus.addAction(placementAction,menu,name);

  QAction *fontAction         = lpub->getAct("fontAction.1");
  commonMenus.addAction(fontAction,menu,name);

  QAction *colorAction        = lpub->getAct("colorAction.1");
  commonMenus.addAction(colorAction,menu,name);

  QAction *marginAction       = lpub->getAct("marginAction.1");
  commonMenus.addAction(marginAction,menu,name);

  QAction *selectedAction     = menu.exec(event->screenPos());

  Where topOfSteps            = page->topOfSteps();
  Where bottomOfSteps         = page->bottomOfSteps();
  bool  useTop                = parentRelativeType != StepGroupType;

  if (selectedAction == nullptr) {
    return;
  } else if (selectedAction == placementAction) {
    placement.setPartsListPerStep(page->meta.LPub.multiStep.pli.perStep.value());
    changePlacement(parentRelativeType,
                    StepNumberType,
                    tr("Move %1").arg(name),
                    topOfSteps,
                    bottomOfSteps,
                   &placement,
                    useTop);

  } else if (selectedAction == fontAction) {

    changeFont(topOfSteps,
               bottomOfSteps,
              &font,1,true,
               useTop);

  } else if (selectedAction == colorAction) {

    changeColor(topOfSteps,
                bottomOfSteps,
               &color,1,true,
                useTop);

  } else if (selectedAction == marginAction) {

    changeMargins(tr("%1 Margins").arg(name),
                  topOfSteps,
                  bottomOfSteps,
                 &margin,
                  useTop);
  }
}

void GroupStepNumberItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    isHovered = !this->isSelected() && !mouseIsDown;
    QGraphicsItem::hoverEnterEvent(event);
}

void GroupStepNumberItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    isHovered = false;
    QGraphicsItem::hoverLeaveEvent(event);
}

void GroupStepNumberItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    mouseIsDown = true;
    NumberPlacementItem::mousePressEvent(event);
}

void GroupStepNumberItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
  mouseIsDown = false;

  NumberPlacementItem::mouseReleaseEvent(event);

  if (isSelected() && (flags() & QGraphicsItem::ItemIsMovable)) {

    QPointF positionMoved;

    Where topOfSteps    = page->topOfSteps();
    Where bottomOfSteps = page->bottomOfSteps();
    bool  useTop        = parentRelativeType != StepGroupType;

    positionMoved = pos() - position;

    if (positionMoved.x() || positionMoved.y()) {
      positionChanged = true;

      PlacementData placementData = placement.value();
      placementData.offsets[0] += positionMoved.x()/relativeToSize[0];
      placementData.offsets[1] += positionMoved.y()/relativeToSize[1];
      placement.setValue(placementData);

      changePlacementOffset(useTop ? topOfSteps : bottomOfSteps,
                           &placement,
                            StepNumberType);
    }
  }
}

void GroupStepNumberItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QPen pen;
    pen.setColor(isHovered ? QColor(Preferences::sceneGuideColor) : Qt::black);
    pen.setWidth(0/*cosmetic*/);
    pen.setStyle(isHovered ? Qt::PenStyle(Preferences::sceneGuidesLine) : Qt::NoPen);
    painter->setPen(pen);
    painter->setBrush(Qt::transparent);
    painter->drawRect(this->boundingRect());
    QGraphicsTextItem::paint(painter,option,widget);
}

// Page Number

PageNumberItem::PageNumberItem(
  Page                *_page,
  NumberPlacementMeta &_number,
  const char          *_format,
  int                  _value,
  QGraphicsItem       *_parent) :
  isHovered(false),
  mouseIsDown(false)
{
  page = _page;
  QString toolTip(tr("Page Number - right click to modify"));
  setAttributes(PageNumberType,
                page->relativeType,                 //Trevor@vers303 changed from static value SingleStepType
                _number,
                _format,
                _value,
                toolTip,
                _parent);
  setData(ObjectId, PageNumberObj);
  setZValue(PAGENUMBER_ZVALUE_DEFAULT);
  setFlag(QGraphicsItem::ItemIsSelectable,true);
  setFlag(QGraphicsItem::ItemIsFocusable, true);
  setAcceptHoverEvents(true);
}

void PageNumberItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
  QMenu menu;
  QString name = tr("Page Number");

  QAction *placementAction    = lpub->getAct("placementAction.1");
  PlacementData placementData = placement.value();
  placementAction->setWhatsThis(commonMenus.naturalLanguagePlacementWhatsThis(relativeType,placementData,name));
  commonMenus.addAction(placementAction,menu,name);

  QAction *fontAction         = lpub->getAct("fontAction.1");
  commonMenus.addAction(fontAction,menu,name);

  QAction *colorAction        = lpub->getAct("colorAction.1");
  commonMenus.addAction(colorAction,menu,name);

  QAction *marginAction       = lpub->getAct("marginAction.1");
  commonMenus.addAction(marginAction,menu,name);

  QAction *selectedAction     = menu.exec(event->screenPos());

  Where topOfSteps            = page->topOfSteps();                   //Trevor@vers303 add
  Where bottomOfSteps         = page->bottomOfSteps();                //Trevor@vers303 add
  bool  useTop                = parentRelativeType != StepGroupType;  //Trevor@vers303 add

  if (selectedAction == nullptr) {
    return;
  } else if (selectedAction == placementAction) {

    changePlacement(parentRelativeType,                           //Trevor@vers303 change from static PageType
                    PageNumberType,
                    tr("Move %1").arg(name),
                    topOfSteps,                                   //Trevor@vers303 change
                    bottomOfSteps,                                //Trevor@vers303 change
                   &placement,
                    useTop);                                      //Trevor@vers303 add

  } else if (selectedAction == fontAction) {

    changeFont(topOfSteps,
               bottomOfSteps,
              &font,1,true,
               useTop);                                           //Trevor@vers303 change

  } else if (selectedAction == colorAction) {                     //Trevor@vers303 change

    changeColor(topOfSteps,
                bottomOfSteps,
               &color,1,true,
                useTop);                                           //Trevor@vers303 change

  } else if (selectedAction == marginAction) {                    //Trevor@vers303 change

    changeMargins(tr("%1 Margins").arg(name),
                  topOfSteps,
                  bottomOfSteps,                                  //Trevor@vers303 change
                 &margin,
                  useTop);                                        //Trevor@vers303 add
  }
}

void PageNumberItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    isHovered = !this->isSelected() && !mouseIsDown;
    QGraphicsItem::hoverEnterEvent(event);
}

void PageNumberItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    isHovered = false;
    QGraphicsItem::hoverLeaveEvent(event);
}

void PageNumberItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    mouseIsDown = true;
    NumberPlacementItem::mousePressEvent(event);
}

void PageNumberItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
  mouseIsDown = false;

  NumberPlacementItem::mouseReleaseEvent(event);

  if (isSelected() && (flags() & QGraphicsItem::ItemIsMovable)) {

    QPointF positionMoved;

    Where topOfSteps    = page->topOfSteps();                       //Trevor@vers303 add
    Where bottomOfSteps = page->bottomOfSteps();                    //Trevor@vers303 add
    bool  useTop        = parentRelativeType != StepGroupType;

    positionMoved = pos() - position;
    
    if (positionMoved.x() || positionMoved.y()) {
      positionChanged = true;

      PlacementData placementData = placement.value();
      placementData.offsets[0] += positionMoved.x()/relativeToSize[0];
      placementData.offsets[1] += positionMoved.y()/relativeToSize[1];
      placement.setValue(placementData);

      changePlacementOffset(useTop ? topOfSteps : bottomOfSteps,     //Trevor@vers303 change from page-bottomOfSteps()
                           &placement,
                            StepNumberType);
    }
  }
}

void PageNumberItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QPen pen;
    pen.setColor(isHovered ? QColor(Preferences::sceneGuideColor) : Qt::black);
    pen.setWidth(0/*cosmetic*/);
    pen.setStyle(isHovered ? Qt::PenStyle(Preferences::sceneGuidesLine) : Qt::NoPen);
    painter->setPen(pen);
    painter->setBrush(Qt::transparent);
    painter->drawRect(this->boundingRect());
    QGraphicsTextItem::paint(painter,option,widget);
}

// Step Number

StepNumberItem::StepNumberItem(
  Step                *_step,
  PlacementType        _parentRelativeType,
  NumberPlacementMeta &_number,
  const char          *_format,
  int                  _value,
  QGraphicsItem       *_parent,
  QString              _name) :
  isHovered(false),
  mouseIsDown(false)
{
  step   = _step;
  top    = _step->topOfStep();
  bottom = _step->bottomOfStep();
  QString _toolTip(tr("Step Number - right-click to modify"));
  setAttributes(StepNumberType,
                _parentRelativeType,
                _number,
                _format,
                _value,
                _toolTip,
                _parent,
                _name);
  setData(ObjectId, StepNumberObj);
  setZValue(STEPNUMBER_ZVALUE_DEFAULT);
}

void StepNumberItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
  QMenu menu;

  QAction *placementAction    = lpub->getAct("placementAction.1");
  PlacementData placementData = placement.value();
  placementAction->setWhatsThis(commonMenus.naturalLanguagePlacementWhatsThis(relativeType,placementData,name));
  commonMenus.addAction(placementAction,menu,name);

  QAction *fontAction         = lpub->getAct("fontAction.1");
  commonMenus.addAction(fontAction,menu,name);

  QAction *colorAction        = lpub->getAct("colorAction.1");
  commonMenus.addAction(colorAction,menu,name);

  QAction *marginAction       = lpub->getAct("marginAction.1");
  commonMenus.addAction(marginAction,menu,name);

  QAction *selectedAction     = menu.exec(event->screenPos());

  if (selectedAction == nullptr) {
    return;
  }

  Where top;
  Where bottom;

  switch (parentRelativeType) {
    case CalloutType:
      top    = step->topOfCallout();
      bottom = step->bottomOfCallout();
    break;
    default:
      top    = step->topOfStep();
      bottom = step->bottomOfStep();
    break;
  }

  if (selectedAction == placementAction) {
      changePlacement(parentRelativeType,
                      StepNumberType,
                      tr("Move %1").arg(name),
                      top,
                      bottom,
                      &placement);
    } else if (selectedAction == fontAction) {
      changeFont(top,
                 bottom,
                 &font);
    } else if (selectedAction == colorAction) {
      changeColor(top,
                  bottom,
                  &color);
    } else if (selectedAction == marginAction) {
      changeMargins(tr("%1 Margins").arg(name),
                    top,
                    bottom,
                    &margin);
    }
}

void StepNumberItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    isHovered = !this->isSelected() && !mouseIsDown;
    QGraphicsItem::hoverEnterEvent(event);
}

void StepNumberItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    isHovered = false;
    QGraphicsItem::hoverLeaveEvent(event);
}

void StepNumberItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    mouseIsDown = true;
    NumberPlacementItem::mousePressEvent(event);
}

void StepNumberItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
  mouseIsDown = false;

  NumberPlacementItem::mouseReleaseEvent(event);

  if (isSelected() && (flags() & QGraphicsItem::ItemIsMovable)) {

    QPointF positionMoved;

    positionMoved = pos() - position;

//    qDebug() << QString("Old Position X %1, Y %2, positionMoved X %3, Y %4")
//                .arg(position.x()).arg(position.y())
//                .arg(positionMoved.x()).arg(positionMoved.y());
    
    if (positionMoved.x() || positionMoved.y()) {
      positionChanged = true;

      PlacementData placementData = placement.value();

      placementData.offsets[0] += positionMoved.x()/relativeToSize[0];
      placementData.offsets[1] += positionMoved.y()/relativeToSize[1];

      placement.setValue(placementData);

      Where topOfStep = step->topOfStep();

      changePlacementOffset(topOfStep, &placement, StepNumberType);
    }
  }
}

void StepNumberItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QPen pen;
    pen.setColor(isHovered ? QColor(Preferences::sceneGuideColor) : Qt::black);
    pen.setWidth(0/*cosmetic*/);
    pen.setStyle(isHovered ? Qt::PenStyle(Preferences::sceneGuidesLine) : Qt::NoPen);
    painter->setPen(pen);
    painter->setBrush(Qt::transparent);
    painter->drawRect(this->boundingRect());
    QGraphicsTextItem::paint(painter,option,widget);
}

