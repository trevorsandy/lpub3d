
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

void clearPliCache();
void clearAndReloadModelFile();
void clearCsiCache();
void clearSubmodelCache();
void clearPliCache();
void clearTempCache();

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

protected:
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

  QCheckBox *getCheckbox() {return check;}

  void setEnabled(bool enabled);

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
    QGroupBox      *parent = nullptr);
  ~UnitsGui() {}

  void setEnabled(bool enabled);

  virtual void apply(QString &modelName);

private:
  UnitsMeta *meta;
  QLabel    *label;
  QLabel    *label2;
  QLineEdit *value0;
  QLineEdit *value1;

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

private:
  bool          showPair;
  FloatPairMeta *meta;
  QLabel        *label0;
  QLabel        *label1;
  QLineEdit     *value0;
  QLineEdit     *value1;

public slots:
  void value0Change(QString const &);
  void value1Change(QString const &);
};

/***********************************************************************
 *
 * Combo Box
 *
 **********************************************************************/

class IntMeta;
class QComboBox;
class ComboGui : public MetaGui
{
  Q_OBJECT
public:

  ComboGui(
    QString const &heading,
    QString const &namedValues,
    IntMeta       *meta,
    QGroupBox     *parent = nullptr,
    bool           useCheck = false);
  ~ComboGui() {}

  virtual void apply(QString &modelName);

private:
  IntMeta   *meta;
  QLabel    *label;
  QCheckBox *check;
  QComboBox *combo;
  bool       useCheck;

public slots:
  void valueChanged(int);
  void valueChanged(bool);
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
  NumberGui(NumberMeta *meta,
    QGroupBox  *_parent = nullptr, QString title = "");
  ~NumberGui() {}

  virtual void apply(QString &modelName);

private:
  NumberMeta  *meta;

  bool        fontModified;
  bool        colorModified;
  bool        marginsModified;

  QGroupBox   *gbFormat;
  QLabel      *fontLabel;
  QLabel      *fontExample;
  QPushButton *fontButton;
  QLabel      *colorLabel;
  QLabel      *colorExample;
  QPushButton *colorButton;
  QLabel      *marginsLabel;
  QLineEdit   *value0;
  QLineEdit   *value1;

public slots:
  void browseFont(bool clicked);
  void browseColor(bool clicked);
  void value0Changed(QString const &);
  void value1Changed(QString const &);
  void enableTextFormatGroup(bool);
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

  QLabel      *fontLabel;
  QLabel      *fontExample;
  QPushButton *fontButton;
  QLabel      *colorLabel;
  QLabel      *colorExample;
  QPushButton *colorButton;
  QLabel      *marginsLabel;

  QLineEdit    *value0;
  QLineEdit    *value1;

  QCheckBox    *display;

  QGroupBox    *gbContentEdit;
  QGroupBox    *gbDescDialog;
  QGroupBox    *gbDiscDialog;

  QTextEdit    *editDesc;
  QTextEdit    *editDisc;
  QLineEdit    *contentEdit;
  QString      content;

  QGroupBox    *gbPlacement;
  QPushButton  *placementButton;

  QLabel       *sectionLabel;
  QComboBox    *sectionCombo;

signals:
  void indexChanged(int index);

public slots:
  void newIndex(int value){
      selection = value;
  }
  void editDescChanged();
  void editDiscChanged();
  void browseFont(bool clicked);
  void browseColor(bool clicked);
  void editChanged(QString const &);
  void value0Changed(QString const &);
  void value1Changed(QString const &);
  void placementChanged(bool);
  void toggled(bool toggled);

  friend class GlobalPageDialog;

};
/***********************************************************************
 *
 * PageAttributePicture
 *
 **********************************************************************/

class PageAttributePictureMeta;
class QDoubleSpinBox;
class FloatMeta;
class PageAttributePictureGui : public MetaGui
{
  Q_OBJECT
public:

  PageAttributePictureGui(
    PageAttributePictureMeta *meta,
    QGroupBox         *parent = nullptr);

  ~PageAttributePictureGui() {}

  virtual void apply(QString &topLevelFile);

  void setEnabled(bool enabled);

private:
  PageAttributePictureMeta *meta;

