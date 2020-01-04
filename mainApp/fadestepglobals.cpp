
/****************************************************************************
**
** Copyright (C) 2015 - 2020 Trevor SANDY. All rights reserved.
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
  QLabel        *label;

  grid = new QGridLayout();
  setLayout(grid);

  box = new QGroupBox("Fade Previous Step(s) (read only)");
  grid->addWidget(box,0,0);

  boxGrid = new QGridLayout();
  box->setLayout(boxGrid);

  child = new FadeStepGui("Fade Color",fadeStepMeta);
  data->children.append(child);
  boxGrid->addWidget(child,0,0);

  box = new QGroupBox("Fade Settings (read only)");
  grid->addWidget(box,1,0);

  boxGrid = new QGridLayout();
  box->setLayout(boxGrid);

  child = new CheckBoxGui("Enabled",&fadeStepMeta->fadeStep);
  child->setDisabled(true);
  data->children.append(child);
  boxGrid->addWidget(child,0,0);

  label = new QLabel();
  label->setText("Set from Preferences dialog");
  label->setDisabled(true);
  boxGrid->addWidget(label,0,1);

  child = new CheckBoxGui("Use Fade Color", &fadeStepMeta->fadeUseColor);
  child->setDisabled(true);
  boxGrid->addWidget(child,1,0);

  label = new QLabel();
  label->setText("Set from Preferences dialog");
  label->setDisabled(true);
  boxGrid->addWidget(label,1,1);

  label = new QLabel();
  label->setText(tr("   Fade Opacity: %1 %").arg(fadeStepMeta->fadeOpacity.value()));
  label->setDisabled(true);
  boxGrid->addWidget(label,2,0);

  label = new QLabel();
  label->setText("Set from Preferences dialog");
  label->setDisabled(true);
  boxGrid->addWidget(label,2,1);

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

  QDialog::accept();
}

void GlobalFadeStepDialog::cancel()
{
  QDialog::reject();
}

