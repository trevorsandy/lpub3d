
/****************************************************************************
**
** Copyright (C) 2015 - 2023 Trevor SANDY. All rights reserved.
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
#include "commonmenus.h"

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

  setWhatsThis(lpubWT(WT_NUM_ENTRIES,windowTitle()));

  setWhatsThis(lpubWT(WT_SETUP_HIGHLIGHT_STEP,windowTitle()));

  QVBoxLayout *layout = new QVBoxLayout(this);
  GlobalSizeWidget sw(QSize(40,50), QSize(40,40));
  layout->addWidget(&sw);
  setLayout(layout);

  QGroupBox *box = new QGroupBox(tr("Highlight Current Step"));
  layout->addWidget(box);
  highlightStepChild = new HighlightStepGui(highlightStepMeta,box);
  data->children.append(highlightStepChild);
  connect (highlightStepChild->getCheckBox(), SIGNAL(clicked(bool)), this, SLOT(enableControls(bool)));
  connect (highlightStepChild->getCheckBox(), SIGNAL(clicked(bool)), this, SLOT(reloadDisplayPage(bool)));

  box = new QGroupBox(tr("Highlight Current Step Setup"));
  box->setWhatsThis(lpubWT(WT_SETUP_HIGHLIGHT_STEP_SETUP,box->title()));
  layout->addWidget(box);
  highlightStepSetupChild = new CheckBoxGui(tr("Setup Highlight Current Step"),&highlightStepMeta->setup,box);
  highlightStepSetupChild->setToolTip(tr("Setup highlight step. Check to enable highlight current step locally."));
  data->children.append(highlightStepSetupChild);
  connect (highlightStepSetupChild->getCheckBox(), SIGNAL(clicked(bool)), this, SLOT(reloadDisplayPage(bool)));

  box = new QGroupBox(tr("Final Model Step"));
  layout->addWidget(box);

  finalModelEnabledChild = new FinalModelEnabledGui(tr("Enable Final Model Step"),&lpubMeta->finalModelEnabled,box);
  finalModelEnabledChild->setToolTip(tr("Automatically, append an un-faded and/or un-highlighted final step "
                                        "to the top level model file. This step will not be saved."));
  data->children.append(finalModelEnabledChild);
  connect (finalModelEnabledChild->getCheckBox(), SIGNAL(clicked(bool)), this, SLOT(reloadDisplayPage(bool)));

  emit highlightStepChild->getCheckBox()->clicked(highlightStepChild->getCheckBox()->isChecked());

  QDialogButtonBox *buttonBox = new QDialogButtonBox();
  buttonBox->addButton(QDialogButtonBox::Ok);
  connect(buttonBox,SIGNAL(accepted()),SLOT(accept()));
  buttonBox->addButton(QDialogButtonBox::Cancel);
  connect(buttonBox,SIGNAL(rejected()),SLOT(cancel()));
  layout->addWidget(buttonBox);

  setModal(true);
}

void GlobalHighlightStepDialog::enableControls(bool b)
{
    highlightStepSetupChild->setEnabled(!b);
    finalModelEnabledChild->setEnabled(b);
}

void GlobalHighlightStepDialog::reloadDisplayPage(bool b)
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

void GlobalHighlightStepDialog::cancel()
{
  QDialog::reject();
}

