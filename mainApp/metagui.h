
/****************************************************************************
**
** Copyright (C) 2007-2009 Kevin Clague. All rights reserved.
** Copyright (C) 2015 - 2022 Trevor SANDY. All rights reserved.
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
 * This file describes a set of classes that implement graphical user
 * interfaces for some of the configuration meta classes described in meta.h
 *
 * Please see lpub.h for an overall description of how the files in LPub
 * make up the LPub program.
 *
 ***************************************************************************/

#ifndef metagui_h
#define metagui_h

#include <QWidget>
#include <QTextEdit>

#include "resolution.h"
#include "metatypes.h"
#include "gradients.h"

class QWidget;
class QString;
class QHBoxLayout;
class LeafMeta;
class UnitsMeta;
class PageSizeMeta;

class MetaGui  : public QWidget
{
  Q_OBJECT
public:
  MetaGui()
  {
    modified = false;
  }
  ~MetaGui() {}

  virtual void apply(QString &topLevelFile) = 0;

  bool      modified;
  QString   changeMessage;

signals:
  void settingsChanged(bool);
  void rendererChanged(int);

protected:
  static bool notEqual(const double v1, const double v2, int p = 4);
  static QString formatMask(
    const float value,
    const int   width = 4,
    const int   precision = 4,
    const int   defaultDecimalPlaces = 1);
  LeafMeta *_meta;
};

/***********************************************************************
 *
 * Bool
 *
 **********************************************************************/

class QGroupBox;
class QLabel;
class QCheckBox;
class BoolMeta;
class CheckBoxGui : public MetaGui
{
  Q_OBJECT
public:

  CheckBoxGui(
    QString const  &heading,
    BoolMeta       *meta,
    QGroupBox      *parent = nullptr);
  CheckBoxGui(){}
  ~CheckBoxGui() {}

  QCheckBox *getCheckBox() {return check;}

  virtual void apply(QString &modelName);

private:
  BoolMeta  *meta;
  QCheckBox *check;

public slots:
  void stateChanged(int state);
};

/***********************************************************************
 *
 * Bool
 *
 **********************************************************************/

class QRadioButton;
class BoolRadioGui : public MetaGui
{
  Q_OBJECT
public:

  BoolRadioGui(
    QString const &trueHeading,
    QString const &falseHeading,
    BoolMeta      *meta,
    QGroupBox     *parent = nullptr);

  ~BoolRadioGui() {}

  virtual void apply(QString &modelName);

private:
  BoolMeta     *meta;
  QRadioButton *trueRadio;
  QRadioButton *falseRadio;

public slots:
  void trueClicked(bool clicked);
  void falseClicked(bool clicked);
};

/***********************************************************************
 *
 * Units
 *
 **********************************************************************/


class QLineEdit;
class UnitsGui : public MetaGui
{
  Q_OBJECT
public:

  UnitsGui(
    QString const  &heading,
    UnitsMeta      *meta,
    QGroupBox      *parent = nullptr,
    bool            isMargin = true);
  ~UnitsGui() {}

  void setEnabled(bool enabled);

  virtual void apply(QString &modelName);

private:
  float      data0;
  float      data1;

  UnitsMeta *meta;
  QLabel    *label;
  QLabel    *label2;
  QLineEdit *value0Edit;
  QLineEdit *value1Edit;
  QAction   *reset0Act;
  QAction   *reset1Act;

private slots:
  void enableReset(QString const &);
  void lineEditReset();

public slots:
  void value0Change(QString const &);
  void value1Change(QString const &);
};

/***********************************************************************
 *
 * Float Pair
 *
 **********************************************************************/

class FloatPairMeta;
class FloatsGui : public MetaGui
{
  Q_OBJECT
public:

  FloatsGui(
    QString const &heading0,
    QString const &heading1,
    FloatPairMeta *meta,
    QGroupBox     *parent = nullptr,
    int            decPlaces = 1,
    bool           showPair = true);
  ~FloatsGui() {}

  void setEnabled(bool enabled);

  virtual void apply(QString &modelName);

private slots:
  void enableReset(QString const &);
  void lineEditReset();

private:
  float         data0;
  float         data1;
  bool          showPair;
  FloatPairMeta *meta;
  QLabel        *label0;
  QLabel        *label1;
  QLineEdit     *value0Edit;
  QLineEdit     *value1Edit;
  QAction       *reset0Act;
  QAction       *reset1Act;

public slots:
  void value0Change(QString const &);
  void value1Change(QString const &);
};

/***********************************************************************
 *
 * Integer Spin Box
 *
 **********************************************************************/

class SpinGui : public MetaGui
{
  Q_OBJECT
public:

  SpinGui(
    QString const &heading,
    IntMeta       *meta,
    int            min,
    int            max,
    int            step,
    QGroupBox     *parent = nullptr);
  ~SpinGui() {}

  virtual void apply(QString &modelName);

  void setEnabled(bool enabled);
  QSpinBox *getSpinBox() {return spin;}

private:
  int          data;
  IntMeta     *meta;
  QLabel      *label;
  QSpinBox    *spin;
  QPushButton *button;

private slots:
  void enableReset(int);
  void spinReset(bool);

public slots:
  void valueChanged(int);
};

/***********************************************************************
 *
 * Double Spin Box
 *
 **********************************************************************/

class DoubleSpinGui : public MetaGui
{
  Q_OBJECT
public:

  DoubleSpinGui(
    QString const &heading,
    FloatMeta     *meta,
    float          min,
    float          max,
    float          step,
    QGroupBox     *parent = nullptr);
  ~DoubleSpinGui() {}

  virtual void apply(QString &modelName);

  void setEnabled(bool enabled);

private slots:
  void enableReset(double);
  void spinReset(bool);

private:
  float           data;
  FloatMeta      *meta;
  QLabel         *label;
  QDoubleSpinBox *spin;
  QPushButton    *button;

public slots:
  void valueChanged(double);
};

/***********************************************************************
 *
 * Number
 *
 **********************************************************************/

class NumberMeta;
class QPushButton;
class NumberGui : public MetaGui
{
  Q_OBJECT

public:
  NumberGui(
    QString     title,
    NumberMeta *meta,
    QGroupBox  *_parent = nullptr);
  ~NumberGui() {}

  virtual void apply(QString &modelName);

private:
  NumberMeta  *meta;

  bool        fontModified;
  bool        colorModified;
  bool        marginsModified;
  float       marginData0;
  float       marginData1;

  QGroupBox   *gbFormat;
  QLabel      *fontLabel;
  QLabel      *fontExample;
  QPushButton *fontButton;
  QLabel      *colorLabel;
  QLabel      *colorExample;
  QPushButton *colorButton;
  QLabel      *marginsLabel;
  QLineEdit   *value0Edit;
  QLineEdit   *value1Edit;
  QAction     *reset0Act;
  QAction     *reset1Act;

private slots:
  void enableReset(QString const &);
  void lineEditReset();

public slots:
  void browseFont(bool clicked);
  void browseColor(bool clicked);
  void value0Changed(QString const &);
  void value1Changed(QString const &);
  void enableTextFormatGroup(bool);
};

/***********************************************************************
 *
 * Stud Style
 *
 **********************************************************************/

class IntMeta;
class QComboBox;
class StudStyleGui : public MetaGui
{
  Q_OBJECT
public:

  StudStyleGui(
    AutoEdgeColorMeta     *autoEdgeMeta,
    StudStyleMeta         *studStyleMeta,
    HighContrastColorMeta *highContrastMeta,
    QGroupBox             *parent = nullptr);
  ~StudStyleGui() {}

  QCheckBox *getCheckBox() {return checkbox;}
  QComboBox *getComboBox() {return combo;}

