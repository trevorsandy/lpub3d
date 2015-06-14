/****************************************************************************
**
** Copyright (C) 2015 Trevor SANDY. All rights reserved.
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

#ifndef PAGEATTRIBUTEITEM_H
#define PAGEATTRIBUTEITEM_H

#include <QGraphicsTextItem>
#include "where.h"
#include "placement.h"
#include "metaitem.h"

class Where;
class Page;
class Step;

class PageAttributeItem : public QGraphicsTextItem, public MetaItem, public Placement
{
public:
  PlacementType  relativeType;
  PlacementType  parentRelativeType;
  Meta          *meta;
  FontMeta      *textFont;
  StringMeta    *textColor;
  MarginsMeta   *margin;
  AlignmentMeta *alignment;
  FloatMeta		*picScale;
  QString        value;
  QString        name;

  bool              positionChanged;
  QPointF           position;
  bool              textValueChanged;

  // Implement complete InsertMeta model (PageAttributeChangeMeta)
  // change to PageAttributeChangeMeta pageAttributeChangeMeta
  // InsertMeta     meta;

  PageAttributeItem();

  PageAttributeItem(
    PlacementType        relativeType,
    PlacementType        parentRelativeType,
    Meta                *meta,
    PageAttributeMeta   &pageAttribute,
    QString             _value,
    QString             &toolTip,
    QGraphicsItem       *parent,
    QString              name = "");

  void setAttributes(
    PlacementType        relativeType,
    PlacementType        parentRelativeType,
    Meta                *meta,
    PageAttributeMeta   &pageAttribute,
    QString             _value,
    QString             &toolTip,
    QGraphicsItem       *parent,
    QString              name = "");

  void setFlags( GraphicsItemFlag flag, bool value)
  {
    QGraphicsTextItem::setFlag(flag,value);
  }

};


class PagePageAttributeItem : public PageAttributeItem
{
  Page                *page;
public:
  PagePageAttributeItem(
    Page              *page,
    PageAttributeItem &pageAttribute,
    QString           _value,
    QGraphicsItem     *parent);
protected:
  void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
  virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

  virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
  virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);

  void focusInEvent(QFocusEvent *event);
  void focusOutEvent(QFocusEvent *event);
  void keyPressEvent(QKeyEvent *event);
  void keyReleaseEvent(QKeyEvent *event);
};


#endif // PAGEATTRIBUTEITEM_H
