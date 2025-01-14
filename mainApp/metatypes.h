
/****************************************************************************
**
** Copyright (C) 2007-2009 Kevin Clague. All rights reserved.
** Copyright (C) 2015 - 2025 Trevor SANDY. All rights reserved.
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
 * Please see lpub.h for an overall description of how the files in LPub3D
 * make up the LPub3D program.
 *
 ***************************************************************************/

#ifndef METATYPES_H
#define METATYPES_H

#include <QString>
#include <QStringList>
#include <QPointF>
#include <QGradient>
#include "lpub_preferences.h"

enum AxisEnc {
    X = 0,
    Y = 1,
    Z = 2
};

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
    CountFalse     = 0,
    CountTrue,     //1
    CountAtModel,  //2
    CountAtStep,   //3
    CountAtTop,    //4
    CountAtOptions //5
};

enum ContStepNumEnc {
    ContStepNumFalse,
    ContStepNumTrue
};

enum FinalModelEnabledEnc {
    FinalModelEnabledFalse,
    FinalModelEnabledTrue
};

enum BuildModEnabledEnc {
    BuildModEnabledFalse,
    BuildModEnabledTrue
};

enum OrientationEnc {
  Portrait = 0,
  Landscape,
  InvalidOrientation
};

enum StudStyleEnc {
    StylePlain,
    StyleThinLineLogo,
    StyleOutlineLogo,
    StyleSharpTopLogo,
    StyleRoundedTopLogo,
    StyleFlattenedLogo,
    StyleHighContrast,
    StyleHighContrastWithLogo,
    StyleCount
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
  TextType,                   //30 Txt
  BomType,                    //31 Bom

  PagePointerType,            //32 Pptr
  SingleStepType,             //33 Ss
  ReserveType,                //34 Res
  CoverPageType,              //35 Cvp
  CsiAnnotationType,          //36 Ca
  DividerPointerType,         //37 Dp

  NumRelatives                //38
};

enum PageTypeEnc{
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

enum SortDirection {
    SortAscending = 0,
    SortDescending,
    SortDirections
};

const QString SortDirectionName[SortDirections] =
{
    "Ascending",
    "Descending"
};

enum SortOrder {
    SortPrimary = 0,
    SortSecondary,
    SortTetriary,
    PrimaryDirection,
    SecondaryDirection,
    TertiaryDirection
};

enum LightType
{
    M_UNDEFINED_LIGHT,
    M_POINTLIGHT,
    M_SUNLIGHT,
    M_SPOTLIGHT,
    M_AREALIGHT,
    M_NUM_LIGHT_TYPES
};

class PlacementData
{
public:
  PlacementEnc   placement;
  PlacementEnc   justification;
  PlacementType  relativeTo;
  PrepositionEnc preposition;
  RectPlacement  rectPlacement;
  QVector<PlacementType>relativeOf;
  PageTypeEnc    pageType;
  bool           partsListPerStep;
  bool           partsListShown;
  bool           subModelShown;
  bool           rotateIconShown;
  bool           stepNumberShown;

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
    pageType   = ContentPage;
    partsListPerStep = false;
    partsListShown   = false;
    subModelShown   = false;
    rotateIconShown = false;
    stepNumberShown = false;
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
};

class BuffExchgData
{
public:
  QString buffer;
  QString type;
};

class BuildModData
{
public:
  QString buildModKey;
  int action;
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
  int itemObj;
  bool  armed;
  float scenePos[2];
  SceneObjectDirection direction;
  SceneObjectData()
  {
    direction   = SendToBack;
    scenePos[0] = 0.0f;
    scenePos[1] = 0.0f;
    armed       = false;
    itemObj     = 0;
  }
};

class PageSizeData
{
public:
  float   sizeW;
  float   sizeH;
  QString sizeID;     // e.g. A4
  OrientationEnc orientation;
  PageSizeData() {
    sizeW       = 0.0f;
    sizeH       = 0.0f;
    orientation = InvalidOrientation;
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
  bool           bomToEndOfSubmodel;
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
    bomToEndOfSubmodel = false;
    rectPlacement      = TopLeftInsideCorner;
    relativeType       = TextType;
    relativeTo         = PageType;
    parentRelativeType = PageType;
  }
};

class BackgroundData
{
public:
  QHash<QString, int> map;
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
    if (map.size() == 0) {
      map["TRANSPARENT"]    = BgTransparent;
      map["COLOR"]          = BgColor;
      map["GRADIENT"]       = BgGradient;
      map["IMAGE"]          = BgImage;
      map["SUBMODEL_COLOR"] = BgSubmodelColor;
    }
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
  QHash<QString, int> map;
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
    BdrNumLines
  } line;

