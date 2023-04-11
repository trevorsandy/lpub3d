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
    modelFile(loadStatus.modelFile),
    elapsedTime(loadStatus.elapsedTime),
    loadedItems(loadStatus.loadedItems)
{
    ui->setupUi(this);

    setWindowTitle(tr("LDraw File Load Status"));

    setWhatsThis(lpubWT(WT_DIALOG_LDRAW_FILES_LOAD,windowTitle()));

    rootNode = lm->invisibleRootItem();

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
    igpc = countItems(INLINE_GENERATED_PART_LOAD_MSG);
    ispc = countItems(INLINE_SUBPART_LOAD_MSG);
    ippc = countItems(INLINE_PRIMITIVE_LOAD_MSG);
    esmc = countItems(EMPTY_SUBMODEL_LOAD_MSG);
    bifc = countItems(BAD_INCLUDE_LOAD_MSG);
    ifc  = countItems(INCLUDE_FILE_LOAD_MSG);
    /* Do not add these into the load status dialogue because they are not loaded in the LDrawFile.subfiles
    ppc  = countItems(PRIMITIVE_LOAD_MSG);
    spc  = countItems(SUBPART_LOAD_MSG);
    */

    summary();

    populate();

    ui->messagesView->setModel(lm);
    ui->messagesView->setSortingEnabled(true);
    ui->messagesView->sortByColumn(0, Qt::AscendingOrder);
    ui->messagesView->setSelectionMode( QAbstractItemView::MultiSelection );
    ui->messagesView->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->messagesView->adjustSize();
    ui->messagesView->setContextMenuPolicy(Qt::ActionsContextMenu);

    createActions();

    setMinimumSize(100, 100);

    adjustSize();

    setModal(true);
}

LdrawFilesLoad::~LdrawFilesLoad()
{
    delete ui;
    delete lm;
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
    rootNode->removeRows(0, rootNode->rowCount());
    populate(false);
    ui->messagesView->sortByColumn(0, Qt::AscendingOrder);
    for (int i = MSG_TYPE; i < rootNode->rowCount(); i++) {
        LoadMsgType msgType = static_cast<LoadMsgType>(lm->item(i, MSG_TYPE)->data().toInt());
        if (msgType == VALID_LOAD_MSG || msgType == MPD_SUBMODEL_LOAD_MSG) {
            QModelIndex current = lm->item(i, MSG_TYPE)->index();
            ui->messagesView->expand(current);
        }
    }
}

void LdrawFilesLoad::groupItems()
{
    rootNode->removeRows(0, rootNode->rowCount());
    populate();
    ui->messagesView->sortByColumn(0, Qt::AscendingOrder);
    for (int i = MSG_TYPE; i < rootNode->rowCount(); i++) {
        LoadMsgType msgType = static_cast<LoadMsgType>(lm->item(i, MSG_TYPE)->data().toInt());
        if (msgType == VALID_LOAD_MSG || msgType == MPD_SUBMODEL_LOAD_MSG) {
            QModelIndex current = lm->item(i, MSG_TYPE)->index();
            ui->messagesView->expand(current);
        }
    }
}

void LdrawFilesLoad::getButton(QAbstractButton *button)
{
    QDialogButtonBox::ButtonRole role = ui->buttonBox->buttonRole(button);
    if (role == QDialogButtonBox::DestructiveRole)
        dl = true;
}

