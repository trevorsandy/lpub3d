
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
#include "render.h"

#include "QsLog.h"

class Pli;

/****************************************************************************
 * Part List
 ***************************************************************************/

class InstanceTextItem;
class AnnotateTextItem;
class PGraphicsPixmapItem;
class PliBackgroundItem;
class PartGroupItem;
class LGraphicsScene;

class PliPart {
  public:
    // where the file exist in an LDraw file
    QList<Where>         instances;
    QString              type;
    QString              color;
    QString              text;
    QString              element;
    PliPartGroupMeta     groupMeta;
    NumberMeta           instanceMeta;
    AnnotationStyleMeta  styleMeta;
    MarginsMeta          csiMargin;
    InstanceTextItem    *instanceText;
    AnnotateTextItem    *annotateText;
    AnnotateTextItem    *annotateElement;
    PGraphicsPixmapItem *pixmap;
    PartGroupItem       *pliPartGroup;

    QString              sortElement;
    QString              sortCategory;
    QString              sortColour;
    QString              sortSize;
    QString              nameKey;
    QString              imageName;
    QString              subOriginalType;

    int                  width;
    int                  height;

    int                  col;

    int                  subType;

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
      subType         = 0;
      placed          = false;
      instanceText    = nullptr;
      annotateText    = nullptr;
      annotateElement = nullptr;
      pixmap          = nullptr;
      pliPartGroup    = nullptr;
    }

    PliPart(QString _type, QString _color)
    {
      type            = _type;
      color           = _color;
      subType         = 0;
      placed          = false;
      instanceText    = nullptr;
      annotateText    = nullptr;
      annotateElement = nullptr;
      pixmap          = nullptr;
      pliPartGroup    = nullptr;
    }

    void addPartGroupToScene(LGraphicsScene *scene,
                             Where &top,
                             Where &bottom,
                             int stepNumber);

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

    QString            viewerPliPartKey;
    QHash<QString,    ViewerOptions *> viewerOptsList;
    ViewerOptions     *viewerOptions;

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
        int sub[NUM_PART_TYPES];
    };

    ImageAttribues ia;
    QList<PartTypesNeeded> ptn;
    QString fadeColour;
    bool fadeSteps;
    bool highlightStep;
    bool displayIcons;
    bool isSubModel;
    bool multistep;
    bool callout;

    Where top,bottom;

    QString imageName;
    QStringList ldrNames;

    ~Pli()
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

    static const QString titleDescription(const QString &part);
    static QString partLine(QString &line, Where & /*here*/, Meta &/*meta*/);
    void setParts(
      QStringList             &csiParts,
      QList<PliPartGroupMeta> &partGroups,
      Meta                    &meta,
      bool                    bom = false,
      bool                    split = false);

    int tsize()
    {
      return parts.size();
    }

    void sortParts(QHash<QString, PliPart*> &_parts, bool setSplit = false);
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
    int  createPartImage(QString &, QString &, QString &, QPixmap*,int = 0);
    int  createPartImagesLDViewSCall(QStringList &, bool, int);      //LDView performance improvement
    QString orient(QString &color, QString part);
    QStringList configurePLIPart(int, QString &, QStringList &, int);
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
            QString &color);
    QString pliToolTip(QString type, QString Color,bool isSub = false);
    PliPart *part;
    Pli     *pli;
    PlacementType  parentRelativeType;
protected:
    virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent* event);
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent* event);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    bool isHovered;
    bool mouseIsDown;
};

class PGraphicsTextItem : public QGraphicsTextItem, public MetaItem
{
public:
    PGraphicsTextItem(QGraphicsTextItem *_parent = nullptr)
        :QGraphicsTextItem(_parent)
    {
        pli = nullptr;
        part = nullptr;
    }
    PGraphicsTextItem(
            Pli     *_pli,
            PliPart *_part,
            QString &text,
            QString &fontString,
            QString &toolTip,
            QGraphicsTextItem *_parent = nullptr)
        : QGraphicsTextItem(_parent)
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
  Pli                *_pli,
  PliPart            *_part,
  QString            &text,
  QString            &fontString,
  QString            &colorString,
  PlacementType      _parentRelativeType,
  PGraphicsTextItem *_parent = nullptr);

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

class AnnotateTextItem : public PGraphicsTextItem
{
public:
  BorderMeta     border;
  BackgroundMeta background;
  IntMeta        style;
  FontMeta       font;
  StringMeta     color;
  MarginsMeta    margin;
  QRectF         textRect;
  QRectF         styleRect;

  AnnotateTextItem(
    Pli           *_pli,
    PliPart       *_part,
    QString       &text,
    QString       &fontString,
    QString       &colorString,
    PlacementType _parentRelativeType,
    bool          _element = false,
    PGraphicsTextItem *_parent = nullptr);

  virtual ~AnnotateTextItem(){}

  void size(int &x, int &y);

  void scaleDownFont();

  void setAlignment( Qt::Alignment flags )
  {
      alignment = flags;
  }

protected:
  QPointF              textOffset;
  Qt::Alignment	       alignment;
  StringListMeta       subModelColor;
  int                  submodelLevel;

  void setAnnotationStyle(QPainter *painter);

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

class PartGroupItem : public QGraphicsItemGroup, public MetaItem
{
public:
    PartGroupItem(){}
    PartGroupItem(PliPartGroupMeta meta);
    ~PartGroupItem(){}

    PliPartGroupMeta meta;
    QPointF      position;
    bool         positionChanged;
    int          stepNumber;
    Where        top, bottom;

protected:
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event)
    {
        position = pos();
        positionChanged = false;
        QGraphicsItem::mousePressEvent(event);
    }
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event)
    {
        positionChanged = true;
        QGraphicsItem::mouseMoveEvent(event);
    }
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
    {
        QGraphicsItem::mouseReleaseEvent(event);

        if (event->button() == Qt::LeftButton){
            if (isSelected() && (flags() & QGraphicsItem::ItemIsMovable)) {
                change();
            }
        }
    }
    virtual void change()
    {
        if (isSelected() && (flags() & QGraphicsItem::ItemIsMovable)) {
            if (positionChanged) {

                QPointF newPosition = pos() - position;

                meta.setOffset(newPosition);

                beginMacro(QString("MoveItemGroup"));

                setPliPartGroupOffset(&meta);

                endMacro();
            }
        }
    }
};

extern QHash<int, QString>     annotationString;
extern QList<QString>          titleAnnotations;

// Cut from here.....

#endif
