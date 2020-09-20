/****************************************************************************
**
** Copyright (C) 2015 - 2020 Trevor SANDY. All rights reserved.
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

#include "lpub.h"

void RotateIconItem::setAttributes(
  Step           *_step,
  PlacementType   _parentRelativeType,
  RotateIconMeta &_rotateIconMeta,
  QGraphicsItem   *parent)
{
  step               = _step;
  parentRelativeType = _parentRelativeType;

  iconSize       = _rotateIconMeta.size;
  picScale       = _rotateIconMeta.picScale;
  border         = _rotateIconMeta.border;
  arrow          = _rotateIconMeta.arrow;
  background     = _rotateIconMeta.background;
  display        = _rotateIconMeta.display;
  subModelColor  = _rotateIconMeta.subModelColor;

  placement      = _rotateIconMeta.placement;
  margin         = _rotateIconMeta.margin;
  relativeType   = RotateIconType;
  backgroundImage= false;

  // initialize pixmap using icon dimensions
  pixmap         = new QPixmap(iconSize.valuePixels(XX),
                               iconSize.valuePixels(YY));

  //gradient settings
  if (background.value().gsize[0] == 0.0f &&
      background.value().gsize[1] == 0.0f) {
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
  sizeit();

  setRotateIconImage(pixmap);

  QString toolTip("Rotate Icon - right-click to modify");
  setToolTip(toolTip);
  setData(ObjectId, RotateIconBackgroundObj);
  setZValue(ROTATEICONBACKGROUND_ZVALUE_DEFAULT);
  setParentItem(parent);
  setPixmap(*pixmap);
  setTransformationMode(Qt::SmoothTransformation);
  setFlag(QGraphicsItem::ItemIsMovable,true);
  setFlag(QGraphicsItem::ItemIsSelectable,true);
  setFlag(QGraphicsItem::ItemIsFocusable, true);
  setAcceptHoverEvents(true);

  delete pixmap;
}

RotateIconItem::RotateIconItem(){
  relativeType       = RotateIconType;
  step               = nullptr;
  pixmap             = nullptr;
  backgroundImage    = false;
}

RotateIconItem::RotateIconItem(
    Step           *_step,
    PlacementType   _parentRelativeType,
    RotateIconMeta &_rotateIconMeta,
    QGraphicsItem  *_parent):
    isHovered(false),
    mouseIsDown(false)
{

  setAttributes(
        _step,
        _parentRelativeType,
        _rotateIconMeta,
        _parent);
}

void RotateIconItem::setSize(
    UnitsMeta _size,
    float _borderThickness)
{
  iconSize        = _size;
  borderThickness =_borderThickness;
}

void RotateIconItem::sizeit()
{
  size[0] = int(iconSize.valuePixels(0));
  size[1] = int(iconSize.valuePixels(1));

  size[0] += 2*int(borderThickness);
  size[1] += 2*int(borderThickness);
}

void RotateIconItem::setRotateIconImage(QPixmap *pixmap)
{
  // set pixmap to transparent
  pixmap->fill(Qt::transparent);

  // set border and background parameters
  BorderData     borderData     = border.valuePixels();
  BorderData     arrowData      = arrow.valuePixels();
  BackgroundData backgroundData = background.value();

  int saveFill = backgroundData.stretch;
  int saveBorderType = borderData.type;
  int saveBorderLine = borderData.line;

  // set defaults for using a background image only
  if (backgroundData.type == BackgroundData::BgImage &&
     !backgroundData.string.isEmpty()) {
      backgroundImage = true;
      arrowData.hideArrows = true;
      backgroundData.stretch = true;
      borderData.type = BorderData::BdrNone;
      borderData.line = BorderData::BdrLnNone;
  }

  // set rectangle size and dimensions parameters
  int ibt = int(borderData.thickness);
  QRectF irect(ibt/2,ibt/2,pixmap->width()-ibt,pixmap->height()-ibt);

  // set painter and render hints (initialized with pixmap)
  QPainter painter;
  painter.begin(pixmap);
  painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
  painter.setRenderHints(QPainter::Antialiasing,true);

  // set the background then set the border and paint both in one go.

  /* BACKGROUND */
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
      {
        // confirm valid file path
        QFileInfo fileInfo(backgroundData.string);                             // set 'default path'
        if (!fileInfo.exists()) {                                              // check 'default path'
            fileInfo.setFile(QDir::currentPath() + QDir::separator() + fileInfo.fileName()); // check 'current path'
            if (!fileInfo.exists()) {
                emit gui->messageSig(LOG_ERROR, QString("Unable to locate 'rotate icon' image %1. Be sure image file "
                                                   "is relative to model file or define using an absolute path.").arg(fileInfo.fileName()));
                backgroundImage = false;
                arrowData.hideArrows = false;
                backgroundData.stretch = saveFill;
                borderData.type = BorderData::Border(saveBorderType);
                borderData.line = BorderData::Line(saveBorderLine);
                brushColor = Qt::transparent;
                break;
            }
        }
        QImageReader reader(fileInfo.absoluteFilePath());
        QImage image = reader.read();
        image = image.convertToFormat(QImage::Format_ARGB32_Premultiplied);
        if (backgroundData.stretch) {
            QSize psize = pixmap->size();
            QSize isize = image.size();
            qreal sx = psize.width();
            qreal sy = psize.height();
            sx /= isize.width();
            sy /= isize.height();
            painter.scale(sx,sy);
            painter.drawImage(0,0,image);
          } else {
            for (int y = 0; y < pixmap->height(); y += image.height()) {
                for (int x = 0; x < pixmap->width(); x += image.width()) {
                    painter.drawImage(x,y,image);
                  }
              }
          }
        brushColor = Qt::transparent;
      }
      break;
    }

  useGradient ? painter.setBrush(QBrush(setGradient())) :
  painter.setBrush(brushColor);

  /* BORDER */
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
  if (borderData.line == BorderData::BdrLnNone){
        borderPen.setStyle(Qt::NoPen);
  }
  else if (borderData.line == BorderData::BdrLnSolid){
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

  // set icon border dimensions
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

  // draw icon rectangle - background and border
  if (borderData.type == BorderData::BdrRound) {
    painter.drawRoundRect(irect,int(rx),int(ry));
  } else {
    painter.drawRect(irect);
  }

  /* ARROWS */
  if (!arrowData.hideArrows) {

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
    }

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
  QAction *placementAction = nullptr;
//  bool singleStep = parentRelativeType == SingleStepType;
//  if (! singleStep) {
      placementAction  = commonMenus.placementMenu(menu,pl,
                         commonMenus.naturalLanguagePlacementWhatsThis(relativeType,placementData,pl));
//    }

  QAction *backgroundAction    = commonMenus.backgroundMenu(menu,pl);
  QAction *marginAction        = commonMenus.marginMenu(menu,pl);
  QAction *displayAction       = commonMenus.displayMenu(menu,pl);

  QAction *borderAction        = nullptr;
  QAction *subModelColorAction = nullptr;
  QAction *editArrowAction     = nullptr;
  if (!backgroundImage) {
      borderAction        = commonMenus.borderMenu(menu,pl);
      subModelColorAction = commonMenus.subModelColorMenu(menu,pl);

      editArrowAction = menu.addAction("Edit "+pl+" Arrows");
      editArrowAction->setWhatsThis("Edit this rotation icon arrows");
      editArrowAction->setIcon(QIcon(":/resources/editrotateicon.png"));
  }

  QAction *rotateIconSizeAction = menu.addAction("Change "+pl+" Size");
  rotateIconSizeAction->setWhatsThis("Change the rotateIcon size");
  rotateIconSizeAction->setIcon(QIcon(":/resources/rotateiconsize.png"));

  QAction *deleteRotateIconAction = menu.addAction("Delete "+pl);
  deleteRotateIconAction->setWhatsThis("Delete this rotate icon");
  deleteRotateIconAction->setIcon(QIcon(":/resources/delete.png"));

  QAction *selectedAction  = menu.exec(event->screenPos());

  if (selectedAction == nullptr) {
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

      // debug logging only
//      if (! singleStep) {
//      bool multiStep = parentRelativeType == StepGroupType;

//      logInfo() << "\nMOVE ROTATE_ICON - "
//                << "\nPAGE- "
//                << (multiStep ? " \nMulti-Step Page" : " \nSingle-Step Page")
//                << "\nPAGE WHERE -                  "
//                << " \nPage TopOf (Model Name):     " << top.modelName
//                << " \nPage TopOf (Line Number):    " << top.lineNumber
//                << " \nPage BottomOf (Model Name):  " << bottom.modelName
//                << " \nPage BottomOf (Line Number): " << bottom.lineNumber
//                << "\nUSING PLACEMENT DATA -        "
//                << " \nPlacement:                   " << PlacNames[placement.value().placement]     << " (" << placement.value().placement << ")"
//                << " \nJustification:               " << PlacNames[placement.value().justification] << " (" << placement.value().justification << ")"
//                << " \nPreposition:                 " << PrepNames[placement.value().preposition]   << " (" << placement.value().justification << ")"
//                << " \nRelativeTo:                  " << RelNames[placement.value().relativeTo]     << " (" << placement.value().relativeTo << ")"
//                << " \nRectPlacement:               " << RectNames[placement.value().rectPlacement] << " (" << placement.value().rectPlacement << ")"
//                << " \nOffset[0]:                   " << placement.value().offsets[0]
//                << " \nOffset[1]:                   " << placement.value().offsets[1]
//                << "\nOTHER DATA -                  "
//                << " \nRelativeType:                " << RelNames[relativeType]       << " (" << relativeType << ")"
//                << " \nParentRelativeType:          " << RelNames[parentRelativeType] << " (" << parentRelativeType << ")"
//                                                ;
//        } // end logging only

      changePlacement(parentRelativeType,
                      RotateIconType,
                      pl+" Placement",
                      top,
                      bottom,
                      &placement,true,1,0,false);
    } else if (selectedAction == backgroundAction) {
      changeBackground(pl+" Background",
                       top,
                       bottom,
                       &background);
    } else if (selectedAction == subModelColorAction) {
      changeSubModelColor(pl+" Submodel Color",
                          top,
                          bottom,
                       &subModelColor,0,false,false);

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
      changeBorder(pl+" Arrows",
                   top,
                   bottom,
                   &arrow,
                   true,1,true,
                   true);   // indicate that this call is from rotate arrow
    } else if (selectedAction == rotateIconSizeAction) {
      changeImageItemSize(pl+" Size",
                           top,
                           bottom,
                           &iconSize,
                           true,1,true);
    } else if (selectedAction == deleteRotateIconAction) {
      beginMacro("DeleteRotateIcon");
      QString metaCommand = QString("ROTATE_ICON");
      deleteImageItem(top,metaCommand);
      endMacro();
    }
}

void RotateIconItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
  position = pos();
  mouseIsDown = true;
  positionChanged = false;
  QGraphicsItem::mousePressEvent(event);
  //placeGrabbers();
}

void RotateIconItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
  positionChanged = true;
  QGraphicsItem::mouseMoveEvent(event);
//  if (isSelected() && (flags() & QGraphicsItem::ItemIsMovable)) {
//      placeGrabbers();
//    }
}

void RotateIconItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    mouseIsDown = false;
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

            if (newPosition.x() != 0.0 || newPosition.y() != 0.0) {
                positionChanged = true;

                PlacementData placementData = placement.value();
                placementData.offsets[0] += newPosition.x()/relativeToSize[0];
                placementData.offsets[1] += newPosition.y()/relativeToSize[1];
                placement.setValue(placementData);

//                logInfo() << QString(" -RELATIVE_TO_SIZE: (%1, %2), NEW_POSITION: (%3, %4), OFFSET: (%5, %6)")
//                                     .arg(QString::number(double(relativeToSize[XX]),'f',5))
//                                     .arg(QString::number(double(relativeToSize[YY]),'f',5))
//                                     .arg(QString::number(double(newPosition.x()),'f',5))
//                                     .arg(QString::number(double(newPosition.y()),'f',5))
//                                     .arg(QString::number(double(placement.value().offsets[XX]),'f',5))
//                                     .arg(QString::number(double(placement.value().offsets[YY]),'f',5));

//                logInfo() << "\nCHANGE ROTATE_ICON - "
//                          << "\nPAGE WHERE - "
//                          << " \nStep TopOf (Model Name): "    << top.modelName
//                          << " \nStep TopOf (Line Number): "   << top.lineNumber
//                          << "\nUSING PLACEMENT DATA - "
//                          << " \nPlacement: "                 << PlacNames[placement.value().placement]     << " (" << placement.value().placement << ")"
//                          << " \nJustification: "             << PlacNames[placement.value().justification] << " (" << placement.value().justification << ")"
//                          << " \nPreposition: "               << PrepNames[placement.value().preposition]   << " (" << placement.value().justification << ")"
//                          << " \nRelativeTo: "                << RelNames[placement.value().relativeTo]     << " (" << placement.value().relativeTo << ")"
//                          << " \nRectPlacement: "             << RectNames[placement.value().rectPlacement] << " (" << placement.value().rectPlacement << ")"
//                          << " \nOffset[0]: "                 << placement.value().offsets[0]
//                          << " \nOffset[1]: "                 << placement.value().offsets[1]
//                          << "\nOTHER DATA - "
//                          << " \nRelativeType: "               << RelNames[relativeType]       << " (" << relativeType << ")"
//                          << " \nParentRelativeType: "         << RelNames[parentRelativeType] << " (" << parentRelativeType << ")"
//                             ;

                changePlacementOffset(top,
                                      &placement,
                                      relativeType);

            }
            endMacro();
        }
    }
}

void RotateIconItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    isHovered = !this->isSelected() && !mouseIsDown;
    QGraphicsItem::hoverEnterEvent(event);
}

void RotateIconItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    isHovered = false;
    QGraphicsItem::hoverLeaveEvent(event);
}

void RotateIconItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
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



