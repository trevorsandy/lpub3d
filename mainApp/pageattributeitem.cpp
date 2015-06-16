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
  PlacementType       _relativeType,
  PlacementType       _parentRelativeType,
  Meta               *_meta,
  PageAttributeTextMeta  &_pageAttributeText,       // change to PageAttributeData
  PageAttributePictureMeta &_pageAttributePicture,
  QString             _value,
  QString            &toolTip,
  QGraphicsItem      *_parent,
  QString             _name)
{
  PageAttributePictureData _Picture = _pageAttributePicture.value();
  relativeType       =  _relativeType;
  parentRelativeType =  _parentRelativeType;
  meta               =  _meta;
  textFont           = &_pageAttributeText.textFont;
  textColor          = &_pageAttributeText.textColor;
  margin             = &_pageAttributeText.margin;
  alignment          = &_pageAttributeText.alignment;
  picScale           =  _Picture.picScale;      // don't need, can just use mouse drag
  value              =  _value;
  name               =  _name;

  QFont qfont;
  qfont.fromString(_pageAttributeText.textFont.valueFoo());
  setFont(qfont);

  QString foo;
  foo = _value;
  setPlainText(foo);
  setDefaultTextColor(LDrawColor::color(textColor->value()));
  setToolTip(toolTip);
  setParentItem(_parent);
}

PageAttributeItem::PageAttributeItem()
{
  relativeType  = PageAttributeType;    // should be from PageAttributeData
  meta          = NULL;
  textFont      = NULL;
  textColor     = NULL;
  margin        = NULL;
  alignment     = NULL;
  picScale      = NULL;
}

PageAttributeItem::PageAttributeItem(
  PlacementType      _relativeType,
  PlacementType      _parentRelativeType,
  Meta              *_meta,
  PageAttributeTextMeta &_pageAttributeText,
  PageAttributePictureMeta &_pageAttributePicture,
  QString            _value,
  QString           &_toolTip,
  QGraphicsItem     *_parent,
  QString            _name)
{
  setAttributes(_relativeType,
                _parentRelativeType,
                _meta,
                _pageAttributeText,
                _pageAttributePicture,
                _value,
                _toolTip,
                _parent,
                _name);
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
  QAction *scaleAction      = menu.addAction("Change Image Scale");

  fontAction->setWhatsThis("You can change the textFont or the size of the page pageAttributeText");
  colorAction->setWhatsThis("You can change the textColor of the page pageAttributeText");
  marginAction->setWhatsThis("You can change how much empty space their is around the page pageAttributeText");
  scaleAction->setWhatsThis("You can change the size of this image using the scale dialog (window).");

  Where topOfSteps      = page->topOfSteps();
  Where bottomOfSteps   = page->bottomOfSteps();
  Where begin           = topOfSteps;

  QAction *selectedAction   = menu.exec(event->screenPos());


  if (selectedAction == placementAction) {

    changePlacement(PageType,
                    PageAttributeType,
                    "Move Page Attribute",
                    topOfSteps,
                    bottomOfSteps,
                    &placement);

  } else if (selectedAction == fontAction) {

    changeFont(topOfSteps,bottomOfSteps,textFont);

  } else if (selectedAction == colorAction) {

    changeColor(topOfSteps,bottomOfSteps,textColor);

  } else if (selectedAction == marginAction) {

    changeMargins("Page Attribute Margins",
                  topOfSteps,bottomOfSteps,margin);

  }

//  else if (selectedAction == scaleAction) {

//      bool allowLocal = parentRelativeType != StepGroupType &&
//                        parentRelativeType != CalloutType;
//      changeFloatSpin("Image",
//                      "Image Size",
//                      begin,
//                      topOfSteps,
//                      &meta->LPub.page.documentLogo.,     //investigate
//                      1,allowLocal);
//  }
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

      changePlacementOffset(page->bottomOfSteps(),&placement,PageAttributeType);

    }
  }
}

void PagePageAttributeItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
  QGraphicsItem::mousePressEvent(event);
  positionChanged = false;
  position = pos();
}

void PagePageAttributeItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
  QGraphicsItem::mouseMoveEvent(event);
  if (isSelected() && (flags() & QGraphicsItem::ItemIsMovable)) {
    positionChanged = true;
  }
}

void PagePageAttributeItem::focusInEvent(QFocusEvent *event)
{
  textValueChanged = false;
  QGraphicsTextItem::focusInEvent(event);
}

void PagePageAttributeItem::focusOutEvent(QFocusEvent *event)
{
  QGraphicsTextItem::focusOutEvent(event);

  // change meta

//  if (textChanged) {
//    InsertData insertData = meta.value();
//    QStringList list = toPlainText().split("\n");
//    insertData.text = list.join("\\n");
//    meta.setValue(insertData);

//    beginMacro(QString("Edit"));
//    changeInsertOffset(&meta);    // Create New one to accept PageAttributeItewm
//    endMacro();
//  }
}

void PagePageAttributeItem::keyPressEvent(QKeyEvent *event)
{
  textValueChanged = true;
  QGraphicsTextItem::keyPressEvent(event);
}
void PagePageAttributeItem::keyReleaseEvent(QKeyEvent *event)
{
  textValueChanged = true;
  QGraphicsTextItem::keyReleaseEvent(event);
}
