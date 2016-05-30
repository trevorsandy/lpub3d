 
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
 * This file describes a set of classes that implement a parse tree for
 * all the meta-commands that LPub supports.  Action metas such as STEP,
 * ROTSTEP, CALLOUT BEGIN, etc. return special return codes.  Configuration
 * metas that imply no action, but specify data for later use, retain
 * the onfiguration information, and return a generic OK return code.
 *
 * The top of tree is the Meta class that is the interface to the traverse
 * function that walks the LDraw model higherarchy.  Meta also tracks
 * locations in files like topOfModel, bottomOfModel, bottomOfSteps,topOfRange,
 * bottomOfRange, topOfStep, bottomOfStep, etc.
 *
 * Please see lpub.h for an overall description of how the files in LPub
 * make up the LPub program.
 *
 ***************************************************************************/

#ifndef META_H
#define META_H

#include <QString>
#include <QStringList>
#include <QList>
#include <QHash>
#include <QRegExp>
#include <QStringList>
#include <QMessageBox>
#include "where.h"
#include "metatypes.h"
#include "resolution.h"

class QStringList;

class Meta;
class BranchMeta;

enum Rc {
         InvalidLDrawLineRc = -3,
         RangeErrorRc = -2,
         FailureRc = -1,
         OkRc = 0,
         StepRc,
         RotStepRc,

         StepGroupBeginRc,
         StepGroupDividerRc,
         StepGroupEndRc,

         CalloutBeginRc,
         CalloutPointerRc,
         CalloutDividerRc,
         CalloutEndRc,             
         
         InsertRc,
         InsertPageRc,
         InsertCoverPageRc,

         ClearRc,
         BufferStoreRc,
         BufferLoadRc,
         MLCadSkipBeginRc,
         MLCadSkipEndRc,
         MLCadGroupRc,

         PliBeginIgnRc,
         PliBeginSub1Rc,
         PliBeginSub2Rc,
         PliEndRc,

         PartBeginIgnRc,
         PartEndRc,

         BomBeginIgnRc,
         BomEndRc,

         ReserveSpaceRc,
         PictureAsStep,

         GroupRemoveRc,
         RemoveGroupRc,
         RemovePartRc,
         RemoveNameRc,

         SynthBeginRc,
         SynthEndRc,

         ResolutionRc,
         
         IncludeRc,

         NoStepRc,

         EndOfFileRc,
};

#define DEFAULT_MARGIN  0.05f
#define DEFAULT_MARGINS DEFAULT_MARGIN,DEFAULT_MARGIN
#define DEFAULT_MARGIN_RANGE 0.0f,100.0f
#define DEFAULT_THICKNESS 1.0f/32.0f

/*
 * This abstract class is the root of all meta-command parsing
 * objects.  Each parsing object knows how to intialize itself,
 * parse from the current argv through the end of args, perform
 * a preamble match (used for recognizing meta commands when
 * making changes to the LDraw file, and it also knows how to
 * document itself.
 */

class AbstractMeta
{
public:
  int       pushed;
  bool      global;
  QString            preamble;

  static bool reportErrors;

  AbstractMeta()
  {
     pushed = 0;
     global = false;
  }
  AbstractMeta(const AbstractMeta &rhs)
  {
     pushed   = rhs.pushed;
     global   = rhs.global;
     preamble = rhs.preamble;
  }
  virtual           ~AbstractMeta() { preamble.clear(); }
  
  /* Initialize thyself */

  virtual void init(BranchMeta *parent, 
                    QString name);

  /* Parse thyself */

  virtual Rc parse(QStringList &argv, int index, Where &here) = 0;

  /* Compare argvs against matching string */

  virtual bool preambleMatch(QStringList &, int index, QString &match) = 0;

  /* Document thyself */
  virtual void doc(QStringList &out, QString preamble);

  /* Undo a push */

  virtual void pop() = 0;
};

/*------------------------*/

/*
 * This abstract class represents terminal nodes of a meta command
 */

class LeafMeta : public AbstractMeta {
public:
  Where     _here[2];

  LeafMeta() : AbstractMeta()
  { 
  }
  LeafMeta(const LeafMeta &rhs) : AbstractMeta(rhs)
  {
    _here[0] = rhs._here[0];
    _here[1] = rhs._here[1];
  }
  const Where &here()
  {
    return _here[pushed];
  }
  void clear()
  {
    Where emptyHere;
    _here[0] = emptyHere;
    _here[1] = emptyHere;
    pushed = 0;
    global = false;
  }

  void pop()
  {
    pushed = 0;
  }

  bool preambleMatch(QStringList &argv, int index, QString &match) 
  {
    if (argv.size() != index) {
    }
    return preamble == match;
  }
  
  virtual QString format(bool local, bool global) = 0;
  
  virtual QString format(bool local, bool global, QString);

  virtual void doc(QStringList &out, QString preamble)  { out << preamble; }
};

/*
 * This class represents non-terminal keywords in a syntax
 */

class BranchMeta : public AbstractMeta {
public:

  /* 
   * This is a list of the possible keywords for this token in
   * the syntax
   */

  QHash<QString, AbstractMeta *> list;
  BranchMeta() : AbstractMeta() {}
  virtual ~BranchMeta();
  
  virtual Rc parse(QStringList &argv, int index, Where &here);
  virtual bool    preambleMatch(QStringList &argv, int index, QString &_preamble);
  virtual void    doc(QStringList &out, QString preamble);
  virtual void    pop();
  BranchMeta &operator= (const BranchMeta &rhs)
  {
    QString key;
    foreach(key, list.keys()) {
      *list[key] = *rhs.list[key];
    }
    preamble = rhs.preamble;
    return *this;
  }
  BranchMeta (const BranchMeta &rhs) : AbstractMeta(rhs)
  {
    QString key;
    foreach(key, list.keys()) {
      *list[key] = *rhs.list[key];
    }
  }
};

/*
 * This leaf parsing class returns a special return code, typically
 * used for action meta-commands like LPUB PLI BEGIN SUB, or LPUB
 * CALLOUT BEGIN
 */

class RcMeta : public LeafMeta {
public:
  Rc rc;  // the return code
  RcMeta() : LeafMeta() {}
  RcMeta(const RcMeta &rhs) : LeafMeta(rhs)
  {
    rc = rhs.rc;
  }
  virtual ~RcMeta() {}
  virtual void    init(BranchMeta *parent, const QString name, Rc _rc=OkRc);
  virtual Rc parse(QStringList &argv, int index, Where &here);
  virtual QString format(bool,bool) { QString foo; return foo; }
  virtual void    doc(QStringList &out, QString preamble);
};

/*
 * This leaf meta is used when the the rest of the input
 * is a simple integer
 */
  
class IntMeta : public RcMeta {
private:
  int       _value[2];
  int       base;  // 10 or 16?
public:
  int       _min, _max;
  int       value()
  {
    return _value[pushed];
  }
  IntMeta()
  {
    _value[0] = 0;
    _value[1] = 0;
    base = 10;
    _min = 0;
    _max = 0;
  }
  IntMeta(const IntMeta &rhs) : RcMeta(rhs)
  {
    _value[0] = rhs._value[0];
    _value[1] = rhs._value[1];
    base      = rhs.base;
    _min      = rhs._min;
    _max      = rhs._max;
  }
  void setRange(int min, int max)
  {
    _min = min;
    _max = max;
  }
  virtual ~IntMeta() {}
  virtual void init(BranchMeta *parent, 
                    const QString name, 
                    Rc _rc=OkRc);
  virtual Rc parse(QStringList &argv, int index, Where &here);
          QString format(bool,bool);
  virtual void    doc(QStringList &out, QString preamble);
};
/*
 * This is a leaf object for floating point number */
  
