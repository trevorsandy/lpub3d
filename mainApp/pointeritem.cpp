
/****************************************************************************
**
** Copyright (C) 2007-2009 Kevin Clague. All rights reserved.
** Copyright (C) 2015 - 2018 Trevor SANDY. All rights reserved.
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

/****************************************************************************
 *
 * This class implements the graphical pointers that extend from base objects
 * (e.g. a Callout) to assembly images as visual indicators to the builder
 * as to where to add the completed submodel into partially assembled final model.
 *
 * Please see lpub.h for an overall description of how the files in LPub
 * make up the LPub program.
 *
 ***************************************************************************/


#include "lpub.h"
#include <QtWidgets>
#include <QGraphicsView>
#include <QGraphicsRectItem>
#include <QGraphicsPolygonItem>
#include <QPolygonF>
#include <QGraphicsSceneContextMenuEvent>
#include <math.h>
#include "pointeritem.h"
#include "borderedlineitem.h"



/* calculate the parameters for the equation of line from two points */

bool rectLineIntersect(
  QPoint  tip,
  QPoint  loc,
  QRect   rect, 
  int     base,
  QPoint &intersect,

  PlacementEnc &placement);

//---------------------------------------------------------------------------

PointerItem::PointerItem(QGraphicsItem *parent)
    : QGraphicsItemGroup(parent){}

PointerItem::~PointerItem(){
  shaftSegments.clear();
}

void PointerItem::drawPointerPoly()
{
  // head
  QPolygonF poly;

  poly << QPointF(-2*grabSize(), 0);
  poly << QPointF(-2*grabSize(),grabSize()/2);
  poly << QPointF(grabSize()/2,0);
  poly << QPointF(-2*grabSize(),-grabSize()/2);
  poly << QPointF(-2*grabSize(),0);
  float headWidth = poly.boundingRect().width();

  enum Seg { first, second, third };

  for (int i = 0; i < segments(); i++) {

      QLineF linef;
      switch (segments()) {
      case OneSegment:
      {
          linef = QLineF(points[Base],points[Tip]);
          removeFromGroup(shaftSegments[i]);
          BorderedLineItem *shaft = shaftSegments[i];
          shaft->setSegment(OneSegment);
          shaft->setSegments(segments());
          shaft->setHeadWidth(headWidth);
          shaft->setBorderedLine(linef);
          shaft->setZValue(10);
          addToGroup(shaft);
      }
          break;
      case TwoSegments:
      {
          if (i == Seg::first) {
              linef = QLineF(points[Base],points[MidBase]);
              removeFromGroup(shaftSegments[i]);
              BorderedLineItem *shaft = shaftSegments[i];
              shaft->setSegment(OneSegment);
              shaft->setSegments(segments());
              shaft->setHeadWidth(headWidth);
              shaft->setBorderedLine(linef);
              shaft->setZValue(20);
              addToGroup(shaft);
          } else if (i == Seg::second) {
              linef = QLineF(points[MidBase],points[Tip]);
              removeFromGroup(shaftSegments[i]);
              BorderedLineItem *shaft = shaftSegments[i];
              shaft->setSegment(TwoSegments);
              shaft->setSegments(segments());
              shaft->setHeadWidth(headWidth);
              shaft->setBorderedLine(linef);
              shaft->setZValue(10);
              addToGroup(shaft);
          }
      }
          break;
      case ThreeSegments:
      {
          if (i == Seg::first) {
              linef = QLineF(points[Base],points[MidBase]);
              removeFromGroup(shaftSegments[i]);
              BorderedLineItem *shaft = shaftSegments[i];
              shaft->setSegment(OneSegment);
              shaft->setSegments(segments());
              shaft->setHeadWidth(headWidth);
              shaft->setBorderedLine(linef);
              shaft->setZValue(30);
              addToGroup(shaft);
          } else if (i == Seg::second) {
              linef = QLineF(points[MidBase],points[MidTip]);
              removeFromGroup(shaftSegments[i]);
              BorderedLineItem *shaft = shaftSegments[i];
              shaft->setSegment(TwoSegments);
              shaft->setSegments(segments());
              shaft->setHeadWidth(headWidth);
              shaft->setBorderedLine(linef);
              shaft->setZValue(20);
              addToGroup(shaft);
          } else if (i == Seg::third) {
              linef = QLineF(points[MidTip],points[Tip]);
              removeFromGroup(shaftSegments[i]);
              BorderedLineItem *shaft = shaftSegments[i];
              shaft->setSegment(ThreeSegments);
              shaft->setSegments(segments());
              shaft->setHeadWidth(headWidth);
              shaft->setBorderedLine(linef);
              shaft->setZValue(10);
              addToGroup(shaft);
          }
      }
          break;
      default:
          break;
      }

      // head
      if (i == (segments()-1)){

          removeFromGroup(head);

          head->setPolygon(poly);

          qreal x;
          qreal y;
          switch (segments()) {
          case OneSegment:
              x = points[Tip].x()-points[Base].x();
              y = points[Tip].y()-points[Base].y();
              break;
          case TwoSegments:
              x = points[Tip].x()-points[MidBase].x();
              y = points[Tip].y()-points[MidBase].y();
              break;
          case ThreeSegments:
              x = points[Tip].x()-points[MidTip].x();
              y = points[Tip].y()-points[MidTip].y();
              break;
          default:
              break;
          }

          qreal h = sqrt(x*x+y*y);
          qreal angle = 180*acos(x/h);

          qreal pi = 22.0/7;

          if (x == 0) {
            if (y < 0) {
              angle = 270.0;
            } else {
              angle = 90.0;
            }
          } else if (y == 0) {
            if (x < 0) {
              angle = 180.0;
            } else {
              angle = 0.0;
            }
          } else {
            volatile qreal h = sqrt(x*x+y*y);
            if (x > 0) {
              if (y > 0) {
                angle = 180-180*acos(-x/h)/pi;
              } else {
                angle = 180+180*acos(-x/h)/pi;
              }
            } else {
              if (y > 0) {
                angle = 180*acos(x/h)/pi;
              } else {
                angle = -180*acos(x/h)/pi;
              }
            }
          }

          head->setZValue(-1);
          head->resetTransform();
          head->setRotation(rotation() + angle);
          head->setPos(points[Tip]);
          addToGroup(head);
      }
  }

  view->updateSceneRect(sceneBoundingRect());
}

