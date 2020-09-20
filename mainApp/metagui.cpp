
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
 * This file describes a set of classes that implement graphical user
 * interfaces for some of the configuration meta classes described in meta.h
 *
 * Please see lpub.h for an overall description of how the files in LPub
 * make up the LPub program.
 *
 ***************************************************************************/

#include "metagui.h"

#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QRadioButton>
#include <QCheckBox>
#include <QSpinBox>
#include <QIntValidator>

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>

#include <QDialogButtonBox>
#include <QFontDialog>
#include <QColorDialog>
#include <QFileDialog>
#include <QString>

//gradient
#include <QGradient>

#include "meta.h"
#include "metatypes.h"
#include "metaitem.h"
#include "placementdialog.h"
#include "color.h"

#include "lpub.h"
#include "lpub_preferences.h"
#include "resolution.h"
#include "render.h"

#include "gradients.h"
#include "pagesizes.h"

/***********************************************************************
 *
 * Checkbox
 *
 **********************************************************************/

CheckBoxGui::CheckBoxGui(
  QString const  &heading,
  BoolMeta       *_meta,
  QGroupBox      *parent)
{
  meta = _meta;

  QHBoxLayout *layout = new QHBoxLayout(parent);

  if (parent) {
    parent->setLayout(layout);
  } else {
    setLayout(layout);
  }

  check = new QCheckBox(heading,parent);
  check->setChecked(meta->value());
  layout->addWidget(check);
  connect(check,SIGNAL(stateChanged(int)),
          this, SLOT(  stateChanged(int)));
}

void CheckBoxGui::stateChanged(int state)
{
  bool checked = meta->value();

  if (state == Qt::Unchecked) {
    checked = false;
  } else if (state == Qt::Checked) {
    checked = true;
  }
  meta->setValue(checked);
  modified = true;
}

void CheckBoxGui::apply(QString &modelName)
{
  if (modified) {
    MetaItem mi;
    mi.setGlobalMeta(modelName,meta);
  }
}

/***********************************************************************
 *
 * BoolRadio
 *
 **********************************************************************/

BoolRadioGui::BoolRadioGui(
  QString const &trueHeading,
  QString const &falseHeading,
  BoolMeta       *_meta,
  QGroupBox      *parent)
{
  meta = _meta;

  QVBoxLayout *layout = new QVBoxLayout(parent);

  parent->setLayout(layout);

  trueRadio = new QRadioButton(trueHeading,parent);
  connect(trueRadio,SIGNAL(clicked(bool)),
          this,     SLOT(  trueClicked(bool)));
  layout->addWidget(trueRadio);
  trueRadio->setChecked(meta->value());

  falseRadio = new QRadioButton(falseHeading,parent);
  connect(falseRadio,SIGNAL(clicked(bool)),
          this,      SLOT(  falseClicked(bool)));
  layout->addWidget(falseRadio);
  falseRadio->setChecked( ! meta->value());
}

void BoolRadioGui::trueClicked(bool clicked)
{
  Q_UNUSED(clicked);
  meta->setValue(true);
  modified = true;
}

void BoolRadioGui::falseClicked(bool clicked)
{
  Q_UNUSED(clicked);
  meta->setValue(false);
  modified = true;
}

void BoolRadioGui::apply(QString &modelName)
{
  if (modified) {
    MetaItem mi;
    mi.setGlobalMeta(modelName,meta);
  }
}

/***********************************************************************
 *
 * Units
 *
 **********************************************************************/

UnitsGui::UnitsGui(
  QString const &heading,
  UnitsMeta     *_meta,
  QGroupBox     *parent)
{
  meta = _meta;

  QHBoxLayout *layout = new QHBoxLayout(parent);

  if (parent) {
    parent->setLayout(layout);
  } else {
    setLayout(layout);
  }

  bool secondLabel = false;
  QStringList labels;
  if (heading != "") {
    labels = heading.split("|");
    secondLabel = labels.size() > 1;
    label = new QLabel(labels.first(),parent);
    layout->addWidget(label);
  } else {
    label = nullptr;
    label2 = nullptr;
  }

  QString      string;

  string = QString("%1") .arg(meta->value(0),
                              meta->_fieldWidth,
                              'f',
                              meta->_precision);
  value0 = new QLineEdit(string,parent);
  connect(value0,SIGNAL(textEdited(  QString const &)),
          this,  SLOT(  value0Change(QString const &)));
  layout->addWidget(value0);

  if (secondLabel) {
    label2 = new QLabel(labels.last(),parent);
    layout->addWidget(label2);
  }

  string = QString("%1") .arg(meta->value(1),
                              meta->_fieldWidth,
                              'f',
                              meta->_precision);
  value1 = new QLineEdit(string,parent);
  connect(value1,SIGNAL(textEdited(  QString const &)),
          this,  SLOT(  value1Change(QString const &)));
  layout->addWidget(value1);
}

void UnitsGui::value0Change(QString const &string)
{
  float v = string.toFloat();
  meta->setValue(0,v);
  modified = true;
}

void UnitsGui::value1Change(QString const &string)
{
  float v = string.toFloat();
  meta->setValue(1,v);
  modified = true;
}

void UnitsGui::setEnabled(bool enable)
{
  if (label) {
    label->setEnabled(enable);
  }
  if (label2) {
    label2->setEnabled(enable);
  }
  value0->setEnabled(enable);
  value1->setEnabled(enable);
}

void UnitsGui::apply(QString &modelName)
{
  if (modified) {
    MetaItem mi;
    mi.setGlobalMeta(modelName,meta);
  }
}

/***********************************************************************
 *
 * Float Pair
 *
 **********************************************************************/

FloatsGui::FloatsGui(
  QString const &heading0,
  QString const &heading1,
  FloatPairMeta *_meta,
  QGroupBox     *parent,
  int            decPlaces,
  bool           _showPair)
{
  meta = _meta;
  showPair = _showPair;

  QHBoxLayout *layout = new QHBoxLayout(parent);

  if (parent) {
    parent->setLayout(layout);
  } else {
    setLayout(layout);
  }

  if (heading0 == "") {
    label0 = nullptr;
  } else {
    label0 = new QLabel(heading0,parent);
    layout->addWidget(label0);
  }

  QString      string, numStr, dynMask;
  int          a, dec;
  float        val;

  val = meta->value(0);
  a = val - int(val);
  dec = (a <= 0 ? 0 : QString::number(a).size() - 2);                          // shameless hack for the number of input decimals
  numStr = dec > 0 ? QString::number(val): QString::number(val,'f',decPlaces); // add 1 decimal place for whole numbers
  for (int i = 0; i < numStr.size(); i++) dynMask.append("x");                 // dynamically create the input mask

  string = QString("%1") .arg(double(val),
                              meta->_fieldWidth,
                              'f',
                              meta->_precision);
  value0 = new QLineEdit(string,parent);
  value0->setInputMask(dynMask);
  connect(value0,SIGNAL(textEdited(  QString const &)),
          this,  SLOT(  value0Change(QString const &)));
  layout->addWidget(value0);

  if (showPair) {
    if (heading1 == "") {
      label1 = nullptr;
    } else {
      label1 = new QLabel(heading1,parent);
      layout->addWidget(label1);
    }

    val = meta->value(1);
    dynMask.clear();
    a = val - (int)val;
    dec = (a <= 0 ? 0 : QString::number(a).size() - 2);                           // shameless hack for the number of input decimals
    numStr = dec > 0 ? QString::number(val): QString::number(val,'f',decPlaces);
    for (int i = 0; i < numStr.size(); i++) dynMask.append("x");                  // dynamically create the input mask
    string = QString("%1") .arg(val,
                                meta->_fieldWidth,
                                'f',
                                meta->_precision);
    value1 = new QLineEdit(string,parent);
    value1->setInputMask(dynMask);
    connect(value1,SIGNAL(textEdited(  QString const &)),
            this,  SLOT(  value1Change(QString const &)));
    layout->addWidget(value1);
  } else {
      QSpacerItem *hSpacer;
      hSpacer = new QSpacerItem(1,1,QSizePolicy::Expanding,QSizePolicy::Fixed);
      layout->addSpacerItem(hSpacer);
  }
}

void FloatsGui::value0Change(QString const &string)
{
  meta->setValue(0,string.toFloat());
  if (!showPair)
     meta->setValue(1,string.toFloat());
  modified = true;
}

void FloatsGui::value1Change(QString const &string)
{
  meta->setValue(1,string.toFloat());
  modified = true;
}

void FloatsGui::setEnabled(bool enable)
{
  if (label0) {
    label0->setEnabled(enable);
  }
  if (label1) {
    label1->setEnabled(enable);
  }
  value0->setEnabled(enable);
  if (value1) {
    value1->setEnabled(enable);
  }
}

void FloatsGui::apply(QString &modelName)
{
  if (modified) {
    MetaItem mi;
    mi.setGlobalMeta(modelName,meta);
  }
}

/***********************************************************************
 *
 * Integer Spin
 *
 **********************************************************************/

SpinGui::SpinGui(
  QString const &heading,
  IntMeta       *_meta,
  int            min,
  int            max,
  int            step,
  QGroupBox     *parent)
{
  meta = _meta;

  QHBoxLayout *layout = new QHBoxLayout(parent);

  if (parent) {
    parent->setLayout(layout);
  } else {
    setLayout(layout);
  }

  if (heading == "") {
    label = nullptr;
  } else {
    label = new QLabel(heading,parent);
    layout->addWidget(label);
  }

  int val = meta->value();

  spin = new QSpinBox(parent);
  layout->addWidget(spin);
  spin->setRange(min,max);
  spin->setSingleStep(step);
  spin->setValue(val);
  connect(spin,SIGNAL(valueChanged(int)),
          this,SLOT  (valueChanged(int)));
}

void SpinGui::valueChanged(int value)
{
  meta->setValue(value);
  modified = true;
}

void SpinGui::setEnabled(bool enable)
{
  if (label) {
    label->setEnabled(enable);
  }
  spin->setEnabled(enable);
}

void SpinGui::apply(QString &modelName)
{
  if (modified) {
    MetaItem mi;
    mi.setGlobalMeta(modelName,meta);
  }
}

/***********************************************************************
 *
 * Double Spin Box
 *
 **********************************************************************/

DoubleSpinGui::DoubleSpinGui(
  QString const &heading,
  FloatMeta     *_meta,
  float          min,
  float          max,
  float          step,
  QGroupBox     *parent)
{
  meta = _meta;

  QHBoxLayout *layout = new QHBoxLayout(parent);

  if (parent) {
    parent->setLayout(layout);
  } else {
    setLayout(layout);
  }

  if (heading == "") {
    label = nullptr;
  } else {
    label = new QLabel(heading,parent);
    layout->addWidget(label);
  }

  float val = meta->value();
  int a = val - (int)val;
  int dec = (a <= 0 ? 2 : QString::number(a).size() < 3 ? 2 : QString::number(a).size());

  spin = new QDoubleSpinBox(parent);
  layout->addWidget(spin);
  spin->setRange(min,max);
  spin->setSingleStep(step);
  spin->setDecimals(dec);
  spin->setValue(val);
  connect(spin,SIGNAL(valueChanged(double)),
          this,SLOT  (valueChanged(double)));
}

void DoubleSpinGui::valueChanged(double value)
{
  meta->setValue(value);
  modified = true;
}

void DoubleSpinGui::setEnabled(bool enable)
{
  if (label) {
    label->setEnabled(enable);
  }
  spin->setEnabled(enable);
}

void DoubleSpinGui::apply(QString &modelName)
{
  if (modified) {
    MetaItem mi;
    mi.setGlobalMeta(modelName,meta);
  }
}

/***********************************************************************
 *
 * Constraint
 *
 **********************************************************************/

ConstrainGui::ConstrainGui(
  QString const &heading,
  ConstrainMeta *_meta,
  QGroupBox     *parent)
{
  meta = _meta;

  QHBoxLayout *layout;

  layout = new QHBoxLayout(parent);

  if (parent) {
    parent->setLayout(layout);
  } else {
    setLayout(layout);
  }

  if (heading != "") {
    headingLabel = new QLabel(heading,parent);
    layout->addWidget(headingLabel);
  } else {
    headingLabel = nullptr;
  }

  QString        string;

  ConstrainData constraint = meta->value();

  string = "";

  switch (constraint.type) {
    case ConstrainData::PliConstrainArea:
    case ConstrainData::PliConstrainSquare:
    break;
    case ConstrainData::PliConstrainWidth:
    case ConstrainData::PliConstrainHeight:
      string = QString("%1") .arg(constraint.constraint,
                                  4,'f',2);
    break;
    case ConstrainData::PliConstrainColumns:
      string = QString("%1") .arg(int(constraint.constraint));
    break;
  }

  combo = new QComboBox(this);
  combo->addItem("Area");
  combo->addItem("Square");
  combo->addItem("Width");
  combo->addItem("Height");
  combo->addItem("Columns");
  combo->setCurrentIndex(int(constraint.type));
  connect(combo,SIGNAL(currentIndexChanged(QString const &)),
          this, SLOT(  typeChange(         QString const &)));
  layout->addWidget(combo);

  /* Constraint */

  value = new QLineEdit(string,parent);
  value->setInputMask("009.99");
  connect(value,SIGNAL(textEdited( QString const &)),
          this, SLOT(  valueChange(QString const &)));
  layout->addWidget(value);
  enable();
}

void ConstrainGui::typeChange(QString const &type)
{
  ConstrainData constraint = meta->valueUnit();
  QString string = "";
  if (type == "Area") {
    constraint.type = ConstrainData::PliConstrainArea;
  } else if (type == "Square") {
    constraint.type = ConstrainData::PliConstrainSquare;
  } else if (type == "Width") {
    string = QString("%1") .arg(constraint.constraint,
                                4,'f',2);
    constraint.type = ConstrainData::PliConstrainWidth;
  } else if (type == "Height") {
    string = QString("%1") .arg(constraint.constraint,
                                4,'f',2);
    constraint.type = ConstrainData::PliConstrainHeight;
  } else {
    string = QString("%1") .arg(int(constraint.constraint));
    constraint.type = ConstrainData::PliConstrainColumns;
  }
  value->setText(string);
  meta->setValueUnit(constraint);
  enable();
  modified = true;
}

void ConstrainGui::valueChange(QString const &value)
{
  ConstrainData constraint = meta->valueUnit();
  constraint.constraint = value.toFloat();
  meta->setValueUnit(constraint);
  modified = true;
}

void ConstrainGui::setEnabled(bool enable)
{
  if (headingLabel) {
    headingLabel->setEnabled(enable);
  }
  combo->setEnabled(enable);
  value->setEnabled(enable);
}

void ConstrainGui::enable()
{
  ConstrainData constraint = meta->valueUnit();
  switch (constraint.type) {
    case ConstrainData::PliConstrainArea:
      value->setDisabled(true);
    break;
    case ConstrainData::PliConstrainSquare:
      value->setDisabled(true);
    break;
    default:
      value->setEnabled(true);
    break;
  }
}

void ConstrainGui::apply(QString &modelName)
{
  if (modified) {
    MetaItem mi;
    mi.setGlobalMeta(modelName,meta);
  }
}

/***********************************************************************
 *
 * Combo Box
 *
 **********************************************************************/

ComboGui::ComboGui(
  QString const &heading,
  QString const &namedValues,
  IntMeta       *_meta,
  QGroupBox     *parent,
  bool           _useCheck)
{
  meta = _meta;
  useCheck = _useCheck;

  QHBoxLayout *layout = new QHBoxLayout(parent);

  if (parent) {
    parent->setLayout(layout);
  } else {
    setLayout(layout);
  }

  int index = meta->value();

  label = nullptr;
  check = nullptr;
  if (!heading.isEmpty()) {
    if (useCheck) {
        check = new QCheckBox(heading,parent);
        check->setChecked(index);
        layout->addWidget(check);
        connect(check,SIGNAL(toggled(bool)),
                this, SLOT(valueChanged(bool)));
    } else {
        label = new QLabel(heading,parent);
        layout->addWidget(label);
    }
  }

  combo = new QComboBox(parent);
  QStringList comboItems;
  if (namedValues.isEmpty()){
     combo->addItem("Default");
  } else {
     comboItems = namedValues.split("|");
     for (QString &item : comboItems)
         combo->addItem(item);
  }
  combo->setEnabled(useCheck ? index : true);
  combo->setCurrentIndex(useCheck ? index ? index - 1 : index : index);
  layout->addWidget(combo);
  connect(combo,SIGNAL(currentIndexChanged(int)),
          this, SLOT  (valueChanged(int)));
}

void ComboGui::valueChanged(bool checked)
{
  combo->setEnabled(checked);
  valueChanged(checked ? combo->currentIndex() : 0);
}

void ComboGui::valueChanged(int value)
{
  meta->setValue(useCheck ? value ? value + 1 : value : value);
  modified = true;
}

void ComboGui::apply(QString &modelName)
{
  if (modified) {
    MetaItem mi;
    mi.setGlobalMeta(modelName,meta);
  }
}

/***********************************************************************
 *
 * Number
 *
 **********************************************************************/

NumberGui::NumberGui(
  NumberMeta *_meta,
  QGroupBox  *parent,
  QString     title)
{
  meta   = _meta;

  QGridLayout *grid = new QGridLayout(parent);

  if (parent) {
      parent->setLayout(grid);
  } else {
      if (!title.isEmpty()) {
          QGridLayout *gridLayout = new QGridLayout(nullptr);
          setLayout(gridLayout);

          gbFormat = new QGroupBox(title,parent);
          parent = gbFormat;
          parent->setLayout(grid);
          gridLayout->addWidget(parent,0,0);
      } else {
          setLayout(grid);
      }
  }

  fontLabel = new QLabel("Font",parent);
  grid->addWidget(fontLabel,0,0);

  fontExample = new QLabel("1234",parent);
  QFont font;
  font.fromString(meta->font.valueFoo());
  fontExample->setFont(font);
  grid->addWidget(fontExample,0,1);

  fontButton = new QPushButton("Change",parent);
  connect(fontButton,SIGNAL(clicked(   bool)),
          this,      SLOT(  browseFont(bool)));
  grid->addWidget(fontButton,0,2);

  colorLabel = new QLabel("Color",parent);
  grid->addWidget(colorLabel,1,0);

  colorExample = new QLabel(parent);
  colorExample->setFrameStyle(QFrame::Sunken|QFrame::Panel);
  QColor c = QColor(meta->color.value());
  QString styleSheet =
      QString("QLabel { background-color: rgb(%1, %2, %3); }").
      arg(c.red()).arg(c.green()).arg(c.blue());
  colorExample->setAutoFillBackground(true);
  colorExample->setStyleSheet(styleSheet);
  grid->addWidget(colorExample,1,1);

  colorButton = new QPushButton("Change");
  connect(colorButton,SIGNAL(clicked(    bool)),
          this,       SLOT(  browseColor(bool)));
  grid->addWidget(colorButton,1,2);

  marginsLabel = new QLabel("Margins",parent);
  grid->addWidget(marginsLabel,2,0);

  QString string;

  string = QString("%1") .arg(meta->margin.value(0),5,'f',4);
  value0 = new QLineEdit(string,parent);
  connect(value0,SIGNAL(textEdited(   QString const &)),
          this,  SLOT(  value0Changed(QString const &)));
  grid->addWidget(value0,2,1);

  string = QString("%1") .arg(meta->margin.value(1),5,'f',4);
  value1 = new QLineEdit(string,parent);
  connect(value1,SIGNAL(textEdited(   QString const &)),
          this,  SLOT(  value1Changed(QString const &)));
  grid->addWidget(value1,2,2);

  fontModified    = false;
  colorModified   = false;
  marginsModified = false;
}

void NumberGui::browseFont(bool clicked)
{
  Q_UNUSED(clicked);
  QFont font;
  QString fontName = meta->font.valueFoo();
  font.fromString(fontName);
  bool ok;
  font = QFontDialog::getFont(&ok,font);
  fontName = font.toString();

  if (ok) {
    meta->font.setValue(font.toString());
    fontExample->setFont(font);
    modified = fontModified = true;
  }
}

void NumberGui::browseColor(bool clicked)
{
  Q_UNUSED(clicked);
  QColor qcolor = LDrawColor::color(meta->color.value());
  QColor newColor = QColorDialog::getColor(qcolor,this);
  if (newColor.isValid() && qcolor != newColor) {
      colorExample->setAutoFillBackground(true);
      QString styleSheet =
          QString("QLabel { background-color: rgb(%1, %2, %3); }")
          .arg(newColor.red()).arg(newColor.green()).arg(newColor.blue());
      colorExample->setStyleSheet(styleSheet);
      meta->color.setValue(newColor.name());
      modified = colorModified = true;
    }
}

void NumberGui::value0Changed(QString const &string)
{
  meta->margin.setValue(0,string.toFloat());
  modified = marginsModified = true;
}

void NumberGui::value1Changed(QString const &string)
{
  meta->margin.setValue(1, string.toFloat());
  modified = marginsModified = true;
}

void NumberGui::enableTextFormatGroup(bool checked)
{
    gbFormat->setEnabled(checked);
}

void NumberGui::apply(
  QString &topLevelFile)
{
  MetaItem mi;
  mi.beginMacro("NumberSettings");
  if (fontModified) {
    mi.setGlobalMeta(topLevelFile,&meta->font);
  }
  if (colorModified) {
    mi.setGlobalMeta(topLevelFile,&meta->color);
  }
  if (marginsModified) {
    mi.setGlobalMeta(topLevelFile,&meta->margin);
  }
  mi.endMacro();
}

/***********************************************************************
 *
 * PageAttribute
 *
 **********************************************************************/

const QString pageAttributeName[25] =
{
    "Page",                "Assembly",   "Step Group",  "Step Number",
    "Parts List",          "Callout",    "Page Number", "Title",
    "Model ID",            "Author",     "URL",         "Model Description",
    "Publish Description", "Copyright",  "Email",       "Disclaimer",
    "Parts",               "Plug",       "Category",    "Logo",
    "Cover Image",         "Plug Image", "Header",      "Footer",
    "BOM",
};

const int attributeKeysOk[16] =
{
    /*  0 Page           0,*/
    /*  0 Title */       fc | bc,
    /*  1 ModelName */   fc,
    /*  2 Author */      fc | bc | ph | pf,
    /*  3 Url */              bc | ph | pf,
    /*  4 ModelDesc */   fc,
    /*  5 PublishDesc */ fc,
    /*  6 Copyright */        bc | ph | pf,
    /*  7 Email */            bc | ph | pf,
    /*  8 Disclaimer */       bc,
    /*  9 Parts */       fc,
    /* 10 Plug */             bc,
    /* 11 Category */    fc | bc,
    /* 12 DocumentLogo */fc | bc,
    /* 13 CoverImage */  fc,
    /* 14 PlugImage */        bc
};

const QString sectionName[4] =
{
    "Front Cover",      //fc
    "Back Cover",       //bc
    "Page Header ",     //ph
    "Page Footer"       //pf
};

/***********************************************************************
 *
 * PageAttributeText
 *
 **********************************************************************/

