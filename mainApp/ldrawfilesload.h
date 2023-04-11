/****************************************************************************
**
** Copyright (C) 2019 - 2023 Trevor SANDY. All rights reserved.
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

#include "declarations.h"

namespace Ui {
class LdrawFilesLoadDialog;
}

class QAbstractButton;
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
    explicit LdrawFilesLoad(const LoadStatus &loadStatus, bool menuAction, QWidget *parent = nullptr);
    ~LdrawFilesLoad();
    static QDialog::DialogCode showLoadStatus(const LoadStatus &loadStatus, bool menuAction);
    bool discardLoad() { return dl; }
private slots:
    void getButton(QAbstractButton *button);
    void enableActions();
    void ungroupItems();
    void groupItems();
    void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
    int countItems(const LoadMsgType lmt, const QString item = QString());
    void copy() const;
private:
    void createActions();
    void summary() const;
    void populate(bool groupItems = true);

    enum StatusEnc {MSG_TYPE, ITEM, DESC};

    Ui::LdrawFilesLoadDialog *ui;
    LdrawFilesLoadModel      *lm;
    QItemSelectionModel      *sm;
    QStandardItem            *rn;

    bool dl;
    bool const isAction;
    bool const isMpd;
    int const loadedLines;
    int const loadedSteps;
    int const subFileCount;
    int const partCount;
    int const uniquePartCount;

    QString const &modelFile;
    QString const &elapsedTime;
    QStringList const &loadedItems;

    int mpc;
    int vpc;
    int msmc;
    int lsmc;
    int ipc;
    int igpc;
    int ispc;
    int ippc;
    int esmc;
    int bifc;
    int ifc;
    /* Do not add these into the load status dialogue because they are not loaded in the LDrawFile.subfiles
    int ppc;
    int spc;
    */

    QColor includeColor;
    QBrush includeBrush;
    QColor warningColor;
    QBrush warningBrush;
    QColor errorColor;
    QBrush errorBrush;

    QAction *selAllAct;
    QAction *copyAct;
    QAction *deselectAct;
    QAction *expandAct;
    QAction *collapseAct;
    QAction *groupAct;
    QAction *ungroupAct;
};

#endif // LDRAWFILESLOAD_H
