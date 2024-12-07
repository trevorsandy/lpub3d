/****************************************************************************
**
** Copyright (C) 2019 - 2024 Trevor SANDY. All rights reserved.
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
 * Please see lpub.h for an overall description of how the files in LPub3D
 * make up the LPub3D program.
 *
 ***************************************************************************/

#include <QStandardItemModel>
#include <QItemSelectionModel>
#include <QHeaderView>
#include <QApplication>
#include <QMessageBox>
#include <QClipboard>
#include <QMimeData>
#include <QKeyEvent>
#include <QDebug>

#include "ldrawfilesload.h"
#include "ui_ldrawfilesloaddialog.h"

#include "declarations.h"
#include "lpub_qtcompat.h"
#include "commonmenus.h"
#include "version.h"

QVariant LdrawFilesLoadModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
        switch (section) {
        case 0:
            return tr("Item (count)");
        case 1:
            return tr("Description");
        }
    }
    return QVariant();
}

LdrawFilesLoad::LdrawFilesLoad(const LoadStatus &loadStatus, bool menuAction, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LdrawFilesLoadDialog),
    lm(new LdrawFilesLoadModel(this)),
    dl(false),
    isAction(menuAction),
    isMpd(loadStatus.isMpd),
    loadedLines(loadStatus.loadedLines),
    loadedSteps(loadStatus.loadedSteps),
    subFileCount(loadStatus.subFileCount),
    partCount(loadStatus.partCount),
    uniquePartCount(loadStatus.uniquePartCount),
    helperPartCount(loadStatus.helperPartCount),
    displayModelPartCount(loadStatus.displayModelPartCount),
    modelFile(loadStatus.modelFile),
    elapsedTime(loadStatus.elapsedTime),
    loadedItems(loadStatus.loadedItems)
{
    ui->setupUi(this);

    setWindowTitle(tr("LDraw File Load Status"));

    setWhatsThis(lpubWT(WT_DIALOG_LDRAW_FILES_LOAD,windowTitle()));

    rn = lm->invisibleRootItem();

    includeColor = QColor(0,153,255);
    includeBrush.setColor(includeColor);

    warningColor = QColor(Qt::darkYellow);
    warningBrush.setColor(warningColor);

    errorColor   = QColor(Qt::red);
    errorBrush.setColor(errorColor);

    mpc  = countItems(MISSING_PART_LOAD_MSG);
    vpc  = countItems(VALID_LOAD_MSG);
    msmc = countItems(MPD_SUBMODEL_LOAD_MSG);
    lsmc = countItems(LDR_SUBFILE_LOAD_MSG);
    ipc  = countItems(INLINE_PART_LOAD_MSG);
    idc  = countItems(INLINE_DATA_LOAD_MSG);
    igpc = countItems(INLINE_GENERATED_PART_LOAD_MSG);
    ispc = countItems(INLINE_SUBPART_LOAD_MSG);
    ippc = countItems(INLINE_PRIMITIVE_LOAD_MSG);
    esmc = countItems(EMPTY_SUBMODEL_LOAD_MSG);
    bidc = countItems(BAD_DATA_LOAD_MSG);
    bifc = countItems(BAD_INCLUDE_LOAD_MSG);
    ifc  = countItems(INCLUDE_FILE_LOAD_MSG);
    hpc  = helperPartCount;
    /* Do not add these into the load status dialogue because they are not loaded in the LDrawFile.subfiles
    ppc  = countItems(PRIMITIVE_LOAD_MSG);
    spc  = countItems(SUBPART_LOAD_MSG);
    */

    ui->messagesView->setModel(lm);
    ui->messagesView->setSortingEnabled(true);
    ui->messagesView->sortByColumn(0, Qt::AscendingOrder);
    ui->messagesView->setSelectionMode( QAbstractItemView::MultiSelection );
    ui->messagesView->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->messagesView->setContextMenuPolicy(Qt::ActionsContextMenu);
    ui->messagesView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->messagesView->adjustSize();

    summary();

    populate();

    createActions();

    setMinimumSize(100, 100);

    adjustSize();

    setModal(true);
}

LdrawFilesLoad::~LdrawFilesLoad()
{
    delete ui;
    delete lm;
    delete sm;
    delete rn;
}

