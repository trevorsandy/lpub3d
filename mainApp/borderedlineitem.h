
/****************************************************************************
**
** Copyright (C) 2018 - 2019 Trevor SANDY. All rights reserved.
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

#ifndef BORDEREDLINEITEM_H
#define BORDEREDLINEITEM_H

#include <QGraphicsLineItem>
#include <QPen>
#include <QPainter>
#include "metatypes.h"
#include "where.h"

#include <cmath>

using namespace std;

class BorderedLineItem : public QGraphicsLineItem
{
public:
    int stepNumber;
    Where top,bottom;
    BorderedLineItem(const QLineF &line,
                     PointerAttribData* paData,
                     QGraphicsItem* parent = nullptr);
    void setSegment(const int seg) {segment = seg;}
    void setSegments(const int segs) {segments = segs;}
    void setHeadWidth(const float width) {headWidth = width;}
    void setBorderedLine(const QLineF &bLine);
    QRectF boundingRect() const;
    QPainterPath shape() const;
    void paint (QPainter* painter,
                const QStyleOptionGraphicsItem* option,
                QWidget* widget = nullptr);
private:
    QPointF getLineP1Offset();
    QPointF getLineP2Offset();
    int segment;
    int segments;
    qreal headWidth;
    PointerAttribData *pad;
    QPolygonF          borderPolygon;
};

#endif // BORDEREDLINEITEM_H
