/****************************************************************************
**
** Copyright (C) 2021 Trevor SANDY. All rights reserved.
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
 * This file implements a dialog that allows the user to select the
 * fade previous steps or highlight current step settings.
 *
 * Please see lpub.h for an overall description of how the files in LPub
 * make up the LPub program.
 *
 ***************************************************************************/

#include <QVBoxLayout>
#include <QGroupBox>
#include <QDialogButtonBox>

#include "fadehighlightdialog.h"
#include "metagui.h"

FadeHighlightDialog::FadeHighlightDialog(
  FadeStepMeta      &_fadeGoods,
  HighlightStepMeta &_highlightGoods,
  UseMeta            _useMeta,
  QString            _name,
  QWidget            *parent)
  : QDialog(parent)
{
  setWindowTitle(_name);

  useMeta = _useMeta;

  if (_useMeta == UseFadeMeta)
    fadeMeta = _fadeGoods;
  else
    highlightMeta = _highlightGoods;

  QVBoxLayout *layout = new QVBoxLayout(this);
  setLayout(layout);

  QGroupBox *box = new QGroupBox(_name,this);
  layout->addWidget(box);

  if (_useMeta == UseFadeMeta)
    fadeSteps = new FadeStepGui(&fadeMeta,false/*global*/,box);
  else
    highlightStep = new HighlightStepGui(&highlightMeta,false/*global*/,box);

  QDialogButtonBox *buttonBox;

  buttonBox = new QDialogButtonBox(this);

  buttonBox->addButton(QDialogButtonBox::Ok);
  connect(buttonBox,SIGNAL(accepted()),SLOT(accept()));
  buttonBox->addButton(QDialogButtonBox::Cancel);
  connect(buttonBox,SIGNAL(rejected()),SLOT(cancel()));

  layout->addWidget(buttonBox);

  setModal(true);
  setMinimumSize(300,20);
}

FadeHighlightDialog::~FadeHighlightDialog()
{
}

bool FadeHighlightDialog::getFadeHighlight(
  FadeStepMeta      &fadeGoods,
  HighlightStepMeta &highlightGoods,
  UseMeta            useMeta,
  QString            name,
  QWidget           *parent)
{
  FadeHighlightDialog *dialog = new FadeHighlightDialog(fadeGoods,highlightGoods,useMeta,name,parent);

  bool ok = dialog->exec() == QDialog::Accepted;
  if (ok) {
    if (useMeta == UseFadeMeta)
      fadeGoods = dialog->fadeMeta;
    else
      highlightGoods = dialog->highlightMeta;
  }
  return ok;
}

void FadeHighlightDialog::accept()
{
  if (useMeta == UseFadeMeta ? fadeSteps->modified : highlightStep->modified) {
    QDialog::accept();
  } else {
    QDialog::reject();
  }
}

void FadeHighlightDialog::cancel()
{
  QDialog::reject();
}
