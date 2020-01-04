/*
 * Copyright (c) 2014-2016 Alex Spataru <alex_spataru@outlook.com>
 * Copyright (C) 2018 - 2020 Trevor SANDY. All rights reserved.
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

#include <QJsonValue>
#include <QJsonObject>
#include <QMessageBox>
#include <QApplication>
#include <QJsonDocument>
#include <QDesktopServices>

#include "updater.h"
#include "downloader.h"
#include "messageboxresizable.h"
#include "version.h"
#include "name.h"
#include "lpubalert.h"
#include "lpub_preferences.h"

Updater::Updater()
{
    m_url = "";
    m_openUrl = "";
    m_changelog = "";
    m_downloadUrl = "";
    m_latestVersion = "";
    m_customAppcast = false;
    m_notifyOnUpdate = true;
    m_notifyOnFinish = false;
    m_updateAvailable = false;
    m_downloaderEnabled = true;
    //m_moduleName = qApp->applicationName(); // Can't use this again cuz its set to the exe name.
    m_moduleName = QString("%1").arg(VER_PRODUCTNAME_STR);
    m_moduleVersion = qApp->applicationVersion();

    // LPub3D Mod
    m_fileName = "";
    m_latestRevision = "";
    m_availableVersions = "";
    m_winPortable = 0; // 0 = installer, 1 = x86Portable, 2 = x86_64Portable
    m_changeLogOnly = false;
    m_directDownload = false;
    m_promptedDownload = false;
    m_versionsRequest = false;
    m_moduleRevision = QString::fromLatin1(VER_REVISION_STR);

    //m_progressDialog = new ProgressDialog();
    // End Mod

    m_downloader = new Downloader();
    m_manager = new QNetworkAccessManager();

#if defined Q_OS_WIN
    m_platform = "windows-exe";
#elif defined Q_OS_MAC
    m_platform = "macos-dmg";
#elif defined Q_OS_LINUX
  #if defined DISTRO_PACKAGE
      m_platform = QString("linux-%1").arg(DISTRO_PACKAGE);
  #else
      m_platform = "linux";
  #endif
#elif defined Q_OS_ANDROID
    m_platform = "android";
#elif defined Q_OS_IOS
    m_platform = "ios";
#endif

    //qDebug() << qPrintable(QString("DISTRO_PACKAGE_CODE (m_platform): %1").arg(m_platform));

    setUserAgentString (QString ("%1/%2 (Qt; QSimpleUpdater)").arg (qApp->applicationName(),
                        qApp->applicationVersion()));
    // LPub3D Mod
    //connect (m_progressDialog, SIGNAL (cancelClicked()), this, SLOT (cancel()));
    // Mod End
    connect (m_downloader, SIGNAL (downloadFinished (QString, QString)),
             this,         SIGNAL (downloadFinished (QString, QString)));
    connect (m_manager,    SIGNAL (finished (QNetworkReply*)),
             this,           SLOT (onReply  (QNetworkReply*)));
    // LPub3D Mod
    connect (m_downloader, SIGNAL (downloadCancelled()),
             this,         SIGNAL (downloadCancelled()));
    // Mod End
}

Updater::~Updater()
{
  // LPub3D Mod
  // delete m_progressDialog;
  if (m_downloader)
    m_downloader->deleteLater();
  if (m_manager)
    m_manager->deleteLater();
  // Mod End
}

/**
 * Returns the URL of the update definitions file
 */
QString Updater::url() const
{
    return m_url;
}

/**
 * Returns the URL that the update definitions file wants us to open in
 * a web browser.
 *
 * \warning You should call \c checkForUpdates() before using this functio
 */
QString Updater::openUrl() const
{
    return m_openUrl;
}

/**
 * Returns the changelog defined by the update definitions file.
 * \warning You should call \c checkForUpdates() before using this function
 */
QString Updater::changelog() const
{
    return m_changelog;
}

/**
 * Returns the name of the module (if defined)
 */
QString Updater::moduleName() const
{
    return m_moduleName;
}

/**
 * Returns the platform key (be it system-set or user-set).
 * If you do not define a platform key, the system will assign the following
 * platform key:
 *    - On iOS: \c ios
 *    - On Mac OSX: \c osx
 *    - On Android: \c android
 *    - On GNU/Linux: \c linux
 *    - On Microsoft Windows: \c windows
 */
