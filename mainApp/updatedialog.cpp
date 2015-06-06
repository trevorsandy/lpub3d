#include "updatedialog.h"
#include "ui_updatedialog.h"
#include "version.h"

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

UpdateDialog::UpdateDialog (QWidget *parent, void *data) :
    QDialog(parent),
    ui(new Ui::UpdateDialog)
{
    // Create and configure the user interface
    ui->setupUi(this);

    initialUpdate = (bool)data;

    ui->versionLabel->setText(tr("Installed Version"));
    ui->versionLineEdit->setText("1.0");
    ui->versionLineEdit->setPlaceholderText("1.0");
    ui->changelogTextEdit->setPlainText(tr("Click the \"Check for updates\" button to download the latest version change log"));

    // Close the dialog when the close button is clicked
    connect (ui->closeButton, SIGNAL(clicked()), this, SLOT(close()));

    // Check for updates when the updates button is clicked
    connect (ui->updatesButton, SIGNAL(clicked()), this, SLOT(checkForUpdates()));

    // Initialize the updater
    updater = new QSimpleUpdater(this);

    // When the updater finishes checking for updates, show a message box
    // and show the change log of the latest version
    connect (updater, SIGNAL(checkingFinished()), this, SLOT(onCheckingFinished()));
}

UpdateDialog::~UpdateDialog() {
    delete ui;
}

void UpdateDialog::checkForUpdates() {
    // Disable the check for updates button while the updater
    // is checking for updates
    ui->updatesButton->setEnabled(false);
    ui->updatesButton->setText(tr("Checking for updates..."));

    // If the user changed the text of the versionLineEdit, then change the
    // application version in the updater too
    if (!ui->versionLineEdit->text().isEmpty()) {
        updater->setApplicationVersion(ui->versionLineEdit->text());
    }

    // If the versionLineEdit is empty, then set the application version
    // to the current application version.
    else {
        updater->setApplicationVersion(VER_PRODUCTVERSION_STR);
    }

    // Tell the updater where we should download the changelog, note that
    // the changelog can be any file you want,
    // such as an HTML page or a text file
    updater->setChangelogUrl(VER_CHANGE_LOG_URL);

    // Tell the updater where we can find the file that tells us the latest version
    // of the application
    updater->setReferenceUrl(VER_UPDATE_CHECK_URL);

    // Tell the updater where to download the update, its recommended to use direct links
    //updater->setDownloadUrl("https://codeload.github.com/alex-97/QSimpleUpdater/zip/master");
    updater->setDownloadUrl(VER_DOWNLOAD_URL);

    // Finally, check for updates...
    updater->checkForUpdates();
}

void UpdateDialog::onCheckingFinished() {
    // Enable the updatesButton and change its text to let the user know
    // that he/she can check for updates again
    ui->updatesButton->setEnabled(true);
    ui->updatesButton->setText(tr("Check for updates"));

    // There's a newer version of the application available, so we inform
    // the user that there's a newer version available and we replace the text
    // of the changelog text edit with the downloaded change log
    if (updater->newerVersionAvailable()) {
        ui->changelogTextEdit->setPlainText(updater->changeLog());

        // Create and configure a message box
        QMessageBox _messagebox;
        _messagebox.setIcon(QMessageBox::Information);
        _messagebox.setWindowTitle(tr("Update available"));
        _messagebox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        _messagebox.setText(tr("There's an update available!"));
        _messagebox.setInformativeText(tr("The latest version of the application is") + " " +
                                       updater->latestVersion() + ", " +
                                       tr("do you want to download it?"));

        // If the user clicks "yes" open the download dialog
        if (_messagebox.exec() == QMessageBox::Yes)
            updater->downloadLatestVersion();
    }

    // The installed version is equal or greater to the "official" latest version,
    // so we inform the user and clear the text of the change log text edit
    else {
        ui->changelogTextEdit->setPlainText("");
        ui->changelogTextEdit->setPlainText(tr("The change log was not downloaded because you "
                                            "are running the latest version of the application..."));

        QMessageBox::information(this, tr("No updates available"),
                                 tr("Congratulations! You are running the latest version of the application!"));
    }
}
