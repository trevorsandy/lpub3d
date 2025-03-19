
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
 * This file describes a set of classes that implement a parse tree for
 * all the meta-commands that LPub supports.  Action metas such as STEP,
 * ROTSTEP, CALLOUT BEGIN, etc. return special return codes.  Configuration
 * metas that imply no action, but specify data for later use, retain
 * the configuration information, and return a generic OK return code.
 *
 * The top of tree is the Meta class that is the interface to the traverse
 * function that walks the LDraw model hierarchy.  Meta also tracks
 * locations in files like topOfModel, bottomOfModel, bottomOfSteps,topOfRange,
 * bottomOfRange, topOfStep, bottomOfStep, etc.
 *
 * Please see lpub.h for an overall description of how the files in LPub3D
 * make up the LPub3D program.
 *
 ***************************************************************************/

#ifndef META_H
#define META_H

#include <QPageLayout>
#include <QString>
#include <QStringList>
#include <QList>
#include <QHash>
#include <QRegExp>
#include <float.h>
#include <QMessageBox>
#include "where.h"
#include "metatypes.h"
#include "resolution.h"
#include "declarations.h"

class Meta;
class BranchMeta;
class QStringList;

enum Rc {
         InvalidLDrawLineRc = -3,
         RangeErrorRc = -2,
         FailureRc = -1,
         EndOfIncludeFileRc = FailureRc,
         BuildModInvalidIndexRc = FailureRc,
         OkRc = 0,
         BuildModNoActionRc = OkRc,

         StepRc,
         RotStepRc,

         CalloutBeginRc,
         CalloutDividerRc,
         CalloutEndRc,

         StepGroupBeginRc,
         StepGroupDividerRc,
         StepGroupEndRc,

         InsertRc,
         InsertPageRc,
         InsertCoverPageRc,

         CalloutPointerRc,
         CalloutDividerPointerRc,
         CalloutPointerAttribRc,
         CalloutDividerPointerAttribRc,

         StepGroupDividerPointerRc,
         StepGroupPointerAttribRc,
         StepGroupDividerPointerAttribRc,

         PagePointerAttribRc,

         PagePointerRc,
         IllustrationPointerRc,

         AssemAnnotationIconRc,
         InsertFinalModelRc,
         InsertDisplayModelRc,

         SepRc,

         ClearRc,
         FadeRc,
         SilhouetteRc,
         ColourRc,
         BufferStoreRc,
         BufferLoadRc,
         MLCadSkipBeginRc,
         MLCadSkipEndRc,
         MLCadGroupRc,
         LDCadGroupRc,
         LeoCadModelRc,
         LeoCadPieceRc,
         LeoCadCameraRc,
         LeoCadLightRc,
         LeoCadLightTypeRc,  // Light NAME and TYPE written on same line
         LeoCadLightPOVRayRc,
         LeoCadLightShadowless,
         LeoCadSynthRc,
         LeoCadGroupBeginRc,
         LeoCadGroupEndRc,

         PliBeginIgnRc,
         PliBeginSub1Rc,
         PliBeginSub2Rc,
         PliBeginSub3Rc,
         PliBeginSub4Rc,
         PliBeginSub5Rc,
         PliBeginSub6Rc,
         PliBeginSub7Rc,
         PliBeginSub8Rc,

         PliEndRc,

         PartBeginIgnRc,
         PartEndRc,
         PartTypeRc,
         PartNameRc,

         BomBeginIgnRc,
         BomEndRc,

         BuildModBeginRc,
         BuildModEndModRc,
         BuildModEndRc,
         BuildModApplyRc,
         BuildModRemoveRc,
         BuildModEnableRc,

         FinalModelEnableRc,

         PreferredRendererCalloutAssemRc,
         PreferredRendererGroupAssemRc,
         PreferredRendererAssemRc,
         PreferredRendererSubModelRc,
         PreferredRendererPliRc,
         PreferredRendererBomRc,
         PreferredRendererRc,

         EnableFadeStepsCalloutAssemRc,
         EnableFadeStepsGroupAssemRc,
         EnableFadeStepsAssemRc,
         EnableFadeStepsRc,

         EnableHighlightStepCalloutAssemRc,
         EnableHighlightStepGroupAssemRc,
         EnableHighlightStepAssemRc,
         EnableHighlightStepRc,

         LPubFadeCalloutAssemRc,
         LPubFadeGroupAssemRc,
         LPubFadeAssemRc,
         LPubFadeRc,

         LPubHighlightCalloutAssemRc,
         LPubHighlightGroupAssemRc,
         LPubHighlightAssemRc,
         LPubHighlightRc,

         PageOrientationRc,
         PageSizeRc,

         ReserveSpaceRc,
         PictureAsStep,

         RemoveGroupRc,
         RemovePartTypeRc,
         RemovePartNameRc,

         SynthBeginRc,
         SynthEndRc,

         StepsPliPerStepRc,

         ResolutionRc,

         PliPartGroupRc,
         BomPartGroupRc,

         ContStepNumRc,
         CountInstanceRc,

         StartPageNumberRc,
         StartStepNumberRc,

         SceneItemZValueDirectionRc,

         IncludeRc,

         IncludeFileErrorRc,

         TexMapRc,

         InvalidLineRc,

         NoStepRc,

         EndOfFileRc,
};

enum DoubleSpinEnc {
  DoubleSpinDefault,
  DoubleSpinScale,
  DoubleSpinZPlane,
  DoubleSpinFOV,
  DoubleSpinDDF
};

// page pointer positions
enum Positions
{
  PP_TOP = 0,
  PP_BOTTOM,
  PP_LEFT,
  PP_RIGHT,
  PP_POSITIONS
};

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
    if (this != &rhs)
    {
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

/*
 * This abstract class is the root of all meta-command parsing
 * objects.  Each parsing object knows how to initialize itself,
 * parse from the current argv through the end of args, perform
 * a preamble match (used for recognizing meta commands when
 * making changes to the LDraw file, and it also knows how to
 * document itself.
 */
#if defined(_MSC_VER)
#pragma warning(disable:4458)   // disable declaration hides class member
#endif /*_MSC_VER */
class AbstractMeta
{
public:
  int       pushed;
  bool      global;
  QString   preamble;

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
  virtual  ~AbstractMeta() { preamble.clear(); }
  AbstractMeta(AbstractMeta&&rhs) = default;
  AbstractMeta& operator=(const AbstractMeta&rhs) = default;
  AbstractMeta& operator=(AbstractMeta&&rhs) = default;

  /* Initialize thyself */

  virtual void init(BranchMeta *parent, QString name);

  /* Parse thyself */

  virtual Rc parse(QStringList &argv, int index, Where &here) = 0;

  /* Compare argvs against matching string */

  virtual bool preambleMatch(QStringList &, int index, QString &match) = 0;

  /* Document thyself */
  virtual void doc(QStringList &out, QString preamble);

  /* Meta command*/
  virtual void metaKeywords(QStringList &out, QString preamble) { out << preamble; }

  /* Undo a push */

  virtual void pop() = 0;
};

/*------------------------*/

/*
 * This abstract class represents terminal nodes of a meta command
 */

class LeafMeta : public AbstractMeta
{
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
    if (argv.size() != index)
    {
    }
    return preamble == match;
  }

  virtual ~LeafMeta() = default;
  LeafMeta(LeafMeta&&rhs) = default;
  LeafMeta& operator=(const LeafMeta&rhs) = default;
  LeafMeta& operator=(LeafMeta&&rhs) = default;

  virtual QString format(bool local, bool global) = 0;
  virtual QString format(bool local, bool global, QString);
  virtual void doc(QStringList &out, QString preamble)  { out << preamble; }
  virtual void metaKeywords(QStringList &out, QString preamble)  { out << preamble; }
};

/*
 * This class represents non-terminal keywords in a syntax
 */

class BranchMeta : public AbstractMeta
{
public:
  /*
   * This is a list of the possible keywords for this token in
   * the syntax
   */
  QHash<QString, AbstractMeta *> list;
  BranchMeta() : AbstractMeta() {}
  virtual ~BranchMeta();

  virtual Rc parse(QStringList &argv, int index, Where &here);
  virtual bool preambleMatch(QStringList &argv, int index, QString &_preamble);
  virtual void doc(QStringList &out, QString preamble);
  virtual void metaKeywords(QStringList &out, QString preamble);
  virtual void pop();
  BranchMeta &operator= (const BranchMeta &rhs)
  {
    Q_FOREACH (const QString &key, list.keys())
    {
      *list[key] = *rhs.list[key];
    }
    preamble = rhs.preamble;
    return *this;
  }
  BranchMeta (const BranchMeta &rhs) : AbstractMeta(rhs)
  {
    Q_FOREACH (const QString &key, list.keys())
    {
      *list[key] = *rhs.list[key];
    }
  }
};

/*
 * This leaf parsing class returns a special return code, typically
 * used for action meta-commands like LPUB PLI BEGIN SUB, or LPUB
 * CALLOUT BEGIN
 */

class RcMeta : public LeafMeta
{
public:
  Rc rc;  // the return code
  RcMeta() : LeafMeta() {}
  RcMeta(const RcMeta &rhs) : LeafMeta(rhs)
  {
    rc = rhs.rc;
  }
  virtual ~RcMeta() = default;

  RcMeta(RcMeta&&rhs) = default;
  RcMeta& operator=(const RcMeta&rhs) = default;
  RcMeta& operator=(RcMeta&&rhs) = default;

  virtual void init(BranchMeta *parent, const QString name, Rc _rc=OkRc);
  virtual Rc parse(QStringList &argv, int index, Where &here);
  virtual QString format(bool,bool) { QString foo; return foo; }
  virtual void doc(QStringList &out, QString preamble);
  virtual void metaKeywords(QStringList &out, QString preamble);
};



/*
 * This leaf meta is used when the the rest of the input
 * is a simple integer
 */

class IntMeta : public RcMeta
{
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
  void setValue(int value)
  {
    _value[pushed] = value;
  }
  void setRange(int min, int max)
  {
    _min = min;
    _max = max;
  }
  virtual ~IntMeta() = default;

  IntMeta(IntMeta&&rhs) = default;
  IntMeta& operator=(const IntMeta&rhs) = default;
  IntMeta& operator=(IntMeta&&rhs) = default;

  virtual void init(BranchMeta *parent, const QString name, Rc _rc=OkRc);
  virtual Rc parse(QStringList &argv, int index, Where &here);
          QString format(bool,bool);
  virtual void doc(QStringList &out, QString preamble);
};
/*
 * This is a leaf object for floating point number */

class FloatMeta : public RcMeta
{
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
  virtual void init(BranchMeta *parent, const QString name, Rc _rc=OkRc);
  virtual Rc parse(QStringList &argv, int index, Where &here);
          QString format(bool,bool);
  virtual void doc(QStringList &out, QString preamble);
};

/* This is a leaf object class for two floating point numbers */

class FloatPairMeta : public RcMeta
{
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
    _value[0][0] = 0.0f;
    _value[0][1] = 0.0f;
    _min = 0;
    _max = 0;
    _fieldWidth = 6;
    _precision = 4;
    _inputMask = "9.9999";
    _default = true;
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
  virtual void    init(BranchMeta *parent, const QString name, Rc _rc=OkRc);
  virtual Rc parse(QStringList &argv, int index, Where &here);
  virtual QString format(bool,bool);
  virtual void doc(QStringList &out, QString preamble);
};

