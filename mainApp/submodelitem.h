/****************************************************************************
**
** Copyright (C) 2018 - 2019 Trevor SANDY. All rights reserved.
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
 * This class provides a simple mechanism for displaying a Submodel image
 * on the page.
 *
 ***************************************************************************/

#ifndef SUBMODELITEM_H
#define SUBMODELITEM_H

#include <QGraphicsTextItem>
#include <QGraphicsPixmapItem>
#include <QGraphicsSceneMouseEvent>
#include <QStringList>
#include <QPair>
#include <QString>
#include <QList>
#include <QHash>
#include <QTextDocument>

#include "meta.h"
#include "placement.h"
#include "backgrounditem.h"
#include "where.h"
#include "name.h"
#include "resize.h"
#include "render.h"

/****************************************************************************
 * Submodel Part
 ***************************************************************************/

class SubModel;                // Pli
class SubModelPart;            // PliPart
class SMGraphicsPixmapItem;    // PGraphicsPixmapItem
class SMInstanceTextItem;      // InstanceTextItem
class SubModelBackgroundItem;  // PliBackgroundItem
class ViewerOptions;

class SubModelPart {
  public:
    // where the exist in an LDraw file
    QString               type;
    QString               color;
    NumberMeta            instanceMeta;
    MarginsMeta           csiMargin;
    SMInstanceTextItem   *instanceText;
    SMGraphicsPixmapItem *pixmap;

    int                   width;
    int                   height;

    int                   col;

    int                   pixmapWidth;
    int                   pixmapHeight;
    int                   textHeight;

    int                   topMargin;
    int                   partTopMargin;
    int                   partBotMargin;

    QList<int>            leftEdge;
    QList<int>            rightEdge;

    // placement info
    bool                  placed;
    int                   left;
    int                   bot;

    SubModelPart()
    {
      placed       = false;
      instanceText = nullptr;
      pixmap       = nullptr;
    }

    SubModelPart(QString _type, QString _color)
    {
      type         = _type;
      color        = _color;
      placed       = false;
      instanceText = nullptr;
      pixmap       = nullptr;
    }

    float maxMargin();

    virtual ~SubModelPart();
};

/****************************************************************************
 * Sub Model
 ***************************************************************************/

class Step;
class Steps;
class Callout;

class SubModel : public Placement {
  private:
    QHash<QString, SubModelPart*> parts;
    QList<QString>           sortedKeys;

    int pageSizeP(Meta *, int which);

  public:
    PlacementType          parentRelativeType;
    bool                   perStep;
    SubModelBackgroundItem *background;
    SubModelMeta            subModelMeta;
    QString                 imageName;
    QString                 viewerStepKey;
    ViewerOptions          *viewerOptions;

    Meta                   *meta;
    Steps                  *steps;      // topOfSteps()
                                        // bottomOfSteps()
    Step                   *step;       // topOfStep()
                                        // bottomOfStep()
    int                     widestPart;
    int                     tallestPart;
    bool                    shared;
    // validity
    bool                    imageOutOfDate;
    bool                    displayInstanceCount;

    Where                   top,bottom;
    bool                    multistep,callout;

    SubModel();

    ~SubModel()
    {
      clear();
    }

    bool loadTheViewer();

    const Where &topOfStep();
    const Where &bottomOfStep();
    const Where &topOfSteps();
    const Where &bottomOfSteps();
    const Where &topOfCallout();
    const Where &bottomOfCallout();

    bool  autoRange(Where &top, Where &bottom);

    void setPos(float x, float y);
    void setFlag(QGraphicsItem::GraphicsItemFlag flag,bool value);
    bool rotateModel(QString ldrName, QString subModel, const QString color, bool noCA);

    void setSubModel(QString &modelName, Meta &meta);

    int tsize() // always 1
    {
      return parts.size();
    }

    void clear();

    int  sizeSubModel(Meta *, PlacementType, bool _perStep);
    int  sizeSubModel(ConstrainData::PliConstrain, unsigned height);
    int  resizeSubModel(Meta *, ConstrainData &constrainData);
    int  generateSubModelItem();

    int  placeSubModel(QList<QString> &,int,int,int&,int&,int&);
    void positionChildren(int height, qreal scaleX, qreal scaleY);
    int  addSubModel (int, QGraphicsItem *);
    void placeCols(QList<QString> &);
    int  createSubModelImage(QString &, QString &, QString &, QPixmap*);