void LdrawFilesLoad::createActions()
{
    QMenu *contextMenu = new QMenu(ui->messagesView);

    copyAct = new QAction(QIcon(":/resources/copy.png"), tr("Copy"), contextMenu);
    copyAct->setShortcut(QKeySequence::Copy);
    copyAct->setToolTip(tr("Copy the current selection's contents to the clipboard"));
    ui->messagesView->addAction(copyAct);
    connect(copyAct, SIGNAL(triggered()), this, SLOT(copy()));

    selAllAct = new QAction(QIcon(":/resources/selectall.png"), tr("Select All"), contextMenu);
    selAllAct->setShortcut(QKeySequence::SelectAll);
    selAllAct->setToolTip(tr("Select all tableview content"));
    ui->messagesView->addAction(selAllAct);
    connect(selAllAct, SIGNAL(triggered()), ui->messagesView, SLOT(selectAll()));
    connect(selAllAct, SIGNAL(triggered()), this, SLOT(enableActions()));

    deselectAct = new QAction(QIcon(":/resources/clearselection.png"), tr("Clear Selection"), contextMenu);
    deselectAct->setToolTip(tr("Clear selected tableview content"));
    deselectAct->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_D));
    ui->messagesView->addAction(deselectAct);
    connect(deselectAct, SIGNAL(triggered()), ui->messagesView, SLOT(clearSelection()));
    connect(deselectAct, SIGNAL(triggered()), this, SLOT(enableActions()));

    expandAct = new QAction(QIcon(":/resources/expandselection.png"), tr("Expand All"), contextMenu);
    expandAct->setToolTip(tr("Expand tableview content"));
    expandAct->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_E));
    ui->messagesView->addAction(expandAct);
    connect(expandAct, SIGNAL(triggered()), ui->messagesView, SLOT(expandAll()));
    connect(expandAct, SIGNAL(triggered()), this, SLOT(enableActions()));

    collapseAct = new QAction(QIcon(":/resources/collapseselection.png"), tr("Collapse All"), contextMenu);
    collapseAct->setToolTip(tr("Collapse tableview content"));
    collapseAct->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_S));
    ui->messagesView->addAction(collapseAct);
    connect(collapseAct, SIGNAL(triggered()), ui->messagesView, SLOT(collapseAll()));
    connect(collapseAct, SIGNAL(triggered()), this, SLOT(enableActions()));

    ungroupAct = new QAction(QIcon(":/resources/ungroupitems.png"), tr("Ungroup Parts"), contextMenu);
    ungroupAct->setToolTip(tr("Show individual part and submodel occurrence"));
    ungroupAct->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_U));
    ui->messagesView->addAction(ungroupAct);
    connect(ungroupAct, SIGNAL(triggered()), this, SLOT(ungroupItems()));
    connect(ungroupAct, SIGNAL(triggered()), this, SLOT(enableActions()));

    groupAct = new QAction(QIcon(":/resources/ungroupitems.png"), tr("Group Parts"), contextMenu);
    groupAct->setToolTip(tr("Group part and submodel occurrence"));
    groupAct->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_G));
    ui->messagesView->addAction(groupAct);
    connect(groupAct, SIGNAL(triggered()), this, SLOT(groupItems()));
    connect(groupAct, SIGNAL(triggered()), this, SLOT(enableActions()));

    sm = ui->messagesView->selectionModel();
    connect(sm,  SIGNAL(selectionChanged(const QItemSelection &,const QItemSelection &)),
            this,  SLOT(selectionChanged(const QItemSelection &,const QItemSelection &)));

    if (!isAction && mpc) {
        ui->buttonBox->addButton(QDialogButtonBox::Discard);
        connect(ui->buttonBox,SIGNAL(clicked(QAbstractButton*)),SLOT(getButton(QAbstractButton*)));
    }

    enableActions();
}

