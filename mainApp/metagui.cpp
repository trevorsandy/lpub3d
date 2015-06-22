
/****************************************************************************
**
** Copyright (C) 2007-2009 Kevin Clague. All rights reserved.
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

#include <QWidget>
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

#include "meta.h"
#include "metatypes.h"
#include "metaitem.h"
#include "color.h"

#include "lpub.h"
#include "lpub_preferences.h"
#include "render.h"

const QString placementEncNames[NumPlacements] =
{
    "Top Left",
    "Top",
    "Top Right",
    "Right",
    "Bottom Right",
    "Bottom",
    "Bottom Left",
    "Left",
    "Center",
};

const QString placementTypeNames[23] =
{
    "Page",                "Assem",      "Step Group",  "Step Number",
    "Parts List",          "Callout",    "Page Number",
    "Title",               "Model ID",   "Author",      "URL",         "Model Description",
    "Publish Description", "Copyright",  "Email",       "Disclaimer",
    "Pieces",              "Plug",       "Category",    "Logo",
    "Cover Image",         "Plug Image", "BOM",
};

const int placementTypeVars[23] =
{
  PageType,            CsiType,           StepGroupType,    StepNumberType,
  PartsListType,       CalloutType,       PageNumberType,
  PageTitleType,       PageModelNameType, PageAuthorType,   PageURLType,         PageModelDescType,
  PagePublishDescType, PageCopyrightType, PageEmailType,    PageDisclaimerType,
  PagePiecesType,      PagePlugType,      PageCategoryType, PageDocumentLogoType,
  PageCoverImageType,  PagePlugImageType, BomType,
};


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
  clicked = clicked;
  meta->setValue(true);
  modified = true;
}

void BoolRadioGui::falseClicked(bool clicked)
{
  clicked = clicked;
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
    label = NULL;
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
    label0 = NULL;
  } else {
    label0 = new QLabel(heading0,parent);
    layout->addWidget(label0);
  }

  QString      string;

  string = QString("%1") .arg(meta->value(0),
                              meta->_fieldWidth,
                              'f',
                              meta->_precision);
  value0 = new QLineEdit(string,parent);
  value0->setInputMask(meta->_inputMask);
  connect(value0,SIGNAL(textEdited(  QString const &)),
          this,  SLOT(  value0Change(QString const &)));
  layout->addWidget(value0);

  if (heading1 == "") {
    label1 = NULL;
  } else {
    label1 = new QLabel(heading1,parent);
    layout->addWidget(label1);
  }

  string = QString("%1") .arg(meta->value(1),
                              meta->_fieldWidth,
                              'f',
                              meta->_precision);
  value1 = new QLineEdit(string,parent);
  value1->setInputMask(meta->_inputMask);
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
    label = NULL;
  } else {
    label = new QLabel(heading,parent);
    layout->addWidget(label);
  }

  spin = new QDoubleSpinBox(parent);
  layout->addWidget(spin);
  spin->setRange(min,max);
  spin->setSingleStep(step);
  spin->setDecimals(6);
  spin->setValue(meta->value());
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
    headingLabel = NULL;
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
  colorExample->setPalette(QPalette(meta->color.value()));
  colorExample->setAutoFillBackground(true);
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

  fontModified = false;
  colorModified = false;
  marginsModified = false;
}

void NumberGui::browseFont(bool clicked)
{
  clicked = clicked;
  QFont font;
  QString fontName = meta->font.valueFoo();
  font.fromString(fontName);
  bool ok;
  font = QFontDialog::getFont(&ok,font);
  fontName = font.toString();

  if (ok) {
    meta->font.setValue(font.toString());
    fontExample->setFont(font);
    fontModified = true;
  }
}

void NumberGui::browseColor(bool clicked)
{
  clicked = clicked;
  QColor qcolor = LDrawColor::color(meta->color.value());
  QColor newColor = QColorDialog::getColor(qcolor,this);
  if (qcolor != newColor) {
    colorExample->setPalette(QPalette(newColor));
    colorExample->setAutoFillBackground(true);
    meta->color.setValue(newColor.name());
    colorModified = true;
  }
}

void NumberGui::value0Changed(QString const &string)
{
  meta->margin.setValue(0,string.toFloat());
  marginsModified = true;
}

void NumberGui::value1Changed(QString const &string)
{
  meta->margin.setValue(1, string.toFloat());
  marginsModified = true;
}

void NumberGui::apply(
  QString &topLevelFile)
{
  MetaItem mi;
  mi.beginMacro("Settings");
  
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

  grid = new QGridLayout(parent);

  if (parent) {
    parent->setLayout(grid);
  }

  // Display
  parent->setCheckable(true);
  parent->setChecked(meta->display.value());
  connect(parent,SIGNAL(toggled(bool)),
          this, SLOT(  toggled(bool)));

  //Positioning
  gbPosition = new QGroupBox("Position",parent);
  gLayout = new QGridLayout();
  gbPosition->setLayout(gLayout);
  grid->addWidget(gbPosition,0,0,1,3);

  //placement group
  //lable
  placement = new QLabel(parent);
  placement->setText("Placement");
  gLayout->addWidget(placement,0,0);
  //combo
  placementCombo = new QComboBox(parent);
  int placementComboCurrentIndex = 0;
  for(int i = 0; i < NumPlacements; i++){
     placementCombo->addItem(placementEncNames[i]);
     if (meta->placement.value().placement == i)
         placementComboCurrentIndex = placementCombo->count()-1;
  }
  placementCombo->setCurrentIndex(placementComboCurrentIndex);
  connect(placementCombo,SIGNAL(currentIndexChanged(int)),
          this, SLOT(  typePositionChanged(         int)));
  gLayout->addWidget(placementCombo, 0,1);

  //justify group
  //lable
  justify = new QLabel(parent);
  justify->setText("Justification");
  gLayout->addWidget(justify,0,2);
  //combo
  justifyCombo = new QComboBox(parent);
  int justifyComboCurrentIndex = 0;
  for(int i = 0; i < NumPlacements; i++){
     justifyCombo->addItem(placementEncNames[i]);
     if (meta->placement.value().justification == i)
         justifyComboCurrentIndex = justifyCombo->count()-1;
  }
  justifyCombo->setCurrentIndex(justifyComboCurrentIndex);
  connect(justifyCombo,SIGNAL(currentIndexChanged(int)),
          this, SLOT(  typePositionChanged(       int)));
  gLayout->addWidget(justifyCombo, 0,3);

  //relativeTo group
  //lable
  relativeTo = new QLabel(parent);
  relativeTo->setText("Relative To");
  gLayout->addWidget(relativeTo,1,0);
  //combo
  relativeToCombo = new QComboBox(parent);
  int placementTypeNamesCount = 23;
  int relativeToComboCurrentIndex = 0;
  for(int i = 0; i < NumRelatives; i++){
      for(int j = 0; j < placementTypeNamesCount ; j++){
          if (placementTypeVars[j] == i){
            relativeToCombo->addItem(placementTypeNames[j]);
          }
      }
      if (meta->placement.value().relativeTo == i){
          relativeToComboCurrentIndex = relativeToCombo->count()-1;
      }
  }
  relativeToCombo->setCurrentIndex(relativeToComboCurrentIndex);
  connect(relativeToCombo,SIGNAL(currentIndexChanged(int)),
          this, SLOT(  typePositionChanged(          int)));
  gLayout->addWidget(relativeToCombo, 1,1);

  //preposition group
  //lable
  preposition = new QLabel(parent);
  preposition->setText("Preposition");
  gLayout->addWidget(preposition,1,2);
  //combo
  prepositionCombo = new QComboBox(parent);
  prepositionCombo->addItem("Inside");
  prepositionCombo->addItem("Outside");
  prepositionCombo->setCurrentIndex(int(meta->placement.value().preposition));

  connect(prepositionCombo,SIGNAL(currentIndexChanged(int)),
          this, SLOT(  typePositionChanged(           int)));
  gLayout->addWidget(prepositionCombo, 1,3);

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

  // colour
  colorLabel = new QLabel("Color",parent);
  grid->addWidget(colorLabel,2,0);

  colorExample = new QLabel(parent);
  colorExample->setFrameStyle(QFrame::Sunken|QFrame::Panel);
  colorExample->setPalette(QPalette(meta->textColor.value()));
  colorExample->setAutoFillBackground(true);
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

  //Content Dialog
  gbContentEdit = new QGroupBox("Content",parent);
  hLayout = new QHBoxLayout();
  gbContentEdit->setLayout(hLayout);
  grid->addWidget(gbContentEdit,4,0,1,3);

  contentLabel = new QLabel("Content",parent);
  hLayout->addWidget(contentLabel);

  content = meta->content.value();
  contentEdit = new QLineEdit(content,parent);

  connect(contentEdit,SIGNAL(textChanged(QString const &)),
        this,  SLOT(  editChanged(QString const &)));
  hLayout->addWidget(contentEdit);

  //Description Dialog
  gbDescDialog = new QGroupBox("Description Dialog Content",parent);
  gbDescDialog->hide();
  hLayout = new QHBoxLayout();
  gbDescDialog->setLayout(hLayout);
  grid->addWidget(gbDescDialog,4,0,1,3);

  edit = new QTextEdit(parent);

  connect(edit,SIGNAL(textChanged()),
          this,  SLOT(  editChanged()));
  hLayout->addWidget(edit);

  if (meta->type == PageModelDescType) {
      gbDescDialog->show();
      gbContentEdit->hide();
      string = QString("%1").arg(meta->content.value());
      edit->setText(string);
  }

  //Disclaimer Dialog
  gbDiscDialog = new QGroupBox("Disclaimer Dialog Content",parent);
  gbDiscDialog->hide();
  hLayout = new QHBoxLayout();
  gbDiscDialog->setLayout(hLayout);
  grid->addWidget(gbDiscDialog,4,0,1,3);

  edit = new QTextEdit(parent);

  hLayout->addWidget(edit);

  connect(edit,SIGNAL(textChanged()),
          this,  SLOT(  editChanged()));

  if (meta->type == PageDisclaimerType) {
      gbDiscDialog->show();
      gbContentEdit->hide();
      string = QString("%1").arg(meta->content.value());
      edit->setText(string);
  }

  fontModified      = false;
  colorModified     = false;
  marginsModified   = false;
  positionModified  = false;
  displayModified   = false;
  editModified      = false;
}

void PageAttributeTextGui::browseFont(bool clicked)
{
  clicked = clicked;
  QFont font;
  QString fontName = meta->textFont.valueFoo();
  font.fromString(fontName);
  bool ok;
  font = QFontDialog::getFont(&ok,font);
  fontName = font.toString();

  if (ok) {
    meta->textFont.setValue(font.toString());
    fontExample->setFont(font);
    fontModified = true;
  }
}

void PageAttributeTextGui::browseColor(bool clicked)
{
  clicked = clicked;
  QColor qcolor = LDrawColor::color(meta->textColor.value());
  QColor newColor = QColorDialog::getColor(qcolor,this);
  if (qcolor != newColor) {
    colorExample->setPalette(QPalette(newColor));
    colorExample->setAutoFillBackground(true);
    meta->textColor.setValue(newColor.name());
    colorModified = true;
  }
}

void PageAttributeTextGui::value0Changed(QString const &string)
{
  meta->margin.setValue(0,string.toFloat());
  marginsModified = true;
}

void PageAttributeTextGui::value1Changed(QString const &string)
{
  meta->margin.setValue(1,string.toFloat());
  marginsModified = true;
}

void PageAttributeTextGui::editChanged()
{
  meta->content.setValue(edit->toPlainText());
  editModified = true;
}

void PageAttributeTextGui::editChanged(const QString &value)
{
  meta->content.setValue(value);
  editModified = true;
}

void PageAttributeTextGui::typePositionChanged(int type)
{
  PlacementData textData = meta->placement.value();
  QObject *obj = sender();
  if(obj == placementCombo)
     textData.placement = PlacementEnc(type);
  if(obj == justifyCombo)
     textData.justification = PlacementEnc(type);
  if(obj == relativeToCombo)
     textData.relativeTo = PlacementType(type);
  if(obj == prepositionCombo)
     textData.preposition = PrepositionEnc(type);
  meta->placement.setValue(textData);
  positionModified = true;
}

void PageAttributeTextGui::toggled(bool toggled)
{
    meta->display.setValue(toggled);
    displayModified = true;
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
  if (positionModified){
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

  meta  = _meta;

  PageAttributePictureData Picture = meta->value();
  PlacementData        pictureData = meta->value().placement;

  grid = new QGridLayout(parent);

  if (parent) {
      parent->setLayout(grid);
  }

  // Display
  parent->setCheckable(true);
  parent->setChecked(Picture.display);
  connect(parent,SIGNAL(toggled(bool)),
          this, SLOT(  toggled(bool)));

  //Positioning
  gbPosition = new QGroupBox("Position",parent);
  gLayout = new QGridLayout();
  gbPosition->setLayout(gLayout);
  grid->addWidget(gbPosition,0,0,1,3);

  //placement group
  //lable
  placement = new QLabel(parent);
  placement->setText("Placement");
  gLayout->addWidget(placement,0,0);
  //combo
  placementCombo = new QComboBox(parent);
  int placementComboCurrentIndex = 0;
  for(int i = 0; i < NumPlacements; i++){
     placementCombo->addItem(placementEncNames[i]);
     if (pictureData.placement == i)
         placementComboCurrentIndex = placementCombo->count()-1;
  }
  placementCombo->setCurrentIndex(placementComboCurrentIndex);
  connect(placementCombo,SIGNAL(currentIndexChanged(int)),
          this, SLOT(  typePositionChanged(         int)));
  gLayout->addWidget(placementCombo, 0,1);

  //justify group
  //lable
  justify = new QLabel(parent);
  justify->setText("Justification");
  gLayout->addWidget(justify,0,2);
  //combo
  justifyCombo = new QComboBox(parent);
  int justifyComboCurrentIndex = 0;
  for(int i = 0; i < NumPlacements; i++){
     justifyCombo->addItem(placementEncNames[i]);
     if (pictureData.justification == i)
         justifyComboCurrentIndex = justifyCombo->count()-1;
  }
  justifyCombo->setCurrentIndex(justifyComboCurrentIndex);
  connect(justifyCombo,SIGNAL(currentIndexChanged(int)),
          this, SLOT(  typePositionChanged(       int)));
  gLayout->addWidget(justifyCombo, 0,3);

  //relativeTo group
  //lable
  relativeTo = new QLabel(parent);
  relativeTo->setText("Relative To");
  gLayout->addWidget(relativeTo,1,0);
  //combo
  relativeToCombo = new QComboBox(parent);
  int placementTypeNamesCount = 23;
  int relativeToComboCurrentIndex = 0;
  for(int i = 0; i < NumRelatives; i++){
      for(int j = 0; j < placementTypeNamesCount; j++){
          if (placementTypeVars[j] == i){
            relativeToCombo->addItem(placementTypeNames[j]);
          }
      }
      if (pictureData.relativeTo == i){
          relativeToComboCurrentIndex = relativeToCombo->count()-1;
      }
  }
  relativeToCombo->setCurrentIndex(relativeToComboCurrentIndex);
  connect(relativeToCombo,SIGNAL(currentIndexChanged(int)),
          this, SLOT(  typePositionChanged(          int)));
  gLayout->addWidget(relativeToCombo, 1,1);

  //preposition group
  //lable
  preposition = new QLabel(parent);
  preposition->setText("Preposition");
  gLayout->addWidget(preposition,1,2);
  //combo
  prepositionCombo = new QComboBox(parent);
  prepositionCombo->addItem("Inside");
  prepositionCombo->addItem("Outside");
  prepositionCombo->setCurrentIndex(int(pictureData.preposition));
  connect(prepositionCombo,SIGNAL(currentIndexChanged(int)),
          this, SLOT(  typePositionChanged(           int)));
  gLayout->addWidget(prepositionCombo, 1,3);

  // Image
  picture = Picture.string;
  pictureEdit = new QLineEdit(picture,parent);
  connect(pictureEdit,SIGNAL(textEdited(   QString const &)),
          this,       SLOT(  pictureChange(QString const &)));
  grid->addWidget(pictureEdit,1,0,1,2);

  pictureButton = new QPushButton("Browse",parent);
  connect(pictureButton,SIGNAL(clicked(     bool)),
          this,         SLOT(  browsePicture(bool)));
  grid->addWidget(pictureButton,1,2,1,1);

  //fill
  gbFill = new QGroupBox("Fill",parent);
  gbFill->setCheckable(true);
  gbFill->setChecked(false);
  hLayout = new QHBoxLayout();
  gbFill->setLayout(hLayout);
  grid->addWidget(gbFill,2,0,1,3);

  stretchRadio = new QRadioButton("Stretch Picture",parent);
  connect(stretchRadio,SIGNAL(clicked(bool)),
          this,        SLOT(  stretch(bool)));
  hLayout->addWidget(stretchRadio);

  tileRadio    = new QRadioButton("Tile Picture",parent);
  connect(tileRadio,SIGNAL(clicked(bool)),
          this,     SLOT(  stretch(bool)));
  hLayout->addWidget(tileRadio);

  stretchRadio->setChecked(Picture.stretch);
  tileRadio->setChecked( !Picture.stretch);

  pictureEdit->setEnabled(true);
  pictureButton->setEnabled(true);

  //scale
  gbScale = new QGroupBox("Scale", parent);
  gbScale->setCheckable(true);
  gbScale->setChecked(false);
  hLayout = new QHBoxLayout();
  gbScale->setLayout(hLayout);
  grid->addWidget(gbScale,3,0,1,3);

  scale = new QLabel("Scale Picture",parent);
  hLayout->addWidget(scale);

  min   = -10000.0;
  max   =  10000.0;
  step  =  0.01;

  spin = new QDoubleSpinBox(parent);
  spin->setRange(min,max);
  spin->setSingleStep(step);
  spin->setDecimals(6);
  spin->setValue(Picture.picScale);
  connect(spin,SIGNAL(valueChanged(double)),
          this,SLOT  (valueChanged(double)));
  hLayout->addWidget(spin);

  connect(gbFill,SIGNAL(clicked(bool)),gbScale,SLOT(setDisabled(bool)));
  connect(gbScale,SIGNAL(clicked(bool)),gbFill,SLOT(setDisabled(bool)));

  if(Picture.type == PageDocumentLogoType ||
     Picture.type == PagePlugImageType) {
      gbFill->hide();
  }

  pictureModified = false;
}

void PageAttributePictureGui::pictureChange(QString const &pic)
{
  PageAttributePictureData Picture = meta->value();
  Picture.string = pic;
  meta->setValue(Picture);
  pictureModified = true;
}

void PageAttributePictureGui::browsePicture(bool)
{
  PageAttributePictureData Picture = meta->value();
  picture = Picture.string;

  QString foo = QFileDialog::getOpenFileName(
    gui,
    tr("Choose Picture File"),
    picture,
    tr("Picture Files (*.png;*.jpg)"));
  if (foo != "") {
    picture = foo;
    Picture.string = foo;
    pictureEdit->setText(foo);
    meta->setValue(Picture);
    pictureModified = true;
  }
}

void PageAttributePictureGui::stretch(bool checked)
{
  PageAttributePictureData Picture = meta->value();
  Picture.stretch = checked;
  meta->setValue(Picture);
  pictureModified = true;
}

void PageAttributePictureGui::tile(bool checked)
{
  PageAttributePictureData Picture = meta->value();
  Picture.stretch = ! checked;
  meta->setValue(Picture);
  pictureModified = true;
}

void PageAttributePictureGui::valueChanged(double value)
{
  PageAttributePictureData Picture = meta->value();
  Picture.picScale = value;
  meta->setValue(Picture);
  pictureModified = true;
}

void PageAttributePictureGui::typePositionChanged(int type)
{
  PageAttributePictureData pictureData = meta->value();
  QObject *obj = sender();
  if(obj == placementCombo)
     pictureData.placement.placement = PlacementEnc(type);
  if(obj == justifyCombo)
     pictureData.placement.justification = PlacementEnc(type);
  if(obj == relativeToCombo)
     pictureData.placement.relativeTo = PlacementType(type);
  if(obj == prepositionCombo)
     pictureData.placement.preposition = PrepositionEnc(type);
  meta->setValue(pictureData);
  pictureModified = true;
}

void PageAttributePictureGui::toggled(bool toggled)
{
  PageAttributePictureData Picture = meta->value();

  Picture.display = toggled;
  meta->setValue(Picture);
  pictureModified = true;
}

void PageAttributePictureGui::apply(QString &topLevelFile)
{
    MetaItem mi;
    mi.beginMacro("Settings");

    if (pictureModified) {
        mi.setGlobalMeta(topLevelFile,meta);
    }
    mi.endMacro();
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

    grid->addWidget(colorLabel);

    colorExample = new QLabel(parent);
    colorExample->setFrameStyle(QFrame::Sunken|QFrame::Panel);
    colorExample->setPalette(QPalette(LDrawColor::color(meta->fadeColor.value())));
    colorExample->setAutoFillBackground(true);

    grid->addWidget(colorExample);

    colorCombo = new QComboBox(parent);
    colorCombo->addItems(LDrawColor::names());
    colorCombo->setCurrentIndex(int(colorCombo->findText(meta->fadeColor.value())));
    if (! Preferences::enableFadeStep)
        colorCombo->setDisabled(true);
    connect(colorCombo,SIGNAL(currentIndexChanged(QString const &)),
            this, SLOT(  colorChange(         QString const &)));
    colorModified = false;

    grid->addWidget(colorCombo);

    if (parent) {
        parent->setLayout(grid);
    } else {
        setLayout(grid);
    }
}


void FadeStepGui::colorChange(QString const &colorName)
{
  QColor qcolor = LDrawColor::color(meta->fadeColor.value());
  QColor newColor = LDrawColor::color(colorName);
  if (qcolor != newColor) {
    meta->fadeColor.setValue(LDrawColor::name(newColor.name()));
    colorExample->setPalette(QPalette(newColor));
    colorExample->setAutoFillBackground(true);
    colorModified = true;
  }
}


void FadeStepGui::apply(
  QString &topLevelFile)
{
  MetaItem mi;
  mi.beginMacro("GlobalFadeStepSettings");

  if (colorModified) {
    mi.setGlobalMeta(topLevelFile,&meta->fadeColor);
  }

  mi.endMacro();
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

  switch (background.type) {
    case BackgroundData::BgImage:
      picture = background.string;
    break;
    case BackgroundData::BgColor:
      color = background.string;
    break;
    default:
    break;
  }

  grid = new QGridLayout(parent);
  parent->setLayout(grid);

  combo = new QComboBox(parent);
  combo->addItem("None (transparent)");
  combo->addItem("Picture");
  combo->addItem("Solid Color");
  combo->addItem("Submodel Level Color");
  combo->setCurrentIndex(int(background.type));
  connect(combo,SIGNAL(currentIndexChanged(QString const &)),
          this, SLOT(  typeChange(         QString const &)));
  grid->addWidget(combo, 0, 0);

  /* Color */
  
  colorLabel = new QLabel(parent);
  colorLabel->setFrameStyle(QFrame::Sunken|QFrame::Panel);
  colorLabel->setPalette(QPalette(color));
  colorLabel->setAutoFillBackground(true);
  colorLabel->setPalette(QPalette(color));
  colorLabel->setAutoFillBackground(true);
  grid->addWidget(colorLabel,0,1);
  
  colorButton = new QPushButton("Change",parent);
  connect(colorButton,SIGNAL(clicked(    bool)),
          this,       SLOT(  browseColor(bool)));
  grid->addWidget(colorButton,0,2);

  /* Image */

  pictureEdit = new QLineEdit(picture,parent);
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

  stretchRadio = new QRadioButton("Stretch Picture",parent);
  connect(stretchRadio,SIGNAL(clicked(bool)),
          this,        SLOT(  stretch(bool)));
  layout->addWidget(stretchRadio);
  tileRadio    = new QRadioButton("Tile Picture",parent);
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
      colorButton->setEnabled(false);
      pictureEdit->setEnabled(true);
      pictureButton->setEnabled(true);
      fill->setEnabled(true);
    break;
    case BackgroundData::BgColor:
      colorLabel->setPalette(QPalette(color));
      // colorLabel->setAutoFillBackground(true);
      colorButton->setEnabled(true);
      pictureEdit->setEnabled(false);
      pictureButton->setEnabled(false);
      fill->setEnabled(false);
    break;
    default:
      colorButton->setEnabled(false);
      pictureEdit->setEnabled(false);
      pictureButton->setEnabled(false);
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

