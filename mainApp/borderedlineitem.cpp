
/****************************************************************************
**
** Copyright (C) 2018 - 2020 Trevor SANDY. All rights reserved.
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
 * This class implements the QGraphicsLineItem with a minimal border.
 *
 * Please see lpub.h for an overall description of how the files in LPub
 * make up the LPub program.
 *
 ***************************************************************************/

#include "borderedlineitem.h"
#include "resolution.h"
#include "QsLog.h"

#define _USE_MATH_DEFINES
#include <math.h>

BorderedLineItem::BorderedLineItem(const QLineF &line, PointerAttribData* paData, QGraphicsItem *parent):
    QGraphicsLineItem(line, parent),pad(paData){

    setFlags(QGraphicsItem::ItemIsSelectable
             |QGraphicsItem::ItemIsMovable);
}

void BorderedLineItem::setBorderedLine(const QLineF &bLine) {

    qreal ft        = (pad->lineData.thickness/2);
    QLineF _bLine   = bLine;
    QPolygonF nPolygon;
    qreal radAngle;
    qreal dx = 0.0;
    qreal dy = 0.0;
    QPointF p1offset;
    QPointF p2offset;
    QPointF offset1;
    QPointF offset2;

    if (! pad->borderData.useDefault) {

        if (segment == segments)
            _bLine.setLength(bLine.length() - (headWidth/1.33));

        setLine(_bLine);
        radAngle = line().angle()* M_PI / 180;
        dx       = (ft) * sin(radAngle);
        dy       = (ft) * cos(radAngle);
        offset1  = QPointF(dx, dy);
        offset2  = QPointF(-dx, -dy);

        p1offset = getLineP1Offset();
        p2offset = getLineP2Offset();

        nPolygon << p1offset + offset1
                 << p1offset + offset2
                 << p2offset + offset2
                 << p2offset + offset1;
    } else {
        setLine(_bLine);
        radAngle = line().angle()* M_PI / 180;
        dx       = (ft) * sin(radAngle);
        dy       = (ft) * cos(radAngle);
        offset1  = QPointF(dx, dy);
        offset2  = QPointF(-dx, -dy);

        nPolygon << line().p1() + offset1
                 << line().p1() + offset2
                 << line().p2() + offset2
                 << line().p2() + offset1;
    }
#ifdef QT_DEBUG_MODE
//    logTrace() << "\n[DEBUG BORDERED-LINE THICKNESS -" << (resolutionType() == DPCM ? "CENTIMETERS]:" : "INCHES]:")
//               << "LINE THICKNESS:" << QString::number(pad->lineData.thickness)
//               << "BORDER THICKNESS:" << QString::number(pad->borderData.thickness)
//    ;
//    logTrace() << "\n[DEBUG BORDERED POLYGON]:"
//               << "\nSEGMENT: " << segment
//               << "\nLINE:    " << line()
//               << "\nP1OFFSET:" << p1offset
//               << "\nP2OFFSET:" << p2offset
//               << "\nPOLYGON: "
//               << p1offset + offset1
//               << p1offset + offset2
//               << p2offset + offset2
//               << p2offset + offset1
//    ;
#endif
    borderPolygon = nPolygon;
    update();
}

