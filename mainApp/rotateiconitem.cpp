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

/****************************************************************************
 *
 * This class provides a simple mechanism for displaying a rotation icon
 * on the page.
 *
 ***************************************************************************/

#include <QMenu>
#include <QAction>
#include <QGraphicsRectItem>
#include <QGraphicsSceneContextMenuEvent>
#include "rotateiconitem.h"
#include "commonmenus.h"
#include "step.h"
#include "ranges.h"
#include "color.h"

void RotateIconItem::setAttributes(
  Step           *_step,
  PlacementType   _parentRelativeType,
  RotateIconMeta &_rotateIconMeta,
  QGraphicsItem   *parent)
{
  step               = _step;
  parentRelativeType = _parentRelativeType;
  rotateIconMeta     = _rotateIconMeta;
  picScale           = rotateIconMeta.picScale;

  placement          = rotateIconMeta.placement;
  margin             = rotateIconMeta.margin;

  // initialize pixmap using icon demensions
  pixmap             = new QPixmap(rotateIconMeta.size.valuePixels(XX),
                                   rotateIconMeta.size.valuePixels(YY));
  // set image size
  placementRotateIcon.sizeit();

  setRotateIconImage(pixmap);

  QString toolTip("Rotate Icon - right-click to modify");
  setToolTip(toolTip);

  setZValue(10000);
  setParentItem(parent);
  setPixmap(*pixmap);
  setTransformationMode(Qt::SmoothTransformation);
//  setFlag(QGraphicsItem::ItemIsMovable,true);
  setFlag(QGraphicsItem::ItemIsSelectable,true);

  delete pixmap;
}

RotateIconItem::RotateIconItem(){
  relativeType       = RotateIconType;
  step               = NULL;
  pixmap             = NULL;
//  rotateIconMeta     = NULL;
//  picScale           = NULL;
}

RotateIconItem::RotateIconItem(
    Step           *_step,
    PlacementType   _parentRelativeType,
    RotateIconMeta &_rotateIconMeta,
    QGraphicsItem  *_parent){

  setAttributes(
        _step,
        _parentRelativeType,
        _rotateIconMeta,
        _parent);
}

CalloutRotateIconItem::CalloutRotateIconItem(
    Step                  *_step,
    PlacementType          _parentRelativeType,
    CalloutRotateIconMeta &_rotateIconMeta,
    QGraphicsItem         *_parent)
{
  step =                   _step;

  rotateIconMeta.arrowColour      = _rotateIconMeta.arrowColour;
  rotateIconMeta.size             = _rotateIconMeta.size;
  rotateIconMeta.picScale         = _rotateIconMeta.picScale;
  rotateIconMeta.border           = _rotateIconMeta.border;
  rotateIconMeta.background       = _rotateIconMeta.background;
  rotateIconMeta.margin           = _rotateIconMeta.margin;
  rotateIconMeta.placement        = _rotateIconMeta.placement;
  rotateIconMeta.display          = _rotateIconMeta.display;
  rotateIconMeta.subModelColor    = _rotateIconMeta.subModelColor;

  setAttributes(
        _step,
        _parentRelativeType,
        rotateIconMeta,
        _parent);
}

MultiStepRotateIconItem::MultiStepRotateIconItem(
    Step                    *_step,
    PlacementType            _parentRelativeType,
    MultiStepRotateIconMeta &_rotateIconMeta,
    QGraphicsItem           *_parent)
{
  step =                     _step;

  rotateIconMeta.arrowColour      = _rotateIconMeta.arrowColour;
  rotateIconMeta.size             = _rotateIconMeta.size;
  rotateIconMeta.picScale         = _rotateIconMeta.picScale;
  rotateIconMeta.border           = _rotateIconMeta.border;
  rotateIconMeta.background       = _rotateIconMeta.background;
  rotateIconMeta.margin           = _rotateIconMeta.margin;
  rotateIconMeta.placement        = _rotateIconMeta.placement;
  rotateIconMeta.display          = _rotateIconMeta.display;
  rotateIconMeta.subModelColor    = _rotateIconMeta.subModelColor;

  setAttributes(
        _step,
        _parentRelativeType,
        rotateIconMeta,
        _parent);

}

