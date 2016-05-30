 
/****************************************************************************
**
** Copyright (C) 2007-2009 Kevin Clague. All rights reserved.
** Copyright (C) 2016 Trevor SANDY. All rights reserved.
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

  child = new UnitsGui("Margins",&assem->margin);
  data->children.append(child);
  boxGrid->addWidget(child,0,0);
  //boxGrid->addWidget(child,0,0,1,2);

  child = new DoubleSpinGui(
    "Scale",&assem->modelScale,
    assem->modelScale._min,
    assem->modelScale._max,
    0.01);
  data->modelScale = child;
  data->children.append(child);
  boxGrid->addWidget(child,1,0);
  //boxGrid->addWidget(child,1,0,1,2);

  /* Step Number */

  box = new QGroupBox("Step Number");
  grid->addWidget(box,1,0);

  NumberPlacementMeta *stepNumber = &data->meta.LPub.stepNumber;

  child = new NumberGui(stepNumber,box);
  data->children.append(child);
  boxGrid->addWidget(child);
  //boxGrid->addWidget(child,0,0,1,2);

  box = new QGroupBox("Display");
  grid->addWidget(box,2,0);

  child = new CheckBoxGui("Step Number",&assem->showStepNumber,box);
  data->children.append(child);
  boxGrid->addWidget(child);
  //boxGrid->addWidget(child,1,0,1,2);

  QDialogButtonBox *buttonBox;

  buttonBox = new QDialogButtonBox(this);
  buttonBox->addButton(QDialogButtonBox::Ok);
  connect(buttonBox,SIGNAL(accepted()),SLOT(accept()));
  buttonBox->addButton(QDialogButtonBox::Cancel);
  connect(buttonBox,SIGNAL(rejected()),SLOT(cancel()));

  grid->addWidget(buttonBox);

  setModal(true);
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
    void clearCsiCache();
    clearCsiCache();
    void clearCsi3dCache();
    clearCsi3dCache();
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
