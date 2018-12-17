 
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
  Landscape,
  InvalidOrientation
};

enum RectPlacement{

    /**************************************************************************************************************************
     *  TopLeftOutsideCorner    * TopLeftOutside         * TopOutside    * TopRightOutSide         * TopRightOutsideCorner    *
     ***************************--------------------------------------------------------------------***************************
     *  LeftTopOutside          - TopLeftInsideCorner    - TopInside     - TopRightInsideCorner    - RightTopOutside          *
     ***************************--------------------------------------------------------------------***************************
     *  LeftOutside             - LeftInside             - CenterCenter  - RightInside             - RightOutside             *
     ***************************--------------------------------------------------------------------***************************
     *  LeftBottomOutside       - BottomLeftInsideCorner - BottomInside  - BottomRightInsideCorner - RightBottomOutside       *
     ***************************--------------------------------------------------------------------***************************
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

enum PlacementType {          // placement dialogue codes:
  PageType,                   // 0 Page
  CsiType,                    // 1 Csi  (Assem)
  StepGroupType,              // 2 Ms   (Multi-Step)
  StepNumberType,             // 3 Sn
  PartsListType,              // 4 Pli
  CalloutType,                // 5 Callout
  PageNumberType,             // 6 Pn

  PageTitleType,              // 7 Tt
  PageModelNameType,          // 8 Mnt
  PageAuthorType,             // 9 At
  PageURLType,                //10 Urlt
  PageModelDescType,          //11 Mdt
  PagePublishDescType,        //12 Pdt
  PageCopyrightType,          //13 Ct
  PageEmailType,              //14 Et
  PageDisclaimerType,         //15 Dt
  PagePartsType,              //16 Pt
  PagePlugType,               //17 Plt
  SubmodelInstanceCountType,  //18 Sic
  PageDocumentLogoType,       //19 Dlt
  PageCoverImageType,         //20 Cit
  PagePlugImageType,          //21 Pit
  PageHeaderType,             //22 Ph
  PageFooterType,             //23 Pf
  PageCategoryType,           //24 Cat
  SubModelType,               //25 Sm
  RotateIconType,             //26 Ri
  BomType,                    //27

  PagePointerType,            //28
  SingleStepType,             //29
  StepType,                   //30
  RangeType,                  //31
  ReserveType,                //32
  CoverPageType,              //33

  NumRelatives                //34
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
  RectPlacement rectPlacement; //Base rect placement
  PlacementEnc  placement;     //Pointer placement relative to base
  float loc;                   // fraction of side/top/bottom of callout
  float base;                  // in units
  int segments;                // Number of segments
  float x1;                    // TipX
  float y1;                    // TipY
  float x2;                    // BaseX
  float y2;                    // BaseY
  float x3;                    // MidBaseX
  float y3;                    // MidBaseY
  float x4;                    // MidTipX
  float y4;                    // MidTipY
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

class PgSizeData
{
public:
  float   sizeW;
  float   sizeH;
  QString sizeID;     // e.g. A4
  OrientationEnc orientation;
  PgSizeData(){
    sizeW       = 0;
    sizeH       = 0;
    orientation = Portrait;
  }
};

// TODO confirm if this is still needed
class PageSizeData{
public:
//  OrientationEnc orientation; //future use
  float          pagesize[2][2];
  QString        sizeid;
  PageSizeData(){
//    orientation = Portrait;
    pagesize[0][0] = 0;
    pagesize[0][1] = 0;
  }
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
    InsertRotateIcon,
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
    type     = BgTransparent;
    stretch  = false;
    string   = "";
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
  Line    line;
  bool    hideArrows;
  bool    useDefault;
  
  BorderData()
  {
    thickness  = 0.125;
    margin[0]  = 0;
    margin[1]  = 0;
    radius     = 15;
    type       = BdrNone;
    line       = BdrLnNone;
    color      = "Black";
    hideArrows = false;
    useDefault = true;
  }
};

class PointerAttribData
{
   public:
    enum Attribute {
      Line,
      Border
    } attribType;
    int id;
    bool borderModified;
    QString parent;
    BorderData lineData;
    BorderData borderData;

    PointerAttribData()
    {
        lineData.color     = "Black";
        lineData.type      = BorderData::BdrRound;
        lineData.line      = BorderData::BdrLnSolid;
        lineData.thickness = 1.0f/32.0f;
        lineData.radius    = 15;
        lineData.margin[0] = 0;
        lineData.margin[1] = 0;
        lineData.useDefault=false;
        lineData.hideArrows=false;

        borderData.color     = QString();
        borderData.type      = BorderData::BdrRound;
        borderData.line      = BorderData::BdrLnSolid;
        borderData.thickness = 1.0f/64.0f;
        borderData.radius    = 15;
        borderData.margin[0] = 0;
        borderData.margin[1] = 0;
        borderData.useDefault=false;
        borderData.hideArrows=false;

        attribType           = PointerAttribData::Line;
        id                   = 0000;
        borderModified       = false;
        parent               = QString();
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
    thickness  = 0.125;
    margin[0]  = 0;
    margin[1]  = 0;
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
 {                                //  placement dialogue codes:
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
{                              	 //  placement dialogue codes:
   "PageType",                 	 // 0 page
   "CsiType",                    // 1 Csi  (Assem)
   "StepGroupType",              // 2 Ms   (Multi-Step)
   "StepNumberType",             // 3 Sn
   "PartsListType",              // 4 Pli
   "CalloutType",                // 5 Callout
   "PageNumberType",             // 6 pn

   "PageTitleType",              // 7 tt
   "PageModelNameType",          // 8 mnt
   "PageAuthorType",             // 9 at
   "PageURLType",                //10 urlt
   "PageModelDescType",          //11 mdt
   "PagePublishDescType",        //12 pdt
   "PageCopyrightType",          //13 ct
   "PageEmailType",              //14 et
   "PageDisclaimerType",         //15 dt
   "PagePartsType",              //16 pt
   "PagePlugType",               //17 plt
   "SubmodelInstanceCountType",  //18 sic
   "PageDocumentLogoType",       //19 dlt
   "PageCoverImageType",         //20 cit
   "PagePlugImageType",          //21 pit
   "PageHeaderType",             //22 ph
   "PageFooterType",             //23 pf
   "PageCategoryType",           //24 cat
   "SubModelType",               //25 Sm
   "RotateIconType",             //26 Ri
   "BomType",                    //27

   "PagePointerType",            //28
   "SingleStepType",             //29
   "StepType",                   //20
   "RangeType",                  //31
   "ReserveType",                //32
   "CoverPageType"               //33

}; //NumRelatives"               //34

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
