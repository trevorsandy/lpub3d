/*
 * Copyright (c) 2014-2016 Alex Spataru <alex_spataru@outlook.com>
 *
 * This file is part of the QSimpleUpdater library, which is released under
 * the DBAD license, you can read a copy of it below:
 *
 * DON'T BE A DICK PUBLIC LICENSE TERMS AND CONDITIONS FOR COPYING,
 * DISTRIBUTION AND MODIFICATION:
 *
 * Do whatever you like with the original work, just don't be a dick.
 * Being a dick includes - but is not limited to - the following instances:
 *
 * 1a. Outright copyright infringement - Don't just copy this and change the
 *     name.
 * 1b. Selling the unmodified original with no work done what-so-ever, that's
 *     REALLY being a dick.
 * 1c. Modifying the original work to contain hidden harmful content.
 *     That would make you a PROPER dick.
 *
 * If you become rich through modifications, related works/services, or
 * supporting the original work, share the love.
 * Only a dick would make loads off this work and not buy the original works
 * creator(s) a pint.
 *
 * Code is provided with no warranty. Using somebody else's code and bitching
 * when it goes wrong makes you a DONKEY dick.
 * Fix the problem yourself. A non-dick would submit the fix back.
 */

//==============================================================================
// Class includes
//==============================================================================

#include "updater.h"
#include "downloader.h"

//==============================================================================
// System includes
//==============================================================================

#include <QJsonArray>
#include <QJsonValue>
#include <QJsonObject>
#include <QMessageBox>
#include <QApplication>
#include <QJsonDocument>
#include <QDesktopServices>

//==============================================================================
// Updater::Updater
//==============================================================================

Updater::Updater() {
    m_url = "";
    m_openUrl = "";
    m_changelog = "";
    m_downloadUrl = "";
    m_downloadName = "";
    m_latestVersion = "";
    m_localDownloadPath = "";
    m_showUpdateNotifications = true;
    m_showAllNotifications = true;
    m_enableDownloader = true;
    m_updateAvailable = false;
    m_directDownload = false;
    m_promptedDownload = false;
    m_isNotSoftwareUpdate = false;
    m_moduleName = qApp->applicationName();
    m_moduleVersion = qApp->applicationVersion();

    m_progressDialog = new ProgressDialog();
    m_downloader = new Downloader();
    m_manager = new QNetworkAccessManager();

#if defined Q_OS_WIN
    m_platform = "windows";
#elif defined Q_OS_MAC
    m_platform = "osx";
#elif defined Q_OS_LINUX
    m_platform = "linux";
#elif defined Q_OS_ANDROID
    m_platform = "android";
#elif defined Q_OS_IOS
    m_platform = "ios";
#endif

#if defined __MINGW32__
    m_compiler = "mingw";
#elif defined _MSC_VER
    m_compiler = "MSVC";
#endif

    m_updateRequest.setRawHeader("User-Agent","Mozilla Firefox");

    connect (m_progressDialog, SIGNAL (cancelClicked()), this, SLOT (cancel()));

    connect (m_downloader, SIGNAL (downloadFinished (QString, QString)),
             this,         SIGNAL (downloadFinished (QString, QString)));
    connect (m_manager,    SIGNAL (finished (QNetworkReply*)),
             this,           SLOT (onReply  (QNetworkReply*)));
}

//==============================================================================
// Updater::~Updater
//==============================================================================

Updater::~Updater() {
    delete m_downloader;
}

//==============================================================================
// Updater::url
//==============================================================================

QString Updater::url() const {
    return m_url;
}

//==============================================================================
// Updater::platformKey
//==============================================================================

QString Updater::platformKey() const {
    return m_platform;
}

//==============================================================================
// Updater::showUpdateNotifications
//==============================================================================

bool Updater::showUpdateNotifications() const {
    return m_showUpdateNotifications;
}

//==============================================================================
// Updater::showAllNotifications
//==============================================================================