PageAttributeTextGui::PageAttributeTextGui(
  PageAttributeTextMeta *_meta,
  QGroupBox  *parent)
{
  QString        string;
  QGridLayout   *grid;
  QGridLayout   *gLayout;
  QHBoxLayout   *hLayout;


  meta = _meta;

  selection = 0;

  grid = new QGridLayout(parent);

  if (parent) {
    parent->setLayout(grid);
  }

  int attributeType;
  attributeType = meta->type - 7; // adjust PlacementType to match smaller PageAttributeType Enum
  int oks;
  oks = attributeKeysOk[attributeType];

  // Display
  parent->setCheckable(true);
  parent->setChecked(meta->display.value());
  connect(parent,SIGNAL(toggled(bool)),
          this, SLOT(  toggled(bool)));

  // Section
  sectionLabel = new QLabel("Section",parent);
  grid->addWidget(sectionLabel,0,0);

  sectionCombo = new QComboBox(parent);
  int currentIndex = 0;

  for (int i = 0; i < 4; i++) {
//  logNotice() << "\n POPULATE PLACEMENT COMBO"
//              << "\n    Index: " << i   << " Bits: " << QString::number(i,2)
//              << "\n      Oks: " << oks << " Bits: " << QString::number(oks,2)
//              << "\n (1 << i): " << (1 << i) << " Bits: " << QString::number((1 << i),2)
//                 ;
    if (oks & (1 << i)) {
//        qDebug() << " MATCH: " << i << " " << oks << " " << (1 << i)
//                     ;
      sectionCombo->addItem(sectionName[i]);
      if (i == selection) {
        currentIndex = sectionCombo->count()-1;
      }
    }
  }
  sectionCombo->setCurrentIndex(currentIndex);

  grid->addWidget(sectionCombo,0,1);
  connect(sectionCombo,SIGNAL(currentIndexChanged(int)),SIGNAL(indexChanged(int)));
  connect(this,SIGNAL(indexChanged(int)),this,SLOT(newIndex(int)));

  //Placement
  gbPlacement = new QGroupBox("Placement",parent);
  gLayout = new QGridLayout();
  gbPlacement->setLayout(gLayout);
  grid->addWidget(gbPlacement,0,2);

  placementButton = new QPushButton("Change Placement",parent);
  gLayout->addWidget(placementButton,0,0);
  connect(placementButton,SIGNAL(clicked(   bool)),
          this,      SLOT(  placementChanged(bool)));

  // font
  fontLabel = new QLabel("Font",parent);
  grid->addWidget(fontLabel,1,0);

  fontExample = new QLabel("1234",parent);
  QFont font;
  font.fromString(meta->textFont.valueFoo());
  fontExample->setFont(font);
  grid->addWidget(fontExample,1,1);

  fontButton = new QPushButton("Change",parent);
  connect(fontButton,SIGNAL(clicked(   bool)),
          this,      SLOT(  browseFont(bool)));
  grid->addWidget(fontButton,1,2);

  // color
  colorLabel = new QLabel("Color",parent);
  grid->addWidget(colorLabel,2,0);

  colorExample = new QLabel(parent);
  colorExample->setFrameStyle(QFrame::Sunken|QFrame::Panel);
  QColor c = QColor(meta->textColor.value());
  QString styleSheet =
      QString("QLabel { background-color: rgb(%1, %2, %3); }").
      arg(c.red()).arg(c.green()).arg(c.blue());
  colorExample->setAutoFillBackground(true);
  colorExample->setStyleSheet(styleSheet);
  grid->addWidget(colorExample,2,1);

  colorButton = new QPushButton("Change");
  connect(colorButton,SIGNAL(clicked(    bool)),
          this,       SLOT(  browseColor(bool)));
  grid->addWidget(colorButton,2,2);

  // margins
  marginsLabel = new QLabel("Margins",parent);
  grid->addWidget(marginsLabel,3,0);

  string = QString("%1") .arg(meta->margin.value(0),5,'f',4);
  value0 = new QLineEdit(string,parent);
  connect(value0,SIGNAL(textEdited(   QString const &)),
          this,  SLOT(  value0Changed(QString const &)));
  grid->addWidget(value0,3,1);

  string = QString("%1") .arg(meta->margin.value(1),5,'f',4);
  value1 = new QLineEdit(string,parent);
  connect(value1,SIGNAL(textEdited(   QString const &)),
          this,  SLOT(  value1Changed(QString const &)));
  grid->addWidget(value1,3,2);

  //Content Dialogue
  gbContentEdit = new QGroupBox("Content",parent);
  hLayout = new QHBoxLayout();
  gbContentEdit->setLayout(hLayout);
  grid->addWidget(gbContentEdit,4,0,1,3);

  content = meta->content.value();
  contentEdit = new QLineEdit(content,parent);

  connect(contentEdit,SIGNAL(textChanged(QString const &)),
        this,  SLOT(  editChanged(QString const &)));
  hLayout->addWidget(contentEdit);

  //Description Dialogue
  gbDescDialog = new QGroupBox("Description Dialogue Content",parent);
  gbDescDialog->hide();
  QVBoxLayout *vLayout = new QVBoxLayout(nullptr);
  gbDescDialog->setLayout(vLayout);
  grid->addWidget(gbDescDialog,4,0,1,3);

  editDesc = new QTextEdit(parent);

  vLayout->addWidget(editDesc);

  //spacer
  QSpacerItem *vSpacer;
  vSpacer = new QSpacerItem(1,1,QSizePolicy::Fixed,QSizePolicy::Expanding);
  vLayout->addSpacerItem(vSpacer);

  connect(editDesc,SIGNAL(textChanged()),
          this,  SLOT(  editDescChanged()));

  if (meta->type == PageModelDescType) {
      gbDescDialog->show();
      gbContentEdit->hide();
      string = QString("%1").arg(meta->content.value());
      editDesc->setText(string);
  }

  //Disclaimer Dialogue
  gbDiscDialog = new QGroupBox("Disclaimer Dialogue Content",parent);
  gbDiscDialog->hide();
  vLayout = new QVBoxLayout(nullptr);
  gbDiscDialog->setLayout(vLayout);
  grid->addWidget(gbDiscDialog,4,0,1,3);

  editDisc = new QTextEdit(parent);

  vLayout->addWidget(editDisc);

  //spacer
  vSpacer = new QSpacerItem(1,1,QSizePolicy::Fixed,QSizePolicy::Expanding);
  vLayout->addSpacerItem(vSpacer);

  connect(editDisc,SIGNAL(textChanged()),
          this,  SLOT(  editDiscChanged()));

  if (meta->type == PageDisclaimerType) {
      gbDiscDialog->show();
      gbContentEdit->hide();
      string = QString("%1").arg(meta->content.value());
      editDisc->setText(string);
  }

  fontModified      = false;
  colorModified     = false;
  marginsModified   = false;
  placementModified = false;
  displayModified   = false;
  editModified      = false;
}

void PageAttributeTextGui::browseFont(bool clicked)
{
  Q_UNUSED(clicked);
  QFont font;
  QString fontName = meta->textFont.valueFoo();
  font.fromString(fontName);
  bool ok;
  font = QFontDialog::getFont(&ok,font);
  fontName = font.toString();

  if (ok) {
    meta->textFont.setValue(font.toString());
    fontExample->setFont(font);
    modified = fontModified = true;
  }
}

void PageAttributeTextGui::browseColor(bool clicked)
{
  Q_UNUSED(clicked);
  QColor qcolor = LDrawColor::color(meta->textColor.value());
  QColor newColor = QColorDialog::getColor(qcolor,this);
  if (newColor.isValid() && qcolor != newColor) {
      colorExample->setAutoFillBackground(true);
      QString styleSheet =
          QString("QLabel { background-color: rgb(%1, %2, %3); }")
          .arg(newColor.red()).arg(newColor.green()).arg(newColor.blue());
      colorExample->setStyleSheet(styleSheet);
      meta->textColor.setValue(newColor.name());
      modified = colorModified = true;
    }
}

void PageAttributeTextGui::value0Changed(QString const &string)
{
  meta->margin.setValue(0,string.toFloat());
  modified = marginsModified = true;
}

void PageAttributeTextGui::value1Changed(QString const &string)
{
  meta->margin.setValue(1,string.toFloat());
  modified = marginsModified = true;
}

void PageAttributeTextGui::editDescChanged()
{
  QStringList  text = editDesc->toPlainText().split("\n");
  if (meta->content.value() != text.join("\\n")) {
    meta->content.setValue(text.join("\\n"));
    modified = editModified = true;
  }
}

void PageAttributeTextGui::editDiscChanged()
{
  QStringList  text = editDisc->toPlainText().split("\n");
  if (meta->content.value() != text.join("\\n")) {
    meta->content.setValue(text.join("\\n"));
    modified = editModified = true;
  }
}

void PageAttributeTextGui::editChanged(const QString &value)
{
  QStringList  text = value.split("\n");
  if (meta->content.value() != text.join("\\n")) {
    meta->content.setValue(text.join("\\n"));
    modified = editModified = true;
  }
}

void PageAttributeTextGui::placementChanged(bool clicked)
{
  Q_UNUSED(clicked);
  PlacementData placementData = meta->placement.value();
  bool ok = false;
  ok = PlacementDialog
       ::getPlacement(SingleStepType,meta->type,placementData,pageAttributeName[meta->type]);
  if (ok) {
      meta->placement.setValue(placementData);
      modified = placementModified = true;
  }
}

void PageAttributeTextGui::toggled(bool toggled)
{
  if (meta->display.value() != toggled) {
    meta->display.setValue(toggled);
    modified = displayModified = true;
  }
}

void PageAttributeTextGui::apply(
  QString &topLevelFile)
{
  MetaItem mi;
  mi.beginMacro("Settings");

  if (fontModified) {
    mi.setGlobalMeta(topLevelFile,&meta->textFont);
  }
  if (colorModified) {
    mi.setGlobalMeta(topLevelFile,&meta->textColor);
  }
  if (marginsModified) {
    mi.setGlobalMeta(topLevelFile,&meta->margin);
  }
  if (placementModified){
      mi.setGlobalMeta(topLevelFile,&meta->placement);
  }
  if (displayModified){
      mi.setGlobalMeta(topLevelFile,&meta->display);
  }
  if (editModified){
      mi.setGlobalMeta(topLevelFile,&meta->content);
  }
  mi.endMacro();
}

/***********************************************************************
 *
 * PageAttributePicture
 *
 **********************************************************************/

 PageAttributePictureGui::PageAttributePictureGui(
  PageAttributePictureMeta *_meta,
  QGroupBox  *parent)
{
  QGridLayout   *grid;
  QGridLayout   *gLayout;
  QHBoxLayout   *hLayout;
  QString        string;

  meta  = _meta;

  selection = 0;

  grid = new QGridLayout(parent);

  if (parent) {
      parent->setLayout(grid);
  }

  int attributeType;
  attributeType = meta->type - 7; // adjust PlacementType to match smaller PageAttributeType Enum
  int oks;
  oks = attributeKeysOk[attributeType];

  // Display
  parent->setCheckable(true);
  parent->setChecked(meta->display.value());
  connect(parent,SIGNAL(toggled(bool)),
          this, SLOT(  toggled(bool)));

  // Section
  sectionLabel = new QLabel("Section",parent);
  grid->addWidget(sectionLabel,0,0);

  sectionCombo = new QComboBox(parent);
  int currentIndex = 0;

  for (int i = 0; i < 4; i++) {
//  logNotice() << "\n POPULATE PLACEMENT COMBO"
//              << "\n    Index: " << i   << " Bits: " << QString::number(i,2)
//              << "\n      Oks: " << oks << " Bits: " << QString::number(oks,2)
//              << "\n (1 << i): " << (1 << i) << " Bits: " << QString::number((1 << i),2)
//                 ;
    if (oks & (1 << i)) {
//        qDebug() << " MATCH: " << i << " " << oks << " " << (1 << i)
//                     ;
      sectionCombo->addItem(sectionName[i]);
      if (i == selection) {
        currentIndex = sectionCombo->count()-1;
      }
    }
  }
  sectionCombo->setCurrentIndex(currentIndex);
  grid->addWidget(sectionCombo,0,1);
  connect(sectionCombo,SIGNAL(currentIndexChanged(int)),SIGNAL(indexChanged(int)));
  connect(this,SIGNAL(indexChanged(int)),this,SLOT(selectionChanged(int)));

  //Placement
  gbPlacement = new QGroupBox("Placement",parent);
  gLayout = new QGridLayout();
  gbPlacement->setLayout(gLayout);
  grid->addWidget(gbPlacement,0,2);

  placementButton = new QPushButton("Change Placement",parent);
  gLayout->addWidget(placementButton,0,0);
  connect(placementButton,SIGNAL(clicked(   bool)),
          this,      SLOT(  placementChanged(bool)));

  // margins
  marginsLabel = new QLabel("Margins",parent);
  grid->addWidget(marginsLabel,1,0);

  string = QString("%1") .arg(meta->margin.value(0),5,'f',4);
  value0 = new QLineEdit(string,parent);
  connect(value0,SIGNAL(textEdited(   QString const &)),
          this,  SLOT(  value0Changed(QString const &)));
  grid->addWidget(value0,1,1);

  string = QString("%1") .arg(meta->margin.value(1),5,'f',4);
  value1 = new QLineEdit(string,parent);
  connect(value1,SIGNAL(textEdited(   QString const &)),
          this,  SLOT(  value1Changed(QString const &)));
  grid->addWidget(value1,1,2);

  // Image
  picture = meta->file.value();
  pictureEdit = new QLineEdit(picture,parent);
  connect(pictureEdit,SIGNAL(textEdited(   QString const &)),
          this,       SLOT(  pictureChange(QString const &)));
  grid->addWidget(pictureEdit,2,0,1,2);

  pictureButton = new QPushButton("Browse",parent);
  connect(pictureButton,SIGNAL(clicked(     bool)),
          this,         SLOT(  browsePicture(bool)));
  grid->addWidget(pictureButton,2,2,1,1);

  //scale
  bool gbChecked = (meta->picScale.value() > 1.0 ||
                    meta->picScale.value() < 1.0) &&
                   (meta->fill.value() == Aspect);
  gbScale = new QGroupBox("Scale", parent);
  gbScale->setCheckable(true);
  gbScale->setChecked(gbChecked);
  gbScale->setEnabled(meta->fill.value() == Aspect);
  hLayout = new QHBoxLayout();
  gbScale->setLayout(hLayout);
  grid->addWidget(gbScale,3,0,1,3);

  scale = new QLabel("Scale " + pageAttributeName[meta->type],parent);
  hLayout->addWidget(scale);

  spin = new QDoubleSpinBox(parent);
  spin->setRange(meta->picScale._min,meta->picScale._max);
  spin->setSingleStep(0.1);
  spin->setDecimals(6);
  spin->setValue(double(meta->picScale.value()));
  connect(spin,SIGNAL(valueChanged(double)),
          this,SLOT  (valueChanged(double)));
  hLayout->addWidget(spin);
  connect(gbScale,SIGNAL(clicked(bool)),this,SLOT(gbScaleClicked(bool)));

  // fill
  gbFill = new QGroupBox("Picture Fill Mode", parent);
  hLayout = new QHBoxLayout();
  gbFill->setLayout(hLayout);
  grid->addWidget(gbFill,4,0,1,3);

  aspectRadio = new QRadioButton("Aspect",gbFill);
  aspectRadio->setChecked(meta->fill.value() == Aspect);
  connect(aspectRadio,SIGNAL(clicked(bool)),
          this,        SLOT(  pictureFill(bool)));
  hLayout->addWidget(aspectRadio);

  stretchRadio = new QRadioButton("Stretch",gbFill);
  stretchRadio->setChecked(meta->fill.value() == Stretch);
  connect(stretchRadio,SIGNAL(clicked(bool)),
          this,        SLOT(  pictureFill(bool)));
  hLayout->addWidget(stretchRadio);
  tileRadio    = new QRadioButton("Tile",gbFill);
  tileRadio->setChecked(meta->fill.value() == Tile);
  connect(tileRadio,SIGNAL(clicked(bool)),
          this,     SLOT(  pictureFill(bool)));
  hLayout->addWidget(tileRadio);

  fillModified      = false;
  pictureModified   = false;
  marginsModified   = false;
  placementModified = false;
  displayModified   = false;
  scaleModified     = false;
}

 void PageAttributePictureGui::pictureFill(bool checked)
 {
   if (sender() == stretchRadio) {
       meta->fill.setValue(Stretch);
       if (checked)
           gbScale->setEnabled(!checked);
   } else if (sender() == tileRadio) {
       meta->fill.setValue(Tile);
       if (checked)
           gbScale->setEnabled(!checked);
   } else { /*aspectRadio*/
       meta->fill.setValue(Aspect);
       if (checked)
           gbScale->setEnabled(checked);
   }
   modified = fillModified = true;
 }

void PageAttributePictureGui::pictureChange(QString const &pic)
{
  meta->file.setValue(pic);
  modified = pictureModified = true;
}

void PageAttributePictureGui::browsePicture(bool)
{
  QString picture = meta->file.value();

  QString foo = QFileDialog::getOpenFileName(
    gui,
    tr("Choose Picture File"),
    picture,
    tr("Image Files (*.png *.jpg *.jpeg *.bmp)"));
  if (foo != "") {
    picture = foo;
    pictureEdit->setText(foo);
    meta->file.setValue(picture);
    modified = pictureModified = true;
  }
}

void PageAttributePictureGui::gbScaleClicked(bool checked)
{
  qreal value = meta->picScale.value();
  meta->picScale.setValue(value);
  modified = scaleModified = true;
  if (checked) {
      aspectRadio->setChecked(checked);
      stretchRadio->setChecked(!checked);
      tileRadio->setChecked(!checked);
  }
}

void PageAttributePictureGui::value0Changed(QString const &string)
{
  meta->margin.setValue(0,string.toFloat());
  modified = marginsModified = true;
}

void PageAttributePictureGui::value1Changed(QString const &string)
{
  meta->margin.setValue(1,string.toFloat());
  modified = marginsModified = true;
}

void PageAttributePictureGui::valueChanged(double value)
{
  meta->picScale.setValue(value);
  modified = pictureModified = true;
}


void PageAttributePictureGui::placementChanged(bool clicked)
{
  Q_UNUSED(clicked);
  PlacementData placementData = meta->placement.value();
  bool ok;
  ok = PlacementDialog
       ::getPlacement(SingleStepType,meta->type,placementData,pageAttributeName[meta->type]);
  if (ok) {
      meta->placement.setValue(placementData);
//      logInfo() << "\n PRE PLACEMENT META - "
//                << "\ngetPlacement INPUT DATA - "
//                << " \nPlacement: "                 << placementData.placement
//                << " \nJustification: "             << placementData.justification
//                << " \nPreposition: "               << placementData.preposition
//                << " \nRelativeTo: "                << placementData.relativeTo
//                << " \nRectPlacement: "             << placementData.rectPlacement
//                << " \nOffset[0]: "                 << placementData.offsets[0]
//                << " \nOffset[1]: "                 << placementData.offsets[1]
//                ;
    modified = placementModified = true;
  }  
}

void PageAttributePictureGui::toggled(bool toggled)
{
    meta->display.setValue(toggled);
    modified = displayModified = true;
}

void PageAttributePictureGui::apply(QString &topLevelFile)
{
    MetaItem mi;
    mi.beginMacro("Settings");

    if (pictureModified) {
      mi.setGlobalMeta(topLevelFile,&meta->file);
    }
    if (scaleModified) {
      mi.setGlobalMeta(topLevelFile,&meta->picScale);
    }
    if (marginsModified) {
      mi.setGlobalMeta(topLevelFile,&meta->margin);
    }
    if (placementModified){
        mi.setGlobalMeta(topLevelFile,&meta->placement);
    }
    if (displayModified){
        mi.setGlobalMeta(topLevelFile,&meta->display);
    }
    if (fillModified){
        mi.setGlobalMeta(topLevelFile,&meta->fill);
    }
    mi.endMacro();
}

/***********************************************************************
 *
 * PageHeaderHeight
 *
 **********************************************************************/

HeaderFooterHeightGui::HeaderFooterHeightGui(
  QString const &heading,
  UnitsMeta     *_meta,
  QGroupBox     *parent)
{
  meta = _meta;

  QHBoxLayout *layout = new QHBoxLayout(parent);

  if (parent) {
    parent->setLayout(layout);
  } else {
    setLayout(layout);
  }

  if (heading != "") {
    label = new QLabel(heading,parent);
    layout->addWidget(label);
  } else {
    label = nullptr;
  }

  QString      string;

  string = QString("%1") .arg(meta->value(0),
                              meta->_fieldWidth,
                              'f',
                              meta->_precision);
  value0 = new QLineEdit(string,parent);
  value0->setDisabled(true);

  layout->addWidget(value0);

  string = QString("%1") .arg(meta->value(1),
                              meta->_fieldWidth,
                              'f',
                              meta->_precision);
  value1 = new QLineEdit(string,parent);
  connect(value1,SIGNAL(textEdited(  QString const &)),
          this,  SLOT(  value1Change(QString const &)));
  layout->addWidget(value1);
}

void HeaderFooterHeightGui::value1Change(QString const &string)
{
  float v = string.toFloat();
  meta->setValue(1,v);
  modified = true;
}

void HeaderFooterHeightGui::setEnabled(bool enable)
{
  if (label) {
    label->setEnabled(enable);
  }
  value0->setEnabled(enable);
}

void HeaderFooterHeightGui::apply(QString &modelName)
{
  if (modified) {
    MetaItem mi;
    mi.setGlobalMeta(modelName,meta);
  }
}

/***********************************************************************
 *
 * FadeStep
 *
 **********************************************************************/

FadeStepGui::FadeStepGui(
        QString const &heading,
        FadeStepMeta *_meta,
        QGroupBox  *parent)
{

    meta = _meta;

    QGridLayout *grid;

    grid = new QGridLayout(parent);

    colorLabel = new QLabel(heading,parent);
    colorLabel->setEnabled(false);

    grid->addWidget(colorLabel,0,0);

    readOnlyLabel = new QLabel("       Set from Preferences dialog");
    readOnlyLabel->setEnabled(false);

    grid->addWidget(readOnlyLabel,0,1);

    colorExample = new QLabel(parent);
    colorExample->setFixedSize(50,50);
    colorExample->setFrameStyle(QFrame::Sunken|QFrame::Panel);
    QColor c = QColor(LDrawColor::color(meta->fadeColor.value()));
    QString styleSheet =
        QString("QLabel { background-color: rgb(%1, %2, %3); }").
        arg(c.red()).arg(c.green()).arg(c.blue());
    colorExample->setAutoFillBackground(true);
    colorExample->setStyleSheet(styleSheet);

    grid->addWidget(colorExample,1,0);

    colorCombo = new QComboBox(parent);
    colorCombo->addItems(LDrawColor::names());
    colorCombo->setCurrentIndex(int(colorCombo->findText(meta->fadeColor.value())));
    if (! Preferences::enableFadeSteps)
        colorCombo->setDisabled(true);
    connect(colorCombo,SIGNAL(currentIndexChanged(QString const &)),
                   this, SLOT(colorChange(        QString const &)));

    grid->addWidget(colorCombo,1,1);

    if (parent) {
        parent->setLayout(grid);
    } else {
        setLayout(grid);
    }
}

void FadeStepGui::colorChange(QString const &colorName)
{
  QColor newColor = LDrawColor::color(colorName);
  if (newColor.isValid()) {
      colorExample->setAutoFillBackground(true);
      QString styleSheet =
          QString("QLabel { background-color: rgb(%1, %2, %3); }")
          .arg(newColor.red()).arg(newColor.green()).arg(newColor.blue());
      colorExample->setStyleSheet(styleSheet);
      meta->fadeColor.setValue(LDrawColor::name(newColor.name()));
      modified = true;
    }
}

void FadeStepGui::apply(
  QString &topLevelFile)
{
  if (modified) {
    MetaItem mi;
    mi.setGlobalMeta(topLevelFile,&meta->fadeColor);
  }
}

/***********************************************************************
 *
 * HighlightStep
 *
 **********************************************************************/

HighlightStepGui::HighlightStepGui(
        QString const &heading,
        HighlightStepMeta *_meta,
        QGroupBox  *parent)
{

    meta = _meta;

    QGridLayout *grid;

    grid = new QGridLayout(parent);

    colorLabel = new QLabel(heading,parent);
    colorLabel->setEnabled(false);

    grid->addWidget(colorLabel,0,0);

    readOnlyLabel = new QLabel("       Set from Preferences dialog");
    readOnlyLabel->setEnabled(false);

    grid->addWidget(readOnlyLabel,0,1);

    colorExample = new QLabel(parent);
    colorExample->setFixedSize(50,50);
    colorExample->setFrameStyle(QFrame::Sunken|QFrame::Panel);
    colorExample->setAutoFillBackground(true);
    QColor c = QColor(meta->highlightColor.value());
    QString styleSheet =
        QString("QLabel { background-color: rgb(%1, %2, %3); }").
        arg(c.red()).arg(c.green()).arg(c.blue());
    colorExample->setStyleSheet(styleSheet);

    grid->addWidget(colorExample,1,0);

    colorButton = new QPushButton(parent);
    colorButton->setText("Highlight Color...");

    if (! Preferences::enableHighlightStep)
        colorButton->setDisabled(true);
    connect(colorButton,SIGNAL(clicked(bool)),
                   this, SLOT(colorChange(bool)));

    grid->addWidget(colorButton,1,1);

    if (parent) {
        parent->setLayout(grid);
    } else {
        setLayout(grid);
    }
}