  QString color;
  float   thickness;  // in units [inches]
  float   radius;     // in units [inches]
  float   margin[2];  // in units [inches]
  float   tipWidth;   // in units [inches]
  float   tipHeight;  // in units [inches]
  bool    hideTip;
  bool    useDefault;
  bool    parametricTip;

  BorderData()
  {
    thickness  = 0.125f;
    margin[0]  = 0.0f;
    margin[1]  = 0.0f;
    radius     = 15.0f;
    tipWidth   = 0.0f;
    tipHeight  = 0.0f;
    type       = BdrNone;
    line       = BdrLnNone;
    color      = "Black";
    hideTip    = false;
    useDefault = true;
    parametricTip = true;
    if (map.size() == 0) {
      map["NONE"]         = BdrLnNone;
      map["SOLID"]        = BdrLnSolid;
      map["DASH"]         = BdrLnDash;
      map["DOT"]          = BdrLnDot;
      map["DASH_DOT"]     = BdrLnDashDot;
      map["DASH_DOT_DOT"] = BdrLnDashDotDot;
    }
  }
};

const QString LineTypeNames[BorderData::BdrNumLines] =
{
  "NONE",
  "SOLID",
  "DASH",
  "DOT",
  "DASH_DOT",
  "DASH_DOT_DOT"
};

class PointerAttribData
{
   public:
    enum Attribute {
      Line,
      Border,
      Tip
    } attribType;
    struct Here {
      QString modelName  = "undefined";
      int     lineNumber = 0;
    };
    int id;
    QString parent;
    BorderData tipData;
    Here       tipHere;
    BorderData lineData;
    Here       lineHere;
    BorderData borderData;
    Here       borderHere;

    PointerAttribData()
    {
        tipData.thickness        = 1.0f/64.0f;  // DEFAULT_BORDER_THICKNESS
        tipData.tipWidth         = 0.125f*2.5f; // DEFAULT_TIP_HEIGHT
        tipData.tipHeight        = 0.125f;      // DEFAULT_TIP_WIDTH
        tipData.useDefault       = true;
        tipData.hideTip          = true;
        tipData.parametricTip    = true;

        lineData.color           = "Black";
        lineData.type            = BorderData::BdrRound;
        lineData.line            = BorderData::BdrLnSolid;
        lineData.thickness       = 1.0f/32.0f;  // DEFAULT_LINE_THICKNESS
        lineData.radius          = 15.0f;       // DEFAULT_ROUND_RADIUS
        lineData.margin[0]       = 0.0f;
        lineData.margin[1]       = 0.0f;
        lineData.useDefault      = true;        // flag if settings unchanged
        lineData.hideTip         = false;       // flag if pointer tip is hidden

        borderData.color         = QString();
        borderData.type          = BorderData::BdrRound;
        borderData.line          = BorderData::BdrLnSolid;
        borderData.thickness     = 1.0f/64.0f;  // DEFAULT_BORDER_THICKNESS
        borderData.radius        = 15.0f;       // DEFAULT_ROUND_RADIUS
        borderData.margin[0]     = 0.0f;
        borderData.margin[1]     = 0.0f;
        borderData.useDefault    = true;
        borderData.hideTip       = true;

        attribType               = PointerAttribData::Line;
        id                       = 0;
        parent                   = QString();
    }
};

