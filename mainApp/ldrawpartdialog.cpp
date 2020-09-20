/****************************************************************************
**
** Copyright (C) 2019 - 2020 Trevor SANDY. All rights reserved.
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
 * This file implements a dialog displays an LDraw part selector.
 *
 * Please see lpub.h for an overall description of how the files in LPub
 * make up the LPub program.
 *
 ***************************************************************************/

#include "ldrawpartdialog.h"
#include "ui_ldrawpartdialog.h"

#include "lc_global.h"
#include "lc_mesh.h"
#include "pieceinf.h"
#include "lc_application.h"
#include "lc_library.h"
#include "lc_mainwindow.h"
#include "lc_partselectionwidget.h"

LDrawPartDialog::LDrawPartDialog(
    const QString partKey,
    QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LDrawPartDialog)
{
     ui->setupUi(this);

     setWindowTitle("LDraw Part Selector");

     int color = partKey.split(";").last().toInt();
     QString partType = partKey.split(";").first();

     QGridLayout* PartSelectorLayout = new QGridLayout(ui->colorFrame);
     PartSelectorLayout->setContentsMargins(0, 0, 0, 0);

     mPartSelectionWidget = gMainWindow->GetPartSelectionWidget();
     mPartSelectionWidget->SetColorIndex(color);
     PartSelectorLayout->addWidget(mPartSelectionWidget);

     PieceInfo* Info = lcGetPiecesLibrary()->FindPiece(partType.toLatin1().constData(), nullptr, false, false);
     if (Info){
         lcPiecesLibrary* Library = lcGetPiecesLibrary();
         for (int categoryIndex = 0; categoryIndex < gCategories.size(); categoryIndex++)
         {
             if (Library->PieceInCategory(Info, gCategories[categoryIndex].Keywords.constData())) {
                 mPartSelectionWidget->SetCategory(categoryIndex);
                 break;
             }
         }
     }

     setMinimumSize(40, 40);
}

LDrawPartDialog::~LDrawPartDialog()
{
  delete ui;
  delete mPartSelectionWidget;
}

PieceInfo* LDrawPartDialog::getLDrawPart(
        const QString partKey,
        QWidget *parent) {

    LDrawPartDialog *dialog = new LDrawPartDialog(partKey,parent);
    bool ok;
    ok = dialog->exec() == QDialog::Accepted;
    if (ok) {
      return dialog->mLDrawPart;
    }
    return nullptr;
}

void LDrawPartDialog::accept()
{
   mLDrawPart = mPartSelectionWidget->GetCurrentPart();

  if (mLDrawPart) {
    QDialog::accept();
  } else {
    QDialog::reject();
  }
}

void LDrawPartDialog::cancel()
{
  QDialog::reject();
}