void HighlightStepGui::colorChange(bool clicked)
{
  Q_UNUSED(clicked);
  QColor highlightColour = QColorDialog::getColor(colorExample->palette().background().color(), this );
  if (highlightColour.isValid()) {
      colorExample->setAutoFillBackground(true);
      QString styleSheet =
          QString("QLabel { background-color: rgb(%1, %2, %3); }")
          .arg(highlightColour.red()).arg(highlightColour.green()).arg(highlightColour.blue());
      colorExample->setStyleSheet(styleSheet);
      meta->highlightColor.setValue(highlightColour.name());
      modified = true;
    }
}

void HighlightStepGui::apply(
  QString &topLevelFile)
{
  if (modified) {
    MetaItem mi;
    mi.setGlobalMeta(topLevelFile,&meta->highlightColor);
  }
}

/***********************************************************************
 *
 * CameraDistFactor
 *
 **********************************************************************/

CameraDistFactorGui::CameraDistFactorGui(
        QString const &heading,
        NativeCDMeta *_meta,
        QGroupBox  *parent)
{

  meta = _meta;

  QHBoxLayout *hLayout = new QHBoxLayout(parent);

  if (parent) {
      parent->setLayout(hLayout);
  } else {
      setLayout(hLayout);
  }

  QString tipMessage = QString("Adjust to change the default Native camera distance.");

  if (heading != "") {
    cameraDistFactorLabel = new QLabel(heading, parent);
    hLayout->addWidget(cameraDistFactorLabel);
  } else {
    cameraDistFactorLabel = nullptr;
  }

  saveFactor = meta->factor.value();
  cameraDistFactorSpin = new QSpinBox(parent);
  cameraDistFactorSpin->setToolTip(tipMessage);
  cameraDistFactorSpin->setRange(-5000,5000);
  cameraDistFactorSpin->setSingleStep(10);
  cameraDistFactorSpin->setValue(meta->factor.value());
  connect(cameraDistFactorSpin,SIGNAL(valueChanged(int)),
          this,                SLOT(cameraDistFactorChange(int)));
  hLayout->addWidget(cameraDistFactorSpin);
}

void CameraDistFactorGui::cameraDistFactorChange(int factor)
{
  meta->factor.setValue(factor);
  changeMessage = QString("Native camera distance factor changed from %1 to %2")
                          .arg(saveFactor)
                          .arg(meta->factor.value());
  modified = true;
}

void CameraDistFactorGui::apply(
  QString &topLevelFile)
{
  if (modified) {
    emit gui->messageSig(LOG_INFO, changeMessage);
    MetaItem mi;
    mi.setGlobalMeta(topLevelFile,&meta->factor);
  }
}

/***********************************************************************
 *
 * JustifyStep
 *
 **********************************************************************/

JustifyStepGui::JustifyStepGui(
        const QString &_label,
        JustifyStepMeta *_meta,
        QGroupBox       *parent)
{

  meta = _meta;

  JustifyStepData data = meta->value();

  QGridLayout *grid = new QGridLayout();

  if (parent) {
      parent->setLayout(grid);
    } else {
      setLayout(grid);
    }

  QLabel    *label;
  label = new QLabel(_label, parent);
  grid->addWidget(label,0,0);

  typeCombo = new QComboBox(parent);
  typeCombo->addItem("Center");
  typeCombo->addItem("Center Horizontal");
  typeCombo->addItem("Center Vertical");
  typeCombo->addItem("Left (Default)");
  typeCombo->setCurrentIndex(int(data.type));
  connect(typeCombo,SIGNAL(currentIndexChanged(int)),
          this,     SLOT(  typeChanged(        int)));
  grid->addWidget(typeCombo,0,1);

  label = new QLabel("Spacing",parent);
  grid->addWidget(label,0,3);
  grid->setAlignment(label, Qt::AlignRight);

  spacingSpinBox = new QDoubleSpinBox(parent);
  spacingSpinBox->setRange(0.0,25.0);
  spacingSpinBox->setSingleStep(0.1);
  spacingSpinBox->setDecimals(4);
  spacingSpinBox->setValue(double(data.spacing));
  spacingSpinBox->setToolTip(QString("Set the spaceing, in %1, between items when step "
                                     "is center justified").arg(units2name()));
  spacingSpinBox->setEnabled(data.type != JustifyLeft);
  connect(spacingSpinBox,SIGNAL(valueChanged(double)),
          this,          SLOT(spacingChanged(double)));
  grid->addWidget(spacingSpinBox,0,4);
}

void JustifyStepGui::typeChanged(int value)
{
  spacingSpinBox->setEnabled(value != JustifyLeft);

  JustifyStepData data = meta->value();
  data.type = JustifyStepEnc(value);
  meta->setValue(data);
  modified = true;
}

void JustifyStepGui::spacingChanged(double value)
{
  JustifyStepData data = meta->value();
  data.spacing = float(value);
  meta->setValue(data);
  modified = true;
}

void JustifyStepGui::apply(QString &modelName)
{
  if (modified) {
    MetaItem mi;
    mi.setGlobalMeta(modelName,meta);
  }
}

/***********************************************************************
 *
 * RotStep
 *
 **********************************************************************/

RotStepGui::RotStepGui(
        RotStepMeta *_meta,
        QGroupBox  *parent)
{

  meta = _meta;

  RotStepData rotStep = meta->value();

  QGridLayout *grid = new QGridLayout();

  if (parent) {
      parent->setLayout(grid);
    } else {
      setLayout(grid);
    }

  QLabel    *rotStepLabel;
  rotStepLabel = new QLabel("Rotation", parent);
  grid->addWidget(rotStepLabel,0,0);

  qreal val;
  auto dec = [] (const qreal v)
  {
      auto places = [&v] () {
          if (v == 0.0)
              return 2;

          int count = 0;
          qreal num = v;
          num = abs(num);
          num = num - int(num);
          while (abs(num) >= 0.00001) {
              num = num * 10;
              count = count + 1;
              num = num - int(num);
          }
          return count;
      };

      int a = v - int(v);
      return (a < 1 ? places() : QString::number(a).size() < 3 ? 2 : QString::number(a).size());
  };

  val = rotStep.rots[0];
  rotStepSpinX = new QDoubleSpinBox(parent);
  rotStepSpinX->setRange(0.0,360.0);
  rotStepSpinX->setSingleStep(1.0);
  rotStepSpinX->setDecimals(dec(val));
  rotStepSpinX->setValue(val);
  connect(rotStepSpinX,SIGNAL(valueChanged(double)),
          this,        SLOT(rotStepChanged(double)));
  grid->addWidget(rotStepSpinX,0,1);

  val = rotStep.rots[1];
  rotStepSpinY = new QDoubleSpinBox(parent);
  rotStepSpinY->setRange(0.0,360.0);
  rotStepSpinY->setSingleStep(1.0);
  rotStepSpinY->setDecimals(dec(val));
  rotStepSpinY->setValue(val);
  connect(rotStepSpinY,SIGNAL(valueChanged(double)),
          this,        SLOT(rotStepChanged(double)));
  grid->addWidget(rotStepSpinY,0,2);

  val = rotStep.rots[2];
  rotStepSpinZ = new QDoubleSpinBox(parent);
  rotStepSpinZ->setRange(0.0,360.0);
  rotStepSpinZ->setSingleStep(1.0);
  rotStepSpinZ->setDecimals(dec(val));
  rotStepSpinZ->setValue(val);
  connect(rotStepSpinZ,SIGNAL(valueChanged(double)),
          this,        SLOT(rotStepChanged(double)));
  grid->addWidget(rotStepSpinZ,0,3);

  QLabel    *typeLabel;
  typeLabel = new QLabel("Transform", parent);
  grid->addWidget(typeLabel,1,0);

  typeCombo = new QComboBox(parent);
  typeCombo->addItem("ABS");
  typeCombo->addItem("REL");
  typeCombo->addItem("ADD");
  typeCombo->setCurrentIndex(!rotStep.type.isEmpty() ? typeCombo->findText(rotStep.type) : 1);
  connect(typeCombo,SIGNAL(currentIndexChanged(QString const &)),
          this,     SLOT(  typeChanged(         QString const &)));
  grid->addWidget(typeCombo,1,1);

}

void RotStepGui::rotStepChanged(double value)
{
  RotStepData data = meta->value();
  if (sender() == rotStepSpinX)
      data.rots[0] = value;
  else
  if (sender() == rotStepSpinY)
      data.rots[1] = value;
  else /* rotStepSpinZ */
      data.rots[2] = value;
  meta->setValue(data);
  modified = true;
}

void RotStepGui::typeChanged(QString const &value)
{
  RotStepData data = meta->value();
  data.type = value;
  meta->setValue(data);
  modified = true;
}

void RotStepGui::apply(QString &modelName)
{
  if (modified) {
    MetaItem mi;
    mi.setGlobalMeta(modelName,meta);
  }
}

/***********************************************************************
 *
 * ContStepNumMeta
 *
 **********************************************************************/

ContStepNumGui::ContStepNumGui(
  QString const   &heading,
  ContStepNumMeta *_meta,
  QGroupBox       *parent)
{
  meta = _meta;

  QHBoxLayout *layout = new QHBoxLayout(parent);

  if (parent) {
    parent->setLayout(layout);
  } else {
    setLayout(layout);
  }

  check = new QCheckBox(heading,parent);
  check->setChecked(meta->value());
  layout->addWidget(check);
  connect(check,SIGNAL(stateChanged(int)),
          this, SLOT(  stateChanged(int)));
}

void ContStepNumGui::stateChanged(int state)
{
  bool checked = meta->value();

  if (state == Qt::Unchecked) {
    checked = false;
  } else if (state == Qt::Checked) {
    checked = true;
  }
  meta->setValue(checked);
  modified = true;
}

void ContStepNumGui::apply(QString &modelName)
{
  if (modified) {
    MetaItem mi;
    mi.setGlobalMeta(modelName,meta);
  }
}

/***********************************************************************
 *
 * CountInstance
 *
 **********************************************************************/

CountInstanceGui::CountInstanceGui(
  CountInstanceMeta *_meta,
  QGroupBox         *parent)
{
  meta = _meta;

  QHBoxLayout *layout = new QHBoxLayout(parent);

  if (parent) {
    parent->setLayout(layout);
  } else {
    setLayout(layout);
  }

  topRadio    = new QRadioButton("At Top",parent);
  topRadio->setChecked(meta->value() == CountAtTop);
  topRadio->setToolTip("Consolidate instances at first occurrence in the entire model file.");
  layout->addWidget(topRadio);
  connect(topRadio,SIGNAL(clicked(bool)),
          this,     SLOT(  radioChanged(bool)));

  modelRadio = new QRadioButton("At Model (default)",parent);
  modelRadio->setChecked(meta->value() > CountFalse && meta->value() < CountAtStep);
  modelRadio->setToolTip("Consolidate instances at first occurrence in the parent model.");
  layout->addWidget(modelRadio);
  connect(modelRadio,SIGNAL(clicked(bool)),
          this,      SLOT(  radioChanged(bool)));

  stepRadio    = new QRadioButton("At Step",parent);
  stepRadio->setChecked(meta->value() == CountAtStep);
  stepRadio->setToolTip("Consolidate instances at first occurrence in the current step.");
  layout->addWidget(stepRadio);
  connect(stepRadio,SIGNAL(clicked(bool)),
          this,     SLOT(  radioChanged(bool)));

  connect(parent,SIGNAL(clicked(bool)),
          this, SLOT(groupBoxChanged(bool)));
}

void CountInstanceGui::radioChanged(bool checked)
{
  Q_UNUSED(checked)

  if (sender() == topRadio) {
      meta->setValue(CountAtTop);
  } else
  if (sender() == modelRadio) {
      meta->setValue(CountAtModel);
  } else
  if (sender() == stepRadio) {
      meta->setValue(CountAtStep);
  }
  modified = true;
}

void CountInstanceGui::groupBoxChanged(bool checked)
{
  meta->setValue(checked ? CountTrue : CountFalse);
  modified = true;
}

void CountInstanceGui::apply(QString &modelName)
{
  if (modified) {
    MetaItem mi;
    mi.setGlobalMeta(modelName,meta);
  }
}

/***********************************************************************
 *
 * Background
 *
 **********************************************************************/

BackgroundGui::BackgroundGui(
        BackgroundMeta *_meta,
        QGroupBox      *parent,
        bool            pictureSettings)
{
  QComboBox     *combo;
  QGridLayout   *grid;
  QHBoxLayout   *layout;

  meta = _meta;

  BackgroundData background = meta->value();

  bool rotateIcon = false;
  QDialog *parentDialog = dynamic_cast<QDialog*>(parent->parent());
  if (parentDialog) {
      //logDebug() << "Grandparent Window TITLE " + parentDialog->windowTitle();
      rotateIcon = parentDialog->windowTitle().contains("Rotate Icon");
  }

  if (rotateIcon){
      QDir extrasDir(Preferences::lpubDataPath + QDir::separator() + "extras");
      QFileInfo rotateIconFileInfo(extrasDir.absolutePath() + QDir::separator() + VER_ROTATE_ICON_FILE);
      if (rotateIconFileInfo.exists())
          picture = QString(rotateIconFileInfo.absoluteFilePath());
      else
          picture = QString(":/resources/rotate-icon.png");
  }

  if (background.type == BackgroundData::BgImage) {
      if (!background.string.isEmpty())
          picture = background.string;
  } else {
      color = background.string;
  }

  grid = new QGridLayout(parent);
  parent->setLayout(grid);

  combo = new QComboBox(parent);
  combo->addItem("None (transparent)");         // 0
  combo->addItem("Solid Color");                // 1
  combo->addItem("Gradient");                   // 2
  if (pictureSettings) {
     combo->addItem("Picture");                 // 3
     combo->addItem("Submodel Level Color");    // 4
     combo->setCurrentIndex(int(background.type));
  } else {
     combo->addItem("Submodel Level Color");    //3
     combo->setCurrentIndex(background.type == 4 ? 3 : int(background.type));
  }
  connect(combo,SIGNAL(currentIndexChanged(QString const &)),
          this, SLOT(  typeChange(         QString const &)));
  grid->addWidget(combo,0,0);

  /* Color and Gradient button */

  colorButton = new QPushButton("Change",parent);
  colorButton->setToolTip(tr("Change Color"));
  connect(colorButton,SIGNAL(clicked(    bool)),
          this,       SLOT(  browseColor(bool)));
  grid->addWidget(colorButton,0,1);

  gradientButton = new QPushButton("Change",parent);
  gradientButton->setToolTip(tr("Change Gradient"));
  connect(gradientButton,SIGNAL(clicked(     bool)),
          this,          SLOT(setGradient(   bool)));
  grid->addWidget(gradientButton,0,1);

  /* Color label */

  colorExample = new QLabel(parent);
  colorExample->setFixedWidth(90);
  colorExample->setFrameStyle(QFrame::Sunken|QFrame::Panel);
  QColor c = QColor(color);
  QString styleSheet =
      QString("QLabel { background-color: %1; }")
      .arg(color.isEmpty() ? "transparent" :
           QString("rgb(%1, %2, %3)")
           .arg(c.red()).arg(c.green()).arg(c.blue()));
  colorExample->setAutoFillBackground(true);
  colorExample->setStyleSheet(styleSheet);
  grid->addWidget(colorExample,0,2);

  /* Image */

  pictureEdit = new QLineEdit(picture,parent);
  pictureEdit->setToolTip("Enter image path");
  connect(pictureEdit,SIGNAL(textEdited(   QString const &)),
          this,       SLOT(  pictureChange(QString const &)));
  grid->addWidget(pictureEdit,1,0);

  pictureButton = new QPushButton("Browse",parent);
  connect(pictureButton,SIGNAL(clicked(     bool)),
          this,         SLOT(  browsePicture(bool)));
  grid->addWidget(pictureButton,1,1);

  /* Fill */

  fill = new QGroupBox("Fill",parent);

  layout = new QHBoxLayout();
  fill->setLayout(layout);
  grid->addWidget(fill,2,0,1,3);

  stretchRadio = new QRadioButton("Stretch Picture",fill);
  connect(stretchRadio,SIGNAL(clicked(bool)),
          this,        SLOT(  stretch(bool)));
  layout->addWidget(stretchRadio);
  tileRadio    = new QRadioButton("Tile Picture",fill);
  connect(tileRadio,SIGNAL(clicked(bool)),
          this,     SLOT(  tile(bool)));
  layout->addWidget(tileRadio);

  if (!pictureSettings) {
      pictureEdit->hide();
      pictureButton->hide();
      fill->hide();
      stretchRadio->hide();
      tileRadio->hide();
  }

  enable();
}

void BackgroundGui::enable()
{
  BackgroundData background = meta->value();

  stretchRadio->setChecked(background.stretch);
  tileRadio->setChecked( !background.stretch);

  switch (background.type) {
    case BackgroundData::BgImage:
      colorButton->show();
      colorButton->setEnabled(false);
      gradientButton->hide();
      pictureButton->setEnabled(true);
      pictureEdit->setEnabled(true);
      fill->setEnabled(true);
    break;
    case BackgroundData::BgGradient:
      colorButton->hide();
      gradientButton->show();
      gradientButton->setEnabled(true);
      pictureButton->setEnabled(false);
      pictureEdit->setEnabled(false);
      fill->setEnabled(false);
    break;
    case BackgroundData::BgColor:
      colorButton->show();
      colorButton->setEnabled(true);
      gradientButton->hide();
      pictureButton->setEnabled(false);
      pictureEdit->setEnabled(false);
      fill->setEnabled(false);
    break;
    default:
      colorButton->show();
      colorButton->setEnabled(false);
      gradientButton->hide();
      pictureButton->setEnabled(false);
      pictureEdit->setEnabled(false);
      fill->setEnabled(false);
    break;
  }
}

void BackgroundGui::typeChange(QString const &type)
{
  BackgroundData background = meta->value();

  if (type == "None (transparent)") {
      background.type = BackgroundData::BgTransparent;
    } else if (type == "Picture") {
      pictureEdit->setText(picture);
      background.string = picture;
      background.type = BackgroundData::BgImage;
    } else if (type == "Solid Color") {
      background.type = BackgroundData::BgColor;
    } else if (type == "Gradient") {
      background.type = BackgroundData::BgGradient;
    } else {
      background.type = BackgroundData::BgSubmodelColor;
    }
  meta->setValue(background);
  enable();
  modified = true;
}

void BackgroundGui::pictureChange(QString const &pic)
{
  BackgroundData background = meta->value();
  background.string = pic;
  meta->setValue(background);
  modified = true;
}

void BackgroundGui::setGradient(bool){

  bool ok = true;

  BackgroundData backgroundData = meta->value();
  QPolygonF pts;
  QGradientStops stops;

  QSize gSize(backgroundData.gsize[0],backgroundData.gsize[1]);

  for (int i=0; i<backgroundData.gpoints.size(); i++)
    pts.append(backgroundData.gpoints.at(i));

  QGradient::CoordinateMode mode = QGradient::LogicalMode;
  switch (backgroundData.gmode){
    case BackgroundData::LogicalMode:
      mode = QGradient::LogicalMode;
    break;
    case BackgroundData::StretchToDeviceMode:
      mode = QGradient::StretchToDeviceMode;
    break;
    case BackgroundData::ObjectBoundingMode:
      mode = QGradient::ObjectBoundingMode;
    break;
    }

  QGradient::Spread spread = QGradient::RepeatSpread;
  switch (backgroundData.gspread){
    case BackgroundData::PadSpread:
      spread = QGradient::PadSpread;
    break;
    case BackgroundData::RepeatSpread:
      spread = QGradient::RepeatSpread;
    break;
    case BackgroundData::ReflectSpread:
      spread = QGradient::ReflectSpread;
    break;
    }

  QGradient *g = new QLinearGradient(pts.at(0), pts.at(1));;
  switch (backgroundData.gtype){
    case BackgroundData::LinearGradient:
      g = new QLinearGradient(pts.at(0), pts.at(1));
    break;
    case BackgroundData::RadialGradient:
      {
        QLineF line(pts[0], pts[1]);
        if (line.length() > 132){
            line.setLength(132);
          }
        g = new QRadialGradient(line.p1(), qMin(gSize.width(), gSize.height()) / 3.0, line.p2());
      }
    break;
    case BackgroundData::ConicalGradient:
      {
        qreal angle = backgroundData.gangle;
        g = new QConicalGradient(pts.at(0), angle);
      }
    break;
    case BackgroundData::NoGradient:
    break;
    }

  for (int i=0; i<backgroundData.gstops.size(); ++i) {
      stops.append(backgroundData.gstops.at(i));
    }

  g->setStops(stops);
  g->setSpread(spread);
  g->setCoordinateMode(mode);

  GradientDialog *dialog = new GradientDialog(gSize,g);

  ok = dialog->exec() == QDialog::Accepted;
  if (ok){

      QGradient bgGradient = dialog->getGradient();
      backgroundData.gstops.clear();
      backgroundData.gpoints.clear();

      //type and points
      if (bgGradient.type() == QGradient::LinearGradient) {
          backgroundData.gtype = BackgroundData::LinearGradient;
          QLinearGradient &newbgGradient = (QLinearGradient&)bgGradient;
          backgroundData.gpoints << newbgGradient.start() << newbgGradient.finalStop();
        } else if (bgGradient.type() == QGradient::RadialGradient) {
          backgroundData.gtype = BackgroundData::RadialGradient;
          QRadialGradient &newbgGradient = (QRadialGradient&)bgGradient;
          backgroundData.gpoints << newbgGradient.center() << newbgGradient.focalPoint();
        } else {
          backgroundData.gtype = BackgroundData::ConicalGradient;
          QConicalGradient &newbgGradient = (QConicalGradient&)bgGradient;
          QLineF l(newbgGradient.center(), QPointF(0, 0));
          l.setAngle(newbgGradient.angle());
          l.setLength(120);
          backgroundData.gpoints << newbgGradient.center() << l.p2();
          backgroundData.gangle = newbgGradient.angle();
        }
      //spread
      if (bgGradient.spread() == QGradient::PadSpread){
          backgroundData.gspread = BackgroundData::PadSpread;
        } else if (bgGradient.spread() == QGradient::RepeatSpread){
          backgroundData.gspread = BackgroundData::RepeatSpread;
        } else {
          backgroundData.gspread = BackgroundData::ReflectSpread;
        }
      //mode
      if (bgGradient.coordinateMode() == QGradient::LogicalMode) {
          backgroundData.gmode = BackgroundData::LogicalMode;
        } else if (bgGradient.coordinateMode() == QGradient::StretchToDeviceMode) {
          backgroundData.gmode = BackgroundData::StretchToDeviceMode;
        } else {
          backgroundData.gmode = BackgroundData::ObjectBoundingMode;
        }
      //stops
      for (int i=0; i<bgGradient.stops().size(); i++)
        backgroundData.gstops.append(bgGradient.stops().at(i));
    }

  meta->setValue(backgroundData);
  modified = true;
}

void BackgroundGui::browsePicture(bool)
{
  BackgroundData background = meta->value();

  QString foo = QFileDialog::getOpenFileName(
    gui,
    tr("Choose Picture File"),
    picture,
    tr("Image Files (*.png *.jpg *.jpeg *.bmp)"));
  if (foo != "") {
    picture = foo;
    background.string = foo;
    pictureEdit->setText(foo);
    meta->setValue(background);
    modified = true;
  }
}