bool Updater::showAllNotifications() const {
    return m_showAllNotifications;
}

//==============================================================================
// Updater::updateAvailable
//==============================================================================

bool Updater::updateAvailable() const {
    return m_updateAvailable;
}

//==============================================================================
// Updater::enableDownloader
//==============================================================================

bool Updater::enableDownloader() const {
    return m_enableDownloader;
}

//==============================================================================
// Updater::directDownload
//==============================================================================

bool Updater::directDownload() const {
    return m_directDownload;
}

//==============================================================================
// Updater::promptedDownload
//==============================================================================

bool Updater::promptedDownload() const {
    return m_promptedDownload;
}

//==============================================================================
// Updater::m_changelog
//==============================================================================

QString Updater::changelog() const {
    return m_changelog;
}

//==============================================================================
// Updater::downloadUrl
//==============================================================================

QString Updater::downloadUrl() const {
    return m_downloadUrl;
}

//==============================================================================
// Updater::downloadName
//==============================================================================

QString Updater::downloadName() const {
    return m_downloadName;
}

//==============================================================================
// Updater::localDownloadPath
//==============================================================================

QString Updater::localDownloadPath() const {
    return m_localDownloadPath;
}

//==============================================================================
// Updater::latestVersion
//==============================================================================

QString Updater::latestVersion() const {
    return m_latestVersion;
}

//==============================================================================
// Updater::moduleName
//==============================================================================

QString Updater::moduleName() const {
    return m_moduleName;
}

//==============================================================================
// Updater::moduleVersion
//==============================================================================

QString Updater::moduleVersion() const {
    return m_moduleVersion;
}

//==============================================================================
// Updater::useCustomInstallProcedures
//==============================================================================

bool Updater::useCustomInstallProcedures() const {
    return m_downloader->useCustomInstallProcedures();
}

//==============================================================================
// Updater::isNotSoftwareUpdate
//==============================================================================

bool Updater::isNotSoftwareUpdate() const {
    return m_isNotSoftwareUpdate;
}

//==============================================================================
// Updater::setUrl
//==============================================================================

void Updater::setUrl (const QString& url) {
    m_url = url;
}

//==============================================================================
// Updater::setShowUpdateNotifications
//==============================================================================

void Updater::setShowUpdateNotifications (const bool& notify) {
    m_showUpdateNotifications = notify;
}

//==============================================================================
// Updater::setShowAllNotifications
//==============================================================================

void Updater::setShowAllNotifications (const bool& notify) {
    m_showAllNotifications = notify;
}

//==============================================================================
// Updater::setPlatformKey
//==============================================================================

void Updater::setPlatformKey (const QString& platformKey) {
    m_platform = platformKey;
}

//==============================================================================
// Updater::setModuleName
//==============================================================================

void Updater::setModuleName (const QString& name) {
    m_moduleName = name;
}

//==============================================================================
// Updater::setEnableDownloader
//==============================================================================

void Updater::setEnableDownloader (const bool& enabled) {
    m_enableDownloader = enabled;
}

//==============================================================================
// Updater::setDirectDownload
//==============================================================================

void Updater::setDirectDownload (const bool& enabled) {
    m_directDownload = enabled;
}

//==============================================================================
// Updater::setPromptedDownload
//==============================================================================

void Updater::setPromptedDownload (const bool& enabled) {
    m_promptedDownload = enabled;
}

//==============================================================================
// Updater::setIsNotSoftwareUpdate
//==============================================================================

void Updater::setIsNotSoftwareUpdate (const bool& enabled) {
    m_isNotSoftwareUpdate = enabled;
}

//==============================================================================
// Updater::setLocalDownloadPath
//==============================================================================

void Updater::setLocalDownloadPath (const QString& path) {
    m_localDownloadPath = path;
}

//==============================================================================
// Updater::setDownloadName
//==============================================================================

void Updater::setDownloadName (const QString& name) {
    m_downloadName = name;
}

