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

  size           = _rotateIconMeta.size;
  picScale       = _rotateIconMeta.picScale;
  border         = _rotateIconMeta.border;
  arrow          = _rotateIconMeta.arrow;
  background     = _rotateIconMeta.background;
  display        = _rotateIconMeta.display;
  subModelColor  = _rotateIconMeta.subModelColor;

  placement      = _rotateIconMeta.placement;
  margin         = _rotateIconMeta.margin;
  relativeType   = RotateIconType;

  // initialize pixmap using icon demensions
  pixmap         = new QPixmap(size.valuePixels(XX),
                               size.valuePixels(YY));

  //gradient settings
  if (background.value().gsize[0] == 0 &&
      background.value().gsize[1] == 0) {
      background.value().gsize[0] = pixmap->width();
      background.value().gsize[1] = pixmap->width();
      QSize gSize(background.value().gsize[0],
          background.value().gsize[1]);
      int h_off = gSize.width() / 10;
      int v_off = gSize.height() / 8;
      background.value().gpoints << QPointF(gSize.width() / 2, gSize.height() / 2)
                                 << QPointF(gSize.width() / 2 - h_off, gSize.height() / 2 - v_off);
    }

  // set image size
  placementRotateIcon.sizeit();

  setRotateIconImage(pixmap);

  QString toolTip("Rotate Icon - right-click to modify");
  setToolTip(toolTip);

  setZValue(10000);
  setParentItem(parent);
  setPixmap(*pixmap);
  setTransformationMode(Qt::SmoothTransformation);
  setFlag(QGraphicsItem::ItemIsSelectable,true);

  delete pixmap;
}

