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

/****************************************************************************
 *
 * This class provides a simple mechanism for displaying a rotation icon
 * on the page.
 *
 ***************************************************************************/

#ifndef ROTATEICONITEM_H
#define ROTATEICONITEM_H

#include <QGraphicsPixmapItem>
#include <QSize>
#include <QRect>
#include "meta.h"
#include "metaitem.h"
#include "resize.h"

#include "QsLog.h"

class RotateIconItem : public ResizePixmapItem
{
public:
  Step                     *step;
  QPixmap                  *pixmap;

  FloatMeta                 picScale;
  RotateIconMeta            rotateIconMeta;
  PlacementType             parentRelativeType;

  PlacementRotateIcon       placementRotateIcon;

  bool                      positionChanged;
  QPointF                   position;

  RotateIconItem();

  RotateIconItem(
    Step           *_step,
    PlacementType   _parentRelativeType,
    RotateIconMeta &_rotateIconMeta,
    QGraphicsItem  *_parent = 0);
  ~RotateIconItem()
  {
  }

  void setAttributes(
      Step           *_step,
      PlacementType   _parentRelativeType,
      RotateIconMeta &_rotateIconMeta,
      QGraphicsItem  *parent = 0);

  void setRotateIconImage(QPixmap *pixmap);

protected:
  virtual void change();
  void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
};

class CalloutRotateIconItem : public RotateIconItem
{
  Step           *step;
  RotateIconMeta rotateIconMeta;
public:
  CalloutRotateIconItem(
    Step                  *_step,
    PlacementType          _parentRelativeType,
    CalloutRotateIconMeta &_rotateIconMeta,
    QGraphicsItem         *_parent = 0);
protected:
  virtual void change();
  void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
};

class MultiStepRotateIconItem : public RotateIconItem
{
  Step           *step;
  RotateIconMeta rotateIconMeta;
public:
  MultiStepRotateIconItem(
    Step                    *_step,
    PlacementType            _parentRelativeType,
    MultiStepRotateIconMeta &_rotateIconMeta,
    QGraphicsItem           *_parent = 0);
protected:
  virtual void change();
  void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
};

#endif // ROTATEICONITEM_H
