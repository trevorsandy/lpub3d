#include "updatedialog.h"
#include "ui_updatedialog.h"
#include "version.h"

#include "QsLog.h"

void DoInitialUpdateCheck()
{

//    int updateFrequency = Preferences::updateFrequency;

//	if (updateFrequency == 0)
//		return;

    // check for update once a week
    int updateFrequency = 7;

	QSettings Settings;
	QDateTime checkTime = Settings.value("Updates/LastCheck", QDateTime()).toDateTime();

	if (!checkTime.isNull())
	{
		checkTime.addDays(updateFrequency == 1 ? 1 : 7);

		if (checkTime > QDateTime::currentDateTimeUtc())
			return;
	}

	new UpdateDialog(NULL, (void*)1);
}

UpdateDialog::UpdateDialog(QWidget *parent, void *data) :
	QDialog(parent),
	ui(new Ui::UpdateDialog)
{
	ui->setupUi(this);

	initialUpdate = (bool)data;
	connect(this, SIGNAL(finished(int)), this, SLOT(finished(int)));

	ui->status->setText(tr("Connecting to update server..."));

//	manager = new QNetworkAccessManager(this);
//	connect(manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinished(QNetworkReply*)));

//    updateRequest.setUrl(QUrl(VER_UPDATE_CHECK_URL));
//    updateRequest.setRawHeader("User-Agent","Mozilla Firefox");

//    updateReply = manager->get(updateRequest);

    // Initialize the updater
    updater = new QSimpleUpdater (this);

    // Check for updates when the updates
    checkForUpdates();

    // When the updater finishes checking for updates, show a message box
    // and show the change log of the latest version
    connect (updater, SIGNAL (checkingFinished()), this, SLOT (onCheckingFinished()));  // can change slot to finished();
}

UpdateDialog::~UpdateDialog()
{
	if (updateReply)
	{
		updateReply->abort();
		updateReply->deleteLater();
	}

	if (manager)
		manager->deleteLater();

	delete ui;
}

void UpdateDialog::onCheckingFinished()
{
    //nothing to do yet.
}

void UpdateDialog::checkForUpdates(){

    ui->status->setText(tr("Checking for updates..."));

    // Set the current application version
    updater->setApplicationVersion (VER_PRODUCTVERSION_STR);

    // Tell the updater where we can find the file that tells us the latest version
    // of the application
    updater->setReferenceUrl (VER_UPDATE_CHECK_URL);

    // Tell the updater where we should download the changelog, note that
    // the changelog can be any file you want,
    // such as an HTML page or (as in this example), a text file
    updater->setChangelogUrl (VER_CHANGE_LOG_URL);

    // Tell the updater where to download the update, its recommended to use direct links
    updater->setDownloadUrl (VER_DOWNLOAD_URL);

    // Show the progress dialog and show messages when checking is finished
    updater->setSilent (false);
    updater->setShowNewestVersionMessage (true);

    // Finally, check for updates...
    updater->checkForUpdates();
}

void UpdateDialog::accept()
{
	QSettings Settings;
	Settings.setValue("Updates/IgnoreVersion", versionData);

	QDialog::accept();
}

void UpdateDialog::reject()
{
	if (updateReply)
	{
		updateReply->abort();
		updateReply->deleteLater();
		updateReply = NULL;
	}

	QDialog::reject();
}

void UpdateDialog::finished(int result)
{
	if (initialUpdate)
		deleteLater();
}

void UpdateDialog::replyFinished(QNetworkReply *reply)
{
	bool updateAvailable = false;

	if (reply->error() == QNetworkReply::NoError)
	{
		int majorVersion, minorVersion, patchVersion;

		versionData = reply->readAll();
		const char *update = versionData;

		QSettings Settings;
		QByteArray ignoreUpdate = Settings.value("Updates/IgnoreVersion", QByteArray()).toByteArray();

        logNotice() << "Reply info: " << update;

		if (initialUpdate && ignoreUpdate == versionData)
		{
			updateAvailable = false;
		}
        else if (sscanf(update, "%d.%d.%d", &majorVersion, &minorVersion, &patchVersion) == 3)
		{
			QString status;

            if (majorVersion > VER_MAJOR)
				updateAvailable = true;
            else if (majorVersion == VER_MAJOR)
			{
                if (minorVersion > VER_MINOR)
					updateAvailable = true;
                else if (minorVersion == VER_MINOR)
				{
                    if (patchVersion > VER_SP)
						updateAvailable = true;
				}
			}

			if (updateAvailable)
                status = QString(tr("<p>There's a newer version of %1 available for download (%2.%3.%4).</p>"
                                    "<p>Your installed version is (%5.%6.%7).</p>")).arg(QString::fromLatin1(VER_PRODUCTNAME_STR),
                                                                                         QString::number(majorVersion),
                                                                                         QString::number(minorVersion),
                                                                                         QString::number(patchVersion),
                                                                                         QString::number(VER_MAJOR),
                                                                                         QString::number(VER_MINOR),
                                                                                         QString::number(VER_SP));
			else
                status = QString(tr("<p>You are using the latest %1 version.</p>")).arg(QString::fromLatin1(VER_PRODUCTNAME_STR));

			if (updateAvailable)
			{
                status += QString(tr("<p>Visit <a href=\"%1\">%1</a> to download.</p>")).arg(QString::fromLatin1(VER_UPDATE_CHECK_URL));
			}

			ui->status->setText(status);
		}
		else
			ui->status->setText(tr("Error parsing update information."));

		Settings.setValue("Updates/LastCheck", QDateTime::currentDateTimeUtc());

		updateReply = NULL;
		reply->deleteLater();
	}
    else {
        ui->status->setText(QString("%1 %2").arg(tr("Error connecting to the update server."))
                            .arg(reply->errorString()));
        versionData = reply->readAll();
        const char *update = versionData;
        logError() << "Reply Info: " << update << ", Network reply error: " << reply->errorString();
    }

	if (initialUpdate)
	{
		if (updateAvailable)
			show();
		else
			deleteLater();
	}

	if (updateAvailable)
		ui->buttonBox->setStandardButtons(QDialogButtonBox::Close | QDialogButtonBox::Ignore);
	else
		ui->buttonBox->setStandardButtons(QDialogButtonBox::Close);
}
