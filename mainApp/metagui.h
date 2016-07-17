
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
 * This file describes a set of classes that implement graphical user
 * interfaces for some of the configuration meta classes described in meta.h
 *
 * Please see lpub.h for an overall description of how the files in LPub
 * make up the LPub program.
 *
 ***************************************************************************/

#ifndef metagui_h
#define metagui_h

#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
#include <QtWidgets/QWidget>
#else
#include <QWidget>
#endif
#include <QTextEdit>
#include "resolution.h"
#include "metatypes.h"
#include "gradients.h"

#include "QsLog.h"

void clearPliCache();

class QWidget;
class QString;
class QHBoxLayout;
class LeafMeta;
class UnitsMeta;

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
    QGroupBox      *parent = NULL);
  ~CheckBoxGui() {}

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
    QGroupBox     *parent = NULL);

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

class QGroupBox;
class QLabel;
class QLineEdit;
class UnitsGui : public MetaGui
{
  Q_OBJECT
public:

  UnitsGui(
    QString const  &heading,
    UnitsMeta      *meta,
    QGroupBox      *parent = NULL);
  ~UnitsGui() {}

  void setEnabled(bool enabled);

  virtual void apply(QString &modelName);

private:
  UnitsMeta *meta;
  QLabel    *label;
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
    QGroupBox     *parent = NULL);
  ~FloatsGui() {}

  void setEnabled(bool enabled);

  virtual void apply(QString &modelName);

private:
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
 * Number
 *
 **********************************************************************/

class NumberMeta;
class QGroupBox;
class QPushButton;
class NumberGui : public MetaGui
{
  Q_OBJECT
public:

  NumberGui(
    NumberMeta *meta,
    QGroupBox  *parent = NULL);
  ~NumberGui() {}

  virtual void apply(QString &modelName);

private:
  NumberMeta  *meta;

  bool        fontModified;
  bool        colorModified;
  bool        marginsModified;

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
};



/***********************************************************************
 *
 * PageAttributeText
 *
 **********************************************************************/

enum RelativeTos
{ fc = 1 ,   bc = 2,     ph  = 4,      pf  = 8};

class PageAttributeTextMeta;
class QGroupBox;
class QLineEdit;
class QPushButton;
class PlacementMeta;
class PageAttributePlacementEnum;
class GlobalPageDialog;
class PageAttributeTextGui : public MetaGui
{
  Q_OBJECT
public:

  PageAttributeTextGui(
    PageAttributeTextMeta *meta,
    QGroupBox  *parent = NULL);

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
      logInfo() << " New Index Received (Text MetaGui): "
                << " INDEX " << value;
                    ;
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

};
/***********************************************************************
 *
 * PageAttributePicture
 *
 **********************************************************************/

class PageAttributePictureMeta;
class QLabel;
class QPushButton;
class QLineEdit;
class QRadioButton;
class QGroupBox;
class QDoubleSpinBox;
class FloatMeta;
class PageAttributePictureGui : public MetaGui
{
  Q_OBJECT
public:

  PageAttributePictureGui(
    PageAttributePictureMeta *meta,
    QGroupBox         *parent = NULL);

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
  bool            stretchTileModified;

  QString         picture;

  QLabel         *marginsLabel;
  QLineEdit      *value0;
  QLineEdit      *value1;

  QLineEdit      *pictureEdit;
  QPushButton    *pictureButton;
//  QRadioButton   *stretchRadio;
//  QRadioButton   *tileRadio;
//  QGroupBox      *gbFill;
  QGroupBox      *gbScale;
  QLabel         *scale;
  QDoubleSpinBox *spin;

  QCheckBox      *display;

  QGroupBox      *gbPlacement;
  QPushButton    *placementButton;

  QLabel         *sectionLabel;
  QComboBox      *sectionCombo;

signals:
  void indexChanged(int index);

public slots:
  void selectionChanged(int value){
      logInfo() << " Selection Change Received (Picture MetaGui): "
                << " INDEX " << value
                    ;
      selection = value;
  }
  void pictureChange(QString const &);
  void browsePicture(bool);
//  void gbFillClicked(bool);
  void gbScaleClicked(bool);
//  void stretch(bool);
//  void tile(bool);

  void value0Changed(QString const &);
  void value1Changed(QString const &);

  void valueChanged(double);

  void placementChanged(bool);
  void toggled(bool toggled);
};

/***********************************************************************
 *
 * PageHeaderFooterHeight
 *
 **********************************************************************/

class QGroupBox;
class QLabel;
class QLineEdit;
class HeaderFooterHeightGui : public MetaGui
{
  Q_OBJECT
public:

  HeaderFooterHeightGui(
    QString const  &heading,
    UnitsMeta      *meta,
    QGroupBox      *parent = NULL);
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
class QGroupBox;
class QPushButton;
class FadeStepGui : public MetaGui
{
  Q_OBJECT
public:

  FadeStepGui(
    QString const &heading,
    FadeStepMeta *meta,
    QGroupBox  *parent = NULL);
  ~FadeStepGui() {}

  virtual void apply(QString &modelName);

private:
  FadeStepMeta  *meta;

