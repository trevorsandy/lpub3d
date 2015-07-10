 
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
 * This file describes the graphical representation of a number displayed
 * in the graphics scene that is used to describe a building instruction
 * page.
 *
 * Please see lpub.h for an overall description of how the files in LPub
 * make up the LPub program.
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
#include "name.h"
#include "placementdialog.h"
#include "commonmenus.h"

#include "ranges.h"
#include "step.h"

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
  meta = NULL;
  font = NULL;
  color = NULL;
  margin = NULL;
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
  relativeType = PageNumberType;
  setFlag(QGraphicsItem::ItemIsMovable,true);
  setFlag(QGraphicsItem::ItemIsSelectable,true);
}

NumberPlacementItem::NumberPlacementItem(
  PlacementType  _relativeType,
  PlacementType  _parentRelativeType,
  NumberPlacementMeta &_number,
  const char    *_format,
  int            _value,
  QString       &toolTip,
  QGraphicsItem *_parent,
  QString        _name)
{
  setAttributes(_relativeType,
                _parentRelativeType,
                _number,
                _format,
                _value,
                toolTip,
                _parent,
                _name);
}

void NumberPlacementItem::setAttributes(
  PlacementType  _relativeType,
  PlacementType  _parentRelativeType,
  NumberPlacementMeta &_number,
  const char    *_format,
  int            _value,
  QString       &toolTip,
  QGraphicsItem *_parent,
  QString        _name)
{
  relativeType = _relativeType;
  parentRelativeType = _parentRelativeType;
  font         =  _number.font;
  color        =  _number.color;
  margin       =  _number.margin;
  placement    =  _number.placement;
  value        =  _value;
  name         =  _name;

  QFont qfont;
  qfont.fromString(_number.font.valueFoo());
  setFont(qfont);

  QString foo;
  foo.sprintf(_format,_value);
  setPlainText(foo);
  setDefaultTextColor(LDrawColor::color(color.value()));

  setToolTip(toolTip);
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

PageNumberItem::PageNumberItem(
  Page                *_page,
  NumberPlacementMeta &_number,
  const char          *_format,
  int                  _value,
  QGraphicsItem       *_parent)
{
  page = _page;
  QString toolTip("Page Number - use popu menu");
  setAttributes(PageNumberType,
                page->relativeType,                 //Trevor@vers303 changed from static value SingleStepType
                _number,
                _format,
                _value,
                toolTip,
                _parent);
}

void PageNumberItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
  QMenu menu;

  PlacementData placementData = placement.value();
  QString name = "Move Page Number";
  QAction *placementAction  = menu.addAction(name);
  placementAction->setWhatsThis(
    commonMenus.naturalLanguagePlacementWhatsThis(PageNumberType,placementData,name));

  QAction *fontAction       = menu.addAction("Change Page Number Font");
  QAction *colorAction      = menu.addAction("Change Page Number Color");
  QAction *marginAction     = menu.addAction("Change Page Number Margins");

  fontAction->setWhatsThis("You can change the font or the size of the page number");
  colorAction->setWhatsThis("You can change the color of the page number");
  marginAction->setWhatsThis("You can change how much empty space their is around the page number");

  QAction *selectedAction   = menu.exec(event->screenPos());

  Where topOfSteps          = page->topOfSteps();                   //Trevor@vers303 add
  Where bottomOfSteps       = page->bottomOfSteps();                //Trevor@vers303 add
  bool multiStep            = parentRelativeType == StepGroupType;  //Trevor@vers303 add

  if (selectedAction == NULL) {
    return;
  } else if (selectedAction == placementAction) {

    changePlacement(parentRelativeType,                           //Trevor@vers303 change from static PageType
                    PageNumberType,
                    "Move Page Number",
                    topOfSteps,                                   //Trevor@vers303 change
                    bottomOfSteps,                                //Trevor@vers303 change
                   &placement,
                    multiStep ? false : true);                    //Trevor@vers303 add

  } else if (selectedAction == fontAction) {

    changeFont(topOfSteps,bottomOfSteps,&font);                   //Trevor@vers303 change

  } else if (selectedAction == colorAction) {                     //Trevor@vers303 change

    changeColor(topOfSteps,bottomOfSteps,&color);                 //Trevor@vers303 change

  } else if (selectedAction == marginAction) {                    //Trevor@vers303 change

    changeMargins("Page Number Margins",
                  topOfSteps,bottomOfSteps,                       //Trevor@vers303 change
                 &margin,
                  multiStep ? false : true);                      //Trevor@vers303 add
  }
}

void PageNumberItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
  QGraphicsItem::mouseReleaseEvent(event);

  if (isSelected() && (flags() & QGraphicsItem::ItemIsMovable)) {

    QPointF newPosition;

    Where topOfSteps    = page->topOfSteps();                       //Trevor@vers303 add
    Where bottomOfSteps = page->bottomOfSteps();                    //Trevor@vers303 add
    bool  useTop        = parentRelativeType != StepGroupType;

    newPosition = pos() - position;
    
    if (newPosition.x() || newPosition.y()) {
      positionChanged = true;

      PlacementData placementData = placement.value();
      placementData.offsets[0] += newPosition.x()/relativeToSize[0];
      placementData.offsets[1] += newPosition.y()/relativeToSize[1];
      placement.setValue(placementData);

      changePlacementOffset(useTop ? topOfSteps : bottomOfSteps,     //Trevor@vers303 change from page-bottomOfSteps()
                           &placement,
                            StepNumberType);
    }
  }
}

StepNumberItem::StepNumberItem(
  Step          *_step,
  PlacementType  parentRelativeType,
  NumberPlacementMeta    &_number,
  const char    *_format,
  int            _value,
  QGraphicsItem *_parent,
  QString        name)
{
  step = _step;
  QString toolTip("Step Number - popup menu");
  setAttributes(StepNumberType,
                parentRelativeType,
                _number,
                _format,
                _value,
                toolTip,
                _parent,
                name);
}

void StepNumberItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
  QMenu menu;
  QAction *placementAction = commonMenus.placementMenu(menu,name);
  QAction *fontAction      = commonMenus.fontMenu(menu,name);
  QAction *colorAction     = commonMenus.colorMenu(menu,name);
  QAction *marginAction    = commonMenus.marginMenu(menu,name);

  QAction *selectedAction   = menu.exec(event->screenPos());
  
  Where topOfStep       = step->topOfStep();
  Where bottomOfStep    = step->bottomOfStep();
  Where topOfSteps      = step->topOfSteps();
  Where bottomOfSteps   = step->bottomOfSteps();
  Where topOfCallout    = step->topOfCallout();                     //Trevor@vers305 add
  Where bottomOfCallout = step->bottomOfCallout();                  //Trevor@vers305 add

  Where top, bottom;
  bool local,useTop;                                                //Trevor@vers305 add useTop

  switch (parentRelativeType) {
  case StepGroupType:
      top    = topOfSteps;                                          //Trevor@vers305 change to use variable
      bottom = bottomOfSteps;                                       //Trevor@vers305 change to use variable
      useTop = false;                                               //Trevor@vers305 add
      local  = false;
  break;
  case CalloutType:
      top    = topOfCallout;                                        //Trevor@vers305 change to use variable
      bottom = bottomOfCallout;                                     //Trevor@vers305 change to use variable
      useTop = true;                                                //Trevor@vers305 add
      local  = false;
  break;
  default:
      top    = topOfStep;                                           //Trevor@vers305 change to use variable
      bottom = bottomOfStep;                                        //Trevor@vers305 change to use variable
      useTop = true;                                                //Trevor@vers305 add
      local  = true;
    break;
  }

  if (selectedAction == placementAction) {

    changePlacement(parentRelativeType,
                    StepNumberType,
                    "Move Step Number",
                    top,
                    bottom,
                   &placement,
                    useTop,                                         //Trevor@vers305 change to useTop
                    1,                  //append
                    local);

  } else if (selectedAction == fontAction) {

    changeFont(top, bottom, &font, 1, local, useTop);               //Trevor@vers305 add useTop

  } else if (selectedAction == colorAction) {

    changeColor(top,bottom, &color, 1, local, useTop);              //Trevor@vers305 add useTop

  } else if (selectedAction == marginAction) {

    changeMargins("Change Step Number Margins",top,bottom,&margin,useTop,1,local); //Trevor@vers305 change to useTop
  } 
}

void StepNumberItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
  QGraphicsItem::mouseReleaseEvent(event);

  if (isSelected() && (flags() & QGraphicsItem::ItemIsMovable)) {

    QPointF newPosition;

    newPosition = pos() - position;
    
    if (newPosition.x() || newPosition.y()) {
      positionChanged = true;

      PlacementData placementData = placement.value();

      placementData.offsets[0] += newPosition.x()/relativeToSize[0];
      placementData.offsets[1] += newPosition.y()/relativeToSize[1];

      placement.setValue(placementData);

      changePlacementOffset(step->topOfStep(),&placement,StepNumberType);
    }
  }
}