void BackgroundGui::browseColor(bool)
{
  BackgroundData background = meta->value();

  QColor qcolor = LDrawColor::color(color);
  QColor newColor = QColorDialog::getColor(qcolor,this);
  if (newColor.isValid() && qcolor != newColor) {
    color = newColor.name();
    background.string = newColor.name();
    QString styleSheet =
        QString("QLabel { background-color: rgb(%1, %2, %3); }").
        arg(newColor.red()).arg(newColor.green()).arg(newColor.blue());
    colorExample->setAutoFillBackground(true);
    colorExample->setStyleSheet(styleSheet);
    meta->setValue(background);
    modified = true;
  }
}

void BackgroundGui::stretch(bool checked)
{
  BackgroundData background = meta->value();
  background.stretch = checked;
  meta->setValue(background);
  modified = true;
}
void BackgroundGui::tile(bool checked)
{
  BackgroundData background = meta->value();
  background.stretch = ! checked;
  meta->setValue(background);
  modified = true;
}

void BackgroundGui::apply(QString &modelName)
{
  if (modified) {
    MetaItem mi;
    mi.setGlobalMeta(modelName,meta);
  }
}

/***********************************************************************
 *
 * Border
 *
 **********************************************************************/

BorderGui::BorderGui(
  BorderMeta *_meta,
  QGroupBox *parent,
  bool rotateArrow,
  bool corners)
{
  meta = _meta;

  BorderData border = meta->value();

  QString        string,chkBoxHideArrowsText;
  QGridLayout   *grid;

  grid = new QGridLayout(parent);
  parent->setLayout(grid);

  /* Arrows CheckBox */
  chkBoxHideArrowsText = border.hideArrows ? "Rotate Icon arrows hidden" : "Hide Rotate Icon arrows";
  hideArrowsChk = new QCheckBox(chkBoxHideArrowsText, parent);
  hideArrowsChk->setChecked(border.hideArrows);
  hideArrowsChk->setToolTip("Set checked when only icon image is desired.");
  connect(hideArrowsChk,SIGNAL(stateChanged(int)),
          this,            SLOT(  checkChange(int)));
  grid->addWidget(hideArrowsChk,0,0,1,3);

  /* Type Combo */

  typeCombo = new QComboBox(parent);
  typeCombo->addItem("Borderless");
  typeCombo->addItem("Square Corners");
  typeCombo->addItem("Round Corners");
  typeCombo->setCurrentIndex(int(border.type));
  connect(typeCombo,SIGNAL(currentIndexChanged(QString const &)),
          this, SLOT(  typeChange(         QString const &)));
  grid->addWidget(typeCombo,0,0);

  /* Radius */

  spinLabel = new QLabel("Radius",parent);
  grid->addWidget(spinLabel,0,1);

  spin = new QSpinBox(parent);
  spin->setRange(0,100);
  spin->setSingleStep(5);
  spin->setValue(int(border.radius));
  grid->addWidget(spin,0,2);
  connect(spin,SIGNAL(valueChanged(int)),
          this,SLOT(  radiusChange(int)));

  /* Line Combo */

  lineCombo = new QComboBox(parent);
  lineCombo->addItem("No Line");
  lineCombo->addItem("Solid Line");
  lineCombo->addItem("Dash Line");
  lineCombo->addItem("Dotted Line");
  lineCombo->addItem("Dot-Dash Line");
  lineCombo->addItem("Dot-Dot-Dash Line");
  lineCombo->setCurrentIndex(border.line);
  connect(lineCombo,SIGNAL(currentIndexChanged(QString const &)),
          this, SLOT(  lineChange(         QString const &)));
  grid->addWidget(lineCombo,1,0);


  /* Thickness */

  thicknessLabel = new QLabel("Width",parent);
  grid->addWidget(thicknessLabel,1,1);

  string = QString("%1") .arg(border.thickness,5,'f',4);
  thicknessEdit = new QLineEdit(string,parent);
  thicknessEdit->setInputMask("9.9000");
  connect(thicknessEdit,SIGNAL(textEdited(     QString const &)),
          this,         SLOT(  thicknessChange(QString const &)));
  grid->addWidget(thicknessEdit,1,2);

  /* Color */

  QLabel *label = new QLabel("Color",parent);
  grid->addWidget(label,2,0);

  colorExample = new QLabel(parent);
  colorExample->setFrameStyle(QFrame::Sunken|QFrame::Panel);
  QColor c = QColor(border.color);
  QString styleSheet =
      QString("QLabel { background-color: rgb(%1, %2, %3); }").
      arg(c.red()).arg(c.green()).arg(c.blue());
  colorExample->setAutoFillBackground(true);
  colorExample->setStyleSheet(styleSheet);
  grid->addWidget(colorExample,2,1);

  colorButton = new QPushButton("Change",parent);
  connect(colorButton,SIGNAL(clicked(    bool)),
          this,       SLOT(  browseColor(bool)));
  grid->addWidget(colorButton,2,2);

  /* Margins */

  label = new QLabel("Margins",parent);
  grid->addWidget(label,3,0);

  string = QString("%1") .arg(border.margin[0],5,'f',4);
  marginEditX = new QLineEdit(string,parent);
  grid->addWidget(marginEditX,3,1);
  connect(marginEditX,SIGNAL(textEdited(QString const &)),
          this,    SLOT(marginXChange(QString const &)));

  string = QString("%1") .arg(border.margin[1],5,'f',4);
  marginEditY = new QLineEdit(string,parent);
  grid->addWidget(marginEditY,3,2);
  connect(marginEditY,SIGNAL(textEdited(QString const &)),
          this,    SLOT(marginYChange(QString const &)));

  enable(rotateArrow);

  if (rotateArrow || !corners) {
      typeCombo->hide();
      spinLabel->hide();
      spin->hide();
      if (!corners)
          hideArrowsChk->hide();
  } else {
      hideArrowsChk->hide();
  }
}

void BorderGui::enable(bool rotateArrow)
{
  BorderData border = meta->value();

  switch (border.type) {
    case BorderData::BdrNone:
      lineCombo->setCurrentIndex(0);
      thicknessLabel->setEnabled(false);
      thicknessEdit->setEnabled(false);
      colorButton->setEnabled(false);
      spin->setEnabled(false);
      spinLabel->setEnabled(false);
    break;
    case BorderData::BdrSquare:
      if (rotateArrow && hideArrowsChk->isChecked()) {
          lineCombo->setEnabled(false);
          thicknessLabel->setEnabled(false);
          thicknessEdit->setEnabled(false);
          colorButton->setEnabled(false);
          marginEditX->setEnabled(false);
          marginEditY->setEnabled(false);
      } else {
          lineCombo->setEnabled(true);
          thicknessLabel->setEnabled(true);
          thicknessEdit->setEnabled(true);
          colorButton->setEnabled(true);
          marginEditX->setEnabled(true);
          marginEditY->setEnabled(true);
      }
      spin->setEnabled(false);
      spinLabel->setEnabled(false);
    break;
    default:
      lineCombo->setEnabled(true);
      thicknessLabel->setEnabled(true);
      thicknessEdit->setEnabled(true);
      colorButton->setEnabled(true);
      spin->setEnabled(true);
      spinLabel->setEnabled(true);
    break;
  }

  switch (border.line){
    case BorderData::BdrLnNone:
      thicknessLabel->setEnabled(false);
      thicknessEdit->setEnabled(false);
      colorButton->setEnabled(false);
      spin->setEnabled(false);
      spinLabel->setEnabled(false);
    break;
    case BorderData::BdrLnSolid:
    case BorderData::BdrLnDash:
    case BorderData::BdrLnDot:
    case BorderData::BdrLnDashDot:
    case BorderData::BdrLnDashDotDot:
    break;
    }
}
void BorderGui::typeChange(QString const &type)
{
  BorderData border = meta->value();

  if (type == "Square Corners") {
      border.type = BorderData::BdrSquare;
    } else if (type == "Round Corners"){
      border.type = BorderData::BdrRound;
    } else {
      border.type = BorderData::BdrNone;
    }

  meta->setValue(border);
  enable();
  modified = true;
}
void BorderGui::lineChange(QString const &line)
{
  BorderData border = meta->value();

  if (line == "Solid Line") {
      border.line = BorderData::BdrLnSolid;
    } else if (line == "Dash Line") {
      border.line = BorderData::BdrLnDash;
    } else if (line == "Dotted Line") {
      border.line = BorderData::BdrLnDot;
    } else if (line == "Dot-Dash Line") {
      border.line = BorderData::BdrLnDashDot;
    } else if (line == "Dot-Dot-Dash Line"){
      border.line = BorderData::BdrLnDashDotDot;
    } else {
      border.line = BorderData::BdrLnNone;
    }

  meta->setValue(border);
  enable();
  modified = true;
}

void BorderGui::checkChange(int value)
{
  Q_UNUSED(value);
  BorderData arrows = meta->value();

  arrows.hideArrows = hideArrowsChk->isChecked();
  if (hideArrowsChk->isChecked())
    hideArrowsChk->setText("Rotate Icon arrows hidden");
  else
    hideArrowsChk->setText("Hide Rotate Icon arrows");

  meta->setValue(arrows);
  enable(true); // Is Rotate Icon
  modified = true;
}

void BorderGui::browseColor(bool)
{
  BorderData border = meta->value();

  QColor color = LDrawColor::color(border.color);
  QColor newColor = QColorDialog::getColor(color,this);
  if (newColor.isValid() && color != newColor) {
    border.color = newColor.name();
    meta->setValue(border);
    QString styleSheet =
        QString("QLabel { background-color: rgb(%1, %2, %3); }").
        arg(newColor.red()).arg(newColor.green()).arg(newColor.blue());
    colorExample->setAutoFillBackground(true);
    colorExample->setStyleSheet(styleSheet);
    modified = true;
  }
}

void BorderGui::thicknessChange(QString const &thickness)
{
  BorderData border = meta->value();
  border.thickness = thickness.toFloat();
  meta->setValue(border);
  modified = true;
}
void BorderGui::radiusChange(int value)
{
  BorderData border = meta->value();
  border.radius = value;
  meta->setValue(border);
  modified = true;
}
void BorderGui::marginXChange(
  QString const &string)
{
  BorderData border = meta->value();
  border.margin[0] = string.toFloat();
  meta->setValue(border);
  modified = true;
}
void BorderGui::marginYChange(
  QString const &string)
{
  BorderData border = meta->value();
  border.margin[1] = string.toFloat();
  meta->setValue(border);
  modified = true;
}

void BorderGui::apply(QString &modelName)
{
  if (modified) {
    MetaItem mi;
    mi.setGlobalMeta(modelName,meta);
  }
}

/***********************************************************************
 *
 * Default Placement
 *
 **********************************************************************/

PlacementGui::PlacementGui(
        PlacementMeta *_meta,
        QString        _title,
        QGroupBox     *parent)
{
    meta = _meta;
    title = _title;

    QHBoxLayout *hlayout = new QHBoxLayout(parent);

    if (parent) {
        parent->setLayout(hlayout);
    } else {
        setLayout(hlayout);
    }

    placementLabel = new QLabel("Default placement",parent);
        hlayout->addWidget(placementLabel);

    placementButton = new QPushButton("Change " + title,parent);
    placementButton->setToolTip("Set default placement");
    hlayout->addWidget(placementButton);
    connect(placementButton,SIGNAL(clicked(   bool)),
            this,           SLOT(  placementChanged(bool)));

    placementModified             = false;
}

void PlacementGui::placementChanged(bool clicked)
{
  Q_UNUSED(clicked);
  PlacementData placementData = meta->value();
  bool ok;
  ok = PlacementDialog
       ::getPlacement(SingleStepType,PartsListType,placementData,title,ContentPage);
  if (ok) {
      meta->setValue(placementData);
      modified = placementModified = true;
  }
}

void PlacementGui::apply(QString &topLevelFile)
{
    if (placementModified){
        MetaItem mi;
        mi.beginMacro("PlacementModified");
        mi.setGlobalMeta(topLevelFile,meta);
        mi.endMacro();
    }
}

/***********************************************************************
 *
 * Pointer Attributes
 *
 **********************************************************************/

 PointerAttribGui::PointerAttribGui(
  PointerAttribMeta *_meta,
  QGroupBox         *parent,
     bool          _isCallout)
{
  meta   = _meta;
  PointerAttribData pad = meta->value();
  lineData = pad.attribType == PointerAttribData::Line;

  QString        string;
  QGridLayout   *grid;

  grid = new QGridLayout(parent);
  parent->setLayout(grid);

  /*  Attributes */

  int index;
  bool hideTip = false;
  QString title;
  QString thickness;
  QString color;
  if (lineData) {
      title = "Line";
      index = (int)pad.lineData.line - 1;            // - 1  adjusts for removal of 'No-Line'
      thickness = QString("%1") .arg(pad.lineData.thickness,5,'f',4);
      color = pad.lineData.color;
      hideTip = pad.lineData.hideArrows;
  } else {
      title = "Border";
      index = (int)pad.borderData.line - 1;
      thickness = QString("%1") .arg(pad.borderData.thickness,5,'f',4);
      color = pad.borderData.color;
  }

    /* Line Combo */

  lineCombo = new QComboBox(parent);
  lineCombo->addItem("Solid Line");
  lineCombo->addItem("Dash Line");
  lineCombo->addItem("Dotted Line");
  lineCombo->addItem("Dot-Dash Line");
  lineCombo->addItem("Dot-Dot-Dash Line");
  lineCombo->setCurrentIndex(index); //
  connect(lineCombo,SIGNAL(currentIndexChanged(QString const &)),
          this,       SLOT(  lineChange(       QString const &)));
  grid->addWidget(lineCombo,0,0);

    /*  Width */

  thicknessLabel = new QLabel(title+" Width",parent);
  grid->addWidget(thicknessLabel,0,1);

  string = thickness;
  thicknessEdit = new QLineEdit(string,parent);
  thicknessEdit->setInputMask("9.9000");
  thicknessEdit->setToolTip(QString("In %1)")
                            .arg(Preferences::preferCentimeters ? "centimetres" : "inches"));
  connect(thicknessEdit,SIGNAL(textEdited(   QString const &)),
          this,         SLOT(thicknessChange(QString const &)));
  grid->addWidget(thicknessEdit,0,2);

  /*  Color */

  QLabel *Label = new QLabel(title+" Color",parent);
  grid->addWidget(Label,1,0);

  colorExample = new QLabel(parent);
  colorExample->setFrameStyle(QFrame::Sunken|QFrame::Panel);
  QColor c = QColor(color);
  QString StyleSheet =
      QString("QLabel { background-color: %1; }")
      .arg(color.isEmpty() ? "transparent" :
           QString("rgb(%1, %2, %3)")
           .arg(c.red()).arg(c.green()).arg(c.blue()));
  colorExample->setAutoFillBackground(true);
  colorExample->setStyleSheet(StyleSheet);
  grid->addWidget(colorExample,1,1);

  colorButton = new QPushButton("Change",parent);
  connect(colorButton,SIGNAL(clicked(   bool)),
          this,        SLOT(browseColor(bool)));
  grid->addWidget(colorButton,1,2);

  /* hide arrows [optional] */
  if (lineData && !_isCallout) {
      hideTipBox = new QCheckBox("Hide Pointer Tip", parent);
      hideTipBox->setChecked(hideTip);
      connect(hideTipBox,SIGNAL(clicked(    bool)),
              this,      SLOT(hideTipChange(bool)));
      grid->addWidget(hideTipBox,2,0,1,3);
  }

}

void PointerAttribGui::lineChange(QString const &line)
{
  BorderData::Line padLine = BorderData::BdrLnSolid;

  if (line == "Solid Line") {
      padLine = BorderData::BdrLnSolid;
    } else if (line == "Dash Line") {
      padLine = BorderData::BdrLnDash;
    } else if (line == "Dotted Line") {
      padLine = BorderData::BdrLnDot;
    } else if (line == "Dot-Dash Line") {
      padLine = BorderData::BdrLnDashDot;
    } else if (line == "Dot-Dot-Dash Line"){
      padLine = BorderData::BdrLnDashDotDot;
    }

  PointerAttribData pad = meta->value();
  if (lineData)
      pad.lineData.line = padLine;
  else
      pad.borderData.line = padLine;

  meta->setValue(pad);
  modified = true;
}

void PointerAttribGui::thicknessChange(QString const &thickness)
{
  PointerAttribData pad = meta->value();
  if (lineData)
      pad.lineData.thickness = thickness.toFloat();
  else
      pad.borderData.thickness = thickness.toFloat();
  meta->setValue(pad);
  modified = true;
}

void PointerAttribGui::browseColor(bool)
{
  PointerAttribData pad = meta->value();
  QString padColor;
  if (lineData)
      padColor = pad.lineData.color;
  else
      padColor = pad.borderData.color;
  QColor color = LDrawColor::color(padColor);
  QColor newColor = QColorDialog::getColor(color,this);
  if (newColor.isValid() && color != newColor) {
    if (lineData)
        pad.lineData.color = newColor.name();
    else
        pad.borderData.color = newColor.name();
    meta->setValue(pad);
    QString styleSheet =
        QString("QLabel { background-color: rgb(%1, %2, %3); }").
        arg(newColor.red()).arg(newColor.green()).arg(newColor.blue());
    colorExample->setAutoFillBackground(true);
    colorExample->setStyleSheet(styleSheet);
    modified = true;
  }
}

void PointerAttribGui::hideTipChange(bool checked)
{
    PointerAttribData pad = meta->value();
    pad.lineData.hideArrows = checked;
    meta->setValue(pad);
    modified = true;
}

void PointerAttribGui::apply(QString &modelName)
{
   if (modified) {
     MetaItem mi;
     mi.setGlobalMeta(modelName,meta);
   }
}

/***********************************************************************
 *
 * Separator/Divider
 *
 **********************************************************************/

SepGui::SepGui(
  SepMeta   *_meta,
  QGroupBox *parent)
{
  meta = _meta;

  QGridLayout *grid = new QGridLayout(parent);
  parent->setLayout(grid);

  QLabel      *label;
  QLineEdit   *lineEdit;
  QPushButton *button;
  QComboBox   *typeCombo;

  label = new QLabel("Width",parent);
  grid->addWidget(label,0,0);

  QString string;

  SepData sep = meta->value();

  string = QString("%1") .arg(sep.thickness,
                              5,'f',4);
  lineEdit = new QLineEdit(string,parent);
  connect(lineEdit,SIGNAL(textEdited(QString const &)),
          this,    SLOT(  thicknessChange(QString const &)));
  grid->addWidget(lineEdit,0,1);

  label = new QLabel("Length",parent);
  grid->addWidget(label,1,0);

  typeCombo = new QComboBox(parent);
  typeCombo->addItem("Default");
  typeCombo->addItem("Page");
  typeCombo->addItem("Custom");
  typeCombo->setCurrentIndex(int(sep.type));
  connect(typeCombo,SIGNAL(currentIndexChanged(int)),
          this,     SLOT(  typeChange(             int)));
  grid->addWidget(typeCombo,1,1);

  string = QString("%1") .arg(sep.length,
                              5,'f',4);
  typeLineEdit = new QLineEdit(string,parent);
  typeLineEdit->setEnabled(sep.type == SepData::LenCustom);
  connect(typeLineEdit,SIGNAL(textEdited(QString const &)),
          this,        SLOT(  lengthChange(QString const &)));
  grid->addWidget(typeLineEdit,1,2);

  label = new QLabel("Color",parent);
  grid->addWidget(label,2,0);

  colorExample = new QLabel(parent);
  colorExample->setFrameStyle(QFrame::Sunken|QFrame::Panel);
  colorExample->setAutoFillBackground(true);
  QColor c = QColor(sep.color);
  QString styleSheet =
      QString("QLabel { background-color: rgb(%1, %2, %3); }").
      arg(c.red()).arg(c.green()).arg(c.blue());
  colorExample->setStyleSheet(styleSheet);
  grid->addWidget(colorExample,2,1);

  button = new QPushButton("Change",parent);
  connect(button,SIGNAL(clicked(bool)),
          this,  SLOT(  browseColor(bool)));
  grid->addWidget(button,2,2);

  label = new QLabel("Margins",parent);
  grid->addWidget(label,3,0);

  string = QString("%1") .arg(sep.margin[0],5,'f',4);
  lineEdit = new QLineEdit(string,parent);
  grid->addWidget(lineEdit,3,1);
  connect(lineEdit,SIGNAL(textEdited(QString const &)),
          this,    SLOT(marginXChange(QString const &)));

  string = QString("%1") .arg(sep.margin[1],5,'f',4);
  lineEdit = new QLineEdit(string,parent);
  grid->addWidget(lineEdit,3,2);
  connect(lineEdit,SIGNAL(textEdited(QString const &)),
          this,    SLOT(marginYChange(QString const &)));
}

void SepGui::typeChange(
  int type)
{
  SepData sep = meta->value();
  sep.type = SepData::LengthType(type);
  meta->setValue(sep);
  typeLineEdit->setEnabled(sep.type == SepData::LenCustom);
  modified = true;
}

void SepGui::lengthChange(
  QString const &string)
{
  SepData sep = meta->value();
  sep.length = string.toFloat();
  meta->setValue(sep);
  modified = true;
}

void SepGui::thicknessChange(
  QString const &string)
{
  SepData sep = meta->value();
  sep.thickness = string.toFloat();
  meta->setValue(sep);
  modified = true;
}

void SepGui::browseColor(
  bool clicked)
{
  Q_UNUSED(clicked);
  SepData sep = meta->value();

  QColor qcolor = LDrawColor::color(sep.color);
  QColor newColor = QColorDialog::getColor(qcolor,this);
  if (newColor.isValid() && qcolor != newColor) {
      colorExample->setAutoFillBackground(true);
      QString styleSheet =
          QString("QLabel { background-color: rgb(%1, %2, %3); }")
          .arg(newColor.red()).arg(newColor.green()).arg(newColor.blue());
      colorExample->setStyleSheet(styleSheet);
      sep.color = newColor.name();
      meta->setValue(sep);
      modified = true;
    }
}

void SepGui::marginXChange(
  QString const &string)
{
  SepData sep = meta->value();
  sep.margin[0] = string.toFloat();
  meta->setValue(sep);
  modified = true;
}

void SepGui::marginYChange(
  QString const &string)
{
  SepData sep = meta->value();
  sep.margin[1] = string.toFloat();
  meta->setValue(sep);
  modified = true;
}

void SepGui::apply(QString &modelName)
{
  if (modified) {
    MetaItem mi;
    mi.setGlobalMeta(modelName,meta);
  }
}

/***********************************************************************
 *
 * Resolution
 *
 **********************************************************************/

ResolutionGui::ResolutionGui(
  ResolutionMeta   *_meta,
  QGroupBox        *parent)
{
  meta = _meta;

  QGridLayout *grid = new QGridLayout(parent);

  QLabel    *label;

  label = new QLabel("Units",parent);
  grid->addWidget(label,0,0);

  type  = meta->type();
  value = meta->value();
  // default value always inches
  // so convert to centimeters if DPCM
  if (meta->isDefault() && type == DPCM) {
      value = inches2centimeters(meta->value());
  }

  QComboBox *combo;

  combo = new QComboBox(parent);
  combo->addItem("Dots Per Inch");
  combo->addItem("Dots Per Centimetre");
  combo->setCurrentIndex(int(type));
  connect(combo,SIGNAL(currentIndexChanged(QString const &)),
          this, SLOT(  unitsChange(        QString const &)));
  grid->addWidget(combo,0,1);

  QString string;

  string = QString("%1") .arg(int(value),4);
  valueEdit = new QLineEdit(string,parent);
  valueEdit->setInputMask("9999");
  connect(valueEdit,SIGNAL(textEdited( QString const &)),
          this,     SLOT(  valueChange(QString const &)));
  grid->addWidget(valueEdit,0,2);
  parent->setLayout(grid);
}

void ResolutionGui::unitsChange(QString const &units)
{
  if (units == "Dots Per Centimetre")
    type = DPCM;
  else
    type = DPI;

  float tvalue = 0.0;

  if (type == meta->type()) {
    tvalue = value;
  } else if (type == DPCM /*DPI*/) { // Changed to Centimeters
    tvalue = inches2centimeters(value)+0.5;
  } else {
    tvalue = centimeters2inches(value)+0.5;
  }

  QString string = QString("%1") .arg(int(tvalue));
  valueEdit->setText(string);
}