  virtual void apply(QString &modelName);

private:
  HighContrastColorMeta *highContrastMeta;
  StudStyleMeta         *studStyleMeta;
  AutoEdgeColorMeta     *autoEdgeMeta;
  QCheckBox             *checkbox;
  QComboBox             *combo;
  QToolButton           *studStyleButton;
  QToolButton           *autoEdgeButton;

  bool    lightDarkIndexModified;
  bool    studCylinderColorModified;
  bool    partEdgeColorModified;
  bool    blackEdgeColorModified;
  bool    darkEdgeColorModified;
  bool    contrastModified;
  bool    saturationModified;

  bool    studStyleModified;
  bool    autoEdgeModified;

public slots:
  void comboChanged(int);
  void checkBoxChanged(bool);
  void processToolButtonClick();
  void enableStudStyleButton(int index);
  void enableAutoEdgeButton();
};

/***********************************************************************
 *
 * PageAttributeText
 *
 **********************************************************************/

enum RelativeTos
{ fc = 1 ,   bc = 2,     ph  = 4,      pf  = 8};

class PageAttributeTextMeta;
class PlacementMeta;
class PageAttributePlacementEnum;
class GlobalPageDialog;
class PageAttributeTextGui : public MetaGui
{
  Q_OBJECT
public:

  PageAttributeTextGui(
    PageAttributeTextMeta *meta,
    QGroupBox  *parent = nullptr);

  ~PageAttributeTextGui() {}

  QGroupBox *getContentEditGBox() {return gbContentEdit;}
  QLineEdit *getContentLineEdit() {return contentLineEdit;}
  QTextEdit *getContentTextEdit() {return contentTextEdit;}

  virtual void apply(QString &topLevelFile);

private:
  PageAttributeTextMeta  *meta;

  int         selection;
  bool        fontModified;
  bool        colorModified;
  bool        marginsModified;
  bool        placementModified;
  bool        displayModified;
  bool        editModified;
  float       marginData0;
  float       marginData1;

  QLabel      *fontLabel;
  QLabel      *fontExample;
  QPushButton *fontButton;
  QLabel      *colorLabel;
  QLabel      *colorExample;
  QPushButton *colorButton;
  QLabel      *marginsLabel;

  QLineEdit    *value0Edit;
  QLineEdit    *value1Edit;
  QAction      *reset0Act;
  QAction      *reset1Act;

  QCheckBox    *display;

  QGroupBox    *gbContentEdit;

  QTextEdit    *contentTextEdit;
  QLineEdit    *contentLineEdit;

  QString      content;

  QGroupBox    *gbPlacement;
  QPushButton  *placementButton;

  QLabel       *sectionLabel;
  QComboBox    *sectionCombo;

signals:
  void indexChanged(int index);

private slots:
  void enableReset(QString const &);
  void lineEditReset();

public slots:
  void newIndex(int value){
      selection = value;
  }

  void textEditChanged();
  void lineEditChanged(QString const &);
  void value0Changed(QString const &);
  void value1Changed(QString const &);
  void placementChanged(bool);
  void toggled(bool);
  void browseFont(bool);
  void browseColor(bool);
  friend class GlobalPageDialog;

};
/***********************************************************************
 *
 * PageAttributeImage
 *
 **********************************************************************/

class PageAttributeImageMeta;
class QDoubleSpinBox;
class FloatMeta;
class PageAttributeImageGui : public MetaGui
{
  Q_OBJECT
public:

  PageAttributeImageGui(
    PageAttributeImageMeta *meta,
    QGroupBox         *parent = nullptr);

  ~PageAttributeImageGui() {}

  virtual void apply(QString &topLevelFile);

  QLineEdit *getImageEdit() {return imageEdit;}

  void setEnabled(bool enabled);

private:
  PageAttributeImageMeta *meta;

  int             selection;
  bool            marginsModified;
  bool            placementModified;
  bool            displayModified;
  bool            imageModified;
  bool            scaleModified;
  bool            fillModified;
  float           marginData0;
  float           marginData1;
  float           scaleData;

  QString         image;

  QLabel         *marginsLabel;
  QLineEdit      *value0Edit;
  QLineEdit      *value1Edit;
  QAction        *reset0Act;
  QAction        *reset1Act;
  QPushButton    *resetButton;

  QLineEdit      *imageEdit;
  QPushButton    *imageButton;
  QGroupBox      *gbScale;
  QLabel         *scale;
  QDoubleSpinBox *spin;

  QGroupBox    *gbFill;
  QRadioButton *stretchRadio;
  QRadioButton *tileRadio;
  QRadioButton *aspectRadio;

  QCheckBox      *display;

  QGroupBox      *gbPlacement;
  QPushButton    *placementButton;

  QLabel         *sectionLabel;
  QComboBox      *sectionCombo;

signals:
  void indexChanged(int index);

private slots:
  void enableEditReset(QString const &);
  void enableSpinReset(double);
  void lineEditReset();
  void spinReset(bool);

public slots:
  void selectionChanged(int value){
      selection = value;
  }
  void imageFill(bool);
  void imageChange(QString const &);
  void browseImage(bool);
  void gbScaleClicked(bool);
  void value0Changed(QString const &);
  void value1Changed(QString const &);

  void valueChanged(double);

  void placementChanged(bool);
  void toggled(bool toggled);

  friend class GlobalPageDialog;
};

/***********************************************************************
 *
 * PageHeaderFooterHeight
 *
 **********************************************************************/

class HeaderFooterHeightGui : public MetaGui
{
  Q_OBJECT
public:

  HeaderFooterHeightGui(
    QString const  &heading,
    UnitsMeta      *meta,
    QGroupBox      *parent = nullptr);
  ~HeaderFooterHeightGui() {}

  void setEnabled(bool enabled);

  virtual void apply(QString &modelName);

private:
  float      data1;
  UnitsMeta *meta;
  QLabel    *label;
  QLineEdit *value0Edit;
  QLineEdit *value1Edit;
  QAction   *reset1Act;

private slots:
  void enableReset(QString const &);
  void lineEditReset();

public slots:
  void value1Change(QString const &);
};

/***********************************************************************
 *
 * Fade Steps
 *
 **********************************************************************/

class FadeStepsMeta;
class FadeStepsGui : public MetaGui
{
  Q_OBJECT
public:

  FadeStepsGui(
    FadeStepsMeta *meta,
    QGroupBox  *parent = nullptr);
  ~FadeStepsGui() {}

  QCheckBox *getCheckBox() { return fadeCheck; }

  virtual void apply(QString &modelName);

private:
  FadeStepsMeta  *meta;

  QCheckBox   *fadeCheck;
  QCheckBox   *useColorCheck;
  QSlider     *fadeOpacitySlider;
  QLabel      *colorExample;
  QComboBox   *colorCombo;

  bool colorModified;
  bool fadeModified;
  bool useColorModified;
  bool opacityModified;

public slots:
  void colorChange(QString const &colorName);
  void valueChanged(int state);
};

/***********************************************************************
 *
 * HighlightStep
 *
 **********************************************************************/

class HighlightStepMeta;
class HighlightStepGui : public MetaGui
{
  Q_OBJECT
public:

  HighlightStepGui(
    HighlightStepMeta *meta,
    QGroupBox  *parent = nullptr);
  ~HighlightStepGui() {}

  virtual void apply(QString &modelName);

  QCheckBox *getCheckBox() { return highlightCheck; }

private:
  HighlightStepMeta  *meta;

  QCheckBox   *highlightCheck;
  QLabel      *colorExample;
  QPushButton *colorButton;
  QSpinBox    *lineWidthSpin;
  QPushButton *button;

  bool global;

  bool colorModified;
  bool highlightModified;
  bool lineWidthModified;
  int  data;

private slots:
  void enableReset(int);
  void spinReset(bool);

public slots:
  void colorChange(bool clicked);
  void valueChanged(int state);
};

