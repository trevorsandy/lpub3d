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
 * This file implements a dialog displays an LDraw color selector.
 *
 * Please see lpub.h for an overall description of how the files in LPub
 * make up the LPub program.
 *
 ***************************************************************************/

#ifndef LDRAWCOLORDIALOG_H
#define LDRAWCOLORDIALOG_H

#include <QDialog>

namespace Ui {
class LDrawColorDialog;
}

class LDrawColorDialog : public QDialog
{
  Q_OBJECT
public:
    explicit LDrawColorDialog(
    const QColor &initial,
    QWidget *parent = nullptr);

    ~LDrawColorDialog();

    static QColor getLDrawColor(
        const QColor &initial,
        QWidget *parent = nullptr);

    bool mModified;
    int mInitialIndex;
    QColor mLDrawColor;

public slots:
  void accept();
  void cancel();

private slots:
  void colorChanged(int colorIndex);

private:
  Ui::LDrawColorDialog *ui;
};

#endif // LDRAWCOLORDIALOG_H