void LdrawFilesLoad::copy() const
{
    QItemSelectionModel * selection = ui->messagesView->selectionModel();
    QModelIndexList indexes = selection->selectedIndexes();

    if(indexes.size() < 1) {
        QMessageBox::information(nullptr, tr("No Selection"), tr("Nothing selected. Make a selection before copying."));
        return;
    }

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
    bool delta = partCount != vpc;

    QString const messages = loadedItems.size() ?
        QObject::tr("Loaded LDraw %1 model file: <b>%2</b>.<br>%3%4%5%6%7%8%9%10%11%12%13%14%15%16%17%18%19%20%21<br>")
            /* 01 */.arg(isMpd ? "<b>MPD</b>" : "<b>LDR</b>",
            /* 02 */     modelFile,
            /* 03 */     delta ? QObject::tr("<br>Parts count:            <b>%1</b>").arg(partCount) : "",
            /* 04 */     mpc   ? QObject::tr("<span style=\"color:red\">"
                                             "<br>Missing parts:          <b>%1</b></span>").arg(mpc) : "",
            /* 05 */     esmc  ? QObject::tr("<span style=\"color:#8B8000\">"
                                             "<br>Empty submodels:        <b>%1</b></span>").arg(esmc) : "",
            /* 06 */     bifc  ? QObject::tr("<span style=\"color:#8B8000\">"
                                             "<br>Include file issues:    <b>%2</b></span>").arg(bifc)  : "",
            /* 07 */     vpc   ? QObject::tr("<br>Total validated parts:  <b>%1</b>").arg(vpc)  : "",
            /* 08 */     upc   ? QObject::tr("<br>Unique validated parts: <b>%1</b>").arg(upc)  : "",
            /* 09 */     msmc  ? QObject::tr("<br>Submodels:              <b>%1</b>").arg(msmc) : "",
            /* 10 */     lsmc  ? QObject::tr("<br>Subfiles:               <b>%1</b>").arg(lsmc) : "",
            /* 11 */     ifc   ? QObject::tr("<br>Include files:          <b>%1</b>").arg(ifc)  : "",
            /* 12 */     ipc   ? QObject::tr("<br>Inline parts:           <b>%1</b>").arg(ipc)  : "",
            /* 13 */     igpc  ? QObject::tr("<br>Inline generated parts: <b>%1</b>").arg(igpc) : "",
            /* 14 */     ippc  ? QObject::tr("<br>Inline primitives:      <b>%1</b>").arg(ippc) : "",
            /* 15 */     ispc  ? QObject::tr("<br>Inline subparts:        <b>%1</b>").arg(ispc) : "",
            // Do not add these into the load status dialogue because they are not loaded in the LDrawFile.subfiles
            //           ppc   ? QObject::tr("<br>Primitive parts:        <b>%1</b>").arg(ppc)  : "",
            //           spc   ? QObject::tr("<br>Subparts:               <b>%1</b>").arg(spc)  : "",
            /* 16 */     ls    ? QObject::tr("<br>Loaded steps:           <b>%1</b>").arg(ls)   : "",
            /* 17 */             QObject::tr("<br>Loaded subfiles:        <b>%1</b>").arg(subFileCount),
            /* 18 */             QObject::tr("<br>Loaded lines:           <b>%1</b>").arg(loadedLines),
            /* 19 */     QString("<br>%1").arg(elapsedTime),
            /* 20 */     mpc   ? QObject::tr("<br><br>Missing %1 %2 not found in the %3 or %4 archive.<br>"
                                             "If %5 custom %1, be sure %6 location is captured in the LDraw search directory list.<br>"
                                             "If %5 new unofficial %1, be sure the unofficial archive library is up to date.")
                                             .arg(mpc > 1 ? QObject::tr("parts") : QObject::tr("part"),          /* 01 */
                                                  mpc > 1 ? QObject::tr("were")  : QObject::tr("was"),           /* 02 */
                                                  VER_LPUB3D_UNOFFICIAL_ARCHIVE,                                 /* 03 */
                                                  VER_LDRAW_OFFICIAL_ARCHIVE,                                    /* 04 */
                                                  mpc > 1 ? QObject::tr("these are") : QObject::tr("this is a"), /* 05 */
                                                  mpc > 1 ? QObject::tr("their")     : QObject::tr("its")) : "", /* 06 */
            /* 21 */     esmc   ? QObject::tr("<br><br>Empty %1 found. These submodels were not added to the model repository")
                                             .arg(esmc > 1 ? QObject::tr("submodels") : QObject::tr("submodel")) : "")
        : tr("No loaded items received.");

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
        case INLINE_SUBPART_LOAD_MSG:
        case INLINE_PRIMITIVE_LOAD_MSG:
        case MPD_SUBMODEL_LOAD_MSG:
        case LDR_SUBFILE_LOAD_MSG:
        {
            QList<QStandardItem *>validRow = prepareRow(item,"");
            QStandardItem *rowItem = validRow.at(0);
            rowItem->setData(msgType);
            rootNode->appendRow(validRow);
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
            rootNode->appendRow(includeRow);
            setChildItems(msgType, includeRow);
        }
            break;
        case EMPTY_SUBMODEL_LOAD_MSG:
        case BAD_INCLUDE_LOAD_MSG:
        /* Do not add these into the load status dialogue because they are not loaded in the LDrawFile.subfiles
        case PRIMITIVE_LOAD_MSG:
        case SUBPART_LOAD_MSG:
        */
        {
            QList<QStandardItem *>warningRow = prepareRow(item,"");
            QStandardItem *rowItem = warningRow.at(0);
            rowItem->setData(msgType);
            rowItem->setForeground(warningBrush);
            rootNode->appendRow(warningRow);
            setChildItems(msgType, warningRow);
        }
            break;
        case MISSING_PART_LOAD_MSG:
        {
            QList<QStandardItem *>errorRow = prepareRow(item,"");
            QStandardItem *rowItem = errorRow.at(0);
            rowItem->setData(msgType);
            rowItem->setForeground(errorBrush);
            rootNode->appendRow(errorRow);
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

    if (msmc)
        setRow(MPD_SUBMODEL_LOAD_MSG,     tr("Submodels (%1)").arg(msmc));

    if (lsmc)
        setRow(LDR_SUBFILE_LOAD_MSG,      tr("Subfiles (%1)").arg(lsmc));

    if (ifc)
        setRow(INCLUDE_FILE_LOAD_MSG,     tr("Include Files (%1)").arg(ifc));

    if (ipc)
        setRow(INLINE_PART_LOAD_MSG,      tr("Inline Parts (%1)").arg(ipc));

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
