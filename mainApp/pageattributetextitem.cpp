/****************************************************************************
**
** Copyright (C) 2016 Trevor SANDY. All rights reserved.
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

#include "pageattributeTextitem.h"
#include "metatypes.h"
#include "color.h"
#include "scaledialog.h"
#include "name.h"
#include "placementdialog.h"
#include "commonmenus.h"

#include "ranges.h"
#include "step.h"

void PageAttributeTextItem::setAttributes(
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

PageAttributeTextItem::PageAttributeTextItem(
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
      toolTip           = tr("Title - click to edit, right-click to modify");
      break;
  case PageModelNameType:
      name              = tr("Model ID");
      toolTip           = tr("Model ID - click to edit, right-click to modify");
      break;
  case PageAuthorType:
      name              = tr("Author");
      toolTip           = tr("Author - click to edit, right-click to modify");
      break;
  case PageURLType:
      name              = tr("URL");
      toolTip           = tr("URL - click to edit, right-click to modify");
      break;
  case PageModelDescType:
      name              = tr("Model Description");
      toolTip           = tr("Model Description - click to edit, right-click to modify");
      break;
  case PagePublishDescType:
      name              = tr("Publish Description");
      toolTip           = tr("Publish Description - click to edit, right-click to modify");
      break;
  case PageCopyrightType:
      name              = tr("Copyright");
      toolTip           = tr("Copyright - click to edit, right-click to modify");
      break;
  case PageEmailType:
      name              = tr("Email");
      toolTip           = tr("Email - click to edit, right-click to modify");
      break;
  case PageDisclaimerType:
      name              = tr("LEGO Disclaimer");
      toolTip           = tr("Disclaimer - click to edit, right-click to modify");
      break;
  case PagePiecesType:
      name              = tr("Pieces");
      toolTip           = tr("Pieces - click to edit, right-click to modify");
      break;
  case PagePlugType:
      name              = tr("Plug");
      toolTip           = tr("Plug - click to edit, right-click to modify");
      break;
  case PageCategoryType:
      name              = tr("Category");
      toolTip           = tr("Category - click to edit, right-click to modify");
      break;
    default:
      break;
  }

  //relativeType  = PageTitleType;
  setFlag(QGraphicsItem::ItemIsMovable,true);
  setFlag(QGraphicsItem::ItemIsSelectable,true);

  setAttributes(_pageAttributeText.type,
                 page->relativeType,
                _pageAttributeText,
                 toolTip,
                _parent,
                 name);
}

void PageAttributeTextItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
  QMenu menu;

  PlacementData placementData = placement.value();
  QAction *placementAction    = commonMenus.placementMenu(menu,name,
                                    commonMenus.naturalLanguagePlacementWhatsThis(relativeType,placementData,name));
  QAction *fontAction         = commonMenus.fontMenu(menu,name);
  QAction *colorAction        = commonMenus.colorMenu(menu,name);
  QAction *marginAction       = commonMenus.marginMenu(menu,name);

  //QAction *displayTextAction  = commonMenus.displayMenu(menu,name);

  int  onPageType;
  if (page->coverPage && page->frontCover){
      onPageType = FrontCoverPage;
      //logInfo() << " On Front Cover Page";
  }
  else if(page->coverPage && page->backCover){
      onPageType = BackCoverPage;
      //logInfo() << " On Back Cover Page";
  }
  else {
      onPageType = ContentPage;
      //logInfo() << " On Content Page";
  }

  //temporary until I figure a way to hide dependent attributes w/o crashing
  //TODO redesign attribute placement relativeTo scheme to allow hide w/o crash
  QAction *displayTextAction = NULL;
  if (onPageType == FrontCoverPage){
      if (relativeType == PageModelNameType || relativeType == PagePublishDescType){
          displayTextAction  = commonMenus.displayMenu(menu,name);
        }
  } else if (onPageType == BackCoverPage) {
      switch(relativeType)
      {
      case PageTitleType:
      case PageAuthorType:
      case PageCopyrightType:
      case PageURLType:
      case PageEmailType:
      case PageDisclaimerType:
      case PagePlugType:
          ;
          break;
      default:
            displayTextAction  = commonMenus.displayMenu(menu,name);
          break;
      }
  }

  QAction *selectedAction     = menu.exec(event->screenPos());

  Where topOfSteps            = page->topOfSteps();
  Where bottomOfSteps         = page->bottomOfSteps();
  bool  useTop                = parentRelativeType != StepGroupType;

  if (selectedAction == NULL) {
    return;
  } else if (selectedAction == placementAction) {

      logInfo() << "\nMOVE TEXT - "
                << "\nPAGE- "
                << (useTop ? " \nSingle-Step Page" : " \nMulti-Step Page")
                << "\nPAGE WHERE -                  "
                << " \nPage TopOf (Model Name):     " << topOfSteps.modelName
                << " \nPage TopOf (Line Number):    " << topOfSteps.lineNumber
                << " \nPage BottomOf (Model Name):  " << bottomOfSteps.modelName
                << " \nPage BottomOf (Line Number): " << bottomOfSteps.lineNumber
                << "\nUSING PLACEMENT DATA -        "
                << " \nPlacement:                   " << PlacNames[placement.value().placement]     << " (" << placement.value().placement << ")"
                << " \nJustification:               " << PlacNames[placement.value().justification] << " (" << placement.value().justification << ")"
                << " \nPreposition:                 " << PrepNames[placement.value().preposition]   << " (" << placement.value().justification << ")"
                << " \nRelativeTo:                  " << RelNames[placement.value().relativeTo]     << " (" << placement.value().relativeTo << ")"
                << " \nRectPlacement:               " << RectNames[placement.value().rectPlacement] << " (" << placement.value().rectPlacement << ")"
                << " \nOffset[0]:                   " << placement.value().offsets[0]
                << " \nOffset[1]:                   " << placement.value().offsets[1]
                << "\nOTHER DATA -                  "
                << " \nRelativeType:                " << RelNames[relativeType]       << " (" << relativeType << ")"
                << " \nParentRelativeType:          " << RelNames[parentRelativeType] << " (" << parentRelativeType << ")"
                << " \nOnPageType:                  " << (onPageType == 0 ? " \nContent Page" :
                                                          onPageType == 1 ? " \nFront Cover Page" :
                                                                            " \nBack Cover Page")  << " (" << onPageType << ")"
                                                   ;
      changePlacement(parentRelativeType,
                      relativeType,
                     "Move " + name,
                      topOfSteps,
                      bottomOfSteps,
                     &placement,
                      useTop,
                      1,                        //append     - default
                      true,                     //local      - default Possible TODO change to multiStep ? false : true (see StepNumber)
                      true,                     //checkLocal - default
                      onPageType);

  } else if (selectedAction == marginAction) {

      logInfo() << "\nCHANGE MARGIN (TEXT) - "
                << "\nPAGE- "
                << (useTop ? " \nSingle-Step Page" : " \nMulti-Step Page")
                             ;
      changeMargins(name + " Margins",
                    topOfSteps,
                    bottomOfSteps,
                   &margin,
                    useTop);

  } else if (selectedAction == fontAction) {
      logInfo() << "\nCHANGE FONT (TEXT) - "
                << "\nPAGE- "
                << (useTop ? " \nSingle-Step Page" : " \nMulti-Step Page")
                             ;
      changeFont(topOfSteps,
                 bottomOfSteps,
                &textFont,1, true,
                 useTop);

  } else if (selectedAction == colorAction) {
      logInfo() << "\nCHANGE COLOUR (TEXT) - "
                << "\nPAGE- "
                << (useTop ? " \nSingle-Step Page" : " \nMulti-Step Page")
                             ;
      changeColor(topOfSteps,
                  bottomOfSteps,
                 &textColor,1, true,
                  useTop);

  } else if (selectedAction == displayTextAction){

      logInfo() << "\nCHANGE DISPLAY (TEXT) - "
                << "\nPAGE- "
                << " \nRelativeType:  " << RelNames[relativeType]       << " (" << relativeType << ")"
                << " \nOnPageType:    " << (onPageType == 0 ? "Content Page" : onPageType == 1 ? "Front Cover Page" : "Back Cover Page")
                << (useTop ? " \nSingle-Step Page" : " \nMulti-Step Page")
                          ;

      changeBool(topOfSteps,
                 bottomOfSteps,
                &displayText,
                 useTop);
  }
}

void PageAttributeTextItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
  QGraphicsItem::mouseReleaseEvent(event);

  if (isSelected() && (flags() & QGraphicsItem::ItemIsMovable/* && positionChanged */)) {

    QPointF newPosition;

    Where topOfSteps    = page->topOfSteps();
    Where bottomOfSteps = page->bottomOfSteps();
    bool  useTop        = parentRelativeType != StepGroupType;

    newPosition = pos() - position;

    if (newPosition.x() || newPosition.y()) {

      positionChanged = true;

      PlacementData placementData = placement.value();
      placementData.offsets[0] += newPosition.x()/relativeToSize[0];
      placementData.offsets[1] += newPosition.y()/relativeToSize[1];
      placement.setValue(placementData);

      logInfo() << "\nDRAG TEXT- "
                << "\nPAGE- "
                << (useTop ? " \nSingle-Step Page" : " \nMulti-Step Page")
                << "\nPAGE WHERE -                  "
                << " \nPage TopOf (Model Name):     " << topOfSteps.modelName
                << " \nPage TopOf (Line Number):    " << topOfSteps.lineNumber
                << " \nPage BottomOf (Model Name):  " << bottomOfSteps.modelName
                << " \nPage BottomOf (Line Number): " << bottomOfSteps.lineNumber
                << "\nUSING PLACEMENT DATA -        "
                << " \nPlacement:                   " << PlacNames[placement.value().placement]     << " (" << placement.value().placement << ")"
                << " \nJustification:               " << PlacNames[placement.value().justification] << " (" << placement.value().justification << ")"
                << " \nPreposition:                 " << PrepNames[placement.value().preposition]   << " (" << placement.value().justification << ")"
                << " \nRelativeTo:                  " << RelNames[placement.value().relativeTo]     << " (" << placement.value().relativeTo << ")"
                << " \nRectPlacement:               " << RectNames[placement.value().rectPlacement] << " (" << placement.value().rectPlacement << ")"
                << " \nOffset[0]:                   " << placement.value().offsets[0]
                << " \nOffset[1]:                   " << placement.value().offsets[1]
                << "\nOTHER DATA -                  "
                << " \nRelativeType:                " << RelNames[relativeType]       << " (" << relativeType << ")"
                << " \nParentRelativeType:          " << RelNames[parentRelativeType] << " (" << parentRelativeType << ")"
                ;

      changePlacementOffset(useTop ? topOfSteps : bottomOfSteps,
                           &placement,
                            relativeType);
    }
  }
}

void PageAttributeTextItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
  QGraphicsItem::mousePressEvent(event);
  positionChanged = false;
  position = pos();
}

void PageAttributeTextItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
  QGraphicsItem::mouseMoveEvent(event);
  if (isSelected() && (flags() & QGraphicsItem::ItemIsMovable)) {
    positionChanged = true;
  }
}

void PageAttributeTextItem::focusInEvent(QFocusEvent *event)
{
  textValueChanged = false;
  QGraphicsTextItem::focusInEvent(event);

}

void PageAttributeTextItem::focusOutEvent(QFocusEvent *event)
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

void PageAttributeTextItem::keyPressEvent(QKeyEvent *event)
{
  textValueChanged = true;
  QGraphicsTextItem::keyPressEvent(event);
}
void PageAttributeTextItem::keyReleaseEvent(QKeyEvent *event)
{
  textValueChanged = true;
  QGraphicsTextItem::keyReleaseEvent(event);
}

PageAttributeTextItem::PageAttributeTextItem()
{
  relativeType  = PageTitleType;
  setFlag(QGraphicsItem::ItemIsMovable,true);
  setFlag(QGraphicsItem::ItemIsSelectable,true);
}


