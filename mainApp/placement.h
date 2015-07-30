 
/****************************************************************************
**
** Copyright (C) 2007-2009 Kevin Clague. All rights reserved.
** Copyright (C) 2015 Trevor SANDY. All rights reserved.
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
 * This class implements a fundamental class for placing things relative to
 * other things.  This concept is the cornerstone of LPub's meta commands
 * for describing what building instructions should look like without having
 * to specify inches, centimeters or pixels.
 *
 * Please see lpub.h for an overall description of how the files in LPub
 * make up the LPub program.
 *
 ***************************************************************************/

#ifndef placementH
#define placementH


#include <QGraphicsPixmapItem>
#include <QGraphicsRectItem>
#include <QSize>
#include <QRect>

#include "meta.h"
#include "metaitem.h"

class QPixmap;
class QGraphicsScene;
class Step;
class Steps;

//---------------------------------------------------------------------------
/*
 * Think of the possible placement as a two dimensional table, of
 * places where something can be placed within a rectangle.
 * -- see step.cpp for detail walkthrough --
 *
 *  CCCCCCCCCCCCCCC
 *  CRRRRRRRRRRRRRC
 *  CRCCCCCCCCCCCRC
 *  CRCSSSSSSSSSCRC
 *  CRCSCCCCCCCSCRC
 *  CRCSCPPPPPCSCRC
 *  CRCSCPCCCPCSCRC
 *  CRCSCPCACPCSCRC
 *  CRCSCPCCCPCSCRC
 *  CRCSCPPPPPCSCRC
 *  CRCSCCCCCCCSCRC
 *  CRCSSSSSSSSSCRC
 *  CRCCCCCCCCCCCRC
 *  CRRRRRRRRRRRRRC
 *  CCCCCCCCCCCCCCC
 *
 *  The table above represents either the Horizontal slice
 *  going through the CSI (represented by A for assembly),
 *  or the Vertical slice going through the CSI.
 *
 *  C0 - callout relative to rotateIcon
 *  R0 - rotateIcon relateive to csi
 *  C1 - callout relative to step number
 *  S0 - step number relative to csi
 *  C2 - callout relative to PLI
 *  P0 - pli relative to csi
 *  C3 - callout relative to csi
 *  A  - csi
 *  C4 - callout relative to csi
 *  P1 - pli relative to csi
 *  C5 - callout relative to PLI
 *  S1 - step number relative to csi
 *  C6 - callout relative to step number
 *  R1 - rotateIcon relateive to csi
 *  C7 - callout relative to rotateIcon
 *
 * const int stepNumberPlace[NumPlacements][2] =
{
  { TblSn0, TblSn0 },  // TopLeft
  { TblCsi, TblSn0 },  // Top
  { TblSn1, TblSn0 },  // TopRight
  { TblSn1, TblCsi },  // Right
  { TblSn1, TblSn1 },  // BOT_RIGHT
  { TblCsi, TblSn1 },  // BOT
  { TblSn0, TblSn1 },  // BOT_LEFT
  { TblSn0, TblCsi },  // Left
  { TblCsi, TblCsi },
};
 */
//---------------------------------------------------------------------------

enum Boundary {
  StartOfSteps       = 1,
  StartOfRange       = 2,
  EndOfRange         = 4,
  EndOfSteps         = 8,
  StartAndEndOfSteps = 9,
  StartAndEndOfRange = 6,
  Middle             = 16
};

enum {
TblCo0 = 0,
TblRi0,
TblCo1,
TblSn0,
TblCo2,
TblPli0,
TblCo3,
TblCsi,
TblCo4,
TblPli1,
TblCo5,
TblSn1,
TblCo6,
TblRi1,
TblCo7,
NumPlaces
};

//enum {
//  TblCo0 = 0,
//  TblSn0,
//  TblCo1,
//  TblPli0,
//  TblCo2,
//  TblCsi,
//  TblCo3,
//  TblPli1,
//  TblCo4,
//  TblSn1,
//  TblCo5,
//  NumPlaces
//};

enum dim {
  XX = 0,
  YY = 1
};