class RendererData
{
public:
  int  renderer;
  bool useLDVSingleCall;
  bool useLDVSnapShotList;
  bool useNativeGenerator;
  bool usePerspectiveProjection;
  RendererData()
  {
    renderer = -1; // RENDERER_INVALID
    useLDVSingleCall = false;
    useLDVSnapShotList = false;
    useNativeGenerator = false;
    usePerspectiveProjection = false;
  }
};

class FadeColorData
{
public:
  bool useColor;
  QString color;
  FadeColorData()
  {
    useColor = false;
  }
};

class SubData
{
public:
  QString attrs;
  QString color;
  QString part;
  int     type;
  bool    ldrawType;
  SubData()
  {
    type      = 0;
    ldrawType = false;
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
    PliConstrainColumns,
    PliConNum
  } type;
  struct Constraint
  {
    float width;
    float height;
    int  columns;
    Constraint()
      : width(0.0f),
        height(0.0f),
        columns(0)
        {}
  };
  Constraint constraint;
  ConstrainData()
  {
    type = ConstrainData::PliConstrainArea;
  }
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
  double      offset[2];
  PliPartGroupData()
  {
    bom       = false;
    bPart     = false; // to indicate if Bom part group exists
    offset[0] = 0.0;
    offset[1] = 0.0;
  }
};

namespace CameraViews
{
    enum CameraView
    {
      Front,  // set camera view at   0 lat,   0 lon
      Back,   // set camera view at   0 lat, 180 lon
      Top,    // set camera view at  90 lat,   0 lon
      Bottom, // set camera view at -90 lat,   0 lon
      Left,   // set camera view at   0 lat,  90 lon
      Right,  // set camera view at   0 lat, -90 lon
      Home,   // set camera view at  30 lat,  45 lon
      LatLon, // set camera view to specified latitude and longitude
      Default // do not set camera view
    };

    const QString cameraViewNames[CameraView::Default] =
    {
      "FRONT","BACK","TOP","BOTTOM",
      "LEFT","RIGHT","HOME","LAT_LON"
    };
}

class CameraAnglesData
{
public:
  QHash<QString, CameraViews::CameraView> map;

  CameraViews::CameraView cameraView;

  float  angles[2];

  bool customViewpoint;

  int type;

  CameraAnglesData()
  {
    cameraView = CameraViews::CameraView::Default;
    customViewpoint = false;
    type = 1; // Options::CSI
    angles[0] = 0.0f;
    angles[1] = 0.0f;
    if (map.size() == 0) {
       map["FRONT"]   = CameraViews::CameraView::Front;
       map["BACK"]    = CameraViews::CameraView::Back;
       map["TOP"]     = CameraViews::CameraView::Top;
       map["BOTTOM"]  = CameraViews::CameraView::Bottom;
       map["LEFT"]    = CameraViews::CameraView::Left;
       map["RIGHT"]   = CameraViews::CameraView::Right;
       map["HOME"]    = CameraViews::CameraView::Home;
       map["LAT_LON"] = CameraViews::CameraView::LatLon;
       map["DEFAULT"] = CameraViews::CameraView::Default;
    }
  }
};

