 
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
#include <QGradient>
#include "lpub_preferences.h"

enum AllocEnc {
  Horizontal = 0,
  Vertical
};

enum OrientationEnc {
  Portrait = 0,
  Landscape
};

enum RectPlacement{

    /**************************************************************************************************************************
     *  TopLeftOutsideCorner    * TopLeftOutside         * TopOutside    * TopRightOutSide         * TopRightOutsideCorner    *
     **************************** *********************************************************************************************
     *  LeftTopOutside          * TopLeftInsideCorner    * TopInside     * TopRightInsideCorner    * RightTopOutside          *
     **************************************************************************************************************************
     *  LeftOutside             * LeftInside             * CenterCenter  * RightInside             * RightOutside             *
     **************************************************************************************************************************
     *  LeftBottomOutside       * BottomLeftInsideCorner * BottomInside  * BottomRightInsideCorner * RightBottomOutside       *
     **************************************************************************************************************************
     *  BottomLeftOutsideCorner * BottomLeftOutside      * BottomOutside * BottomRightOutside      * BottomRightOutsideCorner *
     **************************************************************************************************************************/

    TopLeftOutsideCorner,       //00
    TopLeftOutside,             //01
    TopOutside,                 //02
    TopRightOutSide,            //03
    TopRightOutsideCorner,      //04

    LeftTopOutside,             //05
    TopLeftInsideCorner,        //06 "Page Top Left"
    TopInside,                  //07 "Page Top"
    TopRightInsideCorner,       //08 "Page Top Right"
    RightTopOutside,            //09

    LeftOutside,                //10
    LeftInside,                 //11 "Page Left"
    CenterCenter,               //12 "Page Center"
    RightInside,                //13 "Page Right"
    RightOutside,               //14

    LeftBottomOutside,          //15
    BottomLeftInsideCorner,     //16 "Page Bottom Left"
    BottomInside,               //17 "Page Bottom"
    BottomRightInsideCorner,    //18 "Page Bottom Right"
    RightBottomOutside,         //19

    BottomLeftOutsideCorner,    //20
    BottomLeftOutside,          //21
    BottomOutside,              //22
    BottomRightOutside,         //23
    BottomRightOutsideCorner,   //24

    NumSpots                    //25
};

enum PlacementEnc {
  TopLeft,                      //00
  Top,                          //01
  TopRight,                     //02
  Right,                        //03
  BottomRight,                  //04
  Bottom,                       //05
  BottomLeft,                   //06
  Left,                         //07
  Center,                       //08
  NumPlacements
};

enum PrepositionEnc {
  Inside = 0,
  Outside
};

enum PlacementType {          //  placement dialog codes:
  PageType,                   // 0 page
  CsiType,                    // 1 Csi  (Assem)
  StepGroupType,              // 2 Ms   (Multi-Step)
  StepNumberType,             // 3 Sn
  PartsListType,              // 4 Pli
  CalloutType,                // 5 Callout
  PageNumberType,             // 6 pn

  PageTitleType,              // 7 tt
  PageModelNameType,          // 8 mnt
  PageAuthorType,             // 9 at
  PageURLType,                //10 urlt
  PageModelDescType,          //11 mdt
  PagePublishDescType,        //12 pdt
  PageCopyrightType,          //13 ct
  PageEmailType,              //14 et
  PageDisclaimerType,         //15 dt
  PagePiecesType,             //16 pt
  PagePlugType,               //17 plt
  PageCategoryType,           //18 cat
  PageDocumentLogoType,       //19 dlt
  PageCoverImageType,         //20 cit
  PagePlugImageType,          //21 pit
  PageHeaderType,             //22 ph
  PageFooterType,             //23 pf
  RotateIconType,             //24

  SingleStepType,             //25
  SubmodelInstanceCountType,  //26

  StepType,                   //27
  RangeType,                  //28
  ReserveType,                //29
  BomType,                    //31
  CoverPageType,              //31
  NumRelatives                //32
};

enum pageType{
    ContentPage = 0,
    FrontCoverPage,
    BackCoverPage
};

enum annotationType{
  TitleAnnotationType = 0,
  FreeFormAnnotationType,
  TitleAndFreeFormAnnotationType,
  AnnotationType
};

