/****************************************************************************
**
** Copyright (C) 2016 - 2019 Trevor SANDY. All rights reserved.
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
  LGraphicsScene(QObject *parent = nullptr);
  bool showContextAction();
  bool isSelectedItemOnTop();
  bool isSelectedItemOnBottom();
  qreal getSelectedItemZValue();

public slots:
  void bringSelectedItemToFront();
  void sendSelectedItemToBack();
  void setGuidePen(QString color){
    guidePen = QPen(QBrush(QColor(color)), 2, Qt::DashLine);
  }
  void setGridPen(QString color){
    gridPen =  QPen(QBrush(QColor(color)), 2, Qt::SolidLine);
  }
  void setSceneGuides(bool b){
    mSceneGuides = b;
  }
  void setSnapToGrid(bool b){
    mSnapToGrid = b;
  }
  void setGridSize(int i){
    mGridSize = i;
  }

protected:
  virtual void drawForeground(QPainter* painter, const QRectF& rect);
  virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
  virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
  virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
  virtual void drawBackground(QPainter *painter, const QRectF &rect);
  void snapToGrid();
  void updateGuidePos();
  bool setSelectedItemZValue();
  bool setSelectedItem(const QPointF &);

private:
  // guides
  QPen guidePen;
  QPen gridPen;
  bool mValidItem;
  bool mSceneGuides;
  QPointF mGuidePos;
  int mItemType;
  QGraphicsItem *mBaseItem;
  bool mIsItemOnTop;
  bool mIsItemOnBottom;
  bool mShowContextAction;
  bool mSnapToGrid;
  int mGridSize;
  qreal minZ,maxZ;

};

extern QHash<SceneObject, QString> soMap;

#endif // LGRAPHICSSCENE_H
