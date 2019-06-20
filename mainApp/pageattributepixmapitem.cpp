/****************************************************************************
**
** Copyright (C) 2015 - 2019 Trevor SANDY. All rights reserved.
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
#include "color.h"
#include "step.h"
#include "ranges.h"
#include "name.h"

PageAttributePixmapItem::PageAttributePixmapItem(
  Page                      *_page,
  QPixmap                   &_pixmapPic,
  PageAttributePictureMeta  &papMeta,
  QGraphicsItem             *parent)

{
  page               = _page;
  parentRelativeType = page->relativeType;
  displayPicture     = papMeta.display;
  placement          = papMeta.placement;
  picScale           = papMeta.picScale;
  border             = papMeta.border;
  margin             = papMeta.margin;
  relativeType       = papMeta.type;
  size[XX]           = _pixmapPic.width()  * papMeta.picScale.value();
  size[YY]           = _pixmapPic.height() * papMeta.picScale.value();

  if (relativeType == PageDocumentLogoType ) {
      name    = "Logo";
      setToolTip("Logo - right-click to modify");
    } else if (relativeType == PageCoverImageType) {
      name    = "Cover Image";
      setToolTip("Cover Image - right-click to modify");
    } else if (relativeType == PagePlugImageType) {
      name    = "Plug Image";
      setToolTip("Plug Image - right-click to modify");
    }

  QPixmap *pixmap = new QPixmap(size[XX],size[YY]);

  BorderData  borderData = papMeta.border.valuePixels();

  int bt = int(borderData.thickness);

  QRectF prect(bt/2,bt/2,pixmap->width()-bt,pixmap->height()-bt);
  pixmap->fill(Qt::transparent);

  // set painter and render hints (initialized with pixmap)
  QPainter painter;
  painter.begin(pixmap);
  painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
  painter.setRenderHints(QPainter::Antialiasing,true);

  QImage image(_pixmapPic.toImage());
  image = image.convertToFormat(QImage::Format_ARGB32_Premultiplied);

  if (papMeta.stretch.value()) {                     // stretch
      QSize psize = pixmap->size();
      QSize isize = image.size();
      qreal sx = psize.width();
      qreal sy = psize.height();
      sx /= isize.width();
      sy /= isize.height();
      painter.scale(sx,sy);
      painter.drawImage(0,0,image);
  } else if (papMeta.tile.value()) {                  // tile
      for (int y = 0; y < pixmap->height(); y += image.height()) {
          for (int x = 0; x < pixmap->width(); x += image.width()) {
              painter.drawImage(x,y,image);
            }
        }
  } else {                                            // aspect
      setPixmap(_pixmapPic);
  }


  qreal rx = double(borderData.radius);
  qreal ry = double(borderData.radius);
  qreal dx = pixmap->width();
  qreal dy = pixmap->height();

  if (dx && dy) {
    if (dx > dy) {
      // the rx is going to appear larger that ry, so decrease rx based on ratio
      rx *= dy;
      rx /= dx;
    } else {
      ry *= dx;
      ry /= dy;
    }
  }

  QColor penColor;
  QColor brushColor = Qt::transparent;
  if (borderData.type == BorderData::BdrNone) {
      penColor = Qt::transparent;
   } else {
      penColor =  LDrawColor::color(borderData.color);
   }

  QPen pen;
  pen.setColor(penColor);
  pen.setCapStyle(Qt::RoundCap);
  pen.setJoinStyle(Qt::RoundJoin);
  if (borderData.line == BorderData::BdrLnNone){
        pen.setStyle(Qt::NoPen);
    }
  else if (borderData.line == BorderData::BdrLnSolid){
        pen.setStyle(Qt::SolidLine);
    }
  else if (borderData.line == BorderData::BdrLnDash){
      pen.setStyle(Qt::DashLine);
    }
  else if (borderData.line == BorderData::BdrLnDot){
      pen.setStyle(Qt::DotLine);
    }
  else if (borderData.line == BorderData::BdrLnDashDot){
      pen.setStyle(Qt::DashDotLine);
    }
  else if (borderData.line == BorderData::BdrLnDashDotDot){
      pen.setStyle(Qt::DashDotDotLine);
    }
  pen.setWidth(bt);

  painter.setPen(pen);
  painter.setBrush(brushColor);

  if (borderData.type == BorderData::BdrRound) {
    painter.drawRoundRect(prect,int(rx),int(ry));
  } else {
    painter.drawRect(prect);
  }
  painter.end();

  setData(ObjectId, PageAttributePixmapObj);
  // setZValue(PAGEATTRIBUTEPIXMAP_ZVALUE_DEFAULT); // set from formatpage
  setParentItem(parent);
  //setPixmap(pixmap);
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

  if (selectedAction == nullptr) {
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


