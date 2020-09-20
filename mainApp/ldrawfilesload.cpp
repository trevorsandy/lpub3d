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

#include "ldrawfilesload.h"
#include "ui_ldrawfilesloaddialog.h"
#include <QStandardItemModel>
#include <QItemSelectionModel>
#include <QHeaderView>
#include <QApplication>
#include <QClipboard>
#include <QMimeData>
#include <QKeyEvent>
#include "name.h"
#include "version.h"

QVariant LdrawFilesLoadModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
        switch (section) {
        case 0:
            return QString("Part");
        case 1:
            return QString("Description");
        }
    }
    return QVariant();
}

LdrawFilesLoad::LdrawFilesLoad(QStringList &stringList,QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LdrawFilesLoadDialog),
    lm(new LdrawFilesLoadModel(this))
{
    ui->setupUi(this);

    setWindowTitle("LDrawfiles load");

    QStringList _loadedParts = stringList;

    QString status = _loadedParts.last();
    _loadedParts.removeLast();

    ui->messagesLabel->setText(status);
    ui->messagesLabel->adjustSize();

    auto getCount = [&_loadedParts] (const LoadMsgType lmt)
    {
        int count = 0;
        for (QString part : _loadedParts)
        {
            if (part.startsWith(int(lmt)))
                count++;
        }
        return count;
    };

    auto prepareRow = [] (const QString &item,const QString &desc)
    {
        QList<QStandardItem *> row = { new QStandardItem(item),new QStandardItem(desc) };
        return row;
    };

    auto setChildItems = [&_loadedParts, &prepareRow] (const LoadMsgType lmt, QList<QStandardItem *>parentItem)
    {
        for (QString part : _loadedParts)
        {
            if (part.startsWith(int(lmt))) {
                QStringList components = part.split("|");
                QList<QStandardItem *>childItem = prepareRow(components.at(1),components.at(2));
                parentItem.first()->appendRow(childItem);
            }
        }
    };

    QStandardItem *rootNode = lm->invisibleRootItem();
    if (getCount(MISSING_LOAD_MSG)) {
        QList<QStandardItem *>missingItem = prepareRow("Error - Missing Parts","");
        rootNode->appendRow(missingItem);
        setChildItems(MISSING_LOAD_MSG, missingItem);
        ui->buttonBox->addButton(QDialogButtonBox::Abort);
    }
    if (getCount(VALID_LOAD_MSG)) {
        QList<QStandardItem *>validItem = prepareRow("Validated Parts","");
        rootNode->appendRow(validItem);
        setChildItems(VALID_LOAD_MSG, validItem);
    }
    if (getCount(PRIMITIVE_LOAD_MSG)) {
        QList<QStandardItem *>primitiveItem = prepareRow("Warning - Primitive Parts","");
        rootNode->appendRow(primitiveItem);
        setChildItems(PRIMITIVE_LOAD_MSG, primitiveItem);
    }
    if (getCount(SUBPART_LOAD_MSG)) {
        QList<QStandardItem *>subpartItem = prepareRow("Warning - Sublevel Parts","");
        rootNode->appendRow(subpartItem);
        setChildItems(SUBPART_LOAD_MSG, subpartItem);
    }

    ui->messagesView->setModel(lm);
    ui->messagesView->expandAll();
    ui->messagesView->setSortingEnabled(true);
    ui->messagesView->sortByColumn(0);
    ui->messagesView->setSelectionMode( QAbstractItemView::MultiSelection );
    ui->messagesView->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->messagesView->adjustSize();

    contextMenu = new QMenu(ui->messagesView);
    copyAct = new QAction(tr("Copy - Ctrl+C"),contextMenu);
    ui->messagesView->setContextMenuPolicy(Qt::ActionsContextMenu);
    ui->messagesView->addAction(copyAct);
    connect(copyAct, SIGNAL(triggered()), this, SLOT(copy()));

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

    qSort(indexes);

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

void LdrawFilesLoad::keyPressEvent(QKeyEvent * event)
{
    if(event->matches(QKeySequence::Copy) ||
      (event->key() == Qt::Key_C && (event->modifiers() & Qt::ControlModifier)))
        copy();
    else
        QDialog::keyPressEvent(event);
}

int LdrawFilesLoad::showLoadMessages(
  QStringList &stringList)
{
  LdrawFilesLoad *dlg = new LdrawFilesLoad(stringList);
  return dlg->exec();
}
