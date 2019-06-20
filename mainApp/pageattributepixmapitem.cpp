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
#include <QSize>
#include <QRect>
#include <QGraphicsRectItem>
#include <QGraphicsSceneContextMenuEvent>
#include "pageattributepixmapitem.h"
#include "commonmenus.h"
#include "meta.h"
#include "color.h"
#include "step.h"
#include "ranges.h"
#include "name.h"
#include "QsLog.h"

PageAttributePixmapItem::PageAttributePixmapItem(
  Page                      *_page,
  QPixmap                   &_pixmapPic,
  PageAttributePictureMeta  &_papMeta,
  QGraphicsItem             *parent)
{
  page           = _page;
  relativeType   = _papMeta.type;
  placement      = _papMeta.placement;
  border         = _papMeta.border;
  margin         = _papMeta.margin;
  displayPicture = _papMeta.display;
  picScale       = _papMeta.picScale;
  pixmapPic      = &_pixmapPic;

  fillMode       = _papMeta.fill.value();

  QPixmap *pixmap = nullptr;
  if (fillMode == Aspect) {
    size[XX]     = int(_pixmapPic.width()  * _papMeta.picScale.value());
    size[YY]     = int(_pixmapPic.height() * _papMeta.picScale.value());
  } else {
    size[XX]     = page->pageSize(XX,false/*adjusted*/,false/*forDivider*/);
    size[YY]     = page->pageSize(YY,false,false);
  }
  pixmap = new QPixmap(size[XX],size[YY]);

  adjustImage(
          pixmap,   /* size info only */
          _papMeta);

  if (relativeType == PageDocumentLogoType ) {
    name    = "Logo";
  } else if (relativeType == PageCoverImageType) {
    name    = "Cover Image";
  } else if (relativeType == PagePlugImageType) {
    name    = "Plug Image";
  }

  setToolTip(name + " - right-click to modify");
  setData(ObjectId, PageAttributePixmapObj);
  setPixmap(*pixmap);
  setParentItem(parent);
  setFlag(QGraphicsItem::ItemIsSelectable,true);
  setFlag(QGraphicsItem::ItemIsMovable,true);
}

void PageAttributePixmapItem::adjustImage(
        QPixmap                  *pixmap,
        PageAttributePictureMeta &_papMeta)
{
    parentRelativeType = page->relativeType;

    pixmap->fill(Qt::transparent);

    // set painter and render hints (initialized with pixmap)
    QPainter painter;
    painter.begin(pixmap);
    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
    painter.setRenderHints(QPainter::Antialiasing,true);

    // Set border
    BorderData  borderData = _papMeta.border.valuePixels();

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

    int bt = int(borderData.thickness);

    QRectF prect(bt/2,bt/2,pixmap->width()-bt,pixmap->height()-bt);

    pen.setWidth(bt);

    painter.setPen(pen);
    painter.setBrush(brushColor);

    if (borderData.type == BorderData::BdrRound) {
      painter.drawRoundRect(prect,int(rx),int(ry));
    } else {
      painter.drawRect(prect);
    }

    // Adjust and draw image
    QImage image(pixmapPic->toImage());
    if (fillMode == Stretch) {
        pen.setCosmetic(true);
        QSize psize = pixmap->size();
        QSize isize = image.size();
        qreal sx = psize.width();
        qreal sy = psize.height();
        sx /= isize.width();
        sy /= isize.height();
        painter.scale(sx,sy);
        painter.drawImage(0,0,image);
    } else if (fillMode == Tile) {
        for (int y = 0; y < pixmap->height(); y += image.height()) {
            for (int x = 0; x < pixmap->width(); x += image.width()) {
                painter.drawImage(x,y,image);
            }
        }
    } else {  // Aspect
        painter.drawImage(0,0,image);
    }
    painter.end();
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

#ifdef QT_DEBUG_MODE
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
#endif
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

#ifdef QT_DEBUG_MODE
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
#endif
        endMacro();
    }
  }
}

void PageAttributePixmapItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
  QMenu menu;

  QAction *placementAction      = nullptr;
  QAction *scaleAction          = nullptr;
  QAction *stretchAction        = nullptr;
  QAction *tileAction           = nullptr;
  if (fillMode == Aspect) {
    PlacementData placementData = placement.value();
    placementAction             = commonMenus.placementMenu(menu,name,
                                  commonMenus.naturalLanguagePlacementWhatsThis(relativeType,placementData,name));
    scaleAction                 = commonMenus.scaleMenu(menu,name);
    stretchAction               = commonMenus.stretchImageMenu(menu,name);
    tileAction                  = commonMenus.tileImageMenu(menu,name);
  }
  if (fillMode == Tile)
    stretchAction               = commonMenus.stretchImageMenu(menu,name);
  if (fillMode == Stretch)
    tileAction                  = commonMenus.tileImageMenu(menu,name);
  QAction *imageAction          = commonMenus.changeImageMenu(menu,"Change " + name);
  QAction *borderAction         = commonMenus.borderMenu(menu,name);
  QAction *marginAction         = commonMenus.marginMenu(menu,name);
  QAction *displayPictureAction = commonMenus.displayMenu(menu, "Hide " + name);
  QAction *deleteImageAction    = commonMenus.deleteImageMenu(menu,name);

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

  QAction *selectedAction = menu.exec(event->screenPos());

  if (selectedAction == nullptr) {
    return;
  } else if (selectedAction == placementAction) {
#ifdef QT_DEBUG_MODE
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
#endif
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

   } else if (selectedAction == imageAction) {

      changeImage("Change " + name,
                   topOfSteps,
                   bottomOfSteps,
                   &page->meta.LPub.page.coverImage.file);
    } else if (selectedAction == borderAction) {
      changeBorder(name + " Border",
                   topOfSteps,
                   bottomOfSteps,
                   &border,
                   true,1,true,false,true/*corners*/);
    } else if (selectedAction == marginAction) {

      changeMargins(name + " Margins",
                    topOfSteps,
                    bottomOfSteps,
                   &margin);

  } else if (selectedAction == displayPictureAction){

      changeBool(topOfSteps,
                 bottomOfSteps,
                &displayPicture,
                 true,1,true/*allowLocal*/,false/*askLocal*/);

  } else if (selectedAction == stretchAction){

      page->meta.LPub.page.coverImage.fill.setValue(Stretch);
      changeImageFill(topOfSteps,
                      bottomOfSteps,
                     &page->meta.LPub.page.coverImage.fill);

  } else if (selectedAction == tileAction){

      page->meta.LPub.page.coverImage.fill.setValue(Tile);
      changeImageFill(topOfSteps,
                      bottomOfSteps,
                     &page->meta.LPub.page.coverImage.fill);

  } else if (selectedAction == deleteImageAction){

      deleteMeta(page->meta.LPub.page.coverImage.file.here());
  }
}