/* This is a leaf object class for X,Y and Z floating point numbers */

class Vector3Meta : public RcMeta
{
protected:
  float     _x[2];
  float     _y[2];
  float     _z[2];
  float     _min,_max;
  bool      _populated;
public:
  int       _fieldWidth;
  int       _precision;
  QString   _inputMask;
  Vector3Meta()
  {
    _x[0]       = 0.0f;
    _y[0]       = 0.0f;
    _z[0]       = 0.0f;
    _min        = -FLT_MAX;
    _max        = FLT_MAX;
    _fieldWidth = 6;
    _precision  = 4;
    _inputMask  = "###9.90";
    _populated  = false;
  }
  virtual float x()
  {
    return _x[pushed];
  }
  virtual float y()
  {
    return _y[pushed];
  }
  virtual float z()
  {
    return _z[pushed];
  }
  void setValues(float x, float y, float z)
  {
    _x[pushed] = x;
    _y[pushed] = y;
    _z[pushed] = z;
    _populated = x != 0.0f || y != 0.0f || z != 0.0f;
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
  bool isPopulated()
  {
    return _populated;
  }
  virtual void init(BranchMeta *parent, const QString name, Rc _rc=OkRc);
  virtual Rc parse(QStringList &argv, int index, Where &here);
  virtual QString format(bool,bool);
  virtual void doc(QStringList &out, QString preamble);
};

/* This is a leaf object class for X,Y and Z matrix (x1,x2,x3,y1,y2,y3,z1,z2,z3) floating point numbers */

class Vector33Meta : public RcMeta {
protected:
  float     _x1[2];
  float     _y1[2];
  float     _z1[2];

  float     _x2[2];
  float     _y2[2];
  float     _z2[2];

  float     _x3[2];
  float     _y3[2];
  float     _z3[2];

  float     _min,_max;
  bool      _populated;
public:
  int       _fieldWidth;
  int       _precision;
  QString   _inputMask;
  Vector33Meta()
  {
    _x1[0]      = 0.0f;
    _y1[0]      = 0.0f;
    _z1[0]      = 0.0f;

    _x2[0]      = 0.0f;
    _y2[0]      = 0.0f;
    _z2[0]      = 0.0f;

    _x3[0]      = 0.0f;
    _y3[0]      = 0.0f;
    _z3[0]      = 0.0f;

    _min        = -FLT_MAX;
    _max        = FLT_MAX;
    _fieldWidth = 6;
    _precision  = 4;
    _inputMask  = "###9.90";
    _populated  = false;
  }
  virtual float x1()
  {
    return _x1[pushed];
  }
  virtual float y1()
  {
    return _y1[pushed];
  }
  virtual float z1()
  {
    return _z1[pushed];
  }
  
  virtual float x2()
  {
    return _x2[pushed];
  }
  virtual float y2()
  {
    return _y2[pushed];
  }
  virtual float z2()
  {
    return _z2[pushed];
  }
  
  virtual float x3()
  {
    return _x3[pushed];
  }
  virtual float y3()
  {
    return _y3[pushed];
  }
  virtual float z3()
  {
    return _z3[pushed];
  }
  
  void setValues(float x1, float y1, float z1, float x2, float y2, float z2, float x3, float y3, float z3)
  {
    _x1[pushed] = x1;
    _y1[pushed] = y1;
    _z1[pushed] = z1;

    _x2[pushed] = x2;
    _y2[pushed] = y2;
    _z2[pushed] = z2;

    _x3[pushed] = x3;
    _y3[pushed] = y3;
    _z3[pushed] = z3;

    _populated = x1 != 0.0f || y1 != 0.0f || z1 != 0.0f ||
                 x2 != 0.0f || y2 != 0.0f || z2 != 0.0f ||
                 x3 != 0.0f || y3 != 0.0f || z3 != 0.0f;
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
  bool isPopulated()
  {
    return _populated;
  }  
  virtual void init(BranchMeta *parent,
                    const QString name,
                    Rc _rc=OkRc);
  virtual Rc parse(QStringList &argv, int index, Where &here);
  virtual QString format(bool,bool);
  virtual void doc(QStringList &out, QString preamble);
};

/* This leaf class is used for strings */

class StringMeta : public RcMeta
{
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
  virtual void init(BranchMeta *parent, QString name, Rc _rc=OkRc, QString _delim="\"");
  virtual Rc parse(QStringList &argv, int index, Where &here);
          QString format(bool,bool);
  void    pop()
  {
    if (pushed)
    {
      _value[1].clear();
      pushed = 0;
    }
  }