QString Updater::platformKey() const
{
    return m_platform;
}

/**
 * Returns the download URL defined by the update definitions file.
 * \warning You should call \c checkForUpdates() before using this function
 */
QString Updater::downloadUrl() const
{
    return m_downloadUrl;
}

/**
 * Returns the latest version defined by the update definitions file.
 * \warning You should call \c checkForUpdates() before using this function
 */
QString Updater::latestVersion() const
{
    return m_latestVersion;
}

/**
 * Returns the build number defined by the update definitions file.
 * \warning You should call \c checkForUpdates() before using this function
 */
QString Updater::latestRevision() const
{
    return m_latestRevision;
}

/**
 * Returns the user-agent header used by the client when communicating
 * with the server through HTTP
 */
QString Updater::userAgentString() const
{
    return m_userAgentString;
}

/**
 * Returns the "local" version of the installed module
 */
QString Updater::moduleVersion() const
{
    return m_moduleVersion;
}

/**
 * Returns the "local" revision number of the installed module
 */
QString Updater::moduleRevision() const
{
    return m_moduleRevision;
}

/**
 * Returns \c true if the updater should NOT interpret the downloaded appcast.
 * This is useful if you need to store more variables (or information) in the
 * JSON file or use another appcast format (e.g. XML)
 */
bool Updater::customAppcast() const
{
    return m_customAppcast;
}

/**
 * Returns \c true if the updater should notify the user when an update is
 * available.
 */
bool Updater::notifyOnUpdate() const
{
    return m_notifyOnUpdate;
}

/**
 * Returns \c true if the updater should notify the user when it finishes
 * checking for updates.
 *
 * \note If set to \c true, the \c Updater will notify the user even when there
 *       are no updates available (by congratulating him/her about being smart)
 */
bool Updater::notifyOnFinish() const
{
    return m_notifyOnFinish;
}

/**
 * Returns \c true if there is an update available.
 * \warning You should call \c checkForUpdates() before using this function
 */
bool Updater::updateAvailable() const
{
    return m_updateAvailable;
}

/**
 * Returns \c true if the integrated downloader is enabled.
 * \note If set to \c true, the \c Updater will open the downloader dialog if
 *       the user agrees to download the update.
 */
bool Updater::downloaderEnabled() const
{
    return m_downloaderEnabled;
}

/**
 * Returns \c true if the updater shall not intervene when the download has
 * finished (you can use the \c QSimpleUpdater signals to know when the
 * download is completed).
 */
bool Updater::customProcedure() const
{
    return m_downloader->customProcedure();
}

/**
 * Downloads and interpets the update definitions file referenced by the
 * \c url() function.
 */
void Updater::checkForUpdates()
{
    // LPub3D Mod
    // if (notifyOnFinish())
    // {
    //     if (customProcedure())
    //        m_progressDialog->setDownloadInfo();
    //     m_progressDialog->show();
    // }
    // Mod End
    QNetworkRequest request (url());
    if (!userAgentString().isEmpty())
        request.setRawHeader ("User-Agent", userAgentString().toUtf8());

    m_manager->get (request);
}

/**
 * Changes the \c url in which the \c Updater can find the update definitions
 * file.
 */
void Updater::setUrl (const QString& url)
{
    m_url = url;
}

/**
 * Changes the module \a name.
 * \note The module name is used on the user prompts. If the module name is
 *       empty, then the prompts will show the name of the application.
 */
void Updater::setModuleName (const QString& name)
{
    m_moduleName = name;
}

/**
 * If \a notify is set to \c true, then the \c Updater will notify the user
 * when an update is available.
 */
void Updater::setNotifyOnUpdate (const bool notify)
{
    m_notifyOnUpdate = notify;
}

/**
 * If \a notify is set to \c true, then the \c Updater will notify the user
 * when it has finished interpreting the update definitions file.
 */
void Updater::setNotifyOnFinish (const bool notify)
{
    m_notifyOnFinish = notify;
}

/**
 * Changes the user agent string used to identify the client application
 * from the server in a HTTP session.
 *
 * By default, the user agent will co
 */
void Updater::setUserAgentString (const QString& agent)
{
    m_userAgentString = agent;
    m_downloader->setUserAgentString (agent);
}

