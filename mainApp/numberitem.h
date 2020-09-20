 
/****************************************************************************
**
** Copyright (C) 2007-2009 Kevin Clague. All rights reserved.
** Copyright (C) 2015 - 2020 Trevor SANDY. All rights reserved.
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
 * This file describes the graphical representation of a number displayed
 * in the graphics scene that is used to describe a building instruction
 * page.
 *
 * Please see lpub.h for an overall description of how the files in LPub
 * make up the LPub program.
 *
 ***************************************************************************/

#ifndef NUMBERITEM_H
#define NUMBERITEM_H

#include <QGraphicsTextItem>
#include "where.h"
#include "placement.h"
#include "metaitem.h"

class Where;
class Page;
class Step;

class NumberItem : public QGraphicsTextItem, public MetaItem
{
public:
  PlacementType  relativeType;
  PlacementType  parentRelativeType;
  Meta          *meta;
  FontMeta      *font;
  StringMeta    *color;
  MarginsMeta   *margin;
  int            value;
  QString        name;

  NumberItem();
  
  NumberItem(
    PlacementType  relativeType,
    PlacementType  parentRelativeType,
    Meta          *meta,
    NumberMeta    &number,
    const char    *format,
    int            _value,
    QString       &toolTip,
    QGraphicsItem *parent,
    QString        name = "");

  void setAttributes(
    PlacementType  relativeType,
    PlacementType  parentRelativeType,
    Meta          *meta,
    NumberMeta    &number,
    const char    *format,
    int            _value,
    QString       &toolTip,
    QGraphicsItem *parent,
    QString        name = "");

  void setFlags( GraphicsItemFlag flag, bool value)
  {
    QGraphicsTextItem::setFlag(flag,value);
  }
};

class NumberPlacementItem : public QGraphicsTextItem, public MetaItem, public Placement
{
public:
  PlacementType  relativeType;
  PlacementType  parentRelativeType;
  FontMeta       font;
  StringMeta     color;
  int            value;
  QString        pl;

  bool           positionChanged;
  QPointF        position;

  NumberPlacementItem();
  
  NumberPlacementItem(
    PlacementType         relativeType,
    PlacementType         parentRelativeType,
    NumberPlacementMeta  &number,
    const char           *format,
    int                   value,
    QString              &toolTip,
    QGraphicsItem        *parent,
    QString               name = "");

  void setAttributes(
    PlacementType        relativeType,
    PlacementType        parentRelativeType,
    NumberPlacementMeta &number,
    const char          *format,
    int                  value,
    QString             &toolTip,
    QGraphicsItem       *parent,
    QString              name = "");

  void setFlags( GraphicsItemFlag flag, bool value)
  {
    QGraphicsTextItem::setFlag(flag,value);
  }

protected:
  virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
  virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
  virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
};

class GroupStepNumberItem : public NumberPlacementItem
{
  Page *page;
public:
  GroupStepNumberItem(
    Page          *page,
    NumberPlacementMeta &number,
    const char    *format,
    int            _value,
    QGraphicsItem *parent);
protected:
  void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
  virtual void hoverEnterEvent(QGraphicsSceneHoverEvent* event);
  virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent* event);
  virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
  virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
  virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
  bool isHovered;
  bool mouseIsDown;
};

class PageNumberItem : public NumberPlacementItem
{
  Page *page;
public:
  PageNumberItem(
    Page          *page,
    NumberPlacementMeta &number,
    const char    *format,
    int            _value,
    QGraphicsItem *parent);
protected:
  void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
  virtual void hoverEnterEvent(QGraphicsSceneHoverEvent* event);
  virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent* event);
  virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
  virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
  virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
  bool isHovered;
  bool mouseIsDown;
};

class StepNumberItem : public NumberPlacementItem
{
  Step *step;
public:
  StepNumberItem(
    Step          *step,
    PlacementType  parentRelativeType,
    NumberPlacementMeta &number,
    const char    *format,
    int            _value,
    QGraphicsItem *parent,
    QString        name = "Step Number ");
  Where top, bottom;
protected:
  void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
  virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
  virtual void hoverEnterEvent(QGraphicsSceneHoverEvent* event);
  virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent* event);
  virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
  virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
  bool isHovered;
  bool mouseIsDown;
};
#endif
