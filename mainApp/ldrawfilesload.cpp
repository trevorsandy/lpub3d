/****************************************************************************
**
** Copyright (C) 2019 - 2022 Trevor SANDY. All rights reserved.
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

#include "ldrawfilesload.h"
#include "ui_ldrawfilesloaddialog.h"
#include <QStandardItemModel>
#include <QItemSelectionModel>
#include <QHeaderView>
#include <QApplication>
#include <QClipboard>
#include <QMimeData>
#include <QKeyEvent>
#include <QDebug>

#include "declarations.h"
#include "lpub_qtcompat.h"
#include "commonmenus.h"
#include "version.h"

QVariant LdrawFilesLoadModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
        switch (section) {
        case 0:
            return tr("Part");
        case 1:
            return tr("Description");
        }
    }
    return QVariant();
}

LdrawFilesLoad::LdrawFilesLoad(const QStringList &stringList, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LdrawFilesLoadDialog),
    lm(new LdrawFilesLoadModel(this))
{
    ui->setupUi(this);

    setWindowTitle(tr("LDraw File Load Status"));

    setWhatsThis(lpubWT(WT_DIALOG_LDRAW_FILES_LOAD,windowTitle()));

    QStringList _loadedParts = stringList;

    QString status = _loadedParts.size() ? _loadedParts.takeLast() : tr("No loaded parts received.");

    ui->messagesLabel->setText(status);
    if (_loadedParts.isEmpty())
        ui->messagesLabel->setStyleSheet("QLabel { color: red; }");
    ui->messagesLabel->adjustSize();

    QColor errorColor = QColor(Qt::red);
    QBrush errorBrush (errorColor);

    QColor warningColor = QColor(Qt::darkYellow);
    QBrush warningBrush (warningColor);

    auto getCount = [&_loadedParts] (const LoadMsgType lmt)
    {
        int count = 0;
        for (const QString &part : _loadedParts)
        {
            if (part.startsWith(int(lmt)))
                count++;
        }
        return count;
    };

    auto prepareRow = [] (const QString &column01,const QString &column02)
    {
        QList<QStandardItem *> row = { new QStandardItem(column01), new QStandardItem(column02) };
        return row;
    };

    auto setChildItems = [&] (const LoadMsgType lmt, QList<QStandardItem *>parentRow)
    {
        for (const QString &part : _loadedParts)
        {
            if (part.startsWith(int(lmt))) {
                QStringList columns = part.split("|");
                QList<QStandardItem *>childRow = prepareRow(columns.at(1),columns.at(2));
                if (lmt == MISSING_LOAD_MSG) {
                    QStandardItem *part = childRow.at(0);
                    part->setForeground(errorBrush);
                    QStandardItem *desc = childRow.at(1);
                    desc->setForeground(errorBrush);
                } else if (lmt != VALID_LOAD_MSG) {
                    QStandardItem *part = childRow.at(0);
                    part->setForeground(warningBrush);
                    QStandardItem *desc = childRow.at(1);
                    desc->setForeground(errorBrush);
                }
                parentRow.first()->appendRow(childRow);
            }
        }
    };

    QStandardItem *rootNode = lm->invisibleRootItem();
    if (getCount(MISSING_LOAD_MSG)) {
        QList<QStandardItem *>missingRow = prepareRow(tr("Error - Missing Parts"),"");
        QStandardItem *header = missingRow.at(0);
        header->setForeground(errorBrush);
        rootNode->appendRow(missingRow);
        setChildItems(MISSING_LOAD_MSG, missingRow);
        ui->buttonBox->addButton(QDialogButtonBox::Discard);
        connect(ui->buttonBox,SIGNAL(clicked(QAbstractButton*)),SLOT(getButton(QAbstractButton*)));
    }
    if (getCount(VALID_LOAD_MSG)) {
        QList<QStandardItem *>validRow = prepareRow(tr("Validated Parts"),"");
        rootNode->appendRow(validRow);
        setChildItems(VALID_LOAD_MSG, validRow);
    }
    if (getCount(PRIMITIVE_LOAD_MSG)) {
        QList<QStandardItem *>primitiveRow = prepareRow(tr("Warning - Primitive Parts"),"");
        QStandardItem *header = primitiveRow.at(0);
        header->setForeground(warningBrush);
        rootNode->appendRow(primitiveRow);
        setChildItems(PRIMITIVE_LOAD_MSG, primitiveRow);
    }
    if (getCount(SUBPART_LOAD_MSG)) {
        QList<QStandardItem *>subpartRow = prepareRow(tr("Warning - Sublevel Parts"),"");
        QStandardItem *header = subpartRow.at(0);
        header->setForeground(warningBrush);
        rootNode->appendRow(subpartRow);
        setChildItems(SUBPART_LOAD_MSG, subpartRow);
    }

    ui->messagesView->setModel(lm);
    ui->messagesView->expandAll();
    ui->messagesView->setSortingEnabled(true);
    ui->messagesView->sortByColumn(0, Qt::AscendingOrder);
    ui->messagesView->setSelectionMode( QAbstractItemView::MultiSelection );
    ui->messagesView->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->messagesView->adjustSize();

    contextMenu = new QMenu(ui->messagesView);
    copyAct = new QAction(tr("Copy"),contextMenu);
    copyAct->setShortcut(QKeySequence::Copy);
    ui->messagesView->setContextMenuPolicy(Qt::ActionsContextMenu);
    ui->messagesView->addAction(copyAct);
    connect(copyAct, SIGNAL(triggered()), this, SLOT(copy()));

    setMinimumSize(100, 100);

    setModal(true);
}

LdrawFilesLoad::~LdrawFilesLoad()
{
    delete ui;
    delete lm;
}

void LdrawFilesLoad::copy()
{
    QItemSelectionModel * selection = ui->messagesView->selectionModel();
    QModelIndexList indexes = selection->selectedIndexes();

    if(indexes.size() < 1)
        return;

    lpub_sort(indexes.begin(), indexes.end());

    // You need a pair of indexes to find the row changes
    QModelIndex previous = indexes.first();
    indexes.removeFirst();
    QString selected_text;
    QModelIndex current;
    Q_FOREACH(current, indexes)
    {
        QVariant data = lm->data(previous);
        QString text = data.toString();
        selected_text.append(text);

        // If you are at the start of the row the row number of the previous index
        // isn't the same.  Text is followed by a row separator, which is a newline.
        if (current.row() != previous.row())
            selected_text.append(QLatin1Char('\n'));
        else // Otherwise it's the same row, so append a column separator, which is a tab.
            selected_text.append(QLatin1Char('\t'));
        previous = current;
    }

    // add last element
    selected_text.append(lm->data(current).toString());
    QMimeData * md = new QMimeData;

    md->setText(selected_text);
    qApp->clipboard()->setMimeData(md);
}

void LdrawFilesLoad::getButton(QAbstractButton *button)
{
  QDialogButtonBox::ButtonRole role = ui->buttonBox->buttonRole(button);
  if (role == QDialogButtonBox::DestructiveRole)
    QDialog::reject();
}

void LdrawFilesLoad::keyPressEvent(QKeyEvent * event)
{
    if(event->matches(QKeySequence::Copy) ||
      (event->key() == Qt::Key_C && (event->modifiers() & Qt::ControlModifier)))
        copy();
    else
        QDialog::keyPressEvent(event);
}

int LdrawFilesLoad::showLoadMessages(const QStringList &stringList)
{
  LdrawFilesLoad *filesLoadDialog = new LdrawFilesLoad(stringList);
  return filesLoadDialog->exec();
}