class FloatMeta : public RcMeta {
protected:
  float     _value[2];
public:
  float     _min, _max;
  int       _fieldWidth;
  int       _precision;
  FloatMeta()
  {
    _value[0] = 0;
    _min = 0;
    _max = 0;
    _fieldWidth = 6;
    _precision = 4;
    _inputMask = "9.9999";
  }
  FloatMeta(const FloatMeta &rhs) : RcMeta(rhs)
  {
    _value[0]   = rhs._value[0];
    _value[1]   = rhs._value[1];
    _min        = rhs._min;
    _max        = rhs._max;
    _fieldWidth = rhs._fieldWidth;
    _precision  = rhs._precision;
  }
  virtual ~FloatMeta() {}
  QString   _inputMask;
  virtual float value()
  {
    return _value[pushed];
  }
  virtual void setValue(float value)
  {
    _value[pushed] = value;
  }
  void setRange(float min, float max)
  {
    _min = min;
    _max = max;
  }
  void setFormats(
    int fieldWidth,
    int precision,
    QString inputMask)
  {
    _fieldWidth = fieldWidth;
    _precision  = precision;
    _inputMask  = inputMask;
  }
  virtual void    init(BranchMeta *parent, 
                       const QString name, 
                       Rc _rc=OkRc);
  virtual Rc parse(QStringList &argv, int index, Where &here);
          QString format(bool,bool);
  virtual void    doc(QStringList &out, QString preamble);
};

/* This is a leaf object class for two floating point numbers */
  
class FloatPairMeta : public RcMeta {
protected:
  float     _value[2][2];
  float     _min,_max;
  bool      _default;
public:
  int       _fieldWidth;
  int       _precision;
  QString   _inputMask;
  FloatPairMeta()
  {
    _value[0][0] = 0;
    _value[0][1] = 0;
    _min = 0;
    _max = 0;
    _fieldWidth = 6;
    _precision = 4;
    _inputMask = "9.9999";
    _default = true;
  }
  FloatPairMeta(const FloatPairMeta &rhs) : RcMeta(rhs)
  {
    _value[0][0] = rhs._value[0][0];
    _value[0][1] = rhs._value[0][1];
    _value[1][0] = rhs._value[1][0];
    _value[1][1] = rhs._value[1][1];
    _min         = rhs._min;
    _max         = rhs._max;
    _fieldWidth  = rhs._fieldWidth;
    _precision   = rhs._precision;
    _inputMask   = rhs._inputMask;
  }

  virtual float value(int i)
  {
    return _value[pushed][i];
  }
  void setValue(int which, float v)
  {
    _value[pushed][which] = v;
    _default = false;
  }
  void setValues(float v1, float v2)
  {
    _value[pushed][0] = v1;
    _value[pushed][1] = v2;
    _default = false;
  }
  void setRange(
    float min,
    float max)
  {
    _min = min;
    _max = max;
  }
  void setFormats(
    int fieldWidth,
    int precision,
    QString inputMask)
  {
    _fieldWidth = fieldWidth;
    _precision  = precision;
    _inputMask  = inputMask;
  }
  bool isDefault()
  {
    return _default;
  }
  virtual ~FloatPairMeta() {}
  virtual void    init(BranchMeta *parent, 
                    const QString name,
                    Rc _rc=OkRc);
  virtual Rc parse(QStringList &argv, int index, Where &here);
  virtual QString format(bool,bool);
  virtual void doc(QStringList &out, QString preamble);
};

/*
 * This leaf meta is used when using real world measuring units
 */
  
class UnitMeta : public FloatMeta {
public:
  virtual float value()  // the value as the user sees it 
  {
    float t = _value[pushed];
    
    if (resolutionType() == DPCM) {
      t = inches2centimeters(t);
    }
    return t;
  }
  virtual void setValue(float value)
  {
    if (resolutionType() == DPCM) {
      value = centimeters2inches(value);
    }
    _value[pushed] = value;
  }
  virtual float valueInches()  // the value in inches
  {
    return FloatMeta::value();
  }
  virtual void setValueInches(float value)
  {
    _value[pushed] = value;
  }
  virtual int valuePixels();
  UnitMeta() : FloatMeta()
  {
    _value[0] = 0;
    _min = 0;
    _max = 0;
    _fieldWidth = 6;
    _precision = 4;
    _inputMask = "9.9999";
  }
  QString format(bool, bool);
  virtual ~UnitMeta() {}
};
  
class UnitsMeta : public FloatPairMeta {
private:
public:
  virtual float value(int which)
  {
    float t = _value[pushed][which];
    
    if (resolutionType() == DPCM) {
      t = inches2centimeters(t);
    }
    return t;
  }
  virtual void setValue(int which, float value)
  {
    if (resolutionType() == DPCM) {
      value = centimeters2inches(value);
    }
    _value[pushed][which] = value;
  }
  virtual void setValuesFoo(float v1, float v2)
  {
    if (resolutionType() == DPCM) {
      v1 = centimeters2inches(v1);
      v2 = centimeters2inches(v2);
    }
    _value[pushed][0] = v1;
    _value[pushed][1] = v2;
  }
  
  virtual float valueInches(int which)
  {
    return _value[pushed][which];
  }
  virtual void setValueInches(int which, float value)
  {
    _value[pushed][which] = value;
  }
  virtual void setValuesInches(float v1, float v2)
  {
    _value[pushed][0] = v1;
    _value[pushed][1] = v2;
  }
  
  virtual int valuePixels(int which);

  UnitsMeta() : FloatPairMeta()
  {
    _value[0][0] = 0;
    _value[0][1] = 0;
    _min = 0;
    _max = 0;
    _fieldWidth = 6;
    _precision  = 2;
    _inputMask  = "999.99";
  }
  virtual ~UnitsMeta() {}
  virtual QString format(bool,bool);
};
  
class MarginsMeta : public UnitsMeta {
private:
public:
  MarginsMeta() : UnitsMeta()
  {
    _value[0][0] = DEFAULT_MARGIN;
    _value[0][1] = DEFAULT_MARGIN;
    _min = 0;
    _max = 100;
    _fieldWidth = 6;
    _precision = 4;
    _inputMask = "9.9999";
  }
  virtual ~MarginsMeta() {}
};


/* This leaf class is used for strings */

class StringMeta : public RcMeta {
private:
protected:
  QString _value[2];
  QString delim;
  
public:
  QString value()
  {
    return _value[pushed];
  }
  void setValue(QString value)
  {
    _value[pushed] = value;
  }
  StringMeta() : RcMeta()
  {
  }
  StringMeta(const StringMeta &rhs) : RcMeta(rhs)
  {
    _value[0] = rhs._value[0];
    _value[1] = rhs._value[1];
    delim     = rhs.delim;
  }
  virtual ~StringMeta() {}
  virtual void    init(BranchMeta *parent,
                    QString name, 
                    Rc _rc=OkRc, 
                    QString _delim="\"");
  virtual Rc parse(QStringList &argv, int index, Where &here);
          QString format(bool,bool);
  void    pop() 
  { 
    if (pushed) {
      _value[1].clear(); 
      pushed = 0;
    }
  }

