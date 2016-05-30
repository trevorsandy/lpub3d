 
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
 * This file describes a dialog that the users can use to change attributes
 * associated with borders.  Pages, Parts Lists and Callouts have borders.
 *
 * Please see lpub.h for an overall description of how the files in LPub
 * make up the LPub program.
 *
 ***************************************************************************/

#include "borderdialog.h"
#include <QVBoxLayout>
#include <QGroupBox>
#include <QDialogButtonBox>
#include "metagui.h"

BorderDialog::BorderDialog(
  BorderData &goods,
  QString     _name,
  bool        _rotateArrow)
{
  setWindowTitle(_name);

  meta.setValue(goods);
  QVBoxLayout *layout = new QVBoxLayout(this);
  setLayout(layout);

  QGroupBox *box = new QGroupBox("Border",this);
  layout->addWidget(box);
  border = new BorderGui(&meta,box,_rotateArrow);

  QDialogButtonBox *buttonBox;
  
  buttonBox = new QDialogButtonBox(this);

  buttonBox->addButton(QDialogButtonBox::Ok);
  connect(buttonBox,SIGNAL(accepted()),SLOT(accept()));
  buttonBox->addButton(QDialogButtonBox::Cancel);
  connect(buttonBox,SIGNAL(rejected()),SLOT(cancel()));

  layout->addWidget(buttonBox);

  setModal(true);
}

bool BorderDialog::getBorder(
  BorderData &goods,
  QString     name,
  bool        rotateArrow)
{
  BorderDialog *dialog = new BorderDialog(goods,name,rotateArrow);

  bool ok = dialog->exec() == QDialog::Accepted;
  if (ok) {
    goods = dialog->meta.value();
  }
  return ok;
}

void BorderDialog::accept()
{
  if (border->modified) {
    QDialog::accept();
  } else {
    QDialog::reject();
  }
}

void BorderDialog::cancel()
{
  QDialog::reject();
}
