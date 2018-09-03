/****************************************************************************
**
** Copyright (C) 2016 - 2018 Trevor SANDY. All rights reserved.
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

#ifndef LGRAPHICSSCENE_H
#define LGRAPHICSSCENE_H

#include <QGraphicsScene>
#include "name.h"

class LGraphicsScene : public QGraphicsScene
{
    Q_OBJECT
    Q_ENUMS(ObjItem)

public:
  enum ObjItem { NoGItem = -1, GPixmapItem, GTextItem, GRectItem, GLineItem, GPolyItem, GBaseItem, GNumItems };
  static const QString GItemNames[GNumItems];

  LGraphicsScene(QObject *parent = 0);

public slots:
  void setPageGuides(bool b){
    mPageGuides = b;
  }
  void setGuidePen(Theme t){
    t == ThemeDark ? guidePen.setColor(THEME_GUIDEL_PEN_DARK) :
                     guidePen.setColor(THEME_GUIDE_PEN_DEFAULT);
    guidePen.setWidth(2);
  }

protected:
  virtual void drawForeground(QPainter* painter, const QRectF& rect);
  virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
  virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
  virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
  void updateGuidePos(const QPointF &);
  bool setObjCornerPos(const QPointF &);
  ObjItem setObjItem(const QPointF &);

private:
  // guides
  QPen guidePen;
  bool mPageGuides;
  bool mObjCornerSet;
  QGraphicsItem        *mQGItem;       // 0
  QGraphicsPixmapItem  *mQGPixmapItem; // 1
  QGraphicsTextItem    *mQGTextItem;   // 2
  QGraphicsRectItem    *mQGRectItem;   // 3
  QGraphicsLineItem    *mQGLineItem;   // 4
  QGraphicsPolygonItem *mQGPolyItem;   // 5
  QPointF mGuidePos;
  qreal mAdjustX;       // Adjustment Point X
  qreal mAdjustY;       // Adjustment Point Y
};

#endif // LGRAPHICSSCENE_H
