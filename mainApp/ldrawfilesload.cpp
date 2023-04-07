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

LdrawFilesLoad::LdrawFilesLoad(const QStringList &stringList, bool menuAction, QWidget *parent) :
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

    QStandardItem *rootNode = lm->invisibleRootItem();

    QColor warningColor = QColor(Qt::darkYellow);
    QBrush warningBrush (warningColor);

    QColor errorColor = QColor(Qt::red);
    QBrush errorBrush (errorColor);

    bool ok;

    auto getCount = [&] (const LoadMsgType lmt)
    {
        int count = 0;
        for (const QString &part : _loadedParts)
        {
            LoadMsgType mt = static_cast<LoadMsgType>(part.leftRef(part.indexOf('|')).toInt(&ok));
            if (ok && mt == lmt)
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
            LoadMsgType mt = static_cast<LoadMsgType>(part.leftRef(part.indexOf('|')).toInt(&ok));
            if (ok && mt == lmt) {
                QStringList columns = part.split("|");
                QList<QStandardItem *>childRow = prepareRow(columns.at(1),columns.at(2));
                switch (lmt) {
                    case MISSING_PART_LOAD_MSG:
                    {
                        QStandardItem *part = childRow.at(0);
                        part->setForeground(errorBrush);
                        QStandardItem *desc = childRow.at(1);
                        desc->setForeground(errorBrush);
                    }
                    break;
                    case EMPTY_SUBMODEL_LOAD_MSG:
                    case INCLUDE_FILE_LOAD_MSG:
                    /* Do not add these into the load status dialogue because they are not loaded in the LDrawFile.subfiles
                    case PRIMITIVE_LOAD_MSG:
                    case SUBPART_LOAD_MSG:
                    */
                    {
                        QStandardItem *part = childRow.at(0);
                        part->setForeground(warningBrush);
                        QStandardItem *desc = childRow.at(1);
                        desc->setForeground(warningBrush);
                    }
                    break;
                    default:
                    break;
                }
                parentRow.first()->appendRow(childRow);
            }
        }
    };

    auto setRow = [&] (LoadMsgType msgType, QString const &description)
    {
        switch (msgType)
        {
        case VALID_LOAD_MSG:
        case MPD_SUBMODEL_LOAD_MSG:
        case LDR_SUBMODEL_LOAD_MSG:
        case INLINE_PART_LOAD_MSG:
        case INLINE_SUBPART_LOAD_MSG:
        case INLINE_PRIMITIVE_LOAD_MSG:
        {
            QList<QStandardItem *>validRow = prepareRow(description,"");
            rootNode->appendRow(validRow);
            setChildItems(msgType, validRow);
        }
            break;
        case EMPTY_SUBMODEL_LOAD_MSG:
        case INCLUDE_FILE_LOAD_MSG:
        /* Do not add these into the load status dialogue because they are not loaded in the LDrawFile.subfiles
        case PRIMITIVE_LOAD_MSG:
        case SUBPART_LOAD_MSG:
        */
        {
            QList<QStandardItem *>warningRow = prepareRow(description,"");
            QStandardItem *header = warningRow.at(0);
            header->setForeground(warningBrush);
            rootNode->appendRow(warningRow);
            setChildItems(msgType, warningRow);
        }
            break;
        case MISSING_PART_LOAD_MSG:
        {
            QList<QStandardItem *>errorRow = prepareRow(description,"");
            QStandardItem *header = errorRow.at(0);
            header->setForeground(errorBrush);
            rootNode->appendRow(errorRow);
            setChildItems(msgType, errorRow);
            if (!menuAction) {
                ui->buttonBox->addButton(QDialogButtonBox::Discard);
                connect(ui->buttonBox,SIGNAL(clicked(QAbstractButton*)),SLOT(getButton(QAbstractButton*)));
            }
        }
            break;
        case ALL_LOAD_MSG:
            break;
        }
    };


    int count = getCount(MISSING_PART_LOAD_MSG);
    if (count)
        setRow(MISSING_PART_LOAD_MSG,tr("Error - Missing Parts - %1").arg(count));

    count = getCount(VALID_LOAD_MSG);
    if (count)
        setRow(VALID_LOAD_MSG, tr("Validated Parts - %1").arg(count));

    count = getCount(MPD_SUBMODEL_LOAD_MSG);
    if (count)
        setRow(MPD_SUBMODEL_LOAD_MSG, tr("MPD Submodels - %1").arg(count));

    count = getCount(LDR_SUBMODEL_LOAD_MSG);
    if (count)
        setRow(LDR_SUBMODEL_LOAD_MSG, tr("LDR Submodels - %1").arg(count));

    count = getCount(INLINE_SUBPART_LOAD_MSG);
    if (count)
        setRow(INLINE_SUBPART_LOAD_MSG, tr("Subparts - %1").arg(count));

    count = getCount(INLINE_PRIMITIVE_LOAD_MSG);
    if (count)
        setRow(INLINE_PRIMITIVE_LOAD_MSG, tr("Primitives - %1").arg(count));

    count = getCount(EMPTY_SUBMODEL_LOAD_MSG);
    if (count)
        setRow(EMPTY_SUBMODEL_LOAD_MSG,tr("Warning - Empty Submodels - %1").arg(count));

    count = getCount(INCLUDE_FILE_LOAD_MSG);
    if (count)
        setRow(INCLUDE_FILE_LOAD_MSG,tr("Warning - Include File - %1").arg(count));

    // These should never trigger because to do so means official primitives or subparts have been loaded
    /* Do not add these into the load status dialogue because they are not loaded in the LDrawFile.subfiles
    count = getCount(PRIMITIVE_LOAD_MSG);
    if (count)
        setRow(PRIMITIVE_LOAD_MSG, tr("Warning - Primitives - %1").arg(count));

    count = getCount(SUBPART_LOAD_MSG);
    if (count)
        setRow(SUBPART_LOAD_MSG, tr("Warning - Subparts - %1").arg(count));
    */

    ui->messagesView->setModel(lm);
    //ui->messagesView->expandAll();
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

    adjustSize();

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

int LdrawFilesLoad::showLoadMessages(const QStringList &stringList, bool menuAction)
{
  LdrawFilesLoad *filesLoadDialog = new LdrawFilesLoad(stringList, menuAction);
  return filesLoadDialog->exec();
}