void BackgroundGui::browsePicture(bool)
{
  BackgroundData background = meta->value();

  QString foo = QFileDialog::getOpenFileName(
    gui,
    tr("Choose Picture File"),
    picture,
    tr("Picture Files (*.png;*.jpg)"));
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
  if (qcolor != newColor) {
    color = newColor.name();
    background.string = newColor.name();
    colorLabel->setPalette(QPalette(newColor));
    colorLabel->setAutoFillBackground(true);
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
  QGroupBox *parent)
{
  meta = _meta;

  BorderData border = meta->value();

  QString        string;
  QComboBox     *combo;
  QGridLayout   *grid;

  grid = new QGridLayout(parent);
  parent->setLayout(grid);

  /* Combo */

  combo = new QComboBox(parent);
  combo->addItem("Borderless");
  combo->addItem("Square Corners");
  combo->addItem("Round Corners");
  combo->setCurrentIndex(int(border.type));
  connect(combo,SIGNAL(currentIndexChanged(QString const &)),
          this, SLOT(  typeChange(         QString const &)));
  grid->addWidget(combo,0,0);

  /* Thickness */

  thicknessLabel = new QLabel("Width",parent);
  grid->addWidget(thicknessLabel,0,1);

  string = QString("%1") .arg(border.thickness,5,'f',4);
  thicknessEdit = new QLineEdit(string,parent);
  thicknessEdit->setInputMask("9.9000");
  connect(thicknessEdit,SIGNAL(textEdited(     QString const &)),
          this,         SLOT(  thicknessChange(QString const &)));
  grid->addWidget(thicknessEdit,0,2);

  /* Color */
  
  QLabel *label = new QLabel("Color",parent);
  grid->addWidget(label,1,0);

  colorLabel = new QLabel(parent);
  colorLabel->setFrameStyle(QFrame::Sunken|QFrame::Panel);
  colorLabel->setPalette(QPalette(border.color));
  colorLabel->setAutoFillBackground(true);
  grid->addWidget(colorLabel,1,1);
  
  colorButton = new QPushButton("Change",parent);
  connect(colorButton,SIGNAL(clicked(    bool)),
          this,       SLOT(  browseColor(bool)));
  grid->addWidget(colorButton,1,2);

  /* Radius */

  spinLabel = new QLabel("Radius",parent);
  grid->addWidget(spinLabel,2,0);

  spin = new QSpinBox(parent);
  spin->setRange(0,100);
  spin->setSingleStep(5);
  spin->setValue(int(border.radius));
  grid->addWidget(spin,2,1);
  connect(spin,SIGNAL(valueChanged(int)),
          this,SLOT(  radiusChange(int)));

  /* Margins */
  
  label = new QLabel("Margins",parent);
  grid->addWidget(label,3,0);

  QLineEdit *lineEdit;

  string = QString("%1") .arg(border.margin[0],5,'f',4);
  lineEdit = new QLineEdit(string,parent);
  grid->addWidget(lineEdit,3,1);
  connect(lineEdit,SIGNAL(textEdited(QString const &)),
          this,    SLOT(marginXChange(QString const &)));

  string = QString("%1") .arg(border.margin[1],5,'f',4);
  lineEdit = new QLineEdit(string,parent);
  grid->addWidget(lineEdit,3,2);
  connect(lineEdit,SIGNAL(textEdited(QString const &)),
          this,    SLOT(marginYChange(QString const &)));  

  enable();
}
void BorderGui::enable()
{
  BorderData border = meta->value();

  switch (border.type) {
    case BorderData::BdrNone:
      thicknessLabel->setEnabled(false);
      thicknessEdit->setEnabled(false);
      colorButton->setEnabled(false);
      spin->setEnabled(false);
      spinLabel->setEnabled(false);
    break;
    case BorderData::BdrSquare:
      thicknessLabel->setEnabled(true);
      thicknessEdit->setEnabled(true);
      colorButton->setEnabled(true);
      spin->setEnabled(false);
      spinLabel->setEnabled(false);
    break;
    default:
      thicknessLabel->setEnabled(true);
      thicknessEdit->setEnabled(true);
      colorButton->setEnabled(true);
      spin->setEnabled(true);
      spinLabel->setEnabled(true);
    break;
  }
}

void BorderGui::typeChange(QString const &type)
{
  BorderData border = meta->value();

  if (type == "Borderless") {
    border.type = BorderData::BdrNone;
  } else if (type == "Square Corners") {
    border.type = BorderData::BdrSquare;
  } else {
    border.type = BorderData::BdrRound;
  }
  meta->setValue(border);
  enable();
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
    colorLabel->setPalette(QPalette(newColor));
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

void BorderGui::apply(QString &modelName)
{
  if (modified) {
    MetaItem mi;
    mi.setGlobalMeta(modelName,meta);
  }
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
  colorExample->setPalette(QPalette(sep.color));
  colorExample->setAutoFillBackground(true);
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
  clicked = clicked;
  SepData sep = meta->value();

  QColor color = LDrawColor::color(sep.color);
  QColor newColor = QColorDialog::getColor(color,this);
  if (color != newColor) {
    sep.color = newColor.name();
    meta->setValue(sep);
    colorExample->setPalette(QPalette(newColor));
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
  QGroupBox *parent)
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
  combo->addItem("Dots Per Centimeter");
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
    // We must convert all units in project to centimeters
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
  QGroupBox     *parent)
{
  QHBoxLayout *layout;

  layout = new QHBoxLayout(parent);
  
  if (parent) {
    parent->setLayout(layout);
  } else {
    setLayout(layout);
  }

  combo = new QComboBox(parent);
  if (Preferences::ldgliteExe != "") {
    combo->addItem("LDGLite");
  }
  if (Preferences::ldviewExe != "") {
    combo->addItem("LDView");
  }
  QString renderer = Render::getRenderer();

  int currentIndex = renderer == "LDView";

  combo->setCurrentIndex(currentIndex);
  connect(combo,SIGNAL(currentIndexChanged(QString const &)),
          this, SLOT(  typeChange(         QString const &)));
  layout->addWidget(combo);
}

void RendererGui::typeChange(QString const &type)
{
  pick = type;
  modified = true;
}

void RendererGui::apply(QString & /* unused */)
{
  if (modified) {
    Render::setRenderer(pick);
  }
}
