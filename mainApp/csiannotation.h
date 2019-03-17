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
    PlacementCsiPart(){}
    PlacementCsiPart(
        CsiPartMeta   &_csiPartMeta,
        QGraphicsItem *_parent = nullptr);
    bool hasOffset();

    void paint( QPainter *painter, const QStyleOptionGraphicsItem *o, QWidget *w);
    void setBackground(QPainter *painter);
};

class CsiAnnotation : public Placement
{
public:
  CsiPartMeta         csiPartMeta;
  CsiAnnotationMeta   caMeta;
  AnnotationStyleMeta styleMeta;
  Where               partLine,metaLine; // Part / Meta line in the model file
  StringListMeta      subModelColor;
  int                 submodelLevel;

  CsiAnnotation(){}
  CsiAnnotation(
     const Where       &_here,
     CsiAnnotationMeta &_caMeta);
  virtual ~CsiAnnotation(){}
  bool setPlacement();
  void sizeIt(int &x, int &y);
  bool setCsiPartLoc(int csiSize[]);
  bool setAnnotationLoc(float iconOffset[]);
};

class CsiAnnotationItem : public ResizeTextItem
{
public:
  CsiAnnotation *ca;
  Where          topOf,bottomOf;
  PlacementType  parentRelativeType;
  // DEBUG TRACE STUFF
  PageMeta       pageMeta;
  QRect          csiItemRect;

  CsiAnnotationItem(
    QGraphicsItem *_parent = nullptr);
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
  virtual ~CsiAnnotationItem(){}
  void debugPlacementTrace();

protected:
  void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
  void mousePressEvent(QGraphicsSceneMouseEvent *event);
  void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
  void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
  void change();
  void setBackground(QPainter *painter);
  void paint(QPainter *painter, const QStyleOptionGraphicsItem *o, QWidget *w);
};

#endif // CSIANNOTATION_H
