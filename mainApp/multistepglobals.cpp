
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

#include "globals.h"
#include <QWidget>
#include <QString>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QDialogButtonBox>

#include "meta.h"
#include "metagui.h"
#include "metaitem.h"

class GlobalMultiStepPrivate
{
public:
  Meta       meta;
  QString    topLevelFile;
  QList<MetaGui *> children;
  bool       clearCache;

  GlobalMultiStepPrivate(QString &_topLevelFile, Meta &_meta)
  {
    topLevelFile = _topLevelFile;
    meta         = _meta;
    clearCache   = false;

    MetaItem mi; // examine all the globals and then return

    mi.sortedGlobalWhere(meta,topLevelFile,"ZZZZZZZ");
  }
};

GlobalMultiStepDialog::GlobalMultiStepDialog(
  QString &topLevelFile,
  Meta &meta)
{
  data = new GlobalMultiStepPrivate(topLevelFile,meta);

  setWindowTitle(tr("Step Group Globals Setup"));

  setWhatsThis(lpubWT(WT_SETUP_MULTI_STEP,windowTitle()));

  QTabWidget  *tabwidget = new QTabWidget(nullptr);
  QVBoxLayout *layout = new QVBoxLayout(tabwidget);
  QGridLayout *boxGrid = new QGridLayout();

  GlobalSizeWidget sw(QSize(200,300), QSize(200,200));
  layout->addWidget(&sw);
  setLayout(layout);
  layout->addWidget(tabwidget);

  QVBoxLayout *vlayout;
  QSpacerItem *vSpacer;

  QWidget *widget;
  MetaGui *child;
  QGroupBox *box;

  MultiStepMeta *multiStepMeta = &data->meta.LPub.multiStep;

  /*
   * Contents Tab
   */

  widget = new QWidget();
  widget->setObjectName(tr("Contents"));
  widget->setWhatsThis(lpubWT(WT_SETUP_MULTI_STEP_CONTENTS,widget->objectName()));
  vlayout = new QVBoxLayout(nullptr);
  widget->setLayout(vlayout);

  box = new QGroupBox(tr("Step Group Margins"));
  vlayout->addWidget(box);
  child = new UnitsGui(tr("L/R|T/B"),&multiStepMeta->margin,box);
  data->children.append(child);

  box = new QGroupBox(tr("Step Justification"));
  vlayout->addWidget(box);
  child = new JustifyStepGui(tr("Set Step Justification"),&multiStepMeta->justifyStep,box);
  data->children.append(child);

  box = new QGroupBox(tr("Callout And Rotate Icon"));
  box->setWhatsThis(lpubWT(WT_SETUP_MULTI_STEP_ADJUST_STEP,box->title()));
  vlayout->addWidget(box);
  child = new CheckBoxGui(tr("Adjust Step Row Or Column When Callout Or Rotate Icon Dragged."),&multiStepMeta->adjustOnItemOffset,box);
  data->children.append(child);

  box = new QGroupBox(tr("Step Number"));
  vlayout->addWidget(box);
  child = new NumberGui("",&multiStepMeta->stepNum,box);
  data->children.append(child);

  tabwidget->addTab(widget,widget->objectName());

  /*
  * Display Tab
  */

  widget = new QWidget();
  widget->setObjectName(tr("Display"));
  widget->setWhatsThis(lpubWT(WT_SETUP_MULTI_STEP_DISPLAY,widget->objectName()));
  vlayout = new QVBoxLayout(nullptr);
  widget->setLayout(vlayout);

  QVBoxLayout *childlayout = new QVBoxLayout(nullptr);

  box = new QGroupBox(tr("Parts List"));
  box->setWhatsThis(lpubWT(WT_SETUP_MULTI_STEP_DISPLAY_PARTS_LIST,box->title()));
  vlayout->addWidget(box);
  box->setLayout(childlayout);

  CheckBoxGui *childPliPerStep = new CheckBoxGui(tr("Per Step"),&multiStepMeta->pli.perStep);
  connect(childPliPerStep->getCheckBox(), SIGNAL(stateChanged(int)),
          this,                           SLOT(  pliPerStepStateChanged(int)));
  data->children.append(childPliPerStep);
  childlayout->addWidget(childPliPerStep);

  showGrpStepNumCheckBoxGui = new CheckBoxGui(tr("Show Group Step Number"),&multiStepMeta->showGroupStepNumber);
  showGrpStepNumCheckBoxGui->getCheckBox()->setEnabled(!multiStepMeta->pli.perStep.value());
  data->children.append(showGrpStepNumCheckBoxGui);
  childlayout->addWidget(showGrpStepNumCheckBoxGui);
  connect(showGrpStepNumCheckBoxGui->getCheckBox(), SIGNAL(stateChanged(int)),
          this,                                     SLOT(  showGrpStepNumStateChanged(int)));

  countGrpStepsCheckBoxGui = new CheckBoxGui(tr("Count Group Steps"),&multiStepMeta->countGroupSteps);
  countGrpStepsCheckBoxGui->getCheckBox()->setEnabled(
              multiStepMeta->showGroupStepNumber.value() &&
             !multiStepMeta->pli.perStep.value());
  data->children.append(countGrpStepsCheckBoxGui);
  childlayout->addWidget(countGrpStepsCheckBoxGui);

  child = new UnitsGui(tr("Margins L/R|T/B"),&multiStepMeta->pli.margin);
  data->children.append(child);
  childlayout->addWidget(child);

  box = new QGroupBox(tr("Submodel Preview"));
  box->setWhatsThis(lpubWT(WT_SETUP_MULTI_STEP_DISPLAY_SUBMODEL_PREVIEW,box->title()));
  vlayout->addWidget(box);
  child = new CheckBoxGui(tr("Show Submodel Preview At First Step"),&multiStepMeta->subModel.show,box);
  data->children.append(child);

  box = new QGroupBox(tr("Justify Step Number Vertical Axis Outside Placement"));
  box->setWhatsThis(lpubWT(WT_SETUP_MULTI_STEP_JUSTIFY_Y_AXIS_OUTSIDE_PLACEMENT,box->title()));
  vlayout->addWidget(box);
  child = new CheckBoxGui(tr("Enable For Multiple Ranges - Page Divider Present"),&multiStepMeta->justifyYAxisOutsidePlacement,box);
  data->children.append(child);

  //spacer
  vSpacer = new QSpacerItem(1,1,QSizePolicy::Fixed,QSizePolicy::Expanding);
  vlayout->addSpacerItem(vSpacer);

  tabwidget->addTab(widget,widget->objectName());

  /*
  * CSI Assembly Tab
  */

  widget = new QWidget();
  widget->setObjectName(tr("Assembly"));
  widget->setWhatsThis(lpubWT(WT_SETUP_MULTI_STEP_ASSEMBLY,widget->objectName()));
  vlayout = new QVBoxLayout(nullptr);
  widget->setLayout(vlayout);

  box = new QGroupBox(tr("Assembly Image"));
  box->setWhatsThis(lpubWT(WT_SETUP_SHARED_IMAGE_SIZING,box->title()));
  vlayout->addWidget(box);
  box->setLayout(boxGrid);

  // Scale
  child = new ScaleGui(tr("Scale"),&multiStepMeta->csi.modelScale);
  connect (child, SIGNAL(settingsChanged(bool)), this, SLOT(clearCache(bool)));
  data->children.append(child);
  boxGrid->addWidget(child);

  child = new UnitsGui(tr("Margins L/R|T/B"),&multiStepMeta->margin);
  data->children.append(child);
  boxGrid->addWidget(child);

  /* Assembly camera settings */

  box = new QGroupBox("Default Assembly Orientation");
  box->setWhatsThis(lpubWT(WT_SETUP_SHARED_MODEL_ORIENTATION,box->title()));
  vlayout->addWidget(box);
  boxGrid = new QGridLayout();
  box->setLayout(boxGrid);

  // camera field of view
  child = new CameraFOVGui(tr("FOV"),&multiStepMeta->csi.cameraFoV);
  child->setToolTip(tr("Camera field of view"));
  data->children.append(child);
  connect (child, SIGNAL(settingsChanged(bool)), this, SLOT(clearCache(bool)));
  boxGrid->addWidget(child,0,0);

  // camera z near
  child = new CameraZPlaneGui(tr("Z Near"),&multiStepMeta->csi.cameraZNear);
  child->setToolTip(tr("Camera Z near plane"));
  connect (child, SIGNAL(settingsChanged(bool)), this, SLOT(clearCache(bool)));
  data->children.append(child);
  boxGrid->addWidget(child,0,1);

  // camera z far
  child = new CameraZPlaneGui(tr("Z Far"),&multiStepMeta->csi.cameraZFar,true/*ZFar*/);
  child->setToolTip(tr("Camera Z far plane"));
  connect (child, SIGNAL(settingsChanged(bool)), this, SLOT(clearCache(bool)));
  data->children.append(child);
  boxGrid->addWidget(child,0,2);

  // view angles
  child = new CameraAnglesGui(tr("Camera Angles"),&multiStepMeta->csi.cameraAngles);
  child->setToolTip(tr("Camera Latitude and Longitude angles"));
  connect (child, SIGNAL(settingsChanged(bool)), this, SLOT(clearCache(bool)));
  data->children.append(child);
  boxGrid->addWidget(child,1,0,1,3);

  box = new QGroupBox(tr("Assembly Margins"));
  vlayout->addWidget(box);
  child = new UnitsGui(tr("L/R|T/B"),&multiStepMeta->csi.margin,box);
  data->children.append(child);

  box = new QGroupBox(tr("Stud Style And Automate Edge Color"));
  vlayout->addWidget(box);
  StudStyleGui *childStudStyle = new StudStyleGui(&multiStepMeta->csi.autoEdgeColor,&multiStepMeta->csi.studStyle,&multiStepMeta->csi.highContrast,box);
  childStudStyle->setToolTip(tr("Select stud style or automate edge colors. High Contrast styles repaint stud cylinders and part edges."));
  data->children.append(childStudStyle);
  connect (childStudStyle, SIGNAL(settingsChanged(bool)), this, SLOT(clearCache(bool)));

  //spacer
  vSpacer = new QSpacerItem(1,1,QSizePolicy::Fixed,QSizePolicy::Expanding);
  vlayout->addSpacerItem(vSpacer);

  tabwidget->addTab(widget,widget->objectName());

  /*
   * Divider Tab
   */

  widget = new QWidget(nullptr);
  widget->setObjectName(tr("Divider"));
  widget->setWhatsThis(lpubWT(WT_SETUP_MULTI_STEP_DIVIDER,widget->objectName()));
  vlayout = new QVBoxLayout(nullptr);
  widget->setLayout(vlayout);

  box = new QGroupBox(tr("Divider"));
  vlayout->addWidget(box);
  child = new SepGui(&multiStepMeta->sep,box);
  data->children.append(child);

  box = new QGroupBox(tr("Divider Pointers"));
  box->setWhatsThis(lpubWT(WT_SETUP_SHARED_POINTERS,box->title()));
  vlayout->addWidget(box);
  QVBoxLayout *childLayout = new QVBoxLayout(nullptr);
  box->setLayout(childLayout);

  QGroupBox * childBox = new QGroupBox(tr("Border"));
  childLayout->addWidget(childBox);
  PointerAttribData pad = multiStepMeta->divPointerAttrib.value();
  pad.attribType = PointerAttribData::Border;
  multiStepMeta->divPointerAttrib.setValue(pad);
  child = new PointerAttribGui(&multiStepMeta->divPointerAttrib,childBox);
  data->children.append(child);

  childBox = new QGroupBox(tr("Line"));
  childLayout->addWidget(childBox);
  pad.attribType = PointerAttribData::Line;
  multiStepMeta->divPointerAttrib.setValue(pad);
  child = new PointerAttribGui(&multiStepMeta->divPointerAttrib,childBox);
  data->children.append(child);

  childBox = new QGroupBox(tr("Tip"));
  childLayout->addWidget(childBox);
  pad.attribType = PointerAttribData::Tip;
  multiStepMeta->divPointerAttrib.setValue(pad);
  child = new PointerAttribGui(&multiStepMeta->divPointerAttrib,childBox);
  data->children.append(child);

  //spacer
  vSpacer = new QSpacerItem(1,1,QSizePolicy::Fixed,QSizePolicy::Expanding);
  vlayout->addSpacerItem(vSpacer);

  tabwidget->addTab(widget,widget->objectName());

  QDialogButtonBox *buttonBox;

  buttonBox = new QDialogButtonBox();
  buttonBox->addButton(QDialogButtonBox::Ok);
  connect(buttonBox,SIGNAL(accepted()),SLOT(accept()));
  buttonBox->addButton(QDialogButtonBox::Cancel);
  connect(buttonBox,SIGNAL(rejected()),SLOT(cancel()));

  layout->addWidget(buttonBox);

  setModal(true);
}

