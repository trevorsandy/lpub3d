
/****************************************************************************
**
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
 * Global to Submodel
 *
 ****************************************************************/

class GlobalSubModelPrivate
{
public:
  Meta     meta;
  QString  topLevelFile;
  QList<MetaGui *> children;
  bool clearCache;

  GlobalSubModelPrivate(QString &_topLevelFile, Meta &_meta)
  {
    clearCache = false;
    topLevelFile = _topLevelFile;
    meta = _meta;
    MetaItem mi; // examine all the globals and then return

    mi.sortedGlobalWhere(meta,topLevelFile,"ZZZZZZZ");
  }
};

GlobalSubModelDialog::GlobalSubModelDialog(
  QString &topLevelFile, Meta &meta)
{
  data = new GlobalSubModelPrivate(topLevelFile,meta);

  setWindowTitle(tr("Submodel Globals Setup"));

  setWhatsThis(lpubWT(WT_SETUP_SUBMODEL,windowTitle()));

  QTabWidget  *tabwidget = new QTabWidget(nullptr);
  QVBoxLayout *layout = new QVBoxLayout(tabwidget);
  QVBoxLayout *childlayout = new QVBoxLayout(nullptr);

  GlobalSizeWidget sw(QSize(100,150), QSize(80,80));
  layout->addWidget(&sw);
  setLayout(layout);
  layout->addWidget(tabwidget);

  QWidget *widget;
  QGridLayout *grid;

  QVBoxLayout *vlayout;
  QSpacerItem *vSpacer;

  MetaGui *child;
  QGroupBox *box;

  SubModelMeta *subModelMeta = &data->meta.LPub.subModel;

  /*
   * Submodel Tab
   */

  widget = new QWidget(nullptr);
  widget->setObjectName(tr("Submodel"));
  widget->setWhatsThis(lpubWT(WT_SETUP_SUBMODEL_SUBMODEL,widget->objectName()));
  grid = new QGridLayout(nullptr);
  widget->setLayout(grid);

  QTabWidget *childtabwidget    = new QTabWidget();
  grid->addWidget(childtabwidget);

  tabwidget->addTab(widget,widget->objectName());

  /*
   * Preview Tab
   */

  widget = new QWidget();
  widget->setObjectName(tr("Preview"));
  widget->setWhatsThis(lpubWT(WT_SETUP_SUBMODEL_PREVIEW,widget->objectName()));
  vlayout = new QVBoxLayout(nullptr);
  widget->setLayout(vlayout);

  box = new QGroupBox(tr("Preview Display"));
  vlayout->addWidget(box);
  box->setLayout(childlayout);
  child = new ShowSubModelGui(&data->meta.LPub.subModel);
  data->children.append(child);
  childlayout->addWidget(child);
  connect(child,SIGNAL(instanceCountClicked(bool)),
          this, SLOT(instanceCountClicked(bool)));

  vSpacer = new QSpacerItem(1,1,QSizePolicy::Fixed,QSizePolicy::Expanding);
  vlayout->addSpacerItem(vSpacer);

  childtabwidget->addTab(widget,widget->objectName());

  /*
   * Settings Tab
   */

  widget = new QWidget();
  widget->setObjectName(tr("Settings"));
  widget->setWhatsThis(lpubWT(WT_SETUP_SUBMODEL_BACKGROUND,widget->objectName()));
  vlayout = new QVBoxLayout(nullptr);
  widget->setLayout(vlayout);

  box = new QGroupBox(tr("Background"));
  vlayout->addWidget(box);
  child = new BackgroundGui(&subModelMeta->background,box);
  data->children.append(child);

  box = new QGroupBox(tr("Border"));
  vlayout->addWidget(box);
  child = new BorderGui(&subModelMeta->border,box);
  data->children.append(child);

  box = new QGroupBox(tr("Margins"));
  vlayout->addWidget(box);
  child = new UnitsGui(tr("L/R|T/B"),&subModelMeta->margin,box);
  data->children.append(child);

  vSpacer = new QSpacerItem(1,1,QSizePolicy::Fixed,QSizePolicy::Expanding);
  vlayout->addSpacerItem(vSpacer);

  childtabwidget->addTab(widget,widget->objectName());

  /*
   * Contents Tab
   */

  widget = new QWidget(nullptr);
  widget->setObjectName(tr("Contents"));
  widget->setWhatsThis(lpubWT(WT_SETUP_SUBMODEL_CONTENTS,widget->objectName()));
  grid = new QGridLayout(nullptr);
  widget->setLayout(grid);
  childlayout = new QVBoxLayout(nullptr);

  childtabwidget    = new QTabWidget();
  grid->addWidget(childtabwidget);

  tabwidget->addTab(widget,widget->objectName());

  /*
   * Image Tab
   */

  widget = new QWidget();
  widget->setObjectName(tr("Image"));
  widget->setWhatsThis(lpubWT(WT_SETUP_SUBMODEL_IMAGE,widget->objectName()));
  vlayout = new QVBoxLayout(nullptr);
  widget->setLayout(vlayout);

  box = new QGroupBox("Submodel Image");
  box->setWhatsThis(lpubWT(WT_SETUP_SHARED_IMAGE_SIZING,box->title()));
  vlayout->addWidget(box);
  box->setLayout(childlayout);

  child = new ScaleGui(tr("Scale"),&subModelMeta->modelScale);
  data->children.append(child);
  connect (child, SIGNAL(settingsChanged(bool)), this, SLOT(clearCache(bool)));
  childlayout->addWidget(child);

  child = new UnitsGui(tr("Margins L/R|T/B"),&subModelMeta->part.margin);
  data->children.append(child);
  childlayout->addWidget(child);

  /* Constraints */
  box = new QGroupBox(tr("Constrain"));
  vlayout->addWidget(box);
  child = new ConstrainGui("",&subModelMeta->constrain,box);
  data->children.append(child);

  box = new QGroupBox(tr("Default Submodel Orientation"));
  box->setWhatsThis(lpubWT(WT_SETUP_SHARED_MODEL_ORIENTATION,box->title()));
  vlayout->addWidget(box);
  QGridLayout *boxGrid = new QGridLayout();
  box->setLayout(boxGrid);

  // camera field of view
  child = new CameraFOVGui(tr("FOV"),&subModelMeta->cameraFoV);
  child->setToolTip(tr("Camera field of view"));
  data->children.append(child);
  connect (child, SIGNAL(settingsChanged(bool)), this, SLOT(clearCache(bool)));
  boxGrid->addWidget(child,0,0);

  // camera z near
  child = new CameraZPlaneGui(tr("Z Near"),&subModelMeta->cameraZNear);
  child->setToolTip(tr("Camera Z near plane"));
  connect (child, SIGNAL(settingsChanged(bool)), this, SLOT(clearCache(bool)));
  data->children.append(child);
  boxGrid->addWidget(child,0,1);

  // camera z far
  child = new CameraZPlaneGui(tr("Z Far"),&subModelMeta->cameraZFar,true/*ZFar*/);
  child->setToolTip(tr("Camera Z far plane"));
  connect (child, SIGNAL(settingsChanged(bool)), this, SLOT(clearCache(bool)));
  data->children.append(child);
  boxGrid->addWidget(child,0,2);

  // view angles
  child = new CameraAnglesGui(tr("Camera Angles"),&subModelMeta->cameraAngles);
  child->setToolTip(tr("Camera Latitude and Longitude angles"));
  data->children.append(child);
  connect (child, SIGNAL(settingsChanged(bool)), this, SLOT(clearCache(bool)));
  boxGrid->addWidget(child,1,0,1,3);

  box = new QGroupBox(tr("Default Step Rotation"));
  vlayout->addWidget(box);
  child = new RotStepGui(&subModelMeta->rotStep,box);
  data->children.append(child);
  connect (child, SIGNAL(settingsChanged(bool)), this, SLOT(clearCache(bool)));

  vSpacer = new QSpacerItem(1,1,QSizePolicy::Fixed,QSizePolicy::Expanding);
  vlayout->addSpacerItem(vSpacer);

  childtabwidget->addTab(widget,widget->objectName());

  /*
   * More... Tab
   */

  widget = new QWidget();
  widget->setObjectName(tr("More..."));
  widget->setWhatsThis(lpubWT(WT_SETUP_SUBMODEL_INSTANCE_COUNT,widget->objectName()));
  vlayout = new QVBoxLayout(nullptr);
  widget->setLayout(vlayout);

  instanceCountBox = new QGroupBox(tr("Submodel Instance Count"));
  instanceCountBox->setEnabled(subModelMeta->showInstanceCount.value());
  vlayout->addWidget(instanceCountBox);
  child = new NumberGui("",&subModelMeta->instance,instanceCountBox);
  data->children.append(child);

  box = new QGroupBox(tr("Stud Style And Automate Edge Color"));
  vlayout->addWidget(box);
  child = new StudStyleGui(&subModelMeta->autoEdgeColor,&subModelMeta->studStyle,&subModelMeta->highContrast,box);
  child->setToolTip(tr("Select stud style or automate edge colors. High Contrast styles repaint stud cylinders and part edges."));
  data->children.append(child);
  connect (child, SIGNAL(settingsChanged(bool)), this, SLOT(clearCache(bool)));

  vSpacer = new QSpacerItem(1,1,QSizePolicy::Fixed,QSizePolicy::Expanding);
  vlayout->addSpacerItem(vSpacer);

  childtabwidget->addTab(widget,widget->objectName());

  /*
   * Submodel Colors Tab
   */

  widget = new QWidget(nullptr);
  widget->setObjectName(tr("Submodel Colors"));
  widget->setWhatsThis(lpubWT(WT_SETUP_SHARED_SUBMODEL_LEVEL_COLORS,widget->objectName()));
  vlayout = new QVBoxLayout(nullptr);
  widget->setLayout(vlayout);

  box = new QGroupBox(tr("Submodel Level Colors"));
  vlayout->addWidget(box);
  child = new SubModelColorGui(&subModelMeta->subModelColor,box);
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

  setModal(true);
}

void GlobalSubModelDialog::getSubModelGlobals(
  QString topLevelFile, Meta &meta)
{
  GlobalSubModelDialog *dialog = new GlobalSubModelDialog(topLevelFile, meta);
  dialog->exec();
}

void GlobalSubModelDialog::instanceCountClicked(bool checked)
{
  instanceCountBox->setEnabled(checked);
  if (!checked)
    instanceCountBox->setToolTip(tr("Check 'Show submodel instance count' in the 'Preview' tab to enable."));
}

void GlobalSubModelDialog::clearCache(bool b)
{
  if (!data->clearCache)
    data->clearCache = b;
}

void GlobalSubModelDialog::accept()
{
  MetaItem mi;

  if (data->clearCache)
    mi.clearSMICache();

  mi.beginMacro("GlobalSubmodel");

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

void GlobalSubModelDialog::cancel()
{
  QDialog::reject();
}