void ResolutionGui::valueChange(
  QString const &string)
{
  value = string.toFloat();
}

void ResolutionGui::differences()
{
  if (type == meta->type()) {
    if (value < meta->value() || value > meta->value()) {
      meta->setValue(type,value);
      modified = true;
    }
  } else if (type == DPI) {
    // We must convert all units in project to inches
    meta->setValue(type,value);
    modified = true;
  } else {
    // We must convert all units in project to centimetres
    meta->setValue(type,value);
    modified = true;
  }
}

void ResolutionGui::apply(QString &modelName)
{
  differences();
  if (modified) {
    MetaItem mi;
    mi.setGlobalMeta(modelName,meta);
  }
}

/***********************************************************************
 *
 * Renderer
 *
 **********************************************************************/

RendererGui::RendererGui(
  NativeCDMeta *_meta,
  QGroupBox     *parent)
{
  meta = _meta;
  QGridLayout *grid = new QGridLayout(parent);
  QHBoxLayout *hLayout = new QHBoxLayout();

  if (parent) {
    parent->setLayout(grid);
  } else {
    setLayout(grid);
  }

  combo = new QComboBox(parent);
  if (Preferences::ldgliteExe != "") {
    combo->addItem(RENDERER_LDGLITE);
  }
  if (Preferences::ldviewExe != "") {
    combo->addItem(RENDERER_LDVIEW);
  }
  if (Preferences::povrayExe != "") {
    combo->addItem(RENDERER_POVRAY);
  }
  combo->addItem(RENDERER_NATIVE);

  QString renderer = Render::getRenderer();
  combo->setCurrentIndex(int(combo->findText(renderer)));
  connect(combo,SIGNAL(currentIndexChanged(QString const &)),
          this, SLOT(  typeChange(         QString const &)));
  grid->addWidget(combo,0,0,2,1);

  ldvSingleCallBox = new QCheckBox("Use LDView Single Call",parent);
  ldvSingleCallBox->setToolTip("Process a page's part or assembly images in a single renderer call");
  ldvSingleCallBox->setChecked(Preferences::enableLDViewSingleCall);
  ldvSingleCallBox->setEnabled(renderer == RENDERER_LDVIEW);
  connect(ldvSingleCallBox,SIGNAL(clicked(bool)),
          this,            SLOT(singleCallChange(bool)));
  grid->addWidget(ldvSingleCallBox,0,1);


  ldvSnapshotListBox = new QCheckBox("Use LDView Snapshot List",parent);
  ldvSnapshotListBox->setToolTip("Capture Single Call images in a single list file");
  ldvSnapshotListBox->setChecked(Preferences::enableLDViewSnaphsotList);
  ldvSnapshotListBox->setEnabled(renderer == RENDERER_LDVIEW &&
                                 (Preferences::enableLDViewSingleCall ||
                                  ldvSingleCallBox->isChecked()));
  connect(ldvSnapshotListBox,SIGNAL(clicked(bool)),
          this,              SLOT(snapshotListChange(bool)));
  grid->addWidget(ldvSnapshotListBox,1,1);

  povFileGeneratorGrpBox = new QGroupBox("POV File Generation Renderer",parent);
  povFileGeneratorGrpBox->setEnabled(renderer == RENDERER_POVRAY);
  povFileGeneratorGrpBox->setLayout(hLayout);
  grid->addWidget(povFileGeneratorGrpBox,2,0,1,2);

  nativeButton = new QRadioButton("Native",povFileGeneratorGrpBox);
  nativeButton->setChecked(Preferences::povFileGenerator == RENDERER_NATIVE);
  connect(nativeButton,SIGNAL(clicked(bool)),
          this,        SLOT(povFileGenNativeChange(bool)));
  hLayout->addWidget(nativeButton);

  ldvButton = new QRadioButton("LDView",povFileGeneratorGrpBox);
  ldvButton->setChecked(Preferences::povFileGenerator == RENDERER_LDVIEW);
  connect(ldvButton,SIGNAL(clicked(bool)),
          this,     SLOT(povFileGenLDViewChange(bool)));
  hLayout->addWidget(ldvButton);

  QGridLayout *grpGrid = new QGridLayout();
  cameraDistFactorGrpBox = new QGroupBox("3DViewer/Native Renderer Camera Distance",parent);

  QString tipMessage = QString("Adjust to change the default Native camera distance.");
  cameraDistFactorGrpBox->setToolTip(tipMessage);
  cameraDistFactorGrpBox->setLayout(grpGrid);
  grid->addWidget(cameraDistFactorGrpBox,3,0,1,2);

  cameraDistFactorLabel = new QLabel("Factor", cameraDistFactorGrpBox);
  grpGrid->addWidget(cameraDistFactorLabel,0,0);

  cameraDistFactorNative = meta->factor.value();
  cameraDistFactorSpin = new QSpinBox(parent);
  cameraDistFactorSpin->setRange(-5000,5000);
  cameraDistFactorSpin->setSingleStep(10);
  cameraDistFactorSpin->setValue(cameraDistFactorNative);
  connect(cameraDistFactorSpin,SIGNAL(valueChanged(int)),
          this,                SLOT(cameraDistFactorChange(int)));
  grpGrid->addWidget(cameraDistFactorSpin,0,1);

  QSettings Settings;
  cameraDistFactorDefaulSetting = Settings.contains(QString("%1/%2").arg(SETTINGS,"CameraDistFactorNative"));
  cameraDistFactorDefaultBox = new QCheckBox("Set as default",cameraDistFactorGrpBox);
  cameraDistFactorDefaultBox->setToolTip("Save to application settings.");
  cameraDistFactorDefaultBox->setChecked(cameraDistFactorDefaulSetting);
  grpGrid->addWidget(cameraDistFactorDefaultBox,1,0);

  cameraDistFactorMetaBox = new QCheckBox("Add meta command",cameraDistFactorGrpBox);
  cameraDistFactorMetaBox->setToolTip("Add a global meta command to the LDraw file.");
  cameraDistFactorMetaBox->setChecked(!cameraDistFactorDefaulSetting);
  grpGrid->addWidget(cameraDistFactorMetaBox,1,1);

  clearCaches              = false;
  rendererModified         = false;
  singleCallModified       = false;
  snapshotListModified     = false;
  povFileGenModified       = false;
  cameraDistFactorModified = false;
}

void RendererGui::povFileGenNativeChange(bool checked)
{
  povFileGenChoice = (checked ? RENDERER_NATIVE : RENDERER_LDVIEW);
  if (Preferences::povFileGenerator != povFileGenChoice){
       clearCaches = povFileGenModified = true;
    }
}

void RendererGui::povFileGenLDViewChange(bool checked)
{
  povFileGenChoice = (checked ? RENDERER_LDVIEW : RENDERER_NATIVE );
  if (Preferences::povFileGenerator != povFileGenChoice){
       povFileGenModified = true;
    }
}

void RendererGui::singleCallChange(bool checked)
{
  if (Preferences::enableLDViewSingleCall != checked) {
       modified = singleCallModified = true;
    }
  ldvSnapshotListBox->setEnabled(checked);
}

void RendererGui::snapshotListChange(bool checked)
{
  if (Preferences::enableLDViewSnaphsotList != checked) {
       modified = snapshotListModified = true;
    }
}

void RendererGui::typeChange(QString const &type)
{
  pick = type;
  ldvSingleCallBox->setEnabled(pick == RENDERER_LDVIEW);
  povFileGeneratorGrpBox->setEnabled(pick == RENDERER_POVRAY);
  if (pick != Preferences::preferredRenderer) {
      modified = rendererModified = true;
   }
}

void RendererGui::cameraDistFactorChange(int factor)
{
  meta->factor.setValue(factor);
  cameraDistFactorModified = true;
  if (Preferences::usingNativeRenderer)
      clearCaches = true;
}

void RendererGui::apply(QString &topLevelFile)
{
  QSettings Settings;
  if (rendererModified) {
      changeMessage = QString("Renderer preference changed from %1 to %2 %3")
                     .arg(Preferences::preferredRenderer)
                     .arg(pick)
                     .arg(pick == RENDERER_POVRAY ? QString("(POV file generator is %1)").arg(Preferences::povFileGenerator) :
                          pick == RENDERER_LDVIEW ? Preferences::enableLDViewSingleCall ? "(Single Call)" : "" : "");
      emit gui->messageSig(LOG_INFO, changeMessage);
      Preferences::preferredRenderer = pick;
      Settings.setValue(QString("%1/%2").arg(SETTINGS,"PreferredRenderer"),pick);
      Render::setRenderer(pick);
    }
  if (singleCallModified) {
      changeMessage = QString("Use LDView Single Call is %1")
                      .arg(Preferences::enableLDViewSingleCall ? "ON" : "OFF");
      emit gui->messageSig(LOG_INFO, changeMessage);
      Preferences::enableLDViewSingleCall = ldvSingleCallBox->isChecked();
      QVariant uValue(ldvSingleCallBox->isChecked());
      Settings.setValue(QString("%1/%2").arg(SETTINGS,"EnableLDViewSingleCall"),uValue);
    }
  if (snapshotListModified) {
      changeMessage = QString("Use LDView Snapshot List is %1")
                      .arg(Preferences::enableLDViewSnaphsotList ? "ON" : "OFF");
      emit gui->messageSig(LOG_INFO, changeMessage);
      Preferences::enableLDViewSnaphsotList = ldvSnapshotListBox->isChecked();
      QVariant uValue(ldvSnapshotListBox->isChecked());
      Settings.setValue(QString("%1/%2").arg(SETTINGS,"EnableLDViewSnapshotsList"),uValue);
    }
  if (povFileGenModified) {
      changeMessage = QString("POV file generation renderer changed from %1 to %2")
                      .arg(Preferences::povFileGenerator)
                      .arg(povFileGenChoice);
      emit gui->messageSig(LOG_INFO, changeMessage);
      Preferences::povFileGenerator = povFileGenChoice;
      Settings.setValue(QString("%1/%2").arg(SETTINGS,"POVFileGenerator"),povFileGenChoice);
    }
  if (cameraDistFactorModified) {
      if (!cameraDistFactorDefaultBox->isChecked() &&
          !cameraDistFactorMetaBox->isChecked())
          cameraDistFactorDefaultBox->setChecked(true);
      if (cameraDistFactorMetaBox->isChecked()){
          if (cameraDistFactorDefaulSetting) {
              Settings.remove(QString("%1/%2").arg(SETTINGS,"CameraDistFactorNative"));
          }
          MetaItem mi;
          mi.setGlobalMeta(topLevelFile,&meta->factor);
      }
      else
      if (cameraDistFactorDefaultBox->isChecked()){
          Preferences::cameraDistFactorNative = meta->factor.value();
          Settings.setValue(QString("%1/%2").arg(SETTINGS,"CameraDistFactorNative"),meta->factor.value());
          changeMessage = QString("Native camera distance factor changed from %1 to %2")
                                  .arg(cameraDistFactorNative)
                                  .arg(meta->factor.value());
          emit gui->messageSig(LOG_INFO, changeMessage);
      }
  }
  if (!modified && clearCaches){
      clearPliCache();
      clearCsiCache();
      clearSubmodelCache();
      clearTempCache();
    }
}

/***********************************************************************
 *
 * Show Submodel
 *
 **********************************************************************/

ShowSubModelGui::ShowSubModelGui(
        SubModelMeta  *_meta,
        QGroupBox     *parent)
{
    meta = _meta;

    QGridLayout *grid = new QGridLayout(parent);

    if (parent) {
        parent->setLayout(grid);
    } else {
        setLayout(grid);
    }

    showSubmodelsBox = new QCheckBox("Show submodel at first step",parent);
    showSubmodelsBox->setToolTip("Show Submodel image on first step page");
    showSubmodelsBox->setChecked(meta->show.value());
    connect(showSubmodelsBox,SIGNAL(clicked(bool)),
            this,            SLOT(showSubmodelsChange(bool)));
    grid->addWidget(showSubmodelsBox,0,0,1,2);

    QSettings Settings;
    showSubmodelsDefaultSettings = Settings.contains(QString("%1/%2").arg(SETTINGS,"ShowSubmodels"));
    showSubmodelsDefaultBox = new QCheckBox("Set as default",parent);
    showSubmodelsDefaultBox->setToolTip("Save show submodel to application settings.");
    showSubmodelsDefaultBox->setChecked(showSubmodelsDefaultSettings);
    grid->addWidget(showSubmodelsDefaultBox,1,0);

    showSubmodelsMetaBox = new QCheckBox("Add meta command",parent);
    showSubmodelsMetaBox->setToolTip("Add show submodel as a global meta command to the LDraw file.");
    showSubmodelsMetaBox->setChecked(!showSubmodelsDefaultSettings);
    grid->addWidget(showSubmodelsMetaBox,1,1);

    QFrame* line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    grid->addWidget(line,2,0,1,2);

    showTopModelBox = new QCheckBox("Show main model at first step",parent);
    showTopModelBox->setToolTip("Show main model image on first step page");
    showTopModelBox->setChecked(meta->showTopModel.value());
    connect(showTopModelBox,SIGNAL(clicked(bool)),
            this,           SLOT(showTopModelChange(bool)));
    grid->addWidget(showTopModelBox,3,0,1,2);

    showTopModelDefaultSettings = Settings.contains(QString("%1/%2").arg(SETTINGS,"ShowTopModel"));
    showTopModelDefaultBox = new QCheckBox("Set as default",parent);
    showTopModelDefaultBox->setToolTip("Save show top model to application settings.");
    showTopModelDefaultBox->setChecked(showTopModelDefaultSettings);
    grid->addWidget(showTopModelDefaultBox,4,0);

    showTopModelMetaBox = new QCheckBox("Add meta command",parent);
    showTopModelMetaBox->setToolTip("Add show top model  as a global meta command to the LDraw file.");
    showTopModelMetaBox->setChecked(!showTopModelDefaultSettings);
    grid->addWidget(showTopModelMetaBox,4,1);

    line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    grid->addWidget(line,5,0,1,2);

    showInstanceCountBox = new QCheckBox("Show submodel instance count",parent);
    showInstanceCountBox->setToolTip("Show Submodel instance count");
    showInstanceCountBox->setChecked(meta->showInstanceCount.value());
    connect(showInstanceCountBox,SIGNAL(clicked(bool)),
            this,                SLOT(showInstanceCountChange(bool)));
    connect(showInstanceCountBox,SIGNAL(clicked(bool)),
            this,                SIGNAL(instanceCountClicked(bool)));
    grid->addWidget(showInstanceCountBox,6,0,1,2);

    showInstanceCountDefaultSettings = Settings.contains(QString("%1/%2").arg(SETTINGS,"ShowInstanceCount"));
    showInstanceCountDefaultBox = new QCheckBox("Set as default",parent);
    showInstanceCountDefaultBox->setToolTip("Save show submodel instance count to application settings.");
    showInstanceCountDefaultBox->setChecked(showInstanceCountDefaultSettings);
    grid->addWidget(showInstanceCountDefaultBox,7,0);

    showInstanceCountMetaBox = new QCheckBox("Add meta command",parent);
    showInstanceCountMetaBox->setToolTip("Add show submodel instance count as a global meta command to the LDraw file.");
    showInstanceCountMetaBox->setChecked(!showInstanceCountDefaultSettings);
    grid->addWidget(showInstanceCountMetaBox,7,1);

    line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    grid->addWidget(line,8,0,1,2);

    QLabel *placementLabel = new QLabel("Default placement",parent);
    grid->addWidget(placementLabel,9,0);

    placementButton = new QPushButton("Change Placement",parent);
    placementButton->setToolTip("Set Submodel default placement");
    connect(placementButton,SIGNAL(clicked(   bool)),
            this,           SLOT(  placementChanged(bool)));
    grid->addWidget(placementButton,9,1);

    showSubmodelsModified         = false;
    showTopModelModified          = false;
    showInstanceCountModified     = false;
    placementModified             = false;

    enableSubmodelControls(meta->show.value());
}

void ShowSubModelGui::showSubmodelsChange(bool checked)
{
    if (meta->show.value() != checked) {
        meta->show.setValue(checked);
        modified = showSubmodelsModified = true;
    }
    enableSubmodelControls(checked);
}

void ShowSubModelGui::showTopModelChange(bool checked)
{
    if (meta->showTopModel.value() != checked) {
        meta->showTopModel.setValue(checked);
        modified = showTopModelModified = true;
    }
}

void ShowSubModelGui::showInstanceCountChange(bool checked)
{
    if (meta->showInstanceCount.value() != checked) {
        meta->showInstanceCount.setValue(checked);
        modified = showInstanceCountModified = true;
    }
}

void ShowSubModelGui::placementChanged(bool clicked)
{
  Q_UNUSED(clicked);
  PlacementData placementData = meta->placement.value();
  bool ok;
  ok = PlacementDialog
       ::getPlacement(SingleStepType,SubModelType,placementData,"Submodel Placement",ContentPage);
  if (ok) {
      meta->placement.setValue(placementData);
      modified = placementModified = true;
  }
}

void ShowSubModelGui::enableSubmodelControls(bool checked)
{
    showSubmodelsDefaultBox->setEnabled(checked);
    showSubmodelsMetaBox->setEnabled(checked);

    showTopModelBox->setEnabled(checked);
    showTopModelDefaultBox->setEnabled(checked);
    showTopModelMetaBox->setEnabled(checked);

    showInstanceCountBox->setEnabled(checked);
    showInstanceCountDefaultBox->setEnabled(checked);
    showInstanceCountMetaBox->setEnabled(checked);

    placementButton->setEnabled(checked);
}

void ShowSubModelGui::apply(QString &topLevelFile)
{
    QSettings Settings;
    QString changeMessage;
    if (showSubmodelsModified) {
        changeMessage = QString("Show submodels is %1")
                                .arg(meta->show.value() ? "ON" : "OFF");
        emit gui->messageSig(LOG_INFO, changeMessage);
        if (showSubmodelsDefaultBox->isChecked()){
            changeMessage = QString("Show submodels added as application default.");
            emit gui->messageSig(LOG_INFO, changeMessage);
            Preferences::showSubmodels = meta->show.value();
            Settings.setValue(QString("%1/%2").arg(SETTINGS,"ShowSubmodels"),meta->show.value());
        }
        else
        if (showSubmodelsDefaultSettings) {
            Settings.remove(QString("%1/%2").arg(SETTINGS,"ShowSubmodels"));
        }

        if (showSubmodelsMetaBox->isChecked()){
            MetaItem mi;
            mi.beginMacro("SubModelMeta");
            mi.setGlobalMeta(topLevelFile,&meta->show);
            mi.endMacro();
        }
    }
    if (showTopModelModified) {
        changeMessage = QString("Show top model is %1")
                                .arg(meta->showTopModel.value() ? "ON" : "OFF");
        emit gui->messageSig(LOG_INFO, changeMessage);
        if (showTopModelDefaultBox->isChecked()){
            changeMessage = QString("Show top model added as application default.");
            emit gui->messageSig(LOG_INFO, changeMessage);
            Preferences::showTopModel = meta->showTopModel.value();
            Settings.setValue(QString("%1/%2").arg(SETTINGS,"ShowTopModel"),meta->showTopModel.value());
        }
        else
        if (showTopModelDefaultSettings) {
            Settings.remove(QString("%1/%2").arg(SETTINGS,"ShowTopModel"));
        }

        if (showTopModelMetaBox->isChecked()){
            MetaItem mi;
            mi.beginMacro("TopModelMeta");
            mi.setGlobalMeta(topLevelFile,&meta->showTopModel);
            mi.endMacro();
        }
    }
    if (showInstanceCountModified) {
        changeMessage = QString("Show submodel instance count is %1")
                                .arg(meta->showInstanceCount.value() ? "ON" : "OFF");
        emit gui->messageSig(LOG_INFO, changeMessage);
        if (showInstanceCountDefaultBox->isChecked()){
            changeMessage = QString("Show submodel instance count added as application default.");
            emit gui->messageSig(LOG_INFO, changeMessage);
            Preferences::showInstanceCount = meta->showInstanceCount.value();
            Settings.setValue(QString("%1/%2").arg(SETTINGS,"ShowInstanceCount"),meta->showInstanceCount.value());
        }
        else
        if (showInstanceCountDefaultSettings) {
            Settings.remove(QString("%1/%2").arg(SETTINGS,"ShowInstanceCount"));
        }

        if (showInstanceCountMetaBox->isChecked()){
            MetaItem mi;
            mi.beginMacro("ShowInstanceCount");
            mi.setGlobalMeta(topLevelFile,&meta->showInstanceCount);
            mi.endMacro();
        }
    }
    if (placementModified){
        MetaItem mi;
        mi.beginMacro("PlacementModified");
        mi.setGlobalMeta(topLevelFile,&meta->placement);
        mi.endMacro();
    }
}

/***********************************************************************
 *
 * PliSort [DEPRECATED]
 *
 **********************************************************************/

PliSortGui::PliSortGui(
  const QString   &heading,
  PliSortMeta     *_meta,
  QGroupBox       *parent,
  bool             bom)
{
  meta = _meta;

  QGridLayout *grid = new QGridLayout(parent);

  if (parent) {
    parent->setLayout(grid);
  }

  if (heading != "") {
    headingLabel = new QLabel(heading,parent);
    grid->addWidget(headingLabel);
  } else {
    headingLabel = nullptr;
  }

  QLabel      *label;
  label = new QLabel("Sort By",parent);
  grid->addWidget(label,0,0);

  int currentIndex;
  sortOption  = meta->sortOption.value();
  sortOption == SortOptionName[PartSize]     ? currentIndex = PartSize :
  sortOption == SortOptionName[PartColour]   ? currentIndex = PartColour :
  sortOption == SortOptionName[PartCategory] ? currentIndex = PartCategory :
                                         bom ? currentIndex = PartElement :
                                               currentIndex = PartSize;

  combo = new QComboBox(parent);
  combo->addItem(SortOptionName[PartSize]);
  combo->addItem(SortOptionName[PartColour]);
  combo->addItem(SortOptionName[PartCategory]);
  if (bom)
      combo->addItem(SortOptionName[PartElement]);
  combo->setCurrentIndex(currentIndex);
  connect(combo,SIGNAL(currentIndexChanged(QString const &)),
          this, SLOT(  optionChange(       QString const &)));
  grid->addWidget(combo,0,1);

  modified = false;
}

void PliSortGui::optionChange(QString const &sortOption)
{
  StringMeta sortBy = meta->sortOption;
  sortBy.setValue(sortOption);
  meta->sortOption.setValue(sortBy.value());
  modified = true;
}

void PliSortGui::apply(QString &topLevelFile)
{
  if (modified) {
    MetaItem mi;
    mi.setGlobalMeta(topLevelFile,&meta->sortOption);
  }
}

/***********************************************************************
 *
 * PliSortOrder
 *
 **********************************************************************/