RotateIconItem::RotateIconItem(){
  relativeType       = RotateIconType;
  step               = NULL;
  pixmap             = NULL;
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

void RotateIconItem::setRotateIconImage(QPixmap *pixmap)
{
  // set pixmap to transparent
  pixmap->fill(Qt::transparent);

  // set border and background parameters
  BorderData     borderData     = border.valuePixels();
  BorderData     arrowData      = arrow.valuePixels();
  BackgroundData backgroundData = background.value();

  // set rectangle size and demensions parameters
  int ibt = int(borderData.thickness);
  QRectF irect(ibt/2,ibt/2,pixmap->width()-ibt,pixmap->height()-ibt);

  // set painter and render hints (initialized with pixmap)
  QPainter painter;
  painter.begin(pixmap);
  painter.setRenderHints(QPainter::Antialiasing,true);

        /* BACKGROUND */

  // set icon background colour
  bool useGradient = false;
  QColor brushColor;
  switch(backgroundData.type) {
    case BackgroundData::BgTransparent:
      brushColor = Qt::transparent;
      break;
    case BackgroundData::BgGradient:
      useGradient = true;
      brushColor = Qt::transparent;
      break;
    case BackgroundData::BgColor:
    case BackgroundData::BgSubmodelColor:
      if (backgroundData.type == BackgroundData::BgColor) {
          brushColor = LDrawColor::color(backgroundData.string);
        } else {
          brushColor = LDrawColor::color(subModelColor.value(0));
        }
      break;
    case BackgroundData::BgImage:
      break;
    }

  useGradient ? painter.setBrush(QBrush(setGradient())) :
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
  if (borderData.line == BorderData::BdrLnSolid){
      borderPen.setStyle(Qt::SolidLine);
    }
  else if (borderData.line == BorderData::BdrLnDash){
      borderPen.setStyle(Qt::DashLine);
    }
  else if (borderData.line == BorderData::BdrLnDot){
      borderPen.setStyle(Qt::DotLine);
    }
  else if (borderData.line == BorderData::BdrLnDashDot){
      borderPen.setStyle(Qt::DashDotLine);
    }
  else if (borderData.line == BorderData::BdrLnDashDotDot){
      borderPen.setStyle(Qt::DashDotDotLine);
    }
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
  arrowPenColor = LDrawColor::color(arrowData.color);
  QPen defaultArrowPen;
  int defaultLineWidth = arrowData.thickness;
  defaultArrowPen.setColor(arrowPenColor);
  defaultArrowPen.setCapStyle(Qt::SquareCap);
  defaultArrowPen.setJoinStyle(Qt::MiterJoin);
  if (arrowData.line == BorderData::BdrLnSolid){
      defaultArrowPen.setStyle(Qt::SolidLine);
    }
  else if (arrowData.line == BorderData::BdrLnDash){
      defaultArrowPen.setStyle(Qt::DashLine);
    }
  else if (arrowData.line == BorderData::BdrLnDot){
      defaultArrowPen.setStyle(Qt::DotLine);
    }
  else if (arrowData.line == BorderData::BdrLnDashDot){
      defaultArrowPen.setStyle(Qt::DashDotLine);
    }
  else if (arrowData.line == BorderData::BdrLnDashDotDot){
      defaultArrowPen.setStyle(Qt::DashDotDotLine);
    }
  defaultArrowPen.setWidth(defaultLineWidth);
  QPen arrowPen = defaultArrowPen;

  // set painter for arrows
  painter.setPen(arrowPen);
  painter.setBrush(Qt::transparent);

  // set arrow height and width parameters
  qreal aw = irect.width();
  qreal ah = irect.height() / 2.0;
  float inset = arrowData.margin[0];

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

QGradient RotateIconItem::setGradient(){

  BackgroundData backgroundData = background.value();
  QPolygonF pts;
  QGradientStops stops;

  QSize gSize(backgroundData.gsize[0],backgroundData.gsize[1]);

    for (int i=0; i<backgroundData.gpoints.size(); i++)
      pts.append(backgroundData.gpoints.at(i));

  QGradient::CoordinateMode mode = QGradient::LogicalMode;
  switch (backgroundData.gmode){
    case BackgroundData::LogicalMode:
      mode = QGradient::LogicalMode;
    break;
    case BackgroundData::StretchToDeviceMode:
      mode = QGradient::StretchToDeviceMode;
    break;
    case BackgroundData::ObjectBoundingMode:
      mode = QGradient::ObjectBoundingMode;
    break;
    }

  QGradient::Spread spread = QGradient::RepeatSpread;
  switch (backgroundData.gspread){
    case BackgroundData::PadSpread:
      spread = QGradient::PadSpread;
    break;
    case BackgroundData::RepeatSpread:
      spread = QGradient::RepeatSpread;
    break;
    case BackgroundData::ReflectSpread:
      spread = QGradient::ReflectSpread;
    break;
    }

  QGradient g = QLinearGradient(pts.at(0), pts.at(1));
  switch (backgroundData.gtype){
    case BackgroundData::LinearGradient:
      g = QLinearGradient(pts.at(0), pts.at(1));
    break;
    case BackgroundData::RadialGradient:
      {
        QLineF line(pts[0], pts[1]);
        if (line.length() > 132){
            line.setLength(132);
          }
        g = QRadialGradient(line.p1(),  qMin(gSize.width(), gSize.height()) / 3.0, line.p2());
      }
    break;
    case BackgroundData::ConicalGradient:
      {
        qreal angle = backgroundData.gangle;
        g = QConicalGradient(pts.at(0), angle);
      }
    break;
    case BackgroundData::NoGradient:
    break;
    }

  for (int i=0; i<backgroundData.gstops.size(); ++i) {
      stops.append(backgroundData.gstops.at(i));
    }

  g.setStops(stops);
  g.setSpread(spread);
  g.setCoordinateMode(mode);

  return g;
}

void RotateIconItem::setFlag(GraphicsItemFlag flag, bool value)
{
  QGraphicsItem::setFlag(flag,value);
}

void RotateIconItem::contextMenuEvent(
    QGraphicsSceneContextMenuEvent *event)
{
  QMenu menu;

  PlacementData placementData = placement.value();

  QString pl = "Rotate Icon";
  QAction *placementAction = NULL;
  bool singleStep = parentRelativeType == SingleStepType;
  if (! singleStep) {
      placementAction  = commonMenus.placementMenu(menu,pl,
                                                   commonMenus.naturalLanguagePlacementWhatsThis(relativeType,placementData,pl));
    }

  QAction *backgroundAction = commonMenus.backgroundMenu(menu,pl);
  QAction *borderAction     = commonMenus.borderMenu(menu,pl);
  QAction *marginAction     = commonMenus.marginMenu(menu,pl);
  QAction *displayAction    = commonMenus.displayMenu(menu,pl);

  QAction *editArrowAction = menu.addAction("Edit Arrow");
  editArrowAction->setWhatsThis("Edit this rotation icon arrows");
  editArrowAction->setIcon(QIcon(":/resources/editrotateicon.png"));

  QAction *deleteRotateIconAction = menu.addAction("Delete " +pl);
  deleteRotateIconAction->setWhatsThis("Delete this rotate icon");
  deleteRotateIconAction->setIcon(QIcon(":/resources/delete.png"));

  QAction *selectedAction  = menu.exec(event->screenPos());

  if (selectedAction == NULL) {
      return;
    }

  Where top;
  Where bottom;

  switch (parentRelativeType) {
    case CalloutType:
      top    = step->topOfCallout();
      bottom = step->bottomOfCallout();
    break;
    default:
      top    = step->topOfStep();
      bottom = step->bottomOfStep();
    break;
  }

  if (selectedAction == placementAction) {

      // logging only
      if (! singleStep) {
      bool multiStep = parentRelativeType == StepGroupType;
      logInfo() << "\nMOVE ROTATE_ICON - "
                << "\nPAGE- "
                << (multiStep ? " \nMulti-Step Page" : " \nSingle-Step Page")
                << "\nPAGE WHERE -                  "
                << " \nPage TopOf (Model Name):     " << top.modelName
                << " \nPage TopOf (Line Number):    " << top.lineNumber
                << " \nPage BottomOf (Model Name):  " << bottom.modelName
                << " \nPage BottomOf (Line Number): " << bottom.lineNumber
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
        } // end logging only

      changePlacement(parentRelativeType,
                      SingleStepType,         //not using RotateIconType intentionally
                      pl+" Placement",
                      top,
                      bottom,
                      &placement,true,1,0,false);
    } else if (selectedAction == backgroundAction) {
      changeBackground(pl+" Background",
                       top,
                       bottom,
                       &background);
    } else if (selectedAction == borderAction) {
      changeBorder(pl+" Border",
                   top,
                   bottom,
                   &border);
    } else if (selectedAction == marginAction) {
      changeMargins(pl+" Margins",
                    top,
                    bottom,
                    &margin);
    } else if (selectedAction == displayAction){
      changeBool(top,
                 bottom,
                 &display);
    } else if (selectedAction == editArrowAction) {
      changeBorder(pl+" Arrow",
                   top,
                   bottom,
                   &arrow,
                   true,1,true,
                   true);   // indicate that this call if from rotate arrow
    } else if (selectedAction == deleteRotateIconAction) {
      beginMacro("DeleteRotateIcon");
      deleteRotateIcon(top);
      endMacro();
    }
}

void RotateIconItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
  QGraphicsItem::mousePressEvent(event);
  positionChanged = false;
  position = pos();
}

void RotateIconItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
  QGraphicsItem::mouseMoveEvent(event);
  if (isSelected() && (flags() & QGraphicsItem::ItemIsMovable)) {
      positionChanged = true;
    }
}

void RotateIconItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
  QGraphicsItem::mouseReleaseEvent(event);
  if (isSelected() && (flags() & QGraphicsItem::ItemIsMovable)) {

      Where top;

      switch (parentRelativeType) {
        case CalloutType:
          top    = step->topOfCallout();
          break;
        default:
          top    = step->topOfStep();
          break;
        }

      if (positionChanged) {

          beginMacro(QString("DragRotateIcon"));

          QPointF newPosition;
          newPosition = pos() - position;

          if (newPosition.x() || newPosition.y()) {
              positionChanged = true;

              PlacementData placementData = placement.value();
              placementData.offsets[0] += newPosition.x()/relativeToSize[0];
              placementData.offsets[1] += newPosition.y()/relativeToSize[1];
              placement.setValue(placementData);

              logInfo() << "\nCHANGE ROTATE_ICON - "
                        << "\nPAGE WHERE - "
                        << " \nStep TopOf (Model Name): "    << top.modelName
                        << " \nStep TopOf (Line Number): "   << top.lineNumber
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

              changePlacementOffset(top,
                                    &placement,
                                    relativeType);
              endMacro();
            }
        }
    }
}

void RotateIconItem::change()
{
  if (isSelected() && (flags() & QGraphicsItem::ItemIsMovable)) {

      Where top;
      Where bottom;

      switch (parentRelativeType) {
        case CalloutType:
          top    = step->topOfCallout();
          bottom = step->bottomOfCallout();
        break;
        default:
          top    = step->topOfStep();
          bottom = step->bottomOfStep();
        break;
      }

      if (sizeChanged) {

          beginMacro(QString("ResizeRotateIcon"));

          PlacementData placementData = placement.value();

          qreal topLeft[2] = { sceneBoundingRect().left(),  sceneBoundingRect().top() };
          qreal size[2]    = { sceneBoundingRect().width(), sceneBoundingRect().height() };
          calcOffsets(placementData,placementData.offsets,topLeft,size);
          placement.setValue(placementData);

          changePlacementOffset(top, &placement, relativeType);

          picScale.setValue(picScale.value()*oldScale);
          changeFloat(top,bottom,&picScale, 1, false);

          logInfo() << "\nRESIZE ROTATE_ICON - "
                    << "\nPICTURE DATA - "
                    << " \npicScale: "                   << picScale.value()
                    << " \nMargin X: "                   << margin.value(0)
                    << " \nMargin Y: "                   << margin.value(1)
                    << " \nDisplay: "                    << display.value()
                    << "\nPAGE WHERE - "
                    << " \nStep TopOf (Model Name): "    << top.modelName
                    << " \nStep TopOf (Line Number): "   << top.lineNumber
                    << " \nStep BottomOf (Model Name): " << bottom.modelName
                    << " \nStep BottomOf (Line Number): "<< bottom.lineNumber
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




