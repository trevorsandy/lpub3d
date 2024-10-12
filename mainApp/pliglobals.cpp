
/****************************************************************************
**
** Copyright (C) 2007-2009 Kevin Clague. All rights reserved.
** Copyright (C) 2015 - 2024 Trevor SANDY. All rights reserved.
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

#include "globals.h"
#include <QWidget>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QDialogButtonBox>

#include "meta.h"
#include "metagui.h"
#include "metaitem.h"
#include "declarations.h"

/*****************************************************************
 *
 * Global to pli
 *
 ****************************************************************/

class GlobalPliPrivate
{
public:
  Meta     meta;
  QString  topLevelFile;
  QList<MetaGui *> children;
  bool     bom;
  bool     clearCache;
  bool     clearBomCache;

  GlobalPliPrivate(QString &_topLevelFile, Meta &_meta, bool _bom = false)
  {
    topLevelFile = _topLevelFile;
    meta         = _meta;
    bom          = _bom;
    clearCache   = false;
    clearBomCache= false;

    MetaItem mi; // examine all the globals and then return

    mi.sortedGlobalWhere(meta,topLevelFile,"ZZZZZZZ");
  }
};

GlobalPliDialog::GlobalPliDialog(
  QString &topLevelFile, Meta &meta, bool bom)
{
  data = new GlobalPliPrivate(topLevelFile,meta,bom);

  if (bom) {
    setWindowTitle(tr("Bill Of Materials Globals Setup"));
    setWhatsThis(lpubWT(WT_SETUP_PART_BOM,windowTitle()));
  } else {
    setWindowTitle(tr("Parts List Globals Setup"));
    setWhatsThis(lpubWT(WT_SETUP_PART_PLI,windowTitle()));
  }

  QTabWidget  *tabwidget = new QTabWidget(nullptr);
  QVBoxLayout *layout = new QVBoxLayout(tabwidget);
  QVBoxLayout *childlayout = new QVBoxLayout(nullptr);

  GlobalSizeWidget sw(QSize(200,300), QSize(200,200));
  layout->addWidget(&sw);
  setLayout(layout);
  layout->addWidget(tabwidget);

  QWidget *widget;
  QVBoxLayout *vlayout;
  QVBoxLayout *svlayout;
  QSpacerItem *vSpacer;

  MetaGui *child;
  QGroupBox *box;

  PliMeta *pliMeta = bom ? &data->meta.LPub.bom : &data->meta.LPub.pli;

  /*
   * Background/Border Tab
   */

  widget   = new QWidget(nullptr);
  widget->setObjectName(tr("Background / Border"));
  widget->setWhatsThis(lpubWT(WT_SETUP_PART_BACKGROUND_BORDER,widget->objectName()));
  vlayout  = new QVBoxLayout(nullptr);
  svlayout = new QVBoxLayout(nullptr);
  widget->setLayout(vlayout);

  if ( ! bom) {
    QHBoxLayout *childHLayout = new QHBoxLayout(nullptr);
    box = new QGroupBox(tr("Parts List"));
    box->setWhatsThis(lpubWT(WT_SETUP_PART_BACKGROUND_BORDER_PARTS_LIST,box->title()));
    vlayout->addWidget(box);
    box->setLayout(childHLayout);
    child = new CheckBoxGui(tr("Show Parts List"),&pliMeta->show);
    childHLayout->addWidget(child);
    data->children.append(child);

    child = new PlacementGui(tr("Pli Placement"),&pliMeta->placement, PartsListType);
    childHLayout->addWidget(child);
    data->children.append(child);
  }

  box = new QGroupBox(tr("Background"));
  vlayout->addWidget(box);
  child = new BackgroundGui(&pliMeta->background,box);
  data->children.append(child);

  box = new QGroupBox(tr("Border"));
  vlayout->addWidget(box);
  child = new BorderGui(&pliMeta->border,box);
  data->children.append(child);
  
  box = new QGroupBox(tr("Margins"));
  vlayout->addWidget(box);
  child = new UnitsGui(tr("L/R|T/B"),&pliMeta->margin,box);
  data->children.append(child);
  
  box = new QGroupBox(tr("Constrain"));
  vlayout->addWidget(box);
  child = new ConstrainGui("",&pliMeta->constrain,box);
  data->children.append(child);

  //spacer
  vSpacer = new QSpacerItem(1,1,QSizePolicy::Fixed,QSizePolicy::Expanding);
  vlayout->addSpacerItem(vSpacer);

  tabwidget->addTab(widget,widget->objectName());

  /*
   * Contents Tab
   */

  widget = new QWidget(nullptr);
  widget->setObjectName(tr("Content"));
  widget->setWhatsThis(lpubWT(WT_SETUP_PART_CONTENTS,widget->objectName()));
  vlayout = new QVBoxLayout(nullptr);
  widget->setLayout(vlayout);

  QTabWidget *childtabwidget = new QTabWidget();
  vlayout->addWidget(childtabwidget);

  tabwidget->addTab(widget,widget->objectName());

  /*
   * Parts Tab
   */

  widget = new QWidget(nullptr);
  widget->setObjectName(tr("Parts"));
  widget->setWhatsThis(lpubWT(WT_SETUP_PART_PARTS,widget->objectName()));
  vlayout = new QVBoxLayout(nullptr);
  widget->setLayout(vlayout);

  box = new QGroupBox(tr("Part Images"));
  box->setWhatsThis(lpubWT(WT_SETUP_SHARED_IMAGE_SIZING,box->title()));
  vlayout->addWidget(box);
  box->setLayout(childlayout);

  // Scale
  child = new ScaleGui(tr("Scale"),&pliMeta->modelScale);
  data->children.append(child);
  connect (child, SIGNAL(settingsChanged(bool)), this, SLOT(clearCache(bool)));
  childlayout->addWidget(child);

  child = new UnitsGui(tr("Margins L/R|T/B"),&pliMeta->part.margin);
  data->children.append(child);
  childlayout->addWidget(child);

  /* Camera settings */

  box = new QGroupBox(tr("Part Camera Orientation"));
  box->setWhatsThis(lpubWT(WT_SETUP_SHARED_MODEL_ORIENTATION,box->title()));
  vlayout->addWidget(box);
  QGridLayout *boxGrid = new QGridLayout();
  box->setLayout(boxGrid);

  // camera field of view
  child = new CameraFOVGui(tr("FOV"),&pliMeta->cameraFoV);
  child->setToolTip(tr("Camera field of view"));
  connect (child, SIGNAL(settingsChanged(bool)), this, SLOT(clearCache(bool)));
  data->children.append(child);
  boxGrid->addWidget(child,0,0);

  // camera z near
  child = new CameraZPlaneGui(tr("Z Near"),&pliMeta->cameraZNear);
  child->setToolTip(tr("Camera Z near plane"));
  connect (child, SIGNAL(settingsChanged(bool)), this, SLOT(clearCache(bool)));
  data->children.append(child);
  boxGrid->addWidget(child,0,1);

  // camera z far
  child = new CameraZPlaneGui(tr("Z Far"),&pliMeta->cameraZFar,true/*ZFar*/);
  child->setToolTip(tr("Camera Z far plane"));
  connect (child, SIGNAL(settingsChanged(bool)), this, SLOT(clearCache(bool)));
  data->children.append(child);
  boxGrid->addWidget(child,0,2);

  // view angles
  child = new CameraAnglesGui(tr("Camera Angles"),&pliMeta->cameraAngles);
  child->setToolTip(tr("Camera Latitude and Longitude angles"));
  data->children.append(child);
  connect (child, SIGNAL(settingsChanged(bool)), this, SLOT(clearCache(bool)));
  boxGrid->addWidget(child,1,0,1,3);

  WT_Type wtType = bom ? WT_SETUP_PART_PARTS_MOVABLE_GROUPS_BOM : WT_SETUP_PART_PARTS_MOVABLE_GROUPS_PLI;

  box = new QGroupBox(tr("Part Groups"));
  box->setWhatsThis(lpubWT(wtType,box->title()));
  vlayout->addWidget(box);
  QString description = tr("Movable Part Groups (part image, instance count and annotation)");
  if (bom)
    description = tr("Movable Part Proups (part image, instance count, annotation and element id)");
  child = new CheckBoxGui(description,&pliMeta->enablePliPartGroup,box);
  data->children.append(child);

  box = new QGroupBox(tr("Part Count"));
  vlayout->addWidget(box);
  child = new NumberGui("",&pliMeta->instance,box);
  data->children.append(child);

  vSpacer = new QSpacerItem(1,1,QSizePolicy::Fixed,QSizePolicy::Expanding);
  vlayout->addSpacerItem(vSpacer);

  childtabwidget->addTab(widget,widget->objectName());

  /*
   * More... Tab
   */

  widget = new QWidget();
  widget->setObjectName(tr("More..."));
  widget->setWhatsThis(lpubWT(WT_SETUP_PART_MORE_OPTIONS,widget->objectName()));
  vlayout = new QVBoxLayout(nullptr);
  widget->setLayout(vlayout);

  box = new QGroupBox(tr("Sort Order And Direction"));
  vlayout->addWidget(box);
  child = new PliSortOrderGui("",&pliMeta->sortOrder,box,bom);
  data->children.append(child);

  box = new QGroupBox(tr("Stud Style And Automate Edge Color"));
  vlayout->addWidget(box);
  StudStyleGui *childStudStyle = new StudStyleGui(&pliMeta->autoEdgeColor,&pliMeta->studStyle,&pliMeta->highContrast,box);
  childStudStyle->setToolTip(tr("Select stud style or automate edge colors. High Contrast styles repaint stud cylinders and part edges."));
  data->children.append(childStudStyle);
  connect (childStudStyle, SIGNAL(settingsChanged(bool)), this, SLOT(clearCache(bool)));

  if ( ! bom) {
    box = new QGroupBox(tr("Submodels"));
    box->setWhatsThis(lpubWT(WT_SETUP_PART_MORE_OPTIONS_SHOW_SUBMODELS,box->title()));
    vlayout->addWidget(box);
    child = new CheckBoxGui(tr("Show In Parts List"),&pliMeta->includeSubs,box);
    data->children.append(child);
  }

  //spacer
  vSpacer = new QSpacerItem(1,1,QSizePolicy::Fixed,QSizePolicy::Expanding);
  vlayout->addSpacerItem(vSpacer);

  childtabwidget->addTab(widget,widget->objectName());

  /*
   * PLI Annotations
   */

  widget = new QWidget(nullptr);
  widget->setObjectName(tr("Annotations"));
  widget->setWhatsThis(lpubWT(WT_SETUP_PART_ANNOTATION,widget->objectName()));
  vlayout = new QVBoxLayout(nullptr);
  widget->setLayout(vlayout);

  childtabwidget = new QTabWidget();
  vlayout->addWidget(childtabwidget);

  tabwidget->addTab(widget,widget->objectName());

  /*
   * Annotation Options Tab
   */

  widget = new QWidget();
  widget->setObjectName(tr("Options"));
  widget->setWhatsThis(lpubWT(WT_SETUP_PART_ANNOTATION_OPTIONS,widget->objectName()));
  vlayout = new QVBoxLayout(nullptr);
  widget->setLayout(vlayout);

  box = new QGroupBox(tr("Annotation Options"));
  box->setWhatsThis(lpubWT(WT_SETUP_PART_ANNOTATION_OPTIONS_WITH_TEXT_FORMAT,box->title()));
  vlayout->addWidget(box);

  childlayout = new QVBoxLayout(nullptr);
  box->setLayout(childlayout);

  childPliAnnotation = new PliAnnotationGui("",&pliMeta->annotation,nullptr,bom);
  childPliAnnotation->enableElementStyle(pliMeta->partElements.display.value());
  connect(childPliAnnotation, SIGNAL(toggled(bool)),
          this,               SLOT(  displayAnnotationsChanged(bool)));
  data->children.append(childPliAnnotation);
  childlayout->addWidget(childPliAnnotation);

  if (bom) {
      childPliPartElement = new PliPartElementGui("",&pliMeta->partElements);
      childPliPartElement->enablePliPartElementGroup(pliMeta->annotation.display.value());
      connect(childPliPartElement, SIGNAL(toggled(bool)),
              this,                SLOT(  enableElementStyleChanged(bool)));
      data->children.append(childPliPartElement);
      childlayout->addWidget(childPliPartElement);
  }

  childTextFormat = new NumberGui(tr("Default Text Format"),&pliMeta->annotate);
  childTextFormat->enableTextFormatGroup(pliMeta->annotation.display.value());
  data->children.append(childTextFormat);
  childlayout->addWidget(childTextFormat);

  vSpacer = new QSpacerItem(1,1,QSizePolicy::Fixed,QSizePolicy::Expanding);
  vlayout->addSpacerItem(vSpacer);

  childtabwidget->addTab(widget,widget->objectName());

  /*
   * Edit Styles Tab
   */

  wtType = bom ? WT_SETUP_PART_ANNOTATION_STYLE_BOM : WT_SETUP_PART_ANNOTATION_STYLE_PLI;

  widget = new QWidget();
  widget->setObjectName(tr("Styles"));
  widget->setWhatsThis(lpubWT(wtType,widget->objectName()));
  vlayout = new QVBoxLayout(nullptr);
  widget->setLayout(vlayout);

  //Edit PLI Annotation Style Selection
  annotationEditStyleBox = new QGroupBox(tr("Annotation Style"));
  annotationEditStyleBox->setWhatsThis(lpubWT(WT_GUI_PART_ANNOTATIONS_STYLES,annotationEditStyleBox->title()));
  annotationEditStyleBox->setLayout(svlayout);
  annotationEditStyleBox->setEnabled(pliMeta->annotation.display.value());
  vlayout->addWidget(annotationEditStyleBox);

  wtType = bom ? WT_SETUP_PART_ANNOTATION_STYLE_SELECT_BOM : WT_SETUP_PART_ANNOTATION_STYLE_SELECT_PLI;

  QGroupBox *sbox = new QGroupBox(tr("Select Style To Edit"));
  sbox->setWhatsThis(lpubWT(wtType,sbox->title()));
  svlayout->addWidget(sbox);

  QHBoxLayout *shlayout = new QHBoxLayout(nullptr);
  sbox->setLayout(shlayout);

  noStyleButton = new QRadioButton(tr("None"),sbox);
  noStyleButton->setChecked(true);
  connect(noStyleButton,SIGNAL(clicked(bool)),
          this,   SLOT(  styleOptionChanged(bool)));
  shlayout->addWidget(noStyleButton);

  squareStyleButton = new QRadioButton(tr("Square"),sbox);
  squareStyleButton->setChecked(false);
  connect(squareStyleButton,SIGNAL(clicked(bool)),
          this,       SLOT(  styleOptionChanged(bool)));
  shlayout->addWidget(squareStyleButton);

  circleStyleButton = new QRadioButton(tr("Circle"),sbox);
  circleStyleButton->setChecked(false);
  connect(circleStyleButton,SIGNAL(clicked(bool)),
          this,       SLOT(  styleOptionChanged(bool)));
  shlayout->addWidget(circleStyleButton);

  rectangleStyleButton = new QRadioButton(tr("Rectangle"),sbox);
  rectangleStyleButton->setChecked(false);
  connect(rectangleStyleButton,SIGNAL(clicked(bool)),
          this,          SLOT(  styleOptionChanged(bool)));
  shlayout->addWidget(rectangleStyleButton);

  if (bom) {
      elementStyleButton = new QRadioButton(tr("Element"),sbox);
      elementStyleButton->setChecked(false);
      connect(elementStyleButton,SIGNAL(clicked(bool)),
              this,        SLOT(  styleOptionChanged(bool)));
      shlayout->addWidget(elementStyleButton);
  }

  // square style settings
  squareBkGrndStyleBox = new QGroupBox(tr("Square Background"));
  svlayout->addWidget(squareBkGrndStyleBox);
  squareBkGrndStyleBox->hide();
  child = new BackgroundGui(&pliMeta->squareStyle.background,squareBkGrndStyleBox,false);
  data->children.append(child);

  squareBorderStyleBox = new QGroupBox(tr("Square Border"));
  svlayout->addWidget(squareBorderStyleBox);
  squareBorderStyleBox->hide();
  child = new BorderGui(&pliMeta->squareStyle.border,squareBorderStyleBox);
  data->children.append(child);

  squareFormatStyleBox = new QGroupBox(tr("Square Annotation Text Format"));
  svlayout->addWidget(squareFormatStyleBox);
  squareFormatStyleBox->hide();
  styleMeta = new NumberMeta();
  styleMeta->margin = pliMeta->squareStyle.margin;
  styleMeta->font   = pliMeta->squareStyle.font;
  styleMeta->color  = pliMeta->squareStyle.color;
  child = new NumberGui("",styleMeta,squareFormatStyleBox);
  data->children.append(child);

  squareSizeStyleBox = new QGroupBox(tr("Square Size"));
  svlayout->addWidget(squareSizeStyleBox);
  squareSizeStyleBox->hide();
  child = new FloatsGui(tr("Width"),tr("Height"),&pliMeta->squareStyle.size,squareSizeStyleBox,3);
  data->children.append(child);

  // circle style settings
  circleBkGrndStyleBox = new QGroupBox(tr("Circle Background"));
  svlayout->addWidget(circleBkGrndStyleBox);
  circleBkGrndStyleBox->hide();
  child = new BackgroundGui(&pliMeta->circleStyle.background,circleBkGrndStyleBox,false);
  data->children.append(child);

  circleBorderStyleBox = new QGroupBox(tr("Circle Border"));
  svlayout->addWidget(circleBorderStyleBox);
  circleBkGrndStyleBox->hide();
  child = new BorderGui(&pliMeta->circleStyle.border,circleBorderStyleBox,false,false);
  data->children.append(child);

  circleFormatStyleBox = new QGroupBox(tr("Circle Annotation Text Format"));
  svlayout->addWidget(circleFormatStyleBox);
  circleFormatStyleBox->hide();
  styleMeta = new NumberMeta();
  styleMeta->margin = pliMeta->circleStyle.margin;
  styleMeta->font   = pliMeta->circleStyle.font;
  styleMeta->color  = pliMeta->circleStyle.color;
  child = new NumberGui("",styleMeta,circleFormatStyleBox);
  data->children.append(child);

  circleSizeStyleBox = new QGroupBox(tr("Circle Size"));
  svlayout->addWidget(circleSizeStyleBox);
  circleSizeStyleBox->hide();
  child = new FloatsGui(tr("Diameter"),"",&pliMeta->circleStyle.size,circleSizeStyleBox,3,false/*show pair*/);
  data->children.append(child);

  // rectangle style settings
  rectangleBkGrndStyleBox = new QGroupBox(tr("Rectangle Background"));
  svlayout->addWidget(rectangleBkGrndStyleBox);
  rectangleBkGrndStyleBox->hide();
  child = new BackgroundGui(&pliMeta->rectangleStyle.background,rectangleBkGrndStyleBox,false);
  data->children.append(child);

  rectangleBorderStyleBox = new QGroupBox(tr("Rectangle Border"));
  svlayout->addWidget(rectangleBorderStyleBox);
  rectangleBorderStyleBox->hide();
  child = new BorderGui(&pliMeta->rectangleStyle.border,rectangleBorderStyleBox);
  data->children.append(child);

  rectangleFormatStyleBox = new QGroupBox(tr("Rectangle Annotation Text Format"));
  svlayout->addWidget(rectangleFormatStyleBox);
  rectangleFormatStyleBox->hide();
  styleMeta = new NumberMeta();
  styleMeta->margin = pliMeta->rectangleStyle.margin;
  styleMeta->font   = pliMeta->rectangleStyle.font;
  styleMeta->color  = pliMeta->rectangleStyle.color;
  child = new NumberGui("",styleMeta,rectangleFormatStyleBox);
  data->children.append(child);

  rectangleSizeStyleBox = new QGroupBox(tr("Rectangle Size"));
  svlayout->addWidget(rectangleSizeStyleBox);
  child = new FloatsGui(tr("Width"),tr("Height"),&pliMeta->rectangleStyle.size,rectangleSizeStyleBox,3);
  data->children.append(child);

  if (bom) {
      // element style settings
      elementBkGrndStyleBox = new QGroupBox(tr("Element Background"));
      svlayout->addWidget(elementBkGrndStyleBox);
      elementBkGrndStyleBox->hide();
      child = new BackgroundGui(&pliMeta->elementStyle.background,elementBkGrndStyleBox,false);
      data->children.append(child);

      elementBorderStyleBox = new QGroupBox(tr("Element Border"));
      svlayout->addWidget(elementBorderStyleBox);
      elementBorderStyleBox->hide();
      child = new BorderGui(&pliMeta->elementStyle.border,elementBorderStyleBox);
      data->children.append(child);

      elementFormatStyleBox = new QGroupBox(tr("Element Annotation Text Format"));
      svlayout->addWidget(elementFormatStyleBox);
      elementFormatStyleBox->hide();
      styleMeta = new NumberMeta();
      styleMeta->margin = pliMeta->elementStyle.margin;
      styleMeta->font   = pliMeta->elementStyle.font;
      styleMeta->color  = pliMeta->elementStyle.color;
      child = new NumberGui("",styleMeta,elementFormatStyleBox);
      data->children.append(child);

      elementSizeStyleBox = new QGroupBox(tr("Element Size"));
      svlayout->addWidget(elementSizeStyleBox);
      child = new FloatsGui(tr("Width"),tr("Height"),&pliMeta->elementStyle.size,elementSizeStyleBox,3);
      elementSizeStyleBox->setDisabled(true);
      elementSizeStyleBox->setToolTip(tr("Size automatically adjusts to the size of the annotation text"));
      data->children.append(child);
  }

  //spacer
  vSpacer = new QSpacerItem(1,1,QSizePolicy::Fixed,QSizePolicy::Expanding);
  vlayout->addSpacerItem(vSpacer);

  childtabwidget->addTab(widget,widget->objectName());

  /*
   * Submodel colors
   */

  widget = new QWidget();
  widget->setObjectName(tr("Submodel Colors"));
  widget->setWhatsThis(lpubWT(WT_SETUP_SHARED_SUBMODEL_LEVEL_COLORS,widget->objectName()));
  vlayout = new QVBoxLayout(nullptr);
  widget->setLayout(vlayout);

  box = new QGroupBox(tr("Submodel Level Colors"));
  vlayout->addWidget(box);
  child = new SubModelColorGui(&pliMeta->subModelColor,box);
  data->children.append(child);

  //spacer
  vSpacer = new QSpacerItem(1,1,QSizePolicy::Fixed,QSizePolicy::Expanding);
  vlayout->addSpacerItem(vSpacer);

  tabwidget->addTab(widget,widget->objectName());

  QDialogButtonBox *buttonBox;

  buttonBox = new QDialogButtonBox(nullptr);
  buttonBox->addButton(QDialogButtonBox::Ok);
  connect(buttonBox,SIGNAL(accepted()),SLOT(accept()));
  buttonBox->addButton(QDialogButtonBox::Cancel);
  connect(buttonBox,SIGNAL(rejected()),SLOT(cancel()));

  layout->addWidget(buttonBox);

  styleOptionChanged(true);

  setModal(true);
}