  virtual void doc(QStringList &out, QString preamble);
};

/* This leaf class is for multiple strings */

class StringListMeta : public RcMeta {
public:
  QStringList _value[2];
  QString     delim;
  QString value(int i)
  {
    if (i >= _value[pushed].size()) {
      i = _value[pushed].size() - 1;
    }
    if (i >= 0) {
      return _value[pushed][i];
    } else {
      return "";
    }
  }
  void setValue(QString value)
  {
    _value[0] << value;
  }
  StringListMeta() : RcMeta()
  { 
  }
  StringListMeta(const StringListMeta &rhs) : RcMeta(rhs)
  {
    _value[0] = rhs._value[0];
    _value[1] = rhs._value[1];
    delim     = rhs.delim;
  }
  virtual ~StringListMeta() {}
  virtual void init(BranchMeta *parent,
                    QString name,
                    Rc _rc=OkRc, 
                    QString _delim = "\"");
  virtual Rc parse(QStringList &argv, int index, Where &here);
  QString format(bool,bool);
  void    pop() 
  { 
    if (pushed) {
      _value[1].clear(); 
      pushed = 0;
    }
  }
  virtual void doc(QStringList &out, QString preamble);
};

/* This leaf class is used for fonts */

class FontMeta : public StringMeta {
private:
  
public:
  QString valueFoo()
  {
    // 0 family
    // 1 pointSizeF
    // 2 pixelSize
    // 3 styleHint
    // 4 weight
    // 5 underline
    // 6 strikeout
    // 7 strikeOut
    // 8 fixedPitch
    // 9 rawMode

    QString pixels = _value[pushed];

#if 0
    QStringList list = pixels.split(",");

    // points = 1/72
    // height = points/72
    float units;
    units = list[1].toFloat()/72.0;  // now we have inches
    units *= resolution();

    list[1] = QString("%1") .arg(int(units+0.5));

    pixels = list.join(",");
#endif

    return pixels;
  }
  void setValueFoo(QString value)
  {
    if (resolutionType() == DPI) {
      _value[pushed] = value;
    } else {
      // 0 family
      // 1 pointSizeF
      // 2 pixelSize
      // 3 styleHint
      // 4 weight
      // 5 underline
      // 6 strikeout
      // 7 strikeOut
      // 8 fixedPitch
      // 9 rawMode

      QStringList list = _value[pushed].split(",");
#if 0
      // points = 1/72
      // height = points/72

      float units;
      units = list[1].toFloat()/72.0;  // now we have inches
      units /= 02.54;

      list[1] = QString("%1") .arg(int(units+0.5));
#endif
      QString pixels = list.join(",");

      _value[pushed] = pixels;
    }
  }
  QString valuePoints()
  {
    return _value[pushed];
  }
  void setValuePoints(QString value)
  {
    _value[pushed] = value;
  }
  FontMeta() : StringMeta()
  {
    _value[0] = "Arial,64,-1,255,75,0,0,0,0,0";
  }
  FontMeta(const FontMeta &rhs) : StringMeta(rhs) {}
  virtual ~FontMeta() {}
};

/* This leaf class is used for fonts */

class FontListMeta : public StringListMeta {
private:
public:
  FontListMeta() : StringListMeta() {}
  FontListMeta(const FontListMeta &rhs) : StringListMeta(rhs) {}
  virtual ~FontListMeta() {}
};

/* This leaf is to catch booleans (TRUE or FALSE) */

class BoolMeta : public RcMeta {
private:
  bool  _value[2];
public:
  bool  value()
  {
    return _value[pushed];
  }
  void setValue(bool value)
  {
    _value[pushed] = value;
  }
  BoolMeta () 
  {
  }
  BoolMeta(const BoolMeta &rhs) : RcMeta(rhs)
  {
    _value[0] = rhs._value[0];
    _value[1] = rhs._value[1];
  }
  virtual ~BoolMeta() {}
  Rc parse(QStringList &argv, int index, Where &here);
  QString format(bool,bool);
  virtual void doc(QStringList &out, QString preamble);
};

/* This class is used to parse placement information */

class PlacementMeta  : public LeafMeta
{
private:
  PlacementData _value[2];
public:
  PlacementData &value()
  {
    return _value[pushed];
  }
  void setValue(PlacementData &value)
  {
    _value[pushed] = value;
  }
  void setValue(RectPlacement  placement, PlacementType relativeTo);
  void setValue(int loc[2])
  {
    _value[pushed].offsets[0] = loc[0];
    _value[pushed].offsets[1] = loc[1];
  }
  PlacementMeta();
  PlacementMeta(const PlacementMeta &rhs) : LeafMeta(rhs)
  {
    _value[0] = rhs._value[0];
    _value[1] = rhs._value[1];
  }
  virtual ~PlacementMeta() {}
  void clear()
  {
    LeafMeta::clear();
    _value[0].clear();
    _value[1].clear();
  }
  Rc parse(QStringList &argv, int index, Where &here);
  QString format(bool,bool);
  QString formatOffset(bool,bool);
  virtual void doc(QStringList &out, QString preamble);
};

/* This class is used to parse background data */

class BackgroundMeta : public LeafMeta
{
private:
  BackgroundData _value[2];
public:
  BackgroundData &value()
  {
    return _value[pushed];
  }
  void setValue(BackgroundData &value)
  {
    _value[pushed] = value;
  }
  void setValue(
    BackgroundData::Background type, 
    QString string)
  {
    _value[pushed].type = type;
    _value[pushed].string = string;
  }
  void setValue(
    BackgroundData::Background type)
  {
    _value[pushed].type = type;
  }
  BackgroundMeta() : LeafMeta()
  { 
  }
  BackgroundMeta(const BackgroundMeta &rhs) : LeafMeta(rhs)
  {
    _value[0] = rhs._value[0];
    _value[1] = rhs._value[1];
  }
  virtual ~BackgroundMeta() {}
  Rc parse(QStringList &argv, int index, Where &here);
  QString format(bool,bool);
  void    pop()
  {
    if (pushed) {
      _value[1].string.clear();
      pushed = 0;
    }
  }
  virtual void doc(QStringList &out, QString preamble);
  virtual QString text();
};


/* This leaf class is used to parse border metas */