PliSortOrderGui::PliSortOrderGui(
  const QString     &heading,
  PliSortOrderMeta *_meta,
  QGroupBox         *parent,
  bool              _bom)
{
  meta = _meta;
  bom  = _bom;

  QHBoxLayout *hLayout;
  QGridLayout *grid = new QGridLayout(parent);

  if (parent) {
    parent->setLayout(grid);
  }

  if (heading != "") {
    headingLabel = new QLabel(heading,parent);
    grid->addWidget(headingLabel);
  } else {
    headingLabel = nullptr;
  }

  int priSort = tokenMap[meta->primary.value()];
  int secSort = tokenMap[meta->secondary.value()];
  int triSort = tokenMap[meta->tertiary.value()];

  // Direction Groups
  gbPrimary = new QGroupBox("Primary",parent);
  gbPrimary->setEnabled(priSort != NoSort);
  hLayout = new QHBoxLayout();
  gbPrimary->setLayout(hLayout);
  grid->addWidget(gbPrimary,0,0);

  primaryAscendingRadio = new QRadioButton("Asc",gbPrimary);
  primaryAscendingRadio->setToolTip("Sort Ascending");
  primaryAscendingRadio->setChecked(tokenMap[meta->primaryDirection.value()] == SortAscending);
  hLayout->addWidget(primaryAscendingRadio);
  connect(primaryAscendingRadio,SIGNAL(clicked(bool)),
          this,                 SLOT(  directionChange(bool)));

  primaryDescendingRadio = new QRadioButton("Des",gbPrimary);
  primaryDescendingRadio->setToolTip("Sort Descending");
  primaryDescendingRadio->setChecked(tokenMap[meta->primaryDirection.value()] == SortDescending);
  hLayout->addWidget(primaryDescendingRadio);
  connect(primaryDescendingRadio,SIGNAL(clicked(bool)),
          this,                  SLOT(  directionChange(bool)));

  gbSecondary = new QGroupBox("Secondary",parent);
  gbSecondary->setEnabled(secSort != NoSort);
  hLayout = new QHBoxLayout();
  gbSecondary->setLayout(hLayout);
  grid->addWidget(gbSecondary,0,1);

  secondaryAscendingRadio = new QRadioButton("Asc",gbSecondary);
  secondaryAscendingRadio->setToolTip("Sort Ascending");
  secondaryAscendingRadio->setChecked(tokenMap[meta->secondaryDirection.value()] == SortAscending);
  hLayout->addWidget(secondaryAscendingRadio);
  connect(secondaryAscendingRadio,SIGNAL(clicked(bool)),
          this,                   SLOT(  directionChange(bool)));

  secondaryDescendingRadio = new QRadioButton("Des",gbSecondary);
  secondaryDescendingRadio->setToolTip("Sort Descending");
  secondaryDescendingRadio->setChecked(tokenMap[meta->secondaryDirection.value()] == SortDescending);
  hLayout->addWidget(secondaryDescendingRadio);
  connect(secondaryDescendingRadio,SIGNAL(clicked(bool)),
          this,                    SLOT(  directionChange(bool)));

  gbTertiary = new QGroupBox("Tertiary",parent);
  gbTertiary->setEnabled(triSort != NoSort);
  hLayout = new QHBoxLayout();
  gbTertiary->setLayout(hLayout);
  grid->addWidget(gbTertiary,0,2);

  tertiaryAscendingRadio = new QRadioButton("Asc",gbTertiary);
  tertiaryAscendingRadio->setToolTip("Sort Ascending");
  tertiaryAscendingRadio->setChecked(tokenMap[meta->tertiaryDirection.value()] == SortAscending);
  hLayout->addWidget(tertiaryAscendingRadio);
  connect(tertiaryAscendingRadio,SIGNAL(clicked(bool)),
          this,                  SLOT(  directionChange(bool)));

  tertiaryDescendingRadio = new QRadioButton("Des",gbTertiary);
  tertiaryDescendingRadio->setToolTip("Sort Descending");
  tertiaryDescendingRadio->setChecked(tokenMap[meta->tertiaryDirection.value()] == SortDescending);
  hLayout->addWidget(tertiaryDescendingRadio);
  connect(tertiaryDescendingRadio,SIGNAL(clicked(bool)),
          this,                   SLOT(  directionChange(bool)));

  primaryCombo   = new QComboBox(parent);
  secondaryCombo = new QComboBox(parent);
  tertiaryCombo  = new QComboBox(parent);

  QList<QComboBox *> cbl;
  cbl << primaryCombo
      << secondaryCombo
      << tertiaryCombo;
  for (const auto cb : cbl) {
      for (const auto &so : SortOptionName) {
          if (so == SortOptionName[PartElement] && !bom)
              continue;
          cb->addItem(so);
      }
  }

  // adjust indices for missing PartElement option when not bom
  auto OptionIndex = [this](const QString &value){
      int opt = tokenMap[value];
      return bom ? opt : opt == NoSort ? PartElement : opt;
  };

  primaryCombo->setCurrentIndex(OptionIndex(meta->primary.value()));
  connect(primaryCombo,SIGNAL(currentIndexChanged(int)),
          this,        SLOT(  orderChange(        int)));
  grid->addWidget(primaryCombo,1,0);

  secondaryCombo->setCurrentIndex(OptionIndex(meta->secondary.value()));
  connect(secondaryCombo,SIGNAL(currentIndexChanged(int)),
          this,          SLOT(  orderChange(        int)));
  grid->addWidget(secondaryCombo,1,1);

  tertiaryCombo->setCurrentIndex(OptionIndex(meta->tertiary.value()));
  connect(tertiaryCombo,SIGNAL(currentIndexChanged(int)),
          this,         SLOT(  orderChange(        int)));
  grid->addWidget(tertiaryCombo,1,2);

  if (meta->tertiary.value()  == SortOptionName[NoSort]){
      gbTertiary->setEnabled(false);
  }
  if (meta->secondary.value() == SortOptionName[NoSort]){
      tertiaryCombo->setEnabled(false);
      gbSecondary->setEnabled(false);
      gbTertiary->setEnabled(false);
  }
  if (meta->primary.value()   == SortOptionName[NoSort]){
      secondaryCombo->setEnabled(false);
      tertiaryCombo->setEnabled(false);
      gbPrimary->setEnabled(false);
      gbSecondary->setEnabled(false);
      gbTertiary->setEnabled(false);
  }

  primaryModified            = false;
  secondaryModified          = false;
  tertiaryModified           = false;
  primaryDirectionModified   = false;
  secondaryDirectionModified = false;
  tertiaryDirectionModified  = false;

  primaryDuplicateOption     = false;
  secondaryDuplicateOption   = false;
  tertiaryDuplicateOption    = false;
}

void PliSortOrderGui::duplicateOption(
  QComboBox *combo,
  bool resetOption,
  bool resetText)
{
    if (resetOption){
        resetOption = false;
        if (primaryDuplicateOption) {
            primaryDuplicateOption = resetOption;
            duplicateOption(primaryCombo,resetOption,true);
        }
        if (secondaryDuplicateOption) {
            secondaryDuplicateOption = resetOption;
            duplicateOption(secondaryCombo,resetOption,true);
        }
        if (tertiaryDuplicateOption) {
            tertiaryDuplicateOption = resetOption;
            duplicateOption(tertiaryCombo,resetOption,true);
        }
        return;
    }

    QFont comboFont = combo->font();
    if (resetText){
        comboFont.setBold(false);
        combo->setFont(comboFont);
        combo->setPalette(QApplication::palette(combo));
        return;
    }

    QPalette comboPalette = combo->palette();
    QModelIndex index = ((QTreeView *)combo->view())->currentIndex();
    if(!index.parent().isValid()) // parent index
    {
        comboFont.setBold(true);
        comboPalette.setColor(QPalette::Text, Qt::red);
        comboPalette.setColor(QPalette::WindowText, Qt::red);
    }
    else
    {
        comboFont.setBold(false);
        if(combo->parentWidget() != nullptr)
            comboPalette = combo->parentWidget()->palette();
    }
    combo->setFont(comboFont);
    combo->setPalette(comboPalette);
}

void PliSortOrderGui::orderChange(int option)
{
  bool enable;
  // adjust indices for missing PartElement option when not bom
  int NoSortIndex = bom ? NoSort : PartElement;
  int OptionIndex = bom ? option : option == PartElement ? NoSort : option;
  if (sender() == primaryCombo) {
      enable = option != NoSortIndex;
      meta->primary.setValue(SortOptionName[OptionIndex]);
      secondaryCombo->setEnabled(enable);
      tertiaryCombo->setEnabled(enable);
      gbPrimary->setEnabled(enable);
      gbSecondary->setEnabled(enable);
      gbTertiary->setEnabled(enable);
      duplicateOption(tertiaryCombo,true);
      if ((primaryDuplicateOption =
           secondaryCombo->currentIndex() == option ||
           tertiaryCombo->currentIndex() == option) &&
           option != NoSortIndex) {
          duplicateOption(primaryCombo);
      }
      modified = primaryModified = true;
  }
  if (sender() == secondaryCombo) {
      enable = (option != NoSortIndex &&
                primaryCombo->currentIndex() != NoSortIndex);
      meta->secondary.setValue(SortOptionName[OptionIndex]);
      tertiaryCombo->setEnabled(enable);
      gbSecondary->setEnabled(enable);
      gbTertiary->setEnabled(enable);
      duplicateOption(tertiaryCombo,true);
      if ((secondaryDuplicateOption =
           primaryCombo->currentIndex() == option ||
           tertiaryCombo->currentIndex() == option) &&
           option != NoSortIndex) {
          duplicateOption(secondaryCombo);
      }
      modified = secondaryModified = true;
  }
  if (sender() == tertiaryCombo) {
      enable = (option != NoSortIndex &&
                primaryCombo->currentIndex()   != NoSortIndex &&
                secondaryCombo->currentIndex() != NoSortIndex);
      meta->tertiary.setValue(SortOptionName[OptionIndex]);
      gbTertiary->setEnabled(enable);
      duplicateOption(tertiaryCombo,true);
      if ((tertiaryDuplicateOption =
          (primaryCombo->currentIndex() == option ||
           secondaryCombo->currentIndex() == option) &&
           option != NoSortIndex)) {
          duplicateOption(tertiaryCombo);
      }
      modified = tertiaryModified = true;
  }
}

void PliSortOrderGui::directionChange(bool clicked)
{
    Q_UNUSED(clicked)
    if (sender() == primaryAscendingRadio) {
        meta->primaryDirection.setValue(SortDirectionName[SortAscending]);
        modified = primaryDirectionModified = true;
    }
    if (sender() == primaryDescendingRadio) {
        meta->primaryDirection.setValue(SortDirectionName[SortDescending]);
        modified = primaryDirectionModified = true;
    }

    if (sender() == secondaryAscendingRadio) {
        meta->secondaryDirection.setValue(SortDirectionName[SortAscending]);
        modified = secondaryDirectionModified = true;
    }
    if (sender() == secondaryDescendingRadio) {
        meta->secondaryDirection.setValue(SortDirectionName[SortDescending]);
        modified = secondaryDirectionModified = true;
    }

    if (sender() == tertiaryAscendingRadio) {
        meta->tertiaryDirection.setValue(SortDirectionName[SortAscending]);
        modified = tertiaryDirectionModified = true;
    }
    if (sender() == tertiaryDescendingRadio) {
        meta->tertiaryDirection.setValue(SortDirectionName[SortDescending]);
        modified = tertiaryDirectionModified = true;
    }
}

void PliSortOrderGui::apply(QString &topLevelFile)
{
  MetaItem mi;
  mi.beginMacro("PliSortOrderSettings");
  if (tertiaryDirectionModified) {
      mi.setGlobalMeta(topLevelFile,&meta->tertiaryDirection);
  }
  if (secondaryDirectionModified) {
      mi.setGlobalMeta(topLevelFile,&meta->secondaryDirection);
  }
  if (primaryDirectionModified) {
      mi.setGlobalMeta(topLevelFile,&meta->primaryDirection);
  }

  if (tertiaryModified) {
      mi.setGlobalMeta(topLevelFile,&meta->tertiary);
  }
  if (secondaryModified) {
      mi.setGlobalMeta(topLevelFile,&meta->secondary);
  }
  if (primaryModified) {
      mi.setGlobalMeta(topLevelFile,&meta->primary);
  }
  mi.endMacro();
}

/***********************************************************************
 *
 * PliPartElements
 *
 **********************************************************************/

PliPartElementGui::PliPartElementGui(
    const QString      &heading,
    PliPartElementMeta *_meta,
    QGroupBox          *parent)
{
  meta = _meta;

  QVBoxLayout *vLayout = new QVBoxLayout(parent);
  QHBoxLayout *hLayout = new QHBoxLayout(nullptr);

  if (parent) {
      parent->setLayout(vLayout);
  } else {
      setLayout(vLayout);
  }

  if (heading != "") {
      headingLabel = new QLabel(heading,parent);
      vLayout->addWidget(headingLabel);
  } else {
      headingLabel = nullptr;
  }

  //PLIAnnotation
  gbPliPartElement = new QGroupBox("Display Part Element Annotation",parent);
  gbPliPartElement->setCheckable(true);
  gbPliPartElement->setChecked(meta->display.value());
  gbPliPartElement->setLayout(hLayout);
  vLayout->addWidget(gbPliPartElement);
  connect(gbPliPartElement,SIGNAL(toggled(bool)),
          this,            SIGNAL(toggled(bool)));
  connect(gbPliPartElement,SIGNAL(toggled(bool)),
          this,            SLOT(  gbToggled(bool)));

  bricklinkElementsButton = new QRadioButton("BrickLink",gbPliPartElement);
  bricklinkElementsButton->setToolTip("Use BrickLink element identification");
  connect(bricklinkElementsButton,SIGNAL(clicked(bool)),
          this,                   SLOT(  bricklinkElements(bool)));
  hLayout->addWidget(bricklinkElementsButton);

  legoElementsButton = new QRadioButton("LEGO",gbPliPartElement);
  legoElementsButton->setToolTip("Use LEGO element identification");
  connect(legoElementsButton,SIGNAL(clicked(bool)),
          this,              SLOT(  legoElements(bool)));
  hLayout->addWidget(legoElementsButton);

  localLegoElementsCheck = new QCheckBox("Local",gbPliPartElement);
  localLegoElementsCheck->setToolTip("Use local LEGO Elements file. Default uses BrickLink's LEGO Elements");
  connect(localLegoElementsCheck,SIGNAL(clicked(bool)),
          this,                  SLOT(  localLegoElements(bool)));
  hLayout->addWidget(localLegoElementsCheck);

  bricklinkElementsButton->setChecked(meta->bricklinkElements.value());
  legoElementsButton->setChecked(meta->legoElements.value());
  localLegoElementsCheck->setChecked(meta->localLegoElements.value());
  localLegoElementsCheck->setEnabled(gbPliPartElement->isChecked() &&
                                     legoElementsButton->isChecked());

  displayModified           = false;
  bricklinkElementsModified = false;
  legoElementsModified      = false;
  localLegoElementsModified = false;
}

void PliPartElementGui::bricklinkElements(bool checked)
{
  meta->bricklinkElements.setValue(checked);
  meta->legoElements.setValue(! checked);
  localLegoElementsCheck->setDisabled(checked);
  modified = bricklinkElementsModified = true;
}

void PliPartElementGui::legoElements(bool checked)
{
  meta->bricklinkElements.setValue(! checked);
  meta->legoElements.setValue( checked);
  localLegoElementsCheck->setEnabled(checked);
  modified = legoElementsModified = true;
}

void PliPartElementGui::localLegoElements(bool checked)
{
  meta->localLegoElements.setValue( checked);
  modified = localLegoElementsModified = true;
}

void PliPartElementGui::gbToggled(bool toggled)
{
  meta->display.setValue(toggled);
  if(toggled){
      bricklinkElementsButton->setChecked(meta->bricklinkElements.value());
      legoElementsButton->setChecked(meta->legoElements.value());
      localLegoElementsCheck->setChecked(meta->localLegoElements.value());
  }
  localLegoElementsCheck->setEnabled(toggled && legoElementsButton->isChecked());
  modified = displayModified = true;
}

void PliPartElementGui::enablePliPartElementGroup(bool checked)
{
    gbPliPartElement->setEnabled(checked);
}

void PliPartElementGui::apply(QString &topLevelFile)
{
  MetaItem mi;
  mi.beginMacro("PliPartElementSettings");
  if (displayModified) {
      mi.setGlobalMeta(topLevelFile,&meta->display);
  }
  if (bricklinkElementsModified) {
      mi.setGlobalMeta(topLevelFile,&meta->bricklinkElements);
  }
  if (legoElementsModified) {
      mi.setGlobalMeta(topLevelFile,&meta->legoElements);
  }
  if (localLegoElementsModified) {
      mi.setGlobalMeta(topLevelFile,&meta->localLegoElements);
  }
  mi.endMacro();
}

/***********************************************************************
 *
 * PliAnnotation
 *
 **********************************************************************/

PliAnnotationGui::PliAnnotationGui(
    const QString     &heading,
    PliAnnotationMeta *_meta,
    QGroupBox         *parent,
    bool               bom)
{
  meta = _meta;

  QGridLayout *grid = new QGridLayout(parent);
  QHBoxLayout *hLayout = new QHBoxLayout(nullptr);

  if (parent) {
      parent->setLayout(grid);
  } else {
      setLayout(grid);
  }

  if (heading != "") {
      headingLabel = new QLabel(heading,parent);
      grid->addWidget(headingLabel);
    } else {
      headingLabel = nullptr;
    }

  //PLIAnnotation
  gbPLIAnnotation = new QGroupBox(tr("Display %1 Annotation").arg(bom ? "Bill Of Materials (BOM)" : "Part List (PLI)" ),parent);
  gbPLIAnnotation->setCheckable(true);
  gbPLIAnnotation->setChecked(meta->display.value());
  gbPLIAnnotation->setLayout(hLayout);
  grid->addWidget(gbPLIAnnotation,0,0);
  connect(gbPLIAnnotation,SIGNAL(toggled(bool)),
          this,           SIGNAL(toggled(bool)));
  connect(gbPLIAnnotation,SIGNAL(toggled(bool)),
          this,           SLOT(  gbToggled(bool)));

  bool titleAndFreeForm = meta->titleAndFreeformAnnotation.value();

  titleAnnotationCheck = new QCheckBox("Title",gbPLIAnnotation);
  titleAnnotationCheck->setChecked(titleAndFreeForm ? true : meta->titleAnnotation.value());
  titleAnnotationCheck->setToolTip("Extended background style shape annotations - user configurable");
  connect(titleAnnotationCheck,SIGNAL(clicked(bool)),
          this,                 SLOT(  titleAnnotation(bool)));
  connect(titleAnnotationCheck,SIGNAL(clicked()),
          this,                 SLOT(  enableAnnotations()));
  connect(titleAnnotationCheck,SIGNAL(clicked()),
          this,                 SLOT(  enableExtendedStyle()));
  hLayout->addWidget(titleAnnotationCheck);

  freeformAnnotationCheck = new QCheckBox("Free Form",gbPLIAnnotation);
  freeformAnnotationCheck->setChecked(titleAndFreeForm ? true : meta->freeformAnnotation.value());
  freeformAnnotationCheck->setToolTip("Extended background style shape annotations - user configurable");
  connect(freeformAnnotationCheck,SIGNAL(clicked(bool)),
          this,                    SLOT(  freeformAnnotation(bool)));
  connect(freeformAnnotationCheck,SIGNAL(clicked()),
          this,                    SLOT(  enableAnnotations()));
  connect(freeformAnnotationCheck,SIGNAL(clicked()),
          this,                    SLOT(  enableExtendedStyle()));
  hLayout->addWidget(freeformAnnotationCheck);

  fixedAnnotationsCheck = new QCheckBox("Fixed",gbPLIAnnotation);
  fixedAnnotationsCheck->setChecked(meta->fixedAnnotations.value());
  fixedAnnotationsCheck->setToolTip("Fixed background style shape annotations - axle, beam, cable, connector, hose and panel.");
  connect(fixedAnnotationsCheck,SIGNAL(clicked(bool)),
          this,                 SLOT(  fixedAnnotations(bool)));
  connect(fixedAnnotationsCheck,SIGNAL(clicked()),
          this,                 SLOT(  enableAnnotations()));
  hLayout->addWidget(fixedAnnotationsCheck);

  bool styleEnabled = meta->enableStyle.value();

  // PLI Annotation Style Options
  gbPLIAnnotationStyle = new QGroupBox("Enable Annotation Style",parent);
  QGridLayout *sgrid = new QGridLayout();
  gbPLIAnnotationStyle->setLayout(sgrid);
  grid->addWidget(gbPLIAnnotationStyle,1,0);
  gbPLIAnnotationStyle->setCheckable(true);
  gbPLIAnnotationStyle->setChecked(styleEnabled);
  gbPLIAnnotationStyle->setEnabled(meta->display.value());
  connect(gbPLIAnnotationStyle,SIGNAL(toggled(bool)),
          this,                SLOT(gbStyleToggled(bool)));

  axleStyleCheck = new QCheckBox("Axles",gbPLIAnnotationStyle);
  axleStyleCheck->setChecked(meta->axleStyle.value());
  axleStyleCheck->setEnabled(styleEnabled && meta->fixedAnnotations.value());
  axleStyleCheck->setToolTip("Fixed Axle annotation on circle background");
  connect(axleStyleCheck,SIGNAL(clicked(bool)),
          this,          SLOT(  axleStyle(bool)));
  sgrid->addWidget(axleStyleCheck,0,0);

  beamStyleCheck = new QCheckBox("Beams",gbPLIAnnotationStyle);
  beamStyleCheck->setChecked(meta->beamStyle.value());
  beamStyleCheck->setEnabled(styleEnabled && meta->fixedAnnotations.value());
  beamStyleCheck->setToolTip("Fixed Beam annotation on square background");
  connect(beamStyleCheck,SIGNAL(clicked(bool)),
          this,          SLOT(  beamStyle(bool)));
  sgrid->addWidget(beamStyleCheck,0,1);

  cableStyleCheck = new QCheckBox("Cables",gbPLIAnnotationStyle);
  cableStyleCheck->setChecked(meta->cableStyle.value());
  cableStyleCheck->setEnabled(styleEnabled && meta->fixedAnnotations.value());
  cableStyleCheck->setToolTip("Fixed Cable annotation on square background");
  connect(cableStyleCheck,SIGNAL(clicked(bool)),
          this,           SLOT(  cableStyle(bool)));
  sgrid->addWidget(cableStyleCheck,0,2);

  connectorStyleCheck = new QCheckBox("Connectors",gbPLIAnnotationStyle);
  connectorStyleCheck->setChecked(meta->connectorStyle.value());
  connectorStyleCheck->setEnabled(styleEnabled && meta->fixedAnnotations.value());
  connectorStyleCheck->setToolTip("Fixed Connector annotation on square background");
  connect(connectorStyleCheck,SIGNAL(clicked(bool)),
          this,               SLOT(  connectorStyle(bool)));
  sgrid->addWidget(connectorStyleCheck,0,3);

  elementStyleCheck = new QCheckBox("Elements",gbPLIAnnotationStyle);
  elementStyleCheck->setChecked(meta->elementStyle.value());
  elementStyleCheck->setToolTip("Fixed Part Element ID annotation on rectanglular background");
  elementStyleCheck->setVisible(bom);
  connect(elementStyleCheck,SIGNAL(clicked(bool)),
          this,              SLOT(  elementStyle(bool)));
  sgrid->addWidget(elementStyleCheck,1,0);

  hoseStyleCheck = new QCheckBox("Hoses",gbPLIAnnotationStyle);
  hoseStyleCheck->setChecked(meta->hoseStyle.value());
  hoseStyleCheck->setEnabled(styleEnabled && meta->fixedAnnotations.value());
  hoseStyleCheck->setToolTip("Fixed Hose annotation on square background");
  connect(hoseStyleCheck,SIGNAL(clicked(bool)),
          this,          SLOT(  hoseStyle(bool)));
  sgrid->addWidget(hoseStyleCheck,1,bom ? 1 : 0);

  panelStyleCheck = new QCheckBox("Panels",gbPLIAnnotationStyle);
  panelStyleCheck->setChecked(meta->panelStyle.value());
  panelStyleCheck->setEnabled(styleEnabled && meta->fixedAnnotations.value());
  panelStyleCheck->setToolTip("Fixed Panel annotation on circle background");
  connect(panelStyleCheck,SIGNAL(clicked(bool)),
          this,           SLOT(  panelStyle(bool)));
  sgrid->addWidget(panelStyleCheck,1,bom ? 2 : 1);

  extendedStyleCheck = new QCheckBox("Extended",gbPLIAnnotationStyle);
  extendedStyleCheck->setChecked(meta->extendedStyle.value());
  extendedStyleCheck->setToolTip("Title or Freeform annotation on rectanglular background");
  connect(extendedStyleCheck,SIGNAL(clicked(bool)),
          this,              SLOT(  extendedStyle(bool)));
  sgrid->addWidget(extendedStyleCheck,1,bom ? 3 : 2);

  titleModified            = false;
  freeformModified         = false;
  titleAndFreeformModified = false;
  fixedAnnotationsModified = false;

  displayModified          = false;
  enableStyleModified      = false;

  axleStyleModified        = false;
  beamStyleModified        = false;
  cableStyleModified       = false;
  elementStyleModified     = false;
  connectorStyleModified   = false;
  extendedStyleModified    = false;
  hoseStyleModified        = false;
  panelStyleModified       = false;

}