void LdrawFilesLoad::enableActions()
{
    if (sender() == selAllAct) {
        selAllAct->setEnabled(false);
        deselectAct->setEnabled(true);
    }
    else if (sender() == deselectAct) {
        selAllAct->setEnabled(true);
        deselectAct->setEnabled(false);
    }
    else if (sender() == expandAct) {
        expandAct->setEnabled(false);
        collapseAct->setEnabled(true);
    }
    else if (sender() == collapseAct) {
        expandAct->setEnabled(true);
        collapseAct->setEnabled(false);
    }
    else if (sender() == ungroupAct) {
        ungroupAct->setEnabled(false);
        groupAct->setEnabled(true);
    }
    else if (sender() == groupAct) {
        ungroupAct->setEnabled(true);
        groupAct->setEnabled(false);
    } else {
        copyAct->setEnabled(false);
        selAllAct->setEnabled(true);
        deselectAct->setEnabled(false);
        expandAct->setEnabled(true);
        collapseAct->setEnabled(false);
        ungroupAct->setEnabled(true);
        groupAct->setEnabled(false);
    }
}

void LdrawFilesLoad::ungroupItems()
{
    rn->removeRows(0, rn->rowCount());
    populate(false);
    ui->messagesView->sortByColumn(0, Qt::AscendingOrder);
    for (int i = MSG_TYPE; i < rn->rowCount(); i++) {
        LoadMsgType msgType = static_cast<LoadMsgType>(lm->item(i, MSG_TYPE)->data().toInt());
        if (msgType == VALID_LOAD_MSG || msgType == MPD_SUBMODEL_LOAD_MSG) {
            QModelIndex current = lm->item(i, MSG_TYPE)->index();
            ui->messagesView->expand(current);
        }
    }
}

void LdrawFilesLoad::groupItems()
{
    rn->removeRows(0, rn->rowCount());
    populate();
    ui->messagesView->sortByColumn(0, Qt::AscendingOrder);
    for (int i = MSG_TYPE; i < rn->rowCount(); i++) {
        LoadMsgType msgType = static_cast<LoadMsgType>(lm->item(i, MSG_TYPE)->data().toInt());
        if (msgType == VALID_LOAD_MSG || msgType == MPD_SUBMODEL_LOAD_MSG) {
            QModelIndex current = lm->item(i, MSG_TYPE)->index();
            ui->messagesView->expand(current);
        }
    }
}

void LdrawFilesLoad::selectionChanged(const QItemSelection &selected, const QItemSelection &)
{
    copyAct->setEnabled(selected.indexes().size());
}

void LdrawFilesLoad::getButton(QAbstractButton *button)
{
    QDialogButtonBox::ButtonRole role = ui->buttonBox->buttonRole(button);
    if (role == QDialogButtonBox::DestructiveRole)
        dl = true;
}