class BorderMeta : public LeafMeta
{
private:
  BorderData _value[2];  // some of this is in units
  BorderData _result;
public:
  BorderData &value()
  {
    _result = _value[pushed];
    if (resolutionType() == DPCM) {
      _result.thickness = inches2centimeters(_result.thickness);
      _result.margin[0] = inches2centimeters(_result.margin[0]);
      _result.margin[1] = inches2centimeters(_result.margin[1]);
    }
    return _result;
  }
  void setValue(BorderData border)
  {
    if (resolutionType() == DPCM) {
      border.thickness = centimeters2inches(border.thickness);
      border.margin[0] = centimeters2inches(border.margin[0]);
      border.margin[1] = centimeters2inches(border.margin[1]);
    }
    _value[pushed] = border;
  }
  BorderData valueInches()
  {
    return _value[pushed];
  }
  void setValueInches(BorderData &borderData)
  {
    _value[pushed] = borderData;
  }
  int setBorderLine(const QString argvIndex)
  {
    if (argvIndex == "0"){
        return BorderData::BdrLnNone;
      }
    else if (argvIndex == "1"){
      return BorderData::BdrLnSolid;
      }
    else if (argvIndex == "2"){
      return BorderData::BdrLnDash;
      }
    else if (argvIndex == "3"){
      return BorderData::BdrLnDot;
      }
    else if (argvIndex == "4"){
      return BorderData::BdrLnDashDot;
      }
    else {//"Dot-Dot-Dash Line"
      return BorderData::BdrLnDashDotDot;
      }
  }
  BorderData &valuePixels()
  {
    _result = _value[pushed];
    _result.margin[0]*=resolution();
    _result.margin[1]*=resolution();
    _result.thickness*=resolution();
    return _result;
  }
  BorderMeta() : LeafMeta()
  {
    _value[0].thickness = DEFAULT_THICKNESS;
    _value[0].margin[0] = 0;
    _value[0].margin[1] = 0;
  }
  BorderMeta(const BorderMeta &rhs) : LeafMeta(rhs)
  {
    _value[0] = rhs._value[0];
    _value[1] = rhs._value[1];
    _result = rhs._result;
  }
  virtual ~BorderMeta() { }
  Rc parse(QStringList &argv, int index, Where &here);
  QString format(bool,bool);
  void    pop() 
  { 
    if (pushed) {
      _value[1].color.clear();
      pushed = 0;
    }
  }
  virtual void doc(QStringList &out, QString preamble);
  virtual QString text();
};

/*
 * This class parses LPub POINTER meta commands
 */

class PointerMeta : public LeafMeta
{
private:
  PointerData _value[2];
  PointerData _result;
public:
  PointerData valueUnit()
  {
    return _value[pushed];
  }
  PointerData &value()
  {
    _result = _value[pushed];
    _result.base *= resolution();
    return _result;
  }
  void setValue(
    PlacementEnc placement, 
    float loc, 
    float base, 
    float x, 
    float y)
  {
    _value[pushed].placement = placement;
    _value[pushed].loc       = loc;
    _value[pushed].base      = base/resolution();
    _value[pushed].x         = x;
    _value[pushed].y         = y;
  }
  void setValueUnit(
    PlacementEnc placement, 
    float loc, 
    float base, 
    float x, 
    float y)
  {
    _value[pushed].placement = placement;
    _value[pushed].loc       = loc;
    _value[pushed].base      = base;
    _value[pushed].x         = x;
    _value[pushed].y         = y;
  }
  PointerMeta();
  PointerMeta(const PointerMeta &rhs) : LeafMeta(rhs)
  {
    _value[0] = rhs._value[0];
    _value[1] = rhs._value[1];
    _result   = rhs._result;
  }
  virtual ~PointerMeta() {}
  Rc parse(QStringList &argv, int index, Where &here);
  QString format(bool,bool);
  virtual void doc(QStringList &out, QString preamble);
};

/*
 * This class detects FREEFORM keywords */

class FreeFormMeta : public LeafMeta
{
private:
  FreeFormData _value[2];
public:
  FreeFormData &value()
  {
    return _value[pushed];
  }
  FreeFormMeta();
  FreeFormMeta(const FreeFormMeta &rhs) : LeafMeta(rhs)
  {
    _value[0] = rhs._value[0];
    _value[1] = rhs._value[1];
  }
  virtual ~FreeFormMeta() {}
  Rc parse(QStringList &argv, int index, Where &here);
  QString format(bool,bool);
  virtual void doc(QStringList &out, QString preamble);
};

/*
 * This class parses (Vertical|Horizontal)
 */

class AllocMeta : public LeafMeta
{
private:
  AllocEnc type[2];
public:
  AllocEnc value()
  {
    return type[pushed];
  }
  void setValue(AllocEnc value)
  {
    type[pushed] = value;
  }
  AllocMeta();
  AllocMeta(const AllocMeta &rhs) : LeafMeta(rhs)
  {
    type[0] = rhs.type[0];
    type[1] = rhs.type[1];
  }
  virtual ~AllocMeta() {}
  Rc parse(QStringList &argv, int index, Where &here);
  QString format(bool,bool);
  virtual void doc(QStringList &out, QString preamble);
};

/*
 * INSERT
 *   (PICTURE "name" (SCALE x) |
 *    TEXT "font" "string" |
 *    ARROW head_x head_y tail_x tail_y hafting_depth hafting_x hafting_y
 *    BOM
 *    MODEL)
 *   (OFFSET X Y)
 *
 *              . ' (hox hoy) (hafting outside)
 *          . '  /
 *      . '     /
 *   tx---------hix (hafting center)
 *      ` .     \ 
 *          ` .  \
 *              ` .
 */

class InsertMeta : public LeafMeta
{
private:
public:
  InsertData _value;
  InsertMeta() : LeafMeta()
  {
  }
  InsertMeta(const InsertMeta &rhs) : LeafMeta(rhs)
  {
    _value = rhs._value;
  }
  InsertData value()
  {
    return _value;
  }
  void setValue(InsertData &value)
  {
    _value = value;
  }
  virtual ~InsertMeta() {}
  Rc parse(QStringList &argv, int index, Where &here);
  QString format(bool,bool);
  virtual void doc(QStringList &out, QString preamble);
};

class AlignmentMeta : public LeafMeta
{
private:
  Qt::Alignment _value[2];
public:
  AlignmentMeta()
  {
    _value[0] = Qt::AlignLeft;
  }
  AlignmentMeta(const AlignmentMeta &rhs) : LeafMeta(rhs)
  {
    _value[0]   = rhs._value[0];
    _value[1]   = rhs._value[1];
  }
  Rc parse(QStringList &argv, int index, Where &here);
  QString format(bool,bool);
  virtual void doc(QStringList &out, QString preamble);
};

class TextMeta : public BranchMeta
{
public:
  FontMeta       font;
  StringMeta     color;
  AlignmentMeta  alignment;

  TextMeta() : BranchMeta() {}
  TextMeta(const TextMeta &rhs) : BranchMeta(rhs)
  {
    font = rhs.font;
    color = rhs.color;
    alignment = rhs.alignment;
    preamble = rhs.preamble;
  }
  virtual ~TextMeta() {}
  virtual void init(BranchMeta *parent, QString name);
};

class ArrowHeadMeta : LeafMeta
{
private:
  qreal _value[2][4];
public:
  ArrowHeadMeta() : LeafMeta()
  {
    _value[0][0] = 0.0;
    _value[0][1] = 0.25;
    _value[0][2] = 3.0/8;
    _value[0][3] = 0.25;
  }
  ArrowHeadMeta(const ArrowHeadMeta &rhs) : LeafMeta(rhs)
  {
    for (int i = 0; i < 2; i++) {
      for (int j = 0; j < 4; j++) {
        _value[i][j] = rhs._value[i][j];
      }
    }
  }
  Rc parse(QStringList &argv, int index, Where &here);
  QString format(bool,bool);
  virtual void doc(QStringList &out, QString preamble);
};

