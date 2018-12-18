
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
  connect(value0,SIGNAL(textEdited(  QString const &)),
          this,  SLOT(  value0Change(QString const &)));
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
  QGroupBox     *parent)
{
  meta = _meta;

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
  a = val - (int)val;
  dec = (a <= 0 ? 0 : QString::number(a).size() - 2);                  // shameless hack for the number of input decimals
  numStr = dec > 0 ? QString::number(val): QString::number(val,'f',1); // add 1 decimal place for even numbers
  for (int i = 0; i < numStr.size(); i++) dynMask.append("x");         // dynamically create the input mask

  string = QString("%1") .arg(val,
                              meta->_fieldWidth,
                              'f',
                              meta->_precision);
  value0 = new QLineEdit(string,parent);
  value0->setInputMask(dynMask);
  connect(value0,SIGNAL(textEdited(  QString const &)),
          this,  SLOT(  value0Change(QString const &)));
  layout->addWidget(value0);

  if (heading1 == "") {
    label1 = nullptr;
  } else {
    label1 = new QLabel(heading1,parent);
    layout->addWidget(label1);
  }

  val = meta->value(1);
  dynMask.clear();
  a = val - (int)val;
  dec = (a <= 0 ? 0 : QString::number(a).size() - 2);                  // shameless hack for the number of input decimals
  numStr = dec > 0 ? QString::number(val): QString::number(val,'f',1);
  for (int i = 0; i < numStr.size(); i++) dynMask.append("x");         // dynamically create the input mask
  string = QString("%1") .arg(val,
                              meta->_fieldWidth,
                              'f',
                              meta->_precision);
  value1 = new QLineEdit(string,parent);
  value1->setInputMask(dynMask);
  connect(value1,SIGNAL(textEdited(  QString const &)),
          this,  SLOT(  value1Change(QString const &)));
  layout->addWidget(value1);
}