void PliAnnotationGui::titleAnnotation(bool checked)
{
  meta->titleAnnotation.setValue(checked);
  if (freeformAnnotationCheck->isChecked() == checked)
      titleAndFreeformAnnotation(checked);
  modified = titleModified = true;
}

void PliAnnotationGui::freeformAnnotation(bool checked)
{
  meta->freeformAnnotation.setValue( checked);
  if (titleAnnotationCheck->isChecked() == checked)
      titleAndFreeformAnnotation(checked);
  modified = freeformModified = true;
}

void PliAnnotationGui::titleAndFreeformAnnotation(bool checked)
{      
  if (meta->titleAndFreeformAnnotation.value() == checked)
      return;
  meta->titleAndFreeformAnnotation.setValue( checked);
  modified = titleAndFreeformModified = true;

}

void PliAnnotationGui::fixedAnnotations(bool checked)
{
  axleStyleCheck->setEnabled(checked);
  beamStyleCheck->setEnabled(checked);
  cableStyleCheck->setEnabled(checked);
  connectorStyleCheck->setEnabled(checked);
  hoseStyleCheck->setEnabled(checked);
  panelStyleCheck->setEnabled(checked);

  meta->fixedAnnotations.setValue( checked);
  modified = fixedAnnotationsModified = true;
}

void PliAnnotationGui::enableExtendedStyle()
{
    bool enabled = true;
    if (!titleAnnotationCheck->isChecked() &&
        !freeformAnnotationCheck->isChecked())
    {
        enabled = false;
    }
    extendedStyleCheck->setEnabled(enabled);
}

void PliAnnotationGui::enableAnnotations()
{
    bool enabled = true;
    if (!titleAnnotationCheck->isChecked() &&
        !freeformAnnotationCheck->isChecked() &&
        !fixedAnnotationsCheck->isChecked())
    {
        enabled = false;
    }
    gbPLIAnnotation->setChecked(enabled);
    if (meta->display.value() != enabled)
    {
        meta->display.setValue(enabled);
        modified = displayModified = true;
    }
}

/* Never called because fixed controls are disabled
 * when fixedAnnotationsCheck is unchecked */
void PliAnnotationGui::setFixedAnnotations(bool checked)
{
  if (checked)
  {
    if (!fixedAnnotationsCheck->isChecked())
    {
        fixedAnnotationsCheck->setChecked(checked);
        fixedAnnotations(checked);
    }
  }
}

void PliAnnotationGui::axleStyle(bool checked)
{
  setFixedAnnotations(checked);
  meta->axleStyle.setValue(checked);
  modified = axleStyleModified = true;
}

void PliAnnotationGui::beamStyle(bool checked)
{
  setFixedAnnotations(checked);
  meta->beamStyle.setValue(checked);
  modified = beamStyleModified = true;
}

void PliAnnotationGui::cableStyle(bool checked)
{
  setFixedAnnotations(checked);
  meta->cableStyle.setValue(checked);
  modified = cableStyleModified = true;
}

void PliAnnotationGui::connectorStyle(bool checked)
{
  setFixedAnnotations(checked);
  meta->connectorStyle.setValue(checked);
  modified = connectorStyleModified = true;
}

void PliAnnotationGui::elementStyle(bool checked)
{
  meta->elementStyle.setValue(checked);
  modified = elementStyleModified = true;
}


void PliAnnotationGui::extendedStyle(bool checked)
{
  meta->extendedStyle.setValue(checked);
  modified = extendedStyleModified = true;
}

void PliAnnotationGui::hoseStyle(bool checked)
{
  setFixedAnnotations(checked);
  meta->hoseStyle.setValue(checked);
  modified = hoseStyleModified = true;
}

void PliAnnotationGui::panelStyle(bool checked)
{
  setFixedAnnotations(checked);
  meta->panelStyle.setValue(checked);
  modified = panelStyleModified = true;
}

void PliAnnotationGui::gbToggled(bool checked)
{
    bool saveModified = meta->display.value();
    meta->display.setValue(checked);
    if(checked){
        bool titleAndFreeForm = meta->titleAndFreeformAnnotation.value();
        titleAnnotationCheck->setChecked(titleAndFreeForm ? true : meta->titleAnnotation.value());
        freeformAnnotationCheck->setChecked(titleAndFreeForm ? true : meta->freeformAnnotation.value());
        fixedAnnotationsCheck->setChecked(meta->fixedAnnotations.value());
    }

    gbPLIAnnotationStyle->setEnabled(checked);

    if (saveModified != meta->display.value())
        modified = displayModified = true;
}

void PliAnnotationGui::gbStyleToggled(bool checked)
{
    if (meta->enableStyle.value() != checked) {
        meta->enableStyle.setValue(checked);
        modified = enableStyleModified = true;
    }
    axleStyleCheck->setEnabled(checked);
    beamStyleCheck->setEnabled(checked);
    cableStyleCheck->setEnabled(checked);
    connectorStyleCheck->setEnabled(checked);
    hoseStyleCheck->setEnabled(checked);
    panelStyleCheck->setEnabled(checked);
}

void PliAnnotationGui::enableElementStyle(bool checked)
{
    elementStyleCheck->setEnabled(checked);
}

void PliAnnotationGui::apply(QString &topLevelFile)
{

//  QProgressDialog* ProgressDialog = new QProgressDialog(nullptr);
//  ProgressDialog->setWindowFlags(ProgressDialog->windowFlags() & ~Qt::WindowCloseButtonHint);
//  ProgressDialog->setWindowTitle(tr("Global Settings"));
//  ProgressDialog->setLabelText(tr("Applying Pli Annotation Settings..."));
//  ProgressDialog->setMaximum(11);
//  ProgressDialog->setMinimum(0);
//  ProgressDialog->setValue(0);
//  ProgressDialog->setCancelButton(nullptr);
//  ProgressDialog->setAutoReset(false);
//  ProgressDialog->setModal(true);
//  ProgressDialog->show();

//  int commands = 0;

  MetaItem mi;
  mi.beginMacro("PliAnnotationSettings");
  if (displayModified) {
      mi.setGlobalMeta(topLevelFile,&meta->display);
//      ProgressDialog->setValue(++commands);
//      QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
  }
  if (enableStyleModified){
      mi.setGlobalMeta(topLevelFile, &meta->enableStyle);
  }
  if (titleModified) {
      mi.setGlobalMeta(topLevelFile,&meta->titleAnnotation);
  }
  if (freeformModified) {
      mi.setGlobalMeta(topLevelFile,&meta->freeformAnnotation);
  }
  if (titleAndFreeformModified) {
      mi.setGlobalMeta(topLevelFile,&meta->titleAndFreeformAnnotation);
  }
  if (fixedAnnotationsModified) {
      mi.setGlobalMeta(topLevelFile,&meta->fixedAnnotations);
  }
  if (axleStyleModified) {
      mi.setGlobalMeta(topLevelFile,&meta->axleStyle);
  }
  if (beamStyleModified) {
      mi.setGlobalMeta(topLevelFile,&meta->beamStyle);
  }
  if (cableStyleModified) {
      mi.setGlobalMeta(topLevelFile,&meta->cableStyle);
  }
  if (connectorStyleModified) {
      mi.setGlobalMeta(topLevelFile,&meta->connectorStyle);
  }
  if (elementStyleModified) {
      mi.setGlobalMeta(topLevelFile,&meta->elementStyle);
  }
  if (extendedStyleModified) {
      mi.setGlobalMeta(topLevelFile,&meta->extendedStyle);
  }
  if (hoseStyleModified) {
      mi.setGlobalMeta(topLevelFile,&meta->hoseStyle);
  }
  if (panelStyleModified) {
      mi.setGlobalMeta(topLevelFile,&meta->panelStyle);
  }
  mi.endMacro();
//  ProgressDialog->setValue(commands);
//  QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
//  ProgressDialog->deleteLater();
}

/***********************************************************************
 *
 * CsiAnnotation
 *
 **********************************************************************/

CsiAnnotationGui::CsiAnnotationGui(
    const QString        &heading,
    CsiAnnotationMeta   *_meta,
    QGroupBox           *parent,
    bool                 fixedAnnotations)
{
  meta = _meta;

  QString fixedMessage;
  if (!fixedAnnotations)
      fixedMessage =
              QString("Display Part List (PLI) Annotations 'Fixed Annotations' must be enabled to set this fixed annotaiton display");

  QGridLayout *grid = new QGridLayout(parent);

  if (parent) {
      parent->setLayout(grid);
   } else {
      setLayout(grid);
   }

  if (heading != "") {
      headingLabel = new QLabel(heading,parent);
      grid->addWidget(headingLabel);
    } else {
      headingLabel = nullptr;
    }

  // CSI Annotation Display Options
  gbCSIAnnotationDisplay = new QGroupBox("Display Assembly (CSI) Part Annotation",parent);
  gbCSIAnnotationDisplay->setCheckable(true);
  gbCSIAnnotationDisplay->setChecked(meta->display.value());
  QGridLayout *sgrid = new QGridLayout();
  gbCSIAnnotationDisplay->setLayout(sgrid);
  grid->addWidget(gbCSIAnnotationDisplay);
  connect(gbCSIAnnotationDisplay,SIGNAL(toggled(bool)),
          this,                  SLOT(  gbToggled(bool)));

  axleDisplayCheck = new QCheckBox("Axles",gbCSIAnnotationDisplay);
  axleDisplayCheck->setChecked(meta->axleDisplay.value());
  axleDisplayCheck->setEnabled(fixedAnnotations);
  axleDisplayCheck->setToolTip(fixedAnnotations ? "Display Axle annotation" : fixedMessage);
  connect(axleDisplayCheck,SIGNAL(clicked(bool)),
          this,            SLOT(  axleDisplay(bool)));
  sgrid->addWidget(axleDisplayCheck,0,0);

  beamDisplayCheck = new QCheckBox("Beams",gbCSIAnnotationDisplay);
  beamDisplayCheck->setChecked(meta->beamDisplay.value());
  beamDisplayCheck->setEnabled(fixedAnnotations);
  beamDisplayCheck->setToolTip(fixedAnnotations ? "Display Beam annotation" : fixedMessage);
  connect(beamDisplayCheck,SIGNAL(clicked(bool)),
          this,            SLOT(  beamDisplay(bool)));
  sgrid->addWidget(beamDisplayCheck,0,1);

  cableDisplayCheck = new QCheckBox("Cables",gbCSIAnnotationDisplay);
  cableDisplayCheck->setChecked(meta->cableDisplay.value());
  cableDisplayCheck->setEnabled(fixedAnnotations);
  cableDisplayCheck->setToolTip(fixedAnnotations ? "Display Cable annotation" : fixedMessage);
  connect(cableDisplayCheck,SIGNAL(clicked(bool)),
          this,             SLOT(  cableDisplay(bool)));
  sgrid->addWidget(cableDisplayCheck,0,2);

  connectorDisplayCheck = new QCheckBox("Connectors",gbCSIAnnotationDisplay);
  connectorDisplayCheck->setChecked(meta->connectorDisplay.value());
  connectorDisplayCheck->setEnabled(fixedAnnotations);
  connectorDisplayCheck->setToolTip(fixedAnnotations ? "Display Connector annotation" : fixedMessage);
  connect(connectorDisplayCheck,SIGNAL(clicked(bool)),
          this,                 SLOT(  connectorDisplay(bool)));
  sgrid->addWidget(connectorDisplayCheck,0,3);

  hoseDisplayCheck = new QCheckBox("Hoses",gbCSIAnnotationDisplay);
  hoseDisplayCheck->setChecked(meta->hoseDisplay.value());
  hoseDisplayCheck->setEnabled(fixedAnnotations);
  hoseDisplayCheck->setToolTip(fixedAnnotations ? "Display Hose annotation" : fixedMessage);
  connect(hoseDisplayCheck,SIGNAL(clicked(bool)),
          this,            SLOT(  hoseDisplay(bool)));
  sgrid->addWidget(hoseDisplayCheck,1,0);

  panelDisplayCheck = new QCheckBox("Panels",gbCSIAnnotationDisplay);
  panelDisplayCheck->setChecked(meta->panelDisplay.value());
  panelDisplayCheck->setEnabled(fixedAnnotations);
  panelDisplayCheck->setToolTip(fixedAnnotations ? "Display Panel annotation" : fixedMessage);
  connect(panelDisplayCheck,SIGNAL(clicked(bool)),
          this,             SLOT(  panelDisplay(bool)));
  sgrid->addWidget(panelDisplayCheck,1,1);

  extendedDisplayCheck = new QCheckBox("Extended",gbCSIAnnotationDisplay);
  extendedDisplayCheck->setChecked(meta->extendedDisplay.value());
  extendedDisplayCheck->setToolTip("Display Title or Freeform annotation");
  connect(extendedDisplayCheck,SIGNAL(clicked(bool)),
          this,                SLOT(  extendedDisplay(bool)));
  sgrid->addWidget(extendedDisplayCheck,1,2);

  gbPlacement = new QGroupBox("Part Annotation Default Placement",parent);
  QGridLayout *gLayout = new QGridLayout();
  gbPlacement->setLayout(gLayout);
  grid->addWidget(gbPlacement);

  placementButton = new QPushButton("Change Placement",gbPlacement);
  placementButton->setToolTip("Set annotation placement relative to CSI part");
  connect(placementButton,SIGNAL(clicked(   bool)),
          this,           SLOT(  placementChanged(bool)));
  gLayout->addWidget(placementButton);

  displayModified          = false;
  axleDisplayModified      = false;
  beamDisplayModified      = false;
  cableDisplayModified     = false;
  connectorDisplayModified = false;
  extendedDisplayModified  = false;
  hoseDisplayModified      = false;
  panelDisplayModified     = false;
  placementModified        = false;
}

void CsiAnnotationGui::axleDisplay(bool checked)
{
  meta->axleDisplay.setValue(checked);
  modified = axleDisplayModified = true;
}

void CsiAnnotationGui::beamDisplay(bool checked)
{
  meta->beamDisplay.setValue(checked);
  modified = beamDisplayModified = true;
}

void CsiAnnotationGui::cableDisplay(bool checked)
{
  meta->cableDisplay.setValue(checked);
  modified = cableDisplayModified = true;
}

void CsiAnnotationGui::connectorDisplay(bool checked)
{
  meta->connectorDisplay.setValue(checked);
  modified = connectorDisplayModified = true;
}

void CsiAnnotationGui::extendedDisplay(bool checked)
{
  meta->extendedDisplay.setValue(checked);
  modified = extendedDisplayModified = true;
}

void CsiAnnotationGui::hoseDisplay(bool checked)
{
  meta->hoseDisplay.setValue(checked);
  modified = hoseDisplayModified = true;
}

void CsiAnnotationGui::panelDisplay(bool checked)
{    
  meta->panelDisplay.setValue(checked);
  modified = panelDisplayModified = true;
}

void CsiAnnotationGui::placementChanged(bool clicked)
{
  Q_UNUSED(clicked);
  PlacementData placementData = meta->placement.value();
  bool ok;
  ok = PlacementDialog
       ::getPlacement(SingleStepType,CsiAnnotationType,placementData,"Annotation Placement",ContentPage);
  if (ok) {
      meta->placement.setValue(placementData);
      modified = placementModified = true;
  }
}


void CsiAnnotationGui::gbToggled(bool checked)
{
    bool saveModified = meta->display.value();
    meta->display.setValue(checked);
    if (saveModified != meta->display.value())
       modified = displayModified = true;
    gbPlacement->setEnabled(checked);
}

void CsiAnnotationGui::apply(QString &topLevelFile)
{
  MetaItem mi;
  mi.beginMacro("CsiAnnotationSettings");
  if (displayModified) {
      mi.setGlobalMeta(topLevelFile,&meta->display);
  }
  if (axleDisplayModified) {
      mi.setGlobalMeta(topLevelFile,&meta->axleDisplay);
  }
  if (beamDisplayModified) {
      mi.setGlobalMeta(topLevelFile,&meta->beamDisplay);
  }
  if (cableDisplayModified) {
      mi.setGlobalMeta(topLevelFile,&meta->cableDisplay);
  }
  if (connectorDisplayModified) {
      mi.setGlobalMeta(topLevelFile,&meta->connectorDisplay);
  }
  if (extendedDisplayModified) {
      mi.setGlobalMeta(topLevelFile,&meta->extendedDisplay);
  }
  if (hoseDisplayModified) {
      mi.setGlobalMeta(topLevelFile,&meta->hoseDisplay);
  }
  if (panelDisplayModified) {
      mi.setGlobalMeta(topLevelFile,&meta->panelDisplay);
  }
  if (placementModified){
      mi.setGlobalMeta(topLevelFile,&meta->placement);
  }
  mi.endMacro();
}

/***********************************************************************
 *
 * Page Orientation
 *
 **********************************************************************/

PageOrientationGui::PageOrientationGui(
  QString const            &heading,
  PageOrientationMeta     *_meta,
  QGroupBox               *parent)
{

  meta = _meta;

  QGridLayout *grid   = new QGridLayout(parent);

  if (parent) {
    parent->setLayout(grid);
  } else {
    setLayout(grid);
  }

  if (heading != "") {
    label = new QLabel(heading);
    grid->addWidget(label,0,0);
  } else {
    label = nullptr;
  }

  /* page orientation */
  portraitRadio = new QRadioButton("Portrait",parent);
  portraitRadio->setChecked(meta->value() == Portrait);
  connect(portraitRadio,SIGNAL(clicked(bool)),
          this,        SLOT(  orientationChange(bool)));
  if (heading == "")
    grid->addWidget(portraitRadio,0,0);
  else
    grid->addWidget(portraitRadio,1,0);

  landscapeRadio    = new QRadioButton("Landscape",parent);
  landscapeRadio->setChecked(meta->value() == Landscape);
  connect(landscapeRadio,SIGNAL(clicked(bool)),
          this,     SLOT(  orientationChange(bool)));
  if (heading == "")
    grid->addWidget(landscapeRadio,0,1);
  else
    grid->addWidget(landscapeRadio,1,1);
}

void PageOrientationGui::orientationChange(bool clicked)
{
  Q_UNUSED(clicked);

  QObject *radioButton = sender();
  if (radioButton == portraitRadio)
      meta->setValue(Portrait);
  else
      meta->setValue(Landscape);

  modified     = true;
}

void PageOrientationGui::apply(QString &topLevelFile)
{
  if (modified) {
    MetaItem mi;
    mi.setGlobalMeta(topLevelFile,meta);
  }

}

/***********************************************************************
 *
 * Rotate Icon Size
 *
 **********************************************************************/

RotateIconSizeGui::RotateIconSizeGui(
  QString const            &heading,
  UnitsMeta               *_meta,
  QGroupBox               *parent)
{
  meta = _meta;

  QGridLayout *grid   = new QGridLayout(parent);

  if (parent) {
    parent->setLayout(grid);
  } else {
    setLayout(grid);
  }

  if (heading != "") {
    label = new QLabel(heading);
    grid->addWidget(label,0,0);
  } else {
    label = nullptr;
  }

  QString string;
  string  = QString("%1") .arg(meta->value(0),
                              meta->_fieldWidth,
                              'f',
                              meta->_precision);
  labelW = new QLabel("Width: ",parent);
  labelW->setToolTip(QString("Current Width in pixels is %1").arg(meta->valuePixels(0)));
  valueW = new QLineEdit(string,parent);
  connect(valueW,SIGNAL(textChanged( QString const &)),
          this,  SLOT(  valueWChange(QString const &)));
  if (heading == "") {
    grid->addWidget(labelW,0,0);
    grid->addWidget(valueW,0,1);
  } else {
    grid->addWidget(valueW,1,0);
    grid->addWidget(valueW,1,1);
  }

  string = QString("%1") .arg(meta->value(1),
                              meta->_fieldWidth,
                              'f',
                              meta->_precision);
  labelH = new QLabel("Height: ",parent);
  valueH = new QLineEdit(string,parent);
  labelH->setToolTip(QString("Current Height in pixels is %1").arg(meta->valuePixels(1)));
  connect(valueH,SIGNAL(textChanged( QString const &)),
          this,  SLOT(  valueHChange(QString const &)));
  if (heading == "") {
    grid->addWidget(labelH,0,2);
    grid->addWidget(valueH,0,3);
  } else {
    grid->addWidget(labelH,1,2);
    grid->addWidget(valueH,1,3);
  }

  setEnabled(true);
}

void RotateIconSizeGui::valueWChange(QString const &string)
{
  w = string.toFloat();
  meta->setValue(0,w);
  labelW->setToolTip(QString("Current Height in pixels is %1").arg(meta->valuePixels(1)));
  modified = true;
  //qDebug() << "Meta setValue(0) Width change:" << meta->value(0);
}

void RotateIconSizeGui::valueHChange(QString const &string)
{
  h = string.toFloat();
  meta->setValue(1,h);
  labelH->setToolTip(QString("Current Height in pixels is %1").arg(meta->valuePixels(1)));
  modified = true;
  //qDebug() << "Meta setValue(1) Height change:" << meta->value(1);
}

void RotateIconSizeGui::updateRotateIconSize(){

  meta->setValue(0,w);
  meta->setValue(1,h);
//  qDebug() << "\nMeta setValue(0) Width update:" << meta->value(0)
//           << "\nMeta setValue(1) Height update:" << meta->value(1);
}

void RotateIconSizeGui::setEnabled(bool enable)
{
  valueW->setEnabled(enable);
  valueH->setEnabled(enable);
}

void RotateIconSizeGui::apply(QString &topLevelFile)
{

  if (modified) {
    updateRotateIconSize();
    MetaItem mi;
    mi.setGlobalMeta(topLevelFile,meta);
  }

}

/***********************************************************************
 *
 * Page Size NOT USED KO (using SizeAndOrientationGui instead)
 *
 **********************************************************************/

PageSizeGui::PageSizeGui(
  QString const            &heading,
  PageSizeMeta            *_meta,
  QGroupBox               *parent)
{

  meta = _meta;

  QGridLayout *grid   = new QGridLayout(parent);

//   logNotice() << " \nPageSizeGui Initialized:" <<
//                 " \nSize 0: " << meta->value(0) <<
//                 " \nSize 1: " << meta->value(1) <<
//                 " \nOrientation: " << ometa->value()
//                 ;

  if (parent) {
    parent->setLayout(grid);
  } else {
    setLayout(grid);
  }

  if (heading != "") {
    label = new QLabel(heading);
    grid->addWidget(label,0,0);
  } else {
    label = nullptr;
  }

  /* page size */
  int numPageTypes = PageSizes::numPageTypes();

  typeCombo = new QComboBox(parent);
  for (int i = 0; i < numPageTypes; i++) {
       typeCombo->addItem(PageSizes::pageTypeSizeID(i));
  }

  float pageWidth = meta->value(0);
  float pageHeight = meta->value(1);
  typeCombo->setCurrentIndex(int(getTypeIndex(pageWidth,pageHeight)));
  connect(typeCombo,SIGNAL(currentIndexChanged(QString const &)),
          this,     SLOT(  typeChange(         QString const &)));
  if (heading == "")
    grid->addWidget(typeCombo,0,0);
  else
    grid->addWidget(typeCombo,1,0);

  QString      string;
  string = QString("%1") .arg(meta->value(0),
                              meta->_fieldWidth,
                              'f',
                              meta->_precision);
  valueW = new QLineEdit(string,parent);
  connect(valueW,SIGNAL(textChanged( QString const &)),
          this,  SLOT(  valueWChange(QString const &)));
  if (heading == "")
    grid->addWidget(valueW,0,1);
  else
    grid->addWidget(valueW,1,1);

  string = QString("%1") .arg(meta->value(1),
                              meta->_fieldWidth,
                              'f',
                              meta->_precision);
  valueH = new QLineEdit(string,parent);
  connect(valueH,SIGNAL(textChanged( QString const &)),
          this,  SLOT(  valueHChange(QString const &)));
  if (heading == "")
    grid->addWidget(valueH,0,2);
  else
    grid->addWidget(valueH,1,2);

  if (typeCombo->currentText() == "Custom")
    setEnabled(true);
  else
    setEnabled(false);

//  logDebug() << "Current Page Type: " << typeCombo->currentText();
}

