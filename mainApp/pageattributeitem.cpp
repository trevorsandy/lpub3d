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

#include <QColor>
#include <QPixmap>
#include <QAction>
#include <QMenu>
#include <QGraphicsSceneContextMenuEvent>

#include "pageattributeitem.h"
#include "metatypes.h"
#include "color.h"
#include "scaledialog.h"
#include "name.h"
#include "placementdialog.h"
#include "commonmenus.h"

#include "ranges.h"
#include "step.h"

void PageAttributeItem::setAttributes(
  PlacementType              _relativeType,
  PlacementType              _parentRelativeType,
  PageAttributeTextMeta     &_pageAttributeText,
  QString                   &_toolTip,
  QGraphicsItem             *_parent,
  QString                   &_name)
{
    relativeType          =  _relativeType;
    parentRelativeType    =  _parentRelativeType;

    textFont              = _pageAttributeText.textFont;
    textColor             = _pageAttributeText.textColor;
    margin                = _pageAttributeText.margin;
    placement             = _pageAttributeText.placement;
    displayText           = _pageAttributeText.display;
    content               = _pageAttributeText.content;
    name                  = _name;

    QFont qfont;
    qfont.fromString(_pageAttributeText.textFont.valueFoo());
    setFont(qfont);

    QString text;
    QRegExp rx("\\\\n");
    QStringList list = content.value().split(rx);
    text = list.join("\n");
    setPlainText(text);
    setDefaultTextColor(LDrawColor::color(textColor.value()));
    setTextInteractionFlags(Qt::TextEditorInteraction);

    setToolTip(_toolTip);
    setZValue(1000);
    setParentItem(_parent);
}

PageAttributeItem::PageAttributeItem()
{
  relativeType  = PageTitleType;
  setFlag(QGraphicsItem::ItemIsMovable,true);
  setFlag(QGraphicsItem::ItemIsSelectable,true);
}

PageAttributeItem::PageAttributeItem(
  PlacementType              _relativeType,
  PlacementType              _parentRelativeType,
  PageAttributeTextMeta     &_pageAttributeText,
  QString                   &_toolTip,
  QGraphicsItem             *_parent,
  QString                   &_name)
{
  setAttributes(_relativeType,
                _parentRelativeType,
                _pageAttributeText,
                _toolTip,
                _parent,
                _name);
}

PagePageAttributeItem::PagePageAttributeItem(
  Page                      *_page,
  PageAttributeTextMeta     &_pageAttributeText,
  QGraphicsItem             *_parent)
{
  page          = _page;
  QString       toolTip;
  QString       name;
  switch(_pageAttributeText.type)
  {
  case PageTitleType:
      name              = tr("Title");
      toolTip           = tr("Title text - right-click to modify");
      break;
  case PageModelNameType:
      name              = tr("Model Name");
      toolTip           = tr("Model Name text - right-click to modify");
      break;
  case PageAuthorType:
      name              = tr("Author");
      toolTip           = tr("Author text - right-click to modify");
      break;
  case PageURLType:
      name              = tr("URL");
      toolTip           = tr("URL text - right-click to modify");
      break;
  case PageModelDescType:
      name              = tr("Model Description");
      toolTip           = tr("Model Description text - right-click to modify");
      break;
  case PagePublishDescType:
      name              = tr("Publish Description");
      toolTip           = tr("Publish Description text - right-click to modify");
      break;
  case PageCopyrightType:
      name              = tr("Copyright");
      toolTip           = tr("Copyright text - right-click to modify");
      break;
  case PageEmailType:
      name              = tr("Email");
      toolTip           = tr("Email text - right-click to modify");
      break;
  case PageDisclaimerType:
      name              = tr("LEGO Disclaimer");
      toolTip           = tr("Disclaimer text - right-click to modify");
      break;
  case PagePiecesType:
      name              = tr("Pieces");
      toolTip           = tr("Pieces text - right-click to modify");
      break;
  case PagePlugType:
      name              = tr("Plug");
      toolTip           = tr("Plug text - right-click to modify");
      break;
  case PageCategoryType:
      name              = tr("Category");
      toolTip           = tr("Category text - right-click to modify");
      break;
  }

  setAttributes(_pageAttributeText.type,            //relativeType
                 SingleStepType,                    //SingleStepType / page->relativeType, -parent relative type (using resolution)
                _pageAttributeText,
                 toolTip,
                _parent,
                 name);
}

void PagePageAttributeItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
  QMenu menu;

  PlacementData placementData = placement.value();
  QAction *placementAction        = commonMenus.placementMenu(menu,name,
                                    commonMenus.naturalLanguagePlacementWhatsThis(relativeType,placementData,name));
  QAction *fontAction             = commonMenus.fontMenu(menu,name);
  QAction *colorAction            = commonMenus.colorMenu(menu,name);
  QAction *marginAction           = commonMenus.marginMenu(menu,name);
  QAction *displayTextAction      = commonMenus.displayMenu(menu,name);

  Where topOfSteps          = page->topOfSteps();
  Where bottomOfSteps       = page->bottomOfSteps();

  QAction *selectedAction   = menu.exec(event->screenPos());

  if (selectedAction == NULL) {

    return;

  } else if (selectedAction == placementAction) {

    changePlacement(parentRelativeType,
                    relativeType,
                    "Move Page Attribute",
                    topOfSteps,
                    bottomOfSteps,
                    &placement);

  } else if (selectedAction == marginAction) {

//      changeMargins("Page Attribute Margins",
//                    topOfSteps,bottomOfSteps,&margin);
      float values[2];

      values[0] = margin.value(0);
      values[1] = margin.value(1);

      bool ok   = UnitsDialog::getUnits(values,"Attribute Margins");

      if (ok) {
        margin.setValues(values[0],values[1]);
      }

      mi.setGlobalMeta(topOfSteps.modelName,&margin);

  } else if (selectedAction == fontAction) {

//      changeFont(topOfSteps,bottomOfSteps,&textFont);

      QFont _font;
      QString fontName = textFont.valueFoo();
      _font.fromString(fontName);
      bool ok;
      _font = QFontDialog::getFont(&ok,_font);
      fontName = _font.toString();

      if (ok) {
        textFont.setValue(_font.toString());
      }

      mi.setGlobalMeta(topOfSteps.modelName,&textFont);

  } else if (selectedAction == colorAction) {

//      changeColor(topOfSteps,bottomOfSteps,&textColor);

      QColor _color = LDrawColor::color(textColor.value());
      _color = QColorDialog::getColor(_color,NULL);
      if (_color.isValid())
          textColor.setValue(_color.name());

      mi.setGlobalMeta(topOfSteps.modelName,&textColor);

  } else if (selectedAction == displayTextAction){

      displayText.setValue(false);
      mi.setGlobalMeta(topOfSteps.modelName,&displayText);
  }
}

void PagePageAttributeItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
  QGraphicsItem::mouseReleaseEvent(event);

  if (isSelected() && (flags() & QGraphicsItem::ItemIsMovable && positionChanged)) {

    QPointF newPosition;

    // back annotate the movement of the PLI into the LDraw file. (if ko - look at textItem - 103)
    newPosition = pos() - position;

    if (newPosition.x() || newPosition.y()) {
      positionChanged = true;

      PlacementData placementData = placement.value();
      placementData.offsets[0] += newPosition.x()/relativeToSize[0];
      placementData.offsets[1] += newPosition.y()/relativeToSize[1];
      placement.setValue(placementData);

      changePlacementOffset(page->bottomOfSteps(),&placement,relativeType,false,false,true);
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

  if (textValueChanged) {
    QStringList list = toPlainText().split("\n");
    content.setValue(list.join("\\n"));
    MetaItem mi;
    Where topLevelFile = page->topOfSteps();
    mi.setGlobalMeta(topLevelFile.modelName,&content);
  }
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