  int             selection;
  bool            marginsModified;
  bool            placementModified;
  bool            displayModified;
  bool            pictureModified;
  bool            scaleModified;
  bool            fillModified;

  QString         picture;

  QLabel         *marginsLabel;
  QLineEdit      *value0;
  QLineEdit      *value1;

  QLineEdit      *pictureEdit;
  QPushButton    *pictureButton;
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

public slots:
  void selectionChanged(int value){
      selection = value;
  }
  void pictureFill(bool);
  void pictureChange(QString const &);
  void browsePicture(bool);
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
  UnitsMeta *meta;
  QLabel    *label;
  QLineEdit *value0;
  QLineEdit *value1;

public slots:
  void value1Change(QString const &);
};

/***********************************************************************
 *
 * FadeStep
 *
 **********************************************************************/

class FadeStepMeta;
class FadeStepGui : public MetaGui
{
  Q_OBJECT
public:

  FadeStepGui(
    QString const &heading,
    FadeStepMeta *meta,
    QGroupBox  *parent = nullptr);
  ~FadeStepGui() {}

  virtual void apply(QString &modelName);

private:
  FadeStepMeta  *meta;

  QLabel      *colorLabel;
  QLabel      *colorExample;
  QLabel      *readOnlyLabel;
  QComboBox   *colorCombo;

public slots:
  void colorChange(QString const &colorName);
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
    QString const &heading,
    HighlightStepMeta *meta,
    QGroupBox  *parent = nullptr);
  ~HighlightStepGui() {}

  virtual void apply(QString &modelName);

private:
  HighlightStepMeta  *meta;

  QLabel      *colorLabel;
  QLabel      *colorExample;
  QLabel      *readOnlyLabel;
  QPushButton *colorButton;

public slots:
  void colorChange(bool clicked);
};

/***********************************************************************
 *
 * CameraDistFactor
 *
 **********************************************************************/

class NativeCDMeta;
class QSpinBox;
class CameraDistFactorGui : public MetaGui
{
  Q_OBJECT
public:

  CameraDistFactorGui(
    QString const &heading,
    NativeCDMeta *meta,
    QGroupBox  *parent = nullptr);
  ~CameraDistFactorGui() {}

  virtual void apply(QString &modelName);

private:
  NativeCDMeta  *meta;

  int       saveFactor;
  QSpinBox  *cameraDistFactorSpin;
  QLabel    *cameraDistFactorLabel;

public slots:
   void cameraDistFactorChange(int factor);
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

  RotStepGui(RotStepMeta   *meta,
    QGroupBox     *parent = nullptr);
  ~RotStepGui() {}

  virtual void apply(QString &modelName);

private:
  RotStepMeta     *meta;

  QDoubleSpinBox  *rotStepSpinX;
  QDoubleSpinBox  *rotStepSpinY;
  QDoubleSpinBox  *rotStepSpinZ;
  QComboBox       *typeCombo;

public slots:
   void rotStepChanged(double);
   void typeChanged(QString const &);
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

private:
  IntMeta      *meta;
  QLabel         *label;
  QSpinBox *spin;

public slots:
  void valueChanged(int);
};

/***********************************************************************
 *
 * Double Spin Box
 *
 **********************************************************************/

class FloatMeta;
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

private:
  FloatMeta      *meta;
  QLabel         *label;
  QDoubleSpinBox *spin;

public slots:
  void valueChanged(double);
};

