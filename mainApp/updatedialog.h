#ifndef UPDATEDIALOG_H
#define UPDATEDIALOG_H

#include <QDialog>
#include <QNetworkReply>
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

	void parseUpdate(const char *update);
    void checkForUpdates();

public slots:
    void replyFinished(QNetworkReply *reply);       //dep
    void accept();                                  //dep
    void reject();                                  //dep
    void finished(int result);
    void onCheckingFinished();

private:
	Ui::UpdateDialog *ui;

    QNetworkRequest         updateRequest;
    QNetworkReply           *updateReply;
    QNetworkAccessManager   *manager;
    QByteArray              versionData;
    bool                    initialUpdate;

    QSimpleUpdater          *updater;
    QString                 m_installed_version;
};

#endif // UPDATEDIALOG_H