void GlobalPliDialog::getPliGlobals(
  QString topLevelFile, Meta &meta)
{
  GlobalPliDialog *dialog = new GlobalPliDialog(topLevelFile, meta, false);
  dialog->exec();
}

void GlobalPliDialog::getBomGlobals(
  QString topLevelFile, Meta &meta)
{
  GlobalPliDialog *dialog = new GlobalPliDialog(topLevelFile, meta, true);
  dialog->exec();
}

void GlobalPliDialog::clearCache(bool b)
{
  if (data->bom) {
    if (!data->clearBomCache)
      data->clearBomCache = b;
  } else {
    if (!data->clearCache)
      data->clearCache = b;
  }
}

void GlobalPliDialog::styleOptionChanged(bool b){

  Q_UNUSED(b)

  QObject *obj = sender();
  if (obj == squareStyleButton) {
    squareBkGrndStyleBox->setDisabled(false);
    squareBorderStyleBox->setDisabled(false);
    squareFormatStyleBox->setDisabled(false);
    squareSizeStyleBox->setDisabled(false);

    squareBkGrndStyleBox->show();
    squareBorderStyleBox->show();
    squareFormatStyleBox->show();
    squareSizeStyleBox->show();

    circleBkGrndStyleBox->hide();
    circleBorderStyleBox->hide();
    circleFormatStyleBox->hide();
    circleSizeStyleBox->hide();

    rectangleBkGrndStyleBox->hide();
    rectangleBorderStyleBox->hide();
    rectangleFormatStyleBox->hide();
    rectangleSizeStyleBox->hide();
    if (data->bom) {
      elementBkGrndStyleBox->hide();
      elementBorderStyleBox->hide();
      elementFormatStyleBox->hide();
      elementSizeStyleBox->hide();
    }
  }
  else
  if (obj == circleStyleButton) {
    circleBkGrndStyleBox->show();
    circleBorderStyleBox->show();
    circleFormatStyleBox->show();
    circleSizeStyleBox->show();

    squareBkGrndStyleBox->hide();
    squareBorderStyleBox->hide();
    squareFormatStyleBox->hide();
    squareSizeStyleBox->hide();

    rectangleBkGrndStyleBox->hide();
    rectangleBorderStyleBox->hide();
    rectangleFormatStyleBox->hide();
    rectangleSizeStyleBox->hide();

    if (data->bom) {
      elementBkGrndStyleBox->hide();
      elementBorderStyleBox->hide();
      elementFormatStyleBox->hide();
      elementSizeStyleBox->hide();
    }
  }
  else
  if (obj == rectangleStyleButton) {
    rectangleBkGrndStyleBox->show();
    rectangleBorderStyleBox->show();
    rectangleFormatStyleBox->show();
    rectangleSizeStyleBox->show();

    squareBkGrndStyleBox->hide();
    squareBorderStyleBox->hide();
    squareFormatStyleBox->hide();
    squareSizeStyleBox->hide();

    circleBkGrndStyleBox->hide();
    circleBorderStyleBox->hide();
    circleFormatStyleBox->hide();
    circleSizeStyleBox->hide();

    if (data->bom) {
      elementBkGrndStyleBox->hide();
      elementBorderStyleBox->hide();
      elementFormatStyleBox->hide();
      elementSizeStyleBox->hide();
    }
  }
  else
  if (data->bom && obj == elementStyleButton) {
    elementBkGrndStyleBox->show();
    elementBorderStyleBox->show();
    elementFormatStyleBox->show();
    elementSizeStyleBox->show();

    rectangleBkGrndStyleBox->hide();
    rectangleBorderStyleBox->hide();
    rectangleFormatStyleBox->hide();
    rectangleSizeStyleBox->hide();

    squareBkGrndStyleBox->hide();
    squareBorderStyleBox->hide();
    squareFormatStyleBox->hide();
    squareSizeStyleBox->hide();

    circleBkGrndStyleBox->hide();
    circleBorderStyleBox->hide();
    circleFormatStyleBox->hide();
    circleSizeStyleBox->hide();
  } else {
    squareBkGrndStyleBox->show();
    squareBorderStyleBox->show();
    squareFormatStyleBox->show();
    squareSizeStyleBox->show();

    circleBkGrndStyleBox->hide();
    circleBorderStyleBox->hide();
    circleFormatStyleBox->hide();
    circleSizeStyleBox->hide();

    rectangleBkGrndStyleBox->hide();
    rectangleBorderStyleBox->hide();
    rectangleFormatStyleBox->hide();
    rectangleSizeStyleBox->hide();

    if (data->bom) {
      elementBkGrndStyleBox->hide();
      elementBorderStyleBox->hide();
      elementFormatStyleBox->hide();
      elementSizeStyleBox->hide();
    }

    squareBkGrndStyleBox->setDisabled(true);
    squareBorderStyleBox->setDisabled(true);
    squareFormatStyleBox->setDisabled(true);
    squareSizeStyleBox->setDisabled(true);
  }
}

void GlobalPliDialog::displayAnnotationsChanged(bool b) {
  annotationEditStyleBox->setEnabled(b);
  childTextFormat->enableTextFormatGroup(b);
  if (data->bom)
    childPliPartElement->enablePliPartElementGroup(b);
}

void GlobalPliDialog::enableElementStyleChanged(bool b) {
  childPliAnnotation->enableElementStyle(b);
}

void GlobalPliDialog::accept()
{
  MetaItem mi;

  if (data->clearCache)
    mi.clearPliCache();

  if (data->clearBomCache)
    mi.clearBomCache();

  mi.beginMacro("Global Pli");

  bool noFileDisplay = false;

  MetaGui *child;
  Q_FOREACH (child,data->children) {
    child->apply(data->topLevelFile);
    noFileDisplay |= child->modified;
  }

  mi.setSuspendFileDisplayFlag(!noFileDisplay);

  mi.endMacro();

  QDialog::accept();
}

void GlobalPliDialog::cancel()
{
  QDialog::reject();
}