class ArrowEndMeta : public LeafMeta
{
  bool _value[2]; // false = square, true = round
public:
  ArrowEndMeta() : LeafMeta()
  {
    _value[0] = false;
  }
  ArrowEndMeta(const ArrowEndMeta &rhs) : LeafMeta(rhs)
  {
    _value[0] = rhs._value[0];
    _value[1] = rhs._value[1];
  }

  Rc parse(QStringList &argv, int index, Where &here);
  QString format(bool,bool);
  virtual void doc(QStringList &out, QString preamble);
};

/*
 * This meta parses DIVIDER keywords
 */

class SepMeta : public LeafMeta
{
private:
  SepData _value[2];
  SepData _result;
public:
  SepData &value()
  {
    _result = _value[pushed];
    if (resolutionType() == DPCM) {
      _result.thickness = inches2centimeters(_result.thickness);
      _result.margin[0] = inches2centimeters(_result.margin[0]);
      _result.margin[1] = inches2centimeters(_result.margin[1]);
    }
    return _result;
  }
  void setValue(QString color, 
                float thickness,
                float margin0,
                float margin1)
  {
    if (resolutionType() == DPCM) {
      thickness = centimeters2inches(thickness);
      margin0   = centimeters2inches(margin0);
      margin1   = centimeters2inches(margin1);
    }
    _value[pushed].color = color;
    _value[pushed].thickness = thickness;
    _value[pushed].margin[0] = margin0;
    _value[pushed].margin[1] = margin1;
  }
  void setValue(SepData goods)
  {
    if (resolutionType() == DPCM) {
      goods.thickness = centimeters2inches(goods.thickness);
      goods.margin[0] = centimeters2inches(goods.margin[0]);
      goods.margin[1] = centimeters2inches(goods.margin[1]);
    }
    _value[pushed] = goods;
  }
  SepData &valuePixels()
  {
    _result = _value[pushed];
    _result.thickness *= resolution();
    _result.margin[0] *= resolution();
    _result.margin[1] *= resolution();
    return _result;
  }
  void setValueInches(QString color, 
                float thickness,
                float margin0,
                float margin1)
  {
    _value[pushed].color = color;
    _value[pushed].thickness = thickness;
    _value[pushed].margin[0] = margin0;
    _value[pushed].margin[1] = margin1;
  }
  void setValueInches(SepData &sepData)
  {
    _value[pushed] = sepData;
  }
  SepMeta();
  SepMeta(const SepMeta &rhs) : LeafMeta(rhs)
  {
    _value[0] = rhs._value[0];
    _value[1] = rhs._value[1];
  }

  virtual ~SepMeta() { }
  Rc parse(QStringList &argv, int index, Where &here);
  QString format(bool local,bool);
  void pop()
  {
    if (pushed) {
      _value[1].color.clear();
      pushed = 0;
    }
  }
  virtual void doc(QStringList &out, QString preamble);
};

/*
 * This class parses (Portait|Landscape)
 */

class PageOrientationMeta : public LeafMeta
{
private:
  OrientationEnc type[2];
public:
  OrientationEnc value()
  {
    return type[pushed];
  }
  void setValue(OrientationEnc value)
  {
    type[pushed] = value;
  }
  PageOrientationMeta();
  PageOrientationMeta(const PageOrientationMeta &rhs) : LeafMeta(rhs)
  {
    type[0] = rhs.type[0];
    type[1] = rhs.type[1];
  }
  virtual ~PageOrientationMeta() {}
  Rc parse(QStringList &argv, int index, Where &here);
  QString format(bool,bool);
  virtual void doc(QStringList &out, QString preamble);
};

/*------------------------*/

class CalloutCsiMeta : public BranchMeta
{
public:
  PlacementMeta placement;
  MarginsMeta   margin;
  CalloutCsiMeta();
  CalloutCsiMeta(const CalloutCsiMeta &rhs) : BranchMeta(rhs)
  {
  }

  virtual ~CalloutCsiMeta() {}
  virtual void init(BranchMeta *parent, QString name);
  //virtual void doc(QStringList &out, QString preamble);
};

/*------------------------*/

class CalloutPliMeta : public BranchMeta
{
public:
  PlacementMeta placement;
  MarginsMeta   margin;
  BoolMeta      perStep;
  CalloutPliMeta();
  CalloutPliMeta(const CalloutPliMeta &rhs) : BranchMeta(rhs)
  {
  }

  virtual ~CalloutPliMeta() {}
  virtual void init(BranchMeta *parent, QString name);
};

/*------------------------*/

class NumberMeta : public BranchMeta
{
public:
  int         number;
  FontMeta    font;
  StringMeta  color;
  MarginsMeta margin;
  void setValue(int _value)
  {
    number = _value;
  }
  NumberMeta();
  NumberMeta(const NumberMeta &rhs) : BranchMeta(rhs)
  {
  }

  virtual ~NumberMeta() {}

  virtual void init(BranchMeta *parent, 
                    QString name);
};

class NumberPlacementMeta : public NumberMeta
{
public:
  PlacementMeta  placement;
  NumberPlacementMeta();
  NumberPlacementMeta(const NumberPlacementMeta &rhs) : NumberMeta(rhs)
  {
  }

  virtual ~NumberPlacementMeta() {}
  virtual void init(BranchMeta *parent, 
                    QString name);
};

/*------------------------*/

class PageAttributeTextMeta : public BranchMeta
{
public:
  PlacementType  type;
  FontMeta    	 textFont;
  StringMeta  	 textColor;
  MarginsMeta 	 margin;
  PlacementMeta  placement;
  StringMeta   	 content;
  BoolMeta       display;
  void setValue(QString _value)
  {
      content.setValue(_value);
  }
  QString value()
  {
      return content.value();
  }
  PageAttributeTextMeta();
  PageAttributeTextMeta(const PageAttributeTextMeta &rhs) : BranchMeta(rhs)
  {
  }

  virtual ~PageAttributeTextMeta() {}

  virtual void init(BranchMeta *parent,
                    QString name);
};

/*------------------------*/

class PageAttributePictureMeta : public BranchMeta
{
public:
  PlacementType  type;
  PlacementMeta  placement;
  MarginsMeta 	 margin;
  FloatMeta	 picScale;
  StringMeta	 file;
  BoolMeta	 stretch;
  BoolMeta       tile;
  BoolMeta       display;

  void setValue(QString _value)
  {
      file.setValue(_value);
  }
  QString value()
  {
      return file.value();
  }
  PageAttributePictureMeta();
  PageAttributePictureMeta(const PageAttributePictureMeta &rhs) : BranchMeta(rhs)
  {
  }

  virtual ~PageAttributePictureMeta() {}

  virtual void init(BranchMeta *parent,
                    QString name);
};

/*------------------------*/

class PageHeaderMeta : public BranchMeta
{
    public:
    UnitsMeta      size;
    PlacementMeta  placement;

    PageHeaderMeta();
    PageHeaderMeta(const PageHeaderMeta &rhs) : BranchMeta(rhs)
    {
    }

    virtual ~PageHeaderMeta() {}
    virtual void init(BranchMeta *parent, QString name);
};

