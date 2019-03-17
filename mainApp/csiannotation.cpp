/****************************************************************************
**
** Copyright (C) 2019 Trevor SANDY. All rights reserved.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://www.trolltech.com/products/qt/opensource.html
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

/*********************************************************************
 *
 * This class creates a CSI Annotation icon
 *
 ********************************************************************/

#include <QGraphicsSceneContextMenuEvent>
#include "csiannotation.h"
#include "commonmenus.h"
#include "metaitem.h"
#include "color.h"
#include "step.h"
#include "lpub.h"

PlacementCsiPart::PlacementCsiPart(
    CsiPartMeta   &_csiPartMeta,
    QGraphicsItem *_parent)
{
  relativeType = CsiPartType;
  placement    = _csiPartMeta.placement;
  margin       = _csiPartMeta.margin;
  size[XX]     = _csiPartMeta.size.valuePixels(XX);
  size[YY]     = _csiPartMeta.size.valuePixels(YY);
  loc[XX]     += _csiPartMeta.loc.valuePixels(XX);
  loc[YY]     += _csiPartMeta.loc.valuePixels(YY);
  outline      = false;

  setParentItem(_parent);
}

bool PlacementCsiPart::hasOffset()
{
    bool zero;
    zero  = placement.value().offsets[XX] == 0.0f;
    zero &= placement.value().offsets[YY] == 0.0f;
    return !zero;
}

void PlacementCsiPart::paint( QPainter *painter, const QStyleOptionGraphicsItem *o, QWidget *w)
{
//#ifdef QT_DEBUG_MODE
    if (outline)
        setOutline(painter);
//#endif
    QGraphicsRectItem::paint(painter, o, w);
}

void PlacementCsiPart::toggleOutline()
{
    bool curState = outline;
    outline = ! curState;
    update();
}
void PlacementCsiPart::setOutline(QPainter *painter)
{
    painter->setRenderHints(QPainter::TextAntialiasing | QPainter::HighQualityAntialiasing);
    int ibt = int(1.0f/32.0f);

    /* BORDER */
    QPen borderPen;
    borderPen.setColor(QColor(Qt::blue));       // Qt::transparent
    borderPen.setCapStyle(Qt::SquareCap);
    borderPen.setJoinStyle(Qt::RoundJoin);
    borderPen.setStyle(Qt::DashLine);           // Qt::SolidLine
    borderPen.setWidth(ibt);
    painter->setPen(borderPen);

    /* BACKGROUND */
    painter->setBrush(QColor(Qt::transparent)); // Qt::blue

    QRectF irect(ibt/2,ibt/2,size[XX]-ibt,size[YY]-ibt);
    painter->drawRect(irect);

    setZValue(10000);
}

CsiAnnotation::CsiAnnotation(
    const Where       &_here,
    CsiAnnotationMeta &_caMeta)
{
    caMeta        = _caMeta;
    metaLine      = _here;
    partLine      = _here -1;

    if (caMeta.icon.value().hidden)
        return;

    PlacementData pld;

    // set PlacementCsiPart placement
    pld             = csiPartMeta.placement.value();
    pld.offsets[XX] = caMeta.icon.value().partOffset[XX];
    pld.offsets[YY] = caMeta.icon.value().partOffset[YY];
    csiPartMeta.placement.setValue(pld);
    csiPartMeta.size.setValuePixels(XX,caMeta.icon.value().partSize[XX]);
    csiPartMeta.size.setValuePixels(YY,caMeta.icon.value().partSize[YY]);

    // set CsiAnnotation Icon placement
    pld             = caMeta.placement.value();
    pld.offsets[XX] = caMeta.icon.value().iconOffset[XX];
    pld.offsets[YY] = caMeta.icon.value().iconOffset[YY];
    caMeta.placement.setValue(pld);
    setPlacement();

    margin.setValuesInches(0.0f,0.0f);
    placement       = caMeta.placement;
    relativeType    = CsiAnnotationType;
}