void FloatsGui::value0Change(QString const &string)
{
  meta->setValue(0,string.toFloat());
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
  value1->setEnabled(enable);
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
 * Float Spin
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
 * Number
 *
 **********************************************************************/

NumberGui::NumberGui(
  NumberMeta *_meta,
  QGroupBox  *parent)
{
  meta = _meta;

  QGridLayout *grid;

  grid = new QGridLayout(parent);

  if (parent) {
    parent->setLayout(grid);
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
  meta->content.setValue(text.join("\\n"));
  modified = editModified = true;
}

void PageAttributeTextGui::editDiscChanged()
{
  QStringList  text = editDisc->toPlainText().split("\n");
  meta->content.setValue(text.join("\\n"));
  modified = editModified = true;
}

void PageAttributeTextGui::editChanged(const QString &value)
{
  QStringList  text = value.split("\n");
  meta->content.setValue(text.join("\\n"));
  modified = editModified = true;
}

void PageAttributeTextGui::placementChanged(bool clicked)
{
  Q_UNUSED(clicked);
  PlacementData placementData = meta->placement.value();
  bool ok;
  ok = PlacementDialog
       ::getPlacement(SingleStepType,meta->type,placementData,pageAttributeName[meta->type]);
  if (ok) {
      meta->placement.setValue(placementData);
  }
  modified = placementModified = true;
}

void PageAttributeTextGui::toggled(bool toggled)
{
  meta->display.setValue(toggled);
  modified = displayModified = true;
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
  gbScale = new QGroupBox("Scale", parent);
  gbScale->setCheckable(true);
  gbScale->setChecked(false);
  hLayout = new QHBoxLayout();
  gbScale->setLayout(hLayout);
  grid->addWidget(gbScale,3,0,1,3);

  scale = new QLabel("Scale " + pageAttributeName[meta->type],parent);
  hLayout->addWidget(scale);

  spin = new QDoubleSpinBox(parent);
  spin->setRange(meta->picScale._min,meta->picScale._max);
  spin->setSingleStep(0.1);
  spin->setDecimals(6);
  spin->setValue(meta->picScale.value());
  connect(spin,SIGNAL(valueChanged(double)),
          this,SLOT  (valueChanged(double)));
  hLayout->addWidget(spin);
  connect(gbScale,SIGNAL(clicked(bool)),this,SLOT(gbScaleClicked(bool)));

  pictureModified   = false;
  marginsModified   = false;
  placementModified = false;
  displayModified   = false;
  scaleModified     = false;
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
    checked = checked;
    qreal value = meta->picScale.value();
    meta->picScale.setValue(value);
    modified = scaleModified = true;
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
  }
  modified = placementModified = true;
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
        CameraDistFactorMeta *_meta,
        QGroupBox  *parent)
{

  meta = _meta;

  QHBoxLayout *hLayout = new QHBoxLayout();

  setLayout(hLayout);

  bool nativeRenderer = (Render::getRenderer() == RENDERER_NATIVE);
  QString tipMessage = QString("Native renderer camera distance factor - enabled when Renderer is set to Native.");
  if (nativeRenderer)
      tipMessage = QString("Native renderer camera distance factor, adjust by 10, to scale renderings.");
  parent->setToolTip(tipMessage);
  parent->setEnabled(nativeRenderer);

  cameraDistFactorLabel = new QLabel(heading, parent);
  hLayout->addWidget(cameraDistFactorLabel);

  cameraDistFactorNative = meta->factor.value();
  cameraDistFactorSpin = new QSpinBox(parent);
  cameraDistFactorSpin->setRange(100,5000);
  cameraDistFactorSpin->setSingleStep(10);
  cameraDistFactorSpin->setValue(cameraDistFactorNative);
  connect(cameraDistFactorSpin,SIGNAL(valueChanged(int)),
          this,                SLOT(cameraDistFactorChange(int)));
  hLayout->addWidget(cameraDistFactorSpin);

}

void CameraDistFactorGui::cameraDistFactorChange(int factor)
{
  meta->factor.setValue(factor);
  changeMessage = QString("Native camera distance factor changed from %1 to %2")
                          .arg(cameraDistFactorNative)
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

  float val = rotStep.rots[0];
  int a = val - (int)val;
  int dec = (a <= 0 ? 2 : QString::number(a).size() < 3 ? 2 : QString::number(a).size());
  rotStepSpinX = new QDoubleSpinBox(parent);
  rotStepSpinX->setRange(0.0f,360.0);
  rotStepSpinX->setSingleStep(1.0);
  rotStepSpinX->setDecimals(dec);
  rotStepSpinX->setValue(rotStep.rots[0]);
  connect(rotStepSpinX,SIGNAL(valueChanged(double)),
          this,        SLOT(rotStepXChange(double)));
  grid->addWidget(rotStepSpinX,0,1);

  val = rotStep.rots[1];
  a = val - (int)val;
  dec = (a <= 0 ? 2 : QString::number(a).size() < 3 ? 2 : QString::number(a).size());
  rotStepSpinY = new QDoubleSpinBox(parent);
  rotStepSpinY->setRange(0.0f,360.0);
  rotStepSpinY->setSingleStep(1.0);
  rotStepSpinY->setDecimals(dec);
  rotStepSpinY->setValue(rotStep.rots[1]);
  connect(rotStepSpinY,SIGNAL(valueChanged(double)),
          this,        SLOT(rotStepYChange(double)));
  grid->addWidget(rotStepSpinY,0,2);

  val = rotStep.rots[2];
  a = val - (int)val;
  dec = (a <= 0 ? 2 : QString::number(a).size() < 3 ? 2 : QString::number(a).size());
  rotStepSpinZ = new QDoubleSpinBox(parent);
  rotStepSpinZ->setRange(0.0f,360.0);
  rotStepSpinZ->setSingleStep(1.0);
  rotStepSpinZ->setDecimals(dec);
  rotStepSpinZ->setValue(rotStep.rots[2]);
  connect(rotStepSpinZ,SIGNAL(valueChanged(double)),
          this,        SLOT(rotStepZChange(double)));
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
          this,     SLOT(  typeChange(         QString const &)));
  grid->addWidget(typeCombo,1,1);

}