  bool        colorModified;
  QLabel      *colorLabel;
  QComboBox   *colorCombo;
  QLabel      *colorExample;
  //QPushButton *colorButton;

public slots:
  void colorChange(QString const &colorName);
};
/***********************************************************************
 *
 * Spin Box
 *
 **********************************************************************/

class FloatMeta;
class QLabel;
class QDoubleSpinBox;
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
    QGroupBox     *parent = NULL);
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
 * Background
 *
 **********************************************************************/

class BackgroundMeta;
class QLabel;
class QPushButton;
class QLineEdit;
class QRadioButton;
class QGroupBox;

class BackgroundGui : public MetaGui
{
  Q_OBJECT
public:

  BackgroundGui(
    BackgroundMeta  *meta,
    QGroupBox       *parent = NULL);

  ~BackgroundGui() {}

  BackgroundMeta *meta;

  virtual void apply(QString &modelName);

private:
  QString picture;
  QString color;
  QString gradient;

  QLabel       *colorLabel;
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
class QSpinBox;
class BorderGui : public MetaGui
{
  Q_OBJECT
public:

  BorderGui(
    BorderMeta *meta,
    QGroupBox  *parent  = NULL,
    bool rotateArrow = false);
  ~BorderGui() {}

  virtual void apply(QString &modelName);

private:
  BorderMeta *meta;

  QLineEdit   *thicknessEdit;
  QLabel      *thicknessLabel;
  QLabel      *colorLabel;
  QPushButton *colorButton;
  QSpinBox    *spin;
  QLabel      *spinLabel;
  QComboBox   *combo;
  QComboBox   *lineCombo;

  void enable();

public slots:
  void typeChange(QString const &);
  void lineChange(QString const &);
  void thicknessChange(QString const &);
  void browseColor(bool);
  void radiusChange(int);
  void marginXChange(QString const &);
  void marginYChange(QString const &);
};

/***********************************************************************
 *
 * Constrain PLI
 *
 **********************************************************************/

class ConstrainMeta;
class QComboBox;
class ConstrainGui : public MetaGui
{
  Q_OBJECT
public:

  ConstrainGui(
    QString const &heading,
    ConstrainMeta *meta,
    QGroupBox     *parent = NULL);
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
 * Divider
 *
 **********************************************************************/

class SepMeta;
class SepGui : public MetaGui
{
  Q_OBJECT
public:

  SepGui(
    SepMeta *meta,
    QGroupBox     *parent = NULL);
  ~SepGui() {}

  virtual void apply(QString &modelName);

private:
  SepMeta *meta;
  QLabel  *colorExample;

public slots:
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
    QGroupBox      *parent = NULL);
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

  RendererGui(QGroupBox *parent = NULL);
  ~RendererGui() {}

  virtual void apply(QString &modelName);

private:
  QComboBox *combo;
  QString    pick;

public slots:
  void typeChange(QString const &);
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
      QGroupBox     *parent = NULL);
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
 * PliAnnotation
 *
 **********************************************************************/

class PliAnnotationMeta;
class QLabel;
class QRadioButton;
class PliAnnotationGui : public MetaGui
{
  Q_OBJECT
public:
  bool displayModified;
  bool titleModified;
  bool freeformModified;
  bool titleAndFreeformModified;

  PliAnnotationGui(
      QString const       &heading,
      PliAnnotationMeta   *meta,
      QGroupBox           *parent = NULL);
  ~PliAnnotationGui() {}

  virtual void apply(QString &topLevelFile);

private:
  QLabel            *headingLabel;
  QRadioButton      *titleAnnotationButton;
  QRadioButton      *freeformAnnotationButton;
  QRadioButton      *titleAndFreeformAnnotationButton;
  QGroupBox         *gbPLIAnnotation;
  PliAnnotationMeta *meta;

public slots:
  void titleAnnotation(bool);
  void freeformAnnotation(bool);
  void titleAndFreeformAnnotation(bool);
  void gbToggled(bool toggled);
};

/***********************************************************************
 *
 * Page Orientation
 *
 **********************************************************************/

class QGroupBox;
class QLabel;
class QRadioButton;
class PageOrientationGui : public MetaGui
{
  Q_OBJECT
public:

  PageOrientationGui(
    QString const           &heading,
    PageOrientationMeta    *_meta,
    QGroupBox              *parent = NULL);
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
 * Page Size
 *
 **********************************************************************/

class QGroupBox;
class QLabel;
class QLineEdit;
class PageSizeGui : public MetaGui
{
  Q_OBJECT
public:

  PageSizeGui(
    QString const           &heading,
    UnitsMeta              *_meta,
    QGroupBox              *parent = NULL);
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
  UnitsMeta              *meta;
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

class QGroupBox;
class QLabel;
class QLineEdit;
class QRadioButton;
class SizeAndOrientationGui : public MetaGui
{
  Q_OBJECT
public:

  SizeAndOrientationGui(
    QString const           &heading,
    UnitsMeta              *_smeta,
    PageOrientationMeta    *_ometa,
    QGroupBox              *parent = NULL);
  ~SizeAndOrientationGui() {}

  int  getTypeIndex(float &widthPg, float &heightPg);
  void setEnabled(bool enabled);
//  void updateWidthAndHeight();

  virtual void apply(QString &topLevelFile);

private:
//  bool                    sizeModified;
//  bool                    orientationModified;
  float                   w;
  float                   h;
  UnitsMeta              *smeta;
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

#endif
