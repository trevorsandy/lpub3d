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
 * This file implements a dialog that allows the user review the LDrawFiles
 * load results
 *
 * Please see lpub.h for an overall description of how the files in LPub
 * make up the LPub program.
 *
 ***************************************************************************/

#ifndef LDRAWFILESLOAD_H
#define LDRAWFILESLOAD_H

#include <QDialog>
#include <QStandardItemModel>
#include <QItemSelection>
#include <QMenu>

namespace Ui {
class LdrawFilesLoadDialog;
}

class LdrawFilesLoadModel : public QStandardItemModel
{
    Q_OBJECT
public:
    LdrawFilesLoadModel(QObject *parent = nullptr)
        : QStandardItemModel(parent){}
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
};

class LdrawFilesLoad : public QDialog
{
    Q_OBJECT
public:
    explicit LdrawFilesLoad(QStringList &loadItems, QWidget *parent = nullptr);
    ~LdrawFilesLoad();
    static int showLoadMessages(QStringList &stringList);
private slots:
    void keyPressEvent(QKeyEvent * event);
    void copy();
private:
    Ui::LdrawFilesLoadDialog *ui;
    LdrawFilesLoadModel *lm;
    QMenu *contextMenu;
    QAction *copyAct;
};

#endif // LDRAWFILESLOAD_H