enum sortType{
    PartSize = 0,
    PartColour,
    PartCategory,
    SortByType
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
    InsertBom,
    InsertCoverPage,
    InsertFinalModel,
    InsertRotateIcon
  } type;

  QString     picName;
  qreal       picScale;
  QString     text;
  QString     textFont;
  QString     textColor;
  QPointF     arrowHead;
  QPointF     arrowTail;
  qreal       haftingDepth;
  QPointF     haftingTip;
  float       offsets[2];
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
    BgColor,
    BgGradient,
    BgImage,
    BgSubmodelColor
  } type;

  enum GradientMode   {LogicalMode    = 0,StretchToDeviceMode,ObjectBoundingMode}         gmode;
  enum GradientSpread {PadSpread      = 0,RepeatSpread,       ReflectSpread}              gspread;
  enum GradientType   {LinearGradient = 0,RadialGradient,     ConicalGradient,NoGradient} gtype;
  QVector<QPair<qreal,QColor> > gstops;
  QVector<QPointF> gpoints;
  qreal   gsize[2];                           // in pixels
  qreal   gangle;
  QString string;
  bool    stretch;

  BackgroundData()
  {
    type = BgTransparent;
    stretch = false;
    string = "";
    //gradient initialization
    gmode    = LogicalMode;
    gspread  = RepeatSpread;
    gtype    = LinearGradient;
    gsize[0] = 0;
    gsize[1] = 0;
    gangle   = 0;
    gpoints.clear();
    gstops.append(qMakePair(0.00, QColor::fromRgba(0)));
    gstops.append(qMakePair(0.04, QColor::fromRgba(0xff131360)));
    gstops.append(qMakePair(0.08, QColor::fromRgba(0xff202ccc)));
    gstops.append(qMakePair(0.42, QColor::fromRgba(0xff93d3f9)));
    gstops.append(qMakePair(0.51, QColor::fromRgba(0xffb3e6ff)));
    gstops.append(qMakePair(0.73, QColor::fromRgba(0xffffffec)));
    gstops.append(qMakePair(0.92, QColor::fromRgba(0xff5353d9)));
    gstops.append(qMakePair(0.96, QColor::fromRgba(0xff262666)));
    gstops.append(qMakePair(1.00, QColor::fromRgba(0)));
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
  enum Line {
    BdrLnNone = 0,
    BdrLnSolid,
    BdrLnDash,
    BdrLnDot,
    BdrLnDashDot,
    BdrLnDashDotDot,
  };
  QString color;
  float   thickness;  // in units 
  float   radius;     // in units
  float   margin[2];  // in units
  int     line;
  
  BorderData()
  {
    thickness = 0.125;
    margin[0] = 0;
    margin[1] = 0;
    radius = 15;
    type = BdrNone;
    line = BdrLnNone;
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

const QString SortOptionName[SortByType] =
{
    "Part Size",
    "Part Color",
    "Part Category"
};

// testing and diagnostics only
const QString RectNames[NumSpots] =
 {                                //  placement dialog codes:
    "TopLeftOutsideCorner",       //00
    "TopLeftOutside",             //01
    "TopOutside",                 //02
    "TopRightOutSide",            //03
    "TopRightOutsideCorner",      //04

    "LeftTopOutside",             //05
    "TopLeftInsideCorner",        //06 "Page Top Left"
    "TopInside",                  //07 "Page Top"
    "TopRightInsideCorner",       //08 "Page Top Right"
    "RightTopOutside",            //09

    "LeftOutside",                //10
    "LeftInside",                 //11 "Page Left"
    "CenterCenter",               //12 "Page Center"
    "RightInside",                //13 "Page Right"
    "RightOutside",               //14

    "LeftBottomOutside",          //15
    "BottomLeftInsideCorner",     //16 "Page Bottom Left"
    "BottomInside",               //17 "Page Bottom"
    "BottomRightInsideCorner",    //18 "Page Bottom Right"
    "RightBottomOutside",         //19

    "BottomLeftOutsideCorner",    //20
    "BottomLeftOutside",          //21
    "BottomOutside",              //22
    "BottomRightOutside",         //23
    "BottomRightOutsideCorner",   //24
};	//NumSpots

const QString RelNames[NumRelatives] =
{                              	 //  placement dialog codes:
   "PageType",                 	 // 0 page
   "CsiType",                    // 1 Csi  (Assem)
   "StepGroupType",              // 2 Ms   (Multi-Step)
   "StepNumberType",             // 3 Sn
   "PartsListType",              // 4 Pli
   "CalloutType",                // 5 Callout
   "PageNumberType",             // 6 pn

   "PageTitleType",		 // 7 tt
   "PageModelNameType",		 // 8 mnt
   "PageAuthorType",             // 9 at
   "PageURLType",		 //10 urlt
   "PageModelDescType",		 //11 mdt
   "PagePublishDescType",	 //12 pdt
   "PageCopyrightType",		 //13 ct
   "PageEmailType",		 //14 et
   "PageDisclaimerType",	 //15 dt
   "PagePiecesType",		 //16 pt
   "PagePlugType",		 //17 plt
   "PageCategoryType",		 //18 cat
   "PageDocumentLogoType",	 //19 dlt
   "PageCoverImageType",	 //20 cit
   "PagePlugImageType",		 //21 pit
   "PageHeaderType",             //22 ph
   "PageFooterType",             //23 pf
   "RotateIconType",             //24

   "SingleStepType",             //25
   "SubmodelInstanceCountType",  //26

   "StepType",                   //27
   "RangeType",                  //28
   "ReserveType",                //29
   "BomType",                    //30
   "CoverPageType"               //31

}; //NumRelatives"               //32

const QString PlacNames[NumPlacements] =
 {
  "TopLeft",                      //00
  "Top",                          //01
  "TopRight",                     //02
  "Right",                        //03
  "BottomRight",                  //04
  "Bottom",                       //05
  "BottomLeft",                   //06
  "Left",                         //07
  "Center"                        //08
}; //NumPlacements

const QString PrepNames[2] =
{
  "Inside",
  "Outside"
};

#endif