void PointerItem::addShaftSegment(){

    QLineF linefNew;

    switch (segments()) {
    case OneSegment:
    {
        // get split target segment - first shaft
        points[MidBase] = (QLineF(points[Base],points[Tip]).p1() * 0.5 +
                           QLineF(points[Base],points[Tip]).p2() * 0.5);

        linefNew        = QLineF(points[MidBase],points[Tip]);
    }
        break;
    case TwoSegments:
    {
        // get split target segment - second shaft
        points[MidTip] = (QLineF(points[MidBase],points[Tip]).p1() * 0.5 +
                          QLineF(points[MidBase],points[Tip]).p2() * 0.5);
        linefNew       = QLineF(points[MidTip],points[Tip]);
        if (points[MidTip].y() == points[MidBase].y()){
            emit gui->messageSig(LOG_ERROR, "Pointer segments are on the same line. Move an existing segment before creating a new one.");
            return;
        }
    }
        break;
    case ThreeSegments:
    {
        emit gui->messageSig(LOG_ERROR, "Maximum number (%1) of pointer segments already defined.");
          return;
    }
        break;
    default:
        break;
    }

#ifdef QT_DEBUG_MODE
//        logDebug() << "\n[DEBUG SEGMENTS]"
//                      "\n[linefOrig]: " << linefOrig <<
//                      "\n[centerPos]: " << centerPos <<
//                      "\n[linefNew] : " << linefNew;
#endif

    PointerAttribData *pad = &pointer.pointerAttrib.valuePixels();
    thickness = pad->lineData.thickness;

    QColor penColor(pad->lineData.color);
    QColor brushColor(pad->lineData.color);

    QPen pen(penColor);
    pen.setWidth(pad->lineData.thickness);
    pen.setCapStyle(Qt::RoundCap);
    pen.setJoinStyle(Qt::RoundJoin);
    if (pad->lineData.line == BorderData::BdrLnSolid){
        pen.setStyle(Qt::SolidLine);
    }
    else if (pad->lineData.line == BorderData::BdrLnDash){
        pen.setStyle(Qt::DashLine);
    }
    else if (pad->lineData.line == BorderData::BdrLnDot){
        pen.setStyle(Qt::DotLine);
    }
    else if (pad->lineData.line == BorderData::BdrLnDashDot){
        pen.setStyle(Qt::DashDotLine);
    }
    else if (pad->lineData.line == BorderData::BdrLnDashDotDot){
        pen.setStyle(Qt::DashDotDotLine);
    }

    shaft = new BorderedLineItem(linefNew,pad,this);
    shaft->setPen(pen);
    shaft->setFlag(QGraphicsItem::ItemIsSelectable,false);
    shaft->setToolTip(QString("Pointer segment %1 - drag to move; right click to modify").arg(segments()+1));
    shaftSegments.append(shaft);
    addToGroup(shaft);

    for (int i = 0; i < NumGrabbers; i++) {
      grabbers[i] = nullptr;
    }

    drawPointerPoly();

    setFlag(QGraphicsItem::ItemIsFocusable,true);

    placeGrabbers();
}