QPointF BorderedLineItem::getLineP1Offset(){

    bool hideTip = pad->lineData.hideArrows;

    if (segment == 1 && ! hideTip)
        return line().p1();

    qreal ft = (pad->lineData.thickness/2 + (hideTip ? 1 : 0));

    const QPointF lp1 = line().p1();
    const QPointF lp2 = line().p2();

    QPointF offset;
    if (lp1.x() == lp2.x())                 // vertical line, adjust the y axis
        if (lp1.y() < lp2.y())
            offset = QPointF(lp1.x(), (hideTip ? lp1.y() + ft : lp1.y() - ft));
        else
            offset = QPointF(lp1.x(), (hideTip ? lp1.y() - ft : lp1.y() + ft));
    else
    if (lp1.y() == lp2.y())                 // horizontal line, adjust the x axis
        if (lp1.x() < lp2.x())
            offset = QPointF((hideTip ? lp1.x() + ft : lp1.x() - ft), lp1.y());
        else
            offset = QPointF((hideTip ? lp1.x() - ft : lp1.x() + ft), lp1.y());
    else {                                  // sloped line, adjust both x and y axis
        // first the slope
        qreal m = (lp2.y() - lp1.y())/(lp2.x() - lp1.x());
        // then offset length and point distance
        qreal ld = (hideTip ? ft : -ft);
        qreal dx = (ld / sqrt(1 + (m * m)));
        qreal dy = m * dx;
        // then the offset point
        qreal px,py;
        if (lp2.x() > lp1.x()) {
            px = lp1.x() + dx;
            py = lp1.y() + dy;
        } else {
            px = lp1.x() - dx;
            py = lp1.y() - dy;
        }
        offset = QPointF(px,py);
    }
    return offset;
}

QPointF BorderedLineItem::getLineP2Offset(){

    const QPointF lp1 = line().p1();
    const QPointF lp2 = line().p2();

    // Adjust by 1 pixel
    qreal ft = (pad->lineData.thickness/2) + 1;

    QPointF offset;
    if (lp2.x() == lp1.x())                 // vertical line, adjust the y axis
        if (lp2.y() < lp1.y())
            offset = QPointF(lp2.x(), lp2.y() + ft);
        else
            offset = QPointF(lp2.x(), lp2.y() - ft);
    else
    if (lp2.y() == lp1.y())                 // horizontal line, adjust the x axis
        if (lp2.x() < lp1.x())
            offset = QPointF(lp2.x() + ft, lp2.y());
        else
            offset = QPointF(lp2.x() - ft, lp2.y());
    else {                                  // sloped line, adjust both x and y axis
        // first the slope
        qreal m = (lp2.y() - lp1.y())/(lp2.x() - lp1.x());
        // then offset length and point distance
        qreal ld = line().length() - ft;
        qreal dx = (ld / sqrt(1 + (m * m)));
        qreal dy = m * dx;
        // then the offset point
        qreal px,py;
        if (lp2.x() > lp1.x()) {
            px = lp1.x() + dx;
            py = lp1.y() + dy;
        } else {
            px = lp1.x() - dx;
            py = lp1.y() - dy;
        }
        offset = QPointF(px,py);
    }
    return offset;
}

QRectF BorderedLineItem::boundingRect() const {
    return borderPolygon.boundingRect();
}

QPainterPath BorderedLineItem::shape() const{
    QPainterPath ret;
    ret.addPolygon(borderPolygon);
    return ret;
}

void BorderedLineItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
                             QWidget *widget){
    Q_UNUSED(option);
    Q_UNUSED(widget);

    if (! pad->borderData.useDefault) {

        QPen borderPen(pad->borderData.color);
        borderPen.setWidth(pad->borderData.thickness);
        borderPen.setCapStyle(Qt::RoundCap);
        borderPen.setJoinStyle(Qt::RoundJoin);
        if (pad->borderData.line == BorderData::BdrLnSolid){
            borderPen.setStyle(Qt::SolidLine);
        }
        else if (pad->borderData.line == BorderData::BdrLnDash){
            borderPen.setStyle(Qt::DashLine);
        }
        else if (pad->borderData.line == BorderData::BdrLnDot){
            borderPen.setStyle(Qt::DotLine);
        }
        else if (pad->borderData.line == BorderData::BdrLnDashDot){
            borderPen.setStyle(Qt::DashDotLine);
        }
        else if (pad->borderData.line == BorderData::BdrLnDashDotDot){
            borderPen.setStyle(Qt::DashDotDotLine);
        }
        painter->setPen(borderPen);

    } else {

        QPen borderPen(Qt::NoPen);
        painter->setPen(borderPen);

    }

    painter->drawPolygon(borderPolygon);
    painter->setPen(pen());
    painter->drawLine(line());
}