class Placement {
  public:
    int           size[2];         // How big am I?
    int           loc[2];          // Where do I live within my group
    int           tbl[2];          // Where am I in my grid?
    int           boundingSize[2]; // Me and my neighbors
    int           boundingLoc[2];  // Where do I live within my group
    QList<PlacementData *>  pldList;

    void setSize(int _size[2])
    {
      size[XX] = boundingSize[XX] = _size[XX];
      size[YY] = boundingSize[YY] = _size[YY];
    }
    
    void setSize(int x, int y)
    {
      size[XX] = boundingSize[XX] = x;
      size[YY] = boundingSize[YY] = y;
    }
    
    void setBoundingSize()
    {
      boundingSize[XX] = size[XX];
      boundingSize[YY] = size[YY];
    }
      
    PlacementType relativeType;  // What am I?
    PlacementMeta placement;     // Where am I placed?
    MarginsMeta   margin;        // How much room do I need?
    Placement    *relativeToParent;
    qreal         relativeToLoc[2];
    qreal         relativeToSize[2];

    QList<Placement *> relativeToList; // things placed relative to me

    Placement()
    {
      size[0] = 0;
      size[1] = 0;
      loc[0]  = 0;
      loc[1]  = 0;
      tbl[0]  = 0;
      tbl[1]  = 0;
      relativeToLoc[0] = 0;
      relativeToLoc[1] = 0;
      relativeToSize[0] = 1;
      relativeToSize[1] = 1;
      relativeToParent = NULL;
      boundingSize[0] = 0;
      boundingSize[1] = 0;
      boundingLoc[0] = 0;
      boundingLoc[1] = 0;
    }
    
    void assign(Placement *from)
    {
      size[0] = from->size[0];
      size[1] = from->size[1];
      loc[0] = from->loc[0];
      loc[1] = from->loc[1];
      tbl[0] = from->tbl[0];
      tbl[1] = from->tbl[1];
      relativeType = from->relativeType;
      placement = from->placement;
      margin = from->margin;
      relativeToParent = from->relativeToParent;
      relativeToLoc[0] = from->relativeToLoc[0];
      relativeToLoc[1] = from->relativeToLoc[1];
      relativeToSize[0] = from->relativeToSize[0];
      relativeToSize[1] = from->relativeToSize[1];
      boundingSize[0] = from->boundingSize[0];
      boundingSize[1] = from->boundingSize[1];
      boundingLoc[0] = from->boundingLoc[0];
      boundingLoc[1] = from->boundingLoc[1];
    }

    virtual ~Placement()
    {
      relativeToList.empty();
    }

    void appendRelativeTo(Placement *element);

    int  relativeTo(
      Step      *step);

    int relativeToSg(
      Steps    *steps);

    void placeRelative(
      Placement *placement);

    void placeRelativeBounding(
      Placement *placement);

    void placeRelative(
      Placement *placement,
      int        margin[]);
      
    void placeRelative(
      Placement *them,
      int   them_size[2],
      int   lmargin[2]);

    void justifyX(
      int origin,
      int height);

    void justifyY(
      int origin,
      int height);
      
    void calcOffsets(
      PlacementData &placementData,
      float offsets[2],
      qreal topLeft[2],
      qreal size[2]);
};

class PlacementPixmap : public Placement {
  public:
    QPixmap   *pixmap;

    PlacementPixmap()
    {
    }
};

class PlacementNum : public Placement {
  public:
    QString str;
    QString font;
    QString color;
    int  number;

    PlacementNum()
    {
      number = 0;
    }
    void format(char *format)
    {
      str.sprintf(format,number);
    }
    void sizeit();
    void sizeit(QString fmt);
};

class PlacementHeader : public Placement,
                        public QGraphicsPixmapItem {
public:
    PlacementHeader()
    {
    }
};

class PlacementFooter : public Placement ,
                        public QGraphicsPixmapItem {
public:
    PlacementFooter()
    {
    }
};

class PlacementRotateIcon: public Placement {
public:
  UnitsMeta iconImageSize;
  float     borderThickness;
  PlacementRotateIcon()
  {
  }
  void setSize(
      UnitsMeta _size,
      float     _borderThickness = 0);
  void sizeit();
};

#endif
