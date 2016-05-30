 
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
 * This file implements a dialog that allows the user to change the background
 * of page, callout and PLIs.
 *
 * Please see lpub.h for an overall description of how the files in LPub
 * make up the LPub program.
 *
 ***************************************************************************/

#include <QVBoxLayout>
#include <QGroupBox>
#include <QDialogButtonBox>

#include "backgrounddialog.h"
#include "metagui.h"

BackgroundDialog::BackgroundDialog(
  BackgroundData &goods,
  QString     _name,
  QWidget    *parent)
  : QDialog(parent)
{
  setWindowTitle(_name);

  meta.setValue(goods);
  QVBoxLayout *layout = new QVBoxLayout(this);
  setLayout(layout);

  QGroupBox *box = new QGroupBox("Background",this);
  layout->addWidget(box);
  background = new BackgroundGui(&meta,box);

  QDialogButtonBox *buttonBox;
  
  buttonBox = new QDialogButtonBox(this);

  buttonBox->addButton(QDialogButtonBox::Ok);
  connect(buttonBox,SIGNAL(accepted()),SLOT(accept()));
  buttonBox->addButton(QDialogButtonBox::Cancel);
  connect(buttonBox,SIGNAL(rejected()),SLOT(cancel()));

  layout->addWidget(buttonBox);

  setModal(true);
}

BackgroundDialog::~BackgroundDialog()
{
}

bool BackgroundDialog::getBackground(
  BackgroundData &goods,
  QString         name,
  QWidget        *parent)
{
  BackgroundDialog *dialog = new BackgroundDialog(goods,name,parent);

  bool ok = dialog->exec() == QDialog::Accepted;
  if (ok) {
    goods = dialog->meta.value();
  }
  return ok;
}

void BackgroundDialog::accept()
{
  if (background->modified) {
    QDialog::accept();
  } else {
    QDialog::reject();
  }
}

void BackgroundDialog::cancel()
{
  QDialog::reject();
}
