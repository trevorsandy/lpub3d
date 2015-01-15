
/****************************************************************************
**
** Copyright (C) 2007-2009 Kevin Clague. All rights reserved.
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

#include <QString>
#include <QWidget>
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

  GlobalFadeStepPrivate(QString &_topLevelFile, Meta &_meta)
  {
    topLevelFile = _topLevelFile;
    meta = _meta;

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

  setWindowTitle(tr("Fade Step Globals Setup"));

  QGridLayout   *grid;
  QGridLayout   *boxGrid;
  QGroupBox     *box;
  MetaGui       *child;

  grid = new QGridLayout(this);
  setLayout(grid);

  box = new QGroupBox("Select Fade Color",this);
  grid->addWidget(box,0,0);
  boxGrid = new QGridLayout(this);
  box->setLayout(boxGrid);

  child = new FadeStepGui("Color",fadeStepMeta,NULL);
  data->children.append(child);
  boxGrid->addWidget(child,0,0,1,2);

  box = new QGroupBox("Enable Fade Step",this);
  grid->addWidget(box,1,0);
  boxGrid = new QGridLayout(this);
  box->setLayout(boxGrid);

  child = new CheckBoxGui("Enabled",&fadeStepMeta->fadeStep,NULL);
  child->setDisabled(true); //disabled - property set from properties form
  data->children.append(child);
  boxGrid->addWidget(child,1,0,1,2);

  QDialogButtonBox *buttonBox;

  buttonBox = new QDialogButtonBox(this);
  buttonBox->addButton(QDialogButtonBox::Ok);
  connect(buttonBox,SIGNAL(accepted()),SLOT(accept()));
  buttonBox->addButton(QDialogButtonBox::Cancel);
  connect(buttonBox,SIGNAL(rejected()),SLOT(cancel()));

  grid->addWidget(buttonBox);

  resize(375,200);

  setModal(true);
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

  foreach(child,data->children) {
    child->apply(data->topLevelFile);
  }

  mi.endMacro();

  if (Preferences::enableFadeStep)
      Step::isCsiDataModified = true;

  QDialog::accept();
}

void GlobalFadeStepDialog::cancel()
{
  QDialog::reject();
}