/*------------------------*/

class PageFooterMeta : public BranchMeta
{
    public:
    UnitsMeta      size;
    PlacementMeta  placement;

    PageFooterMeta();
    PageFooterMeta(const PageFooterMeta &rhs) : BranchMeta(rhs)
    {
    }

    virtual ~PageFooterMeta() {}
    virtual void init(BranchMeta *parent, QString name);
};

/*------------------------*/
/*
 * Fade Step Meta
 */
class FadeStepMeta : public BranchMeta
{
public:
  StringMeta  fadeColor;
  BoolMeta    fadeStep;

  FadeStepMeta();
  FadeStepMeta(const FadeStepMeta &rhs) : BranchMeta(rhs)
  {
  }

  virtual ~FadeStepMeta() {}
  virtual void init(BranchMeta *parent,
                    QString name);
};
/*------------------------*/

class RemoveMeta : public BranchMeta
{
public:
  StringMeta group;
  StringMeta parttype;
  StringMeta partname;
  RemoveMeta() 
  {
  }
  RemoveMeta(const RemoveMeta &rhs) : BranchMeta(rhs)
  {
  }

  virtual ~RemoveMeta() {}
  virtual void init(BranchMeta *parent, QString name);
};
  
/* This class is used to parse PART IGNORE in PLI metas
 * do we want to try to use values instead of the PART
 * keyword
 */

class PartMeta : public BranchMeta
{
public:
  MarginsMeta margin;

  PartMeta();
  PartMeta(const PartMeta &rhs) : BranchMeta(rhs)
  {
  }

  virtual ~PartMeta() {}
  virtual void init(BranchMeta *parent, QString name);
};

/* 
 * This class is used to parse PLI substitute
 * commands 
 */

class SubMeta : public LeafMeta
{
private:
  SubData _value;
public:
  SubData value()
  {
    return _value;
  }
  SubMeta() : LeafMeta()
  { 
  }
  SubMeta(const SubMeta &rhs) : LeafMeta(rhs)
  {
    _value = rhs._value;
  }

  virtual ~SubMeta() {}
  Rc parse(QStringList &argv, int index, Where &here);
  QString format(bool,bool);
  virtual void doc(QStringList &out, QString preamble);
};

/* This class parses PLI constraints */

class ConstrainMeta : public LeafMeta
{
private:
  ConstrainData _value[2];
  ConstrainData _result;
  bool _default;
  
public:
  bool isDefault()
  {
    return _default;
  }
  void resetToDefault()
  {
    _default = true;
  }
  void pop()
  {
    pushed = 0;
    _default = true;
  }
  ConstrainData valueUnit()
  {
    return _value[pushed];
  }
  ConstrainData &value()
  {
    _result = _value[pushed];
    if (_result.type == ConstrainData::PliConstrainWidth ||
        _result.type == ConstrainData::PliConstrainHeight) {
      _result.constraint *= resolution();
    }
    return _result;
  }
  void setValue(ConstrainData &value)
  {
    value.constraint /= resolution();
    _default = false;
    _value[pushed] = value;
  }
  void setValueUnit(ConstrainData &value)
  {
    _default = false;
    _value[pushed] = value;
  }
  ConstrainMeta();
  ConstrainMeta(const ConstrainMeta &rhs) : LeafMeta(rhs)
  {
    _value[0] = rhs._value[0];
    _value[1] = rhs._value[1];
    _result   = rhs._result;
    _default  = rhs._default;
  }

  virtual ~ConstrainMeta() {}
  Rc parse(QStringList &argv, int index, Where &here);
  QString format(bool,bool);
  virtual void doc(QStringList &out, QString preamble);

};

/*------------------------*/

class PliBeginMeta : public BranchMeta
{
public:
  RcMeta        ignore;
  SubMeta       sub;
  PliBeginMeta() 
  {
  }
  PliBeginMeta(const PliBeginMeta &rhs) : BranchMeta(rhs)
  {
  }

  virtual ~PliBeginMeta() {}
  virtual void init(BranchMeta *parent, QString name);
};

/*------------------------*/

// 0 LPUB PART BEGIN // (IGN|IGNORE)

class PartBeginMeta : public BranchMeta
{
public:
  RcMeta          ignore;
  PartBeginMeta()
  {
  }
  PartBeginMeta(const PartBeginMeta &rhs) : BranchMeta(rhs)
  {
  }

  virtual ~PartBeginMeta() {}
  virtual void init(BranchMeta *parent, QString name);
};

/*------------------------*/

// 0 LPUB PART

class PartIgnMeta : public BranchMeta
{
public:
  PartBeginMeta begin;
  RcMeta        end;
  PartIgnMeta() 
  {
  }
  PartIgnMeta(const PartIgnMeta &rhs) : BranchMeta(rhs)
  {
  }

  virtual ~PartIgnMeta() {}
  virtual void init(BranchMeta *parent, QString name);
};

/*------------------------*/

// 0 LPUB CALLOUT BEGIN

class CalloutBeginMeta : public RcMeta
{

private:
public:
  enum CalloutMode { Unassembled, Assembled, Rotated } mode;

  CalloutMode value()
  {
    return mode;
  }
  CalloutBeginMeta() : RcMeta()
  {
    mode = Unassembled;
  }
  CalloutBeginMeta(const CalloutBeginMeta &rhs) : RcMeta(rhs)
  {
    mode = rhs.mode;
  }

  virtual ~CalloutBeginMeta() {}
  Rc parse(QStringList &argv, int index, Where &here);
  QString format(bool,bool);
  virtual void doc(QStringList &out, QString preamble);
};

/*------------------------*/

/* This class is to parse MLCad's rotation step */

class RotStepMeta : public LeafMeta
{
private:
  RotStepData _value;
public:
  RotStepData value() const
  {
    return _value;
  }
  void clear()
  {
    _value.type.clear();
    _value.rots[0] = 0;
    _value.rots[1] = 0;
    _value.rots[2] = 0;
  }
  RotStepMeta() { 
    _value.type.clear(); 
  }
  RotStepMeta(const RotStepMeta &rhs) : LeafMeta(rhs)
  {
    _value = rhs.value();
  }
  RotStepMeta& operator=(const RotStepMeta &rhs)
  {
    LeafMeta::operator=(rhs);
    _value = rhs.value();
    return *this;
  }

  virtual ~RotStepMeta() {}
  Rc parse(QStringList &argv, int index, Where &here);
  QString format(bool,bool);
  virtual void doc(QStringList &out, QString preamble);
};

/* This class is to parse MLCad's buffer exchange */

class BuffExchgMeta : public LeafMeta
{
private:
  BuffExchgData _value;
public:
  QString value()
  {
    return _value.buffer;
  }
  BuffExchgMeta() 
  { 
  }
  BuffExchgMeta(const BuffExchgMeta &rhs) : LeafMeta(rhs)
  {
  }

  virtual ~BuffExchgMeta() { }
  Rc parse(QStringList &argv, int index, Where &here);
  QString format(bool,bool);
  void    pop() { pushed = 0; }
  virtual void doc(QStringList &out, QString preamble);
};

/*------------------------*/

class PliSortMeta : public BranchMeta
{
public:
  StringMeta      sortOption;