/***********************************************************************
 *
 * ContStepNumMeta
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

  QCheckBox *getCheckbox() {return check;}

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
 * CountInstance
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

  QRadioButton *getTopRadio() {return topRadio;}
  QRadioButton *getModelRadio() {return modelRadio;}
  QRadioButton *getStepRadio() {return stepRadio;}

  void setEnabled(bool enabled);

  virtual void apply(QString &modelName);

private:
  CountInstanceMeta  *meta;
  QRadioButton       *topRadio;
  QRadioButton       *modelRadio;
  QRadioButton       *stepRadio;

public slots:
  void radioChanged(bool checked);
  void groupBoxChanged(bool checked);
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
  void pictureChange(QString const &);
  void browsePicture(bool);
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
  BorderMeta *meta;

  QLineEdit   *thicknessEdit;
  QLineEdit   *marginEditX;
  QLineEdit   *marginEditY;
  QLabel      *thicknessLabel;
  QLabel      *colorExample;
  QPushButton *colorButton;
  QSpinBox    *spin;
  QLabel      *spinLabel;
  QComboBox   *typeCombo;
  QComboBox   *lineCombo;
  QCheckBox   *hideArrowsChk;

  void enable(bool rotateArrow = false);

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
    PlacementMeta *_meta,
    QString        _title,
    QGroupBox     *parent    = nullptr);
  ~PlacementGui() {}

  virtual void apply(QString &modelName);

private:
  PlacementMeta  *meta;
  QLabel         *placementLabel;
  QPushButton    *placementButton;
  bool            placementModified;
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
  PointerAttribGui(PointerAttribMeta *meta,
    QGroupBox         *parent    = nullptr,
    bool               _isCallout= false);
  ~PointerAttribGui() {}

  virtual void apply(QString &modelName);

private:
  bool        lineData;
  PointerAttribMeta *meta;

  QLineEdit   *thicknessEdit;
  QLabel      *thicknessLabel;
  QLabel      *colorExample;
  QPushButton *colorButton;
  QComboBox   *lineCombo;
  QCheckBox   *hideTipBox;

public slots:
  void lineChange(QString const &);
  void thicknessChange(QString const &);
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

  QLabel    *headingLabel;
  QComboBox *combo;
  QLineEdit *value;

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
  QLabel    *colorExample;
  QLineEdit *typeLineEdit;

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
  ResolutionType  type;
  float           value;
  QLineEdit      *valueEdit;
  QIntValidator  *validator;

  void differences();

public slots:
  void unitsChange(QString const &);
  void valueChange(QString const &);
};

/***********************************************************************
 *
 * Renderer
 *
 **********************************************************************/

class RendererGui : public MetaGui
{
  Q_OBJECT
public:
  bool          clearCaches;

  RendererGui(NativeCDMeta *_meta,
              QGroupBox *parent = nullptr);
  ~RendererGui() {}

  virtual void apply(QString &topLevelFile);

private:
  NativeCDMeta  *meta;

  QComboBox    *combo;
  QCheckBox    *ldvSingleCallBox;
  QCheckBox    *ldvSnapshotListBox;
  QCheckBox    *cameraDistFactorDefaultBox;
  QCheckBox    *cameraDistFactorMetaBox;
  QGroupBox    *povFileGeneratorGrpBox;
  QGroupBox    *cameraDistFactorGrpBox;
  QRadioButton *nativeButton;
  QRadioButton *ldvButton;

  QString       pick;
  QString       povFileGenChoice;

  int           cameraDistFactorNative;
  QSpinBox      *cameraDistFactorSpin;
  QLabel        *cameraDistFactorLabel;

  bool          rendererModified;
  bool          singleCallModified;
  bool          snapshotListModified;
  bool          povFileGenModified;
  bool          cameraDistFactorModified;
  bool          cameraDistFactorDefaulSetting;

public slots:
  void typeChange(QString const &);
  void singleCallChange(bool checked);
  void snapshotListChange(bool checked);
  void povFileGenNativeChange(bool checked);
  void povFileGenLDViewChange(bool checked);
  void cameraDistFactorChange(int factor);
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

  QCheckBox    *showInstanceCountBox;
  QCheckBox    *showInstanceCountDefaultBox;
  QCheckBox    *showInstanceCountMetaBox;

  QPushButton  *placementButton;

  bool          showSubmodelsModified;
  bool          showTopModelModified;
  bool          showInstanceCountModified;
  bool          placementModified;

  bool          showSubmodelsDefaultSettings;
  bool          showTopModelDefaultSettings;
  bool          showInstanceCountDefaultSettings;

signals:
  void instanceCountClicked(bool);

public slots:
  void enableSubmodelControls(bool checked);
  void showSubmodelsChange(bool checked);
  void showTopModelChange(bool checked);
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
  QGroupBox         *gbPLIAnnotation;
  QCheckBox         *fixedAnnotationsCheck;
  QCheckBox         *axleStyleCheck;
  QCheckBox         *beamStyleCheck;
  QCheckBox         *cableStyleCheck;
  QCheckBox         *connectorStyleCheck;
  QCheckBox         *elementStyleCheck;
  QCheckBox         *extendedStyleCheck;
  QCheckBox         *hoseStyleCheck;
  QCheckBox         *panelStyleCheck;