bool CsiAnnotation::setPlacement()
{
    QString placement, justification, preposition, relativeTo;
    if (caMeta.icon.value().placements.size() == 2) {
        placement   = placementNames[PlacementEnc(caMeta.icon.value().placements.at(0).toInt())];
        preposition = prepositionNames[PrepositionEnc(caMeta.icon.value().placements.at(1).toInt())];
    }
    else
    if (caMeta.icon.value().placements.size() == 3) {
        placement     = placementNames[PlacementEnc(caMeta.icon.value().placements.at(0).toInt())];
        justification = placementNames[PlacementEnc(caMeta.icon.value().placements.at(1).toInt())];
        preposition   = prepositionNames[PrepositionEnc(caMeta.icon.value().placements.at(2).toInt())];
    }
    if (preposition == "INSIDE" && justification == "CENTER") {
        justification = "";
    }

    int i;
    for (i = 0; i < NumSpots; i++) {
        if (placementOptions[i][0] == placement &&
            placementOptions[i][1] == justification &&
            placementOptions[i][2] == preposition) {
            break;
          }
    }
    if (i == NumSpots) {
        return false;
    }
    RectPlacement  _placementR = RectPlacement(i);;
    PlacementType  _relativeTo = caMeta.placement.value().relativeTo;
    caMeta.placement.setValue(_placementR,_relativeTo);
    return true;
}

bool CsiAnnotation::setCsiPartLoc(int csiSize[])
{
    float partOffset[2] =
    { caMeta.icon.value().partOffset[XX],
      caMeta.icon.value().partOffset[YY]};

    if (partOffset[XX] != 0.0f || partOffset[YY] != 0.0f) {
        csiPartMeta.loc.setValuePixels(XX,int(csiSize[XX] * partOffset[XX]));
        csiPartMeta.loc.setValuePixels(YY,int(csiSize[YY] * partOffset[YY]));
    } else {
        csiPartMeta.loc.setValue(XX,0.0f);
        csiPartMeta.loc.setValue(YY,0.0f);
        return false;
    }
    return true;
}

bool CsiAnnotation::setAnnotationLoc(float iconOffset[])
{
    if (iconOffset[XX] != 0.0f || iconOffset[YY] != 0.0f) {
        loc[XX] += iconOffset[XX];
        loc[YY] += iconOffset[YY];
    } else {
        loc[XX] = 0;
        loc[YY] = 0;
        return false;
    }
    return true;
}

CsiAnnotationItem::CsiAnnotationItem(
   QGraphicsItem  *_parent)
  : ResizeTextItem(_parent)
{
   relativeType         = CsiAnnotationType;
   placementCsiPart     = nullptr;
}

