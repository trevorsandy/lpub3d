 
/****************************************************************************
**
** Copyright (C) 2007-2009 Kevin Clague. All rights reserved.
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
 * This file describes some of the compound data types managed by meta
 * classes such as background, border, number, etc.
 *
 * Please see lpub.h for an overall description of how the files in LPub
 * make up the LPub program.
 *
 ***************************************************************************/

#ifndef METATYPES_H
#define METATYPES_H

#include <QString>
#include <QStringList>
#include <QPointF>

enum AllocEnc {
  Horizontal = 0,
  Vertical
};

enum RectPlacement{

  TopLeftOutsideCorner, 
  TopLeftOutside, 
  TopOutside, 
  TopRightOutSide, 
  TopRightOutsideCorner,

  LeftTopOutside,
  TopLeftInsideCorner, 
  TopInside,
  TopRightInsideCorner,
  RightTopOutside,

  LeftOutside,
  LeftInside,
  CenterCenter,
  RightInside,
  RightOutside,

  LeftBottomOutside,
  BottomLeftInsideCorner,
  BottomInside,
  BottomRightInsideCorner,
  RightBottomOutside,

  BottomLeftOutsideCorner,
  BottomLeftOutside,
  BottomOutside,
  BottomRightOutside,
  BottomRightOutsideCorner,

  NumSpots
};

enum PlacementEnc {
  TopLeft,
  Top,
  TopRight,
  Right,
  BottomRight,
  Bottom,
  BottomLeft,
  Left,
  Center,
  NumPlacements
};

enum PrepositionEnc {
  Inside = 0,
  Outside
};

enum PlacementType {
  PageType,
  CsiType,
  StepGroupType,
  StepNumberType,
  PartsListType,
  CalloutType,
  PageNumberType,
  SingleStepType,
  SubmodelInstanceCountType,
  StepType,
  RangeType,
  ReserveType,
  BomType,
  CoverPageType,
  NumRelatives,
  PageAttributeType,

  PageTitleType,
  PageModelNumType,
  PageAuthorType,
  PageAuthorURLType,
  PageLogoType,
  PageShortDescType,
  PageCopyrightType,
  PageEmailType,
  PageDisclaimerType,
  PagePiecesType,
  PagePlugType,
  PagePlugImageType,
  PageFrontCoverType,
  PageBackCoverType,
  PageAttributePictureType,
  PageAttributeTextType
};

class PlacementData
{
public:
  PlacementEnc   placement;
  PlacementEnc   justification;
  PlacementType  relativeTo;
  PrepositionEnc preposition;
  RectPlacement  rectPlacement;

  float offsets[2];
  PlacementData()
  {
    clear();
  }
  void clear()
  {
    rectPlacement = TopLeftOutsideCorner;
    offsets[0] = 0;
    offsets[1] = 0;
  }
};

class PointerData
{
public:
  PlacementEnc placement;
  float loc;  // fraction of side/top/bottom of callout
  float x;    // fraction of CSI size
  float y;
  float base; // in units
};

class RotStepData
{
public:
  double  rots[3];
  QString type;
  RotStepData()
  {
    type = "";
    rots[0] = 0;
    rots[1] = 0;
    rots[2] = 0;
  }
  RotStepData & operator=(const RotStepData &rhs)
  {
    type = rhs.type;
    rots[0] = rhs.rots[0];
    rots[1] = rhs.rots[1];
    rots[2] = rhs.rots[2];
    return *this;
  }
};

class BuffExchgData
{
public:
  QString buffer;
  QString type;
};

class InsertData
{
public:  
  enum InsertType
  {
    InsertPicture,
    InsertText,
    InsertArrow,
    InsertBom
  } type;

  QString picName;
  qreal   picScale;
  QString text;
  QString textFont;
  QString textColor;
  QPointF arrowHead;
  QPointF arrowTail;
  qreal   haftingDepth;
  QPointF haftingTip;
  float   offsets[2];
  InsertData()
  {
    picScale = 1.0;
    offsets[0] = 0.5;
    offsets[1] = 0.5;
    haftingDepth = 0;
    textFont  = "Arial,48,-1,255,75,0,0,0,0,0";
    textColor = "Black";
  }
};

class BackgroundData
{
public:
  enum Background {
    BgTransparent,
    BgImage,
    BgColor,
    BgSubmodelColor
  } type;
  QString    string;
  bool       stretch;
};

class PictureData
{
public:
  QString           picName;
  qreal             picScale;
  QString           string;
  bool              stretch;
  PictureData()
  {
      picScale = 1.0;
  }
};

class BorderData
{
public:
  enum Border {
    BdrNone = 0,
    BdrSquare,
    BdrRound
  } type;
  QString color;
  float   thickness;  // in units 
  float   radius;     // in units
  float   margin[2];  // in units
  
  BorderData()
  {
    thickness = 0.125;
    margin[0] = 0;
    margin[1] = 0;
    radius = 15;
    type = BdrNone;
    color = "Black";
  }
};

class SubData
{
public:
  QString color;
  QString part;
  int     type;
};

class FreeFormData
{
public:
  bool         mode;
  PlacementEnc base;
  PlacementEnc justification;
};

class ConstrainData
{
public:
  enum PliConstrain {
    PliConstrainArea,
    PliConstrainSquare,
    PliConstrainWidth,
    PliConstrainHeight,
    PliConstrainColumns
  } type;
  float        constraint;
};

class SepData
{
public:
  float   thickness;  // in units
  QString color;
  float   margin[2];  // in units
  SepData()
  {
    thickness = 0.125;
    margin[0] = 0;
    margin[1] = 0;
  }
};

  
#endif