  void setValue(QString _value)
  {
    sortOption.setValue(_value);
  }
  QString value()
  {
    return sortOption.value();
  }
  PliSortMeta();
  PliSortMeta(const PliSortMeta &rhs) : BranchMeta(rhs)
  {
  }

  virtual ~PliSortMeta() {}
  virtual void init(BranchMeta *parent, QString name);
};

/*------------------------*/

class PliAnnotationMeta : public BranchMeta
{
public:
  BoolMeta      titleAnnotation;
  BoolMeta      freeformAnnotation;
  BoolMeta      titleAndFreeformAnnotation;
  BoolMeta      display;

  PliAnnotationMeta();
  PliAnnotationMeta(const PliAnnotationMeta &rhs) : BranchMeta(rhs)
  {
  }

  virtual ~PliAnnotationMeta() {}
  virtual void init(BranchMeta *parent, QString name);
};

/*------------------------*/

class RotateIconMeta  : public BranchMeta
{
public:
  UnitsMeta         size;
  FloatMeta	    picScale;
  BorderMeta        arrow;
  BorderMeta        border;
  BackgroundMeta    background;
  MarginsMeta       margin;
  PlacementMeta     placement;
  BoolMeta          display;
  StringListMeta    subModelColor;

  RotateIconMeta();
  RotateIconMeta(const RotateIconMeta &rhs) : BranchMeta(rhs)
  {
  }
  virtual ~RotateIconMeta() {}
  virtual void init(BranchMeta *parent, QString name);
};

/*---------------------------------------------------------------
 * The Top Level LPub3D Metas
 *---------------------------------------------------------------*/
class PageMeta : public BranchMeta
{
public:
  // top    == top of page
  // bottom == bottom of page
  UnitsMeta                 size;
  PageOrientationMeta       orientation;
  MarginsMeta               margin;
  BorderMeta                border;
  BackgroundMeta            background;
  BoolMeta                  dpn;
  BoolMeta                  togglePnPlacement;
  NumberPlacementMeta       number;
  NumberPlacementMeta       instanceCount;
  StringListMeta            subModelColor;

  PageHeaderMeta            pageHeader;
  PageFooterMeta            pageFooter;

  //pageAttributes 
  PageAttributeTextMeta     titleFront;              //from LDrawFile - LDraw: File
  PageAttributeTextMeta     titleBack;               //from LDrawFile - LDraw: File
  PageAttributeTextMeta     modelName;               //from LDrawFile - LDraw: Name
  PageAttributeTextMeta     modelDesc;               //from LDrawFile - LDraw: 2nd line in <topLevelFile>.ldr
  PageAttributeTextMeta     publishDesc;             //from preferences
  PageAttributeTextMeta     authorFront;             //from LDrawFile - LDraw: Author
  PageAttributeTextMeta     authorBack;              //from LDrawFile - LDraw: Author
  PageAttributeTextMeta     author;                  //from LDrawFile - LDraw: Author
  PageAttributeTextMeta     url;                     //from preferences
  PageAttributeTextMeta     urlBack;                 //from preferences
  PageAttributeTextMeta     email;                   //from preferences
  PageAttributeTextMeta     emailBack;               //from preferences
  PageAttributeTextMeta     disclaimer;              //from preferences static
  PageAttributePictureMeta  documentLogoFront;       //from preferences
  PageAttributePictureMeta  documentLogoBack;        //from preferences
  PageAttributePictureMeta  coverImage;              //from Globals setup
  PageAttributeTextMeta     pieces;                  //from LDrawFile - count .dat during load
  PageAttributeTextMeta     copyrightBack;           //from preferences static
  PageAttributeTextMeta     copyright;               //from preferences static
  PageAttributeTextMeta     plug;                    //from preferences static
  PageAttributePictureMeta  plugImage;               //from preferences static
  PageAttributeTextMeta     category;                //from LDrawFile - LDraw: !CATEGORY (NOT IMPLEMENTED)

  PageMeta();
  PageMeta(const PageMeta &rhs) : BranchMeta(rhs)
  {
  }

  virtual ~PageMeta() {}
  virtual void init(BranchMeta *parent, QString name);
};

/*------------------------*/

class AssemMeta : public BranchMeta
{
public:
  // top    == last step
  // bottom == cur step
  MarginsMeta   margin;
  PlacementMeta placement;
  FloatMeta     modelScale;
  StringMeta    ldviewParms;
  StringMeta    ldgliteParms;
  StringMeta    l3pParms;
  StringMeta    povrayParms;
  BoolMeta      showStepNumber;

  AssemMeta();
  AssemMeta(const AssemMeta &rhs) : BranchMeta(rhs)
  {
  }

  virtual ~AssemMeta() {}
  virtual void init(BranchMeta *parent, QString name);
};

/*------------------------*/


class PliMeta  : public BranchMeta
{
public:
  BorderMeta        border;
  BackgroundMeta    background;
  MarginsMeta       margin;
  NumberMeta        instance;
  NumberMeta        annotate;
  PlacementMeta     placement;
  ConstrainMeta     constrain;
  FloatMeta         modelScale;
  FloatPairMeta     angle;
  PartMeta          part;
  PliBeginMeta      begin;
  RcMeta            end;
  BoolMeta          includeSubs;
  BoolMeta          show;
  StringListMeta    subModelColor;     // FIXME: we need a dialog for submodel level color
  FontListMeta      subModelFont;
  StringListMeta    subModelFontColor;
  StringMeta        ldviewParms;
  StringMeta        ldgliteParms;
  StringMeta        l3pParms;
  StringMeta        povrayParms;
  BoolMeta          pack;
  BoolMeta          sort;
  PliSortMeta       sortBy;
  PliAnnotationMeta annotation;

  PliMeta();
  PliMeta(const PliMeta &rhs) : BranchMeta(rhs)
  {
  }
  virtual ~PliMeta() {}
  virtual void init(BranchMeta *parent, QString name);
};

/*------------------------*/

class BomMeta  : public PliMeta
{
public:
  BomMeta();
  BomMeta(const BomMeta &rhs) : PliMeta(rhs)
  {
  }

  virtual ~BomMeta() {}
  virtual void init(BranchMeta *parent, QString name);
};

/*------------------------*/

class CalloutMeta  : public BranchMeta
{
public:
  // top    == start of callout
  // bottom == end of callout
  // Placement needs to be outside callout begin/end

  PlacementMeta  placement;     // outside

  MarginsMeta    margin;
  CalloutCsiMeta csi;               
  CalloutPliMeta pli;
  RotateIconMeta rotateIcon;
  NumberPlacementMeta stepNum;
  SepMeta        sep;
  FreeFormMeta   freeform;
  CalloutBeginMeta begin;
  RcMeta         divider;
  RcMeta         end;
  AllocMeta      alloc;
  NumberPlacementMeta instance;
  BorderMeta     border;
  BackgroundMeta background;
  PointerMeta    pointer;
  StringListMeta subModelColor;
  FontListMeta   subModelFont;
  StringListMeta subModelFontColor;
  CalloutMeta();
  CalloutMeta(const CalloutMeta &rhs) : BranchMeta(rhs)
  {
  }

  virtual ~CalloutMeta() {}
  virtual void init(BranchMeta *parent, QString name);
};

