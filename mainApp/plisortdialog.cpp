/****************************************************************************
**
** Copyright (C) 2016 Trevor SANDY. All rights reserved.
**
** This file may be used under the terms of the
** GNU General Public Liceense (GPL) version 3.0
** which accompanies this distribution, and is
** available at http://www.gnu.org/licenses/gpl.html
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

/****************************************************************************
 *
 * This class implements a simple dialog that lets the user specify sort
 * options for the parts list images.
 *
 * Please see lpub.h for an overall description of how the files in LPub
 * make up the LPub program.
 *
 ***************************************************************************/

#include "plisortdialog.h"
#include <QVBoxLayout>
#include <QGroupBox>
#include <QDialogButtonBox>
#include "metagui.h"

PliSortDialog::PliSortDialog(
  QString       &goods,
  QString       _name,
  QWidget       *parent)
  : QDialog(parent)
{
  setWindowTitle(_name);

  meta.setValue(goods);

  QVBoxLayout *layout = new QVBoxLayout(this);
  setLayout(layout);

  QGroupBox *box = new QGroupBox(_name,this);
  layout->addWidget(box);
  pliSortBy = new PliSortGui("",&meta,box);

  QDialogButtonBox *buttonBox;

  buttonBox = new QDialogButtonBox(this);
  buttonBox->addButton(QDialogButtonBox::Ok);
  connect(buttonBox,SIGNAL(accepted()),SLOT(accept()));
  buttonBox->addButton(QDialogButtonBox::Cancel);
  connect(buttonBox,SIGNAL(rejected()),SLOT(cancel()));

  layout->addWidget(buttonBox);

  setModal(true);
}

PliSortDialog::~PliSortDialog()
{
}

bool PliSortDialog::getPliSortOption(
  QString        &goods,
  QString         name,
  QWidget        *parent)
{
  PliSortDialog *dialog = new PliSortDialog(goods,name,parent);

  bool ok = dialog->exec() == QDialog::Accepted;

  if (ok) {
    goods = dialog->meta.value();
  }
  return ok;
}

void PliSortDialog::accept()
{
  if (pliSortBy->modified) {
    QDialog::accept();
  } else {
    QDialog::reject();
  }
}

void PliSortDialog::cancel()
{
  QDialog::reject();
}