/**
 * Changes the module \a version
 * \note The module version is used to compare the local and remote versions.
 *       If the \a version parameter is empty, then the \c Updater will use the
 *       application version (referenced by \c qApp)
 */
void Updater::setModuleVersion (const QString& version)
{
    m_moduleVersion = version;
}

/**
 * Changes the module \a revision
 * \note The module revision is used to compare the local and remote revision.
 */
void Updater::setModuleRevision (const QString& revision)
{
    m_moduleRevision = revision;
}

/**
 * If the \a enabled parameter is set to \c true, the \c Updater will open the
 * integrated downloader if the user agrees to install the update (if any)
 */
void Updater::setDownloaderEnabled (const bool enabled)
{
    m_downloaderEnabled = enabled;
}

/**
 * Changes the platform key.
 * If the platform key is empty, then the system will use the following keys:
 *    - On iOS: \c ios
 *    - On macOS: \c macos-dmg
 *    - On Android: \c android
 *    - On GNU/Linux: \c linux
 *    - On Microsoft Windows: \c windows-exe
 */
void Updater::setPlatformKey (const QString& platformKey)
{
    m_platform = platformKey;
}

/**
 * If the \a customAppcast parameter is set to \c true, then the \c Updater
 * will not try to read the network reply from the server, instead, it will
 * emit the \c appcastDownloaded() signal, which allows the application to
 * read and interpret the appcast file by itself
 */
void Updater::setUseCustomAppcast (const bool customAppcast)
{
    m_customAppcast = customAppcast;
}

/**
 * If the \a custom parameter is set to \c true, the \c Updater will not try
 * to open the downloaded file. Use the signals fired by the \c QSimpleUpdater
 * to install the update from the downloaded file by yourself.
 */
void Updater::setCustomProcedure (const bool custom)
{
    m_downloader->setCustomProcedure (custom);
}

/**
 * Called when the download of the update definitions file is finished.
 */