/*------------------------*/

class MultiStepMeta : public BranchMeta
{
public:
  // top    == start of multistep
  // bot    == bottom of multistep
  PlacementMeta  placement;
  MarginsMeta    margin;
  CalloutCsiMeta csi;
  CalloutPliMeta pli;
  RotateIconMeta rotateIcon;
  NumberPlacementMeta stepNum;
  SepMeta        sep;
  FreeFormMeta   freeform;
  RcMeta         begin;
  RcMeta         divider;
  RcMeta         end;
  AllocMeta      alloc;
  FontListMeta   subModelFont;
  StringListMeta subModelFontColor;
  MultiStepMeta();
  MultiStepMeta(const MultiStepMeta &rhs) : BranchMeta(rhs)
  {
  }

  virtual ~MultiStepMeta() {}
  virtual void init(BranchMeta *parent, QString name);
};

/*------------------------*/

/*
 * Resolution meta
 */
  
class ResolutionMeta : public LeafMeta {
private:
public:
  ResolutionType type()
  {
    return resolutionType();
  }
  float value()
  {
    return resolution();
  }
  void setValue(float _resolution)
  {
    setResolution(_resolution);
  }
  void setValue(ResolutionType _type, float _resolution)
  {
    setResolutionType(_type);
    setResolution(_resolution);
  }
  float ldu()
  {
    return 1.0/64;
  }
  ResolutionMeta() 
  {
  }
  ResolutionMeta(const ResolutionMeta &rhs) : LeafMeta(rhs)
  {
  }
  virtual ~ResolutionMeta() {}
  virtual void init(BranchMeta *parent, 
                    QString name);
  virtual Rc parse(QStringList &argv, int index, Where &here);
          QString format(bool,bool);
  virtual void    doc(QStringList &out, QString preamble);
};

class NoStepMeta : RcMeta {
private:
public:
  NoStepMeta()
  {
  }
  virtual ~NoStepMeta() {}
  virtual void init(BranchMeta *parent,
                    const QString name,
                    Rc _rc=OkRc);
  virtual Rc parse(QStringList &argv, int index, Where &here);
          QString format(bool,bool);
  virtual void    doc(QStringList &out, QString preamble);
};

class LPubMeta : public BranchMeta
{
public:
  ResolutionMeta        resolution;
  PageMeta              page;
  AssemMeta             assem;
  NumberPlacementMeta   stepNumber;
  CalloutMeta           callout;
  MultiStepMeta         multiStep;
  PliMeta               pli;
  BomMeta               bom;
  RemoveMeta            remove;
  FloatMeta             reserve;
  PartIgnMeta           partSub;
  InsertMeta            insert;
  StringMeta            include;
  NoStepMeta            nostep;
  FadeStepMeta          fadeStep;
  RotateIconMeta        rotateIcon;

  LPubMeta();
  virtual ~LPubMeta() {}
  virtual void init(BranchMeta *parent, QString name);
  LPubMeta(const LPubMeta &rhs) : BranchMeta(rhs)
  {
  }
};

/*------------------------*/

class MLCadMeta : public BranchMeta
{
public:
  RcMeta     MLCadSB;
  RcMeta     MLCadSE;
  StringMeta MLCadGrp;
  MLCadMeta() {}
  virtual ~MLCadMeta() {}
  virtual void init(BranchMeta *parent, QString name);
  virtual Rc parse(QStringList &argv, int index, Where &here);
  MLCadMeta(const MLCadMeta &rhs) : BranchMeta(rhs)
  {
  }
};

/*------------------------*/

class LSynthMeta : public BranchMeta
{
public:
  RcMeta        begin;
  RcMeta        end;
  RcMeta        synthesized;
  RcMeta        show;
  RcMeta        hide;
  RcMeta        inside;
  RcMeta        outside;
  RcMeta        cross;
  LSynthMeta() {}
  virtual ~LSynthMeta() {}
  virtual void init(BranchMeta *parent, QString name);
  LSynthMeta(const LSynthMeta &rhs) : BranchMeta(rhs)
  {
  }
};

/*------------------------*/

class SubmodelStack
{
public:
  QString modelName;
  int     lineNumber;
  int     stepNumber;
  SubmodelStack(QString _modelName, int _lineNumber, int _stepNumber)
  {
    modelName = _modelName;
    lineNumber = _lineNumber;
    stepNumber = _stepNumber;
  }
  SubmodelStack()
  {
    modelName = "undefined";
    lineNumber = 0;
    stepNumber = 0;
  }
  SubmodelStack operator=(const SubmodelStack &rhs)
  {
    if (this != &rhs) {
      modelName = rhs.modelName;
      lineNumber = rhs.lineNumber;
      stepNumber = rhs.stepNumber;
    }
    return *this;
  }
  SubmodelStack(const SubmodelStack &rhs)
  {
    modelName = rhs.modelName;
    lineNumber = rhs.lineNumber;
    stepNumber = rhs.stepNumber;
  }
};

class Meta : public BranchMeta
{
public:
  LPubMeta      LPub;
  RcMeta        step;
  RcMeta        clear;
  RotStepMeta   rotStep;
  BuffExchgMeta bfx;
  MLCadMeta     MLCad;
  LSynthMeta    LSynth;

  QList<SubmodelStack>  submodelStack;

  Meta();
  virtual ~Meta();
  Rc    parse(QString &line, Where &here, bool reportErrors = 0);
  bool  preambleMatch(QString &line, QString &preamble);
  virtual void  init(BranchMeta *parent, QString name);
  virtual void  pop();
  void  doc(QStringList &out);

  Meta (const Meta &rhs) : BranchMeta(rhs)
  {
    QString empty;
    init(NULL,empty);
    LPub    = rhs.LPub;
    step    = rhs.step;
    clear   = rhs.clear;
    rotStep = rhs.rotStep;
    LSynth  = rhs.LSynth;
    submodelStack = rhs.submodelStack;
  }

private:
};

const QString RcNames[56] =
{
     "InvalidLDrawLineRc = -3",
     "RangeErrorRc = -2",
     "FailureRc = -1",
     "OkRc = 0",
     "StepRc",
     "RotStepRc",

     "StepGroupBeginRc",
     "StepGroupDividerRc",
     "StepGroupEndRc",

     "CalloutBeginRc",
     "CalloutPointerRc",
     "CalloutDividerRc",
     "CalloutEndRc",

     "InsertRc",
     "InsertPageRc",
     "InsertCoverPageRc",

     "ClearRc",
     "BufferStoreRc",
     "BufferLoadRc",
     "MLCadSkipBeginRc",
     "MLCadSkipEndRc",
     "MLCadGroupRc",

     "PliBeginIgnRc",
     "PliBeginSub1Rc",
     "PliBeginSub2Rc",
     "PliEndRc",

     "PartBeginIgnRc",
     "PartEndRc",

     "BomBeginIgnRc",
     "BomEndRc",

     "ReserveSpaceRc",
     "PictureAsStep",

     "GroupRemoveRc",
     "RemoveGroupRc",
     "RemovePartRc",
     "RemoveNameRc",

     "SynthBeginRc",
     "SynthEndRc",

     "ResolutionRc",

     "IncludeRc",

     "NoStepRc",

     "EndOfFileRc",
};

#endif