void RotStepGui::rotStepXChange(double value)
{
  RotStepData data = meta->value();
  data.rots[0] = value;
  meta->setValue(data);
  modified = true;
}

void RotStepGui::rotStepYChange(double value)
{
  RotStepData data = meta->value();
  data.rots[1] = value;
  meta->setValue(data);
  modified = true;
}

void RotStepGui::rotStepZChange(double value)
{
  RotStepData data = meta->value();
  data.rots[2] = value;
  meta->setValue(data);
  modified = true;
}

void RotStepGui::typeChange(QString const &value)
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
 * Background
 *
 **********************************************************************/

BackgroundGui::BackgroundGui(
  BackgroundMeta *_meta,
  QGroupBox      *parent)
{
  QComboBox     *combo;
  QGridLayout   *grid;
  QHBoxLayout   *layout;

  meta = _meta;

  BackgroundData background = meta->value();

  if (background.type == BackgroundData::BgImage)
      picture = background.string;
  else
      color = background.string;

  grid = new QGridLayout(parent);
  parent->setLayout(grid);

  combo = new QComboBox(parent);
  combo->addItem("None (transparent)");
  combo->addItem("Solid Color");
  combo->addItem("Gradient");
  combo->addItem("Picture");
  combo->addItem("Submodel Level Color");
  combo->setCurrentIndex(int(background.type));
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
          this,     SLOT(  stretch(bool)));
  layout->addWidget(tileRadio);

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
      {
      colorButton->show();
      colorButton->setEnabled(true);
      gradientButton->hide();
      pictureButton->setEnabled(false);
      pictureEdit->setEnabled(false);
      fill->setEnabled(false);
      }
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

