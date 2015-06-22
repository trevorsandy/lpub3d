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
#include "name.h"
#include "placementdialog.h"
#include "commonmenus.h"

#include "ranges.h"
#include "step.h"

void PageAttributeItem::setAttributes(
  PlacementType              _relativeType,
  PlacementType              _parentRelativeType,
  Page                      *_page,
  PageAttributeTextMeta     &_pageAttributeText,
  QString                   &toolTip,
  QGraphicsItem             *_parent)
{
    page                  =  _page;
    relativeType          =  _relativeType;
    parentRelativeType    =  _parentRelativeType;

    textFont              = &_pageAttributeText.textFont;
    textColor             = &_pageAttributeText.textColor;
    margin                = &_pageAttributeText.margin;
    placement             = &_pageAttributeText.placement;
    displayText           = &_pageAttributeText.display;
    content               = &_pageAttributeText.content;

    QFont qfont;
    qfont.fromString(_pageAttributeText.textFont.valueFoo());
    setFont(qfont);

    QString text;
    //text = content->value();
    QRegExp rx("\\\\n");
    QStringList list = content->value().split(rx);   //rem this if ko
    text = list.join("\n");
    setPlainText(text);

    setToolTip(toolTip);
    setDefaultTextColor(LDrawColor::color(textColor->value()));
    setTextInteractionFlags(Qt::TextEditorInteraction);
    setFlag(QGraphicsItem::ItemIsMovable);
    setFlag(QGraphicsItem::ItemIsSelectable);
    setZValue(1000);
    setParentItem(_parent);
}

PageAttributeItem::PageAttributeItem()
{
  relativeType  = PageAttributeTextType;
  textFont      = NULL;
  textColor     = NULL;
  margin        = NULL;
  content       = NULL;
}

PageAttributeItem::PageAttributeItem(
  PlacementType              _relativeType,
  PlacementType              _parentRelativeType,
  Page                      *_page,
  PageAttributeTextMeta     &_pageAttributeText,
  QString                   &_toolTip,
  QGraphicsItem             *_parent)
{
  setAttributes(_relativeType,
                _parentRelativeType,
                _page,
                _pageAttributeText,
                _toolTip,
                _parent);
}

PagePageAttributeItem::PagePageAttributeItem(
  Page                      *_page,
  PageAttributeTextMeta     &_pageAttributeText,
  QGraphicsItem             *_parent)
{
  page          = _page;
  QString       toolTip;
  PlacementType pageAttributeType;
  switch(_pageAttributeText.type)
  {
  case PageTitleType:
      pageAttributeType = PageTitleType;
      toolTip           = tr("Title text - right-click to modify");
      break;
  case PageModelNameType:
      pageAttributeType = PageModelNameType;
      toolTip           = tr("Model Name text - right-click to modify");
      break;
  case PageAuthorType:
      pageAttributeType = PageAuthorType;
      toolTip           = tr("Author text - right-click to modify");
      break;
  case PageURLType:
      pageAttributeType = PageURLType;
      toolTip           = tr("URL text - right-click to modify");
      break;
  case PageModelDescType:
      pageAttributeType = PageModelDescType;
      toolTip           = tr("Model Description text - right-click to modify");
      break;
  case PagePublishDescType:
      pageAttributeType = PagePublishDescType;
      toolTip           = tr("Publish Description text - right-click to modify");
      break;
  case PageCopyrightType:
      pageAttributeType = PageCopyrightType;
      toolTip           = tr("Copyright text - right-click to modify");
      break;
  case PageEmailType:
      pageAttributeType = PageEmailType;
      toolTip           = tr("Email text - right-click to modify");
      break;
  case PageDisclaimerType:
      pageAttributeType = PageDisclaimerType;
      toolTip           = tr("Disclaimer text - right-click to modify");
      break;
  case PagePiecesType:
      pageAttributeType = PagePiecesType;
      toolTip           = tr("Pieces text - right-click to modify");
      break;
  case PagePlugType:
      pageAttributeType = PagePlugType;
      toolTip           = tr("Plug text - right-click to modify");
      break;
  case PageCategoryType:
      pageAttributeType = PageCategoryType;
      toolTip           = tr("Category text - right-click to modify");
      break;
  }

  setAttributes(pageAttributeType,
                SingleStepType,
                _page,
                _pageAttributeText,
                 toolTip,
                _parent);
}

void PagePageAttributeItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
  QMenu menu;

  PlacementData placementData = placement->value();
  QString name = "Move Attribute";
  QAction *placementAction        = menu.addAction(name);
  QAction *fontAction             = menu.addAction("Edit Font");
  QAction *colorAction            = menu.addAction("Edit Color");
  QAction *marginAction           = menu.addAction("Edit margins");
  QAction *doNotDisplayTextAction = menu.addAction("Do not display");

  fontAction->setWhatsThis("Edit this attribute's font");
  colorAction->setWhatsThis("Edit this attribute's colour");
  marginAction->setWhatsThis("Edit the margin space around this attribute");
  doNotDisplayTextAction->setWhatsThis("Do not display this attribute");
  placementAction->setWhatsThis(
    commonMenus.naturalLanguagePlacementWhatsThis(PageAttributeTextType,placementData,name));

  Where topOfSteps          = page->topOfSteps();
  Where bottomOfSteps       = page->bottomOfSteps();

  QAction *selectedAction   = menu.exec(event->screenPos());

  if (selectedAction == NULL) {

    return;

  } else if (selectedAction == placementAction) {

    changePlacement(PageType,
                    PageAttributeTextType,  // likely not resolvable
                    "Move Page Attribute",
                    topOfSteps,
                    bottomOfSteps,
                    placement);         //used to be &placement (before making placement a pointer)

  } else if (selectedAction == marginAction) {

      changeMargins("Page Attribute Margins",
                    topOfSteps,bottomOfSteps,margin);
  } else if (selectedAction == fontAction) {

      changeFont(topOfSteps,bottomOfSteps,textFont);

  } else if (selectedAction == colorAction) {

      changeColor(topOfSteps,bottomOfSteps,textColor);

  } else if (selectedAction == doNotDisplayTextAction){

      displayText->setValue(false);

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

      PlacementData placementData = placement->value();

      placementData.offsets[0] += newPosition.x()/relativeToSize[0];
      placementData.offsets[1] += newPosition.y()/relativeToSize[1];

      placement->setValue(placementData);

      changePlacementOffset(page->bottomOfSteps(),placement,PageAttributeTextType);  //used to be &placement (before making placement a pointer)
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
    content->setValue(list.join("\\n"));

    changePlacementOffset(page->bottomOfSteps(),placement,PageAttributeTextType); //used to be &placement (before making placement a pointer)
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
