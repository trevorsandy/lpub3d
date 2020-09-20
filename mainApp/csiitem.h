 
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
 * The class described in this file is the graphical representation of
 * a step's construction step image (CSI), or assembly image.
 *
 * Please see lpub.h for an overall description of how the files in LPub
 * make up the LPub program.
 *
 ***************************************************************************/

#ifndef csiH
#define csiH

#include <QGraphicsPixmapItem>
#include <QSize>
#include <QRect>
#include "meta.h"
#include "metaitem.h"
#include "resize.h"

#include "QsLog.h"

class Step;
class CsiAnnotation;
class CsiAnnotationItem;
class PartLineAttributes;

class CsiItem : public ResizePixmapItem, public QObject
{
public:
  Meta              *meta;
  AssemMeta         *assem;
  RcMeta            *divider;
  Step              *step;
  PlacementType      parentRelativeType;
  bool               hiddenAnnotations;
  int                submodelLevel;
  FloatMeta          modelScale;

  CsiItem(
          Step          *_step,
          Meta          *_meta,
          QPixmap       &pixmap,
          int            _submodelLevel,
          QGraphicsItem *parent,
          PlacementType  _parentRelativeType);

  void placeCsiPartAnnotations();

  void setFlag(GraphicsItemFlag flag, bool value);

  static void partLine(
     const QString      &partLine,
     PartLineAttributes &pla,
     int                 index,
     Rc                  rc);

  static void partLine(
     const QString &partLine,
     QStringList   &csiParts,
     QVector<int>  &typeIndexes,
     int            lineNumber,
     Rc             rc);

  virtual void change();

private:
  virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
  virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
  virtual void hoverEnterEvent(QGraphicsSceneHoverEvent* event);
  virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent* event);
  virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
  virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
  virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
  bool isHovered;
  bool mouseIsDown;
};

#endif

