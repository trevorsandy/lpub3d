
/****************************************************************************
**
** Copyright (C) 2007-2009 Kevin Clague. All rights reserved.
** Copyright (C) 2015 - 2018 Trevor SANDY. All rights reserved.
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
 * This class implements part list images.  It gathers and maintains a list
 * of part/colors that need to be displayed.  It provides mechanisms for
 * rendering the parts.  It provides methods for organizing the parts into
 * a reasonable looking box for display in your building instructions.
 *
 * Please see lpub.h for an overall description of how the files in LPub
 * make up the LPub program.
 *
 ***************************************************************************/

#ifndef pliH
#define pliH

#include <QGraphicsTextItem>
#include <QGraphicsPixmapItem>
#include <QGraphicsSceneMouseEvent>
#include <QStringList>
#include <QPair>
#include <QString>
#include <QList>
#include <QHash>
#include <QCache>
#include <QTextDocument>

#include "meta.h"
#include "placement.h"
#include "backgrounditem.h"
#include "where.h"
#include "name.h"
#include "resize.h"
#include "annotations.h"

#include "QsLog.h"

class Pli;

/****************************************************************************
 * Part List
 ***************************************************************************/

class InstanceTextItem;
class AnnotateTextItem;
class PGraphicsPixmapItem;
class PliBackgroundItem;

class PliPart {
  public:
    // where the file exist in an LDraw file
    QList<Where>         instances;
    QString              type;
    QString              color;
    QString              text;
    QString              element;
    NumberMeta           instanceMeta;
    AnnotationStyleMeta  styleMeta;
    MarginsMeta          csiMargin;
    InstanceTextItem    *instanceText;
    AnnotateTextItem    *annotateText;
    AnnotateTextItem    *annotateElement;
    PGraphicsPixmapItem *pixmap;

    QString              sortElement;
    QString              sortCategory;
    QString              sortColour;
    QString              sortSize;
    QString              nameKey;
    QString              imageName;

    int                  width;
    int                  height;

    int                  col;

    int                  pixmapWidth;
    int                  pixmapHeight;
    int                  textHeight;
    int                  elementHeight;
    int                  annotHeight;
    int                  annotWidth;

    int                  textMargin;
    int                  topMargin;
    int                  partTopMargin;
    int                  partBotMargin;

    QList<int>           leftEdge;
    QList<int>           rightEdge;

    // placement info
    bool                 placed;
    int                  left;
    int                  bot;

    PliPart()
    {
      placed          = false;
      instanceText    = nullptr;
      annotateText    = nullptr;
      annotateElement = nullptr;
      pixmap          = nullptr;
    }

    PliPart(QString _type, QString _color)
    {
      type            = _type;
      color           = _color;
      placed          = false;
      instanceText    = nullptr;
      annotateText    = nullptr;
      annotateElement = nullptr;
      pixmap          = nullptr;
    }

    float maxMargin();

    virtual ~PliPart();
};

#define INSTANCE_SEP ":"

class Step;
class Steps;
class Callout;

class Pli : public Placement {
  private:
    static QCache<QString, QString> orientation;

    QHash<QString, PliPart*> tempParts;          // temp list used to devide the BOM
    QHash<QString, PliPart*> parts;
    QList<QString>           sortedKeys;
    Annotations              annotations;        // this is an internal list of title and custom part annotations

    int pageSizeP(Meta *, int which);

  public:
    PlacementType      parentRelativeType;
    bool               perStep;
    PliBackgroundItem *background;
    bool               bom;
    bool               splitBom;
    PliMeta            pliMeta;
    Meta              *meta;
    Steps             *steps;   // topOfSteps()
                                // bottomOfSteps()
    Step              *step;    // topOfStep()
                                // bottomOfStep()
    int                widestPart;
    int                tallestPart;

    Pli(bool _bom = false);

    struct PartTypesNeeded
    {
        PartType partType;
        QString typeName;
    };

    struct ImageAttribues
    {
        QStringList imageKeys[NUM_PART_TYPES];
        QStringList imageNames[NUM_PART_TYPES];
        QStringList ldrNames[NUM_PART_TYPES];
        QString baseName[NUM_PART_TYPES];
        QString partColor[NUM_PART_TYPES];
    };

    ImageAttribues ia;
    QList<PartTypesNeeded> ptn;
    QString fadeColour;
    bool fadeSteps;
    bool highlightStep;
    bool displayIcons;
    bool isSubModel;

    QString imageName;
    QStringList ldrNames;

    ~Pli()
    {
      clear();
    }

    const Where &topOfStep();
    const Where &bottomOfStep();
    const Where &topOfSteps();
    const Where &bottomOfSteps();
    const Where &topOfCallout();
    const Where &bottomOfCallout();
    bool  autoRange(Where &top, Where &bottom);

    void setPos(float x, float y);
    void setFlag(QGraphicsItem::GraphicsItemFlag flag,bool value);

    static const QString titleDescription(QString &part);
    static QString partLine(QString &line, Where & /*here*/, Meta &/*meta*/);
    void setParts(
      QStringList &csiParts,
      Meta        &meta,
      bool         bom = false,
      bool       split = false);

    int tsize()
    {
      return parts.size();
    }

