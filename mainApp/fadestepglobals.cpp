
/****************************************************************************
**
** Copyright (C) 2015 - 2022 Trevor SANDY. All rights reserved.
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

#include <QWidget>
#include <QString>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QCheckBox>
#include <QDialogButtonBox>

#include "globals.h"
#include "meta.h"
#include "metagui.h"
#include "metaitem.h"

#include "lpub_preferences.h"
#include "step.h"


/**********************************************************************
 *
 * FadeStep
 *
 *********************************************************************/


class GlobalFadeStepPrivate
{
public:
  Meta       meta;
  QString    topLevelFile;
  QList<MetaGui *> children;
  bool  reloadFile;

  GlobalFadeStepPrivate(QString &_topLevelFile, Meta &_meta)
  {
    topLevelFile = _topLevelFile;
    meta         = _meta;
    reloadFile   = false;

    MetaItem mi; // examine all the globals and then return

    mi.sortedGlobalWhere(meta,topLevelFile,"ZZZZZZZ");
  }
};

/*
 * fade step
 * color
 */

GlobalFadeStepDialog::GlobalFadeStepDialog(
  QString &topLevelFile,
  Meta &meta)
{
  data = new GlobalFadeStepPrivate(topLevelFile,meta);
  FadeStepMeta *fadeStepMeta = &data->meta.LPub.fadeStep;
  LPubMeta *lpubMeta = &data->meta.LPub;

  setWindowTitle(tr("Fade Step Globals Setup"));

  QGridLayout   *grid;
  QGridLayout   *boxGrid;
  QGroupBox     *box;

  grid = new QGridLayout();
  setLayout(grid);

  box = new QGroupBox("Fade Previous Steps");
  grid->addWidget(box,0,0);

  boxGrid = new QGridLayout();
  box->setLayout(boxGrid);

  fadeStepChild = new FadeStepGui(fadeStepMeta);
  data->children.append(fadeStepChild);
  connect (fadeStepChild->getCheckBox(), SIGNAL(clicked(bool)), this, SLOT(enableControls(bool)));
  connect (fadeStepChild->getCheckBox(), SIGNAL(clicked(bool)), this, SLOT(reloadModelFile(bool)));
  boxGrid->addWidget(fadeStepChild,0,0);

  box = new QGroupBox("Fade Previous Steps Setup");
  grid->addWidget(box,1,0);

  boxGrid = new QGridLayout();
  box->setLayout(boxGrid);

  fadeStepSetupChild = new CheckBoxGui("Setup Fade Previous Steps",&fadeStepMeta->setup);
  fadeStepSetupChild->setToolTip(tr("Setup fade steps. Check to enable fade previous steps locally."));
  data->children.append(fadeStepSetupChild);
  connect (fadeStepSetupChild->getCheckBox(), SIGNAL(clicked(bool)), this, SLOT(reloadModelFile(bool)));
  boxGrid->addWidget(fadeStepSetupChild,0,0);

  box = new QGroupBox("Final Model Step");
  grid->addWidget(box,2,0);

  boxGrid = new QGridLayout();
  box->setLayout(boxGrid);
  box->setToolTip("Automatically, append an un-faded final step to the top level model file.");

  finalModelEnabledChild = new FinalModelEnabledGui("Enable Final Model Step",&lpubMeta->finalModelEnabled);
  data->children.append(finalModelEnabledChild);
  connect (finalModelEnabledChild->getCheckBox(), SIGNAL(clicked(bool)), this, SLOT(reloadModelFile(bool)));
  boxGrid->addWidget(finalModelEnabledChild,0,0);

  emit fadeStepChild->getCheckBox()->clicked(fadeStepChild->getCheckBox()->isChecked());

  QDialogButtonBox *buttonBox;

  buttonBox = new QDialogButtonBox();
  buttonBox->addButton(QDialogButtonBox::Ok);
  connect(buttonBox,SIGNAL(accepted()),SLOT(accept()));
  buttonBox->addButton(QDialogButtonBox::Cancel);
  connect(buttonBox,SIGNAL(rejected()),SLOT(cancel()));

  grid->addWidget(buttonBox);

  setModal(true);
  setMinimumSize(40,20);
}

void GlobalFadeStepDialog::enableControls(bool b)
{
    fadeStepSetupChild->setEnabled(!b);
    finalModelEnabledChild->setEnabled(b);
}

void GlobalFadeStepDialog::reloadModelFile(bool b)
{
    if (!data->reloadFile)
        data->reloadFile = b;
}

void GlobalFadeStepDialog::getFadeStepGlobals(
  QString topLevelFile, Meta &meta)
{
  GlobalFadeStepDialog *dialog = new GlobalFadeStepDialog(topLevelFile, meta);
  dialog->exec();
}

void GlobalFadeStepDialog::accept()
{
  MetaItem mi;

  mi.beginMacro("GlobalFadeStep");

  MetaGui *child;

  Q_FOREACH (child,data->children) {
    child->apply(data->topLevelFile);
  }

  mi.setLoadingFileFlag(data->reloadFile);

  mi.endMacro();

  if (data->reloadFile) {
    mi.reloadModelFile(true);
  }

  QDialog::accept();
}

void GlobalFadeStepDialog::cancel()
{
  QDialog::reject();
}

