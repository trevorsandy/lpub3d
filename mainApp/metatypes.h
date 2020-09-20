 
/****************************************************************************
**
** Copyright (C) 2007-2009 Kevin Clague. All rights reserved.
** Copyright (C) 2015 - 2019 Trevor SANDY. All rights reserved.
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

enum FillEnc {
  Aspect = 0,
  Stretch,
  Tile
};

enum JustifyStepEnc {
    JustifyCenter,
    JustifyCenterHorizontal,
    JustifyCenterVertical,
    JustifyLeft
};

enum CountInstanceEnc {
    CountFalse = 0,
    CountTrue,
    CountAtModel,
    CountAtStep,
    CountAtTop,
    CountAtOptions
};

enum ContStepNumEnc {
    ContStepNumFalse,
    ContStepNumTrue
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

/*
* PlacementTypes are defined here and in
* PlacementDialog. Placement
* Names are defined at PlacementMeta in
* meta.cpp as relativeNames[] and below as RelNames[]
* PlacementMeta also defines _relativeTo
* PlacementDialog also defined RelativeTos
* Which correspond to each PlacementType
*
* -enum       PlacementType                (metatypes.h)
* -QString    RelNames[NumRelatives]       (metatypes.h)
* -QString    relativeNames[]              (meta.cpp)
* -QString    relativeTos                  (meta.cpp, PlacementMeta::parse())
* -enum       RelativeTos                  (placementdialog.h)
* -QList<int> relativeToOks[NumRelatives]  (placementdialog.cpp)
* -int        prepositionOks[NumRelatives] (placementdialog.cpp)
* -QString    relativeNames[NumRelatives]  (placementdialog.cpp)
*/
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
  CsiPartType,                //27 Cp
  StepType,                   //28 Stp
  RangeType,                  //29 Rng
  TextType,                   //30
  BomType,                    //31

  PagePointerType,            //32
  SingleStepType,             //33
  ReserveType,                //34
  CoverPageType,              //35
  CsiAnnotationType,          //36
  DividerPointerType,         //37

  NumRelatives                //38
};

enum pageType{
    ContentPage = 0,
    FrontCoverPage,
    BackCoverPage,
    DefaultPage
};

enum AnnotationStyle{
    none = 0,
    circle,
    square,
    rectangle,
    element
};

enum AnnotationCategory{
    notdefined = 0,
    axle,
    beam,
    cable,
    connector,
    hose,
    panel,
    extended
};

/*
enum annotationType{
  TitleAnnotationType = 0,
  FreeFormAnnotationType,
  TitleAndFreeFormAnnotationType,
  AnnotationType
};
*/

enum SortOption {   // Original Order
    PartColour = 0, // Part Size
    PartCategory,   // Part Color
    PartSize,       // Part Category
    PartElement,    // Part Element
    NoSort,
    SortByOptions
};

const QString SortOptionName[SortByOptions] =
{                    // Original Order
    "Part Color" ,   // "Part Size"
    "Part Category", // "Part Color"
    "Part Size",     // "Part Category"
    "Part Element",  // "Part Element"
    "No Sort"        //
};

enum SceneObjectDirection
{
    BringToFront = 1/*True*/,
    SendToBack   = 0/*False*/
};

enum sortDirection {
    SortAscending = 0,
    SortDescending,
    SortDirections
};

const QString SortDirectionName[SortDirections] =
{
    "Ascending",
    "Descending"
};

enum sortOrder {
    SortPrimary = 0,
    SortSecondary,
    SortTetriary,
    PrimaryDirection,
    SecondaryDirection,
    TertiaryDirection
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

class CsiAnnotationIconData
{
public:
  QStringList placements;     // My placement attributes
  float       iconOffset[2];  // My offset from the part;
  float       partOffset[2];  // My part offset from the csi
  int         partSize[2];    // How big is my part (in pixels)?
  QString     typeBaseName;   // My part name without extension
  int         typeColor;      // My part color
  bool        hidden;         // Am I hidden ?

