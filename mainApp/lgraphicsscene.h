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
  void setPageGuides(bool b){
    mPageGuides = b;
  }
  void setGuidePen(Theme t){
    t == ThemeDark ? guidePen.setColor(THEME_GUIDE_PEN_DARK) :
                     guidePen.setColor(THEME_GUIDE_PEN_DEFAULT);
    guidePen.setWidth(2);
  }

protected:
  virtual void drawForeground(QPainter* painter, const QRectF& rect);
  virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
  virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
  virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
  void updateGuidePos();
  bool setSelectedItemZValue();
  bool setSelectedItem(const QPointF &);

private:
  // guides
  QPen guidePen;
  bool mValidItem;
  bool mPageGuides;
  QPointF mGuidePos;
  int mItemType;
  QGraphicsItem *mBaseItem;
  bool mIsItemOnTop;
  bool mIsItemOnBottom;
  bool mShowContextAction;
  qreal minZ,maxZ;
};

extern QHash<SceneObject, QString> soMap;

#endif // LGRAPHICSSCENE_H