void LdrawFilesLoad::copy() const
{
    QModelIndexList selection = sm->selectedIndexes();

    if(!selection.size()) {
        QMessageBox::information(nullptr, tr("No Selection"), tr("Nothing selected. Make a selection before copying."));
        return;
    }

    lpub_sort(selection.begin(), selection.end());

    // You need a pair of indexes to find the row changes
    QModelIndex previous = selection.first();
    selection.removeFirst();
    QString selected_text;
    QModelIndex current;
    Q_FOREACH(current, selection)
    {
        QVariant ldata = lm->data(previous);
        QString text = ldata.toString();
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

int LdrawFilesLoad::countItems(const LoadMsgType lmt, const QString item)
{
    int count = 0;
    bool ok = false;
    bool found = false;

    for (const QString &loadedItem : loadedItems)
    {
        LoadMsgType mt = static_cast<LoadMsgType>(loadedItem.leftRef(loadedItem.indexOf('|')).toInt(&ok));
        if (ok && mt == lmt) {
            if (!item.isEmpty()) {
                QString const current = loadedItem.split("|").at(ITEM);
                if (item == current) {
                    found = true;
                    count++;
                } else if (found)
                    break;
            } else {
                count++;
            }
        }
    }

    return count;
}

void LdrawFilesLoad::summary() const
{
    int ls     = loadedSteps;
    int upc    = uniquePartCount;
    int dmpc   = displayModelPartCount;
    bool delta = partCount != vpc;

    QString messages  = tr("No loaded items received.");
    if (loadedItems.size())
        messages = QObject::tr("Loaded LDraw <b>%1</b> model file:  <b>%2</b>.<br>").arg(isMpd ? QString("MPD") : QString("LDR"), modelFile);
    if (mpc)
        messages.append(QObject::tr("%1<br> - Missing parts:        <b>%2</b></span>").arg("<span style=\"color:#FF0000\">").arg(mpc));
    if (esmc)
        messages.append(QObject::tr("%1<br> - Empty submodels:      <b>%2</b></span>").arg("<span style=\"color:#8B8000\">").arg(esmc));
    if (bidc)
        messages.append(QObject::tr("%1<br> - Data issues:          <b>%2</b></span>").arg("<span style=\"color:#8B8000\">").arg(bidc));
    if (bifc)
        messages.append(QObject::tr("%1<br> - Include file issues:  <b>%2</b></span>").arg("<span style=\"color:#8B8000\">").arg(bifc));
    if (delta)
        messages.append(QObject::tr("<br> - Model parts:            <b>%1</b>").arg(partCount));
    if (vpc)
        messages.append(QObject::tr("<br> - %1 parts:               <b>%2</b>").arg(delta ? QObject::tr("Validated") : QObject::tr("Model")).arg(vpc));
    if (upc)
        messages.append(QObject::tr("<br> - Unique parts:           <b>%1</b>").arg(upc));
    if (dmpc)
        messages.append(QObject::tr("<br> - Display parts:          <b>%1</b>").arg(dmpc));
    if (hpc)
        messages.append(QObject::tr("<br> - Helper parts:           <b>%1</b>").arg(hpc));
    if (msmc)
        messages.append(QObject::tr("<br> - Submodels:              <b>%1</b>").arg(msmc));
    if (lsmc)
        messages.append(QObject::tr("<br> - Subfiles:               <b>%1</b>").arg(lsmc));
    if (ifc)
        messages.append(QObject::tr("<br> - Include files:          <b>%1</b>").arg(ifc));
    if (ipc)
        messages.append(QObject::tr("<br> - Inline parts:           <b>%1</b>").arg(ipc));
    if (igpc)
        messages.append(QObject::tr("<br> - Inline generated parts: <b>%1</b>").arg(igpc));
    if (ippc)
        messages.append(QObject::tr("<br> - Inline primitives:      <b>%1</b>").arg(ippc));
    if (ispc)
        messages.append(QObject::tr("<br> - Inline subparts:        <b>%1</b>").arg(ispc));
    if (idc)
        messages.append(QObject::tr("<br> - Inline data:            <b>%1</b>").arg(idc));
    /* Do not add these into the load status dialogue because they are not loaded in the LDrawFile.subfiles
    if (ppc)
        messages.append(QObject::tr("<br> - Primitive parts:        <b>%1</b>").arg(ppc));
    if (spc)
        messages.append(QObject::tr("<br> - Subparts:               <b>%1</b>").arg(spc)); */
    if (ls)
        messages.append(QObject::tr("<br> - Loaded steps:           <b>%1</b>").arg(ls));

    QString suffix =    QObject::tr("<br> - Loaded lines:           <b>%1</b>"
                                    "<br> - %2").arg(loadedLines).arg(elapsedTime);

    if (subFileCount != (isMpd ? msmc : lsmc))
        suffix.prepend( QObject::tr("<br> - Loaded subfiles:        <b>%1</b>").arg(subFileCount));

    messages.append(suffix);

    if (mpc) {
        QString const partsStr  = tr("%1").arg(mpc > 1 ? QObject::tr("Some parts") : QObject::tr("A part"));
        QString const wereStr   = tr("%1").arg(mpc > 1 ? QObject::tr("were")       : QObject::tr("was"));
        QString const theseStr  = tr("%1").arg(mpc > 1 ? QObject::tr("these are")  : QObject::tr("this is"));
        QString const theirStr  = tr("%1").arg(mpc > 1 ? QObject::tr("their")      : QObject::tr("its"));
        messages.append(QObject::tr("<br><br>%1 %2 not found. The following locations were searched;<br>"
                                    "model file, LDraw search paths, %3 and %4 library archives.<br>"
                                    "If %5 custom %1, be sure %6 location is in the LDraw search directory list.<br>"
                                    "If %5 new unofficial %1, be sure the unofficial archive library is up to date.")
                                    .arg(partsStr,wereStr,VER_LPUB3D_UNOFFICIAL_ARCHIVE,VER_LDRAW_OFFICIAL_ARCHIVE,theseStr,theirStr));
    }
    if (esmc) {
        QString const subModStr = tr("%1").arg(esmc > 1 ? QObject::tr("submodels") : QObject::tr("submodel"));
        messages.append(QObject::tr("<br><br>Empty %1 found. These submodels were not added to the model repository").arg(subModStr));
    }

    ui->messagesLabel->setText(messages);
    if (loadedItems.isEmpty())
        ui->messagesLabel->setStyleSheet("QLabel { color: red; }");
    ui->messagesLabel->adjustSize();
}

void LdrawFilesLoad::populate(bool groupItems)
{
    auto prepareRow = [] (const QString &rowItem,const QString &rowDesc)
    {
        QList<QStandardItem *> row = { new QStandardItem(rowItem), new QStandardItem(rowDesc) };
        return row;
    };

    auto setChildItems = [&] (const LoadMsgType lmt, QList<QStandardItem *>parentRow, bool group = false)
    {
        bool ok;
        QString item;
        for (const QString &loadedItem : loadedItems)
        {
            LoadMsgType mt = static_cast<LoadMsgType>(loadedItem.leftRef(loadedItem.indexOf('|')).toInt(&ok));
            if (ok && mt == lmt) {
                QStringList columns = loadedItem.split("|");
                QString const toolTip = columns[DESC];
                if (group) {
                    if (columns.at(ITEM) != item) {
                        item = columns.at(ITEM);
                        columns[ITEM] = QString("%1 (%2)").arg(columns[ITEM]).arg(countItems(mt, item));
                        columns[DESC] = QString(columns.at(DESC).left(columns.at(DESC).indexOf('('))).trimmed();
                    } else {
                        continue;
                    }
                }
                QList<QStandardItem *>childRow = prepareRow(columns.at(ITEM),columns.at(DESC));
                childRow[ITEM]->setToolTip(toolTip);
                switch (lmt) {
                    case INCLUDE_FILE_LOAD_MSG:
                    {
                        QStandardItem *rowItem = childRow.at(0);
                        rowItem->setForeground(includeBrush);
                        QStandardItem *rowDesc = childRow.at(1);
                        rowDesc->setForeground(includeBrush);
                    }
                    break;
                    case MISSING_PART_LOAD_MSG:
                    {
                        QStandardItem *rowItem = childRow.at(0);
                        rowItem->setForeground(errorBrush);
                        QStandardItem *rowDesc = childRow.at(1);
                        rowDesc->setForeground(errorBrush);
                    }
                    break;
                    case EMPTY_SUBMODEL_LOAD_MSG:
                    case BAD_INCLUDE_LOAD_MSG:
                    case BAD_DATA_LOAD_MSG:
                    /* Do not add these into the load status dialogue because they are not loaded in the LDrawFile.subfiles
                    case PRIMITIVE_LOAD_MSG:
                    case SUBPART_LOAD_MSG:
                    */
                    {
                        QStandardItem *rowItem = childRow.at(0);
                        rowItem->setForeground(warningBrush);
                        QStandardItem *rowDesc = childRow.at(1);
                        rowDesc->setForeground(warningBrush);
                    }
                    break;
                    default:
                    break;
                }
                parentRow.first()->appendRow(childRow);
            }
        }
    };

    auto setRow = [&] (LoadMsgType msgType, QString const &item)
    {
        switch (msgType)
        {
        case VALID_LOAD_MSG:
        case INLINE_PART_LOAD_MSG:
        case INLINE_GENERATED_PART_LOAD_MSG:
        case INLINE_PRIMITIVE_LOAD_MSG:
        case INLINE_SUBPART_LOAD_MSG:
        case INLINE_DATA_LOAD_MSG:
        case MPD_SUBMODEL_LOAD_MSG:
        case LDR_SUBFILE_LOAD_MSG:
        case HELPER_PART_LOAD_MSG:
        {
            QList<QStandardItem *>validRow = prepareRow(item,"");
            QStandardItem *rowItem = validRow.at(0);
            rowItem->setData(msgType);
            rn->appendRow(validRow);
            bool group = groupItems && (msgType == VALID_LOAD_MSG || msgType == MPD_SUBMODEL_LOAD_MSG);
            setChildItems(msgType, validRow, group);
        }
            break;
        case INCLUDE_FILE_LOAD_MSG:
        {
            QList<QStandardItem *>includeRow = prepareRow(item,"");
            QStandardItem *rowItem = includeRow.at(0);
            rowItem->setData(msgType);
            rowItem->setForeground(includeBrush);
            rn->appendRow(includeRow);
            setChildItems(msgType, includeRow);
        }
            break;
        case EMPTY_SUBMODEL_LOAD_MSG:
        case BAD_INCLUDE_LOAD_MSG:
        case BAD_DATA_LOAD_MSG:
        /* Do not add these into the load status dialogue because they are not loaded in the LDrawFile.subfiles
        case PRIMITIVE_LOAD_MSG:
        case SUBPART_LOAD_MSG:
        */
        {
            QList<QStandardItem *>warningRow = prepareRow(item,"");
            QStandardItem *rowItem = warningRow.at(0);
            rowItem->setData(msgType);
            rowItem->setForeground(warningBrush);
            rn->appendRow(warningRow);
            setChildItems(msgType, warningRow);
        }
            break;
        case MISSING_PART_LOAD_MSG:
        {
            QList<QStandardItem *>errorRow = prepareRow(item,"");
            QStandardItem *rowItem = errorRow.at(0);
            rowItem->setData(msgType);
            rowItem->setForeground(errorBrush);
            rn->appendRow(errorRow);
            setChildItems(msgType, errorRow);
        }
            break;
        case ALL_LOAD_MSG:
            break;
        }
    };

    if (mpc)
        setRow(MISSING_PART_LOAD_MSG,     tr("Error - Missing Parts (%1)").arg(mpc));

    if (vpc)
        setRow(VALID_LOAD_MSG,            tr("Validated Parts (%1)").arg(vpc));

    if (hpc)
        setRow(HELPER_PART_LOAD_MSG,      tr("Helper Parts (%1)").arg(hpc));

    if (msmc)
        setRow(MPD_SUBMODEL_LOAD_MSG,     tr("Submodels (%1)").arg(msmc));

    if (lsmc)
        setRow(LDR_SUBFILE_LOAD_MSG,      tr("Subfiles (%1)").arg(lsmc));

    if (ifc)
        setRow(INCLUDE_FILE_LOAD_MSG,     tr("Include Files (%1)").arg(ifc));

    if (ipc)
        setRow(INLINE_PART_LOAD_MSG,      tr("Inline Parts (%1)").arg(ipc));

    if (idc)
        setRow(INLINE_DATA_LOAD_MSG,      tr("Inline Data (%1)").arg(ipc));

    if (igpc)
        setRow(INLINE_GENERATED_PART_LOAD_MSG, tr("Inline Generated Parts (%1)").arg(igpc));

    if (ispc)
        setRow(INLINE_SUBPART_LOAD_MSG,   tr("Inline Subparts (%1)").arg(ispc));

    if (ippc)
        setRow(INLINE_PRIMITIVE_LOAD_MSG, tr("Inline Primitives (%1)").arg(ippc));

    if (esmc)
        setRow(EMPTY_SUBMODEL_LOAD_MSG,   tr("Warning - Empty Submodels (%1)").arg(esmc));

    if (bifc)
        setRow(BAD_INCLUDE_LOAD_MSG,      tr("Warning - Include File Issues (%1)").arg(bifc));

    if (bidc)
        setRow(BAD_DATA_LOAD_MSG,         tr("Warning - Data File Issues (%1)").arg(bidc));

    // These should never trigger because to do so means official primitives or subparts have been loaded
    /* Do not add these into the load status dialogue because they are not loaded in the LDrawFile.subfiles
    if (ppc)
        setRow(PRIMITIVE_LOAD_MSG,        tr("Warning - Primitives (%1)").arg(ppc));

    if (spc)
        setRow(SUBPART_LOAD_MSG,          tr("Warning - Subparts (%1)").arg(spc));
    */
}

QDialog::DialogCode LdrawFilesLoad::showLoadStatus(const LoadStatus &loadStatus, bool menuAction)
{
  DialogCode result = QDialog::Accepted;
  LdrawFilesLoad *filesLoadDialog = new LdrawFilesLoad(loadStatus, menuAction);
  if (filesLoadDialog->exec() == QDialog::Rejected) {
    if (filesLoadDialog->discardLoad())
      result = QDialog::Rejected;
  }
  return result;
}