void Updater::onReply (QNetworkReply* reply)
{
    // LPub3D Mod
    // Hide the progress dialog
    //if (m_progressDialog->isVisible())
    //    m_progressDialog->hide();
    // Mod End

    /* Check if we need to redirect */
    QUrl redirect = reply->attribute (
                        QNetworkRequest::RedirectionTargetAttribute).toUrl();
    if (!redirect.isEmpty()) {
        setUrl (redirect.toString());
        checkForUpdates();
        return;
    }

    /* There was a network error */
    if (reply->error() != QNetworkReply::NoError) {
        if (reply->errorString().contains(QRegExp("Host.*not found"))) {
            showErrorMessage("Error connecting to update server - newtork access may be interrupted: <br>" + reply->errorString() + ".");
        } else {
            showErrorMessage("Error connecting to update server: <br>" + reply->errorString() + ".");
        }
        emit checkingFinished (url());
        return;
    }

    /* The application wants to interpret the appcast by itself */
    if (customAppcast()) {
        emit appcastDownloaded (url(), reply->readAll());
        emit checkingFinished (url());
        return;
    }

    /* Try to create a JSON document from downloaded data */
    QJsonDocument document = QJsonDocument::fromJson (reply->readAll());

    /* JSON is invalid */
    if (document.isNull()) {
        showErrorMessage("Error retrieving JSON data: JSON document is empty.");
        emit checkingFinished (url());
        return;
    }

    /* Get the platform information */
    QJsonObject updates = document.object().value ("updates").toObject();
    QJsonObject platform = updates.value (platformKey()).toObject();

    /* Get the available versions information if requested */
    if (versionsRequested()){
      if (! platform.isEmpty()) {
          m_availableVersions = platform.value ("available-versions").toString();
      }
      setVersionsRequested(false);
      emit checkingFinished (url());
      return;
    }

    // LPub3D Mod
    /* Get update information */
    if (directDownload() || promptedDownload()) {

        m_openUrl = platform.value ("open-url").toString();
        m_fileName = platform.value( "download-name").toString();
        m_downloadUrl = platform.value ("download-url").toString();

        setUpdateAvailable(true);

    } else {

        bool _updateAvailable = false;

        if (moduleVersion() == qApp->applicationVersion()) {

            /* We are looking to update the latest version */
            m_openUrl = platform.value ("open-url").toString();
            m_latestVersion = platform.value ("latest-version").toString();
            m_latestRevision = platform.value ("latest-revision").toString();
            if (Preferences::portableDistribution) {
                if (QString::fromLatin1(VER_BUILD_ARCH_STR).toInt() == 64) {
                   m_downloadUrl = platform.value ("x86_64-win-portable-download-url").toString();
                   m_winPortable = 2;
                } else {
                   m_downloadUrl = platform.value ("x86-win-portable-download-url").toString();
                   m_winPortable = 1;
                }
                if (!m_downloadUrl.isEmpty()) {
                    setPortableDistro(true);
                }
            } else {
                m_downloadUrl = platform.value ("download-url").toString();
            }
            m_changelogUrl = platform.value ("changelog-url").toString();

            /* Compare latest and current version/revision */
             _updateAvailable = compare (latestVersion(), moduleVersion()) ||
                                compare (latestRevision(), moduleRevision());

        } else {
            /* We are looking to update an alternate version */
            bool rollback = compare (qApp->applicationVersion(),moduleVersion());

            QMessageBox box;
            box.setTextFormat (Qt::RichText);
            box.setIcon (QMessageBox::Warning);
            box.setStandardButtons (QMessageBox::Ok | QMessageBox::Cancel);
            box.setWindowFlags (Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
            box.setWindowTitle(tr ("Software Update"));

            QString title = "<b>" + tr ("This is a version %1 update.")
                                        .arg(rollback ? "rollback" : "rollforwad")
                                        + "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</b>";
            QString text = tr ("Your current %1 install is v%2 and the requested update is v%3.\n"
                               "Be sure you are comfortable with this type of update as advanced "
                               "system configuration may be necessary and user data loss may occur.\n\n"
                               "Click \"OK\" to continue.")
                               .arg(moduleName(),qApp->applicationVersion(),moduleVersion());

            box.setText (title);
            box.setInformativeText (text);

            if (box.exec() == QMessageBox::Ok) {
                QStringList versions = platform.value ("available-versions").toString().split(",");
                QString latestVersion = platform.value ("latest-version").toString();

                // sort versions descending 0...10...
                std::sort(versions.begin(),versions.end());
                //qDebug() << "Versions (sorted): " << versions;
                for (int i = 0; i < versions.size(); i++) {
                    if (versions[i] == moduleVersion()) {
                        _updateAvailable = true;
                        int updateIndex = i;
                        (updateIndex + 1) == versions.size() ? updateIndex = i : updateIndex = updateIndex + 1;
                        //qDebug() << "Update to version: " << versions[updateIndex];
                        if (versions[updateIndex] == latestVersion){
                            // Update to version is same as latest version - i.e. reinstall latest version
                            m_openUrl = platform.value ("open-url").toString();
                            m_latestVersion = platform.value ("latest-version").toString();
                            m_latestRevision = platform.value ("latest-revision").toString();
                            m_changelogUrl = platform.value ("changelog-url").toString();
                            if (Preferences::portableDistribution) {
                                if (QString::fromLatin1(VER_BUILD_ARCH_STR).toInt() == 64) {
                                   m_downloadUrl = platform.value ("x86_64-win-portable-download-url").toString();
                                   m_winPortable = 2;
                                } else {
                                   m_downloadUrl = platform.value ("x86-win-portable-download-url").toString();
                                   m_winPortable = 1;
                                }
                                if (!m_downloadUrl.isEmpty()) {
                                    setPortableDistro(true);
                                }
                            } else {
                                m_downloadUrl = platform.value ("download-url").toString();
                            }
                        } else {
                            // Update to version is other than the latest version
                            QString distro_suffix = m_platform.section("-",1);
                            QJsonObject altVersion = platform.value(QString("alternate-version-%1-%2")
                                                                    .arg(versions[updateIndex])
                                                                    .arg(distro_suffix)).toObject();
                            if (altVersion.isEmpty()) {
                                showErrorMessage("Unable to retrieve version " + versions[updateIndex] + ". Version number not found.");
                                return;
                            }
                            m_openUrl = altVersion.value ("open-url").toString();
                            m_latestVersion = altVersion.value ("latest-version").toString();
                            m_latestRevision = altVersion.value ("latest-revision").toString();
                            m_changelogUrl = altVersion.value ("changelog-url").toString();
                            if (Preferences::portableDistribution) {
                                if (QString::fromLatin1(VER_BUILD_ARCH_STR).toInt() == 64) {
                                   m_downloadUrl = altVersion.value ("x86_64-win-portable-download-url").toString();
                                   m_winPortable = 2;
                                } else {
                                   m_downloadUrl = altVersion.value ("x86-win-portable-download-url").toString();
                                   m_winPortable = 1;
                                }
                                if (!m_downloadUrl.isEmpty()) {
                                    setPortableDistro(true);
                                }
                            } else {
                                m_downloadUrl = altVersion.value ("download-url").toString();
                            }
                        }
                        break;
                    }
                }
            } else {return;}
        }

        if (m_latestRevision.isEmpty())
            m_latestRevision = "0";

        if (_updateAvailable || getChangeLogOnly()) {
            QUrl _url(m_changelogUrl);
            changeLogRequest(_url);
        }

        /* All done if we only wanted to get the change log */      
        if (getChangeLogOnly()) {
            emit checkingFinished (url());
            return;
        }

        setUpdateAvailable (_updateAvailable);
    }
    // Mod End
    emit checkingFinished (url());
}

/**
 * Prompts the user based on the value of the \a available parameter and the
 * settings of this instance of the \c Updater class.
 */
void Updater::setUpdateAvailable (const bool available)
{
    m_updateAvailable = available;

    // LPub3D Mod
    if (directDownload()) {
        m_downloader->setFileName(m_fileName);
        m_downloader->startDownload (QUrl (downloadUrl()));
        return;
    }

    // Get the application icon as a pixmap
    QPixmap _icon = QPixmap(":/icons/lpub96.png");
    if (_icon.isNull())
        _icon = QPixmap (":/icons/update.png");

    if (promptedDownload()) {

        QMessageBox box;
        box.setWindowIcon(QIcon());
        box.setIconPixmap (_icon);
        box.setTextFormat (Qt::RichText);
        box.setWindowFlags (Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);

        box.setWindowTitle(tr ("Library Update"));

        QString title = "<b>" + tr ("Download LDraw library archive %1 ?")
                                    .arg(fileName()) + "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</b>";
        QString text = tr ("The latest version of %1 will be downloaded and written to\n'%2'.\n\n"
                           "NOTICE: %3 will restart to properly load the LDraw %4 Library archive.\n\n"
                           "Click \"OK\" to continue.")
                            .arg(fileName())
                            .arg(QDir::toNativeSeparators(Preferences::lpubDataPath+"/libraries/"+fileName()))
                            .arg(VER_PRODUCTNAME_STR)
                            .arg(fileName() == VER_LDRAW_OFFICIAL_ARCHIVE ? "Official" : "Unofficial");
        box.setText (title);
        box.setInformativeText (text);
        box.setDefaultButton   (QMessageBox::Ok);
        box.setStandardButtons (QMessageBox::Ok | QMessageBox::Cancel);

        if (box.exec() == QMessageBox::Ok) {
            m_downloader->setFileName(fileName());
            m_downloader->startDownload (QUrl (downloadUrl()));
        } else {
            emit downloadCancelled();
            m_manager->disconnect();
        }

    } else {

        QMessageBoxResizable box;
        box.setWindowIcon(QIcon());
        box.setIconPixmap (_icon);
        box.setTextFormat (Qt::RichText);
        box.setWindowFlags (Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);

        box.setWindowTitle(tr ("Software Update"));

        if (updateAvailable() && (notifyOnUpdate() || notifyOnFinish())) {
            QString versionStr = compare (latestRevision(), moduleRevision()) ? "revision" : "version";
            QString moduleDescStr = m_winPortable == 2 ? " x86_64 Windows Portable" : m_winPortable == 1 ? " x86 Windows Portable" : "" ;
            QString title = "<b>" + tr ("A new %1 of %2 is available!")
                                        .arg(versionStr)
                                        .arg (moduleName() + moduleDescStr) +
                                        "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"
                                        "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"
                                        "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"
                                        "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</b>";
            QString versionMessage;
            if (latestVersion() == moduleVersion()) {
                versionMessage = tr ("revision %1")
                                     .arg(moduleRevision());
            } else {
                versionMessage = tr ("v%1 revision %2")
                                     .arg(moduleVersion())
                                     .arg(moduleRevision());
            }
            QString text = tr ("%1 v%2 revision %3 is available - you have %4.\n\nClick \"OK\" to download it now.")
                    .arg(moduleName())
                    .arg(latestVersion())
                    .arg(latestRevision())
                    .arg(versionMessage);

            box.setText (title);
            box.setInformativeText (text);
            box.setDetailedText(changelog(), compare(latestVersion(), PLAINTEXT_CHANGE_LOG_CUTOFF_VERSION));
            box.setStandardButtons (QMessageBox::Ok | QMessageBox::Cancel);
            box.setDefaultButton   (QMessageBox::Ok);

            if (box.exec() == QMessageBox::Ok) {
                // LPub3D Mod
                if (!openUrl().isEmpty() && !downloaderEnabled())
                    QDesktopServices::openUrl (QUrl (openUrl()));
                // Mod End
                else if (downloaderEnabled()) {
                    m_downloader->setUrlId (url());
                    m_downloader->setFileName (downloadUrl().split ("/").last());
                    m_downloader->startDownload (QUrl (downloadUrl()));
                }

                else
                    QDesktopServices::openUrl (QUrl (downloadUrl()));
            } else {
                m_manager->disconnect();
                emit downloadCancelled();
            }
        }
        else if (notifyOnFinish()) {

            QMessageBox box;
            box.setWindowIcon(QIcon());
            box.setIconPixmap (_icon);
            box.setTextFormat (Qt::RichText);
            box.setWindowFlags (Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);

            box.setStandardButtons (QMessageBox::Close);

            box.setText ("<b>" + tr ("The latest version of %1 is already installed.")
                         .arg(moduleName()) + "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</b>");
            box.setInformativeText (
                        tr ("%1 %2.%3 is currently the latest version available.")
                            .arg (moduleName(),moduleVersion(),moduleRevision()));
            box.exec();
        }
    }
    // Mod End
}

/**
 * Compares the two version strings (\a x and \a y).
 *     - If \a x is greater than \y, this function returns \c true.
 *     - If \a y is greater than \x, this function returns \c false.
 *     - If both versions are the same, this function returns \c false.
 */
bool Updater::compare (const QString& x, const QString& y)
{
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

// LPub3D Mod
/**
 * Disconnects the network access manager when the user
 * clicks on the "cancel" button in the progress dialog.
 */
void Updater::cancel (void)
{
    m_manager->disconnect();
    emit checkingCancelled();
}

/**
 * If \a enabled is set to true, then the user will be able to directly
 * directly download content without running the version compare logic
 * This is used for direct content download like library files
 */
void Updater::setDirectDownload (const bool& enabled) {
    m_directDownload = enabled;
}

/**
 * If \a enabled is set to true, then the user will be prompted
 * to download the requeste dcontent with an option to proceed.
 * This is used for in-applicaton content download like library files
 */
void Updater::setPromptedDownload (const bool& enabled) {
    m_promptedDownload = enabled;
}

/**
 * Sets the flag to indicate download is a portable distribution
 */
void Updater::setPortableDistro (const bool& enabled) {
    m_downloader->setPortableDistro (enabled);
}

/**
 * Sets the flag to display HTTP redirects
 */
void Updater::setShowRedirects (const bool& enabled) {
    m_downloader->setShowRedirects (enabled);
}

/**
 * Sets the path for download content
 */
void Updater::setDownloadDir (const QString& path) {
    m_downloader->setDownloadDir (path);
}

/**
 * Sets the flage to return the changelog of the \c Updater instance
 * registered with the given \a url. The full update check is not performed.
 *
 */
void Updater::setChangelogOnly (const bool& enabled) {
    m_changeLogOnly = enabled;
}

/**
 * Set the version request flag
 */
void Updater::setVersionsRequested(const bool& version) {
    m_versionsRequest = version;
}

/**
 * Downloads the update definitions file and analyzes it to determine the
 * available versions for the platform
 */
void Updater::retrieveAvailableVersions() {
    setVersionsRequested(true);
    QNetworkRequest versionsRequest (url());
    if (!userAgentString().isEmpty())
      versionsRequest.setRawHeader ("User-Agent", userAgentString().toUtf8());

    m_manager->get (versionsRequest);
}

/**
 * Return the version request flag
 */
bool Updater::versionsRequested() {
    return m_versionsRequest;
}

/**
 * Returns the changeLogOnly flag
 */
bool Updater::getChangeLogOnly() const
{
    return m_changeLogOnly;
}

/**
 * Returns the download file name
 * This is used for non-software updates only
 */
QString Updater::fileName() const {
    return m_fileName;
}

/**
 * Returns the download local path
 */
QString Updater::downloadDir() const {
    return m_downloader->downloadDir();
}

/**
 * Returns \c if the update request is a direct download
 * and and bypasses the version compare logic.
 */
bool Updater::directDownload() const {
    return m_directDownload;
}

/**
 * Returns \c if the update request is a prompted download
 * which presents a notification and option to proceed.
 */
bool Updater::promptedDownload() const {
    return m_promptedDownload;
}

/**
 * Returns the available versions for download, which is
 * automatically retrieved at application startup.
 */
QString Updater::getAvailableVersions() const {
    return m_availableVersions;
}

/**
 * Returns the remote module revision of the \c Updater instance registered with
 * the given \a url.
 *
 * \warning You should call \c checkForUpdates() before using this function
 * \note If an \c Updater instance registered with the given \a url is not
 *       found, that \c Updater instance will be initialized automatically
 */
QString QSimpleUpdater::getLatestRevision (const QString& url) const
{
    return getUpdater (url)->latestRevision();
}

/**
 * Returns the module revision of the \c Updater instance registered with the
 * given \a url.
 *
 * \note If an \c Updater instance registered with the given \a url is not
 *       found, that \c Updater instance will be initialized automatically
 */
QString QSimpleUpdater::getModuleRevision (const QString& url) const
{
    return getUpdater (url)->moduleRevision();
}

/**
 * Alerts the user when there is an error.
 */
void Updater::showErrorMessage (QString error)
{
  if (notifyOnFinish()) // ! not silent
    {
      // Hide the progress dialog
      //if (m_progressDialog->isVisible())
      //    m_progressDialog->hide();

      QMessageBox box;
      box.setTextFormat (Qt::RichText);
      box.setIcon (QMessageBox::Critical);
      box.setStandardButtons (QMessageBox::Ok );
      box.setWindowFlags (Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
      box.setWindowTitle(customProcedure() ? tr ("Library Update") : tr ("Software Update"));
      QString title = customProcedure() ?
            "<b>" + tr ("An error occured while downloading %1.&nbsp;&nbsp;&nbsp;&nbsp;")
                        .arg(fileName().isEmpty() ? QString("archive ibrary") : fileName()) + "</b>" :
            "<b>" + tr ("An error occured while checking for %1 update.&nbsp;&nbsp;&nbsp;")
                        .arg(moduleName().isEmpty() ? QString(VER_PRODUCTNAME_STR) : moduleName()) + "</b>";
      QString text = tr("%1").arg(error);

      box.setText (title);
      box.setInformativeText (text);
      box.exec();
    }
}

/**
 * Request the change log.
 */
void Updater::changeLogRequest(const QUrl &_url){

    QNetworkAccessManager *_manager = new QNetworkAccessManager (this);

    connect (_manager, SIGNAL (finished (QNetworkReply *)),
             this,     SLOT (changeLogReply (QNetworkReply *)));

    QNetworkRequest updateRequest (_url.toString());
    if (!userAgentString().isEmpty())
        updateRequest.setRawHeader ("User-Agent", userAgentString().toUtf8());

    _manager->get(updateRequest);

    QEventLoop wait;
    wait.connect(this, SIGNAL(changeLogReplyFinished()),&wait, SLOT(quit()));
    wait.exec();
}

/**
 * Reads and analyzes the downloaded change log.
 */
void Updater::changeLogReply (QNetworkReply *reply){
     if (reply->error() == QNetworkReply::NoError) {

        /* Get redirection url */
        QVariant redirectionTarget = reply->attribute(QNetworkRequest::RedirectionTargetAttribute);
        if (!redirectionTarget.isNull()) {
            QUrl url(m_changelogUrl);
            QUrl newUrl = url.resolved(redirectionTarget.toUrl());
            url = newUrl;
            emit changeLogReplyFinished();
            reply->deleteLater();
            changeLogRequest(url);
            return;
        }
        QString _reply = QString::fromUtf8 (reply->readAll());
        m_changelog = _reply;

     } else {
         showErrorMessage("Error receiving change log: " + reply->errorString() + ".");
     }
     emit changeLogReplyFinished();
}
    // Mod End
