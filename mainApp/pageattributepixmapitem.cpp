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
#include <QMenu>
#include <QAction>
#include <QGraphicsRectItem>
#include <QGraphicsSceneContextMenuEvent>

#include "pageattributepixmapitem.h"
#include "commonmenus.h"
#include "step.h"
#include "ranges.h"

PageAttributePixmapItem::PageAttributePixmapItem(
  Page                      *_page,
  QPixmap                   &pixmap,
  PageAttributePictureMeta  &pageAttributePictureMeta,
  QGraphicsItem             *parent)

{
  page              = _page;
  placement         = pageAttributePictureMeta.placement;
  picScale          = pageAttributePictureMeta.picScale;
  displayPicture    = pageAttributePictureMeta.display;
  margin            = pageAttributePictureMeta.margin;
  relativeType      = pageAttributePictureMeta.type;
  size[0]           = pixmap.width() *pageAttributePictureMeta.picScale.value();
  size[1]           = pixmap.height()*pageAttributePictureMeta.picScale.value();
  parentRelativeType= page->relativeType;

  switch(relativeType){
  case PageDocumentLogoType:
      name    = "Logo";
      setToolTip("Logo - right-click to modify");
      break;
  case PageCoverImageType:
      name    = "Cover Image";
      setToolTip("Cover Image - right-click to modify");
      break;
  case PagePlugImageType:
      name    = "Plug Image";
      setToolTip("Plug Image - right-click to modify");
      break;
  }

  setZValue(500);
  setParentItem(parent);
  setPixmap(pixmap);
  setFlag(QGraphicsItem::ItemIsSelectable,true);
  setFlag(QGraphicsItem::ItemIsMovable,true);
}

void PageAttributePixmapItem::change()
{
  if (isSelected() && (flags() & QGraphicsItem::ItemIsMovable)) {

    Where topOfSteps              = page->topOfSteps();
    Where bottomOfSteps           = page->bottomOfSteps();

    if (positionChanged) {

      beginMacro(QString("DragPicture"));

      qreal topLeft[2] = { sceneBoundingRect().left(),  sceneBoundingRect().top() };
      qreal size[2]    = { sceneBoundingRect().width(), sceneBoundingRect().height() };
      calcOffsets(placement.value(),placement.value().offsets,topLeft,size);

      logInfo() << "\nCHANGE PICTURE META - "
                << "\nPAGE WHERE - "
                << " \nPage TopOf (Model Name): "    << topOfSteps.modelName
                << " \nPage TopOf (Line Number): "   << topOfSteps.lineNumber
                << " \nPage BottomOf (Model Name): " << bottomOfSteps.modelName
                << " \nPage BottomOf (Line Number): "<< bottomOfSteps.lineNumber
                << "\nUSING PLACEMENT DATA - "
                << " \nPlacement: "                 << PlacNames[placement.value().placement]     << " (" << placement.value().placement << ")"
                << " \nJustification: "             << PlacNames[placement.value().justification] << " (" << placement.value().justification << ")"
                << " \nPreposition: "               << PrepNames[placement.value().preposition]   << " (" << placement.value().justification << ")"
                << " \nRelativeTo: "                << RelNames[placement.value().relativeTo]     << " (" << placement.value().relativeTo << ")"
                << " \nRectPlacement: "             << RectNames[placement.value().rectPlacement] << " (" << placement.value().rectPlacement << ")"
                << " \nOffset[0]: "                 << placement.value().offsets[0]
                << " \nOffset[1]: "                 << placement.value().offsets[1]
                << "\nOTHER DATA - "
                << " \nRelativeType: "               << RelNames[relativeType]       << " (" << relativeType << ")"
                << " \nParentRelativeType: "         << RelNames[parentRelativeType] << " (" << parentRelativeType << ")"
                ;

      changePlacementOffset(topOfSteps,
                           &placement,
                            relativeType);

      endMacro();

    } else if (sizeChanged) {

        beginMacro(QString("Resize"));

        qreal topLeft[2] = { sceneBoundingRect().left(),  sceneBoundingRect().top() };
        qreal size[2]    = { sceneBoundingRect().width(), sceneBoundingRect().height() };
        calcOffsets(placement.value(),placement.value().offsets,topLeft,size);

        changePlacementOffset(topOfSteps,
                             &placement,
                              relativeType);

        picScale.setValue(picScale.value()*oldScale);
        changeFloat(topOfSteps,bottomOfSteps,&picScale, 1, false);

        logInfo() << "\nRESIZE PICTURE META - "                    
                  << "\nPICTURE DATA - "
                  << " \npicScale: "                   << picScale.value()
                  << " \nMargin X: "                   << margin.value(0)
                  << " \nMargin Y: "                   << margin.value(1)
                  << " \nDisplay: "                    << displayPicture.value()
                  << "\nPAGE WHERE - "
                  << " \nPage TopOf (Model Name): "    << topOfSteps.modelName
                  << " \nPage TopOf (Line Number): "   << topOfSteps.lineNumber
                  << " \nPage BottomOf (Model Name): " << bottomOfSteps.modelName
                  << " \nPage BottomOf (Line Number): "<< bottomOfSteps.lineNumber
                  << "\nUSING PLACEMENT DATA - "
                  << " \nPlacement: "                  << PlacNames[placement.value().placement]     << " (" << placement.value().placement << ")"
                  << " \nJustification: "              << PlacNames[placement.value().justification] << " (" << placement.value().justification << ")"
                  << " \nPreposition: "                << PrepNames[placement.value().preposition]   << " (" << placement.value().justification << ")"
                  << " \nRelativeTo: "                 << RelNames[placement.value().relativeTo]     << " (" << placement.value().relativeTo << ")"
                  << " \nRectPlacement: "              << RectNames[placement.value().rectPlacement] << " (" << placement.value().rectPlacement << ")"
                  << " \nOffset[0]: "                  << placement.value().offsets[0]
                  << " \nOffset[1]: "                  << placement.value().offsets[1]
                  << "\nMETA WHERE - "
                  << " \nMeta Here (Model Name): "     << placement.here().modelName
                  << " \nMeta Here (Line Number): "    << placement.here().lineNumber
                  << "\nOTHER DATA - "
                  << " \nRelativeType: "               << RelNames[relativeType]       << " (" << relativeType << ")"
                  << " \nParentRelativeType: "         << RelNames[parentRelativeType] << " (" << parentRelativeType << ")"
                  ;

        endMacro();
    }
  }
}

void PageAttributePixmapItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
  QMenu menu;

  PlacementData placementData   = placement.value();
  QAction *placementAction      = commonMenus.placementMenu(menu,name,
                                    commonMenus.naturalLanguagePlacementWhatsThis(relativeType,placementData,name));
  QAction *scaleAction          = commonMenus.scaleMenu(menu,name);
  QAction *marginAction         = commonMenus.marginMenu(menu,name);
  QAction *displayPictureAction = commonMenus.displayMenu(menu,name);
  QAction *selectedAction       = menu.exec(event->screenPos());

  Where topOfSteps              = page->topOfSteps();
  Where bottomOfSteps           = page->bottomOfSteps();
  bool multiStep                = parentRelativeType == StepGroupType;

  int  onPageType;
  if (page->coverPage && page->frontCover)
      onPageType = FrontCoverPage;
  else if(page->coverPage && page->backCover)
      onPageType = BackCoverPage;
  else
      onPageType = ContentPage;

  if (selectedAction == NULL) {
    return;
  } else if (selectedAction == placementAction) {

      logInfo() << "\nMOVE IMAGE - "
                << "\nPAGE- "
                << (multiStep ? " \nMulti-Step Page" : " \nSingle-Step Page")
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
                      true,                 //default
                      1,                    //default
                      true,true,            //default
                      onPageType);

   } else if (selectedAction == scaleAction) {

      changeFloatSpin("Scale " + name,
                      name + " Size",
                      topOfSteps,
                      bottomOfSteps,
                     &picScale);

   } else if (selectedAction == marginAction) {

      changeMargins(name + " Margins",
                    topOfSteps,
                    bottomOfSteps,
                   &margin);

  } else if (selectedAction == displayPictureAction){

      changeBool(topOfSteps,
                 bottomOfSteps,
                &displayPicture);
  }
}