  QGroupBox         *gbPLIAnnotationStyle;
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
  QGroupBox         *gbCSIAnnotationDisplay;
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

  void updateRotateIconSize();
  void setEnabled(bool enabled);

  virtual void apply(QString &topLevelFile);

  QLineEdit              *valueW;
  QLineEdit              *valueH;

private:
  float                   w;
  float                   h;
  UnitsMeta              *meta;
  QLabel                 *label;
  QLabel                 *labelW;
  QLabel                 *labelH;
  QGroupBox              *size;

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
  void updatePageSize();
  void setEnabled(bool enabled);

  virtual void apply(QString &topLevelFile);

  QLineEdit              *valueW;
  QLineEdit              *valueH;
  QComboBox              *typeCombo;

private:
  float                   w;
  float                   h;
  PageSizeMeta           *meta;
  QLabel                 *label;
  QGroupBox              *size;


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
    PageSizeMeta           *_smeta,
    PageOrientationMeta    *_ometa,
    QGroupBox              *parent = nullptr);
  ~SizeAndOrientationGui() {}

  void setEnabled(bool enabled);

  virtual void apply(QString &topLevelFile);

private:
  bool                    sizeModified;
  bool                    orientationModified;
  float                   w;
  float                   h;
  PageSizeMeta           *smeta;
  PageOrientationMeta    *ometa;
  QLabel                 *label;
  QRadioButton           *portraitRadio;
  QRadioButton           *landscapeRadio;
  QGroupBox              *orientation;
  QGroupBox              *size;

  QLineEdit              *valueW;
  QLineEdit              *valueH;
  QComboBox              *typeCombo;

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

class OpenWithProgramDialogGui : public QObject
{
    Q_OBJECT
public:
    OpenWithProgramDialogGui();
    ~OpenWithProgramDialogGui() {}

    void setOpenWithProgram();
    void setProgramEntries();
    void validateProgramEntries();

public slots:
    void browseOpenWithProgram(bool);
    void maxProgramsValueChanged(int);

private:
    int                  maxPrograms;
    QStringList          programEntries;
    QGridLayout         *gridLayout;
    QDialog             *dialog;
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
    void getBuildMod(QStringList & buildModKeys, bool apply = true);

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

    enum {
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

    enum {                                       // Index
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

    enum {
        LBL_BLENDER_PATH,                       //  0 QLineEdit/QPushButton
        LBL_BLENDFILE_PATH,                     //  1 QLineEdit/QPushButton
        LBL_ENVIRONMENT_PATH,                   //  2 QLineEdit/QPushButton
        LBL_LDCONFIG_PATH,                      //  3 QLineEdit/QPushButton
        LBL_LDRAW_DIRECTORY,                    //  4 QLineEdit/QPushButton
        LBL_IMAGES_DIRECTORY,                   //  5 QLineEdit/QPushButton
        LBL_LSYNTH_DIRECTORY,                   //  6 QLineEdit/QPushButton
        LBL_STUDLOGO_DIRECTORY,                 //  7 QLineEdit/QPushButton

        NUM_BLENDER_PATHS
    };

    enum {
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

    enum {                                               // Index
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

    enum {
       PARAMS_CUSTOM_COLOURS,
       PARAMS_SLOPED_BRICKS,
       PARAMS_LIGHTED_BRICKS
    };

    struct BlenderSettings {
        QString key;
        QString value;
        QString label;
        QString tooltip;
    };

    struct ComboOptItems {
        QString valueStr;
        QString labelStr;
    };

    static BlenderSettings blenderSettings [];
    static BlenderSettings defaultSettings [];
    static BlenderSettings blenderPaths [];
    static BlenderSettings defaultPaths [];
    static ComboOptItems comboOptItems [];

    static QString ldrawDirectoryKey;
    static QString customLDConfigKey;
    static QString fadeStepsKey;
    static QString highlightStepKey;
    static QString searchDirectoriesKey;
    static QString parameterFileKey;
    static QString blenderVersion;

    static bool documentRender;

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
