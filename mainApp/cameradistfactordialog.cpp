/****************************************************************************
**
** Copyright (C) 2018 - 2020 Trevor SANDY. All rights reserved.
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
  QString   title,
  QString   heading,
  int      &goods,
  QWidget  *parent)
  : QDialog(parent)
{
  setWindowTitle(title);

  meta.factor.setValue(goods);

  QVBoxLayout *layout = new QVBoxLayout(this);
  setLayout(layout);

  QGroupBox *box = new QGroupBox(heading,this);
  layout->addWidget(box);
  cameraDistFactor = new CameraDistFactorGui(heading+" Factor",&meta,box);

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

CameraDistFactorDialog::~CameraDistFactorDialog()
{
}

bool CameraDistFactorDialog::getCameraDistFactor(
  QString   title,
  QString   heading,
  int      &value,
  QWidget  *parent)
{
  CameraDistFactorDialog *dialog = new CameraDistFactorDialog(title,heading,value,parent);

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