void PointerItem::removeShaftSegment(){

    int LastSegment = segments()-1;
    removeFromGroup(shaftSegments[LastSegment]);
    shaftSegments.removeLast();
    change();
}

/*
 * Given the location of Loc (as dragged around by the user)
 * calculate a reasonable placement and Loc for points[Base]
 */

bool PointerItem::autoLocFromBase(
  QPoint loc)
{
  QRect rect(0,
             0,
             baseX,
             baseY);

  QPoint intersect;
  PlacementEnc tplacement;

  QPoint point;
  if(segments() == OneSegment){
      point = QPoint(points[Tip].x() + 0.5, points[Tip].y() + 0.5);
  } else {
      point = QPoint(points[MidBase].x() + 0.5, points[MidBase].y() + 0.5);
  }

  if (rectLineIntersect(point,
                        loc,
                        rect,
                        thickness,
                        intersect,
                        tplacement)) {
    placement = tplacement;
    points[Base] = intersect;
    return true;
  }    
  return false;
}

/*
 * Given the location of the MidTip point (as dragged around by the user)
 * calculate a reasonable position for point MidBase
 */
bool PointerItem::autoMidBaseFromMidTip()
{
    PointerData pointerData = pointer.pointerMeta.value();
    int tx,ty;

    tx = points[MidTip].x();
    ty = pointerData.y2;

    points[MidBase].setY(ty);
    points[MidBase].setX(tx);

    return true;
}

void PointerItem::placeGrabbers()
{
  int numGrabbers;
  switch (segments()) {
  case OneSegment:
      numGrabbers = NumGrabbers-2;
      break;
  case TwoSegments:
      numGrabbers = NumGrabbers-1;
      break;
  case ThreeSegments:
      numGrabbers = NumGrabbers;
      break;
  default:
      break;
  }

  if (grabbers[0] == nullptr) {
    for (int i = 0; i < numGrabbers; i++) {
      grabbers[i] = new Grabber(i,this,myParentItem());
    }
  }

  for (int i = 0; i < numGrabbers; i++) {
    grabbers[i]->setPos(points[i].x() - grabSize()/2, points[i].y()-grabSize()/2);
  }

}
      /*
       *     c1 c2  c1 c2
       *    +--+------+--+
       *  c1|            | c1
       *    +            +
       *  c2|            | c2
       *    |            |
       *    |            |
       *  c1|            | c1
       *    +            +
       *  c2|            | c2
       *    +--+------+--+
       *     c1 c2  c1 c2
       *
       */

      /*     c1        c1
       *    +------------+
       *  c2|            | c2
       *    |            |
       *    |            |   
       *    |            |
       *    |            |
       *    |            |   
       *    |            |
       *  c2|            | c2
       *    +------------+
       *     c1        c1
       */
void PointerItem::mousePressEvent  (QGraphicsSceneMouseEvent * /* unused */)
{
  placeGrabbers();
}

/*
 * Mouse move.  What we do depends on what was selected by the mouse click.
 *
 * If pointer's tip is selected then allow the tip to move anywhere within the
 * CSI or page.
 *
 * If pointer's mid-base is selected then allow the mid-base to move anywhere
 * within the CSI or page.
 *
 * If the pointer's location (where the pointer is attached to the base object)
 * is selected then allow the pointer to move to any valid edges or corner
 * (constrained by base).
 *
 * If the two outside corners are selected, then the user is trying to adjust the
 * base.
 */