void RotateIconItem::setRotateIconImage(QPixmap *pixmap)
{
  // set pixmap to transparent
  pixmap->fill(Qt::transparent);

  // set border and background parameters
  BorderData     borderData     = rotateIconMeta.border.valuePixels();
  BackgroundData backgroundData = rotateIconMeta.background.value();

  // set rectangle size and demensions parameters
  int ibt = int(borderData.thickness);
  QRectF irect(ibt/2,ibt/2,pixmap->width()-ibt,pixmap->height()-ibt);

  // set painter and render hints (initialized with pixmap)
  QPainter painter;
  painter.begin(pixmap);
  painter.setRenderHints(QPainter::HighQualityAntialiasing,true);
  painter.setRenderHints(QPainter::Antialiasing,true);

        /* BACKGROUND */

  // set icon background colour
  QColor brushColor;
  switch(backgroundData.type) {
    case BackgroundData::BgTransparent:
      brushColor = Qt::transparent;
      break;
    case BackgroundData::BgColor:
    case BackgroundData::BgSubmodelColor:
      if (backgroundData.type == BackgroundData::BgColor) {
          brushColor = LDrawColor::color(backgroundData.string);
        } else {
          brushColor = LDrawColor::color(rotateIconMeta.subModelColor.value(0));
        }
      break;
    case BackgroundData::BgImage:
      break;
    }
    painter.setBrush(brushColor);

      /* BORDER */

  // set icon border pen colour
  QPen borderPen;
  QColor borderPenColor;
  if (borderData.type == BorderData::BdrNone) {
      borderPenColor = Qt::transparent;
    } else {
      borderPenColor =  LDrawColor::color(borderData.color);
    }
  borderPen.setColor(borderPenColor);
  borderPen.setCapStyle(Qt::RoundCap);
  borderPen.setJoinStyle(Qt::RoundJoin);
  borderPen.setStyle(Qt::SolidLine);
  borderPen.setWidth(ibt);

  painter.setPen(borderPen);

  // set icon border demensions
  qreal rx = borderData.radius;
  qreal ry = borderData.radius;
  qreal dx = pixmap->width();
  qreal dy = pixmap->height();

  if (dx && dy) {
      if (dx > dy) {
          rx *= dy;
          rx /= dx;
        } else {
          ry *= dx;
          ry /= dy;
        }
    }

  // draw icon rectangle
  if (borderData.type == BorderData::BdrRound) {
      painter.drawRoundRect(irect,int(rx),int(ry));
    } else {
      painter.drawRect(irect);
    }

  /* ARROWS */

  // set arrow parts (head, tips etc...)
  qreal arrowTipLength = 9.0;
  qreal arrowTipHeight = 4.0;
  QPolygonF arrowHead;
  arrowHead << QPointF()
            << QPointF(arrowTipLength + 2.5, -arrowTipHeight)
            << QPointF(arrowTipLength      , 0.0)
            << QPointF(arrowTipLength + 2.5,  arrowTipHeight);

  // set default arrow pen and transfer to working arrow pen
  QColor arrowPenColor;
  arrowPenColor = LDrawColor::color(rotateIconMeta.arrowColour.value());
  QPen defaultArrowPen;
  int defaultLineWidth = 2;
  defaultArrowPen.setColor(arrowPenColor);
  defaultArrowPen.setCapStyle(Qt::SquareCap);
  defaultArrowPen.setJoinStyle(Qt::MiterJoin);
  defaultArrowPen.setStyle(Qt::SolidLine);
  defaultArrowPen.setWidth(defaultLineWidth);
  QPen arrowPen = defaultArrowPen;

  // set painter for arrows
  painter.setPen(arrowPen);
  painter.setBrush(Qt::transparent);

  // set arrow height and width parameters
  qreal aw = irect.width();
  qreal ah = irect.height() / 2.0;
  float inset = 6.0;

  float ix    = inset * 1.8;
  float iy    = inset * 2.5;

  // draw upper and lower arrow arcs
  QPainterPath path;

  QPointF start(     inset, ah - inset);
  QPointF end  (aw - inset, ah - inset);
  path.moveTo(start);
  path.cubicTo(start + QPointF( ix, -iy),   end + QPointF(-ix, -iy),end);

  start += QPointF(0, inset + inset);
  end   += QPointF(0, inset + inset);
  path.moveTo(end);
  path.cubicTo(end   + QPointF(-ix,  iy), start + QPointF( ix,  iy),start);

  painter.drawPath(path);

  // draw upper and lower arrow heads
  painter.setBrush(arrowPen.color());

  painter.save();
  painter.translate(aw - inset, ah - inset);
  painter.rotate(-140);
  painter.drawPolygon(arrowHead);
  painter.restore();

  painter.save();
  painter.translate(inset, ah + inset);
  painter.rotate(40);
  painter.drawPolygon(arrowHead);
  painter.restore();

  painter.end();
}