/***********************************************************************
 *
 * JustifyStep
 *
 **********************************************************************/

class JustifyStepGui : public MetaGui
{
  Q_OBJECT
public:

  JustifyStepGui(
    QString const   &label,
    JustifyStepMeta *meta,
    QGroupBox       *parent = nullptr);
  ~JustifyStepGui() {}

  virtual void apply(QString &modelName);

private:
  JustifyStepMeta *meta;
  QComboBox       *typeCombo;
  QDoubleSpinBox  *spacingSpinBox;
  QPushButton     *button;

  JustifyStepData data;

private slots:
  void enableReset(double);
  void spinReset(bool);

public slots:
   void typeChanged(int);
   void spacingChanged(double);
};

/***********************************************************************
 *
 * RotStep
 *
 **********************************************************************/

class RotStepData;
class RotStepGui : public MetaGui
{
  Q_OBJECT
public:

  RotStepGui(
    RotStepMeta *meta,
    QGroupBox   *parent = nullptr);
  ~RotStepGui() {}

  virtual void apply(QString &modelName);

private:
  RotStepMeta     *meta;
  RotStepData      data;

  QDoubleSpinBox  *rotStepSpinX;
  QDoubleSpinBox  *rotStepSpinY;
  QDoubleSpinBox  *rotStepSpinZ;
  QComboBox       *typeCombo;

  QPushButton    *button0;
  QPushButton    *button1;
  QPushButton    *button2;

private slots:
  void enableReset(double);
  void spinReset(bool);

public slots:
   void rotStepChanged(double);
   void typeChanged(QString const &);
};

/***********************************************************************
 *
 * Continuous Step Number
 *
 **********************************************************************/

class ContStepNumGui : public MetaGui
{
  Q_OBJECT
public:

  ContStepNumGui(
    QString const  &heading,
    ContStepNumMeta *meta,
    QGroupBox       *parent = nullptr);
  ContStepNumGui(){}
  ~ContStepNumGui() {}

  QCheckBox *getCheckBox() {return check;}

  void setEnabled(bool enabled);

  virtual void apply(QString &modelName);

private:
  ContStepNumMeta *meta;
  QCheckBox       *check;

public slots:
  void stateChanged(int state);
};

/***********************************************************************
 *
 * Count Instance
 *
 **********************************************************************/

class CountInstanceGui : public MetaGui
{
  Q_OBJECT
public:

  CountInstanceGui(
    CountInstanceMeta *_meta,
    QGroupBox         *parent = nullptr);
  ~CountInstanceGui() {}

  QCheckBox    *getCheckBox() {return countCheck;}
  QRadioButton *getTopRadio() {return topRadio;}
  QRadioButton *getModelRadio() {return modelRadio;}
  QRadioButton *getStepRadio() {return stepRadio;}

  void setEnabled(bool enabled);

  virtual void apply(QString &modelName);

private:
  CountInstanceMeta  *meta;
  QCheckBox          *countCheck;
  QRadioButton       *topRadio;
  QRadioButton       *modelRadio;
  QRadioButton       *stepRadio;

public slots:
  void radioChanged(bool checked);
  void valueChanged(bool checked);
};

/***********************************************************************
 *
 * Build Modification
 *
 **********************************************************************/

class BuildModEnabledGui : public MetaGui
{
  Q_OBJECT
public:

  BuildModEnabledGui(
    QString const  &heading,
    BuildModEnabledMeta *meta,
    QGroupBox       *parent = nullptr);
  BuildModEnabledGui(){}
  ~BuildModEnabledGui() {}

  QCheckBox *getCheckBox() {return check;}

  void setEnabled(bool enabled);

  virtual void apply(QString &modelName);

private:
  BuildModEnabledMeta *meta;
  QCheckBox           *check;
  bool                 data;

public slots:
  void stateChanged(int state);
};

/***********************************************************************
 *
 * Final Model Enabled
 *
 **********************************************************************/

class FinalModelEnabledGui : public MetaGui
{
  Q_OBJECT
public:

  FinalModelEnabledGui(
    QString const  &heading,
    FinalModelEnabledMeta *meta,
    QGroupBox       *parent = nullptr);
  FinalModelEnabledGui(){}
  ~FinalModelEnabledGui() {}

  QCheckBox *getCheckBox() {return check;}

  virtual void apply(QString &modelName);

private:
  FinalModelEnabledMeta *meta;
  QCheckBox             *check;
  bool                   data;

public slots:
  void stateChanged(int state);
};

/***********************************************************************
 *
 * Cover Page Model View Enabled
 *
 **********************************************************************/

class CoverPageViewEnabledGui : public MetaGui
{
  Q_OBJECT
public:

  CoverPageViewEnabledGui(
    QString const &heading,
    BoolMeta       *meta,
    QGroupBox      *parent = nullptr);
  CoverPageViewEnabledGui(){}
  ~CoverPageViewEnabledGui() {}

  QCheckBox *getCheckBox() {return check;}

  void setEnabled(bool enabled);

  virtual void apply(QString &modelName);

private:
  BoolMeta  *meta;
  QCheckBox *check;
  bool       data;

public slots:
  void stateChanged(int state);
};

/***********************************************************************
 *
 * Load Unofficial Parts In Editor Enabled
 *
 **********************************************************************/

class LoadUnoffPartsEnabledGui : public MetaGui
{
  Q_OBJECT
public:

  LoadUnoffPartsEnabledGui(
    QString const       &heading,
    LoadUnoffPartsMeta  *meta,
    QGroupBox           *parent = nullptr);
  LoadUnoffPartsEnabledGui(){}
  ~LoadUnoffPartsEnabledGui() {}

  QCheckBox *getCheckBox() {return check;}

  void setEnabled(bool enabled);

  virtual void apply(QString &modelName);

private:
  LoadUnoffPartsMeta  *meta;
  QCheckBox           *check;
  bool                 data;

public slots:
  void stateChanged(int state);
};

/***********************************************************************
 *
 * Background
 *
 **********************************************************************/

class BackgroundMeta;
class BackgroundGui : public MetaGui
{
  Q_OBJECT
public:

  BackgroundGui(
    BackgroundMeta  *meta,
    QGroupBox       *parent = nullptr,
    bool             pictureSettings = true);

  ~BackgroundGui() {}

  BackgroundMeta *meta;

  virtual void apply(QString &modelName);

private:
  QString picture;
  QString color;
  QString gradient;

  QLabel       *colorExample;
  QPushButton  *colorButton;
  QPushButton  *gradientButton;
  QLineEdit    *pictureEdit;
  QPushButton  *pictureButton;
  QRadioButton *stretchRadio;
  QRadioButton *tileRadio;
  QGroupBox    *fill;

  void enable();

public slots:
  void typeChange(QString const &);
  void imageChange(QString const &);
  void browseImage(bool);
  void setGradient(bool);
  void browseColor(bool);
  void stretch(bool);
  void tile(bool);
};

/***********************************************************************
 *
 * Border
 *
 **********************************************************************/

class BorderMeta;
class BorderGui : public MetaGui
{
  Q_OBJECT
public:

  BorderGui(
    BorderMeta *meta,
    QGroupBox  *parent  = nullptr,
    bool rotateArrow = false,
    bool corners = true);
  ~BorderGui() {}

  virtual void apply(QString &modelName);

private:
  BorderMeta  *meta;
  BorderData   border;