void PointerItem::resize(QPointF grabbed)
{
    grabbed -= scenePos();

    bool changed = false;
    resizeRequested = true;

    switch (selectedGrabber) {
    /*
     * Loc must track along an edge.  If you draw a line from tip to the mouse
     * cursor, the line intersects with the edge of the base object one or two places.
     * The single intersection is at corners, and the dual intersection is not
     * on corners.  In the case of dual intersection, the one that is closest
     * to tip is the one used.
     *
     * If the tip to mouse line does not intersect with the base object, do nothing.
     * If the top to mouse line only intersects with corner, then force corner
     * placement.
     *
     * If the the tip to mouse line intersects twice, then the closest intersect is
     * used.  if the distance between the intersect and the corner is less than
     * base, react as thought we were at corner.
     */

    case Base:
    {
        QPoint loc(grabbed.x(),grabbed.y());

        if (autoLocFromBase(loc)) {
            changed = true;
        }
    }
        break;
    case Tip:
        points[Tip] = grabbed;
        if (autoLocFromTip()) {
            changed = true;
        }
        break;
    case MidBase:
        points[MidBase] = grabbed;
        if (autoLocFromMidBase()) {
            changed = true;
        }
        break;
    case MidTip:
        points[MidTip] = grabbed;
        if (autoMidBaseFromMidTip()) {
            changed = true;
        }
        break;
    default:
        break;
    }
  if (changed) {
    drawPointerPoly();
    positionChanged = true;
    resizeRequested = false;
  }
  switch (segments()) {
  case OneSegment:
      grabbers[Base    ]->setPos(points[Base    ].x() - grabSize()/2, points[Base    ].y() - grabSize()/2);
      grabbers[Tip     ]->setPos(points[Tip     ].x() - grabSize()/2, points[Tip     ].y() - grabSize()/2);
      break;
  case TwoSegments:
      grabbers[Base    ]->setPos(points[Base    ].x() - grabSize()/2, points[Base    ].y() - grabSize()/2);
      grabbers[MidBase ]->setPos(points[MidBase ].x() - grabSize()/2, points[MidBase ].y() - grabSize()/2);
      grabbers[Tip     ]->setPos(points[Tip     ].x() - grabSize()/2, points[Tip     ].y() - grabSize()/2);
      break;
  case ThreeSegments:
      grabbers[Base    ]->setPos(points[Base    ].x() - grabSize()/2, points[Base    ].y() - grabSize()/2);
      grabbers[MidBase ]->setPos(points[MidBase ].x() - grabSize()/2, points[MidBase ].y() - grabSize()/2);
      grabbers[MidTip  ]->setPos(points[MidTip  ].x() - grabSize()/2, points[MidTip  ].y() - grabSize()/2);
      grabbers[Tip     ]->setPos(points[Tip     ].x() - grabSize()/2, points[Tip     ].y() - grabSize()/2);
      break;
  default:
      break;
  }
}

void PointerItem::change()
{
  calculatePointerMeta();
  MetaItem::updatePointer(pointer.here, &pointer.pointerMeta);
}

/* Meta related stuff */

void PointerItem::drawTip(QPoint delta)
{
  points[Tip] += delta;
  autoLocFromTip();
  drawPointerPoly();
  points[Tip] -= delta;
}

void PointerItem::updatePointer(
  QPoint &delta)
{
  points[Tip] += delta;
  autoLocFromTip();
  drawPointerPoly();
  calculatePointerMetaLoc();

  MetaItem::updatePointer(pointer.here, &pointer.pointerMeta);
}

void PointerItem::addPointerMeta()
{
  QString metaString = pointer.pointerMeta.format(false,false);
  Where here = pointer.here+1;
  insertMeta(here,metaString);
  gui->displayPage();
}

void PointerItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
  QMenu menu;
  bool isCallout = pointerParentType == CalloutType;
  Where pointerAttribTop, pointerAttribBottom;
  pointerAttribTop    = pointer.pointerAttrib.here();
  pointerAttribBottom = pointer.pointerAttrib.here();

  QAction *setFillAttributesAction = menu.addAction("Set Line Attributes");
  setFillAttributesAction->setIcon(QIcon(":/resources/fillattributes.png"));
  setFillAttributesAction->setWhatsThis(     "Set pointer line attributes:\n"
                                         "Set the pointer line color, thickness, and type");

  QAction *setBorderAttributesAction = menu.addAction("Set Border Attributes");
  setBorderAttributesAction->setIcon(QIcon(":/resources/borderattributes.png"));
  setBorderAttributesAction->setWhatsThis(     "Set pointer fill attributes:\n"
                                         "Set the pointer border color, thickness, and line type");

  QAction *removeAction = menu.addAction("Delete Pointer");
  removeAction->setIcon(QIcon(":/resources/deletepointer.png"));
  removeAction->setWhatsThis(            "Delete this Pointer:\n"
                                         "Deletes this pointer");

  QAction *addSegmentAction = nullptr;
  if (segments() < 3) {
      addSegmentAction = menu.addAction("Add Pointer Segment");
      addSegmentAction->setIcon(QIcon(":/resources/addpointersegment.png"));
      addSegmentAction->setWhatsThis(        "Add pointer segment:\n"
                                             "Introduce a new pointer shaft segment.");
  }

  QAction *removeSegmentAction = nullptr;
  if (segments() > 1) {
      removeSegmentAction = menu.addAction("Remove Pointer Segment");
      removeSegmentAction->setIcon(QIcon(":/resources/removepointersegment.png"));
      removeSegmentAction->setWhatsThis(     "Remove pointer segment:\n"
                                             "Remove pointer shaft segment.");
  }

  QAction *selectedAction   = menu.exec(event->screenPos());

  if ( ! selectedAction ) {
      return;
    }

  if (selectedAction == setFillAttributesAction) {
      setPointerAttrib("Pointer Line Attributes",
                        pointerAttribTop,
                        pointerAttribBottom,
                       &pointer.pointerAttrib,false,1,false,isCallout);
  }
  else
  if (selectedAction == setBorderAttributesAction) {
      setPointerAttrib("Pointer Border Attributes",
                        pointerAttribTop,
                        pointerAttribBottom,
                       &pointer.pointerAttrib,false,1,false,isCallout,false);
  }
  else
  if (selectedAction == removeAction) {
    deletePointer(pointer.here);
  }
  else
  if (selectedAction == addSegmentAction) {
    addShaftSegment();
  }
  else
  if (selectedAction == removeSegmentAction) {
    removeShaftSegment();
  }
}


/* calculate the parameters for the equation of line from two points */

bool lineEquation(
  QPoint point[2],
  qreal &a,
  qreal &b,
  qreal &c)
{
  qreal xlk = point[0].x() - point[1].x();
  qreal ylk = point[0].y() - point[1].y();
  qreal rsq = xlk*xlk+ylk*ylk;
  if (rsq < 1) {
    return false;
  }
  qreal rinv = 1.0/sqrt(rsq);
  a = -ylk*rinv;
  b =  xlk*rinv;
  c = (point[1].x()*point[0].y() - point[0].x()*point[1].y())*rinv;
  return true;
}

/* calculate the intersection of two lines */

bool lineIntersect(
  qreal a1, qreal b1, qreal c1,
  qreal a2, qreal b2, qreal c2,
  QPoint &intersect)
{
  qreal det = a1*b2 - a2*b1;
  if (fabs(det) < 1e-6) {
    return false;
  } else {
    qreal dinv = 1.0/det;
    intersect.setX((b1*c2 - b2*c1)*dinv);
    intersect.setY((a2*c1 - a1*c2)*dinv);
    return true;
  }
}

/* determine if a line intersects a Horizontal line segment, if so return the intersection */

bool lineIntersectHorizSeg(
  qreal a1, qreal b1, qreal c1,
  QPoint seg[2], QPoint &intersect)
{
  qreal a2, b2, c2;
  if ( ! lineEquation(seg,a2,b2,c2)) {
    return false;
  }

  if ( ! lineIntersect(a1,b1,c1,a2,b2,c2,intersect)) {
    return false;
  }

  return intersect.x() >= seg[0].x() && intersect.x() <= seg[1].x();
}

/* determine if a line intersects a Vertical line segment, if so return the intersection */

