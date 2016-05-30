/****************************************************************************

**

** Copyright (C) 2007-2009 Kevin Clague. All rights reserved.
** Copyright (C) 2016 Trevor SANDY. All rights reserved.

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
 * This file describes a class that implements an LPub background.
 * Page, Parts Lists and Callouts have backgrounds.
 *
 * Please see lpub.h for an overall description of how the files in LPub
 * make up the LPub program.
 *
 ***************************************************************************/

#ifndef BACKGROUND_H 
#define BACKGROUND_H

#include <QObject>
#include <QGraphicsPixmapItem>
#include <QPixmap>
#include <QGradient>
#include "placement.h"
#include "metaitem.h"

class BackgroundItem : public QGraphicsPixmapItem, public MetaItem   
{ 
public: 
  Meta           *meta;
  BackgroundMeta  background;
  BorderMeta      border;
  MarginsMeta     margin;
  StringListMeta  subModelColor;
  int             submodelLevel;
  PlacementType   parentRelativeType;

  BackgroundItem()
  {}

  void setBackground(
      QPixmap         *pixmap,
      PlacementType    parentRelativeType,
      Meta            *meta,
      BackgroundMeta  &background,
      BorderMeta      &border,
      MarginsMeta     &margin,
      StringListMeta  &subModelColor,
      int              submodelLevel,
      QString         &toolTip);

private: 
  QGradient setGradient();
};

class PlacementBackgroundItem : public BackgroundItem   
{ 
public:  
  PlacementMeta  placement;
  bool           positionChanged;
  QPointF        position;
  PlacementType  relativeType;

  PlacementBackgroundItem()
  {
  }
  void setBackground(
      QPixmap         *pixmap,
      PlacementType   _relativeType,
      PlacementType   _parentRelativeType,
      PlacementMeta   &placement,
      BackgroundMeta  &background,
      BorderMeta      &border,
      MarginsMeta     &margin,
      StringListMeta  &subModelColor,
      int              submodelLevel,
      QString         &toolTip);

protected: 
  virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
  virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
  virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

private:
};

#endif