  QLineEdit   *thicknessEdit;
  QLineEdit   *marginXEdit;
  QLineEdit   *marginYEdit;
  QAction     *resetThicknessEditAct;
  QAction     *resetXEditAct;
  QAction     *resetYEditAct;
  QLabel      *thicknessLabel;
  QLabel      *colorExample;
  QPushButton *colorButton;
  QPushButton *resetButton;
  QSpinBox    *spin;
  QLabel      *spinLabel;
  QComboBox   *typeCombo;
  QComboBox   *lineCombo;
  QCheckBox   *hideArrowsChk;

  void enable(bool rotateArrow = false);

private slots:
  void enableEditReset(QString const &);
  void enableSpinReset(int);
  void lineEditReset();
  void spinReset(bool);

public slots:
  void typeChange(QString const &);
  void lineChange(QString const &);
  void thicknessChange(QString const &);
  void browseColor(bool);
  void checkChange(int);
  void radiusChange(int);
  void marginXChange(QString const &);
  void marginYChange(QString const &);
};

/***********************************************************************
 *
 * Default Placement
 *
 **********************************************************************/

class PlacementGui : public MetaGui
{
  Q_OBJECT
public:
  PlacementGui(
    QString        _title,
    PlacementMeta *_meta,
    PlacementType  _type = NumRelatives,
    QGroupBox     *parent = nullptr);
  ~PlacementGui() {}

  virtual void apply(QString &modelName);

private:
  PlacementMeta  *meta;
  PlacementData   data;
  QLabel         *placementLabel;
  QPushButton    *placementButton;
  QString         title;

public slots:
  void placementChanged(bool);
};


/***********************************************************************
 *
 * Pointer Attribute
 *
 **********************************************************************/

class PointerAttribMeta;
class PointerAttribGui : public MetaGui
{
  Q_OBJECT
public:
  PointerAttribGui(
    PointerAttribMeta *meta,
    QGroupBox         *parent    = nullptr,
    bool              _isCallout = false);
  ~PointerAttribGui() {}

  virtual void apply(QString &modelName);

  QCheckBox *getHideTipCheck() { return hideTipBox; }

  PointerAttribData data;

  bool        tipModified;
  bool        lineModified;
  bool        borderModified;

private:
  bool        isBorder;
  bool        isLine;
  bool        isTip;

  PointerAttribMeta *meta;

  QLineEdit   *widthEdit;
  QLineEdit   *heightEdit;
  QLineEdit   *thicknessEdit;
  QAction     *resetWidthEditAct;
  QAction     *resetHeightEditAct;
  QAction     *resetThicknessEditAct;
  QLabel      *colorExample;
  QPushButton *colorButton;
  QComboBox   *lineCombo;
  QCheckBox   *hideTipBox;

private slots:
  void enableEditReset(QString const &);
  void lineEditReset();

public slots:
  void lineChange(QString const &);
  void sizeChange(QString const &);
  void browseColor(bool);
  void hideTipChange(bool);
};

/***********************************************************************
 *
 * Constrain PLI
 *
 **********************************************************************/

class ConstrainMeta;
class ConstrainGui : public MetaGui
{
  Q_OBJECT
public:

  ConstrainGui(
    QString const &heading,
    ConstrainMeta *meta,
    QGroupBox     *parent = nullptr);
  ~ConstrainGui() {}

  virtual void apply(QString &modelName);

  void setEnabled(bool enable);

private:
  ConstrainMeta *meta;
  ConstrainData data;

  QLabel    *headingLabel;
  QComboBox *combo;
  QLineEdit *valueEdit;

  void enable();

public slots:
  void typeChange( QString const &);
  void valueChange(QString const &);
};

/***********************************************************************
 *
 * Separator/Divider
 *
 **********************************************************************/

class SepMeta;
class SepGui : public MetaGui
{
  Q_OBJECT
public:

  SepGui(
  SepMeta   *meta,
  QGroupBox *parent = nullptr);
  ~SepGui() {}

  virtual void apply(QString &modelName);

private:
  SepMeta   *meta;
  SepData    data;
  QLabel    *colorExample;

  QLineEdit *thicknessEdit;
  QLineEdit *lengthEdit;
  QAction   *resetThicknessEditAct;
  QAction   *resetLengthEditAct;

  QLineEdit *marginXEdit;
  QLineEdit *marginYEdit;
  QAction   *resetXEditAct;
  QAction   *resetYEditAct;

private slots:
  void enableReset(QString const &);
  void lineEditReset();

public slots:
  void typeChange(int type);
  void lengthChange(QString const &);
  void thicknessChange(QString const &);
  void browseColor(bool clicked);
  void marginXChange(QString const &);
  void marginYChange(QString const &);
};

/***********************************************************************
 *
 * Resolution
 *
 **********************************************************************/

class ResolutionMeta;
class QIntValidator;
class ResolutionGui : public MetaGui
{
  Q_OBJECT
public:

  ResolutionGui(
    ResolutionMeta *meta,
    QGroupBox      *parent = nullptr);
  ~ResolutionGui() {}

  virtual void apply(QString &modelName);

private:
  ResolutionMeta *meta;
  ResolutionType  dataT;
  float           dataV;
  QLineEdit      *valueEdit;
  QAction        *reset0Act;

signals:
  void unitsChanged(int);

private slots:
  void enableReset(QString const &);
  void lineEditReset();

public slots:
  void unitsChange(QString const &);
  void valueChange(QString const &);
};

/***********************************************************************
 *
 * Preferred Renderer
 *
 **********************************************************************/

class PreferredRendererGui : public MetaGui
{
  Q_OBJECT
public:

  PreferredRendererGui(
          PreferredRendererMeta *meta,
          QGroupBox *parent = nullptr);
  ~PreferredRendererGui() {}

  virtual void apply(QString &topLevelFile);

  QComboBox *getComboBox() {return combo;}

private:
  PreferredRendererMeta *meta;

  QComboBox    *combo;
  QCheckBox    *ldvSingleCallBox;
  QCheckBox    *ldvSnapshotListBox;
  QGroupBox    *povFileGeneratorGrpBox;
  QRadioButton *nativeButton;
  QRadioButton *ldvButton;

public slots:
  void valueChanged(int state);
  void buttonChanged(bool checked);
};

/***********************************************************************
 *
 * CameraAngles
 *
 **********************************************************************/

class CameraAnglesMeta;
class CameraAnglesGui : public MetaGui
{
  Q_OBJECT
public:

  CameraAnglesGui(
    QString const    &heading,
    CameraAnglesMeta *meta,
    QGroupBox        *parent = nullptr);
  ~CameraAnglesGui() {}

  void setEnabled(bool enabled);

  virtual void apply(QString &modelName);

private:
  CameraAnglesMeta *meta;
  CameraAnglesData data;

  QLabel           *longitudeLabel;
  QLabel           *latitudeLabel;
  QLabel           *cameraViewLabel;
  QLineEdit        *longitudeEdit;
  QLineEdit        *latitudeEdit;
  QComboBox        *cameraViewCombo;
  QCheckBox        *homeViewpointBox;

  QAction *setLongitudeResetAct;
  QAction *setLatitudeResetAct;

private slots:
  void enableReset(QString const &);
  void lineEditReset();

public slots:
  void longitudeChange(QString const &);
  void latitudeChange(QString const &);
  void homeViewpointChanged(int);
  void cameraViewChange(int);
};

/***********************************************************************
 *
 * CameraFOV
 *
 **********************************************************************/

class CameraFOVGui : public MetaGui
{
  Q_OBJECT
public:

  CameraFOVGui(
    QString const &heading,
    FloatMeta     *meta,
    QGroupBox     *parent = nullptr);
  ~CameraFOVGui() {}

  void setEnabled(bool enabled);

  virtual void apply(QString &modelName);

private:
  float           data;
  FloatMeta      *meta;
  QLabel         *label;
  QDoubleSpinBox *spin;
  QPushButton    *button;

private slots:
  void enableReset(double);
  void spinReset(bool);

public slots:
  void valueChanged(double);
};