void RotateIconItem::contextMenuEvent(
    QGraphicsSceneContextMenuEvent *event)
{
  QMenu menu;

  PlacementData placementData = placement.value();

  QString pl = "Rotate Icon";
  QAction *placementAction  = commonMenus.placementMenu(menu,pl,
                                                        commonMenus.naturalLanguagePlacementWhatsThis(relativeType,placementData,pl));
  QAction *backgroundAction = commonMenus.backgroundMenu(menu,pl);
  QAction *borderAction     = commonMenus.borderMenu(menu,pl);
  QAction *marginAction     = commonMenus.marginMenu(menu,pl);
  QAction *displayAction    = commonMenus.displayMenu(menu,pl);

  QAction *editArrowAction = menu.addAction("Edit Arrow");
  editArrowAction->setWhatsThis("Edit this rotation icon arrows");

  QAction *deleteRotateIconAction = menu.addAction("Delete " +pl);
  deleteRotateIconAction->setWhatsThis("Delete this rotate icon");
  deleteRotateIconAction->setIcon(QIcon(":/resources/delete.png"));

  QAction *selectedAction  = menu.exec(event->screenPos());

  if (selectedAction == NULL) {
      return;
    }

  Where topOfStep     = step->topOfStep();
  Where bottomOfStep  = step->bottomOfStep();
  Where topOfSteps    = step->topOfSteps();
  Where bottomOfSteps = step->bottomOfSteps();

  if (selectedAction == placementAction) {

      bool multiStep                = parentRelativeType == StepGroupType;
      logInfo() << "\nMOVE ROTATE_ICON - "
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
                                                ;

      changePlacement(parentRelativeType,
                      SingleStepType,         //not using RotateIconType intentionally
                      pl+" Placement",
                      topOfSteps,
                      bottomOfSteps,
                      &rotateIconMeta.placement,true,1,0,false);
    } else if (selectedAction == backgroundAction) {
      changeBackground(pl+" Background",
                       topOfSteps,
                       bottomOfSteps,
                       &rotateIconMeta.background);
    } else if (selectedAction == borderAction) {
      changeBorder(pl+" Border",
                   topOfSteps,
                   bottomOfSteps,
                   &rotateIconMeta.border);
    } else if (selectedAction == marginAction) {
      changeMargins(pl+" Margins",
                    topOfSteps,
                    bottomOfSteps,
                    &rotateIconMeta.margin);
    } else if (selectedAction == displayAction){
      changeBool(topOfSteps,
                 bottomOfSteps,
                 &rotateIconMeta.display);
    } else if (selectedAction == editArrowAction) {

      //TODO
    } else if (selectedAction == deleteRotateIconAction) {
      beginMacro("DeleteRotateIcon");
      deleteMeta(topOfStep);
      endMacro();
    }
}