bool lineIntersectVertSeg(
  qreal a1, qreal b1, qreal c1,
  QPoint seg[2], QPoint &intersect)
{
  qreal a2, b2, c2;
  if ( ! lineEquation(seg,a2,b2,c2)) {
    return false;
  }

  if ( ! lineIntersect(a1,b1,c1,a2,b2,c2,intersect)) {
    return false;
  }

  return intersect.y() >= seg[0].y() && intersect.y() <= seg[1].y();
}
/*
 * Given Tip and Loc that form a line and an rectangle in 2D space, calculate the
 * intersection of the line and rect that is closes to tip.
 *
 * iv_top && iv_left
 * iv_top && iv_bottom
 * iv_top && iv_right
 * iv_left && iv_bottom
 * iv_left && iv_right
 * iv_bottom && iv_right
 *
 * The line will always intersect two sides, unless it just hits the corners.
 *
 */

bool rectLineIntersect(
  QPoint        tip,
  QPoint        loc,
  QRect         rect, 
  int           base,
  QPoint       &intersect,
  PlacementEnc &placement)
{
  int width = rect.width();
  int height = rect.height();
  int tipX = tip.x();
  int tipY = tip.y();
  int tx,ty;

  QPoint seg[2];
  QPoint intersect_top,intersect_bottom,intersect_left,intersect_right;
  bool   iv_top, iv_bottom,iv_left,iv_right;

  /* calculate formula for line between tip and {C1,C2,Loc) */

  seg[0] = tip;
  seg[1] = loc;
  qreal a, b, c;
  if ( ! lineEquation(seg,a,b,c)) {
    return false;
  }

  /* top Horizontal line */

  seg[0].setX(0);
  seg[1].setX(rect.width());
  seg[0].setY(rect.top());
  seg[1].setY(rect.top());
  iv_top = lineIntersectHorizSeg(a,b,c,seg,intersect_top);

  tx = tipX - intersect_top.x();
  ty = tipY - intersect_top.y();
  int top_dist = tx*tx+ty*ty;

  /* bottom Horizontal line */

  seg[0].setY(rect.height());
  seg[1].setY(rect.height());

  iv_bottom = lineIntersectHorizSeg(a,b,c,seg,intersect_bottom);

  tx = tipX - intersect_bottom.x();
  ty = tipY - intersect_bottom.y();
  int bot_dist = tx*tx+ty*ty;

  /* left Vertical line */

  seg[0].setX(0);
  seg[1].setX(0);
  seg[0].setY(rect.top());
  seg[1].setY(rect.height());

  iv_left = lineIntersectVertSeg(a,b,c,seg,intersect_left);

  tx = tipX - intersect_left.x();
  ty = tipY - intersect_left.y();
  int left_dist = tx*tx+ty*ty;

  /* right Vertical line */

  seg[0].setX(rect.width());
  seg[1].setX(rect.width());

  iv_right = lineIntersectVertSeg(a,b,c,seg,intersect_right);

  tx = tipX - intersect_right.x();
  ty = tipY - intersect_right.y();
  int right_dist = tx*tx+ty*ty;

  if (iv_top && iv_bottom) {

    /* Is mouse tip closer to top or bottom? */

    if (top_dist < bot_dist) {

      /* tip closer to top */

      if (intersect_top.x() < base) {
        placement = TopLeft;
        intersect.setX(0);
      } else if (width - intersect_top.x() < base) {
        placement = TopRight;
        intersect.setX(width);
      } else {
        placement = Top;
        int x = intersect_top.x();
        x = x < base
              ? base
              : x > width - base
                  ? width - base
                  : x;
        intersect.setX(x);
      }
      intersect.setY(0);
    } else {
      if (intersect_bottom.x() < base) {
        placement = BottomLeft;
        intersect.setX(0);
      } else if (width - intersect_bottom.x() < base) {
        placement = BottomRight;
        intersect.setX(width);
      } else {
        placement = Bottom;
        int x = intersect_top.x();
        x = x < base
              ? base
              : x > width - base
                  ? width - base
                  : x;
        intersect.setY(intersect_bottom.y());
        intersect.setX(x);
      }
      intersect.setY(height);
    }
  } else if (iv_left && iv_right) {

    /* Is the tip closer to right or left? */

    if (left_dist < right_dist) {

      /* closer to left */

      if (intersect_left.y() < base) {
        placement = TopLeft;
        intersect.setY(0);
      } else if (height - intersect_left.y() < base) {
        placement = BottomLeft;
        intersect.setY(height);
      } else {
        placement = Left;
        int y = intersect_left.y();
        y = y < base
              ? base
              : y > height - base
                ? height - base
                : y;
        intersect.setY(intersect_left.y());
      }
      intersect.setX(0);
    } else {
      if (intersect_right.y() < base && tipY < 0) {
        placement = TopRight;
        intersect.setY(0);
      } else if (height - intersect_right.y() < base && tipY > height) {
        placement = BottomRight;
        intersect.setY(height);
      } else {
        placement = Right;
        int y = intersect_right.y();
        y = y < base
                ? base
                : y > height - base
                  ? height - base
                  : y;
        intersect.setY(y);
      }
      intersect.setX(width);
    }
  } else if (iv_top && iv_left) {

    if (top_dist <= left_dist) {

      /* tip above the rectangle - line going down/left */

      if (width - intersect_top.x() < base) {
        placement = TopRight;
        intersect.setX(width);
      } else {
        placement = Top; 
        intersect.setX(intersect_top.x());
        if (intersect.x() < base) {
          intersect.setX(base);
        }
      }
      intersect.setY(0);
    } else {

      /* tip to left of rectangle - line going up/right*/
     
      if (height - intersect_left.y() < base) {
        placement = BottomLeft;
        intersect.setY(height);
      } else {
        placement = Left;
        intersect.setY(intersect_left.y());
        if (intersect.y() < base) {
          intersect.setY(base);
        }
      }
      intersect.setX(0);
    }
  } else if (iv_top && iv_right) {

    if (top_dist <= right_dist) {

      /* tip above the rectangle - line going down/right */

      if (intersect_top.x() < base) {
        placement = TopLeft;
        intersect.setX(0);
      } else {
        placement = Top; 
        intersect.setX(intersect_top.x());
        if (width - intersect.x() < base) {
          intersect.setX(width - base);
        }
      }
      intersect.setY(0);
    } else {

      /* tip to right of rectangle - line going up/left*/
     
      if (height - intersect_left.y() < base) {
        placement = BottomRight;
        intersect.setY(height);
      } else {
        placement = Right;
        intersect.setY(intersect_right.y());
        if (intersect.y() < base) {
          intersect.setY(base);
        }
      }
      intersect.setX(width);
    }
  } else if (iv_bottom && iv_right) {

    if (bot_dist <= right_dist) {

      /* tip below the rectangle - line going up/right */

      if (intersect_bottom.x() < base) {
        placement = BottomLeft;
        intersect.setX(0);
      } else {
        placement = Bottom; 
        intersect.setX(intersect_bottom.x());
        if (width - intersect.x() < base) {
          intersect.setX(width - base);
        }
      }
      intersect.setY(height);
    } else {

      /* tip to right of rectangle - line going down/left */
     
      if (intersect_right.y() < base) {
        placement = TopRight;
        intersect.setY(0);
      } else {
        placement = Right;
        intersect.setY(intersect_right.y());
        if (height - intersect.y() < base) {
          intersect.setY(height - base);
        }
      }
      intersect.setX(width);
    }
  } else if (iv_bottom && iv_left) {

    if (bot_dist <= left_dist) {

      /* tip below the rectangle - line going up/left */

      if (width - intersect_bottom.x() < base) {
        placement = BottomRight;
        intersect.setX(width);
      } else {
        placement = Bottom; 
        intersect.setX(intersect_bottom.x());
        if (intersect.x() < base) {
          intersect.setX(base);
        }
      }
      intersect.setY(height);
    } else {

      /* tip left of the rectangle */
     
      if (intersect_left.y() < base) {
        placement = TopLeft;
        intersect.setY(0);
      } else {
        placement = Left;
        intersect.setY(intersect_left.y());
        if (height - intersect.y() < base) {
          intersect.setY(height - base);
        }
      }
      intersect.setX(0);
    }
  } else {
    /* Bah! The user isn't anywhere near the rectangle */
    return false;
  }
  return true;
}