class LightData
{
public:
  LightData() :
      type(typeNames[Point]),
      areaShape("SQUARE"),
      specular(1.0f),
      spotConeAngle(80.0f),
      cutoffDistance(40.0f),
      povrayPower(1.0f),
      blenderPower(10.0f),
      diffuse(1.0f),
      sunAngle(11.4f),
      pointRadius(0.25f),
      spotRadius(0.25f),
      spotBlend(0.15f),
      fadePower(0.0f),
      fadeDistance(0.0f),
      spotTightness(0.0f),
      spotPenumbraAngle(0.0f),
      areaGridX(2), // 10
      areaGridY(2), // 10
      areaWidth(0.0f),
      areaHeight(0.0f),
      areaSizeX(250.0f), // 0.25f
      areaSizeY(250.0f), // 0.25f
      areaSize(250.0f),  // 0.25f
      latitude(32.0f),
      longitude(45.0f),
      povrayLight(false),
      shadowless(false),
      defaultLight(true)
  {
    if (typeMap.size() == 0)
    {
       typeMap[typeNames[Point]] = Point;
       typeMap[typeNames[Area]] = Area;
       typeMap[typeNames[Sun]] = Sun;
       typeMap[typeNames[Spot]] = Spot;
    }
    color[0] = 1.0f;
    color[1] = 1.0f;
    color[2] = 1.0f;
    target[X] = 0.0f;
    target[Y] = 0.0f;
    target[Z] = 0.0f;
    position[X] = 0.0f;
    position[Y] = 0.0f;
    position[Z] = 0.0f;
    rotation1[X] = 0.0f;
    rotation1[Y] = 0.0f;
    rotation1[Z] = 0.0f;
    rotation2[X] = 0.0f;
    rotation2[Y] = 0.0f;
    rotation2[Z] = 0.0f;
    rotation3[X] = 0.0f;
    rotation3[Y] = 0.0f;
    rotation3[Z] = 0.0f;
  }

  QString getPOVLightMacroString() const
  {
    if (povrayLight)
    {
       const int typeEnc = typeMap[type];
       const float spotFalloff = spotConeAngle / 2.0f;
       const bool isCircle = areaShape.toUpper() == QLatin1String("DISK") || areaShape.toUpper() == QLatin1String("CIRCLE");
       return QString("%1 %2 %3 %4 %5 %6 %7 %8 %9 %10 %11 %12 %13 %14 %15 %16 %17")
           /*01 LightType    */ .arg(typeEnc)
           /*02 Shadowless   */ .arg(shadowless)
           /*03 Latitude     */ .arg(double(latitude),1)
           /*04 Longitude    */ .arg(double(longitude),1)
           /*05 LightTarget  */ .arg(QString("<%1,%2,%3>").arg(double(target[0]),1).arg(double(target[1]),1).arg(double(target[2]),1))
           /*06 LightColor   */ .arg(QString("<%1,%2,%3>").arg(color[0]).arg(color[1]).arg(color[2]))
           /*07 Power        */ .arg(double(povrayPower),1)
           /*08 FadeDistance */ .arg(double(fadeDistance),1)
           /*09 FadePower    */ .arg(double(fadePower),1)
           /*10 SpotRadius   */ .arg(double(typeEnc == Spot ? spotFalloff - spotPenumbraAngle : 0),1)
           /*11 SpotFalloff  */ .arg(double(spotFalloff),1)
           /*12 SpotTightness*/ .arg(double(spotTightness),1)
           /*13 AreaCircle   */ .arg(isCircle ? "1" : "0")
           /*14 AreaWidth    */ .arg(double(areaWidth),1)
           /*15 AreaHeight   */ .arg(double(areaHeight),1)
           /*16 AreaGrid.x   */ .arg(int(areaGridX))
           /*17 AreaGrid.y   */ .arg(int(areaGridY));
    }
    return QString();
  }

  QHash<QString, int>typeMap;
  enum TypeEnc { Point, Area, Sun, Spot, NumTypes };
  QString typeNames[NumTypes] = { "POINT", "AREA", "SUN", "SPOT" };

  QString name;              // QString   mName;
  QString type;              // QString   mLightType; (Light NAME (mName) written on TYPE line)
  QString areaShape;         // QString   mAreaShape;

  float   specular;          // float     mBlenderSpecular;
  float   spotConeAngle;     // float     mSpotConeAngle;
  float   cutoffDistance;    // float     mBlenderCutoffDistance;
  float   povrayPower;       // float     mPOVRayPower;
  float   blenderPower;      // float     mBlenderPower;
  float   diffuse;           // float     mBlenderDiffuse