void RotateIconItem::change()
{
  if (isSelected() && (flags() & QGraphicsItem::ItemIsMovable)) {

    Where topOfSteps              = step->topOfSteps();
    Where bottomOfSteps           = step->bottomOfSteps();

    if (positionChanged) {

      beginMacro(QString("DragPicture"));

      qreal topLeft[2] = { sceneBoundingRect().left(),  sceneBoundingRect().top() };
      qreal size[2]    = { sceneBoundingRect().width(), sceneBoundingRect().height() };
      calcOffsets(placement.value(),placement.value().offsets,topLeft,size);

      logInfo() << "\nCHANGE ROTATE_ICON - "
                << "\nPAGE WHERE - "
                << " \nStep TopOf (Model Name): "    << topOfSteps.modelName
                << " \nStep TopOf (Line Number): "   << topOfSteps.lineNumber
                << " \nStep BottomOf (Model Name): " << bottomOfSteps.modelName
                << " \nStep BottomOf (Line Number): "<< bottomOfSteps.lineNumber
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

        logInfo() << "\nRESIZE ROTATE_ICON - "
                  << "\nPICTURE DATA - "
                  << " \npicScale: "                   << picScale.value()
                  << " \nMargin X: "                   << margin.value(0)
                  << " \nMargin Y: "                   << margin.value(1)
//                  << " \nDisplay: "                    << displayPicture.value()
                  << "\nPAGE WHERE - "
                  << " \nStep TopOf (Model Name): "    << topOfSteps.modelName
                  << " \nStep TopOf (Line Number): "   << topOfSteps.lineNumber
                  << " \nStep BottomOf (Model Name): " << bottomOfSteps.modelName
                  << " \nStep BottomOf (Line Number): "<< bottomOfSteps.lineNumber
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

void CalloutRotateIconItem::contextMenuEvent(
    QGraphicsSceneContextMenuEvent *event)
{
  QMenu menu;

  PlacementData placementData = placement.value();

  QString pl = "Rotate Icon";
  QAction *placementAction  = commonMenus.placementMenu(menu,pl,
                                                        commonMenus.naturalLanguagePlacementWhatsThis(relativeType,placementData,pl));
  QAction *backgroundAction = commonMenus.backgroundMenu(menu,pl);
  QAction *borderAction     = commonMenus.borderMenu(menu,pl);
  QAction *marginAction     = commonMenus.marginMenu(menu,pl);
  QAction *displayAction    = commonMenus.displayMenu(menu,pl);

  QAction *editArrowAction = menu.addAction("Edit Arrow");
  editArrowAction->setWhatsThis("Edit this rotation icon arrows");

  QAction *deleteRotateIconAction = menu.addAction("Delete " +pl);
  deleteRotateIconAction->setWhatsThis("Delete this rotate icon");
  deleteRotateIconAction->setIcon(QIcon(":/resources/delete.png"));

  QAction *selectedAction  = menu.exec(event->screenPos());

  if (selectedAction == NULL) {
      return;
    }

  Where topOfStep     = step->topOfStep();
  Where bottomOfStep  = step->bottomOfStep();
  Where topOfSteps    = step->topOfSteps();
  Where bottomOfSteps = step->bottomOfSteps();

  if (selectedAction == placementAction) {

      bool multiStep                = parentRelativeType == StepGroupType;
      logInfo() << "\nMOVE CALLOUT ROTATE_ICON - "
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
                                                ;

      changePlacement(parentRelativeType,
                      SingleStepType,         //not using RotateIconType intentionally
                      pl+" Placement",
                      topOfSteps,
                      bottomOfSteps,
                      &rotateIconMeta.placement,true,1,0,false);
    } else if (selectedAction == backgroundAction) {
      changeBackground(pl+" Background",
                       topOfSteps,
                       bottomOfSteps,
                       &rotateIconMeta.background);
    } else if (selectedAction == borderAction) {
      changeBorder(pl+" Border",
                   topOfSteps,
                   bottomOfSteps,
                   &rotateIconMeta.border);
    } else if (selectedAction == marginAction) {
      changeMargins(pl+" Margins",
                    topOfSteps,
                    bottomOfSteps,
                    &rotateIconMeta.margin);
    } else if (selectedAction == displayAction){
      changeBool(topOfSteps,
                 bottomOfSteps,
                 &rotateIconMeta.display);
    } else if (selectedAction == editArrowAction) {

      //TODO
    } else if (selectedAction == deleteRotateIconAction) {
      beginMacro("DeleteRotateIcon");
      deleteMeta(topOfStep);
      endMacro();
    }
}

void CalloutRotateIconItem::change()
{
  if (isSelected() && (flags() & QGraphicsItem::ItemIsMovable)) {

    Where topOfSteps              = step->topOfSteps();
    Where bottomOfSteps           = step->bottomOfSteps();

    if (positionChanged) {

      beginMacro(QString("DragPicture"));

      qreal topLeft[2] = { sceneBoundingRect().left(),  sceneBoundingRect().top() };
      qreal size[2]    = { sceneBoundingRect().width(), sceneBoundingRect().height() };
      calcOffsets(placement.value(),placement.value().offsets,topLeft,size);

      logInfo() << "\nCHANGE CALLOUT ROTATE_ICON - "
                << "\nPAGE WHERE - "
                << " \nStep TopOf (Model Name): "    << topOfSteps.modelName
                << " \nStep TopOf (Line Number): "   << topOfSteps.lineNumber
                << " \nStep BottomOf (Model Name): " << bottomOfSteps.modelName
                << " \nStep BottomOf (Line Number): "<< bottomOfSteps.lineNumber
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

        logInfo() << "\nRESIZE CALLOUT ROTATE_ICON - "
                  << "\nPICTURE DATA - "
                  << " \npicScale: "                   << picScale.value()
                  << " \nMargin X: "                   << margin.value(0)
                  << " \nMargin Y: "                   << margin.value(1)
//                  << " \nDisplay: "                    << displayPicture.value()
                  << "\nPAGE WHERE - "
                  << " \nStep TopOf (Model Name): "    << topOfSteps.modelName
                  << " \nStep TopOf (Line Number): "   << topOfSteps.lineNumber
                  << " \nStep BottomOf (Model Name): " << bottomOfSteps.modelName
                  << " \nStep BottomOf (Line Number): "<< bottomOfSteps.lineNumber
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

void MultiStepRotateIconItem::contextMenuEvent(
    QGraphicsSceneContextMenuEvent *event)
{
  QMenu menu;

  PlacementData placementData = placement.value();

  QString pl = "Rotate Icon";
  QAction *placementAction  = commonMenus.placementMenu(menu,pl,
                                                        commonMenus.naturalLanguagePlacementWhatsThis(relativeType,placementData,pl));
  QAction *backgroundAction = commonMenus.backgroundMenu(menu,pl);
  QAction *borderAction     = commonMenus.borderMenu(menu,pl);
  QAction *marginAction     = commonMenus.marginMenu(menu,pl);
  QAction *displayAction    = commonMenus.displayMenu(menu,pl);

  QAction *editArrowAction = menu.addAction("Edit Arrow");
  editArrowAction->setWhatsThis("Edit this rotation icon arrows");

  QAction *deleteRotateIconAction = menu.addAction("Delete " +pl);
  deleteRotateIconAction->setWhatsThis("Delete this rotate icon");
  deleteRotateIconAction->setIcon(QIcon(":/resources/delete.png"));

  QAction *selectedAction  = menu.exec(event->screenPos());

  if (selectedAction == NULL) {
      return;
    }

  Where topOfStep     = step->topOfStep();
  Where bottomOfStep  = step->bottomOfStep();
  Where topOfSteps    = step->topOfSteps();
  Where bottomOfSteps = step->bottomOfSteps();

  if (selectedAction == placementAction) {

      bool multiStep                = parentRelativeType == StepGroupType;
      logInfo() << "\nMOVE MULTISCREEN ROTATE_ICON - "
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
                                                ;

      changePlacement(parentRelativeType,
                      SingleStepType,         //not using RotateIconType intentionally
                      pl+" Placement",
                      topOfSteps,
                      bottomOfSteps,
                      &rotateIconMeta.placement,true,1,0,false);
    } else if (selectedAction == backgroundAction) {
      changeBackground(pl+" Background",
                       topOfSteps,
                       bottomOfSteps,
                       &rotateIconMeta.background);
    } else if (selectedAction == borderAction) {
      changeBorder(pl+" Border",
                   topOfSteps,
                   bottomOfSteps,
                   &rotateIconMeta.border);
    } else if (selectedAction == marginAction) {
      changeMargins(pl+" Margins",
                    topOfSteps,
                    bottomOfSteps,
                    &rotateIconMeta.margin);
    } else if (selectedAction == displayAction){
      changeBool(topOfSteps,
                 bottomOfSteps,
                 &rotateIconMeta.display);
    } else if (selectedAction == editArrowAction) {

      //TODO
    } else if (selectedAction == deleteRotateIconAction) {
      beginMacro("DeleteRotateIcon");
      deleteMeta(topOfStep);
      endMacro();
    }
}

void MultiStepRotateIconItem::change()
{
  if (isSelected() && (flags() & QGraphicsItem::ItemIsMovable)) {

    Where topOfSteps              = step->topOfSteps();
    Where bottomOfSteps           = step->bottomOfSteps();

    if (positionChanged) {

      beginMacro(QString("DragPicture"));

      qreal topLeft[2] = { sceneBoundingRect().left(),  sceneBoundingRect().top() };
      qreal size[2]    = { sceneBoundingRect().width(), sceneBoundingRect().height() };
      calcOffsets(placement.value(),placement.value().offsets,topLeft,size);

      logInfo() << "\nCHANGE MULTISCREEN ROTATE_ICON - "
                << "\nPAGE WHERE - "
                << " \nStep TopOf (Model Name): "    << topOfSteps.modelName
                << " \nStep TopOf (Line Number): "   << topOfSteps.lineNumber
                << " \nStep BottomOf (Model Name): " << bottomOfSteps.modelName
                << " \nStep BottomOf (Line Number): "<< bottomOfSteps.lineNumber
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

        logInfo() << "\nRESIZE MULTISCREEN ROTATE_ICON - "
                  << "\nPICTURE DATA - "
                  << " \npicScale: "                   << picScale.value()
                  << " \nMargin X: "                   << margin.value(0)
                  << " \nMargin Y: "                   << margin.value(1)
//                  << " \nDisplay: "                    << displayPicture.value()
                  << "\nPAGE WHERE - "
                  << " \nStep TopOf (Model Name): "    << topOfSteps.modelName
                  << " \nStep TopOf (Line Number): "   << topOfSteps.lineNumber
                  << " \nStep BottomOf (Model Name): " << bottomOfSteps.modelName
                  << " \nStep BottomOf (Line Number): "<< bottomOfSteps.lineNumber
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

//void RotateIconItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
//{
//  QGraphicsItem::mousePressEvent(event);
//  positionChanged = false;
//  position = pos();
//}

//void RotateIconItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
//{
//  QGraphicsItem::mouseMoveEvent(event);
//  if (isSelected() && (flags() & QGraphicsItem::ItemIsMovable)) {
//      positionChanged = true;
//    }
//}

//void RotateIconItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
//{
//  QGraphicsItem::mouseReleaseEvent(event);

//  if (isSelected() && (flags() & QGraphicsItem::ItemIsMovable)) {

//    QPointF newPosition;
//    newPosition = pos() - position;

//    if (newPosition.x() || newPosition.y()) {
//      positionChanged = true;

//      PlacementData placementData = placement.value();

//      placementData.offsets[0] += newPosition.x()/relativeToSize[0];
//      placementData.offsets[1] += newPosition.y()/relativeToSize[1];
//      placement.setValue(placementData);

//      changePlacementOffset(step->topOfStep(),&placement,StepNumberType);
//    }
//  }
//}