/***********************************************************************
 *
 * CameraPlane
 *
 **********************************************************************/

class CameraZPlaneGui : public MetaGui
{
  Q_OBJECT
public:

  CameraZPlaneGui(
    QString const &heading,
    FloatMeta     *meta,
    bool           zfar = false,
    QGroupBox     *parent = nullptr);
  ~CameraZPlaneGui() {}

  void setEnabled(bool enabled);

  virtual void apply(QString &modelName);

private:
  float           data;
  FloatMeta      *meta;
  QLabel         *label;
  QDoubleSpinBox *spin;
  QPushButton    *button;

private slots:
  void enableReset(double);
  void spinReset(bool);

public slots:
  void valueChanged(double);
};

/***********************************************************************
 *
 * CameraDDF
 *
 **********************************************************************/

class CameraDDFGui : public MetaGui
{
  Q_OBJECT
public:

  CameraDDFGui(
    QString const &heading,
    FloatMeta     *meta,
    QGroupBox     *parent = nullptr);
  ~CameraDDFGui() {}

  void setEnabled(bool enabled);

  virtual void apply(QString &modelName);

private:
  float           data;
  FloatMeta      *meta;
  QLabel         *label;
  QDoubleSpinBox *spin;
  QPushButton    *button;

private slots:
  void enableReset(double);
  void spinReset(bool);

public slots:
  void valueChanged(double);
};

/***********************************************************************
 *
 * Scale
 *
 **********************************************************************/

class ScaleGui : public MetaGui
{
  Q_OBJECT
public:

  ScaleGui(
    QString const &heading,
    FloatMeta     *meta,
    QGroupBox     *parent = nullptr);
  ~ScaleGui() {}

  void setEnabled(bool enabled);

  virtual void apply(QString &modelName);

private:
  float           data;
  FloatMeta      *meta;
  QLabel         *label;
  QDoubleSpinBox *spin;
  QPushButton    *button;

private slots:
  void enableReset(double);
  void spinReset(bool);

public slots:
  void valueChanged(double);
};

/***********************************************************************
 *
 * Show Submodel
 *
 **********************************************************************/

class ShowSubModelGui : public MetaGui
{
  Q_OBJECT
public:

  ShowSubModelGui(
          SubModelMeta *_meta,
          QGroupBox    *parent = nullptr);
  ~ShowSubModelGui() {}

  virtual void apply(QString &topLevelFile);

private:
  SubModelMeta *meta;

  QCheckBox    *showSubmodelsBox;
  QCheckBox    *showSubmodelsDefaultBox;
  QCheckBox    *showSubmodelsMetaBox;

  QCheckBox    *showTopModelBox;
  QCheckBox    *showTopModelDefaultBox;
  QCheckBox    *showTopModelMetaBox;

  QCheckBox    *showSubmodelInCalloutBox;
  QCheckBox    *showSubmodelInCalloutDefaultBox;
  QCheckBox    *showSubmodelInCalloutMetaBox;

  QCheckBox    *showInstanceCountBox;
  QCheckBox    *showInstanceCountDefaultBox;
  QCheckBox    *showInstanceCountMetaBox;

  QPushButton  *placementButton;

  bool          showSubmodelsModified;
  bool          showTopModelModified;
  bool          showSubmodelInCalloutModified;
  bool          showInstanceCountModified;
  bool          placementModified;

  bool          showSubmodelsDefaultSettings;
  bool          showTopModelDefaultSettings;
  bool          showSubmodelInCalloutDefaultSettings;
  bool          showInstanceCountDefaultSettings;

signals:
  void instanceCountClicked(bool);

public slots:
  void enableSubmodelControls(bool checked);
  void showSubmodelsChange(bool checked);
  void showTopModelChange(bool checked);
  void showSubmodelInCalloutChange(bool checked);
  void showInstanceCountChange(bool checked);
  void placementChanged(bool);
};

/***********************************************************************
 *
 * PliSort
 *
 **********************************************************************/

class PliSortMeta;
class PliSortGui : public MetaGui
{
  Q_OBJECT
public:

  PliSortGui(
      QString const &heading,
      PliSortMeta   *meta,
      QGroupBox     *parent = nullptr,
      bool           bom = false);
  ~PliSortGui() {}

  virtual void apply(QString &topLevelFile);

private:
  QLabel       *headingLabel;
  QComboBox    *combo;
  PliSortMeta  *meta;
  QString       sortOption;

public slots:
  void optionChange(QString const &);
};

/***********************************************************************
 *
 * PliSortOrder
 *
 **********************************************************************/

class PliSortOrderMeta;
class PliSortOrderGui : public MetaGui
{
  Q_OBJECT
public:

  PliSortOrderGui(
      QString const    &heading,
      PliSortOrderMeta *meta,
      QGroupBox        *parent = nullptr,
      bool              bom = false);
  ~PliSortOrderGui() {}

  void duplicateOption(QComboBox *box, bool resetOption = false, bool resetText = false);

  virtual void apply(QString &topLevelFile);

private:
  PliSortOrderMeta *meta;

  QLabel           *headingLabel;

  QComboBox        *primaryCombo;
  QComboBox        *secondaryCombo;
  QComboBox        *tertiaryCombo;

  QGroupBox        *gbPrimary;
  QGroupBox        *gbSecondary;
  QGroupBox        *gbTertiary;

  QRadioButton     *primaryAscendingRadio;
  QRadioButton     *primaryDescendingRadio;
  QRadioButton     *secondaryAscendingRadio;
  QRadioButton     *secondaryDescendingRadio;
  QRadioButton     *tertiaryAscendingRadio;
  QRadioButton     *tertiaryDescendingRadio;

  QString          sortOption;
  QString          sortDirection;

  bool             bom;
  bool             primaryModified;
  bool             secondaryModified;
  bool             tertiaryModified;
  bool             primaryDirectionModified;
  bool             secondaryDirectionModified;
  bool             tertiaryDirectionModified;

  bool             primaryDuplicateOption;
  bool             secondaryDuplicateOption;
  bool             tertiaryDuplicateOption;

public slots:
  void orderChange(int);
  void directionChange(bool);
};

/***********************************************************************
 *
 * PliPartElements
 *
 **********************************************************************/

class PliPartElementMeta;
class PliPartElementGui : public MetaGui
{
  Q_OBJECT
public:
  bool        displayModified;
  bool        bricklinkElementsModified;
  bool        legoElementsModified;
  bool        localLegoElementsModified;


  PliPartElementGui(
      QString const       &heading,
      PliPartElementMeta  *meta,
      QGroupBox           *parent = nullptr);
  ~PliPartElementGui() {}

  virtual void apply(QString &topLevelFile);

private:
  QLabel            *headingLabel;
  QRadioButton      *bricklinkElementsButton;
  QRadioButton      *legoElementsButton;
  QCheckBox         *localLegoElementsCheck;
  QGroupBox         *gbPliPartElement;

  PliPartElementMeta *meta;

signals:
  void toggled(bool);

public slots:
  void bricklinkElements(bool);
  void legoElements(bool);
  void localLegoElements(bool);

  void gbToggled(bool toggled);
  void enablePliPartElementGroup(bool);
};


/***********************************************************************
 *
 * PliAnnotation
 *
 **********************************************************************/

class PliAnnotationMeta;
class PliAnnotationGui : public MetaGui
{
  Q_OBJECT
public:
  bool displayModified;
  bool enableStyleModified;
  bool titleModified;
  bool freeformModified;
  bool titleAndFreeformModified;
  bool fixedAnnotationsModified;
  bool axleStyleModified;
  bool beamStyleModified;
  bool cableStyleModified;
  bool connectorStyleModified;
  bool elementStyleModified;
  bool extendedStyleModified;
  bool hoseStyleModified;
  bool panelStyleModified;

