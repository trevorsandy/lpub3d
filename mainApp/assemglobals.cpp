 
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
#include "name.h"

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

  QTabWidget  *tab = new QTabWidget(nullptr);
  QVBoxLayout *layout = new QVBoxLayout(nullptr);
  QGridLayout *boxGrid = new QGridLayout();
  setLayout(layout);
  layout->addWidget(tab);

  QWidget *widget;
  QGridLayout *grid;

  MetaGui *child;
  QGroupBox *box;

  AssemMeta *assem = &data->meta.LPub.assem;

  /*
   * Contents tab
   */

  widget = new QWidget(nullptr);
  grid = new QGridLayout(nullptr);
  widget->setLayout(grid);

  box = new QGroupBox("Assembly");
  grid->addWidget(box);
  box->setLayout(boxGrid);

  // Scale/Native Camera Distance Factor
  if (Preferences::usingNativeRenderer) {
      child = new CameraDistFactorGui("Camera Distance Factor",
                                      &assem->cameraDistNative);
      data->children.append(child);
      data->clearCache = child->modified;
      boxGrid->addWidget(child,0,0);
  } else {
      child = new DoubleSpinGui("Scale",
        &assem->modelScale,
        assem->modelScale._min,
        assem->modelScale._max,
        0.01);
      data->children.append(child);
      data->clearCache = child->modified;
      boxGrid->addWidget(child,0,0);
  }
  child = new UnitsGui("Margins",&assem->margin);
  data->children.append(child);
  boxGrid->addWidget(child,1,0);

  /* Assembly camera settings */

  box = new QGroupBox("Default Assembly Orientation");
  grid->addWidget(box,1,0);
  boxGrid = new QGridLayout();
  box->setLayout(boxGrid);

  // camera field of view
  child = new DoubleSpinGui("Camera FOV",
                            &assem->cameraFoV,
                            assem->cameraFoV._min,
                            assem->cameraFoV._max,
                            assem->cameraFoV.value());
  data->children.append(child);
  boxGrid->addWidget(child,0,0,1,2);

  // view angles
  child = new FloatsGui("Latitude","Longitude",&assem->cameraAngles);
  data->children.append(child);
  boxGrid->addWidget(child,1,0);

  /* Step Number */
  box = new QGroupBox("Step Number");
  grid->addWidget(box,3,0);

  boxGrid->addWidget(child);
  NumberPlacementMeta *stepNumber = &data->meta.LPub.stepNumber;
  child = new NumberGui(stepNumber,box);
  data->children.append(child);

  tab->addTab(widget,"Contents");

  /*
   * Display tab
   */

  widget = new QWidget(nullptr);
  grid = new QGridLayout(nullptr);
  widget->setLayout(grid);

  box = new QGroupBox("Step");
  grid->addWidget(box);
  QVBoxLayout *vLayout = new QVBoxLayout();
  box->setLayout(vLayout);

  child = new CheckBoxGui("Step Number",&assem->showStepNumber);
  data->children.append(child);
  vLayout->addWidget(child);

  box = new QGroupBox("Annotation");
  grid->addWidget(box);
  vLayout = new QVBoxLayout();
  box->setLayout(vLayout);

  if (data->meta.LPub.pli.annotation.display.value()) {
     child = new CsiAnnotationGui("",&assem->annotation);
     data->children.append(child);
     vLayout->addWidget(child);
  }

  //spacer
  QSpacerItem *vSpacer = new QSpacerItem(1,1,QSizePolicy::Fixed,QSizePolicy::Expanding);
  vLayout->addSpacerItem(vSpacer);

  tab->addTab(widget,"Display");

  QDialogButtonBox *buttonBox;

  buttonBox = new QDialogButtonBox(this);
  buttonBox->addButton(QDialogButtonBox::Ok);
  connect(buttonBox,SIGNAL(accepted()),SLOT(accept()));
  buttonBox->addButton(QDialogButtonBox::Cancel);
  connect(buttonBox,SIGNAL(rejected()),SLOT(cancel()));

  grid->addWidget(buttonBox);

  setModal(true);
  setMinimumSize(40,20);
}

void GlobalAssemDialog::getAssemGlobals(
  QString topLevelFile, Meta &meta)
{
  GlobalAssemDialog *dlg = new GlobalAssemDialog(topLevelFile, meta);

  dlg->exec();
}

void GlobalAssemDialog::accept()
{
  MetaItem mi;

  if (data->clearCache) {
    clearCsiCache();
    clearTempCache();
  }

  mi.beginMacro("Global Assem");

  MetaGui *child;

  foreach(child,data->children) {
    child->apply(data->topLevelFile);
  }
  mi.endMacro();

  QDialog::accept();
}

void GlobalAssemDialog::cancel()
{
  QDialog::reject();
}
