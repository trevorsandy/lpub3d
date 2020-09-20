
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
 * This is the graphical representation of step groups.
 *
 * Please see lpub.h for an overall description of how the files in LPub
 * make up the LPub program.
 *
 ***************************************************************************/

#ifndef ranges_itemH
#define ranges_itemH

#include <QGraphicsRectItem>
#include "placement.h"
#include "metaitem.h"
#include "ranges.h"
#include "where.h"

class Page;
class Steps;

class MultiStepRangesBackgroundItem : public QGraphicsRectItem, public MetaItem
{
  Steps  *page;
  Meta   *meta;
  bool    positionChanged;
  QPointF position;

public:
  MultiStepRangesBackgroundItem(
    Steps *_steps,
    QRectF rect,
    QGraphicsItem *parent, 
    Meta *_meta);
protected:
  virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
  virtual void mouseMoveEvent( QGraphicsSceneMouseEvent *event);
  virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
  virtual void hoverEnterEvent(QGraphicsSceneHoverEvent* event);
  virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent* event);
  virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
  bool isHovered;
  bool mouseIsDown;
};

class Range;

class MultiStepRangeBackgroundItem : public QGraphicsRectItem, public MetaItem
{
public:
    PlacementType   relativeType; // single step, multi step
    PlacementType   parentRelativeType;
    Meta           *meta;
    BackgroundMeta *background;
    BorderMeta     *border;
    PlacementMeta  *placement;
    MarginsMeta    *margin;
    BoolMeta       *perStep;
    AllocMeta      *alloc;
    FreeFormMeta   *freeform;
    FontListMeta   *subModelFont;
    StringListMeta *subModelFontColor;
    Steps          *page;

    MultiStepRangeBackgroundItem(
      Steps         *_steps,
      Range         *_range,
      Meta          *_meta,
      int            _offset_x,
      int            _offset_y,
      QGraphicsItem *parent);

    void addStep(Meta *meta);
  
protected:
    void contextMenuEvent (QGraphicsSceneContextMenuEvent *event);
};

class DividerBackgroundItem : public QGraphicsRectItem, public MetaItem
{
public:
  BorderMeta     *border;
  int             stepNumber;
  Where           top,bottom;
  DividerBackgroundItem(
    Meta         *_meta,
    QRect        &_dividerRect,
    QGraphicsItem *parent = nullptr);

  void setPos(float x, float y)
  {
    QGraphicsRectItem::setPos(x,y);
  }
};

class Step;
class DividerLine;
class DividerItem;
class Pointer;
class DividerPointerItem;
class DividerBackgroundItem;
class DividerItem : public Steps, public QGraphicsRectItem, public MetaItem
{
public:
    Step                        *parentStep;
    DividerLine                 *lineItem;
    DividerBackgroundItem       *background;
    PlacementType                parentRelativeType;

    DividerItem() {}
    DividerItem(
      Step       *_step,
      Meta       *_meta,
      int         offsetX,
      int         offsetY);
    ~DividerItem();

    virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent* event);
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent* event);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    /******************************************************************************
     * Divider pointer item routines
     *****************************************************************************/

    virtual void addGraphicsPointerItem(Pointer *pointer, QGraphicsView *view);

    virtual void drawTips(QPoint &delta, int type = 0);

    virtual void updatePointers(QPoint &delta);

protected:
    bool isHovered;
    bool mouseIsDown;
};

class DividerLine : public QGraphicsLineItem, public MetaItem
{
public:
  int stepNumber;
  Where top,bottom;
  DividerLine(DividerItem *parent);

protected:
  void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
};    
  
#endif