int PageSizeGui::getTypeIndex(float &widthPg, float &heightPg){

  bool dpi = gui->page.meta.LPub.resolution.type() == DPI;
  int  numPageTypes = PageSizes::numPageTypes();
  int index = -1;
  QString pageWidth;
  QString pageHeight;
  QString typeWidth;
  QString typeHeight;
  for (int i = 0; i < numPageTypes; i++) {

      pageWidth  = QString::number( widthPg,  'f', 1 /*meta->_precision*/ );
      pageHeight = QString::number( heightPg, 'f', 1 /*meta->_precision*/ );
      typeWidth  = QString::number((dpi ? PageSizes::pageWidthIn(i) : PageSizes::pageWidthCm(i)),  'f', 1 /*meta->_precision*/ );
      typeHeight = QString::number((dpi ? PageSizes::pageHeightIn(i) : PageSizes::pageHeightCm(i)), 'f', 1 /*meta->_precision*/ );

//      qDebug() << "\n" << PageSizes::pageTypeSizeID(i) << " @ index: " << i
//               << "\nType: (" << typeWidth << "x" << typeHeight << ") "
//               << "\nPage: (" << pageWidth << "x" << pageHeight << ")";

      if ((pageWidth == typeWidth) && (pageHeight == typeHeight)){
        index = i;
        break;
        }
  }
  if (index == -1)
      index = typeCombo->findText("Custom");

  return index;
}

void PageSizeGui::typeChange(const QString &pageType){

  float pageWidth = meta->value(0);
  float pageHeight = meta->value(1);;
  bool  editLine = true;

//  qDebug() << "\nPage Type: " << pageType ;

  if (pageType != "Custom") {

      bool dpi = gui->page.meta.LPub.resolution.type() == DPI;
      int   numPageTypes = PageSizes::numPageTypes();

      for (int i = 0; i < numPageTypes; i++) {
          if (pageType == PageSizes::pageTypeSizeID(i)) {
              pageWidth  = dpi ? PageSizes::pageWidthIn(i) : PageSizes::pageWidthCm(i);
              pageHeight = dpi ? PageSizes::pageHeightIn(i) : PageSizes::pageHeightCm(i);
              break;
            }
        }
      editLine = false;
    }

  QString      string;
  string = QString("%1") .arg(pageWidth,
                              meta->_fieldWidth,
                              'f',
                              meta->_precision);
  valueW->setText(string);

  string = QString("%1") .arg(pageHeight,
                              meta->_fieldWidth,
                              'f',
                              meta->_precision);
  valueH->setText(string);

  setEnabled(editLine);
}

void PageSizeGui::valueWChange(QString const &string)
{
  w = string.toFloat();
  modified     = true;
//  qDebug() << "Meta setValue(0) Change:" << meta->value(0);
}

void PageSizeGui::valueHChange(QString const &string)
{
  h = string.toFloat();
  modified     = true;
//  qDebug() << "Meta setValue(1) Change:" << meta->value(1);
}

void PageSizeGui::updatePageSize(){

  if (gui->page.meta.LPub.page.orientation.value() == Portrait){
      meta->setValue(0,w);
      meta->setValue(1,h);
//      qDebug() << "\nMeta setValue(0) Portrait Update:" << meta->value(0)
//               << "\nMeta setValue(1) Portrait Update:" << meta->value(1);
    }
  else{
      meta->setValue(0,h);
      meta->setValue(1,w);
//      qDebug() << "\nMeta setValue(0) Landscape Update:" << meta->value(0)
//               << "\nMeta setValue(1) Landscape Update:" << meta->value(1);
    }
}

void PageSizeGui::setEnabled(bool enable)
{
  valueW->setEnabled(enable);
  valueH->setEnabled(enable);
}

void PageSizeGui::apply(QString &topLevelFile)
{
  if (modified) {
    updatePageSize();
    MetaItem mi;
    mi.setGlobalMeta(topLevelFile,meta);
  }

}

/***********************************************************************
 *
 * Page Size And Orientation
 *
 **********************************************************************/

SizeAndOrientationGui::SizeAndOrientationGui(
  QString const            &heading,
  PageSizeMeta            *_smeta,
  PageOrientationMeta     *_ometa,
  QGroupBox               *parent)
{

  smeta = _smeta;
  ometa = _ometa;

  QGridLayout *grid   = new QGridLayout(parent);

  if (parent) {
    parent->setLayout(grid);
  } else {
    setLayout(grid);
  }

  if (heading != "") {
    label = new QLabel(heading);
    grid->addWidget(label,0,0);
  } else {
    label = nullptr;
  }

//  logNotice() << " \nSizeAndOrientationGui Initialized:" <<
//                 " \nSize 0:      " << smeta->value(0) <<
//                 " \nSize 1:      " << smeta->value(1) <<
//                 " \nField Width: " << smeta->_fieldWidth <<
//                 " \nPrecision:   " << smeta->_precision <<
//                 " \nInput Mask:  " << smeta->_inputMask <<
//                 " \nTypeSizeID:  " << smeta->valueSizeID() <<
//                 " \nOrientation: " << ometa->value()
//                 ;

  float typeWidth,typeHeight;
  QString pageTypeSizeID = smeta->valueSizeID();
  int   numPageTypes     = PageSizes::numPageTypes();
  bool  dpi              = gui->page.meta.LPub.resolution.type() == DPI;
  typeCombo              = new QComboBox(parent);
  int typeIndex          = -1;

  for (int i = 0; i < numPageTypes; i++) {

      typeWidth  = dpi ? PageSizes::pageWidthIn(i) : inches2centimeters(PageSizes::pageWidthIn(i));
      typeHeight = dpi ? PageSizes::pageHeightIn(i) : inches2centimeters(PageSizes::pageHeightIn(i));

//      qDebug() << "\n" << pageSizeTypes[i].pageType << " @ index: " << i
//               << "\nType: (" << QString::number(typeWidth, 'f', 3) << "x" <<  QString::number(typeHeight, 'f', 3) << ") "
//               << "\nPage: (" <<  QString::number(pageWidth, 'f', 3) << "x" <<  QString::number(pageHeight, 'f', 3) << ")";

      QString type = QString("%1 (%2 x %3)")
          .arg(PageSizes::pageTypeSizeID(i))
          .arg(QString::number(typeWidth, 'f', 1))
          .arg(QString::number(typeHeight, 'f', 1));

      typeCombo->addItem(type);

      if (pageTypeSizeID != "Custom" && PageSizes::pageTypeSizeID(i) == pageTypeSizeID){
          typeIndex = i;
        }
    }

  if (typeIndex == -1) {

      float pageWidth = smeta->value(0);
      float pageHeight = smeta->value(1);

      QString customType = QString("%1 (%2 x %3)")
          .arg(pageTypeSizeID)
          .arg(QString::number(pageWidth,'f',1))
          .arg(QString::number(pageHeight,'f',1));
      int lastItem = typeCombo->count() - 1;

      typeCombo->removeItem(lastItem);
      typeCombo->addItem(customType);
      typeIndex = lastItem;

    }

  typeCombo->setCurrentIndex(typeIndex);

  connect(typeCombo,SIGNAL(currentIndexChanged(QString const &)),
          this, SLOT(  typeChange(             QString const &)));
  if (heading == "")
    grid->addWidget(typeCombo,0,0);
  else
    grid->addWidget(typeCombo,1,0);

  QString      string;
  string = QString("%1") .arg(smeta->value(0),
                              smeta->_fieldWidth,
                              'f',
                              smeta->_precision);
  valueW = new QLineEdit(string,parent);
  connect(valueW,SIGNAL(textChanged( QString const &)),
          this,  SLOT(  valueWChange(QString const &)));
  if (heading == "")
    grid->addWidget(valueW,0,1);
  else
    grid->addWidget(valueW,1,1);

  string = QString("%1") .arg(smeta->value(1),
                              smeta->_fieldWidth,
                              'f',
                              smeta->_precision);
  valueH = new QLineEdit(string,parent);
  connect(valueH,SIGNAL(textChanged( QString const &)),
          this,  SLOT(  valueHChange(QString const &)));

  if (heading == "")
    grid->addWidget(valueH,0,2);
  else
    grid->addWidget(valueH,1,2);

  if (typeCombo->currentText() == "Custom")
    setEnabled(true);
  else
    setEnabled(false);

  //spacer
  QHBoxLayout *hLayout = new QHBoxLayout(nullptr);
  if (heading == "")
    grid->addLayout(hLayout,1,0);
  else
    grid->addLayout(hLayout,2,0);
  QSpacerItem *hSpacer;
  hSpacer = new QSpacerItem(1,1,QSizePolicy::Fixed,QSizePolicy::Expanding);
  hLayout->addSpacerItem(hSpacer);

  /* page orientation */
  portraitRadio = new QRadioButton("Portrait",parent);
  portraitRadio->setChecked(ometa->value() == Portrait);
  connect(portraitRadio,SIGNAL(clicked(bool)),
          this,         SLOT(  orientationChange(bool)));
  if (heading == "")
    grid->addWidget(portraitRadio,1,1);
  else
    grid->addWidget(portraitRadio,2,1);

  landscapeRadio    = new QRadioButton("Landscape",parent);
  landscapeRadio->setChecked(ometa->value() == Landscape);
  connect(landscapeRadio,SIGNAL(clicked(bool)),
          this,          SLOT(  orientationChange(bool)));
  if (heading == "")
    grid->addWidget(landscapeRadio,1,2);
  else
    grid->addWidget(landscapeRadio,2,2);

  sizeModified        = false;
  orientationModified = false;

//  logDebug() << "Current Page Type: " << typeCombo->currentText();

}

void SizeAndOrientationGui::typeChange(const QString &pageType){

    float pageWidth  = smeta->value(0);
    float pageHeight = smeta->value(1);
    bool  editLine   = true;

  int size = pageType.indexOf(" (");
  QString newType = pageType.left(size);

  smeta->setValueSizeID(newType);

//  logDebug() << "\nPage Type: " << pageType << "type: " << newType ;

  if (newType != "Custom") {
      bool dpi = gui->page.meta.LPub.resolution.type() == DPI;
      int  numPageTypes = PageSizes::numPageTypes();

      for (int i = 0; i < numPageTypes; i++) {

          if (newType == PageSizes::pageTypeSizeID(i)) {
              pageWidth  = dpi ? PageSizes::pageWidthIn(i) : inches2centimeters(PageSizes::pageWidthIn(i));
              pageHeight = dpi ? PageSizes::pageHeightIn(i) : inches2centimeters(PageSizes::pageHeightIn(i));
              break;
            }
        }
      editLine = false;
    }

  QString      string;
  string = QString("%1") .arg(pageWidth,
                              smeta->_fieldWidth,
                              'f',
                              smeta->_precision);
  valueW->setText(string);

  string = QString("%1") .arg(pageHeight,
                              smeta->_fieldWidth,
                              'f',
                              smeta->_precision);
  valueH->setText(string);

  setEnabled(editLine);
}

void SizeAndOrientationGui::orientationChange(bool clicked)
{
  Q_UNUSED(clicked);

  QObject *radioButton = sender();
  if (radioButton == portraitRadio)
    {
      ometa->setValue(Portrait);
    }
  else
    {
      ometa->setValue(Landscape);
    }

  int sizeIdIndx = typeCombo->currentText().indexOf(" (");
  QString newType = typeCombo->currentText().left(sizeIdIndx);

  typeChange(newType);

   modified = orientationModified = true;
//  logDebug() << "Meta Orientation newType:" << newType << "setValue() Value Change:" << ometa->value();
}

void SizeAndOrientationGui::valueWChange(QString const &string)
{
  bool ok;
  w = string.toFloat(&ok);
  if (ok){
      smeta->setValue(0,w);
      modified = sizeModified = true;
//      logDebug() << "Meta Size Width setValue(0) Value Change:" << smeta->value(0);
    }
}

void SizeAndOrientationGui::valueHChange(QString const &string)
{
  bool ok;

  h = string.toFloat(&ok);
  if (ok){
      smeta->setValue(1,h);
      modified = sizeModified = true;
//      logDebug() << "Meta size Height setValue(1) Value Change:" << smeta->value(1);
    }
}

void SizeAndOrientationGui::setEnabled(bool enable)
{
  valueW->setEnabled(enable);
  valueH->setEnabled(enable);
}

void SizeAndOrientationGui::apply(QString &topLevelFile)
{
  MetaItem mi;
  if (sizeModified) {
      mi.setGlobalMeta(topLevelFile,smeta);
  }
  if (orientationModified) {
      mi.setGlobalMeta(topLevelFile,ometa);
  }
}

/***********************************************************************
 *
 * SubModelColor
 *
 **********************************************************************/

SubModelColorGui::SubModelColorGui(
  StringListMeta *_meta,
  QGroupBox  *parent)
{
  meta = _meta;

  QGridLayout *grid;

  grid = new QGridLayout(parent);

  if (parent) {
    parent->setLayout(grid);
  }

  // 01
  subModelColor0Label = new QLabel("Level One",parent);
  grid->addWidget(subModelColor0Label,0,0);

  subModelColor0Example = new QLabel(parent);
  subModelColor0Example->setFrameStyle(QFrame::Sunken|QFrame::Panel);
  subModelColor0Example->setAutoFillBackground(true);
  QColor c = QColor(meta->value(Level1));
  QString styleSheet = QString("QLabel { background-color: rgb(%1, %2, %3); }").
      arg(c.red()).arg(c.green()).arg(c.blue());
  subModelColor0Example->setStyleSheet(styleSheet);
  grid->addWidget(subModelColor0Example,0,1);

  subModelColor0Button = new QPushButton("Change");
  connect(subModelColor0Button,SIGNAL(clicked(      bool)),
          this,       SLOT(  browseSubModelColor0(bool)));

  grid->addWidget(subModelColor0Button,0,2);

  // 02
  subModelColor1Label = new QLabel("Level Two",parent);
  grid->addWidget(subModelColor1Label,1,0);

  subModelColor1Example = new QLabel(parent);
  subModelColor1Example->setFrameStyle(QFrame::Sunken|QFrame::Panel);
  subModelColor1Example->setAutoFillBackground(true);
  c = QColor(meta->value(Level2));
  styleSheet = QString("QLabel { background-color: rgb(%1, %2, %3); }").
      arg(c.red()).arg(c.green()).arg(c.blue());
  subModelColor1Example->setStyleSheet(styleSheet);
  grid->addWidget(subModelColor1Example,1,1);

  subModelColor1Button = new QPushButton("Change");
  connect(subModelColor1Button,SIGNAL(clicked(      bool)),
          this,       SLOT(  browseSubModelColor1(bool)));

  grid->addWidget(subModelColor1Button,1,2);

  // 03
  subModelColor2Label = new QLabel("Level Three",parent);
  grid->addWidget(subModelColor2Label,2,0);

  subModelColor2Example = new QLabel(parent);
  subModelColor2Example->setFrameStyle(QFrame::Sunken|QFrame::Panel);
  subModelColor2Example->setAutoFillBackground(true);
  c = QColor(meta->value(Level3));
  styleSheet = QString("QLabel { background-color: rgb(%1, %2, %3); }").
      arg(c.red()).arg(c.green()).arg(c.blue());
  subModelColor2Example->setStyleSheet(styleSheet);
  grid->addWidget(subModelColor2Example,2,1);

  subModelColor2Button = new QPushButton("Change");
  connect(subModelColor2Button,SIGNAL(clicked(      bool)),
          this,       SLOT(  browseSubModelColor2(bool)));

  grid->addWidget(subModelColor2Button,2,2);

  // 04
  subModelColor3Label = new QLabel("Level Four",parent);
  grid->addWidget(subModelColor3Label,3,0);

  subModelColor3Example = new QLabel(parent);
  subModelColor3Example->setFrameStyle(QFrame::Sunken|QFrame::Panel);
  subModelColor3Example->setAutoFillBackground(true);
  c = QColor(meta->value(Level4));
  styleSheet = QString("QLabel { background-color: rgb(%1, %2, %3); }").
      arg(c.red()).arg(c.green()).arg(c.blue());
  subModelColor3Example->setStyleSheet(styleSheet);
  grid->addWidget(subModelColor3Example,3,1);

  subModelColor3Button = new QPushButton("Change");
  connect(subModelColor3Button,SIGNAL(clicked(      bool)),
          this,       SLOT(  browseSubModelColor3(bool)));

  grid->addWidget(subModelColor3Button,3,2);
}

void SubModelColorGui::browseSubModelColor0(bool clicked)
{
  Q_UNUSED(clicked);
  QColor qcolor = LDrawColor::color(meta->value(Level1));
  QColor newColor = QColorDialog::getColor(qcolor,this);
  if (newColor.isValid() && qcolor != newColor) {
    subModelColor0Example->setAutoFillBackground(true);
    QString styleSheet = QString("QLabel { background-color: rgb(%1, %2, %3); }")
        .arg(newColor.red()).arg(newColor.green()).arg(newColor.blue());
    subModelColor0Example->setStyleSheet(styleSheet);
    meta->setValue(Level1, newColor.name());
    modified = true;
  }
}

void SubModelColorGui::browseSubModelColor1(bool clicked)
{
  Q_UNUSED(clicked);
  QColor qcolor = LDrawColor::color(meta->value(Level2));
  QColor newColor = QColorDialog::getColor(qcolor,this);
  if (newColor.isValid() && qcolor != newColor) {
    subModelColor1Example->setAutoFillBackground(true);
    QString styleSheet = QString("QLabel { background-color: rgb(%1, %2, %3); }")
        .arg(newColor.red()).arg(newColor.green()).arg(newColor.blue());
    subModelColor1Example->setStyleSheet(styleSheet);
    meta->setValue(Level2, newColor.name());
    modified = true;
  }
}

void SubModelColorGui::browseSubModelColor2(bool clicked)
{
  Q_UNUSED(clicked);
  QColor qcolor = LDrawColor::color(meta->value(Level3));
  QColor newColor = QColorDialog::getColor(qcolor,this);
  if (newColor.isValid() && qcolor != newColor) {
    subModelColor2Example->setAutoFillBackground(true);
    QString styleSheet = QString("QLabel { background-color: rgb(%1, %2, %3); }")
        .arg(newColor.red()).arg(newColor.green()).arg(newColor.blue());
    subModelColor2Example->setStyleSheet(styleSheet);
    meta->setValue(Level3, newColor.name());
    modified = true;
  }
}

void SubModelColorGui::browseSubModelColor3(bool clicked)
{
  Q_UNUSED(clicked);
  QColor qcolor = LDrawColor::color(meta->value(Level4));
  QColor newColor = QColorDialog::getColor(qcolor,this);
  if (newColor.isValid() && qcolor != newColor) {
    subModelColor3Example->setAutoFillBackground(true);
    QString styleSheet = QString("QLabel { background-color: rgb(%1, %2, %3); }")
        .arg(newColor.red()).arg(newColor.green()).arg(newColor.blue());
    subModelColor3Example->setStyleSheet(styleSheet);
    meta->setValue(Level4, newColor.name());
    modified = true;
  }
}

void SubModelColorGui::apply(
  QString &topLevelFile)
{
  if (modified) {
    MetaItem mi;
    mi.setGlobalMeta(topLevelFile,meta);
  }
}

/***********************************************************************
 *
 * Universal Roate Step and Target
 *
 **********************************************************************/

void TargetRotateDialogGui::getTargetAndRotateValues(QStringList &keyList){

    QDialog *dialog = new QDialog();

    QFormLayout *form = new QFormLayout(dialog);
    form->addRow(new QLabel("LookAt Target"));
    QGroupBox *targetBbox = new QGroupBox("Enter axis distance");
    form->addWidget(targetBbox);
    QFormLayout *targetSubform = new QFormLayout(targetBbox);

    // Target
    int targetValues[3] = {keyList.at(K_TARGETX).toInt(),
                           keyList.at(K_TARGETY).toInt(),
                           keyList.at(K_TARGETZ).toInt()};

    QList<QLabel *> targetLabelList;
    QList<QSpinBox *> targetSpinBoxList;
    QStringList targetLabels = QStringList()
            << QString("X Distance:")
            << QString("Y Distance:")
            << QString("Z Distance:");

    for(int i = 0; i < targetLabels.size(); ++i) {
        QLabel *label = new QLabel(targetLabels[i], dialog);
        targetLabelList << label;
        QSpinBox * spinBox = new QSpinBox(dialog);
        spinBox->setRange(0,10000);
        spinBox->setSingleStep(1);
        spinBox->setValue(targetValues[i]);
        targetSpinBoxList << spinBox;
        targetSubform->addRow(label,spinBox);
    }

    form->addRow(new QLabel("Rotate"));
    QGroupBox *rotateBox = new QGroupBox("Enter axis rotation");
    form->addWidget(rotateBox);
    QFormLayout *rotateSubform = new QFormLayout(rotateBox);

    // Rotstep
    auto dec = [] (const qreal v)
    {
        auto places = [&v] () {
            if (v == 0.0)
                return 2;

            int count = 0;
            qreal num = v;
            num = abs(num);
            num = num - int(num);
            while (abs(num) >= 0.00001) {
                num = num * 10;
                count = count + 1;
                num = num - int(num);
            }
            return count;
        };

        int a = v - int(v);
        return (a < 1 ? places() : QString::number(a).size() < 3 ? 2 : QString::number(a).size());
    };

    double rotateValues[3] = {keyList.at(K_ROTSX).toDouble(),
                              keyList.at(K_ROTSY).toDouble(),
                              keyList.at(K_ROTSZ).toDouble()};

    QList<QLabel *> rotateLabelList;
    QList<QDoubleSpinBox *> rotateDoubleSpinBoxList;
    QStringList rotateLabels = QStringList()
            << QString("X Rotation:")
            << QString("Y Rotation:")
            << QString("Z Rotation:");

    for(int i = 0; i < rotateLabels.size(); ++i) {
        QLabel *label = new QLabel(rotateLabels[i], dialog);
        rotateLabelList << label;
        QDoubleSpinBox * doubleSpinBox = new QDoubleSpinBox(dialog);
        doubleSpinBox->setRange(0.0,360.0);
        doubleSpinBox->setSingleStep(1.0);
        doubleSpinBox->setDecimals(dec(rotateValues[i]));
        doubleSpinBox->setValue(rotateValues[i]);
        rotateDoubleSpinBoxList << doubleSpinBox;
        rotateSubform->addRow(label,doubleSpinBox);
    }

    QComboBox *typeCombo;
    QLabel *label = new QLabel("Transform:", dialog);
    typeCombo = new QComboBox(dialog);
    typeCombo->addItem("ABS");
    typeCombo->addItem("REL");
    typeCombo->addItem("ADD");
    typeCombo->setCurrentIndex(typeCombo->findText(keyList.at(K_ROTSTYPE)));
    rotateSubform->addRow(label,typeCombo);

    QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
                               Qt::Horizontal, dialog);
    form->addRow(&buttonBox);
    QObject::connect(&buttonBox, SIGNAL(accepted()), dialog, SLOT(accept()));
    QObject::connect(&buttonBox, SIGNAL(rejected()), dialog, SLOT(reject()));
    dialog->setMinimumWidth(250);

    if (dialog->exec() == QDialog::Accepted) {
        keyList.replace(K_TARGETX,QString::number(targetSpinBoxList[0]->value()));
        keyList.replace(K_TARGETY,QString::number(targetSpinBoxList[1]->value()));
        keyList.replace(K_TARGETZ,QString::number(targetSpinBoxList[2]->value()));

        keyList.replace(K_ROTSX,QString::number(rotateDoubleSpinBoxList[0]->value()));
        keyList.replace(K_ROTSY,QString::number(rotateDoubleSpinBoxList[1]->value()));
        keyList.replace(K_ROTSZ,QString::number(rotateDoubleSpinBoxList[2]->value()));
        keyList.replace(K_ROTSTYPE,typeCombo->currentText());
    } else {
        return;
    }
}