  float   sunAngle;          // float     mDirectionalBlenderAngle
  float   pointRadius;       // float     mPointBlenderRadius
  float   spotRadius;        // float     mSpotBlenderRadius
  float   spotBlend;         // float     calculated (Blender only)
  float   fadePower;         // float     mPOVRayFadePower
  float   fadeDistance;      // float     mPOVRayFadeDistance
  float   spotTightness;     // float     mSpotPOVRayTightness
  float   spotPenumbraAngle; // float     mSpotPenumbraAngle

  int     areaGridX;         // int       mAreaPOVRayGridX
  int     areaGridY;         // int       mAreaPOVRayGridY
  float   areaWidth;         // float     mAreaX (POVRay light area size X)
  float   areaHeight;        // float     mAreaY (POVRay light area size Y)
  float   areaSizeX;         // float     mAreaSizeX
  float   areaSizeY;         // float     mAreaSizeY
  float   areaSize;          // float     mAreaSizeX

  float   color[3];          // lcVector3 mLightColor
  float   target[3];         // lcVector3 mPosition
  float   position[3];       // lcVector3 mTargetPosition

  float   rotation1[3];      // lcMatrix33 Rotation maxtix X
  float   rotation2[3];      // lcMatrix33 Rotation maxtix Y
  float   rotation3[3];      // lcMatrix33 Rotation maxtix Z

  float   latitude;          // float Calculated
  float   longitude;         // float Calculated

  bool    povrayLight;       // bool      mPOVRayLight
  bool    shadowless;        // bool      mShadowless
  bool    defaultLight;
};

/*********************************************
 *
 * Default camera settings
 *
 *********************************************/

class MetaDefaults
{
public:
  static float getCameraDDF();
  static float getCameraFOV();
  static float getFOVMinRange();
  static float getFOVMaxRange();
  static float getNativeCameraZNear();
  static float getNativeCameraZFar();
  static float getAssemblyCameraLongitude();
  static float getAssemblyCameraLatitude();
  static float getPartCameraLongitude();
  static float getPartCameraLatitude();
  static float getSubmodelCameraLongitude();
  static float getSubmodelCameraLatitude();
  static QString getPreferredRenderer(int=-1);
  static QString getPreferredUnits();
  static QString getDefaultCSIKeys();
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

   "PageTitleType",              // 7 Tt
   "PageModelNameType",          // 8 Mnt
   "PageAuthorType",             // 9 At
   "PageURLType",                //10 Urlt
   "PageModelDescType",          //11 Mdt
   "PagePublishDescType",        //12 Pdt
   "PageCopyrightType",          //13 Ct
   "PageEmailType",              //14 Et
   "PageDisclaimerType",         //15 Dt
   "PagePartsType",              //16 Pt
   "PagePlugType",               //17 Plt
   "SubmodelInstanceCountType",  //18 Sic
   "PageDocumentLogoType",       //19 Dlt
   "PageCoverImageType",         //20 Cit
   "PagePlugImageType",          //21 Pit
   "PageHeaderType",             //22 Ph
   "PageFooterType",             //23 Pf
   "PageCategoryType",           //24 Cat
   "SubModelType",               //25 Sm
   "RotateIconType",             //26 Ri
   "CsiPartType",                //27 Cp
   "StepType",                   //28 Stp
   "RangeType",                  //29 Rng
   "TextType",                   //30 Txt
   "BomType",                    //31 Bom

   "PagePointerType",            //32 Pptr
   "SingleStepType",             //33 Ss
   "ReserveType",                //34 Res
   "CoverPageType",              //35 Cvp
   "CsiAnnotationType",          //36 Ca
   "DividerPointerType"          //37 Dp

}; //NumRelatives"               //38

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