  PliAnnotationGui(
      QString const       &heading,
      PliAnnotationMeta   *meta,
      QGroupBox           *parent = nullptr,
      bool                 bom = false);
  ~PliAnnotationGui() {}

  virtual void apply(QString &topLevelFile);

private:
  QLabel            *headingLabel;
  QCheckBox         *titleAnnotationCheck;
  QCheckBox         *freeformAnnotationCheck;
  QGroupBox         *gbPLIAnnotationSource;
  QCheckBox         *fixedAnnotationsCheck;
  QCheckBox         *axleStyleCheck;
  QCheckBox         *beamStyleCheck;
  QCheckBox         *cableStyleCheck;
  QCheckBox         *connectorStyleCheck;
  QCheckBox         *elementStyleCheck;
  QCheckBox         *extendedStyleCheck;
  QCheckBox         *hoseStyleCheck;
  QCheckBox         *panelStyleCheck;

  QGroupBox         *gbPLIAnnotationType;
  PliAnnotationMeta *meta;

signals:
  void toggled(bool);

public slots:
  void titleAnnotation(bool);
  void freeformAnnotation(bool);
  void titleAndFreeformAnnotation(bool);
  void fixedAnnotations(bool);
  void axleStyle(bool);
  void beamStyle(bool);
  void cableStyle(bool);
  void connectorStyle(bool);
  void elementStyle(bool);
  void extendedStyle(bool);
  void hoseStyle(bool);
  void panelStyle(bool);

  void gbToggled(bool);
  void gbStyleToggled(bool);
  void enableElementStyle(bool);
  void enableExtendedStyle();
  void enableAnnotations();
  void setFixedAnnotations(bool);
};

/***********************************************************************
 *
 * CsiAnnotation
 *
 **********************************************************************/

class CsiAnnotationMeta;
class CsiAnnotationGui : public MetaGui
{
  Q_OBJECT
public:
  bool displayModified;
  bool axleDisplayModified;
  bool beamDisplayModified;
  bool cableDisplayModified;
  bool connectorDisplayModified;
  bool extendedDisplayModified;
  bool hoseDisplayModified;
  bool panelDisplayModified;
  bool placementModified;

  CsiAnnotationGui(
      QString const     &heading,
      CsiAnnotationMeta *meta,
      QGroupBox         *parent = nullptr,
      bool               fixedAnnotations = true);
  ~CsiAnnotationGui() {}

  virtual void apply(QString &topLevelFile);

private:
  QLabel            *headingLabel;
  QGroupBox         *gbPlacement;
  QGroupBox         *gbCSIAnnotationType;
  QCheckBox         *axleDisplayCheck;
  QCheckBox         *beamDisplayCheck;
  QCheckBox         *cableDisplayCheck;
  QCheckBox         *connectorDisplayCheck;
  QCheckBox         *extendedDisplayCheck;
  QCheckBox         *hoseDisplayCheck;
  QCheckBox         *panelDisplayCheck;
  QPushButton       *placementButton;

  CsiAnnotationMeta *meta;

public slots:
  void axleDisplay(bool);
  void beamDisplay(bool);
  void cableDisplay(bool);
  void connectorDisplay(bool);
  void extendedDisplay(bool);
  void hoseDisplay(bool);
  void panelDisplay(bool);
  void placementChanged(bool);
  void gbToggled(bool checked);
};

/***********************************************************************
 *
 * Page Orientation
 *
 **********************************************************************/

class PageOrientationGui : public MetaGui
{
  Q_OBJECT
public:

  PageOrientationGui(
    QString const           &heading,
    PageOrientationMeta    *_meta,
    QGroupBox              *parent = nullptr);
  ~PageOrientationGui() {}

  virtual void apply(QString &topLevelFile);

private:
  PageOrientationMeta    *meta;
  QLabel                 *label;
  QRadioButton           *portraitRadio;
  QRadioButton           *landscapeRadio;
  QGroupBox              *orientation;

public slots:
  void orientationChange(bool);
};

/***********************************************************************
 *
 * Rotate Icon Size
 *
 **********************************************************************/

class RotateIconSizeGui : public MetaGui
{
  Q_OBJECT
public:

  RotateIconSizeGui(
    QString const           &heading,
    UnitsMeta              *_meta,
    QGroupBox              *parent = nullptr);
  ~RotateIconSizeGui() {}

  void setEnabled(bool enabled);

  virtual void apply(QString &topLevelFile);

private:
  float                   dataW;
  float                   dataH;
  QLineEdit              *valueW;
  QLineEdit              *valueH;
  QAction                *resetWAct;
  QAction                *resetHAct;

  UnitsMeta              *meta;
  QLabel                 *label;
  QLabel                 *labelW;
  QLabel                 *labelH;
  QGroupBox              *size;

private slots:
  void enableReset(QString const &);
  void lineEditReset();

public slots:
  void valueWChange(QString const &);
  void valueHChange(QString const &);
};

/***********************************************************************
 *
 * Page Size
 *
 **********************************************************************/

class PageSizeGui : public MetaGui
{
  Q_OBJECT
public:

  PageSizeGui(
    QString const           &heading,
    PageSizeMeta           *_meta,
    QGroupBox              *parent = nullptr);
  ~PageSizeGui() {}

  int  getTypeIndex(float &pgWidth, float &pgHeight);
  void setEnabled(bool enabled);

  virtual void apply(QString &topLevelFile);

private:
  float                   dataW;
  float                   dataH;
  PageSizeMeta           *meta;
  QLabel                 *label;
  QGroupBox              *size;
  QLineEdit              *valueW;
  QLineEdit              *valueH;
  QAction                *resetHAct;
  QAction                *resetWAct;
  QComboBox              *sizeCombo;

private slots:
  void enableReset(QString const &);
  void lineEditReset();

public slots:
  void typeChange(QString const &);
  void valueWChange(QString const &);
  void valueHChange(QString const &);
};

/***********************************************************************
 *
 * Page Size And Orientation
 *
 **********************************************************************/

class SizeAndOrientationGui : public MetaGui
{
  Q_OBJECT
public:

  SizeAndOrientationGui(
    QString const           &heading,
    PageSizeMeta           *_metaS,
    PageOrientationMeta    *_metaO,
    QGroupBox              *parent = nullptr);
  ~SizeAndOrientationGui() {}

  void setEnabled(bool enabled);

  virtual void apply(QString &topLevelFile);

  bool                    sizeModified;
  bool                    sizeIDModified;
  bool                    orientationModified;
private:

  PageSizeData            dataS;
  OrientationEnc          dataO;

  PageSizeMeta           *metaS;
  PageOrientationMeta    *metaO;
  QLabel                 *label;
  QRadioButton           *portraitRadio;
  QRadioButton           *landscapeRadio;
  QGroupBox              *orientation;
  QGroupBox              *size;

  QLineEdit              *valueW;
  QLineEdit              *valueH;
  QAction                *resetWAct;
  QAction                *resetHAct;
  QComboBox              *typeCombo;

private slots:
  void enableReset(QString const &);
  void lineEditReset();

public slots:
  void typeChange(QString const &);
  void valueWChange(QString const &);
  void valueHChange(QString const &);
  void orientationChange(bool);
};

/***********************************************************************
 *
 * SubModelColor
 *
 **********************************************************************/

class StringListMeta;
class SubModelColorGui : public MetaGui
{
  Q_OBJECT
  Q_ENUMS(SubModCol)

public:

  enum SubModCol { Level1, Level2, Level3, Level4 };

  SubModelColorGui(
  StringListMeta *meta,
  QGroupBox  *parent = nullptr);
  ~SubModelColorGui() {}

