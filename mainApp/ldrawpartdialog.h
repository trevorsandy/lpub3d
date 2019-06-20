/****************************************************************************
**
** Copyright (C) 2019 Trevor SANDY. All rights reserved.
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

#ifndef LDRAWPARTDIALOG_H
#define LDRAWPARTDIALOG_H

#include <QDialog>

namespace Ui {
class LDrawPartDialog;
}

class PieceInfo;
class lcPartSelectionWidget;
class LDrawPartDialog : public QDialog
{
// Causes Unresolved external symbol "public: virtual struct QMetaObject const * __thiscall error...
// See - https://stackoverflow.com/questions/14170770/unresolved-external-symbol-public-virtual-struct-qmetaobject-const-thiscal
//  Q_OBJECT
public:
    explicit LDrawPartDialog(
    QString const partKey,  // part;colour key
    QWidget *parent = nullptr);

    ~LDrawPartDialog();

    static PieceInfo* getLDrawPart(
        QString const partKey,
        QWidget *parent = nullptr);

public slots:
  void accept();
  void cancel();

private slots:

private:
  Ui::LDrawPartDialog   *ui;
  lcPartSelectionWidget *mPartSelectionWidget;
  PieceInfo             *mLDrawPart;
};

#endif // LDRAWPARTDIALOG_H
