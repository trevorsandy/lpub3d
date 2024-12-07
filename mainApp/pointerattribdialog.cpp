
/****************************************************************************
**
** Copyright (C) 2018 - 2024 Trevor SANDY. All rights reserved.
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
 * This file describes a dialog that the users can use to change attributes
 * associated with pointers.  Pages, Dividers, Assemblies and Callouts have pointers.
 *
 * Please see lpub.h for an overall description of how the files in LPub3D
 * make up the LPub3D program.
 *
 ***************************************************************************/

#include "pointerattribdialog.h"
#include <QVBoxLayout>
#include <QGroupBox>
#include <QDialogButtonBox>

#include "commonmenus.h"
#include "metagui.h"

PointerAttribDialog::PointerAttribDialog(
  PointerAttribData &goods,
  QString           _name,
  bool               isCallout)
{
  setWindowTitle(_name);

  setWhatsThis(lpubWT(WT_DIALOG_POINTER_ATTRIB,windowTitle()));

  meta.setValue(goods);
  QVBoxLayout *layout = new QVBoxLayout(this);
  setLayout(layout);

  QGroupBox *box = new QGroupBox(_name.replace("Pointer ",""),this);
  layout->addWidget(box);
  pointerAttrib = new PointerAttribGui(&meta,box,isCallout);

  QDialogButtonBox *buttonBox;

  buttonBox = new QDialogButtonBox(this);

  buttonBox->addButton(QDialogButtonBox::Ok);
  connect(buttonBox,SIGNAL(accepted()),SLOT(accept()));
  buttonBox->addButton(QDialogButtonBox::Cancel);
  connect(buttonBox,SIGNAL(rejected()),SLOT(cancel()));

  layout->addWidget(buttonBox);

  setModal(true);
  setMinimumSize(40,20);
}

bool PointerAttribDialog::getPointerAttrib(
  PointerAttribData &goods,
  QString            name,
  bool               isCallout)
{
  PointerAttribDialog *dialog = new PointerAttribDialog(goods,name,isCallout);

  bool ok = dialog->exec() == QDialog::Accepted;
  if (ok) {
    goods = dialog->meta.value();
  }
  return ok;
}

void PointerAttribDialog::accept()
{
  if (pointerAttrib->modified) {
    QDialog::accept();
  } else {
    QDialog::reject();
  }
}

void PointerAttribDialog::cancel()
{
  QDialog::reject();
}