BorderGui::BorderGui(BorderMeta *_meta,
  QGroupBox *parent,
  bool rotateArrow)
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

  if (rotateArrow) {
      typeCombo->hide();
      spinLabel->hide();
      spin->hide();
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
  if (color != newColor) {
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
  if (color != newColor) {
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
 * Separator
 *
 **********************************************************************/

SepGui::SepGui(
  SepMeta   *_meta,
  QGroupBox *parent)
{
  meta = _meta;

  QGridLayout *grid = new QGridLayout(parent);
  parent->setLayout(grid);

  QLabel    *label;
  QLineEdit *lineEdit;
  QPushButton *button;

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

  label = new QLabel("Color",parent);
  grid->addWidget(label,1,0);

  colorExample = new QLabel(parent);
  colorExample->setFrameStyle(QFrame::Sunken|QFrame::Panel);
  colorExample->setAutoFillBackground(true);
  QColor c = QColor(sep.color);
  QString styleSheet =
      QString("QLabel { background-color: rgb(%1, %2, %3); }").
      arg(c.red()).arg(c.green()).arg(c.blue());
  colorExample->setStyleSheet(styleSheet);
  grid->addWidget(colorExample,1,1);

  button = new QPushButton("Change",parent);
  connect(button,SIGNAL(clicked(bool)),
          this,  SLOT(  browseColor(bool)));
  grid->addWidget(button,1,2);

  label = new QLabel("Margins",parent);
  grid->addWidget(label,2,0);

  string = QString("%1") .arg(sep.margin[0],5,'f',4);
  lineEdit = new QLineEdit(string,parent);
  grid->addWidget(lineEdit,2,1);
  connect(lineEdit,SIGNAL(textEdited(QString const &)),
          this,    SLOT(marginXChange(QString const &)));

  string = QString("%1") .arg(sep.margin[1],5,'f',4);
  lineEdit = new QLineEdit(string,parent);
  grid->addWidget(lineEdit,2,2);
  connect(lineEdit,SIGNAL(textEdited(QString const &)),
          this,    SLOT(marginYChange(QString const &)));
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
  if (units == "Dots Per Inch") {
    //type = DPI;
  } else {
    type = DPCM;
  }

  float tvalue;

  if (type == meta->type()) {
    tvalue = value;
  } else if (type == DPI) {
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
    if (value != meta->value()) {
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
  CameraDistFactorMeta *_meta,
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
  cameraDistFactorGrpBox = new QGroupBox("Native Renderer Camera Distance",parent);

  bool nativeRenderer = (Render::getRenderer() == RENDERER_NATIVE);
  QString tipMessage = QString("Native renderer camera distance factor - enabled when Renderer is set to Native.");
  if (nativeRenderer)
      tipMessage = QString("Native renderer camera distance factor, adjust by 10, to scale renderings.");
  cameraDistFactorGrpBox->setToolTip(tipMessage);
  cameraDistFactorGrpBox->setEnabled(nativeRenderer);
  cameraDistFactorGrpBox->setLayout(grpGrid);
  grid->addWidget(cameraDistFactorGrpBox,3,0,1,2);

  cameraDistFactorLabel = new QLabel("Factor", cameraDistFactorGrpBox);
  grpGrid->addWidget(cameraDistFactorLabel,0,0);

  cameraDistFactorNative = meta->factor.value();
  cameraDistFactorSpin = new QSpinBox(parent);
  cameraDistFactorSpin->setRange(100,5000);
  cameraDistFactorSpin->setSingleStep(10);
  cameraDistFactorSpin->setValue(cameraDistFactorNative);
  connect(cameraDistFactorSpin,SIGNAL(valueChanged(int)),
          this,                SLOT(cameraDistFactorChange(int)));
  grpGrid->addWidget(cameraDistFactorSpin,0,1);

  QSettings Settings;
  cameraDistFactorDefaulSettings = Settings.contains(QString("%1/%2").arg(SETTINGS,"cameraDistFactorNative"));
  cameraDistFactorDefaultBox = new QCheckBox("Set as default",cameraDistFactorGrpBox);
  cameraDistFactorDefaultBox->setEnabled(nativeRenderer);
  cameraDistFactorDefaultBox->setToolTip("Save to application settings.");
  cameraDistFactorDefaultBox->setChecked(cameraDistFactorDefaulSettings);
  grpGrid->addWidget(cameraDistFactorDefaultBox,1,0);

  cameraDistFactorMetaBox = new QCheckBox("Add meta command",cameraDistFactorGrpBox);
  cameraDistFactorMetaBox->setEnabled(nativeRenderer);
  cameraDistFactorMetaBox->setToolTip("Add a global meta command to the LDraw file.");
  cameraDistFactorMetaBox->setChecked(!cameraDistFactorDefaulSettings);
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
  cameraDistFactorGrpBox->setEnabled(pick == RENDERER_NATIVE);
  if (pick != RENDERER_LDVIEW) {
      ldvSingleCallBox->setChecked(false);
    }
  QString tipMessage = QString("Native renderer camera distance factor - enabled when Renderer is set to Native.");
  if (pick == RENDERER_NATIVE) {
      cameraDistFactorDefaultBox->setEnabled(true);
      cameraDistFactorMetaBox->setEnabled(true);
      tipMessage = QString("Native renderer camera distance factor, adjust by 10, to scale renderings.");
    }
  cameraDistFactorGrpBox->setToolTip(tipMessage);
  if (pick != Preferences::preferredRenderer) {
      modified = rendererModified = true;
    }
}

void RendererGui::cameraDistFactorChange(int factor)
{
  meta->factor.setValue(factor);
  clearCaches = cameraDistFactorModified = true;
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
      changeMessage = QString("Native camera distance factor changed from %1 to %2")
                      .arg(cameraDistFactorNative)
                      .arg(meta->factor.value());
      emit gui->messageSig(LOG_INFO, changeMessage);
      Preferences::cameraDistFactorNative = meta->factor.value();
      if (cameraDistFactorDefaultBox->isChecked()){
          changeMessage = QString("Factor added as application default.");
          emit gui->messageSig(LOG_INFO, changeMessage);
          Settings.setValue(QString("%1/%2").arg(SETTINGS,"CameraDistFactorNative"),meta->factor.value());
        } else
        if (cameraDistFactorDefaulSettings) {
            Settings.remove(QString("%1/%2").arg(SETTINGS,"CameraDistFactorNative"));
         }
      if (cameraDistFactorMetaBox->isChecked()){
          MetaItem mi;
          mi.setGlobalMeta(topLevelFile,&meta->factor);
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
    QHBoxLayout *hLayout = new QHBoxLayout();

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

    showSubmodelInCalloutBox = new QCheckBox("Show submodel in callout",parent);
    showSubmodelInCalloutBox->setToolTip("Show Submodel image in callout");
    showSubmodelInCalloutBox->setChecked(meta->showSubmodelInCallout.value());
    connect(showSubmodelInCalloutBox,SIGNAL(clicked(bool)),
            this,            SLOT(showSubmodelInCalloutChange(bool)));
    grid->addWidget(showSubmodelInCalloutBox,6,0,1,2);

    showSubmodelInCalloutDefaultSettings = Settings.contains(QString("%1/%2").arg(SETTINGS,"ShowSubmodelInCallout"));
    showSubmodelInCalloutDefaultBox = new QCheckBox("Set as default",parent);
    showSubmodelInCalloutDefaultBox->setToolTip("Save show submodel in callout to application settings.");
    showSubmodelInCalloutDefaultBox->setChecked(showSubmodelInCalloutDefaultSettings);
    grid->addWidget(showSubmodelInCalloutDefaultBox,7,0);

    showSubmodelInCalloutMetaBox = new QCheckBox("Add meta command",parent);
    showSubmodelInCalloutMetaBox->setToolTip("Add show submodel in callout as a global meta command to the LDraw file.");
    showSubmodelInCalloutMetaBox->setChecked(!showSubmodelInCalloutDefaultSettings);
    grid->addWidget(showSubmodelInCalloutMetaBox,7,1);

    showSubmodelsModified         = false;
    showTopModelModified          = false;
    showSubmodelInCalloutModified = false;
}

void ShowSubModelGui::showSubmodelsChange(bool checked)
{
    if (meta->show.value() != checked) {
        meta->show.setValue(checked);
        modified = showSubmodelsModified = true;
    }
}

void ShowSubModelGui::showTopModelChange(bool checked)
{
    if (meta->showTopModel.value() != checked) {
        meta->showTopModel.setValue(checked);
        modified = showTopModelModified = true;
    }
}

void ShowSubModelGui::showSubmodelInCalloutChange(bool checked)
{
    if (meta->showSubmodelInCallout.value() != checked) {
        meta->showSubmodelInCallout.setValue(checked);
        modified = showSubmodelInCalloutModified = true;
    }
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
    if (showSubmodelInCalloutModified) {
        changeMessage = QString("Show submodel in callout is %1")
                                .arg(meta->showSubmodelInCallout.value() ? "ON" : "OFF");
        emit gui->messageSig(LOG_INFO, changeMessage);
        if (showSubmodelInCalloutDefaultBox->isChecked()){
            changeMessage = QString("Show submodel in callout added as application default.");
            emit gui->messageSig(LOG_INFO, changeMessage);
            Preferences::showSubmodelInCallout = meta->showSubmodelInCallout.value();
            Settings.setValue(QString("%1/%2").arg(SETTINGS,"ShowSubmodelInCallout"),meta->showSubmodelInCallout.value());
        }
        else
        if (showSubmodelInCalloutDefaultSettings) {
            Settings.remove(QString("%1/%2").arg(SETTINGS,"ShowSubmodelInCallout"));
        }

        if (showSubmodelInCalloutMetaBox->isChecked()){
            MetaItem mi;
            mi.beginMacro("ShowSubmodelInCallout");
            mi.setGlobalMeta(topLevelFile,&meta->showSubmodelInCallout);
            mi.endMacro();
        }
    }
}

/***********************************************************************
 *
 * PliSort
 *
 **********************************************************************/

PliSortGui::PliSortGui(
  const QString   &heading,
  PliSortMeta     *_meta,
  QGroupBox       *parent)
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
  sortOption == SortOptionName[PartSize]   ? currentIndex = PartSize :
  sortOption == SortOptionName[PartColour] ? currentIndex = PartColour :
                                             currentIndex = PartCategory;

  combo = new QComboBox(parent);
  combo->addItem(SortOptionName[PartSize]);
  combo->addItem(SortOptionName[PartColour]);
  combo->addItem(SortOptionName[PartCategory]);
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
 * PliAnnotation
 *
 **********************************************************************/

PliAnnotationGui::PliAnnotationGui(
    const QString     &heading,
    PliAnnotationMeta *_meta,
    QGroupBox         *parent)
{
  meta = _meta;

  QGridLayout *grid = new QGridLayout(parent);
  QHBoxLayout *hLayout;

  if (parent) {
      parent->setLayout(grid);
    }

  if (heading != "") {
      headingLabel = new QLabel(heading,parent);
      grid->addWidget(headingLabel);
    } else {
      headingLabel = nullptr;
    }

  //PLIAnnotation
  gbPLIAnnotation = new QGroupBox("Display PLI Annotation",parent);
  gbPLIAnnotation->setCheckable(true);
  gbPLIAnnotation->setChecked(meta->display.value());
  hLayout = new QHBoxLayout();
  gbPLIAnnotation->setLayout(hLayout);
  grid->addWidget(gbPLIAnnotation,0,0);
  connect(gbPLIAnnotation,SIGNAL(toggled(bool)),
          this, SLOT(  gbToggled(bool)));

  titleAnnotationButton = new QRadioButton("Title",gbPLIAnnotation);
  connect(titleAnnotationButton,SIGNAL(clicked(bool)),
          this,        SLOT(  titleAnnotation(bool)));
  hLayout->addWidget(titleAnnotationButton);

  freeformAnnotationButton = new QRadioButton("Free Form",gbPLIAnnotation);
  connect(freeformAnnotationButton,SIGNAL(clicked(bool)),
          this,     SLOT(  freeformAnnotation(bool)));
  hLayout->addWidget(freeformAnnotationButton);

  titleAndFreeformAnnotationButton = new QRadioButton("Both",gbPLIAnnotation);
  connect(titleAndFreeformAnnotationButton,SIGNAL(clicked(bool)),
          this,     SLOT(  titleAndFreeformAnnotation(bool)));
  hLayout->addWidget(titleAndFreeformAnnotationButton);

  titleAnnotationButton->setChecked(meta->titleAnnotation.value());
  freeformAnnotationButton->setChecked(meta->freeformAnnotation.value());
  titleAndFreeformAnnotationButton->setChecked(meta->titleAndFreeformAnnotation.value());

  titleModified            = false;
  freeformModified         = false;
  titleAndFreeformModified = false;
  displayModified          = false;
}

void PliAnnotationGui::titleAnnotation(bool checked)
{
  meta->titleAnnotation.setValue(checked);
  meta->freeformAnnotation.setValue(! checked);
  meta->titleAndFreeformAnnotation.setValue(! checked);
  modified = titleModified = true;
}

void PliAnnotationGui::freeformAnnotation(bool checked)
{
  meta->titleAnnotation.setValue(! checked);
  meta->freeformAnnotation.setValue( checked);
  meta->titleAndFreeformAnnotation.setValue(! checked);
  modified = freeformModified = true;
}

void PliAnnotationGui::titleAndFreeformAnnotation(bool checked)
{
  meta->titleAnnotation.setValue(! checked);
  meta->freeformAnnotation.setValue(! checked);
  meta->titleAndFreeformAnnotation.setValue( checked);
  modified = titleAndFreeformModified = true;
}

void PliAnnotationGui::gbToggled(bool toggled)
{
  meta->display.setValue(toggled);
  if(toggled){
      titleAnnotationButton->setChecked(meta->titleAnnotation.value());
      freeformAnnotationButton->setChecked(meta->freeformAnnotation.value());
      titleAndFreeformAnnotationButton->setChecked(meta->titleAndFreeformAnnotation.value());
    }
  modified = displayModified = true;
}

void PliAnnotationGui::apply(QString &topLevelFile)
{
  MetaItem mi;
  mi.beginMacro("PliAnnotationSettings");
  if (displayModified) {
      mi.setGlobalMeta(topLevelFile,&meta->display);
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
