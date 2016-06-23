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

#ifndef _QSIMPLEUPDATER_UPDATER_H
#define _QSIMPLEUPDATER_UPDATER_H

#include <QUrl>
#include <QObject>
#include <QSettings>
#include <QNetworkReply>
#include <QNetworkAccessManager>

#include <qsimpleupdater.h>
#include <progress_dialog.h>

class Downloader;

///
/// The Updater class is in charge of downloading and analyzing
/// the appcast and "reacting" based on the options given by the
/// user/developer/application.
///
class QSU_DECL Updater : public QObject {
    Q_OBJECT

  public:
    Updater();
    ~Updater();

    ///
    /// Returns the AppCast URL (from which we extract the update definitions)
    ///
    QString url() const;

    ///
    /// Returns the current platform key, which is used to differentiate the
    /// different download links (and download versions) based on the current
    /// operating system.
    ///
    /// You can modify this value with the \c setPlatformKey() function
    ///
    QString platformKey() const;

    ///
    /// Returns \c true if the class is set to notify the user when an update
    /// is found online. By default this value is set to \c true.
    ///
    bool showUpdateNotifications() const;

    ///
    /// Returns \c true if the class is set to notify the user when it finishes
    /// checking for updates (even if there are no updates available).
    /// By default this value is set to \c true.
    ///
    bool showAllNotifications() const;

    ///
    /// Returns \c true if the updater found an updated version of the
    /// application/module online.
    ///
    bool updateAvailable() const;

    ///
    /// Returns \c if the updater allows the integrated downloader to download
    /// and install the update (if aplicable).
    ///
    bool enableDownloader() const;

    ///
    /// Returns \c if the update request is a direct download
    /// and and bypasses the version compare logic.
    ///
    bool directDownload() const;

    ///
    /// Returns \c if the update request is a prompted download
    /// which presents a notification and option to proceed.
    ///
    bool promptedDownload() const;

    ///
    /// Returns the latest changelog
    ///
    QString changelog() const;

    ///
    /// Returns the URL from where we can download the update
    ///
    QString downloadUrl() const;

    ///
    /// Returns the download local path
    ///
    QString localDownloadPath() const;

    ///
    /// Returns the latest version online
    ///
    QString latestVersion() const;

    ///
    /// Returns the application name, which can be set manually or
    /// automatically using the \c qApp->applicationName() function.
    ///
    QString moduleName() const;

    ///
    /// Returns the download content name,
    ///
    QString downloadName() const;

    ///
    /// Returns the application version, which can be set manually or
    /// automatically using the \c qApp->applicationVersion() function.
    ///
    QString moduleVersion() const;

    ///
    /// Returns \c true if the downloader will not attempt to install the
    /// downloaded file.
    ///
    /// This can be useful if you want to use the \c downloadFinished() signal
    /// to implement your own install procedures.
    ///
    bool useCustomInstallProcedures() const;

    ///
    /// Sets the type of content update or download
    ///
    bool isNotSoftwareUpdate () const;

  public slots:
    ///
    /// Downloads the update definitions file and analyzes it to determine the
    /// latest version and the download links
    ///
    void checkForUpdates();

    ///
    /// Changes the \a url from where we download the update definitions
    ///
    void setUrl (const QString& url);

    ///
    /// If \c notify is set to true, the \c QSimpleUpdater will notify the user
    /// when an update is available.
    ///
    /// If \c notify is set to false, the \c QSimpleUpdater will not notify the
    /// user when an update is available.
    ///
    void setShowUpdateNotifications (const bool& notify);

    ///
    /// If set to \c true, the updater will notify the user when it finishes
    /// checking for updates (even where there are no updates available).
    ///
    /// If set to \c false (default), the updater will only notify the user
    /// when there is an update available (if setShowUpdateNotifications() is \c true).
    ///
    void setShowAllNotifications (const bool& notify);

    ///
    /// Changes the name of the module, this can be useful in large applications
    /// that only need to update certain components of them (e.g. plugins).
    ///
    void setModuleName (const QString& name);

