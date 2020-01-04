
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

  GlobalHighlightStepPrivate(QString &_topLevelFile, Meta &_meta)
  {
    topLevelFile = _topLevelFile;
    meta = _meta;

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

  setWindowTitle(tr("Highlight Step Globals Setup"));

  QGridLayout   *grid;
  QGridLayout   *boxGrid;
  QGroupBox     *box;
  MetaGui       *child;
  QLabel        *label;
  QSpinBox      *spinbox;

  grid = new QGridLayout();
  setLayout(grid);

  box = new QGroupBox("Highlight Current Step (read only)");
  grid->addWidget(box,0,0);

  boxGrid = new QGridLayout();
  box->setLayout(boxGrid);

  child = new HighlightStepGui("Highlight Color",highlightStepMeta);
  data->children.append(child);
  boxGrid->addWidget(child,0,0);

  box = new QGroupBox("Highlight Settings (read only)");
  grid->addWidget(box,1,0);

  boxGrid = new QGridLayout();
  box->setLayout(boxGrid);

  child = new CheckBoxGui("Enabled",&highlightStepMeta->highlightStep);
  child->setDisabled(true);
  data->children.append(child);
  boxGrid->addWidget(child,0,0);

  label = new QLabel();
  boxGrid->addWidget(label,0,1);

  label = new QLabel();
  label->setText("Set from Preferences dialog");
  label->setDisabled(true);
  boxGrid->addWidget(label,0,2);

  spinbox = new QSpinBox();
  spinbox->setValue(highlightStepMeta->highlightLineWidth.value());
  spinbox->setDisabled(true);
  boxGrid->addWidget(spinbox,1,0);

  label = new QLabel();
  label->setText("Line width");
  label->setDisabled(true);
  boxGrid->addWidget(label,1,1);

  label = new QLabel();
  label->setText("Set from Preferences dialog");
  label->setDisabled(true);
  boxGrid->addWidget(label,1,2);

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

  foreach(child,data->children) {
    child->apply(data->topLevelFile);
  }

  mi.endMacro();

  QDialog::accept();
}

void GlobalHighlightStepDialog::cancel()
{
  QDialog::reject();
}