void CsiAnnotationItem::addGraphicsItems(
   CsiAnnotation        *_ca,
   Step                 *_step,
   PliPart              *_part,
   CsiItem              *_csiItem,
   bool                  _movable)
{
    icon                = _ca->caMeta.icon;
    partLine            = _ca->partLine;
    metaLine            = _ca->metaLine;
    placement           = _ca->placement;
    margin              = _part->styleMeta.margin;
    border              = _part->styleMeta.border;
    background          = _part->styleMeta.background;
    style               = _part->styleMeta.style;
    submodelLevel       = _csiItem->submodelLevel;
    parentRelativeType  = _csiItem->parentRelativeType;
    subModelColor       = _step->pli.pliMeta.subModelColor;
    positionChanged     = false;
    switch (parentRelativeType) {
      case CalloutType:
        topOf           = _step->topOfCallout();
        bottomOf        = _step->bottomOfCallout();
        break;
      default:
        topOf           = _step->topOfStep();
        bottomOf        = _step->bottomOfStep();
        break;
    }

    setParentItem(_csiItem);

    QString textString  = _part->text;
    QString fontString  = _part->styleMeta.font.valueFoo();
    QString colorString = _part->styleMeta.color.value();

    // TODO - automatically resize text until it fits
    if ((style.value() == AnnotationStyle::circle ||
         style.value() == AnnotationStyle::square) &&
         textString.size() > 2) {
       fontString = "Arial,17,-1,5,50,0,0,0,0,0";
    }

    QString toolTip = QString("Part %1 %2 (%3) \"%4\" - right-click to modify")
                              .arg(_part->type)
                              .arg(LDrawColor::name(_part->color))
                              .arg(LDrawColor::ldColorCode(LDrawColor::name(_part->color)))
                              .arg(Pli::titleDescription(_part->type));

    setText(textString,fontString,toolTip);

    QColor color(colorString);
    setDefaultTextColor(color);

    QRectF annotateRect;
    QRectF docSize  = QRectF(0,0,document()->size().width(),document()->size().height());
    if (style.value() == AnnotationStyle::none) {
        annotateRect = docSize;
    } else {
        bool dw = style.value() == AnnotationStyle::rectangle;
        QRectF styleSize = QRectF(0,0,dw ? docSize.width() : _part->styleMeta.size.valuePixels(XX), _part->styleMeta.size.valuePixels(YY));
        annotateRect = boundingRect().adjusted(0,0,styleSize.width()-docSize.width(),styleSize.height()-docSize.height());
        // center the document on the new size
        setTextWidth(-1);
        setTextWidth(annotateRect.width());
        QTextBlockFormat format;
        format.setAlignment(Qt::AlignCenter);
        QTextCursor cursor = textCursor();
        cursor.select(QTextCursor::Document);
        cursor.mergeBlockFormat(format);
        cursor.clearSelection();
        setTextCursor(cursor);
    }
    size[XX] = int(annotateRect.size().width());
    size[YY] = int(annotateRect.size().height());

    sizeIt();

    // set PlacementCsiPart location based on csi size

    _ca->setCsiPartLoc(_csiItem->size);

    // place PlacementCsiPart relative to CSI

    placementCsiPart = new PlacementCsiPart(_ca->csiPartMeta,_csiItem);
    if (! placementCsiPart->hasOffset())
        _csiItem->placeRelative(placementCsiPart);

    placementCsiPart->setPos(placementCsiPart->loc[XX],
                             placementCsiPart->loc[YY]);

    // place CsiAnnotation Icon relative to PlacementCsiPart

    bool hasLoc = _ca->setAnnotationLoc(placement.value().offsets);
    if (hasLoc) {
        loc[XX] = _ca->loc[XX];
        loc[YY] = _ca->loc[YY];
    } else {
        placementCsiPart->placeRelative(this);
        _ca->assign(this);
    }
    setPos(loc[XX],loc[YY]);

    setZValue(10000);
    setFlag(QGraphicsItem::ItemIsMovable, _movable);
    setFlag(QGraphicsItem::ItemIsSelectable, _movable);
}

void CsiAnnotationItem::sizeIt()
{
    size[XX] += int(border.valuePixels().margin[XX]);
    size[YY] += int(border.valuePixels().margin[YY]);
    size[XX] += int(border.valuePixels().thickness);
    size[YY] += int(border.valuePixels().thickness);
}

void CsiAnnotationItem::setBackground(QPainter *painter)
{
    // set border and background parameters
    BorderData     borderData     = border.valuePixels();
    BackgroundData backgroundData = background.value();

    // set rectangle size and dimensions parameters
    int ibt = int(borderData.thickness);
    QRectF irect(ibt/2,ibt/2,size[XX]-ibt,size[YY]-ibt);

    // set painter and render hints (initialized with pixmap)
    painter->setRenderHints(QPainter::TextAntialiasing | QPainter::HighQualityAntialiasing);

    // set the background then set the border and paint both in one go.

    /* BACKGROUND */
    QColor brushColor;
    switch(backgroundData.type) {
    case BackgroundData::BgColor:
        brushColor = LDrawColor::color(backgroundData.string);
        break;
    case BackgroundData::BgSubmodelColor:
        brushColor = LDrawColor::color(subModelColor.value(0));
        break;
    default:
        brushColor = Qt::transparent;
        break;
    }
    painter->setBrush(brushColor);

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

    painter->setPen(borderPen);

    // set icon border dimensions
    qreal rx = borderData.radius;
    qreal ry = borderData.radius;
    qreal dx = size[XX];
    qreal dy = size[YY];

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
    if (style.value() != AnnotationStyle::circle) {
        if (borderData.type == BorderData::BdrRound) {
            painter->drawRoundRect(irect,int(rx),int(ry));
        } else {
            painter->drawRect(irect);
        }
    } else {
        painter->drawEllipse(irect);
    }
}

