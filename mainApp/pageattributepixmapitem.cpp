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
#include <QMenu>
#include <QAction>
#include <QGraphicsRectItem>
#include <QGraphicsSceneContextMenuEvent>

#include "pageattributepixmapitem.h"
#include "commonmenus.h"
#include "color.h"
#include "step.h"
#include "ranges.h"
#include "declarations.h"
#include "lpub_object.h"

PageAttributePixmapItem::PageAttributePixmapItem(
  Page                      *_page,
  QPixmap                   &pixmap,
  PageAttributeImageMeta    &pageAttributeImageMeta,
  QGraphicsItem             *parent):
    isHovered(false),
    mouseIsDown(false)
{
  page               = _page;
  placement          = pageAttributeImageMeta.placement;
  picScale           = pageAttributeImageMeta.picScale;
  border             = pageAttributeImageMeta.border;
  displayPicture     = pageAttributeImageMeta.display;
  margin             = pageAttributeImageMeta.margin;
  relativeType       = pageAttributeImageMeta.type;
  fillMode           = pageAttributeImageMeta.fill.value();
  parentRelativeType = page->relativeType;

  if (relativeType == PageDocumentLogoType ) {
    name    = QObject::tr("Logo Image");
  } else if (relativeType == PageCoverImageType) {
    name    = QObject::tr("Cover Image");
  } else if (relativeType == PagePlugImageType) {
    name    = QObject::tr("Plug Image");
  }
  setToolTip(QObject::tr("%1 [%2 x %3 px] - right-click to modify")
             .arg(name)
             .arg(pixmap.width())
             .arg(pixmap.height()));

  bool movable = true;
  if (fillMode == Aspect) {
    size[XX] = int(pixmap.width() * pageAttributeImageMeta.picScale.value());
    size[YY] = int(pixmap.height() * pageAttributeImageMeta.picScale.value());
  } else {
    movable  = false;
    size[XX] = page->pageSizeDiv(XX,false/*adjusted*/,false/*forDivider*/);
    size[YY] = page->pageSizeDiv(YY,false,false);
  }

  // create image from pixmap
  QImage image(pixmap.toImage());
  image = image.convertToFormat(QImage::Format_ARGB32_Premultiplied);

  if (fillMode != Aspect)
      pixmap = pixmap.scaled(size[XX],size[YY]);
  pixmap.fill(Qt::transparent);

  // set painter and render hints (initialized with pixmap)
  QPainter painter;
  painter.begin(&pixmap);
  painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
  painter.setRenderHints(QPainter::Antialiasing,true);

  // Set border
  BorderData  borderData = pageAttributeImageMeta.border.valuePixels();

  qreal rx = double(borderData.radius);
  qreal ry = double(borderData.radius);
  qreal dx = pixmap.width();
  qreal dy = pixmap.height();

  if (dx > 0 && dy > 0) {
    if (dx > dy) {
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

  QRectF prect(bt/2,bt/2,pixmap.width()-bt,pixmap.height()-bt);

  pen.setWidth(bt);

  painter.setPen(pen);
  painter.setBrush(brushColor);

  if (borderData.type == BorderData::BdrRound) {
    painter.drawRoundRect(prect,int(rx),int(ry));
  } else {
    painter.drawRect(prect);
  }

  // Adjust and draw image
  if (fillMode == Stretch) {                                         // stretch
      QSize psize = pixmap.size();
      QSize isize = image.size();
      qreal sx = psize.width();
      qreal sy = psize.height();
      sx /= isize.width();
      sy /= isize.height();
      painter.scale(sx,sy);
      painter.drawImage(0,0,image);
  } else if (fillMode == Tile) {                                    // tile
      for (int y = 0; y < pixmap.height(); y += image.height()) {
          for (int x = 0; x < pixmap.width(); x += image.width()) {
              painter.drawImage(x,y,image);
            }
        }
  } else {                                                          // aspect
      painter.drawImage(0,0,image);
  }
  painter.end();

  setParentItem(parent);
  setPixmap(pixmap);
  setAcceptHoverEvents(true);
  setFlag(QGraphicsItem::ItemIsFocusable,true);
  setFlag(QGraphicsItem::ItemIsSelectable,movable);
  setFlag(QGraphicsItem::ItemIsMovable,movable);
  setData(ObjectId, PageAttributePixmapObj);
  setZValue(PAGEATTRIBUTEPIXMAP_ZVALUE_DEFAULT);
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

void PageAttributePixmapItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
  mouseIsDown = true;
  position = pos();
  positionChanged = false;
  if (fillMode == Aspect) {
    if (event->button() == Qt::LeftButton){
      placeGrabbers();
    }
  }
  QGraphicsItem::mousePressEvent(event);
}

void PageAttributePixmapItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
  QMenu menu;

  QAction *placementAction    = nullptr;
  QAction *scaleAction        = nullptr;
  QAction *stretchImageAction = nullptr;
  QAction *tileImageAction    = nullptr;
  if (fillMode == Aspect) {
    placementAction           = lpub->getAct("placementAction.1");
    PlacementData placementData = placement.value();
    placementAction->setWhatsThis(commonMenus.naturalLanguagePlacementWhatsThis(relativeType,placementData,name));
    commonMenus.addAction(placementAction,menu,name);

    scaleAction               = lpub->getAct("scaleAction.1");
    commonMenus.addAction(scaleAction,menu,name);

    stretchImageAction        = lpub->getAct("stretchImageAction.1");
    commonMenus.addAction(stretchImageAction,menu,name);

    tileImageAction           = lpub->getAct("tileImageAction.1");
    commonMenus.addAction(tileImageAction,menu,name);
  }

  if (fillMode == Tile) {
    tileImageAction           = lpub->getAct("tileImageAction.1");
    commonMenus.addAction(tileImageAction,menu,name);
  }

  if (fillMode == Stretch) {
    stretchImageAction        = lpub->getAct("stretchImageAction.1");
    commonMenus.addAction(stretchImageAction,menu,name);
  }

  QAction *changeImageAction  = lpub->getAct("changeImageAction.1");
  commonMenus.addAction(changeImageAction,menu,name);

   QAction *borderAction      = lpub->getAct("borderAction.1");
  commonMenus.addAction(borderAction,menu,name);

  QAction *marginAction       = lpub->getAct("marginAction.1");
  commonMenus.addAction(marginAction,menu,name);

  QAction *displayImageAction = lpub->getAct("displayImageAction.1");
  commonMenus.addAction(displayImageAction,menu,name);

  QAction *deleteImageAction  = lpub->getAct("deleteImageAction.1");
  commonMenus.addAction(deleteImageAction,menu,name);

  Where topOfSteps    = page->topOfSteps();
  Where bottomOfSteps = page->bottomOfSteps();

  PageTypeEnc pageType = ContentPage;
  if (page->coverPage) {
    if (page->frontCover)
      pageType = FrontCoverPage;
    else if(page->backCover)
      pageType = BackCoverPage;
  }

  QAction *selectedAction     = menu.exec(event->screenPos());

  if (selectedAction == nullptr) {
    return;
  } else if (selectedAction == placementAction) {
#ifdef QT_DEBUG_MODE
      bool multiStep = parentRelativeType == StepGroupType;
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
                    QObject::tr("Move %1").arg(name),
                    topOfSteps,
                    bottomOfSteps,
                   &placement);

  } else if (selectedAction == scaleAction) {
    changeFloatSpin(QObject::tr("Scale %1").arg(name),
                    QObject::tr("%1 Size").arg(name),
                    topOfSteps,
                    bottomOfSteps,
                   &picScale,
                    0.01f,1,true, // step, append, checklocal
                    DoubleSpinScale);

  } else if (selectedAction == changeImageAction) {
    changeImage(QObject::tr("Change %1").arg(name),
                 topOfSteps,
                 bottomOfSteps,
                 &page->meta.LPub.page.coverImage.file);

  } else if (selectedAction == borderAction) {
    changeBorder(QObject::tr("%1 Border").arg(name),
                 topOfSteps,
                 bottomOfSteps,
                 &border,
                 true,1,true,false,true/*corners*/);

  } else if (selectedAction == marginAction) {
    changeMargins(QObject::tr("%1 Margins").arg(name),
                  topOfSteps,
                  bottomOfSteps,
                 &margin);

  } else if (selectedAction == displayImageAction){
    changeBool(topOfSteps,
               bottomOfSteps,
              &displayPicture,
               true,1,true/*allowLocal*/,false/*askLocal*/);

  } else if (selectedAction == stretchImageAction){
    page->meta.LPub.page.coverImage.fill.setValue(Stretch);
    changeImageFill(topOfSteps,
                    bottomOfSteps,
                   &page->meta.LPub.page.coverImage.fill);

  } else if (selectedAction == tileImageAction){
    page->meta.LPub.page.coverImage.fill.setValue(Tile);
    changeImageFill(topOfSteps,
                    bottomOfSteps,
                   &page->meta.LPub.page.coverImage.fill);

  } else if (selectedAction == deleteImageAction){
    deleteMeta(page->meta.LPub.page.coverImage.file.here());
  }
}

/* Highlight bounding rectangle on hover */

void PageAttributePixmapItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
  mouseIsDown = false;
  QGraphicsItem::mouseReleaseEvent(event);
}

void PageAttributePixmapItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
  isHovered = !this->isSelected() && !mouseIsDown;
  QGraphicsItem::hoverEnterEvent(event);
}

void PageAttributePixmapItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
  isHovered = false;
  QGraphicsItem::hoverLeaveEvent(event);
}

void PageAttributePixmapItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
  QPen pen;
  pen.setColor(isHovered ? QColor(Preferences::sceneGuideColor) : Qt::black);
  pen.setWidth(0/*cosmetic*/);
  pen.setStyle(isHovered ? Qt::PenStyle(Preferences::sceneGuidesLine) : Qt::NoPen);
  painter->setPen(pen);
  painter->setBrush(Qt::transparent);
  painter->drawRect(this->boundingRect());
  QGraphicsPixmapItem::paint(painter,option,widget);
}
