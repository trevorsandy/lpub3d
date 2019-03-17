 
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
  MetaGui   *modelScale;

  GlobalAssemPrivate(QString &_topLevelFile, Meta &_meta)
  {
    topLevelFile = _topLevelFile;
    meta = _meta;

    MetaItem mi; // examine all the globals and then return

    mi.sortedGlobalWhere(meta,topLevelFile,"ZZZZZZZ");
  }
};

GlobalAssemDialog::GlobalAssemDialog(
  QString &topLevelFile, Meta &meta)
{
  data = new GlobalAssemPrivate(topLevelFile,meta);

  QGridLayout *grid;
  QGridLayout *boxGrid;
  QGroupBox   *box;

  setWindowTitle(tr("Assembly Globals Setup"));

  grid = new QGridLayout();
  setLayout(grid);

  box = new QGroupBox("Assembly");
  grid->addWidget(box,0,0);
  boxGrid = new QGridLayout();
  box->setLayout(boxGrid);

  MetaGui *child;

  AssemMeta *assem = &data->meta.LPub.assem;

  // Scale/Native Camera Distance Factor
  if (Preferences::usingNativeRenderer) {
      child = new CameraDistFactorGui("Camera Distance Factor",
                                      &assem->cameraDistNative,
                                      box);
      data->children.append(child);
      boxGrid->addWidget(child,0,0);
  } else {
      child = new DoubleSpinGui("Scale",
        &assem->modelScale,
        assem->modelScale._min,
        assem->modelScale._max,
        0.01);
      data->children.append(child);
      data->modelScale = child;
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

  NumberPlacementMeta *stepNumber = &data->meta.LPub.stepNumber;

  child = new NumberGui(stepNumber,box);
  data->children.append(child);
  boxGrid->addWidget(child);

  box = new QGroupBox("Display");
  grid->addWidget(box,4,0);

  child = new CheckBoxGui("Step Number",&assem->showStepNumber,box);
  data->children.append(child);
  boxGrid->addWidget(child);

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

  if (data->modelScale->modified) {
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