void GlobalMultiStepDialog::getMultiStepGlobals(QString topLevelFile, Meta &meta)
{
  GlobalMultiStepDialog *dialog = new GlobalMultiStepDialog(topLevelFile, meta);
  dialog->exec();
}

void GlobalMultiStepDialog::pliPerStepStateChanged(int state)
{
    showGrpStepNumCheckBoxGui->getCheckBox()->setEnabled(!state);
    countGrpStepsCheckBoxGui->getCheckBox()->setEnabled(!state &&
    showGrpStepNumCheckBoxGui->getCheckBox()->isChecked());
}

void GlobalMultiStepDialog::showGrpStepNumStateChanged(int state) {
    if (state && !data->meta.LPub.multiStep.pli.perStep.value()
              && data->meta.LPub.contStepNumbers.value()) {
        QMessageBox box;
        box.setTextFormat (Qt::RichText);
        box.setIcon (QMessageBox::Critical);
        box.setWindowFlags (Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
        box.setWindowTitle(tr ("Count Group Step Numbers&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"));
        box.setStandardButtons (QMessageBox::Ok);
        QString text = "<b> Continuous step numbers conflict; </b>";
        QString message = QString("Count group step numbers cannot coexist with continuous step numbers.");
        box.setText (text);
        box.setInformativeText (message);
        box.exec();
    } else {
        countGrpStepsCheckBoxGui->getCheckBox()->setEnabled(state);
    }
}

void GlobalMultiStepDialog::clearCache(bool b)
{
  if (!data->clearCache)
    data->clearCache = b;
}

void GlobalMultiStepDialog::accept()
{
  MetaItem mi;

  if (data->clearCache)
    mi.clearAllCaches();

  mi.beginMacro("GlobalMultiStep");

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
void GlobalMultiStepDialog::cancel()
{
  QDialog::reject();
}