    ///
    /// Changes the name of the download content.
    ///
    void setDownloadName (const QString& name);

    ///
    /// Changes the version of the module, this can be useful in large
    /// applications that only need to update certain components of them
    /// (e.g. plugins).
    ///
    void setModuleVersion (const QString& version);

    ///
    /// If \a enabled is set to true, then the user will be able to download
    /// and install updates directly from the application, without the need
    /// of opening the download URL from a browser and manually installing
    /// the update.
    ///
    void setEnableDownloader (const bool& enabled);

    ///
    /// If \a enabled is set to true, then the user will be able to directly
    /// directly download content without running the version compare logic
    /// This is used for direct content download like library files
    ///
    void setDirectDownload (const bool& enabled);

    ///
    /// If \a enabled is set to true, then the user will be prompted
    /// to download the requeste dcontent with an option to proceed.
    /// This is used for in-applicaton content download like library files
    ///
    void setPromptedDownload (const bool& enabled);

    ///
    /// Sets the path for download content
    ///
    void setLocalDownloadPath (const QString& path);

    ///
    /// Changes the platform key/id. This can be useful if the update depends
    /// on more than the underlying operating system on which the application
    /// runs.
    ///
    void setPlatformKey (const QString& platformKey);

    ///
    /// If \c custom is set to true, then the Downloader will not attempt to
    /// open or install the downloaded updates. This can be useful if you want
    /// to implement your own install procedures using the \c downloadFinished()
    /// signal.
    ///
    void setUseCustomInstallProcedures (const bool& custom);

    ///
    /// Sets the type of content update or download
    ///
    void setIsNotSoftwareUpdate (const bool& enabled);

  signals:
    ///
    /// Emitted when the download definitions have been downloaded and analyzed.
    ///
    void checkingFinished (const QString& url);

    ///
    /// Emitted when the download has finished.
    /// You can use this to implement your own procedures to install the
    /// downloaded updates.
    ///
    void downloadFinished (const QString& url, const QString& filepath);   

    ///
    /// Emitted when the change log download is complete
    ///
    void changeLogReplyFinished();

  private slots:
    ///
    /// Reads and analyzes the downloaded update definition.
    ///
    void onReply (QNetworkReply* reply);

    ///
    /// Reads and analyzes the downloaded change log.
    ///
    void changeLogReply (QNetworkReply* reply);

    ///
    /// Changes the appropiate internal values, shows notifications (if allowed)
    /// and (if allowed) initializes the internal downloader.
    ///
    void setUpdateAvailable (const bool& available);

    ///
    /// Shows notification and initializes the
    /// internal downloader for content downloads.
    ///
    void setDownloadAvailable ();

    ///
    /// Disconnects the network access manager when the user
    /// clicks on the "cancel" button in the progress dialog.
    ///
    void cancel (void);

    ///
    /// Alerts the user when the download of version information
    /// data is corrupted.
    ///
    void showErrorMessage (QString error);

  private:
    ///
    /// Returns \c true if version \a x is greater than version \a y.
    /// This is used to determine if the online version is greater than the
    /// installed version of the module.
    ///
    bool compare (const QString& x, const QString& y);

  private:
    QString m_url;

    bool m_isNotSoftwareUpdate;
    bool m_showUpdateNotifications;
    bool m_showAllNotifications;
    bool m_updateAvailable;
    bool m_enableDownloader;
    bool m_directDownload;
    bool m_promptedDownload;

    QString m_openUrl;
    QString m_platform;
    QString m_changelog;
    QString m_moduleName;
    QString m_downloadUrl;
    QString m_downloadName;
    QString m_moduleVersion;
    QString m_latestVersion;
    QString m_localDownloadPath;

    ProgressDialog *m_progressDialog;
    Downloader* m_downloader;
    QNetworkRequest m_updateRequest;
    QNetworkAccessManager* m_manager;
};

#endif