  virtual void apply(QString &modelName);

private:
  StringListMeta  *meta;

  bool        subModelColorModified;

  QLabel      *subModelColor0Label;
  QLabel      *subModelColor0Example;
  QPushButton *subModelColor0Button;

  QLabel      *subModelColor1Label;
  QLabel      *subModelColor1Example;
  QPushButton *subModelColor1Button;

  QLabel      *subModelColor2Label;
  QLabel      *subModelColor2Example;
  QPushButton *subModelColor2Button;

  QLabel      *subModelColor3Label;
  QLabel      *subModelColor3Example;
  QPushButton *subModelColor3Button;

public slots:
  void browseSubModelColor0(bool);
  void browseSubModelColor1(bool);
  void browseSubModelColor2(bool);
  void browseSubModelColor3(bool);
};

/***********************************************************************
 *
 * Universal Rotate Step and Target
 *
 **********************************************************************/

class TargetRotateDialogGui : public QObject
{
  Q_OBJECT
public:
  TargetRotateDialogGui(){}
  void getTargetAndRotateValues(QStringList & keyList);
};

/***********************************************************************
 *
 * Open With Program
 *
 **********************************************************************/

class OpenWithProgramDialogGui : public QWidget
{
  Q_OBJECT
public:
  OpenWithProgramDialogGui();
  ~OpenWithProgramDialogGui() {}

  void setOpenWithProgram();
  void setProgramEntry(int i);
  void validateProgramEntries();

  QIcon getProgramIcon(const QString &programPath);

public slots:
  void adjustWidget();
  void browseOpenWithProgram(bool);
  void browseSystemEditor(bool);
  void maxProgramsValueChanged(int);

private:
  int                  maxPrograms;
  QStringList          programEntries;
  QGridLayout         *programsLayout;
  QGridLayout         *systemEditorLayout;
  QDialog             *dialog;
  QLineEdit           *systemEditorEdit;
  QPushButton         *systemEditorButton;
  QList<QLabel *>      programIconList;
  QList<QLineEdit *>   programNameEditList;
  QList<QLineEdit *>   programPathEditList;
  QList<QPushButton *> programBrowseButtonList;
};

/***********************************************************************
 *
 * Build Modifications
 *
 **********************************************************************/

class BuildModDialogGui : public QObject
{
  Q_OBJECT
public:
  BuildModDialogGui(){}
  void getBuildMod(QStringList & buildModKeys, int action);

public slots:
  void setBuildModActive(QListWidgetItem *item);

private:
  QListWidgetItem *activeBuildModItem;
};

/***********************************************************************
 *
 * POVRay renderer
 *
 **********************************************************************/

class POVRayRenderDialogGui : public QObject
{
  Q_OBJECT
public:
  POVRayRenderDialogGui(){}
  ~POVRayRenderDialogGui(){}

  void getRenderSettings(QStringList &csiKeyList,
                         int       &width,
                         int       &height,
                         int       &quality,
                         bool      &alpha);
  static int numSettings();

public slots:
  void setLookAtTargetAndRotate();
  void setLdvExportSettings();
  void setLdvLDrawPreferences();
  void resetSettings();
  void textChanged(const QString &value);

private:
  struct PovraySettings {
      QString label;
      QString tooltip;
  };

  enum POVRayRenLabelType
  {
    LBL_ALPHA,               // 0  QCheckBox
    LBL_ASPECT,              // 1  QCheckBox
    LBL_WIDTH,               // 2  QLineEdit
    LBL_HEIGHT,              // 3  QLineEdit
    LBL_LATITUDE,            // 4  QLineEdit
    LBL_LONGITUDE,           // 5  QLineEdit
    LBL_RESOLUTION,          // 6  QLineEdit
    LBL_SCALE,               // 7  QLineEdit
    LBL_QUALITY,             // 8  QComboBox
    LBL_TARGET_AND_ROTATE,   // 9  QToolButton
    LBL_LDV_EXPORT_SETTINGS, // 10 QToolButton
    LBL_LDV_LDRAW_SETTINGS,  // 11 QToolButton

    NUM_SETTINGS
  };

  enum POVRayRenControlType
  {                                          // Index
    WIDTH_EDIT,                              // 0
    ALPHA_BOX  = WIDTH_EDIT,                 // 0
    TARGET_BTN = WIDTH_EDIT,                 // 0
    HEIGHT_EDIT,                             // 1
    ASPECT_BOX = HEIGHT_EDIT,                // 1
    LDV_EXPORT_SETTINGS_BTN = HEIGHT_EDIT,   // 1
    LATITUDE_EDIT,                           // 2
    LDV_LDRAW_SETTINGS_BTN  = LATITUDE_EDIT, // 2
    LONGITUDE_EDIT,                          // 3
    RESOLUTION_EDIT,                         // 4
    SCALE_EDIT                               // 5
  };

  static PovraySettings povraySettings [];

  QComboBox *qualityCombo;
  QList<QLabel *> settingLabelList;
  QList<QCheckBox *> checkBoxList;
  QList<QLineEdit *> lineEditList;
  QList<QToolButton *> toolButtonList;

  QStringList mCsiKeyList, editedCsiKeyList;

  int mWidth;
  int mHeight;
  int mQuality;
};

/***********************************************************************
 *
 * Blender renderer
 *
 **********************************************************************/

class BlenderRenderDialogGui : public QWidget
{
  Q_OBJECT
public:
  BlenderRenderDialogGui(){}
  ~BlenderRenderDialogGui(){}

  void getRenderSettings(int &width,
                         int &height,
                         double &scale,
                         bool docRender);

  static void loadSettings();
  static void saveSettings();

  static int numPaths(bool = false);
  static int numSettings(bool = false);
  static int numComboOptItems();
  static bool extractBlenderAddon(const QString &);
  static void loadDefaultParameters(QByteArray& Buffer, int Which);
  static bool exportParameterFile();
  static bool overwriteFile(const QString &file);

protected:
  bool settingsModified(int &width, int &height, double &scale);

public slots:
  void resetSettings();
  void browseBlender(bool);
  void configureBlender();
  void updateLDrawAddon();
  void showPathsGroup();
  void sizeChanged(const QString &);
  bool promptAccept();
  void setDefaultColor(int value) const;
  void colorButtonClicked(bool);
  void setModelSize(bool);
  void validateColourScheme(QString const &);

  void update();
  void reject();
  void readStdOut();
  void writeStdOut();
  bool promptCancel();
  void showResult();
  QString readStdErr(bool &hasError) const;
  void showMessage(const QString &message);
  void statusUpdate(bool ok = false, const QString &message = QString());

private:
  QFormLayout *blenderForm;
  QWidget     *blenderContent;
  QHBoxLayout *blenderVersionHLayout;

  QGroupBox   *blenderPathsBox;
  QGroupBox   *blenderSettingsBox;
  QLineEdit   *blenderVersionEdit;
  QLabel      *blenderLabel;

  QDialog      *dialog;
  QProgressBar *progressBar;
  QProcess     *process;
  QTimer       updateTimer;
  QStringList  stdOutList;
  QString      versionText;
  int          lineCount;

  enum BlenderRenPathType
  {
    LBL_BLENDER_PATH,                       //  0 QLineEdit/QPushButton
    LBL_BLENDFILE_PATH,                     //  1 QLineEdit/QPushButton
    LBL_ENVIRONMENT_PATH,                   //  2 QLineEdit/QPushButton
    LBL_LDCONFIG_PATH,                      //  3 QLineEdit/QPushButton
    LBL_LDRAW_DIRECTORY,                    //  4 QLineEdit/QPushButton
    LBL_LSYNTH_DIRECTORY,                   //  5 QLineEdit/QPushButton
    LBL_STUDLOGO_DIRECTORY,                 //  6 QLineEdit/QPushButton

