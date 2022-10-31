
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

  setWindowTitle(tr("Fade Steps Globals Setup"));

  setWhatsThis(lpubWT(WT_SETUP_FADE_STEPS,windowTitle()));

  QVBoxLayout *layout = new QVBoxLayout(this);
  setLayout(layout);

  QGroupBox *box = new QGroupBox(tr("Fade Previous Steps"));
  layout->addWidget(box);
  fadeStepChild = new FadeStepGui(fadeStepMeta,box);
  fadeStepChild->setToolTip(tr("Enable fade previous steps"));
  data->children.append(fadeStepChild);
  connect (fadeStepChild->getCheckBox(), SIGNAL(clicked(bool)), this, SLOT(enableControls(bool)));
  connect (fadeStepChild->getCheckBox(), SIGNAL(clicked(bool)), this, SLOT(reloadDisplayPage(bool)));

  box = new QGroupBox(tr("Fade Previous Steps Setup"));
  box->setWhatsThis(lpubWT(WT_SETUP_FADE_STEPS_SETUP,box->title()));
  layout->addWidget(box);
  fadeStepSetupChild = new CheckBoxGui(tr("Setup Fade Previous Steps"),&fadeStepMeta->setup,box);
  fadeStepSetupChild->setToolTip(tr("Setup fade steps. Check to enable fade previous steps locally."));
  data->children.append(fadeStepSetupChild);
  connect (fadeStepSetupChild->getCheckBox(), SIGNAL(clicked(bool)), this, SLOT(reloadDisplayPage(bool)));

  box = new QGroupBox("Final Model Step");
  layout->addWidget(box);
  finalModelEnabledChild = new FinalModelEnabledGui(tr("Enable Final Model Step"),&lpubMeta->finalModelEnabled,box);
  finalModelEnabledChild->setToolTip(tr("Automatically, append an un-faded final step to the top level model file."));
  data->children.append(finalModelEnabledChild);
  connect (finalModelEnabledChild->getCheckBox(), SIGNAL(clicked(bool)), this, SLOT(reloadDisplayPage(bool)));

  emit fadeStepChild->getCheckBox()->clicked(fadeStepChild->getCheckBox()->isChecked());

  QDialogButtonBox *buttonBox = new QDialogButtonBox();
  buttonBox->addButton(QDialogButtonBox::Ok);
  connect(buttonBox,SIGNAL(accepted()),SLOT(accept()));
  buttonBox->addButton(QDialogButtonBox::Cancel);
  connect(buttonBox,SIGNAL(rejected()),SLOT(cancel()));
  layout->addWidget(buttonBox);

  setModal(true);
  setMinimumSize(40,20);
}

void GlobalFadeStepDialog::enableControls(bool b)
{
    fadeStepSetupChild->setEnabled(!b);
    finalModelEnabledChild->setEnabled(b);
}

void GlobalFadeStepDialog::reloadDisplayPage(bool b)
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

  bool noFileDisplay = false;

  MetaGui *child;
  Q_FOREACH (child,data->children) {
    child->apply(data->topLevelFile);
    noFileDisplay |= child->modified;
  }

  if (data->reloadFile)
    mi.setSuspendFileDisplayFlag(true);
  else
    mi.setSuspendFileDisplayFlag(!noFileDisplay);

  mi.endMacro();

  if (data->reloadFile)
    mi.clearAndReloadModelFile(false/*closeAndOpen*/, true/*savePrompt*/);

  QDialog::accept();
}

void GlobalFadeStepDialog::cancel()
{
  QDialog::reject();
}

