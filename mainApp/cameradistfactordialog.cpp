/****************************************************************************
**
** Copyright (C) 2018 Trevor SANDY. All rights reserved.
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
* These dialogs provide graphical user interfaces for the Native Renderer
* CameraDistanceFactor
*
* Please see lpub.h for an overall description of how the files in LPub
* make up the LPub program.
*
***************************************************************************/

#include "cameradistfactordialog.h"
#include <QVBoxLayout>
#include <QGroupBox>
#include <QDialogButtonBox>
#include "metagui.h"

CameraDistFactorDialog::CameraDistFactorDialog(
  int      &value,
  QString  _name,
  QWidget  *parent)
  : QDialog(parent)
{
  setWindowTitle(_name);

  QVBoxLayout *layout = new QVBoxLayout(this);
  setLayout(layout);

  QGroupBox *box = new QGroupBox(_name,this);
  layout->addWidget(box);
  meta.factor.setValue(value);
  cameraDistFactor = new CameraDistFactorGui("Factor",&meta,box);

  QDialogButtonBox *buttonBox;
  buttonBox = new QDialogButtonBox(this);
  buttonBox->addButton(QDialogButtonBox::Ok);
  connect(buttonBox,SIGNAL(accepted()),SLOT(accept()));
  buttonBox->addButton(QDialogButtonBox::Cancel);
  connect(buttonBox,SIGNAL(rejected()),SLOT(cancel()));
  layout->addWidget(buttonBox);

  setModal(true);
}

CameraDistFactorDialog::~CameraDistFactorDialog()
{
}

bool CameraDistFactorDialog::getCameraDistFactor(
  int      &value,
  QString   name,
  QWidget  *parent)
{
  CameraDistFactorDialog *dialog = new CameraDistFactorDialog(value,name,parent);

  bool ok = dialog->exec() == QDialog::Accepted;

  if (ok) {
    value = dialog->meta.factor.value();
  }
  return ok;
}

void CameraDistFactorDialog::accept()
{
  if (cameraDistFactor->modified) {
    QDialog::accept();
  } else {
    QDialog::reject();
  }
}

void CameraDistFactorDialog::cancel()
{
  QDialog::reject();
}
