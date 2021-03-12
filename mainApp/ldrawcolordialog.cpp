/****************************************************************************
**
** Copyright (C) 2019 - 2021 Trevor SANDY. All rights reserved.
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
 * This file implements a dialog displays an LDraw color selector.
 *
 * Please see lpub.h for an overall description of how the files in LPub
 * make up the LPub program.
 *
 ***************************************************************************/

#include "ldrawcolordialog.h"
#include "ui_ldrawcolordialog.h"
#include "lc_global.h"
#include "lc_qcolorlist.h"
#include "color.h"

LDrawColorDialog::LDrawColorDialog(
    const int &initial,
    QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LDrawColorDialog)
{
     ui->setupUi(this);

     setWindowTitle("LDraw Color Selector");

     QGridLayout* ColorLayout = new QGridLayout(ui->colorFrame);
     ColorLayout->setContentsMargins(0, 0, 0, 0);

     lcQColorList* ColorList = new lcQColorList(ui->colorFrame);
     ColorLayout->addWidget(ColorList);
     connect(ColorList, SIGNAL(colorChanged(int)), this, SLOT(colorChanged(int)));

     mInitialIndex = lcGetColorIndex(quint32(initial));
     ColorList->setCurrentColor(mInitialIndex);

     mSelectedIndex = -1;
     mModified = false;

     setMinimumSize(40, 40);
}

LDrawColorDialog::~LDrawColorDialog()
{
    delete ui;
}

QColor LDrawColorDialog::getLDrawColor(
  const int &initial,
        int &colorIndex,
        QWidget *parent)  {

    LDrawColorDialog *dialog = new LDrawColorDialog(initial,parent);
    bool ok;
    ok = dialog->exec() == QDialog::Accepted;
    if (ok) {
      colorIndex = dialog->mSelectedIndex;
      return dialog->mLDrawColor;
    }
    colorIndex = lcGetColorIndex(initial);
    return initial;
}

QColor LDrawColorDialog::getLDrawColor(
    const int &initial,
      QWidget *parent)
{
    int unusedIndex;
    return getLDrawColor(initial, unusedIndex, parent);
}

void LDrawColorDialog::colorChanged(int colorIndex)
{
    mModified = colorIndex != mInitialIndex;
    if (mModified) {
        lcColor* color = &gColorList[colorIndex];
        QColor rgb(color->Value[0] * 255, color->Value[1] * 255, color->Value[2] * 255);
        mLDrawColor = rgb;
        mSelectedIndex = colorIndex;
    }
}

void LDrawColorDialog::accept()
{
  if (mModified) {
    QDialog::accept();
  } else {
    QDialog::reject();
  }
}

void LDrawColorDialog::cancel()
{
  QDialog::reject();
}