    NUM_BLENDER_PATHS
  };

  enum BlenderRenLabelType
  {
    LBL_ADD_ENVIRONMENT,                    //  0   QCheckBox
    LBL_ADD_GAPS,                           //  1   QCheckBox
    LBL_BEVEL_EDGES,                        //  2   QCheckBox
    LBL_BLENDFILE_TRUSTED,                  //  3   QCheckBox
    LBL_CROP_IMAGE,                         //  4   QCheckBox
    LBL_CURVED_WALLS,                       //  5   QCheckBox
    LBL_FLATTEN_HIERARCHY,                  //  6   QCheckBox
    LBL_IMPORT_CAMERAS,                     //  7   QCheckBox
    LBL_IMPORT_LIGHTS,                      //  8   QCheckBox
    LBL_INSTANCE_STUDS,                     //  9   QCheckBox
    LBL_KEEP_ASPECT_RATIO,                  // 10   QCheckBox
    LBL_LINK_PARTS,                         // 11   QCheckBox
    LBL_NUMBER_NODES,                       // 12   QCheckBox
    LBL_OVERWRITE_IMAGE,                    // 13   QCheckBox
    LBL_OVERWRITE_MATERIALS,                // 14   QCheckBox
    LBL_OVERWRITE_MESHES,                   // 15   QCheckBox
    LBL_POSITION_CAMERA,                    // 16   QCheckBox
    LBL_REMOVE_DOUBLES,                     // 17   QCheckBox
    LBL_RENDER_WINDOW,                      // 18   QCheckBox
    LBL_SEARCH_ADDL_PATHS,                  // 19   QCheckBox
    LBL_SMOOTH_SHADING,                     // 20   QCheckBox
    LBL_TRANSPARENT_BACKGROUND,             // 21   QCheckBox
    LBL_UNOFFICIAL_PARTS,                   // 22   QCheckBox
    LBL_USE_LOGO_STUDS,                     // 23   QCheckBox
    LBL_VERBOSE,                            // 24   QCheckBox

    LBL_BEVEL_WIDTH,                        // 25/0 QLineEdit
    LBL_CAMERA_BORDER_PERCENT,              // 26/1 QLineEdit
    LBL_DEFAULT_COLOUR,                     // 27/2 QLineEdit
    LBL_GAPS_SIZE,                          // 28/3 QLineEdit
    LBL_IMAGE_WIDTH,                        // 29/4 QLineEdit
    LBL_IMAGE_HEIGHT,                       // 30/5 QLineEdit
    LBL_IMAGE_SCALE,                        // 31/6 QLineEdit

    LBL_COLOUR_SCHEME,                      // 32/0 QComboBox
    LBL_FLEX_PARTS_SOURCE,                  // 33/1 QComboBox
    LBL_LOGO_STUD_VERSION,                  // 34/2 QComboBox
    LBL_LOOK,                               // 25/3 QComboBox
    LBL_POSITION_OBJECT,                    // 36/4 QComboBox
    LBL_RESOLUTION,                         // 37/5 QComboBox
    LBL_RESOLVE_NORMALS,                    // 38/6 QComboBox

    NUM_SETTINGS
  };

  enum BlenderRenControlType
  {                                                    // Index
    BLENDER_PATH_EDIT,                               // 0
    ADD_ENVIRONMENT_BOX     = BLENDER_PATH_EDIT,     // 0
    BEVEL_WIDTH_EDIT        = BLENDER_PATH_EDIT,     // 0
    COLOUR_SCHEME_COMBO     = BLENDER_PATH_EDIT,     // 0
    ADD_GAPS_BOX,                                    // 1
    CAMERA_BORDER_PERCENT_EDIT = ADD_GAPS_BOX,       // 1
    FLEX_PARTS_SOURCE_COMBO = ADD_GAPS_BOX,          // 1
    DEFAULT_SETTINGS        = ADD_GAPS_BOX,          // 1
    BEVEL_EDGES_BOX,                                 // 2
    DEFAULT_COLOUR_EDIT     = BEVEL_EDGES_BOX,       // 2
    LOGO_STUD_VERSION_COMBO = BEVEL_EDGES_BOX,       // 2
    BLENDFILE_TRUSTED_BOX,                           // 3
    GAPS_SIZE_EDIT          = BLENDFILE_TRUSTED_BOX, // 3
    LOOK_COMBO              = BLENDFILE_TRUSTED_BOX, // 3
    CROP_IMAGE_BOX,                                  // 4
    IMAGE_SCALE_EDIT        = CROP_IMAGE_BOX,        // 4
    POSITION_OBJECT_COMBO   = CROP_IMAGE_BOX,        // 4
    IMAGE_WIDTH_EDIT        = CROP_IMAGE_BOX,        // 4
    CURVED_WALLS_BOX,                                // 5
    RESOLUTION_COMBO        = CURVED_WALLS_BOX,      // 5
    IMAGE_HEIGHT_EDIT       = CURVED_WALLS_BOX,      // 5
    FLATTEN_HIERARCHY_BOX,                           // 6
    RESOLVE_NORMALS_COMBO   = FLATTEN_HIERARCHY_BOX, // 6
    LINE_EDIT_ITEMS         = FLATTEN_HIERARCHY_BOX, // 6
    COMBO_BOX_ITEMS         = FLATTEN_HIERARCHY_BOX, // 6
    IMPORT_CAMERAS_BOX,                              // 7
    IMPORT_LIGHTS_BOX,                               // 8
    INSTANCE_STUDS_BOX,                              // 9
    KEEP_ASPECT_RATIO_BOX,                           //10
    LINK_PARTS_BOX,                                  //11
    NUMBER_NODES_BOX,                                //12
    OVERWRITE_IMAGE_BOX,                             //13
    OVERWRITE_MATERIALS_BOX,                         //14
    OVERWRITE_MESHES_BOX,                            //15
    POSITION_CAMERA_BOX,                             //16
    REMOVE_DOUBLES_BOX,                              //17
    RENDER_WINDOW_BOX,                               //18
    SEARCH_ADDL_PATHS_BOX,                           //19
    SMOOTH_SHADING_BOX,                              //20
    TRANSPARENT_BACKGROUND_BOX,                      //21
    UNOFFICIAL_PARTS_BOX,                            //22
    USE_LOGO_STUDS_BOX,                              //23
    VERBOSE_BOX                                      //24
  };

  enum BlenderRenBrickType
  {
     PARAMS_CUSTOM_COLOURS,
     PARAMS_SLOPED_BRICKS,
     PARAMS_LIGHTED_BRICKS
  };

  struct BlenderSettings
  {
      QString key;
      QString value;
      QString label;
      QString tooltip;
  };

  struct ComboOptItems
  {
      QString data;
      QString items;
  };

  static BlenderSettings blenderSettings [];
  static BlenderSettings defaultSettings [];
  static BlenderSettings blenderPaths [];
  static BlenderSettings defaultPaths [];
  static ComboOptItems comboOptItems [];

  static bool    documentRender;
  static QString blenderVersion;
  static QString searchDirectoriesKey;
  static QString parameterFileKey;

  QAction     *defaultColourEditAction;
  QAction     *blenderPathEditAction;
  QPushButton *pathsGroupButton;

  QList<QLabel *> settingLabelList;
  QList<QLineEdit   *> sizeLineEditList;
  QList<QLineEdit   *> pathLineEditList;
  QList<QPushButton *> pathBrowseButtonList;
  QList<QCheckBox *> checkBoxList;
  QList<QLineEdit *> lineEditList;
  QList<QComboBox *> comboBoxList;

  bool mBlenderConfigured;
  bool mBlenderAddonUpdate;
  int mWidth;
  int mHeight;
  qreal mScale;
};

#endif
