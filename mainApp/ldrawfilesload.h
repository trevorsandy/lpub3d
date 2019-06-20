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