    void operator= (SubModel& from)
    {
      QString key;
      foreach(key,from.parts.keys()) {
        SubModelPart *part = from.parts[key];
        parts.insert(key,part);
      }
      placement = from.placement;
      margin    = from.margin;
    }

    void getLeftEdge(QImage &, QList<int> &);
    void getRightEdge(QImage &, QList<int> &);
};

/****************************************************************************
 * Sub Model BackgroundItem
 ***************************************************************************/

class SubModelBackgroundItem : public BackgroundItem, public AbstractResize, public Placement
{
public:
  SubModel       *subModel;
  PlacementType  parentRelativeType;
  bool           positionChanged;
  QPointF        position;

  // resize
  Grabber       *grabber;
  QPointF        point;
  int            grabHeight;

  SubModelBackgroundItem(
    SubModel      *_subModel,
    int             width,
    int             height,
    PlacementType  _parentRelativeType,
    int             submodelLevel,
    QGraphicsItem  *parent);

  void setPos(float x, float y)
  {
    QGraphicsPixmapItem::setPos(x,y);
  }
  void setFlag(GraphicsItemFlag flag, bool value)
  {
    QGraphicsItem::setFlag(flag,value);
  }

  virtual void placeGrabbers();
  virtual QGraphicsItem *myParentItem()
  {
    return parentItem();
  }

protected:
  virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
  virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
  virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
  void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);

  //-----------------------------------------

  virtual void resize(QPointF);
  virtual void change();
  virtual QRectF currentRect();

};

/****************************************************************************
 * SMGraphicsTextItem
 ***************************************************************************/

class SMGraphicsTextItem : public QGraphicsTextItem, public MetaItem
{
public:
  SMGraphicsTextItem()
  {
    subModel = nullptr;
    part = nullptr;
  }
  SMGraphicsTextItem(
    SubModel     *_subModel,
    SubModelPart *_part,
    QString       &text,
    QString       &fontString,
    QString       &toolTip)
  {
    setText(_subModel,
            _part,
            text,
            fontString,
            toolTip);
  }
  void setText(
    SubModel     *_subModel,
    SubModelPart *_part,
    QString       &text,
    QString       &fontString,
    QString       &toolTip)
  {
    subModel  = _subModel;
    part = _part;
    setPlainText(text);
    QFont font;
    font.fromString(fontString);
    setFont(font);
    setToolTip(toolTip);
  }
  void size(int &x, int &y)
  {
    QSizeF size = document()->size();
    x = int(size.width());
    y = int(size.height());
  }
  SubModelPart  *part;
  SubModel      *subModel;
  PlacementType  parentRelativeType;
};

/****************************************************************************
 * SMInstanceTextItem
 ***************************************************************************/

class SMInstanceTextItem : public SMGraphicsTextItem
{
public:
  SMInstanceTextItem(
    SubModel       *_subModel,
    SubModelPart   *_part,
    QString        &text,
    QString        &fontString,
    QString        &colorString,
    PlacementType _parentRelativeType);

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

/****************************************************************************
 * SMGraphicsPixmapItem
 ***************************************************************************/
class SMGraphicsPixmapItem : public QGraphicsPixmapItem,
                            public MetaItem  // ResizePixmapItem
{
public:
  SMGraphicsPixmapItem(
    SubModel      *_subModel,
    SubModelPart  *_part,
    QPixmap        &pixmap,
    PlacementType  _parentRelativeType,
    QString        &type,
    QString        &color) :
      isHovered(false),
      mouseIsDown(false)
  {
    Q_UNUSED(color);
    parentRelativeType = _parentRelativeType;
    subModel           = _subModel;
    part               = _part;
    setPixmap(pixmap);
    setFlag(QGraphicsItem::ItemIsSelectable,true);
    setFlag(QGraphicsItem::ItemIsFocusable, true);
    setAcceptHoverEvents(true);
    setToolTip(subModelToolTip(type));
  }
  QString subModelToolTip(QString type);
  SubModelPart *part;
  SubModel     *subModel;
  PlacementType parentRelativeType;

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

#endif // SUBMODELITEM_H