//==============================================================================
// Updater::setUseCustomInstallProcedures
//==============================================================================

void Updater::setUseCustomInstallProcedures (const bool& custom) {
    m_downloader->setUseCustomInstallProcedures (custom);
}

//==============================================================================
// Updater::setModuleVersion
//==============================================================================

void Updater::setModuleVersion (const QString& version) {
    m_moduleVersion = version;
}

//==============================================================================
// Updater::checkForUpdates
//==============================================================================

void Updater::checkForUpdates() {
    if (showAllNotifications()) {
        if (isNotSoftwareUpdate()){
            m_progressDialog->setDownloadInfo();
        }
        m_progressDialog->show();
    }
    m_updateRequest.setUrl(QUrl(url()));
    m_manager->get(m_updateRequest);
}

//==============================================================================
// Updater::onReply
//==============================================================================

void Updater::onReply (QNetworkReply* reply) {
    if (reply->error() == QNetworkReply::NoError) {

        // Hide the progress dialog
        m_progressDialog->hide();

        QJsonDocument document = QJsonDocument::fromJson (reply->readAll());

        if (document.isNull())
            return;

        QJsonObject updates = document.object().value ("updates").toObject();
        QJsonObject platform = updates.value (platformKey()).toObject();

        if (directDownload()) {

            m_openUrl = platform.value ("open-url").toString();
            m_downloadUrl = platform.value ("download-url").toString();
            m_downloadName = platform.value( "download-name").toString();

            m_downloader->setIsNotSoftwareUpdate(isNotSoftwareUpdate());
            m_downloader->setDownloadName(downloadName());
            m_downloader->setLocalDownloadPath(localDownloadPath());
            m_downloader->startDownload (downloadUrl());

        } else if (promptedDownload()) {

            m_openUrl = platform.value ("open-url").toString();
            m_downloadUrl = platform.value ("download-url").toString();
            m_downloadName = platform.value( "download-name").toString();
            setDownloadAvailable();

        } else {

            QString _changelogUrl;
            bool _updateAvailable;

            if (moduleVersion() == qApp->applicationVersion()) {
                // we are looking to update the latest version

                m_openUrl = platform.value ("open-url").toString();
                if (m_compiler == "mingw")
                    m_downloadUrl = platform.value ("download-url-mingw").toString();
                else if (m_compiler == "MSVC")
                    m_downloadUrl = platform.value ("download-url-").toString();
                m_latestVersion = platform.value ("latest-version").toString();
                _changelogUrl = platform.value ("changelog-url-").toString();
                qDebug() << "LATEST m_compiler: " << m_compiler << " \nm_downloadUrl: " << m_downloadUrl;

                _updateAvailable = compare (latestVersion(), moduleVersion());

            } else {
                // we are looking to update an alternate version

                QMessageBox box;
                box.setTextFormat (Qt::RichText);
                box.setIcon (QMessageBox::Warning);
                box.setStandardButtons (QMessageBox::Ok | QMessageBox::Cancel);
                box.setWindowFlags (Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
                box.setWindowTitle(tr ("Software Update"));
                QString title = "<b> This is a rollback or reinstall update. </b>";
                QString text = tr("Be sure you are comfortable with this type of update as some advanced, manual system configuration may be necessary.");

                box.setText (title);
                box.setInformativeText (text);

                if (box.exec() == QMessageBox::Ok) {
                    QStringList versions = platform.value ("available-versions").toString().split(",");
                    QString latestVersion = platform.value ("latest-version").toString();

                    // sort versions descending 0...10...
                    std::sort(versions.begin(),versions.end());
                    qDebug() << "Versions (sorted): " << versions;
                    for (int i = 0; i < versions.size(); i++) {
                        if (versions[i] == moduleVersion()) {
                            _updateAvailable = true;
                            int updateIndex = i;
                            (updateIndex + 1) == versions.size() ? updateIndex = i : updateIndex = updateIndex + 1;
                            qDebug() << "Update to version: " << versions[updateIndex];

                            if (versions[updateIndex] == latestVersion){
                                // Update to version is same as latest version - i.e. reinstall latest version

                                m_openUrl = platform.value ("open-url").toString();
                                if (m_compiler == "mingw")
                                    m_downloadUrl = platform.value ("download-url-mingw").toString();
                                else if (m_compiler == "MSVC")
                                    m_downloadUrl = platform.value ("download-url-").toString();
                                m_latestVersion = platform.value ("latest-version").toString();
                                _changelogUrl = platform.value ("changelog-url-").toString();
                                qDebug() << "LATEST LOOKUP m_compiler: " << m_compiler << " \nm_downloadUrl: " << m_downloadUrl;
                            } else {
                                // Update to version is othere than the latest version

                                QJsonObject altVersion = platform.value(QString("alternate-version-%1").arg(versions[updateIndex])).toObject();
                                if (altVersion.isEmpty()) {
                                    showErrorMessage("Unable to retrieve version " + versions[updateIndex] + ". Version number not found.");
                                    return;
                                }
                                m_openUrl = altVersion.value ("open-url").toString();
                                if (m_compiler == "mingw")
                                    m_downloadUrl = altVersion.value ("download-url-mingw").toString();
                                else if (m_compiler == "MSVC")
                                    m_downloadUrl = altVersion.value ("download-url").toString();
                                m_latestVersion = altVersion.value ("latest-version").toString();
                                _changelogUrl = altVersion.value ("changelog-url").toString();
                                qDebug() << "ALTERNATE LOOKUP m_compiler: " << m_compiler << " \nm_downloadUrl: " << m_downloadUrl;
                            }
                            break;
                        }
                    }
                } else {return;}
            }

            if (_updateAvailable) {
                QNetworkAccessManager *_manager = new QNetworkAccessManager (this);

                connect (_manager, SIGNAL (finished (QNetworkReply *)), this,
                         SLOT (changeLogReply (QNetworkReply *)));

                m_updateRequest.setUrl(QUrl(_changelogUrl));
                _manager->get(m_updateRequest);

                QEventLoop wait;
                wait.connect(this, SIGNAL(changeLogReplyFinished()),&wait, SLOT(quit()));
                wait.exec();
            }

            setUpdateAvailable (_updateAvailable);

            QSettings Settings;
            Settings.setValue("Updates/LastCheck", QDateTime::currentDateTimeUtc());
        }

    } else {
        showErrorMessage("Error connecting to update server: " + reply->errorString());
    }

    emit checkingFinished (url());
}

//==============================================================================
// Updater::changeLogReply
//==============================================================================

void Updater::changeLogReply (QNetworkReply *reply){
     if (reply->error() == QNetworkReply::NoError) {

        QString _reply = QString::fromUtf8 (reply->readAll());
        m_changelog = _reply;

     } else {
         showErrorMessage("Error receiving change log: " + reply->errorString());
     }
     emit changeLogReplyFinished();
}

//==============================================================================
// Updater::setUpdateAvailable
//==============================================================================

void Updater::setUpdateAvailable (const bool& available) {
    m_updateAvailable = available;

    // Get the application icon as a pixmap
    QPixmap _icon = QPixmap(":/icons/lpub96.png");
    if (_icon.isNull())
        _icon = QPixmap (":/icons/update.png");

    QMessageBox box;
    box.setWindowIcon(QIcon());
    box.setIconPixmap (_icon);
    box.setTextFormat (Qt::RichText);
    box.setWindowTitle(tr ("Software Update"));
    box.setWindowFlags (Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);

    if (updateAvailable() && (showUpdateNotifications() || showAllNotifications())) {

        QString title = "<b>" + tr ("A new version of %1 is available!").arg (moduleName()) + "</b>";
        QString text = tr ("%1 %2 is available - you have %3.\nWould you like to download it now?")
                           .arg(moduleName(), latestVersion(), moduleVersion());
        box.setText (title);
        box.setInformativeText (text);
        box.setDetailedText(changelog());
        box.setStandardButtons (QMessageBox::No | QMessageBox::Yes);
        box.setDefaultButton   (QMessageBox::Yes);

        if (box.exec() == QMessageBox::Yes) {
            if (!m_openUrl.isEmpty() && !enableDownloader())
                QDesktopServices::openUrl (QUrl (m_openUrl));

            else if (enableDownloader())
                m_downloader->startDownload (downloadUrl());

            else
                QDesktopServices::openUrl (QUrl (downloadUrl())); 
        }
    }

    else if (showAllNotifications()) {
        box.setStandardButtons (QMessageBox::Close);        
        box.setText ("<b>" + tr ("You are up-to-date!") +
                     "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</b>");
        box.setInformativeText (
                    tr ("%1 %2 is currently the latest version available")
                        .arg (moduleName(),moduleVersion()));
        box.exec();
    }
}

//==============================================================================
// Updater::setDownloadAvailable
//==============================================================================

void Updater::setDownloadAvailable () {

    // Get the application icon as a pixmap
    QPixmap _icon = QPixmap(":/icons/lpub96.png");
    if (_icon.isNull())
        _icon = QPixmap (":/icons/update.png");

    QMessageBox box;
    box.setWindowIcon(QIcon());
    box.setIconPixmap (_icon);
    box.setTextFormat (Qt::RichText);
    box.setWindowTitle(tr ("Library Update"));
    box.setWindowFlags (Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint );

    QString title = "<b>" + tr ("Download %1?")
                                .arg(downloadName()) + "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</b>";
    QString text = tr ("The latest version of %1 will be downloaded and written to the\n"
                       "%2 folder.\n\nClick Yes to continue or No to cancel.")
                        .arg(downloadName(),localDownloadPath());
    box.setText (title);
    box.setInformativeText (text);
    box.setStandardButtons (QMessageBox::No | QMessageBox::Yes);
    box.setDefaultButton   (QMessageBox::Yes);

    if (box.exec() == QMessageBox::Yes) {
        m_downloader->setIsNotSoftwareUpdate(isNotSoftwareUpdate());
        m_downloader->setDownloadName(downloadName());
        m_downloader->setLocalDownloadPath(localDownloadPath());
        m_downloader->startDownload (downloadUrl());
    }

}

//==============================================================================
// Updater::compare
//==============================================================================

bool Updater::compare (const QString& x, const QString& y) {
    QStringList versionsX = x.split (".");
    QStringList versionsY = y.split (".");

    int count = qMin (versionsX.count(), versionsY.count());

    for (int i = 0; i < count; ++i) {
        int a = QString (versionsX.at (i)).toInt();
        int b = QString (versionsY.at (i)).toInt();

        if (a > b)
            return true;

        else if (b > a)
            return false;
    }

    return versionsY.count() < versionsX.count();
}

//==============================================================================
// Updater::cancel
//==============================================================================

void Updater::cancel (void)
{
    m_manager->disconnect();
}

//==============================================================================
// Updater::showErrorMessage
//==============================================================================

void Updater::showErrorMessage (QString error)
{
  if (showAllNotifications()) // ! not silent
    {
      m_progressDialog->hide();

      QMessageBox box;
      box.setTextFormat (Qt::RichText);
      box.setIcon (QMessageBox::Critical);
      box.setStandardButtons (QMessageBox::Ok );
      box.setWindowFlags (Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
      box.setWindowTitle(isNotSoftwareUpdate() ? tr ("Library Update") : tr ("Software Update"));
      QString title = isNotSoftwareUpdate() ? "<b>" + tr ("An error occured while downloading %1") .arg(downloadName()) + "</b>" :
                                              "<b> An error occured while checking for update. </b>";
      QString text = tr("%1").arg(error);

      box.setText (title);
      box.setInformativeText (text);
      box.exec();
    }
}