    void clear();


    int  sizePli(Meta *, PlacementType, bool perStep);
    int  sizePli(ConstrainData::PliConstrain, unsigned height);
    int  sortPli();
    int  partSize();
    int  partSizeLDViewSCall();                          //LDView performance improvement
    int  resizePli(Meta *, ConstrainData &constrainData);
    int  placePli(QList<QString> &, int,int,bool,bool,int&,int&,int&);
    void positionChildren(int height, qreal scaleX, qreal scaleY);
    int  addPli (int, QGraphicsItem *);

    void placeCols(QList<QString> &);
    bool initAnnotationString();
    void getAnnotation(QString &, QString &);
    void partClass(QString &, QString &);
    int  createPartImage(QString &, QString &, QString &, QPixmap*);
    int  createPartImagesLDViewSCall(QStringList &, bool);      //LDView performance improvement
    QString orient(QString &color, QString part);
    QStringList configurePLIPart(QString &,QString &,PartType,bool,bool);
    int createSubModelIcons();

    void operator= (Pli& from)
    {
      QString key;
      foreach(key,from.parts.keys()) {
        PliPart *part = from.parts[key];
        parts.insert(key,part);
      }
      placement = from.placement;
      margin    = from.margin;
      bom       = from.bom;
    }

    void getParts(QHash<QString, PliPart*> &_parts)
    {
      _parts = parts;
    }

    void getLeftEdge(QImage &, QList<int> &);
    void getRightEdge(QImage &, QList<int> &);
};

class PliBackgroundItem : public BackgroundItem, public AbstractResize, public Placement
{
public:
  Pli *pli;
  PlacementType  parentRelativeType;
  bool           bom;
  bool           positionChanged;
  QPointF        position;

  // resize
  Grabber       *grabber;
  QPointF        point;
  int            grabHeight;

  PliBackgroundItem(
    Pli           *_pli,
    int            width,
    int            height,
    PlacementType  _parentRelativeType,
    int            submodelLevel,
    QGraphicsItem *parent);

  void setPos(double x, double y)
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

  virtual void resize(QPointF);
  virtual void change();
  virtual QRectF currentRect();

private:
};

//-----------------------------------------
//-----------------------------------------
//-----------------------------------------

class PGraphicsPixmapItem : public QGraphicsPixmapItem,
                            public MetaItem  // ResizePixmapItem
{
public:
PGraphicsPixmapItem(
  Pli     *_pli,
  PliPart *_part,
  QPixmap &pixmap,
  PlacementType  _parentRelativeType,
  QString &type,
  QString &color)
{
  parentRelativeType = _parentRelativeType;
  pli = _pli;
  part = _part;
  setPixmap(pixmap);
  setToolTip(pliToolTip(type,color));
}
QString pliToolTip(QString type, QString Color);
PliPart *part;
Pli     *pli;
PlacementType  parentRelativeType;

protected:
void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
};

class PGraphicsTextItem : public QGraphicsTextItem, public MetaItem
{
public:
PGraphicsTextItem()
{
  pli = nullptr;
  part = nullptr;
}
PGraphicsTextItem(
  Pli     *_pli,
  PliPart *_part,
  QString &text,
  QString &fontString,
  QString &toolTip)
{
  setText(_pli,
          _part,
          text,
          fontString,
          toolTip);
}
void setText(
  Pli     *_pli,
  PliPart *_part,
  QString &text,
  QString &fontString,
  QString &toolTip)
{
  pli  = _pli;
  part = _part;
  setPlainText(text);
  QFont font;
  font.fromString(fontString);
  setFont(font);
  setToolTip(toolTip);
}

virtual void size(int &x, int &y)
{
  QSizeF size = document()->size();
  x = int(size.width());
  y = int(size.height());
}
PliPart      *part;
Pli          *pli;
PlacementType parentRelativeType;
bool          isElement;
};

class InstanceTextItem : public PGraphicsTextItem
{
public:
InstanceTextItem(
  Pli            *_pli,
  PliPart        *_part,
  QString        &text,
  QString        &fontString,
  QString        &colorString,
  PlacementType _parentRelativeType)
{
  parentRelativeType = _parentRelativeType;
  QString toolTip(tr("Times used - right-click to modify"));
  setText(_pli,_part,text,fontString,toolTip);
  QColor color(colorString);
  setDefaultTextColor(color);
}

protected:
void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
};

class AnnotateTextItem : public PGraphicsTextItem
{
public:
  QRectF               annotateRect;
  AnnotationStyleMeta *styleMeta;

  AnnotateTextItem(
    Pli           *_pli,
    PliPart       *_part,
    QString       &text,
    QString       &fontString,
    QString       &colorString,
    PlacementType _parentRelativeType,
    bool          _element = false);

  virtual ~AnnotateTextItem(){}

  void size(int &x, int &y);

protected:
  StringListMeta       subModelColor;
  int                  submodelLevel;

  void setBackground(QPainter *painter);
  void paint(QPainter *painter, const QStyleOptionGraphicsItem *o, QWidget *w);
  void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
};

extern QHash<int, QString>     annotationString;
extern QList<QString>          titleAnnotations;

// Cut from here.....

#endif
