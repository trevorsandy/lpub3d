/****************************************************************************
**
** Copyright (C) 2015 Trevor SANDY. All rights reserved.
**
** This file may be used under the terms of the
** GNU General Public Liceense (GPL) version 3.0
** which accompanies this distribution, and is
** available at http://www.gnu.org/licenses/gpl.html
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "pageattributeitem.h"
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

void PageAttributeItem::setAttributes(
  PlacementType      _relativeType,
  PlacementType      _parentRelativeType,
  Meta               *_meta,
  PageAttributeMeta  &_pageAttribute,
  const char         *_format,
  QString            _value,
  QString            &toolTip,
  QGraphicsItem      *_parent,
  QString            _name)
{
  relativeType       = _relativeType;
  parentRelativeType = _parentRelativeType;
  meta               = _meta;
  textFont           = &_pageAttribute.textFont;
  textColor          = &_pageAttribute.textColor;
  margin             = &_pageAttribute.margin;
  value              = _value;
  name               = _name;

  QFont qfont;
  qfont.fromString(_pageAttribute.textFont.valueFoo());
  setFont(qfont);

  QString foo;
  foo.sprintf(_format,_value);
  setPlainText(foo);
  setDefaultTextColor(LDrawColor::textColor(textColor->value()));
  setToolTip(toolTip);
  setParentItem(_parent);
}

PageAttributeItem::PageAttributeItem()
{
  relativeType  = PageAttributeType;
  meta          = NULL;
  textFont      = NULL;
  textColor     = NULL;
  margin        = NULL;
}

PageAttributeItem::PageAttributeItem(
  PlacementType     _relativeType,
  PlacementType     _parentRelativeType,
  Meta              *_meta,
  PageAttributeMeta &_pageAttribute,
  const char        *_format,
  QString           _value,
  QString           &_toolTip,
  QGraphicsItem     *_parent,
  QString           _name)
{
  setAttributes(_relativeType,
                _parentRelativeType,
                _meta,
                _pageAttribute,
                _format,
                _value,
                _toolTip,
                _parent,
                _name);
}

PageAttributePlacementItem::PageAttributePlacementItem()
{
  relativeType = PageAttributeType;
  setFlag(QGraphicsItem::ItemIsMovable,true);
  setFlag(QGraphicsItem::ItemIsSelectable,true);
}

PageAttributePlacementItem::PageAttributePlacementItem(
  PlacementType                 _relativeType,
  PlacementType                 _parentRelativeType,
  PageAttributePlacementMeta    &_pageAttribute,
  const char                    *_format,
  QString                       _value,
  QString                       &toolTip,
  QGraphicsItem                 *_parent,
  QString                       _name)
{
  setAttributes(_relativeType,
                _parentRelativeType,
                _pageAttribute,
                _format,
                _value,
                toolTip,
                _parent,
                _name);
}

void PageAttributePlacementItem::setAttributes(
  PlacementType                 _relativeType,
  PlacementType                 _parentRelativeType,
  PageAttributePlacementMeta    &_pageAttribute,
  const char                    *_format,
  QString                       _value,
  QString                       &toolTip,
  QGraphicsItem                 *_parent,
  QString                       _name)
{
  relativeType          = _relativeType;
  parentRelativeType    = _parentRelativeType;
  textFont              =  _pageAttribute.textFont;
  textColor             =  _pageAttribute.textColor;
  margin                =  _pageAttribute.margin;
  placement             =  _pageAttribute.placement;
  value                 =  _value;
  name                  =  _name;

  QFont qfont;
  qfont.fromString(_pageAttribute.textFont.valueFoo());
  setFont(qfont);

  QString foo;
  foo.sprintf(_format,_value);
  setPlainText(foo);
  setDefaultTextColor(LDrawColor::textColor(textColor.value()));

  setToolTip(toolTip);
  setParentItem(_parent);
}
void PageAttributePlacementItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
  QGraphicsItem::mousePressEvent(event);
  positionChanged = false;
  position = pos();
}

void PageAttributePlacementItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
  QGraphicsItem::mouseMoveEvent(event);
  if (isSelected() && (flags() & QGraphicsItem::ItemIsMovable)) {
    positionChanged = true;
  }
}

void PageAttributePlacementItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
  QGraphicsItem::mouseReleaseEvent(event);
}

PagePageAttributeItem::PagePageAttributeItem(
  Page                          *_page,
  PageAttributePlacementMeta    &_pageAttribute,
  const char                    *_format,
  QString                       _value,
  QGraphicsItem                 *_parent)
{
  page = _page;
  QString toolTip("Page Attribute - use popu menu");
  setAttributes(PageAttributeType,
                SingleStepType,
                _pageAttribute,
                _format,
                _value,
                toolTip,
                _parent);
}

void PagePageAttributeItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
  QMenu menu;

  PlacementData placementData = placement.value();
  QString name = "Move Page Attribute";
  QAction *placementAction  = menu.addAction(name);
  placementAction->setWhatsThis(
    commonMenus.naturalLanguagePlacementWhatsThis(PageAttributeType,placementData,name));

  QAction *fontAction       = menu.addAction("Change Page Attribute Font");
  QAction *colorAction      = menu.addAction("Change Page Attribute Color");
  QAction *marginAction     = menu.addAction("Change Page Attribute Margins");

  fontAction->setWhatsThis("You can change the textFont or the size of the page pageAttribute");
  colorAction->setWhatsThis("You can change the textColor of the page pageAttribute");
  marginAction->setWhatsThis("You can change how much empty space their is around the page pageAttribute");

  QAction *selectedAction   = menu.exec(event->screenPos());

  if (selectedAction == placementAction) {

    changePlacement(PageType,
                    PageAttributeType,
                    "Page Attribute Placement",
                    page->topOfSteps(),
                    page->bottomOfSteps(),
                  &placement);

  } else if (selectedAction == fontAction) {

    changeFont(page->topOfSteps(),page->bottomOfSteps(),&textFont);

  } else if (selectedAction == colorAction) {

    changeColor(page->topOfSteps(),page->bottomOfSteps(),&textColor);

  } else if (selectedAction == marginAction) {

    changeMargins("Page Attribute Margins",
                  page->topOfSteps(),page->bottomOfSteps(),
                &margin);
  }
}

void PagePageAttributeItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
  QGraphicsItem::mouseReleaseEvent(event);

  if (isSelected() && (flags() & QGraphicsItem::ItemIsMovable)) {

    QPointF newPosition;

    // back annotate the movement of the PLI into the LDraw file.
    newPosition = pos() - position;

    if (newPosition.x() || newPosition.y()) {
      positionChanged = true;

      PlacementData placementData = placement.value();

      placementData.offsets[0] += newPosition.x()/relativeToSize[0];
      placementData.offsets[1] += newPosition.y()/relativeToSize[1];

      placement.setValue(placementData);

      changePlacementOffset(page->bottomOfSteps(),&placement,StepNumberType);
    }
  }
}
