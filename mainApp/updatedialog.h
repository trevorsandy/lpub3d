#ifndef UPDATEDIALOG_H
#define UPDATEDIALOG_H

#include <QDialog>
#include <QMessageBox>
#include <QSimpleUpdater>

namespace Ui {
class UpdateDialog;
}

void DoInitialUpdateCheck();

class UpdateDialog : public QDialog
{
    Q_OBJECT

public:
    explicit UpdateDialog(QWidget *parent, void *data);
    ~UpdateDialog();

public slots:
    void checkForUpdates();
    void onCheckingFinished();

private:
    Ui::UpdateDialog *ui;

    QString m_installed_version;
    QSimpleUpdater *updater;
    // from original
    bool initialUpdate;
};

#endif //UPDATEDIALOG_H
