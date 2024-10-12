/****************************************************************************
**
** Copyright (C) 2015 - 2024 Trevor SANDY. All rights reserved.
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

#include "pageattributetextitem.h"
#include "metatypes.h"
#include "color.h"
#include "scaledialog.h"
#include "declarations.h"
#include "placementdialog.h"
#include "commonmenus.h"
#include "lpub.h"

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

    setParentItem(_parent);

    QFont qfont;
    qfont.fromString(_pageAttributeText.textFont.valueFoo());
    setFont(qfont);

    QColor color(LDrawColor::color(textColor.value()));
    setDefaultTextColor(color);

    QRegExp rx("\\\\n");
    QStringList list = content.value().split(rx);
    QString  text = list.join("\n");

    QRegExp rx2("\\\\""");
    QStringList list2 = text.split(rx2);
    QString text2 = list2.join("""");

    setPlainText(text2);

    setTextInteractionFlags(Qt::TextEditorInteraction);

    setAcceptHoverEvents(true);
    setFlag(QGraphicsItem::ItemIsFocusable);
    setFlag(QGraphicsItem::ItemIsMovable);
    setFlag(QGraphicsItem::ItemIsSelectable);
    setToolTip(_toolTip);
    setData(ObjectId, PageAttributeTextObj);
    setZValue(PAGEATTRIBUTETEXT_ZVALUE_DEFAULT);
}

PageAttributeTextItem::PageAttributeTextItem(
  Page                      *_page,
  PageAttributeTextMeta     &_pageAttributeText,
  QGraphicsItem             *_parent):
    isHovered(false),
    mouseIsDown(false)
{
  page          = _page;

  int partCount = 0;
  bool partCountOk = true;

  QString       toolTip;
  QString       name;
  switch(_pageAttributeText.type)
  {
  case PageTitleType:
      name    = tr("Title");
      toolTip = tr("Title - click to edit, right-click to modify");
      break;
  case PageModelNameType:
      name    = tr("Model ID");
      toolTip = tr("Model ID - click to edit, right-click to modify");
      break;
  case PageAuthorType:
      name    = tr("Author");
      toolTip = tr("Author - click to edit, right-click to modify");
      break;
  case PageURLType:
      name    = tr("URL");
      toolTip = tr("URL - click to edit, right-click to modify");
      break;
  case PageModelDescType:
      name    = tr("Model Description");
      toolTip = tr("Model Description - click to edit, right-click to modify");
      break;
  case PagePublishDescType:
      name    = tr("Publish Description");
      toolTip = tr("Publish Description - click to edit, right-click to modify");
      break;
  case PageCopyrightType:
      name    = tr("Copyright");
      toolTip = tr("Copyright - click to edit, right-click to modify");
      break;
  case PageEmailType:
      name    = tr("Email");
      toolTip = tr("Author Email - click to edit, right-click to modify");
      break;
  case PageDisclaimerType:
      name    = tr("LEGO Disclaimer");
      toolTip = tr("LEGO Disclaimer - click to edit, right-click to modify");
      break;
  case PagePartsType:
      partCount = _pageAttributeText.content.value().split(" ").first().toInt(&partCountOk);
      partCountOk &= partCount == 1;
      name    = partCountOk ? tr("Part") : tr("Parts");
      toolTip = tr("Part count - click to edit, right-click to modify");
      break;
  case PagePlugType:
      name    = tr("Plug");
      toolTip = tr("%1 Plug - click to edit, right-click to modify").arg(VER_PRODUCTNAME_STR);
      break;
  case PageCategoryType:
      name    = tr("Category");
      toolTip = tr("Category - click to edit, right-click to modify");
      break;
    default:
      break;
  }

//  relativeType  = PageTitleType;
//  setFlag(QGraphicsItem::ItemIsMovable,true);
//  setFlag(QGraphicsItem::ItemIsSelectable,true);

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

  QAction *placementAction      = lpub->getAct("placementAction.1");
  PlacementData placementData   = placement.value();
  placementAction->setWhatsThis(commonMenus.naturalLanguagePlacementWhatsThis(relativeType,placementData,name));
  commonMenus.addAction(placementAction,menu);

  QAction *fontAction           = lpub->getAct("fontAction.1");
  commonMenus.addAction(fontAction,menu);

  QAction *colorAction          = lpub->getAct("colorAction.1");
  commonMenus.addAction(colorAction,menu);

  QAction *marginAction         = lpub->getAct("marginAction.1");
  commonMenus.addAction(marginAction,menu);

  //temporary until I figure a way to hide dependent attributes w/o crashing
  //TODO redesign attribute placement relativeTo scheme to allow hide w/o crash
  PageTypeEnc  pageType = ContentPage;
  QAction *displayTextAction    = nullptr;
  if (page->coverPage) {
      if (page->frontCover) {
          pageType = FrontCoverPage;
          if (relativeType == PageModelNameType || relativeType == PagePublishDescType) {
              displayTextAction = lpub->getAct("displayTextAction.1");
              commonMenus.addAction(displayTextAction,menu);
          }
      } else if(page->backCover) {
          pageType = BackCoverPage;
          switch(relativeType)
          {
              case PageTitleType:
              case PageAuthorType:
              case PageCopyrightType:
              case PageURLType:
              case PageEmailType:
              case PageDisclaimerType:
              case PagePlugType:
                  break;
              default:
                  displayTextAction = lpub->getAct("displayTextAction.1");
                  commonMenus.addAction(displayTextAction,menu);
                  break;
          }
      }
      //logInfo() << (page->coverPage ? "On Front Cover Page" : page->backCover ? "On Back Cover Page" : "On Content Page");
  }

  QAction *selectedAction = menu.exec(event->screenPos());

  Where topOfSteps        = page->topOfSteps();
  Where bottomOfSteps     = page->bottomOfSteps();
  bool  useTop            = parentRelativeType != StepGroupType;

  if (selectedAction == nullptr) {
    return;
  } else if (selectedAction == placementAction) {

#ifdef QT_DEBUG_MODE
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
                << " \nPageType:                    " << (pageType == ContentPage
                                                            ? " \nContent Page"
                                                            : pageType == FrontCoverPage
                                                                  ? " \nFront Cover Page"
                                                                  : pageType == BackCoverPage
                                                                        ? " \nBack Cover Page"
                                                                        : " \nDefault Page")
                                                      << " (" << pageType << ")"
                                                   ;
#endif
      placement.setPageType(pageType);
      changePlacement(parentRelativeType,
                      relativeType,
                      tr("Move %1").arg(name),
                      topOfSteps,
                      bottomOfSteps,
                     &placement,
                      useTop,
                      1,     //append     - default
                      true); //local      - default Possible TODO change to multiStep ? false : true (see StepNumber)


  } else if (selectedAction == marginAction) {
#ifdef QT_DEBUG_MODE
      logInfo() << "\nCHANGE MARGIN (TEXT) - "
                << "\nPAGE- "
                << (useTop ? " \nSingle-Step Page" : " \nMulti-Step Page")
                             ;
#endif
      changeMargins(tr("%1 Margins").arg(name),
                    topOfSteps,
                    bottomOfSteps,
                   &margin,
                    useTop);

  } else if (selectedAction == fontAction) {
#ifdef QT_DEBUG_MODE
      logInfo() << "\nCHANGE FONT (TEXT) - "
                << "\nPAGE- "
                << (useTop ? " \nSingle-Step Page" : " \nMulti-Step Page")
                             ;
#endif
      changeFont(topOfSteps,
                 bottomOfSteps,
                &textFont,1, true,
                 useTop);

  } else if (selectedAction == colorAction) {
#ifdef QT_DEBUG_MODE
      logInfo() << "\nCHANGE COLOUR (TEXT) - "
                << "\nPAGE- "
                << (useTop ? " \nSingle-Step Page" : " \nMulti-Step Page")
                             ;
#endif
      changeColor(topOfSteps,
                  bottomOfSteps,
                 &textColor,1, true,
                  useTop);

  } else if (selectedAction == displayTextAction) {
#ifdef QT_DEBUG_MODE
      logInfo() << "\nCHANGE DISPLAY (TEXT) - "
                << "\nPAGE- "
                << " \nRelativeType:  " << RelNames[relativeType]       << " (" << relativeType << ")"
                << " \nOnPageType:    " << (pageType == ContentPage
                                                ? "Content Page"
                                                : pageType == FrontCoverPage
                                                      ? "Front Cover Page"
                                                      : pageType == BackCoverPage
                                                            ? "Back Cover Page"
                                                            : "Default Page")
                                        << " (" << pageType << ")"
                << (useTop ? " \nSingle-Step Page" : " \nMulti-Step Page")
                          ;
#endif
      changeBool(topOfSteps,
                 bottomOfSteps,
                &displayText,
                 useTop);
  }
}

void PageAttributeTextItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
  mouseIsDown = true;
  positionChanged = false;
  position = pos();
  QGraphicsItem::mousePressEvent(event);
}

void PageAttributeTextItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
  if (isSelected() && (flags() & QGraphicsItem::ItemIsMovable)) {
    positionChanged = true;
  }
  QGraphicsItem::mouseMoveEvent(event);
}

void PageAttributeTextItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
  mouseIsDown = false;
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

#ifdef QT_DEBUG_MODE
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
#endif
          changePlacementOffset( useTop ? topOfSteps : bottomOfSteps,
                                &placement,
                                 relativeType);
        }
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
      Where topLevelFile = page->topOfSteps();

      QRegExp rx("\\n");
      QStringList list = toPlainText().split(rx);

      QStringList list2;
      Q_FOREACH (QString string, list) {
        string.replace("\"","\\\"");
        list2 << string;
        }

      content.setValue(list2.join("\\n"));

      beginMacro(QString("EditTextAttribute"));
      setGlobalMeta(topLevelFile.modelName,&content);
      endMacro();
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

void PageAttributeTextItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
  isHovered = !this->isSelected() && !mouseIsDown;
  QGraphicsItem::hoverEnterEvent(event);
}

void PageAttributeTextItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
  isHovered = false;
  QGraphicsItem::hoverLeaveEvent(event);
}

void PageAttributeTextItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
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