  CsiAnnotationIconData()
  {
    placements    = QStringList() << QString::number(BottomLeft) << QString::number(Outside);
    iconOffset[0] = 0.0f;
    iconOffset[1] = 0.0f;
    partOffset[0] = 0.0f;
    partOffset[1] = 0.0f;
    partSize[0]   = 0;
    partSize[1]   = 0;
    typeBaseName  = QString();
    typeColor     = -1;
    hidden        = false;
  }
};

class RotStepData
{
public:
  double  rots[3];
  QString type;
  bool    populated;
  RotStepData()
  {
    populated  = false;
    type       = QString();
    rots[0]    = 0;
    rots[1]    = 0;
    rots[2]    = 0;
  }
  RotStepData & operator=(const RotStepData &rhs)
  {
    populated = rhs.populated;
    type      = rhs.type;
    rots[0]   = rhs.rots[0];
    rots[1]   = rhs.rots[1];
    rots[2]   = rhs.rots[2];
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

class JustifyStepData
{
public:
  JustifyStepEnc type;
  float          spacing;
  JustifyStepData()
  {
    type    = JustifyLeft;
    spacing = 0.05f;
  }
};

class SceneObjectData
{
public:
  qreal z;
  float scenePos[2];
  SceneObjectDirection direction;
  bool  armed;
  SceneObjectData()
  {
    direction   = SendToBack;
    scenePos[0] = 0.0f;
    scenePos[1] = 0.0f;
    z           = 0.0f;
    armed       = false;
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
    InsertRichText,
    InsertArrow,
    InsertBom,
    InsertRotateIcon,
  } type;

  struct InsertWhere {
    QString      modelName  = "undefined";
    int          lineNumber = 0;
  };
  InsertWhere    where;
  QString        picName;
  qreal          picScale;
  QString        text;
  QString        textFont;
  QString        textColor;
  QPointF        arrowHead;
  QPointF        arrowTail;
  qreal          haftingDepth;
  QPointF        haftingTip;
  float          offsets[2];
  RectPlacement  rectPlacement;
  PlacementType  relativeTo;
  PlacementEnc   placement;
  PlacementEnc   justification;
  PrepositionEnc preposition;
  PlacementType  relativeType;
  PlacementType  parentRelativeType;
  bool           defaultPlacement;
  bool           placementCommand;
  InsertData()
  {
    picScale           = 1.0;
    offsets[0]         = 0.5;
    offsets[1]         = 0.5;
    haftingDepth       = 0;
    textFont           = "Arial,48,-1,255,75,0,0,0,0,0";
    textColor          = "Black";
    defaultPlacement   = true;
    placementCommand   = false;
    rectPlacement      = TopLeftInsideCorner;
    relativeType       = TextType;
    relativeTo         = PageType;
    parentRelativeType = PageType;
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
    stretch  = true;
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
  } line;
  QString color;
  float   thickness;  // in units [inches]
  float   radius;     // in units [inches]
  float   margin[2];  // in units [inches]
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
    struct Where {
      QString modelName  = "undefined";
      int     lineNumber = 0;
    };
    int id;
    QString parent;
    BorderData lineData;
    Where      lineWhere;
    BorderData borderData;
    Where      borderWhere;

    PointerAttribData()
    {
        lineData.color        = "Black";
        lineData.type         = BorderData::BdrRound;
        lineData.line         = BorderData::BdrLnSolid;
        lineData.thickness    = 1.0f/32.0f;
        lineData.radius       = 15;
        lineData.margin[0]    = 0;
        lineData.margin[1]    = 0;
        lineData.useDefault   = true;    // flag if using attribute meta
        lineData.hideArrows   = false;   // flag if pointer tip is hidden

        borderData.color      = QString();
        borderData.type       = BorderData::BdrRound;
        borderData.line       = BorderData::BdrLnSolid;
        borderData.thickness  = 1.0f/64.0f;
        borderData.radius     = 15;
        borderData.margin[0]  = 0;
        borderData.margin[1]  = 0;
        borderData.useDefault = true;
        borderData.hideArrows = false;

        attribType            = PointerAttribData::Line;
        id                    = 0;
        parent                = QString();
    }
};

class SubData
{
public:
  QString attrs;
  QString color;
  QString part;
  int     type;
  SubData()
  {
    type  = 0;
  }
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
  enum LengthType {
    Default,
    LenPage,
    LenCustom
  } type;
  float   length;     // in units [inches]
  float   thickness;  // in units [inches]
  QString color;
  float   margin[2];  // in units [inches]
  SepData()
  {
    type       = Default;
    length     = -1;
    thickness  = 0.125;
    margin[0]  = 0;
    margin[1]  = 0;
  }
};

class PliPartGroupData
{
public:
  struct Where {
    QString   modelName  = "undefined";
    int       lineNumber = 0;
  };
  Where       group;
  QString     type;
  QString     color;
  bool        bom;
  bool        bPart;
  double      zValue;
  double      offset[2];
  PliPartGroupData()
  {
    bom       = false;
    bPart     = false; // to indicate if Bom part group exists
    zValue    = 0;
    offset[0] = 0.0;
    offset[1] = 0.0;
  }
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
   "CsiPartType",                //27 Cp
   "BomType",                    //28

   "PagePointerType",            //29
   "SingleStepType",             //30
   "StepType",                   //31
   "RangeType",                  //32
   "ReserveType",                //33
   "CoverPageType",              //34
   "CsiAnnotationType",          //35
   "DividerPointerType"          //36

}; //NumRelatives"               //37

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
