
/****************************************************************************
**
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

#include "globals.h"
#include <QWidget>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QDialogButtonBox>

#include "meta.h"
#include "metagui.h"
#include "metaitem.h"
#include "name.h"

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

  QTabWidget  *tab = new QTabWidget(nullptr);
  QVBoxLayout *layout = new QVBoxLayout(nullptr);
  QVBoxLayout *childlayout = new QVBoxLayout(nullptr);
  setLayout(layout);
  layout->addWidget(tab);

  QWidget *widget;
  QGridLayout *grid;

  QVBoxLayout *vlayout;
  QSpacerItem *vSpacer;

  MetaGui *child;
  QGroupBox *box;

  SubModelMeta *subModelMeta = &data->meta.LPub.subModel;

  /*
   * Submodel tab
   */

  widget = new QWidget(nullptr);
  grid = new QGridLayout(nullptr);
  widget->setLayout(grid);

  QTabWidget *childtab    = new QTabWidget();
  grid->addWidget(childtab);
  tab->addTab(widget,"Submodel");

  widget = new QWidget();
  vlayout = new QVBoxLayout(nullptr);
  widget->setLayout(vlayout);

  box = new QGroupBox("Preview Options");
  vlayout->addWidget(box);
  box->setLayout(childlayout);

  child = new ShowSubModelGui(&data->meta.LPub.subModel);
  data->children.append(child);
  childlayout->addWidget(child);
  connect(child,SIGNAL(instanceCountClicked(bool)),
          this, SLOT(instanceCountClicked(bool)));

  vSpacer = new QSpacerItem(1,1,QSizePolicy::Fixed,QSizePolicy::Expanding);
  vlayout->addSpacerItem(vSpacer);

  childtab->addTab(widget,"Preview");

  widget = new QWidget();
  vlayout = new QVBoxLayout(nullptr);
  widget->setLayout(vlayout);

  box = new QGroupBox("Background");
  vlayout->addWidget(box);
  child = new BackgroundGui(&subModelMeta->background,box);
  data->children.append(child);

  box = new QGroupBox("Border");
  vlayout->addWidget(box);
  child = new BorderGui(&subModelMeta->border,box);
  data->children.append(child);

  box = new QGroupBox("Margins");
  vlayout->addWidget(box);
  child = new UnitsGui("",&subModelMeta->margin,box);
  data->children.append(child);

  vSpacer = new QSpacerItem(1,1,QSizePolicy::Fixed,QSizePolicy::Expanding);
  vlayout->addSpacerItem(vSpacer);

  childtab->addTab(widget,"Settings");

  /*
   * Contents tab
   */

  widget = new QWidget(nullptr);
  grid = new QGridLayout(nullptr);
  widget->setLayout(grid);
  childlayout = new QVBoxLayout(nullptr);

  childtab    = new QTabWidget();
  grid->addWidget(childtab);
  tab->addTab(widget,"Contents");

  widget = new QWidget();
  vlayout = new QVBoxLayout(nullptr);
  widget->setLayout(vlayout);

  box = new QGroupBox("Submodel Image");
  vlayout->addWidget(box);
  box->setLayout(childlayout);

  if (Preferences::usingNativeRenderer) {
      child = new CameraDistFactorGui("Camera Distance Factor",
                                      &subModelMeta->cameraDistNative);
      data->children.append(child);
      childlayout->addWidget(child);
  } else {
      child = new DoubleSpinGui("Scale",
        &subModelMeta->modelScale,
        subModelMeta->modelScale._min,
        subModelMeta->modelScale._max,
        0.01f);
      data->children.append(child);
      childlayout->addWidget(child);
  }
  data->clearCache = (data->clearCache ? data->clearCache : child->modified);

  child = new UnitsGui("Margins",&subModelMeta->part.margin);
  data->children.append(child);
  childlayout->addWidget(child);

  /* Constraints */
  box = new QGroupBox("Constrain");
  vlayout->addWidget(box);
  child = new ConstrainGui("",&subModelMeta->constrain,box);
  data->children.append(child);

  box = new QGroupBox("Default Submodel Orientation");
  vlayout->addWidget(box);
  QGridLayout *boxGrid = new QGridLayout();
  box->setLayout(boxGrid);

  // camera field of view
  child = new DoubleSpinGui("Camera FOV",
                             &subModelMeta->cameraFoV,
                             subModelMeta->cameraFoV._min,
                             subModelMeta->cameraFoV._max,
                             0.01f);
  data->children.append(child);
  data->clearCache = (data->clearCache ? data->clearCache : child->modified);
  boxGrid->addWidget(child,0,0,1,2);

  // view angles
  child = new FloatsGui("Latitude","Longitude",&subModelMeta->cameraAngles);
  data->children.append(child);
  data->clearCache = (data->clearCache ? data->clearCache : child->modified);
  boxGrid->addWidget(child,1,0);

  box = new QGroupBox("Default Step Rotation");
  vlayout->addWidget(box);
  child = new RotStepGui(&subModelMeta->rotStep,box);
  data->children.append(child);
  data->clearCache = (data->clearCache ? data->clearCache : child->modified);

  vSpacer = new QSpacerItem(1,1,QSizePolicy::Fixed,QSizePolicy::Expanding);
  vlayout->addSpacerItem(vSpacer);

  childtab->addTab(widget,"Image");

  widget = new QWidget();
  vlayout = new QVBoxLayout(nullptr);
  widget->setLayout(vlayout);

  instanceCountBox = new QGroupBox("Submodel Instance Count");
  instanceCountBox->setEnabled(subModelMeta->showInstanceCount.value());
  vlayout->addWidget(instanceCountBox);
  child = new NumberGui(&subModelMeta->instance,instanceCountBox);
  data->children.append(child);

  vSpacer = new QSpacerItem(1,1,QSizePolicy::Fixed,QSizePolicy::Expanding);
  vlayout->addSpacerItem(vSpacer);

  childtab->addTab(widget,"Instance");

  /*
   * SM Submodel level color
   */

  widget = new QWidget(nullptr);
  //QVBoxLayout *vlayout = new QVBoxLayout(nullptr);
  vlayout = new QVBoxLayout(nullptr);
  widget->setLayout(vlayout);

  box = new QGroupBox(tr("Submodel Level Colors"));
  vlayout->addWidget(box);
  child = new SubModelColorGui(&subModelMeta->subModelColor,box);
  data->children.append(child);

  //spacer
  //QSpacerItem *vSpacer = new QSpacerItem(1,1,QSizePolicy::Fixed,QSizePolicy::Expanding);
  vSpacer = new QSpacerItem(1,1,QSizePolicy::Fixed,QSizePolicy::Expanding);
  vlayout->addSpacerItem(vSpacer);

  tab->addTab(widget,"Submodel Colors");

  QDialogButtonBox *buttonBox;

  buttonBox = new QDialogButtonBox(nullptr);
  buttonBox->addButton(QDialogButtonBox::Ok);
  connect(buttonBox,SIGNAL(accepted()),SLOT(accept()));
  buttonBox->addButton(QDialogButtonBox::Cancel);
  connect(buttonBox,SIGNAL(rejected()),SLOT(cancel()));

  layout->addWidget(buttonBox);

  setModal(true);
  setMinimumSize(40,20);
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
      instanceCountBox->setToolTip("Check 'Show submodel instance count' in the 'Preview' tab to enable.");
}

void GlobalSubModelDialog::accept()
{
  if (data->clearCache) {
      clearSubmodelCache();
  }

  MetaItem mi;

  mi.beginMacro("Global Submodel");

  MetaGui *child;

  foreach(child,data->children) {
    child->apply(data->topLevelFile);
  }
  mi.endMacro();
  QDialog::accept();
}

void GlobalSubModelDialog::cancel()
{
  QDialog::reject();
}
