#include "ldrawfilesload.h"
#include "ui_ldrawfilesloaddialog.h"
#include <QStandardItemModel>
#include <QItemSelectionModel>
#include <QHeaderView>
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
    ui->messagesView->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->messagesView->adjustSize();

    setModal(true);
}

LdrawFilesLoad::~LdrawFilesLoad()
{
    delete ui;
    delete lm;
}

int LdrawFilesLoad::showLoadMessages(
  QStringList &stringList)
{
  LdrawFilesLoad *dlg = new LdrawFilesLoad(stringList);
  return dlg->exec();
}
