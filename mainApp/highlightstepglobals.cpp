
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

#include "globals.h"
#include <QWidget>
#include <QString>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QCheckBox>
#include <QDialogButtonBox>

#include "meta.h"
#include "metagui.h"
#include "metaitem.h"

#include "lpub_preferences.h"
#include "step.h"


/**********************************************************************
 *
 * HighlightStep
 *
 *********************************************************************/


class GlobalHighlightStepPrivate
{
public:
  Meta       meta;
  QString    topLevelFile;
  QList<MetaGui *> children;
  bool  reloadFile;

  GlobalHighlightStepPrivate(QString &_topLevelFile, Meta &_meta)
  {
    topLevelFile = _topLevelFile;
    meta         = _meta;
    reloadFile   = false;

    MetaItem mi; // examine all the globals and then return

    mi.sortedGlobalWhere(meta,topLevelFile,"ZZZZZZZ");
  }
};

/*
 * highlight step
 * color
 */

GlobalHighlightStepDialog::GlobalHighlightStepDialog(
  QString &topLevelFile,
  Meta &meta)
{
  data = new GlobalHighlightStepPrivate(topLevelFile,meta);
  HighlightStepMeta *highlightStepMeta = &data->meta.LPub.highlightStep;
  LPubMeta *lpubMeta = &data->meta.LPub;

  setWindowTitle(tr("Highlight Step Globals Setup"));

  QGridLayout   *grid;
  QGridLayout   *boxGrid;
  QGroupBox     *box;

  grid = new QGridLayout();
  setLayout(grid);

  box = new QGroupBox("Highlight Current Step");
  grid->addWidget(box,0,0);

  boxGrid = new QGridLayout();
  box->setLayout(boxGrid);

  highlightStepChild = new HighlightStepGui(highlightStepMeta);
  data->children.append(highlightStepChild);
  connect (highlightStepChild->getCheckBox(), SIGNAL(clicked(bool)), this, SLOT(enableControls(bool)));
  connect (highlightStepChild->getCheckBox(), SIGNAL(clicked(bool)), this, SLOT(reloadModelFile(bool)));
  boxGrid->addWidget(highlightStepChild,0,0);

  box = new QGroupBox("Highlight Current Step Setup");
  grid->addWidget(box,1,0);

  boxGrid = new QGridLayout();
  box->setLayout(boxGrid);

  highlightStepSetupChild = new CheckBoxGui("Setup Highlight Current Step",&highlightStepMeta->setup);
  highlightStepSetupChild->setToolTip(tr("Setup highlight step. Check to enable highlight current step locally."));
  data->children.append(highlightStepSetupChild);
  connect (highlightStepSetupChild->getCheckBox(), SIGNAL(clicked(bool)), this, SLOT(reloadModelFile(bool)));
  boxGrid->addWidget(highlightStepSetupChild,0,0);

  box = new QGroupBox("Final Model Step");
  grid->addWidget(box,2,0);

  boxGrid = new QGridLayout();
  box->setLayout(boxGrid);
  box->setToolTip("Automatically, append an un-faded and/or un-highlighted final step "
                  "to the top level model file. This step will not be saved.");

  finalModelEnabledChild = new FinalModelEnabledGui("Enable Final Model Step",&lpubMeta->finalModelEnabled);
  data->children.append(finalModelEnabledChild);
  connect (finalModelEnabledChild->getCheckBox(), SIGNAL(clicked(bool)), this, SLOT(reloadModelFile(bool)));
  boxGrid->addWidget(finalModelEnabledChild,0,0);

  emit highlightStepChild->getCheckBox()->clicked(highlightStepChild->getCheckBox()->isChecked());

  QDialogButtonBox *buttonBox;

  buttonBox = new QDialogButtonBox();
  buttonBox->addButton(QDialogButtonBox::Ok);
  connect(buttonBox,SIGNAL(accepted()),SLOT(accept()));
  buttonBox->addButton(QDialogButtonBox::Cancel);
  connect(buttonBox,SIGNAL(rejected()),SLOT(cancel()));

  grid->addWidget(buttonBox);

  setModal(true);
  setMinimumSize(300,20);
}

void GlobalHighlightStepDialog::enableControls(bool b)
{
    highlightStepSetupChild->setEnabled(!b);
    finalModelEnabledChild->setEnabled(b);
}

void GlobalHighlightStepDialog::reloadModelFile(bool b)
{
    if (!data->reloadFile)
        data->reloadFile = b;
}

void GlobalHighlightStepDialog::getHighlightStepGlobals(
  QString topLevelFile, Meta &meta)
{
  GlobalHighlightStepDialog *dialog = new GlobalHighlightStepDialog(topLevelFile, meta);
  dialog->exec();
}

void GlobalHighlightStepDialog::accept()
{
  MetaItem mi;

  mi.beginMacro("GlobalHighlightStep");

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

void GlobalHighlightStepDialog::cancel()
{
  QDialog::reject();
}