  virtual void doc(QStringList &out, QString preamble);
};

/* This leaf class is for multiple strings */

class StringListMeta : public RcMeta
{
public:
  QStringList _value[2];
  QString     delim;
  QString value(int i)
  {
    if (i >= _value[pushed].size())
    {
      i = _value[pushed].size() - 1;
    }
    if (i >= 0) {
      return _value[pushed][i];
    } else {
      return "";
    }
  }
  QStringList value()
  {
    return _value[pushed];
  }
  void setValue(QString value)
  {
    _value[pushed] << value;
  }
  void setValue(int pos, QString value)
  {
    if ((_value[pushed].size() >= pos) && (pos >= 0))
    {
      _value[pushed].removeAt(pos);
      _value[pushed].insert(pos, value);
    }
  }
  void setValue(QStringList value)
  {
    _value[pushed] = value;
  }
  StringListMeta() : RcMeta()
  {
  }
  virtual void init(BranchMeta *parent, QString name, Rc _rc=OkRc, QString _delim = "\"");
  virtual Rc parse(QStringList &argv, int index, Where &here);
  QString format(bool,bool);
  void    pop()
  {
    if (pushed)
    {
      _value[1].clear();
      pushed = 0;
    }
  }
  virtual void doc(QStringList &out, QString preamble);
};

/*
 * This leaf is to catch booleans (TRUE or FALSE)
 */

class BoolMeta : public RcMeta
{
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
    _value[0] = false;
    _value[1] = false;
  }
  Rc parse(QStringList &argv, int index, Where &here);
  QString format(bool,bool);
  virtual void doc(QStringList &out, QString preamble);
};

/*
 * This leaf meta is used when using real world measuring units
 */

class UnitMeta : public FloatMeta
{
public:
  virtual float value()  // the value as the user sees it
  {
    float t = _value[pushed];

    if (resolutionType() == DPCM)
    {
      t = inches2centimeters(t);
    }
    return t;
  }
  virtual void setValue(float value)
  {
    if (resolutionType() == DPCM)
    {
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

class UnitsMeta : public FloatPairMeta
{
private:
public:
  virtual float value(int which)
  {
    float t = _value[pushed][which];

    if (resolutionType() == DPCM)
    {
      t = inches2centimeters(t);
    }
    return t;
  }

  virtual void setValue(int which, float value)
  {
    if (resolutionType() == DPCM)
    {
      value = centimeters2inches(value);
    }
    _value[pushed][which] = value;
  }

  virtual void setValuesFoo(float v1, float v2)
  {
    if (resolutionType() == DPCM)
    {
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

  virtual void setValuePixels(int which, int pixels);

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
  virtual QString format(bool,bool);
};

/* ------------------ */
/*
 * This meta parses page size keywords
 */
class PageSizeMeta : public RcMeta
{
protected:
  PageSizeData _value[2];
  float        _min,_max;
  bool         _default;
public:
  int          _fieldWidth;
  int          _precision;
  QString      _inputMask;
  PageSizeMeta()
  {
    _value[0].sizeW       = 0.0f;
    _value[0].sizeH       = 0.0f;
    _value[0].sizeID      = QStringLiteral("A4");
    _value[0].orientation = Portrait;
    _value[1].sizeW       = 0.0f;
    _value[1].sizeH       = 0.0f;
    _value[1].sizeID      = QStringLiteral("A4");
    _value[1].orientation = Portrait;
    _min = 0;
    _max = 0;
    _fieldWidth = 6;
    _precision  = 4;
    _inputMask  = "9.9999";
    _default    = true;
  }
  // data
  virtual PageSizeData &value()
  {
    return _value[pushed];
  }
  virtual void setValue(PageSizeData &value)
  {
    _value[pushed] = value;
  }
  //size id
  virtual void setValueSizeID(QString v3)
  {
    _value[pushed].sizeID = v3;
    _default              = false;
  }
  virtual QString valueSizeID()
  {
    return _value[pushed].sizeID;
  }
  //orientation
  virtual OrientationEnc valueOrientation()
  {
    return _value[pushed].orientation;
  }
  virtual void setValueOrientation(OrientationEnc value)
  {
    _value[pushed].orientation = value;
  }
  //page size values
  virtual float value(int which)
  {
    float t = which ? _value[pushed].sizeH :
                      _value[pushed].sizeW;

    if (resolutionType() == DPCM)
    {
      t = inches2centimeters(t);
    }
    return t;
  }
  virtual void setValue(int which, float value)
  {
    if (resolutionType() == DPCM)
    {
        value = centimeters2inches(value);
    }
    if (which)
      _value[pushed].sizeH = value;
    else
      _value[pushed].sizeW = value;
    _default               = false;
  }
  virtual float valueInches(int which)
  {
    return which ? _value[pushed].sizeH :
                   _value[pushed].sizeW;
  }
  virtual void setValueInches(int which, float value)
  {
    if (which)
      _value[pushed].sizeH = value;
    else
      _value[pushed].sizeW = value;
    _default               = false;
  }
  virtual void setValuesInches(float v1, float v2)
  {
    _value[pushed].sizeW = v1;
    _value[pushed].sizeH = v2;
    _default             = false;
  }
  virtual void setValuePixels(int which, int pixels)
  {
    float r = resolution();
    float value = float(pixels/r);

    if (which)
      _value[pushed].sizeH = value;
    else
      _value[pushed].sizeW = value;
    _default               = false;
  }
  virtual void setValuesPixels(float v1pixels, float v2pixels)
  {
    float r = resolution();
    float v1 = float(v1pixels/r);
    float v2 = float(v2pixels/r);

    _value[pushed].sizeW  = v1;
    _value[pushed].sizeH  = v2;
    _value[pushed].sizeID = QStringLiteral("Custom");
    _default              = false;
  }
  virtual int valuePixels(int which)
  {
    float t = which ? _value[pushed].sizeH :
                      _value[pushed].sizeW;
    float r = resolution();

    return int(t*r);
  }
  // formatting
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
  virtual void init(BranchMeta *parent, const QString name, Rc _rc=OkRc);
  virtual Rc parse(QStringList &argv, int index, Where &here);
  virtual QString format(bool,bool);
  virtual void doc(QStringList &out, QString preamble);
};

/* ------------------ */

class MarginsMeta : public UnitsMeta
{
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
};

/* This leaf class is used for fonts */
class FontMeta : public StringMeta
{
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
    if (resolutionType() == DPI)
    {
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
};

/* This leaf class is used for fonts */

class FontListMeta : public StringListMeta {
private:
public:
  FontListMeta() : StringListMeta() {}
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
  void setValue(PlacementType value)
  {
    if (!_value[pushed].relativeOf.contains(value))
      _value[pushed].relativeOf.append(value);
  }
  void setValue(RectPlacement  placement, PlacementType relativeTo);
  void setValue(int loc[2])
  {
    _value[pushed].offsets[0] = loc[0];
    _value[pushed].offsets[1] = loc[1];
  }
  void setPartsListPerStep(bool b)
  {
    _value[pushed].partsListPerStep = b;
  }
  void setPartsListShown(bool b)
  {
    _value[pushed].partsListShown = b;
  }
  void setPageType(PageTypeEnc v)
  {
    _value[pushed].pageType = v;
  }
  void setStepNumberShown(bool b)
  {
    _value[pushed].stepNumberShown = b;
  }
  void setRotateIconShown(bool b)
  {
    _value[pushed].rotateIconShown = b;
  }
  void setSubModelShown(bool b)
  {
    _value[pushed].subModelShown = b;
  }
  PlacementMeta();
  void clear()
  {
    LeafMeta::clear();
    _value[0].clear();
    _value[1].clear();
  }
  Rc parse(QStringList &argv, int index, Where &here);
  QString format(bool,bool);
  virtual void doc(QStringList &out, QString preamble);
  virtual void metaKeywords(QStringList &out, QString preamble);
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
  Rc parse(QStringList &argv, int index, Where &here);
  QString format(bool,bool);
  void    pop()
  {
    if (pushed)
    {
      _value[1].string.clear();
      pushed = 0;
    }
  }
  virtual void doc(QStringList &out, QString preamble);
  virtual void metaKeywords(QStringList &out, QString preamble);
  virtual QString text();
};

/* This leaf class is used to parse border metas */

class BorderMeta : public LeafMeta
{
private:
  BorderData _value[2];  // some of this is in units
  BorderData _result;
public:
  void setValue(BorderData border)
  {
    if (resolutionType() == DPCM)
    {
      border.thickness = centimeters2inches(border.thickness);
      border.margin[0] = centimeters2inches(border.margin[0]);
      border.margin[1] = centimeters2inches(border.margin[1]);
    }
    _value[pushed] = border;
  }
  void setValue(
    BorderData::Border type,
    BorderData::Line line,
    QString string)
  {
    _value[pushed].type  = type;
    _value[pushed].line  = line;
    _value[pushed].color = string;
  }
  void setValueInches(BorderData &borderData)
  {
    _value[pushed] = borderData;
  }
  BorderData::Line setBorderLine(const QString argvIndex)
  {
    if (argvIndex == "0")
    {
        return BorderData::BdrLnNone;
    }
    else if (argvIndex == "1")
    {
      return BorderData::BdrLnSolid;
    }
    else if (argvIndex == "2")
    {
      return BorderData::BdrLnDash;
    }
    else if (argvIndex == "3")
    {
      return BorderData::BdrLnDot;
    }
    else if (argvIndex == "4")
    {
      return BorderData::BdrLnDashDot;
    }
    else
    { //"Dot-Dot-Dash Line"
      return BorderData::BdrLnDashDotDot;
    }
  }
  BorderData &value()
  {
    _result = _value[pushed];
    if (resolutionType() == DPCM)
    {
      _result.thickness = inches2centimeters(_result.thickness);
      _result.margin[0] = inches2centimeters(_result.margin[0]);
      _result.margin[1] = inches2centimeters(_result.margin[1]);
    }
    return _result;
  }
  BorderData valueInches()
  {
    return _value[pushed];
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
    _value[0].thickness = DEFAULT_LINE_THICKNESS;
    _value[0].margin[0] = 0;
    _value[0].margin[1] = 0;
    _value[1].thickness = DEFAULT_LINE_THICKNESS;
    _value[1].margin[0] = 0;
    _value[1].margin[1] = 0;
  }
  Rc parse(QStringList &argv, int index, Where &here);
  QString format(bool,bool);
  void pop()
  {
    if (pushed) {
      _value[1].color.clear();
      pushed = 0;
    }
  }
  virtual void doc(QStringList &out, QString preamble);
  virtual void metaKeywords(QStringList &out, QString preamble);
  virtual QString text();
};

/*------------------------*/
class CameraAnglesMeta  : public RcMeta
{
protected:
  CameraAnglesData _value[2];
  float            _min,_max;
  bool             _default;
public:
  int              _fieldWidth;
  int              _precision;
  QString          _inputMask;

  bool isDefault()
  {
    return _default;
  }

  CameraAnglesData &value()
  {
    return _value[pushed];
  }

  virtual float value(int which)
  {
    return _value[pushed].angles[which];
  }

  virtual bool customViewpoint()
  {
    return _value[pushed].customViewpoint;
  }

  virtual CameraViews::CameraView cameraView()
  {
    return _value[pushed].cameraView;
  }

  virtual int type()
  {
    return _value[pushed].type;
  }

  virtual void setValue(CameraAnglesData &value)
  {
    _value[pushed] = value;
    _default = false;
  }

  virtual void setCameraView(CameraViews::CameraView value)
  {
    _value[pushed].cameraView = value;
    _default = false;
  }

  virtual void setCustomViewpoint(bool value)
  {
    _value[pushed].customViewpoint = value;
    _default = false;
  }

  virtual void setType(int value)
  {
    _value[pushed].type = value;
    _default = false;
  }

  virtual void setValue(int which, float value)
  {
    _value[pushed].angles[which] = value;
    _default = false;
  }

  virtual void setValues(float value1, float value2)
  {
    _value[pushed].angles[0] = value1;
    _value[pushed].angles[1] = value2;
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

  CameraAnglesMeta()
  {
    _value[0].cameraView = CameraViews::CameraView::Default;
    _value[0].type      = 1; //Options::CSI
    _value[0].angles[0] = 0.0f;
    _value[0].angles[1] = 0.0f;
    _value[1].cameraView = CameraViews::CameraView::Default;
    _value[1].type      = 1; //Options::CSI
    _value[1].angles[0] = 0.0f;
    _value[1].angles[1] = 0.0f;
    _min = 0;
    _max = 0;
    _fieldWidth = 6;
    _precision  = 1;
    _inputMask  = "###9.9";
    _default    = true;
  }
  Rc parse(QStringList &argv, int index, Where &here);
  QString format(bool,bool);
  virtual void init(BranchMeta *parent, const QString name, Rc _rc=OkRc);
  virtual void doc(QStringList &out, QString preamble);
  virtual void metaKeywords(QStringList &out, QString preamble);
};

/*
 * This class parses pointer attribute
 * meta commands
 */

class PointerAttribMeta  : public LeafMeta
{
private:
  PointerAttribData _value[2];      // some of this is in units
  PointerAttribData _result;
public:

  PointerAttribData &valuePixels()
  {
    _result = _value[pushed];
    _result.borderData.thickness*=resolution();
    _result.lineData.thickness  *=resolution();
    _result.tipData.thickness   *=resolution();
    _result.tipData.tipWidth    *=resolution();
    _result.tipData.tipHeight   *=resolution();
    return _result;
  }

  PointerAttribData &value()
  {
    _result = _value[pushed];
    if (resolutionType() == DPCM) {
      _result.borderData.thickness = inches2centimeters(_result.borderData.thickness);
      _result.lineData.thickness   = inches2centimeters(_result.lineData.thickness);
      _result.tipData.thickness    = inches2centimeters(_result.tipData.thickness);
      _result.tipData.tipWidth     = inches2centimeters(_result.tipData.tipWidth);
      _result.tipData.tipHeight    = inches2centimeters(_result.tipData.tipHeight);
    }
    return _result;
  }

  void setValue(PointerAttribData pointerAttribData)
  {
    if (resolutionType() == DPCM) {
      pointerAttribData.borderData.thickness = centimeters2inches(pointerAttribData.borderData.thickness);
      pointerAttribData.lineData.thickness   = centimeters2inches(pointerAttribData.lineData.thickness);
      pointerAttribData.tipData.thickness    = centimeters2inches(pointerAttribData.tipData.thickness);
      pointerAttribData.tipData.tipWidth     = centimeters2inches(pointerAttribData.tipData.tipWidth);
      pointerAttribData.tipData.tipHeight    = centimeters2inches(pointerAttribData.tipData.tipHeight);
    }
    _value[pushed] = pointerAttribData;
  }

  void setOtherDataInches(PointerAttribData pointerAttribData);

  PointerAttribData valueInches()
  {
    return _value[pushed];
  }

  void setValueInches(PointerAttribData &pointerAttribData)
  {
    _value[pushed] = pointerAttribData;
  }

  BorderData::Line setBorderLine(const QString argvIndex)
  {
    if (argvIndex == "1") {
      return BorderData::BdrLnSolid;
      }
    else if (argvIndex == "2") {
      return BorderData::BdrLnDash;
      }
    else if (argvIndex == "3") {
      return BorderData::BdrLnDot;
      }
    else if (argvIndex == "4") {
      return BorderData::BdrLnDashDot;
      }
    else {//"Dot-Dot-Dash Line"
      return BorderData::BdrLnDashDotDot;
      }
  }

  void setWhere(Where &here)
  {
     _here[pushed] = here;
  }

  void setPointerId(int id)
  {
     _value[pushed].id = id;
  }

  void setDefaultColor(QString &color)
  {
    if (_value[pushed].attribType == PointerAttribData::Line) {
      if (_value[pushed].lineData.useDefault)
         _value[pushed].lineData.color = color;
    } else {
      if (_value[pushed].borderData.useDefault)
         _value[pushed].borderData.color = color;
    }
  }

  PointerAttribData &parseAttributes(const QStringList &argv, Where &here)
  {
    Rc rc;
    int arge = 0;
    return parseAttributes(argv, here, 0, arge, rc);
  }

  PointerAttribData &parseAttributes(const QStringList &argv, const Where &here, const int index, int &arge, Rc &rc);

  PointerAttribMeta() : LeafMeta()
  {
    _value[0].borderData.thickness = DEFAULT_BORDER_THICKNESS;
    _value[0].lineData.thickness   = DEFAULT_LINE_THICKNESS;
    _value[0].tipData.thickness    = DEFAULT_BORDER_THICKNESS;
    _value[0].tipData.tipWidth     = DEFAULT_TIP_WIDTH;
    _value[0].tipData.tipHeight    = DEFAULT_TIP_HEIGHT;
    _value[1].borderData.thickness = DEFAULT_BORDER_THICKNESS;
    _value[1].lineData.thickness   = DEFAULT_LINE_THICKNESS;
    _value[1].tipData.thickness    = DEFAULT_BORDER_THICKNESS;
    _value[1].tipData.tipWidth     = DEFAULT_TIP_WIDTH;
    _value[1].tipData.tipHeight    = DEFAULT_TIP_HEIGHT;
  }

  Rc parse(QStringList &argv, int index, Where &here);
  QString format(bool,bool);
  virtual void doc(QStringList &out, QString preamble);
  virtual void metaKeywords(QStringList &out, QString preamble);
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
    int segments,
    float x1,
    float y1,
    float x2,
    float y2,
    float x3,
    float y3,
    float x4,
    float y4)
  {
    _value[pushed].placement = placement;     //placementEnc
    _value[pushed].loc       = loc;
    _value[pushed].base      = base/resolution();
    _value[pushed].segments  = segments;
    _value[pushed].x1        = x1;            // TipX
    _value[pushed].y1        = y1;            // TipY
    _value[pushed].x2        = x2;            // BaseX
    _value[pushed].y2        = y2;            // BaseY
    _value[pushed].x3        = x3;            // MidBaseX
    _value[pushed].y3        = y3;            // MidBaseY
    _value[pushed].x4        = x4;            // MidTipX
    _value[pushed].y4        = y4;            // MidTipY
  }
  void setValueUnit(
    PlacementEnc placement,
    float loc,
    float base,
    int segments,
    float x1,
    float y1,
    float x2,
    float y2,
    float x3,
    float y3,
    float x4,
    float y4)
  {
    _value[pushed].placement = placement;
    _value[pushed].loc       = loc;
    _value[pushed].base      = base;
    _value[pushed].segments  = segments;
    _value[pushed].x1        = x1;
    _value[pushed].y1        = y1;
    _value[pushed].x2        = x2;
    _value[pushed].y2        = y2;
    _value[pushed].x3        = x3;
    _value[pushed].y3        = y3;
    _value[pushed].x4        = x4;
    _value[pushed].y4        = y4;
  }
  // used for page pointer - includes rectPlacement
  void setValuePage(
    RectPlacement rectPlacement,
    PlacementEnc placement,
    float loc,
    float base,
    int segments,
    float x1,
    float y1,
    float x2,
    float y2,
    float x3,
    float y3,
    float x4,
    float y4)
  {
    _value[pushed].rectPlacement = rectPlacement;
    _value[pushed].placement     = placement;
    _value[pushed].loc           = loc;
    _value[pushed].base          = base;
    _value[pushed].segments      = segments;
    _value[pushed].x1            = x1;
    _value[pushed].y1            = y1;
    _value[pushed].x2            = x2;
    _value[pushed].y2            = y2;
    _value[pushed].x3            = x3;
    _value[pushed].y3            = y3;
    _value[pushed].x4            = x4;
    _value[pushed].y4            = y4;
  }
  PointerMeta();
  Rc parse(QStringList &argv, int index, Where &here);
  QString format(bool,bool);
  virtual void doc(QStringList &out, QString preamble);
  virtual void metaKeywords(QStringList &out, QString preamble);
};

/*
 * This class detects preferred renderer keywords */

class PreferredRendererMeta : public LeafMeta
{
private:
  RendererData _value[2];
public:
  RendererData &value()
  {
    return _value[pushed];
  }
  void setValue(RendererData &data)
  {
    _value[pushed] = data;
  }
  void setPreferences(bool = false);
  PreferredRendererMeta();
  Rc parse(QStringList &argv, int index, Where &here);
  QString format(bool,bool);
  virtual void doc(QStringList &out, QString preamble);
  virtual void metaKeywords(QStringList &out, QString preamble);
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
  Rc parse(QStringList &argv, int index, Where &here);
  QString format(bool,bool);
  virtual void doc(QStringList &out, QString preamble);
  virtual void metaKeywords(QStringList &out, QString preamble);
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
  Rc parse(QStringList &argv, int index, Where &here);
  QString format(bool,bool);
  virtual void doc(QStringList &out, QString preamble);
  virtual void metaKeywords(QStringList &out, QString preamble);
};

/*
 * This class parses the Pixmap fill mode (Axpect|Stretch|Tile)
 */

class FillMeta : public LeafMeta
{
private:
  FillEnc type[2];
public:
  FillEnc value()
  {
    return type[pushed];
  }
  void setValue(FillEnc value)
  {
    type[pushed] = value;
  }
  FillMeta();
  Rc parse(QStringList &argv, int index, Where &here);
  QString format(bool,bool);
  virtual void doc(QStringList &out, QString preamble);
  virtual void metaKeywords(QStringList &out, QString preamble);
};

/*
 * This class parses the Stud Style Id
 */

class StudStyleMeta : public LeafMeta
{
private:
  StudStyleEnc type[2];
public:
  QHash<QString, int> studStyleMap;
  int value()
  {
    return StudStyleEnc(type[pushed]);
  }
  void setValue(int value)
  {
    type[pushed] = StudStyleEnc(value);
  }
  StudStyleMeta();
  Rc parse(QStringList &argv, int index, Where &here);
  QString format(bool,bool);
  virtual void doc(QStringList &out, QString preamble);
  virtual void metaKeywords(QStringList &out, QString preamble);
};

/*
 * This is the class that parses an RGBA color string
 */

class ColorMeta : public LeafMeta
{
private:
  int _format;
  quint32 _value[2];
public:
  quint32 value()
  {
    return _value[pushed];
  }
  quint32 validValue(const QString& defaultString)
  {
      return (value()) ? value() : getRGBAFromString(defaultString);
  }
  QString stringValue()
  {
    return getRGBAString(_value[pushed]);
  }
  QString hexRgbValue()
  {
      return QColor(_value[pushed]).name(QColor::HexRgb);
  }
  QString validStringValue(const QString& defaultString)
  {
      return value() ? getRGBAString(value()) : defaultString;
  }
  void setValue(int value)
  {
    _value[pushed] = quint32(value);
  }
  void setStringValue(const QString& value)
  {
    _value[pushed] = getRGBAFromString(value);
  }

  static void getRGBA(quint32 color, int& r, int& g, int& b, int& a);
  static quint32 getRGBA(int r, int g, int b, int a);
  static quint32 getRGBAFromString(const QString& value);
  static QString getRGBAString(quint32 rgba);
  ColorMeta();
  Rc parse(QStringList &argv, int index, Where &here);
  QString format(bool,bool);
  virtual void doc(QStringList &out, QString preamble);
  virtual void metaKeywords(QStringList &out, QString preamble);
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
  InsertData value()
  {
    return _value;
  }
  void setValue(InsertData &value)
  {
    _value = value;
  }
  void initPlacement();
  Rc parse(QStringList &argv, int index, Where &here);
  QString format(bool,bool);
  virtual void doc(QStringList &out, QString preamble);
  virtual void metaKeywords(QStringList &out, QString preamble);
};


/*
 * This class parses the Merge Submodel Instance flag (CountAtTop|CountAtModel|CountAtStep|CountTrue|CountFalse
 */

class CountInstanceMeta : public LeafMeta
{
private:
  CountInstanceEnc type[2];
public:
  QHash<QString, int> countInstanceMap;
  int value()
  {
    return CountInstanceEnc(type[pushed]);
  }
  void setValue(int value)
  {
    type[pushed] = CountInstanceEnc(value);
  }
  CountInstanceMeta();
  Rc parse(QStringList &argv, int index, Where &here);
  QString format(bool,bool);
  virtual void doc(QStringList &out, QString preamble);
  virtual void metaKeywords(QStringList &out, QString preamble);
};

/*
 * This class parses the Continuous Step Number flag (ContStepNumTrue|ContStepNumFalse)
 */
class ContStepNumMeta : public LeafMeta
{
private:
  ContStepNumEnc type[2];
public:
  QHash<QString, int> contStepNumMap;
  int value()
  {
    return ContStepNumEnc(type[pushed]);
  }
  void setValue(int value)
  {
    type[pushed] = ContStepNumEnc(value);
  }
  ContStepNumMeta();
  Rc parse(QStringList &argv, int index, Where &here);
  QString format(bool,bool);
  virtual void doc(QStringList &out, QString preamble);
  virtual void metaKeywords(QStringList &out, QString preamble);
};

/*
 * This class parses the BuildModEnabled flag (BuildModEnabledTrue|BuildModEnabledFalse)
 */
class BuildModEnabledMeta : public LeafMeta
{
private:
  BuildModEnabledEnc type[2];
public:
  QHash<QString, int> buildModEnabledMap;
  int value()
  {
    return BuildModEnabledEnc(type[pushed]);
  }
  void setValue(int value)
  {
    type[pushed] = BuildModEnabledEnc(value);
  }
  BuildModEnabledMeta();
  Rc parse(QStringList &argv, int index, Where &here);
  QString format(bool,bool);
  virtual void doc(QStringList &out, QString preamble);
  virtual void metaKeywords(QStringList &out, QString preamble);
};

/*
 * This class parses the FinalModelEnabled flag (FinalModelEnabledTrue|FinalModelEnabledFalse)
 */
class FinalModelEnabledMeta : public LeafMeta
{
private:
  FinalModelEnabledEnc type[2];
public:
  QHash<QString, int> finalModelEnabledMap;
  int value()
  {
    return FinalModelEnabledEnc(type[pushed]);
  }
  void setValue(int value)
  {
    type[pushed] = FinalModelEnabledEnc(value);
  }
  FinalModelEnabledMeta();
  Rc parse(QStringList &argv, int index, Where &here);
  QString format(bool,bool);
  virtual void doc(QStringList &out, QString preamble);
  virtual void metaKeywords(QStringList &out, QString preamble);
};


class AlignmentMeta : public LeafMeta
{
private:
  Qt::Alignment _value[2];
public:
  AlignmentMeta()
  {
    _value[0] = Qt::AlignLeft;
    _value[1] = Qt::AlignLeft;
  }
  Rc parse(QStringList &argv, int index, Where &here);
  QString format(bool,bool);
  virtual void doc(QStringList &out, QString preamble);
  virtual void metaKeywords(QStringList &out, QString preamble);
};

class ArrowHeadMeta : public LeafMeta
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
  virtual void metaKeywords(QStringList &out, QString preamble);
};

class ArrowEndMeta : public LeafMeta
{
  bool _value[2]; // false = square, true = round
public:
  ArrowEndMeta() : LeafMeta()
  {
    _value[0] = false;
    _value[1] = false;
  }
  ArrowEndMeta(const ArrowEndMeta &rhs) : LeafMeta(rhs)
  {
    _value[0] = rhs._value[0];
    _value[1] = rhs._value[1];
  }

  Rc parse(QStringList &argv, int index, Where &here);
  QString format(bool,bool);
  virtual void doc(QStringList &out, QString preamble);
  virtual void metaKeywords(QStringList &out, QString preamble);
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
      _result.length    = inches2centimeters(_result.length);
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
  void setValue(QString color,
                SepData::LengthType type,
                float length,
                float thickness,
                float margin0,
                float margin1)
  {
    if (resolutionType() == DPCM) {
      length    = centimeters2inches(length);
      thickness = centimeters2inches(thickness);
      margin0   = centimeters2inches(margin0);
      margin1   = centimeters2inches(margin1);
    }
    _value[pushed].color     = color;
    _value[pushed].type      = type;
    _value[pushed].length    = length;
    _value[pushed].thickness = thickness;
    _value[pushed].margin[0] = margin0;
    _value[pushed].margin[1] = margin1;
  }
  void setValue(SepData goods)
  {
    if (resolutionType() == DPCM) {
      goods.length    = centimeters2inches(goods.length);
      goods.thickness = centimeters2inches(goods.thickness);
      goods.margin[0] = centimeters2inches(goods.margin[0]);
      goods.margin[1] = centimeters2inches(goods.margin[1]);
    }
    _value[pushed] = goods;
  }
  SepData &valuePixels()
  {
    _result = _value[pushed];
    _result.length    *= resolution();
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
    _value[pushed].color     = color;
    _value[pushed].thickness = thickness;
    _value[pushed].margin[0] = margin0;
    _value[pushed].margin[1] = margin1;
  }
  void setValueInches(QString color,
                SepData::LengthType type,
                float length,
                float thickness,
                float margin0,
                float margin1)
  {
    _value[pushed].color     = color;
    _value[pushed].type      = type;
    _value[pushed].length    = length;
    _value[pushed].thickness = thickness;
    _value[pushed].margin[0] = margin0;
    _value[pushed].margin[1] = margin1;
  }
  void setValueInches(SepData &sepData)
  {
    _value[pushed] = sepData;
  }
  SepMeta();
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
  virtual void metaKeywords(QStringList &out, QString preamble);
};

/*
 * This class sets the justification for steps in a step group
 */

class JustifyStepMeta : public LeafMeta
{
private:
  JustifyStepData _value[2];
  JustifyStepData _result;
public:
  JustifyStepData &value()
  {
    _result = _value[pushed];
    if (resolutionType() == DPCM) {
      _result.spacing = inches2centimeters(_value[pushed].spacing);
    }
    return _result;
  }
  int spacingValuePixels()
  {
    float spacing = _value[pushed].spacing;
    return int(spacing*resolution());
  }
  void setValue(JustifyStepData &value)
  {
    float spacing = value.spacing;
    if (resolutionType() == DPCM) {
      value.spacing = centimeters2inches(spacing);
    }
    _value[pushed] = value;
  }
  JustifyStepMeta();
  Rc parse(QStringList &argv, int index, Where &here);
  QString format(bool,bool);
  virtual void doc(QStringList &out, QString preamble);
  virtual void metaKeywords(QStringList &out, QString preamble);
};

/*
 * This class parses (Portrait|Landscape)
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
  Rc parse(QStringList &argv, int index, Where &here);
  QString format(bool,bool);
  virtual void doc(QStringList &out, QString preamble);
  virtual void metaKeywords(QStringList &out, QString preamble);
};

/* ------------------ */
/*
 * Scene Depth Meta
 */

class SceneObjectMeta : public LeafMeta
{
private:
  SceneObjectData _value[2];
public:
  SceneObjectData &value()
  {
    return _value[pushed];
  }
  void setValue(SceneObjectData &value)
  {
    _value[pushed] = value;
  }
  void setItemObj(int value)
  {
    _value[0].itemObj = value;
    _value[1].itemObj = value;
  }
  int itemObj()
  {
    return _value[pushed].itemObj;
  }
  bool isArmed()
  {
    return _value[pushed].armed;
  }
  float posX()
  {
    return _value[pushed].scenePos[0];
  }
  float posY()
  {
    return _value[pushed].scenePos[1];
  }
  SceneObjectDirection direction()
  {
    return _value[pushed].direction;
  }
  SceneObjectMeta() : LeafMeta()
  {
  }
  Rc parse(QStringList &argv, int index, Where &here);
  QString format(bool,bool);
  void    pop() { pushed = 0; }
  virtual void doc(QStringList &out, QString preamble);
  virtual void metaKeywords(QStringList &out, QString preamble);
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
  void clearAttributes()
  {
    _value.attrs.clear();
    _value.type = 0;
  }
  Rc parse(QStringList &argv, int index, Where &here);
  QString format(bool,bool);
  virtual void doc(QStringList &out, QString preamble);
  virtual void metaKeywords(QStringList &out, QString preamble);
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
  ConstrainData &valueUnit()
  {
    return _value[pushed];
  }
  ConstrainData &value()
  {
    _result = _value[pushed];
    switch (_result.type)
    {
    case ConstrainData::PliConstrainWidth:
      _result.constraint.width *= resolution();
      break;
    case ConstrainData::PliConstrainHeight:
      _result.constraint.height *= resolution();
      break;
    default:
      break;
    }
    return _result;
  }
  void setValue(ConstrainData &value)
  {
    _default = false;
    switch (value.type)
    {
    case ConstrainData::PliConstrainWidth:
      value.constraint.width /= resolution();
      break;
    case ConstrainData::PliConstrainHeight:
      value.constraint.height /= resolution();
      break;
    default:
      break;
    }
    _value[pushed] = value;
  }
  void setValueUnit(ConstrainData &value)
  {
    _default = false;
    _value[pushed] = value;
  }
  ConstrainMeta();
  Rc parse(QStringList &argv, int index, Where &here);
  QString format(bool,bool);
  virtual void doc(QStringList &out, QString preamble);
  virtual void metaKeywords(QStringList &out, QString preamble);
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
  bool isPopulated()
  {
    return _value.populated;
  }
  void clear()
  {
    _value.populated = false;
    _value.type.clear();
    _value.rots[0] = 0;
    _value.rots[1] = 0;
    _value.rots[2] = 0;
  }
  RotStepMeta()
  {
    _value.type.clear();
    _value.populated = false;
  }
  RotStepMeta(const RotStepMeta &rhs) : LeafMeta(rhs)
  {
    _value      = rhs.value();
  }
  void setValue(RotStepData &rhs)
  {
    _value.populated = rhs.rots[0] != 0 ||
                       rhs.rots[1] != 0 ||
                       rhs.rots[2] != 0 ||
                       rhs.type    != _value.type;
    _value.type      = rhs.type;
    _value.rots[0]   = rhs.rots[0];
    _value.rots[1]   = rhs.rots[1];
    _value.rots[2]   = rhs.rots[2];
  }
  RotStepMeta& operator=(const RotStepMeta &rhs)
  {
    LeafMeta::operator=(rhs);
    _value      = rhs.value();
    return *this;
  }
  Rc parse(QStringList &argv, int index, Where &here);
  QString format(bool,bool);
  virtual void doc(QStringList &out, QString preamble);
  virtual void metaKeywords(QStringList &out, QString preamble);
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
    _value = rhs._value;
  }

  virtual ~BuffExchgMeta() { }
  Rc parse(QStringList &argv, int index, Where &here);
  QString format(bool,bool);
  void    pop() { pushed = 0; }
  virtual void doc(QStringList &out, QString preamble);
  virtual void metaKeywords(QStringList &out, QString preamble);
};

/* This class is parses the buildMod action */

class BuildModMeta : public LeafMeta
{
private:
  BuildModData _value;
public:
  QString key()
  {
    return _value.buildModKey;
  }
  Rc action()
  {
    return static_cast<Rc>(_value.action);
  }
  BuildModData value()
  {
    return _value;
  }
  void setValue(BuildModData &value)
  {
    _value = value;
  }
  BuildModMeta()
  {
    _value.action = 0;
  }
  Rc parse(QStringList &argv, int index, Where &here);
  QString format(bool,bool);
  virtual void doc(QStringList &out, QString preamble);
  virtual void metaKeywords(QStringList &out, QString preamble);
};

/*------------------------*/

class PliPartGroupMeta : public LeafMeta
{
private:
  PliPartGroupData _value;
public:
  PliPartGroupMeta() : LeafMeta()
  {
  }
  PliPartGroupData value()
  {
    return _value;
  }
  bool bom()
  {
    return _value.bom;
  }
  bool bomPart()
  {
    return _value.bPart;
  }
  QString key()
  {
    return _value.type + "_" + _value.color;
  }
  QPointF offset()
  {
    return QPointF(double(_value.offset[0]),
                   double(_value.offset[1]));
  }
  void setValue(PliPartGroupData &value)
  {
    _value = value;
  }
  void setBom (bool b)
  {
    _value.bom = b;
  }
  void setBomPart(bool b)
  {
    _value.bPart = b;
  }
  void setKey(QString &_type, QString &_color) {
    _value.type  = _type;
    _value.color = _color;
  }
  void setWhere(Where &here)
  {
    _here[0] = here;
  }
  void setOffset(QPointF offset)
  {
    _value.offset[0] += offset.x();
    _value.offset[1] += offset.y();
  }
  Rc parse(QStringList &argv, int index, Where &here);
  QString format(bool,bool);
  virtual void doc(QStringList &out, QString preamble);
  virtual void metaKeywords(QStringList &out, QString preamble);
};

/*
 * This class parses the CSI annotation
 */
 class CsiAnnotationIconMeta : public LeafMeta
{
private:
  CsiAnnotationIconData _value[2];
public:
  CsiAnnotationIconData &value()
  {
    return _value[pushed];
  }
  void setValue(CsiAnnotationIconData &rhs)
  {
    _value[pushed] = rhs;
  }
  CsiAnnotationIconMeta()
  {
  }
  Rc parse(QStringList &argv, int index, Where &here);
  QString format(bool,bool);
  virtual void doc(QStringList &out, QString preamble);
  virtual void metaKeywords(QStringList &out, QString preamble);
};

 /*------------------------*/

 /*
  * Resolution meta
  */

 class ResolutionMeta : public LeafMeta {
public:
  bool isDefault()
  {
    return getIsDefaultResolution();
  }
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
    setIsDefaultResolution(false);
  }
  void setValue(ResolutionType _type, float _resolution)
  {
    setResolutionType(_type);
    setResolution(_resolution);
    setIsDefaultResolution(false);
  }
  float ldu()
  {
    return 1.0/64;
  }
  ResolutionMeta()
  {
    setIsDefaultResolution(true);
  }
  void pop() { pushed = 0; setIsDefaultResolution(true); }
  virtual void init(BranchMeta *parent, QString name);
  virtual Rc parse(QStringList &argv, int index, Where &here);
  virtual QString format(bool,bool);
  virtual void doc(QStringList &out, QString preamble);
  virtual void metaKeywords(QStringList &out, QString preamble);
 };

/*
 *
 * This leaf is to catch fade and highlight steps booleans (TRUE or FALSE) and optional RESET flag
 *
 */

class EnableMeta : public RcMeta {
private:
  bool _value[2];
public:
  bool value()
  {
    return _value[pushed];
  }
  void setValue(bool value)
  {
    _value[pushed] = value;
  }
  EnableMeta ()
  {
    _value[0] = true;
    _value[1] = true;
  }
  Rc parse(QStringList &argv, int index, Where &here);
  QString format(bool,bool);
  virtual void doc(QStringList &out, QString preamble);
  virtual void metaKeywords(QStringList &out, QString preamble);
};

/*------------------------*/

/*
 *
 * This leaf is to catch true fade boolean (TRUE or FALSE) flag
 *
 */

class LPubFaHiMeta : public RcMeta {
  private:
  bool _value[2];
  public:
  bool value()
  {
    return _value[pushed];
  }
  void setValue(bool value)
  {
    _value[pushed] = value;
  }
  LPubFaHiMeta ()
  {
    _value[0] = false;
    _value[1] = false;
  }
  Rc parse(QStringList &argv, int index, Where &here);
  QString format(bool,bool);
  virtual void doc(QStringList &out, QString preamble);
  virtual void metaKeywords(QStringList &out, QString preamble);
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
  Rc parse(QStringList &argv, int index, Where &here);
  QString format(bool,bool);
  virtual void doc(QStringList &out, QString preamble);
  virtual void metaKeywords(QStringList &out, QString preamble);
};

class FadeColorMeta : public RcMeta {
private:
  FadeColorData _value[2];
public:
  FadeColorData &value()
  {
    return _value[pushed];
  }
  void setValue(FadeColorData &value)
  {
    _value[pushed] = value;
  }
  FadeColorMeta ()
  {
  }
  Rc parse(QStringList &argv, int index, Where &here);
  QString format(bool,bool);
  virtual void doc(QStringList &out, QString preamble);
  virtual void metaKeywords(QStringList &out, QString preamble);
};

class NoStepMeta : public RcMeta
{
public:
  NoStepMeta()
  {
  }
  virtual Rc parse(QStringList &argv, int index, Where &here);
          QString format(bool,bool);
  virtual void    doc(QStringList &out, QString preamble);
};

/*
 * non-terminal meta command keywords classes
 */

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

class AutoEdgeColorMeta : public BranchMeta
{
public:
  FloatMeta contrast;
  FloatMeta saturation;
  BoolMeta  enable;

  AutoEdgeColorMeta();
  AutoEdgeColorMeta(const AutoEdgeColorMeta &rhs) : BranchMeta(rhs)
  {
  }

  virtual ~AutoEdgeColorMeta() {}
  virtual void init(BranchMeta *parent, QString name);
};

/*------------------------*/

class HighContrastColorMeta : public BranchMeta
{
public:
  FloatMeta lightDarkIndex;
  ColorMeta studCylinderColor;
  BoolMeta  studCylinderColorEnabled;
  ColorMeta partEdgeColor;
  BoolMeta  partEdgeColorEnabled;
  ColorMeta blackEdgeColor;
  BoolMeta  blackEdgeColorEnabled;
  ColorMeta darkEdgeColor;
  BoolMeta  darkEdgeColorEnabled;

  HighContrastColorMeta();
  HighContrastColorMeta(const HighContrastColorMeta &rhs) : BranchMeta(rhs)
  {
  }

  virtual ~HighContrastColorMeta() {}
  virtual void init(BranchMeta *parent, QString name);
};

/*------------------------*/
/*
 * Fade Steps Meta
 */
class FadeStepsMeta : public BranchMeta
{
public:
  EnableMeta    enable;
  LPubFaHiMeta  lpubFade;
  BoolMeta      setup;
  FadeColorMeta color;
  IntMeta       opacity;
  void setPreferences(bool reset = false);
  FadeStepsMeta();
  virtual void init(BranchMeta *parent, QString name);
};

/*------------------------*/

/*
 * Highlight Step Meta
 */
class HighlightStepMeta : public BranchMeta
{
public:
  EnableMeta   enable;
  LPubFaHiMeta lpubHighlight;
  BoolMeta     setup;
  StringMeta   color;
  IntMeta      lineWidth;
  void setPreferences(bool reset = false);
  HighlightStepMeta();
  virtual void init(BranchMeta *parent, QString name);
};

/* ------------------ */

/*
 * For step group and called out metas
 */
class SettingsMeta : public BranchMeta
{
public:
  PlacementMeta        placement;
  MarginsMeta          margin;

  // stud style and automate edge colour
  StudStyleMeta         studStyle;
  AutoEdgeColorMeta     autoEdgeColor;
  HighContrastColorMeta highContrast;

  // fade, highlight and preferred renderer
  FadeStepsMeta         fadeSteps;
  HighlightStepMeta     highlightStep;
  PreferredRendererMeta preferredRenderer;

  // image scale
  FloatMeta            modelScale;

  // shared camera settings
  FloatMeta            cameraDistance;
  CameraAnglesMeta     cameraAngles;
  BoolMeta             isOrtho;
  FloatPairMeta        imageSize;
  StringMeta           cameraName;
  FloatMeta            cameraFoV;
  FloatMeta            cameraZNear;
  FloatMeta            cameraZFar;
  Vector3Meta          target;
  Vector3Meta          position;
  Vector3Meta          upvector;

  void resetCameraFoV();
  SettingsMeta();
  virtual void init(BranchMeta *parent, QString name);
};

/*------------------------*/

class StepsPliMeta : public BranchMeta
{
public:
  PlacementMeta placement;
  MarginsMeta   margin;
  BoolMeta      perStep;
  StepsPliMeta();
  virtual void init(BranchMeta *parent, QString name);
};

/*------------------------*/

class StepPliMeta : public BranchMeta
{
public:
  BoolMeta        perStep;
  StepPliMeta();
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
  virtual void init(BranchMeta *parent, QString name);
};

class NumberPlacementMeta : public NumberMeta
{
public:
  PlacementMeta  placement;
  NumberPlacementMeta();
  virtual void init(BranchMeta *parent, QString name);
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
  virtual void init(BranchMeta *parent, QString name);
};

/*------------------------*/

class PageAttributeImageMeta : public BranchMeta
{
public:
  PlacementType type;
  PlacementMeta placement;
  BorderMeta    border;
  MarginsMeta   margin;
  FloatMeta     picScale;
  StringMeta    file;
  BoolMeta      display;
  FillMeta      fill;

  void setValue(QString _value)
  {
      file.setValue(_value);
  }
  QString value()
  {
      return file.value();
  }
  PageAttributeImageMeta();
  virtual void init(BranchMeta *parent, QString name);
};

/*------------------------*/

class PageHeaderMeta : public BranchMeta
{
  public:
  UnitsMeta      size;
  PlacementMeta  placement;

  PageHeaderMeta();
  virtual void init(BranchMeta *parent, QString name);
};

/*------------------------*/

class PageFooterMeta : public BranchMeta
{
  public:
  UnitsMeta      size;
  PlacementMeta  placement;

  PageFooterMeta();
  virtual void init(BranchMeta *parent, QString name);
};

/*------------------------*/
/*
* Scene Object Meta
*/
class SceneItemMeta : public BranchMeta
{
public:
 SceneObjectMeta     assemAnnotation;       // CsiAnnotationType
 SceneObjectMeta     assemAnnotationPart;   // CsiPartType
 SceneObjectMeta     assem;                 // CsiType
 SceneObjectMeta     calloutBackground;     // CalloutType
 SceneObjectMeta     calloutInstance;       //
 SceneObjectMeta     calloutPointer;        //
 SceneObjectMeta     calloutUnderpinning;   //
 SceneObjectMeta     dividerBackground;     //
 SceneObjectMeta     divider;               //
 SceneObjectMeta     dividerLine;           //
 SceneObjectMeta     dividerPointer;        //
 SceneObjectMeta     pointerGrabber;        //
 SceneObjectMeta     pliGrabber;            //
 SceneObjectMeta     submodelGrabber;       //
 SceneObjectMeta     insertPicture;         //
 SceneObjectMeta     insertText;            //
 SceneObjectMeta     multiStepBackground;   // StepGroupType
 SceneObjectMeta     multiStepsBackground;  //
 SceneObjectMeta     pageAttributePixmap;   //
 SceneObjectMeta     pageAttributeText;     //
 SceneObjectMeta     pageBackground;        // PageType
 SceneObjectMeta     pageNumber;            // PageNumberType
 SceneObjectMeta     pagePointer;           // PagePointerType
 SceneObjectMeta     partsListAnnotation;   //
 SceneObjectMeta     partsListBackground;   // PartsListType
 SceneObjectMeta     partsListInstance;     //
 SceneObjectMeta     pointerFirstSeg;       //
 SceneObjectMeta     pointerHead;           //
 SceneObjectMeta     pointerSecondSeg;      //
 SceneObjectMeta     pointerThirdSeg;       //
 SceneObjectMeta     rotateIconBackground;  // RotateIconType
 SceneObjectMeta     reserveBackground;     // ReserveType
 SceneObjectMeta     stepNumber;            // StepNumberType
 SceneObjectMeta     subModelBackground;    // SubModelType
 SceneObjectMeta     subModelInstance;      //
 SceneObjectMeta     submodelInstanceCount; // SubmodelInstanceCountType
 SceneObjectMeta     partsListPixmap;       //
 SceneObjectMeta     partsListGroup;        //
 SceneObjectMeta     stepBackground;        //
 SceneItemMeta();
  virtual void init(BranchMeta *parent, QString name);
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
  virtual void init(BranchMeta *parent, QString name);
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
  virtual void init(BranchMeta *parent, QString name);
};

/*------------------------*/

class AnnotationStyleMeta : public BranchMeta
{
public:
  MarginsMeta     margin;
  BorderMeta      border;
  BackgroundMeta  background;
  FontMeta        font;
  StringMeta      color;
  UnitsMeta       size;
  IntMeta         style;

  AnnotationStyleMeta();
  virtual void init(BranchMeta *parent, QString name);
};

/*------------------------*/

class PliSortOrderMeta : public BranchMeta
{
public:
  StringMeta      primary;
  StringMeta      secondary;
  StringMeta      tertiary;
  StringMeta      primaryDirection;
  StringMeta      secondaryDirection;
  StringMeta      tertiaryDirection;

  PliSortOrderMeta();
  virtual void init(BranchMeta *parent, QString name);
};

class PliSortMeta : public BranchMeta // DEPRECATED
{
public:
  StringMeta sortOption;

  void setValue(QString _value)
  {
    sortOption.setValue(_value);
  }
  QString value()
  {
    return sortOption.value();
  }
  PliSortMeta();
  virtual void init(BranchMeta *parent, QString name);
};

/*------------------------*/

class PliPartElementMeta : public BranchMeta
{
public:
  BoolMeta      display;
  BoolMeta      legoElements;
  BoolMeta      bricklinkElements;
  BoolMeta      userElements;
  BoolMeta      userElementsLDrawKey;
  BoolMeta      localElements;

  PliPartElementMeta();
  virtual void init(BranchMeta *parent, QString name);
};

/*------------------------*/

class PliAnnotationMeta : public BranchMeta
{
public:
  BoolMeta      titleAnnotation;            // title
  BoolMeta      freeformAnnotation;         // freeform
  BoolMeta      titleAndFreeformAnnotation; // title and freeform
  BoolMeta      fixedAnnotations;           // fixed annotaitons
  BoolMeta      display;                    // global display
  BoolMeta      enableStyle;
  BoolMeta      axleStyle;
  BoolMeta      beamStyle;
  BoolMeta      cableStyle;
  BoolMeta      connectorStyle;
  BoolMeta      elementStyle;
  BoolMeta      extendedStyle;
  BoolMeta      hoseStyle;
  BoolMeta      panelStyle;

  PliAnnotationMeta();
  virtual void init(BranchMeta *parent, QString name);
};

/*------------------------*/

class CsiAnnotationMeta  : public BranchMeta
{
public:
  PlacementMeta         placement;
  BoolMeta              display;
  BoolMeta              axleDisplay;
  BoolMeta              beamDisplay;
  BoolMeta              cableDisplay;
  BoolMeta              connectorDisplay;
  BoolMeta              extendedDisplay;
  BoolMeta              hoseDisplay;
  BoolMeta              panelDisplay;
  CsiAnnotationIconMeta icon;

  CsiAnnotationMeta();
  virtual void init(BranchMeta *parent, QString name);
};

/*------------------------*/

class CsiPartMeta : public BranchMeta
{
  public:
  UnitsMeta     loc;
  UnitsMeta     size;
  MarginsMeta   margin;
  PlacementMeta placement;

  CsiPartMeta();
  virtual void init(BranchMeta *parent, QString name);
};

/*------------------------*/

class LoadUnoffPartsMeta : public BranchMeta
{
public:
  BoolMeta enableSetting;
  BoolMeta enabled;

  LoadUnoffPartsMeta();
  virtual void init(BranchMeta *parent, QString name);
};

/*------------------------*/

class ShowSubModelMeta : public BranchMeta
{
public:
  PlacementMeta placement;
  MarginsMeta   margin;
  BoolMeta      show;
  void setPreferences()
  {
      Preferences::showSubmodelInCallout = show.value();
  }
  ShowSubModelMeta();
  virtual void init(BranchMeta *parent, QString name);
};

/*------------------------*/

class RotateIconMeta  : public BranchMeta
{
public:
  UnitsMeta          size;
  FloatMeta          picScale;
  BorderMeta         arrow;
  BorderMeta         border;
  BackgroundMeta     background;
  MarginsMeta        margin;
  PlacementMeta      placement;
  BoolMeta           display;
  StringListMeta     subModelColor;

  RotateIconMeta();
  virtual void init(BranchMeta *parent, QString name);
};

/*------------------------*/

/*
 * PagePointer meta
 */
class PointerBaseMeta  : public BranchMeta
{
public:
  PlacementMeta  placement;     // inside
  BorderMeta     border;
  BackgroundMeta background;
  MarginsMeta    margin;
  PointerBaseMeta();
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
  PageSizeMeta              size;
  PageOrientationMeta       orientation;
  MarginsMeta               margin;
  BorderMeta                border;
  BackgroundMeta            background;
  BoolMeta                  dpn;
  BoolMeta                  togglePnPlacement;
  NumberPlacementMeta       number;
  NumberPlacementMeta       instanceCount;
  StringListMeta            subModelColor;
  PointerMeta               pointer;
  PointerAttribMeta         pointerAttrib;
  SceneItemMeta             scene;
  IntMeta                   countInstanceOverride;
  BoolMeta                  textPlacement;
  PlacementMeta             textPlacementMeta;

  PageHeaderMeta            pageHeader;
  PageFooterMeta            pageFooter;

  //pageAttributes
  PageAttributeTextMeta     titleFront;              //from LDrawFile: File
  PageAttributeTextMeta     titleBack;               //from LDrawFile: File
  PageAttributeTextMeta     modelName;               //from LDrawFile: Name
  PageAttributeTextMeta     modelDesc;               //from LDrawFile: 2nd line in <topLevelFile>.ldr
  PageAttributeTextMeta     publishDesc;             //from Preferences::publishDescription
  PageAttributeTextMeta     authorFront;             //from Preferences::defaultAuthor / LDrawFile: Author
  PageAttributeTextMeta     authorBack;              //from Preferences::defaultAuthor / LDrawFile: Author
  PageAttributeTextMeta     author;                  //from Preferences::defaultAuthor / LDrawFile: Author
  PageAttributeTextMeta     url;                     //from Preferences::defaultURL
  PageAttributeTextMeta     urlBack;                 //from Preferences::defaultURL
  PageAttributeTextMeta     email;                   //from Preferences::defaultEmail
  PageAttributeTextMeta     emailBack;               //from Preferences::defaultEmail
  PageAttributeTextMeta     disclaimer;              //from Preferences::disclaimer (static)
  PageAttributeImageMeta    documentLogoFront;       //from Preferences::documentLogoFile
  PageAttributeImageMeta    documentLogoBack;        //from Preferences::documentLogoFile
  PageAttributeImageMeta    coverImage;              //from Globals: Page setup
  PageAttributeTextMeta     parts;                   //from LDrawFile: count .dat during load
  PageAttributeTextMeta     copyrightBack;           //from Preferences::copyright (static)
  PageAttributeTextMeta     copyright;               //from Preferences::copyright (static)
  PageAttributeTextMeta     plug;                    //from Preferences::plug (static)
  PageAttributeImageMeta    plugImage;               //from Preferences::plugImage (static)
  PageAttributeTextMeta     category;                //from LDrawFile: !CATEGORY (NOT IMPLEMENTED)

  PageMeta();
  virtual void init(BranchMeta *parent, QString name);
};

/*------------------------*/

class PliMeta  : public BranchMeta
{
public:
  BorderMeta           border;
  BackgroundMeta       background;
  MarginsMeta          margin;
  NumberMeta           instance;
  NumberMeta           annotate;
  PlacementMeta        placement;
  ConstrainMeta        constrain;
  FloatMeta            modelScale;
  PartMeta             part;
  PliPartGroupMeta     pliPartGroup;
  BoolMeta             enablePliPartGroup;
  PliBeginMeta         begin;
  RcMeta               end;
  BoolMeta             includeSubs;
  BoolMeta             show;
  StringListMeta       subModelColor;
  FontListMeta         subModelFont;
  StringListMeta       subModelFontColor;
  BoolMeta             individualParts;
  StringMeta           ldviewParms;
  StringMeta           ldviewEnvVars;
  StringMeta           ldgliteParms;
  StringMeta           ldgliteEnvVars;
  StringMeta           povrayParms;
  StringMeta           povrayEnvVars;
  BoolMeta             pack;
  BoolMeta             sort;
  PliSortMeta          sortBy;  //deprecated
  PliSortOrderMeta     sortOrder;
  PliAnnotationMeta    annotation;
  PliPartElementMeta   partElements;
  AnnotationStyleMeta  elementStyle;
  AnnotationStyleMeta  rectangleStyle;
  AnnotationStyleMeta  circleStyle;
  AnnotationStyleMeta  squareStyle;
  AnnotationStyleMeta  defaultStyle;
  StudStyleMeta        studStyle;
  AutoEdgeColorMeta    autoEdgeColor;
  HighContrastColorMeta highContrast;
  PreferredRendererMeta preferredRenderer;

  // pli/bom/smp camera settings
  FloatMeta            cameraFoV;
  FloatMeta            cameraZNear;
  FloatMeta            cameraZFar;
  CameraAnglesMeta     cameraAngles;
  FloatMeta            cameraDistance;
  BoolMeta             isOrtho;
  FloatPairMeta        imageSize;
  StringMeta           cameraName;
  Vector3Meta          target;
  Vector3Meta          position;
  Vector3Meta          upvector;

  RotStepMeta          rotStep;

  void resetCameraFoV();
  PliMeta();
  virtual void init(BranchMeta *parent, QString name);
};

/*------------------------*/

class AssemMeta : public BranchMeta
{
public:
  // top    == last step
  // bottom == cur step
  MarginsMeta           margin;
  PlacementMeta         placement;
  FloatMeta             modelScale;
  StringMeta            ldviewParms;
  StringMeta            ldviewEnvVars;
  StringMeta            ldgliteParms;
  StringMeta            ldgliteEnvVars;
  StringMeta            povrayParms;
  StringMeta            povrayEnvVars;
  StringMeta            blenderParms;
  StringMeta            blenderEnvVars;
  BoolMeta              showStepNumber;
  CsiAnnotationMeta     annotation;
  StudStyleMeta         studStyle;
  AutoEdgeColorMeta     autoEdgeColor;
  HighContrastColorMeta highContrast;
  FadeStepsMeta         fadeSteps;
  HighlightStepMeta     highlightStep;
  PreferredRendererMeta preferredRenderer;

  // assem camera settings
  FloatMeta            cameraDistance;
  FloatMeta            cameraFoV;
  FloatMeta            cameraZNear;
  FloatMeta            cameraZFar;
  CameraAnglesMeta     cameraAngles;
  BoolMeta             isOrtho;
  FloatPairMeta        imageSize;
  StringMeta           cameraName;
  Vector3Meta          target;
  Vector3Meta          position;
  Vector3Meta          upvector;

  void resetCameraFoV();

  AssemMeta();
  virtual void init(BranchMeta *parent, QString name);
};

/*------------------------*/

class CalloutMeta  : public BranchMeta
{
public:
  // top    == start of callout
  // bottom == end of callout
  // Placement needs to be outside callout begin/end
  AllocMeta           alloc;
  CalloutBeginMeta    begin;
  SettingsMeta        csi;
  RcMeta              divider;
  PointerMeta         divPointer;
  PointerAttribMeta   divPointerAttrib;
  RcMeta              end;
  FreeFormMeta        freeform;
  JustifyStepMeta     justifyStep;
  MarginsMeta         margin;
  PlacementMeta       placement;      //  outside
  SepMeta             sep;
  NumberPlacementMeta stepNum;
  FontListMeta        subModelFont;
  StringListMeta      subModelFontColor;

  ShowSubModelMeta    subModel;
  BackgroundMeta      background;
  BorderMeta          border;
  NumberPlacementMeta instance;
  StepsPliMeta        pli;
  PointerMeta         pointer;
  PointerAttribMeta   pointerAttrib;
  RotateIconMeta      rotateIcon;
  StringListMeta      subModelColor;

  CalloutMeta();
  virtual void init(BranchMeta *parent, QString name);
};

/*------------------------*/

class MultiStepMeta : public BranchMeta
{
public:
  // top    == start of multistep
  // bot    == bottom of multistep
  AllocMeta             alloc;
  RcMeta                begin;
  SettingsMeta          csi;
  RcMeta                divider;
  PointerMeta           divPointer;
  PointerAttribMeta     divPointerAttrib;
  RcMeta                end;
  FreeFormMeta          freeform;
  JustifyStepMeta       justifyStep;
  MarginsMeta           margin;
  PlacementMeta         placement;
  StepsPliMeta        pli;
  SepMeta               sep;
  NumberPlacementMeta   stepNum;
  BoolMeta              showGroupStepNumber;
  BoolMeta              countGroupSteps;
  FontListMeta          subModelFont;
  StringListMeta        subModelFontColor;

  BoolMeta              adjustOnItemOffset;
  BoolMeta              justifyYAxisOutsidePlacement;
  RotateIconMeta        rotateIcon;
  UnitsMeta             stepSize;
  ShowSubModelMeta      subModel;

  MultiStepMeta();
  virtual void init(BranchMeta *parent, QString name);
};

/*------------------------*/
class LightMeta : public BranchMeta
{
public:
  StringMeta  name;              // QString   mName;
  StringMeta  type;              // QString   mLightType; (Light NAME (mName) written on TYPE line)
  StringMeta  areaShape;         // QString   mAreaShape;

  FloatMeta   specular;          // float     mBlenderSpecular;
  FloatMeta   spotConeAngle;     // float     mSpotConeAngle;
  FloatMeta   cutoffDistance;    // float     mSpotCutoff;
  FloatMeta   povrayPower;       // float     mPOVRayPower;
  FloatMeta   blenderPower;      // float     mBlenderPower;
  FloatMeta   diffuse;           // float     mBlenderDiffuse

  FloatMeta   sunAngle;          // float     mDirectionalBlenderAngle
  FloatMeta   pointRadius;       // float     mPointBlenderRadius
  FloatMeta   spotRadius;        // float     mSpotBlenderRadius
  FloatMeta   spotBlend;         // float     calculated (Blender only)
  FloatMeta   fadePower;         // float     mPOVRayFadePower
  FloatMeta   fadeDistance;      // float     mPOVRayFadeDistance
  FloatMeta   spotTightness;     // float     mSpotPOVRayTightness
  FloatMeta   spotPenumbraAngle; // float     mSpotPenumbraAngle

  IntMeta     areaGridX;         // int       mAreaPOVRayGridX
  IntMeta     areaGridY;         // int       mAreaPOVRayGridY
  FloatMeta   areaWidth;         // float     mAreaX (POVRay light area size X)
  FloatMeta   areaHeight;        // float     mAreaY (POVRay light area size Y)
  FloatMeta   areaSizeX;         // float     mAreaSizeX
  FloatMeta   areaSizeY;         // float     mAreaSizeY
  FloatMeta   areaSize;          // float     mAreaSizeX

  Vector3Meta color;             // lcVector3 mLightColor
  Vector3Meta target;            // lcVector3 mPosition
  Vector3Meta position;          // lcVector3 mTargetPosition
  Vector33Meta rotation;         // lcVector33 mRotation

  RcMeta      _povrayLight;
  bool        povrayLight;       // bool      mPOVRayLight
  RcMeta      _shadowless;
  bool        shadowless;        // bool      mShadowless

  float       latitude;          // float Calculated
  float       longitude;         // float Calculated

  LightData value()
  {
    LightData               value;
    value.name              = name.value();
    value.type              = type.value();
    value.areaShape         = areaShape.value();

    value.specular          = specular.value();
    value.spotConeAngle     = spotConeAngle.value();
    value.cutoffDistance    = cutoffDistance.value();
    value.povrayPower       = povrayPower.value();
    value.blenderPower      = blenderPower.value();
    value.diffuse           = diffuse.value();

    value.sunAngle          = sunAngle.value();
    value.pointRadius       = pointRadius.value();
    value.spotRadius        = spotRadius.value();
    value.areaSizeX         = areaSizeX.value();
    value.areaSizeY         = areaSizeY.value();
    value.areaSize          = areaSize.value();
    value.spotBlend         = spotBlend.value();
    value.fadePower         = fadePower.value();
    value.fadeDistance      = fadeDistance.value();
    value.spotTightness     = spotTightness.value();
    value.spotPenumbraAngle = spotPenumbraAngle.value();

    value.areaGridX         = areaGridX.value();
    value.areaGridY         = areaGridY.value();

    value.color[X]          = color.x();
    value.color[Y]          = color.y();
    value.color[Z]          = color.z();

    value.target[X]         = target.x();
    value.target[Y]         = target.y();
    value.target[Z]         = target.z();

    value.position[X]       = position.x();
    value.position[Y]       = position.y();
    value.position[Z]       = position.z();

    value.rotation1[X]      = rotation.x1();
    value.rotation1[Y]      = rotation.y1();
    value.rotation1[Z]      = rotation.z1();
    value.rotation2[X]      = rotation.x2();
    value.rotation2[Y]      = rotation.y2();
    value.rotation2[Z]      = rotation.z2();
    value.rotation3[X]      = rotation.x3();
    value.rotation3[Y]      = rotation.y3();
    value.rotation3[Z]      = rotation.z3();

    setLatLong();

    value.latitude          = latitude;
    value.longitude         = longitude;

    value.povrayLight       = povrayLight;
    value.shadowless        = shadowless;
    value.defaultLight      = name.value().isEmpty();

    return value;
  }

  void setValue(LightData &value)
  {
    name             .setValue(value.name);
    type             .setValue(value.type);
    areaShape        .setValue(value.areaShape);

    specular         .setValue(value.specular);
    spotConeAngle    .setValue(value.spotConeAngle);
    cutoffDistance   .setValue(value.cutoffDistance);
    povrayPower      .setValue(value.povrayPower);
    blenderPower     .setValue(value.blenderPower);
    diffuse          .setValue(value.diffuse);

    sunAngle         .setValue(value.sunAngle);
    pointRadius      .setValue(value.pointRadius);
    spotRadius       .setValue(value.spotRadius);
    areaSizeX        .setValue(value.areaSizeX);
    areaSizeY        .setValue(value.areaSizeY);
    areaSize         .setValue(value.areaSize);
    spotBlend        .setValue(value.spotBlend);
    fadePower        .setValue(value.fadePower);
    fadeDistance     .setValue(value.fadeDistance);
    spotTightness    .setValue(value.spotTightness);
    spotPenumbraAngle.setValue(value.spotPenumbraAngle);

    areaGridX        .setValue(value.areaGridX);
    areaGridY        .setValue(value.areaGridY);

    color            .setValues(value.color[X],value.color[Y],value.color[Z]);
    target           .setValues(value.target[X],value.target[Y],value.target[Z]);
    position         .setValues(value.position[X],value.position[Y],value.position[Z]);
    rotation         .setValues(value.rotation1[X],value.rotation2[X],value.rotation3[X],
                                value.rotation1[Y],value.rotation2[Y],value.rotation3[Y],
                                value.rotation1[Z],value.rotation2[Z],value.rotation3[Z]);

    povrayLight = value.povrayLight;
    shadowless  = value.shadowless;

    setLatLong();
  }

  void reset()
  {
    LightData values;
    setValue(values);
  }

  void setLatLong();

  LightMeta();
  virtual void init(BranchMeta *parent, QString name);
};

/*------------------------*/

class BomMeta  : public PliMeta
{
public:
  BomMeta();
  virtual void init(BranchMeta *parent, QString name);
};

/*------------------------*/

class SubModelMeta  : public PliMeta
{
public:
  IntMeta    showStepNum; // Used to indicate the 'first' step number of a step group when continuous step numbers flag is On
  BoolMeta   showInstanceCount;
  BoolMeta   showTopModel;
  BoolMeta   showSubmodelInCallout;
  void setPreferences()
  {
      Preferences::showSubmodels         = show.value();
      Preferences::showTopModel          = showTopModel.value();
      Preferences::showSubmodelInCallout = showSubmodelInCallout.value();
      Preferences::showInstanceCount     = showInstanceCount.value();
  }

  SubModelMeta();
  virtual void init(BranchMeta *parent, QString name);
};

/*------------------------*/

class LeoCadGroupMeta : public BranchMeta
{
public:
  RcMeta     begin;
  RcMeta     end;
  LeoCadGroupMeta() {}
  virtual void init(BranchMeta *parent, QString name);
  virtual Rc parse(QStringList &argv, int index, Where &here);
};

/*------------------------*/

class LPubMeta : public BranchMeta
{
public:
  ResolutionMeta       resolution;
  PageMeta             page;
  AssemMeta            assem;
  NumberPlacementMeta  stepNumber;
  CalloutMeta          callout;
  PointerBaseMeta      pointerBase;
  MultiStepMeta        multiStep;
  PliMeta              pli;
  BomMeta              bom;
  RemoveMeta           remove;
  BuildModMeta         buildMod;
  BuildModEnabledMeta  buildModEnabled;
  FinalModelEnabledMeta finalModelEnabled;
  FloatMeta            reserve;
  FloatMeta            cameraDDF;
  PartIgnMeta          partSub;
  InsertMeta           insert;
  StringMeta           include;
  NoStepMeta           nostep;
  FadeStepsMeta        fadeSteps;
  HighlightStepMeta    highlightStep;
  PreferredRendererMeta preferredRenderer;
  RotateIconMeta       rotateIcon;
  SubModelMeta         subModel;
  CountInstanceMeta    countInstance;
  BoolMeta             countInstanceByColour;
  ContStepNumMeta      contStepNumbers;
  IntMeta              contModelStepNum;
  IntMeta              startPageNumber;
  IntMeta              startStepNumber;
  StepPliMeta          stepPli;
  StudStyleMeta        studStyle;
  AutoEdgeColorMeta    autoEdgeColor;
  HighContrastColorMeta highContrast;
  BoolMeta             parseNoStep;
  BoolMeta             coverPageViewEnabled;
  BoolMeta             setSubstituteAsUnofficialPart;
  LoadUnoffPartsMeta   loadUnoffPartsInEditor;

  LeoCadGroupMeta      group;
  LightMeta            light;
  RcMeta               model;
  RcMeta               piece;
  RcMeta               camera;
  RcMeta               synth;

  void resetCamerasFoV();

  LPubMeta();
  virtual void init(BranchMeta *parent, QString name);
};

/*------------------------*/

class MLCadMeta : public BranchMeta
{
public:
  RcMeta     MLCadSB;
  RcMeta     MLCadSE;
  StringMeta MLCadGrp;
  MLCadMeta() {}
  virtual void init(BranchMeta *parent, QString name);
  virtual Rc parse(QStringList &argv, int index, Where &here);
};

/*------------------------*/

class LDCadMeta : public BranchMeta
{
public:
  StringMeta LDCadGrp;
  RcMeta     LDCadGenerated;
  LDCadMeta() {}
  virtual void init(BranchMeta *parent, QString name);
  virtual Rc parse(QStringList &argv, int index, Where &here);
};

/*------------------------*/

class LeoCadMeta : public BranchMeta
{
public:
  LeoCadGroupMeta group;
  LightMeta       light;
  RcMeta          model;
  RcMeta          piece;
  RcMeta          camera;
  RcMeta          synth;
  LeoCadMeta() {}
  virtual void init(BranchMeta *parent, QString name);
  virtual Rc parse(QStringList &argv, int index, Where &here);
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
  virtual void init(BranchMeta *parent, QString name);
};

/*------------------------*/

class PartTypeMeta : public BranchMeta
{
public:
  StringMeta partType;
  PartTypeMeta() {}
  virtual void init(BranchMeta *parent, QString name);
  virtual Rc parse(QStringList &argv, int index, Where &here);
};

/*------------------------*/

class PartNameMeta : public BranchMeta
{
public:
  StringMeta partName;
  PartNameMeta() {}
  virtual void init(BranchMeta *parent, QString name);
  virtual Rc parse(QStringList &argv, int index, Where &here);
};

class Meta : public BranchMeta
{
public:
  LPubMeta      LPub;
  RcMeta        step;
  RcMeta        clear;
  RcMeta        fade;
  RcMeta        silhouette;
  RcMeta        texmap;
  RcMeta        colour;
  RotStepMeta   rotStep;
  BuffExchgMeta bfx;
  MLCadMeta     MLCad;
  LDCadMeta     LDCad;
  LeoCadMeta    LeoCad;
  LSynthMeta    LSynth;

  QList<SubmodelStack>  submodelStack;

  Meta();
  virtual ~Meta() {};
  Rc    parse(QString &line, Where &here, bool reportErrors = 0);
  bool  preambleMatch(QString &line, QString &preamble);
  virtual void  init(BranchMeta *parent, QString name);
  virtual void  pop();
  void  doc(QStringList &out);
  void  metaKeywords(QStringList &out, bool = false);
  void  processSpecialCases(QString &, Where &);
  Meta (const Meta &rhs) : BranchMeta(rhs)
  {
    QString empty;
    init(nullptr,empty);
    LPub          = rhs.LPub;
    step          = rhs.step;
    clear         = rhs.clear;
    fade          = rhs.fade;
    silhouette    = rhs.silhouette;
    colour        = rhs.colour;
    rotStep       = rhs.rotStep;
    bfx           = rhs.bfx;
    MLCad         = rhs.MLCad;
    LDCad         = rhs.LDCad;
    LeoCad        = rhs.LeoCad;
    LSynth        = rhs.LSynth;
    submodelStack = rhs.submodelStack;
  }

private:
};

extern const QString relativeNames[];
extern const QString bRectPlacementNames[];
extern const QString bPlacementEncNames[];
extern const QString placementNames[];
extern const QString prepositionNames[];
extern const QString placementOptions[][3];
extern int placementDecode[][3];
extern QHash<QString, int> tokenMap;
extern QHash<Rc, QRegExp> groupRegExMap;

#endif