void CsiAnnotationItem::paint( QPainter *painter, const QStyleOptionGraphicsItem *o, QWidget *w)
{
    if (style.value() != AnnotationStyle::none)
        setBackground(painter);
    QGraphicsTextItem::paint(painter, o, w);
}

void CsiAnnotationItem::change() {
    updateCsiAnnotationIconMeta(metaLine, &icon);
}

void CsiAnnotationItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
  position = pos();
  positionChanged = false;
  QGraphicsItem::mousePressEvent(event);
  //placeGrabbers();
}

void CsiAnnotationItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
  positionChanged = true;
  QGraphicsItem::mouseMoveEvent(event);
  if (isSelected() && (flags() & QGraphicsItem::ItemIsMovable)) {
      //placeGrabbers();
  }
}

void CsiAnnotationItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsItem::mouseReleaseEvent(event);

    if (isSelected() && (flags() & QGraphicsItem::ItemIsMovable)) {

        // back annotate the movement of the PLI into the LDraw file.

        if (positionChanged) {

            beginMacro(QString("DragCsiAnnotation"));

            QPointF newPosition;
            newPosition = pos() - position;

            if (newPosition.x() || newPosition.y()) {

                positionChanged = true;

                PlacementData placementData    = placement.value();
                if (relativeToSize[XX] > 1 || relativeToSize[YY] > 1) {
                    placementData.offsets[XX] += newPosition.x()+loc[XX];
                    placementData.offsets[YY] += newPosition.y()+loc[YY];
                } else {
                    placementData.offsets[XX] += newPosition.x()/relativeToSize[XX];
                    placementData.offsets[YY] += newPosition.y()/relativeToSize[YY];
                }
                placement.setValue(placementData);

                CsiAnnotationIconData caiData = icon.value();
                caiData.iconOffset[XX]        = placementData.offsets[XX];
                caiData.iconOffset[YY]        = placementData.offsets[YY];
                icon.setValue(caiData);

                change();
            }
            endMacro();
        }
    }
}

void CsiAnnotationItem::contextMenuEvent(
    QGraphicsSceneContextMenuEvent *event)
{
  QMenu menu;

  QString pl = "CSI Part Annotation";
  PlacementData placementData = placement.value();
  QString whatsThis = commonMenus.naturalLanguagePlacementWhatsThis(CsiAnnotationType,placementData,pl);

  QAction *placementAction  = commonMenus.placementMenu(menu, pl, whatsThis);

  QAction *hideAction = menu.addAction("Hide Part Annotation");
  hideAction->setIcon(QIcon(":/resources/hidepartannotation.png"));

  QAction *toggleCsiPartRectAction = menu.addAction("Toggle Part Outline");
  toggleCsiPartRectAction->setIcon(QIcon(":/resources/togglepartoutline.png"));

  QAction *selectedAction   = menu.exec(event->screenPos());

  if (selectedAction == nullptr) {
      return;
    }
  else if (selectedAction == toggleCsiPartRectAction) {
      placementCsiPart->toggleOutline();
      gui->pagescene()->update();
  } else if (selectedAction == placementAction) {
      changeCsiAnnotationPlacement(
              parentRelativeType,
              CsiAnnotationType,
              pl+" Placement",
              metaLine,
              metaLine,
             &placement,
             &icon,true,1,0,false);
   } else if (selectedAction == hideAction) {
              CsiAnnotationIconData caid = icon.value();
              caid.hidden = true;
              icon.setValue(caid);
              updateCsiAnnotationIconMeta(metaLine, &icon);
   }
}
