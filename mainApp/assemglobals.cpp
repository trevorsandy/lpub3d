 
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

/****************************************************************************
 *
 * This file implements a dialog that lets the user configure global settings
 * for assembly images (see global.h for the class declarations)
 *
 * Please see lpub.h for an overall description of how the files in LPub
 * make up the LPub program.
 *
 ***************************************************************************/

#include <QDialog>
#include <QGridLayout>
#include <QGroupBox>
#include <QDialogButtonBox>

#include "globals.h"
#include "meta.h"
#include "metaitem.h"
#include "metagui.h"
#include "declarations.h"

class GlobalAssemPrivate
{
public:
  Meta       meta;
  QString    topLevelFile;
  QList<MetaGui *> children;
  bool     clearCache;

  GlobalAssemPrivate(QString &_topLevelFile, Meta &_meta)
  {
    topLevelFile = _topLevelFile;
    meta = _meta;
    clearCache   = false;

    MetaItem mi; // examine all the globals and then return

    mi.sortedGlobalWhere(meta,topLevelFile,"ZZZZZZZ");
  }
};

GlobalAssemDialog::GlobalAssemDialog(
  QString &topLevelFile, Meta &meta)
{
  data = new GlobalAssemPrivate(topLevelFile,meta);

  setWindowTitle(tr("Assembly Globals Setup"));

  setWhatsThis(lpubWT(WT_SETUP_ASSEM,windowTitle()));

  QTabWidget  *tabwidget = new QTabWidget(nullptr);
  QVBoxLayout *layout = new QVBoxLayout(tabwidget);
  QGridLayout *boxGrid = new QGridLayout();
  QVBoxLayout *childlayout = new QVBoxLayout(nullptr);
  QSpacerItem *vSpacer = new QSpacerItem(1,1,QSizePolicy::Fixed,QSizePolicy::Expanding);

  GlobalSizeWidget sw(QSize(200,300), QSize(200,200));
  layout->addWidget(&sw);
  setLayout(layout);
  layout->addWidget(tabwidget);

  QWidget *widget;
  QVBoxLayout *vlayout;
  MetaGui *child;
  QGroupBox *box;

  AssemMeta *assem = &data->meta.LPub.assem;

  /*
   * Contents tab
   */

  widget = new QWidget(nullptr);
  widget->setObjectName(tr("Contents"));
  widget->setWhatsThis(lpubWT(WT_SETUP_ASSEM_CONTENTS,widget->objectName()));
  vlayout  = new QVBoxLayout(nullptr);
  widget->setLayout(vlayout);

  box = new QGroupBox(tr("Assembly Image"));
  box->setWhatsThis(lpubWT(WT_SETUP_SHARED_IMAGE_SIZING,box->title()));
  vlayout->addWidget(box);
  box->setLayout(boxGrid);

  // scale
  child = new ScaleGui(tr("Scale"),&assem->modelScale);
  data->children.append(child);
  connect (child, SIGNAL(settingsChanged(bool)), this, SLOT(clearCache(bool)));
  boxGrid->addWidget(child,0,0);

  child = new UnitsGui(tr("Margins L/R|T/B"),&assem->margin);
  data->children.append(child);
  boxGrid->addWidget(child,1,0);

  /* assembly camera settings */
  box = new QGroupBox(tr("Assembly Camera Orientation"));
  box->setWhatsThis(lpubWT(WT_SETUP_SHARED_MODEL_ORIENTATION,box->title()));
  vlayout->addWidget(box);
  boxGrid = new QGridLayout();
  box->setLayout(boxGrid);

  // camera field of view
  child = new CameraFOVGui(tr("FOV"),&assem->cameraFoV);
  child->setToolTip(tr("Camera field of view"));
  data->children.append(child);
  connect (child, SIGNAL(settingsChanged(bool)), this, SLOT(clearCache(bool)));
  boxGrid->addWidget(child,0,0);

  // camera z near
  child = new CameraZPlaneGui(tr("Z Near"),&assem->cameraZNear);
  child->setToolTip(tr("Camera Z near plane"));
  connect (child, SIGNAL(settingsChanged(bool)), this, SLOT(clearCache(bool)));
  data->children.append(child);
  boxGrid->addWidget(child,0,1);

  // camera z far
  child = new CameraZPlaneGui(tr("Z Far"),&assem->cameraZFar,true/*ZFar*/);
  child->setToolTip(tr("Camera Z far plane"));
  connect (child, SIGNAL(settingsChanged(bool)), this, SLOT(clearCache(bool)));
  data->children.append(child);
  boxGrid->addWidget(child,0,2);

  // view angles
  child = new CameraAnglesGui(tr("Camera Angles"),&assem->cameraAngles);
  child->setToolTip(tr("Camera Latitude and Longitude angles"));
  data->children.append(child);
  connect (child, SIGNAL(settingsChanged(bool)), this, SLOT(clearCache(bool)));
  boxGrid->addWidget(child,1,0,1,3);

  /* Step Number */
  box = new QGroupBox(tr("Step Number"));
  vlayout->addWidget(box);
  NumberPlacementMeta *stepNumber = &data->meta.LPub.stepNumber;
  child = new NumberGui("",stepNumber,box);
  data->children.append(child);

  tabwidget->addTab(widget,widget->objectName());

  /*
   * Display tab
   */

  bool enableAnnotations = data->meta.LPub.pli.annotation.display.value();
  bool fixedAnnotations  = data->meta.LPub.pli.annotation.fixedAnnotations.value();

  widget = new QWidget(nullptr);
  widget->setObjectName(tr("Display"));
  widget->setWhatsThis(lpubWT(WT_SETUP_ASSEM_DISPLAY,widget->objectName()));
  vlayout = new QVBoxLayout(nullptr);
  widget->setLayout(vlayout);

  box = new QGroupBox(tr("Step"));
  box->setWhatsThis(lpubWT(WT_SETUP_ASSEM_DISPLAY_STEP,box->title()));
  vlayout->addWidget(box);
  box->setLayout(childlayout);

  child = new CheckBoxGui(tr("Step Number"),&assem->showStepNumber);
  data->children.append(child);
  childlayout->addWidget(child);

  box = new QGroupBox(tr("Part Annotation"));
  box->setWhatsThis(lpubWT(WT_SETUP_ASSEM_ANNOTATION,box->title()));
  vlayout->addWidget(box);
  childlayout = new QVBoxLayout();
  box->setLayout(childlayout);

  child = new CsiAnnotationGui("",&assem->annotation,nullptr,fixedAnnotations);
  data->children.append(child);
  childlayout->addWidget(child);
  box->setEnabled(enableAnnotations);
  if (!enableAnnotations)
      box->setToolTip(tr("'Display Part List (PLI) Annotations' must be enabled to set Assembly (CSI) Part annotation."));

  box = new QGroupBox("Stud Style And Automate Edge Color");
  vlayout->addWidget(box);
  child = new StudStyleGui(&assem->autoEdgeColor,&assem->studStyle,&assem->highContrast, box);
  child->setToolTip(tr("Select stud style, High Contrast styles repaint stud cylinders and part edges."));
  data->children.append(child);
  connect(child, SIGNAL(settingsChanged(bool)), this, SLOT(clearCache(bool)));

  //spacer

  vlayout->addSpacerItem(vSpacer);

  tabwidget->addTab(widget,widget->objectName());

  QDialogButtonBox *buttonBox;

  buttonBox = new QDialogButtonBox(this);
  buttonBox->addButton(QDialogButtonBox::Ok);
  connect(buttonBox,SIGNAL(accepted()),SLOT(accept()));
  buttonBox->addButton(QDialogButtonBox::Cancel);
  connect(buttonBox,SIGNAL(rejected()),SLOT(cancel()));

  layout->addWidget(buttonBox);

  setModal(true);
}

void GlobalAssemDialog::getAssemGlobals(
  QString topLevelFile, Meta &meta)
{
  GlobalAssemDialog *dlg = new GlobalAssemDialog(topLevelFile, meta);

  dlg->exec();
}

void GlobalAssemDialog::clearCache(bool b)
{
  if (!data->clearCache)
    data->clearCache = b;
}

void GlobalAssemDialog::accept()
{
  MetaItem mi;

  if (data->clearCache)
    mi.clearCsiCache();

  mi.beginMacro("Global Assem");

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

void GlobalAssemDialog::cancel()
{
  QDialog::reject();
}
