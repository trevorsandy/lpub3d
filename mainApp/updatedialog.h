#ifndef UPDATEDIALOG_H
#define UPDATEDIALOG_H

#include <QDialog>
#include <QNetworkReply>

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

public slots:
	void replyFinished(QNetworkReply *reply);
	void accept();
	void reject();
	void finished(int result);

private:
	Ui::UpdateDialog *ui;

    QNetworkRequest         updateRequest;
    QNetworkReply           *updateReply;
    QNetworkAccessManager   *manager;
    QByteArray              versionData;
    bool                    initialUpdate;
};

#endif // UPDATEDIALOG_H
