/****************************************************************************
**
** Copyright (C) 2019 - 2020 Trevor SANDY. All rights reserved.
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

#ifndef CSIANNOTATION_H
#define CSIANNOTATION_H

#include <QGraphicsRectItem>
#include <QGraphicsTextItem>
#include "range_element.h"
#include "csiitem.h"
#include "ranges.h"
#include "resize.h"

class Step;
class QGraphicsView;
class CsiAnnotation;

class PlacementCsiPart : public Placement,
                         public QGraphicsRectItem
{
public:
    bool outline;
    int stepNumber;
    Where top, bottom;
    PlacementCsiPart(){}
    PlacementCsiPart(
        CsiPartMeta   &_csiPartMeta,
        QGraphicsItem *_parent = nullptr);
    bool hasOffset();
    void toggleOutline();
    void setOutline(QPainter *painter);
    void paint( QPainter *painter, const QStyleOptionGraphicsItem *o, QWidget *w);
};

class CsiAnnotation : public Placement
{
public:
  CsiAnnotationMeta caMeta;
  CsiPartMeta       csiPartMeta;
  Where             partLine,metaLine; // Part / Meta line in the model file

  CsiAnnotation(){}
  CsiAnnotation(
     const Where       &_here,
     CsiAnnotationMeta &_caMeta);
  virtual ~CsiAnnotation(){}
  bool setPlacement();
  bool setCsiPartLoc(int csiSize[]);
  bool setAnnotationLoc(float iconOffset[]);
};

class CsiAnnotationItem : public ResizeTextItem
{
public:
  PlacementCsiPart *placementCsiPart;
  Where             topOf,bottomOf;
  Where             partLine, metaLine;
  BorderMeta        border;
  BackgroundMeta    background;
  IntMeta           style;
  PlacementType     parentRelativeType;
  int               submodelLevel;
  StringListMeta    subModelColor;
  CsiAnnotationIconMeta icon;
  QRectF            textRect;
  QRectF            styleRect;
  int               stepNumber;

  // DEBUG TRACE STUFF
  PageMeta          pageMeta;
  QRect             csiItemRect;

  CsiAnnotationItem(
    QGraphicsItem *_parent = nullptr);

  virtual ~CsiAnnotationItem(){}

  void sizeIt();

  void setText(
    QString &text,
    QString &fontString,
    QString &toolTip)
  {
    setPlainText(text);
    QFont font;
    font.fromString(fontString);
    setFont(font);
    setToolTip(toolTip);
    setData(ObjectId, AssemAnnotationObj);
  }

  virtual void addGraphicsItems(
     CsiAnnotation *_ca,
     Step          *_step,
     PliPart       *_part,
     CsiItem       *_csiItem,
     bool           _movable);

  void setPos(double x, double y)
  {
    QGraphicsTextItem::setPos(x,y);
  }

  void setFlag(GraphicsItemFlag flag, bool value)
  {
    QGraphicsItem::setFlag(flag,value);
  }

  void setAlignment( Qt::Alignment flags )
  {
    alignment = flags;
  }

  void scaleDownFont();

protected:
  void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
  void mousePressEvent(QGraphicsSceneMouseEvent *event);
  void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
  void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
  void setAnnotationStyle(QPainter *painter);
  void paint(QPainter *painter, const QStyleOptionGraphicsItem *o, QWidget *w);
  void change();

  QPointF              textOffset;
  Qt::Alignment	       alignment;
};

#endif // CSIANNOTATION_H
