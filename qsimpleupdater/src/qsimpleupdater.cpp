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

#include "updater.h"
#include "qsimpleupdater.h"

static QList<QString> URLS;
static QList<Updater*> UPDATERS;

QSimpleUpdater::~QSimpleUpdater()
{
    URLS.clear();

    foreach (Updater* updater, UPDATERS)
        updater->deleteLater();

    UPDATERS.clear();
}

/**
 * Returns the only instance of the class
 */
QSimpleUpdater* QSimpleUpdater::getInstance()
{
    static QSimpleUpdater updater;
    return &updater;
}

/**
 * Returns \c true if the \c Updater instance registered with the given \a url
 * uses a custom appcast format and/or allows the application to read and
 * interpret the downloaded appcast file
 *
 * \note If an \c Updater instance registered with the given \a url is not
 *       found, that \c Updater instance will be initialized automatically
 */
bool QSimpleUpdater::usesCustomAppcast (const QString& url) const
{
    return getUpdater (url)->customAppcast();
}

/**
 * Returns \c true if the \c Updater instance registered with the given \a url
 * shall notify the user when an update is available.
 *
 * \note If an \c Updater instance registered with the given \a url is not
 *       found, that \c Updater instance will be initialized automatically
 */
bool QSimpleUpdater::getNotifyOnUpdate (const QString& url) const
{
    return getUpdater (url)->notifyOnUpdate();
}

/**
 * Returns \c true if the \c Updater instance registered with the given \a url
 * shall notify the user when it finishes checking for updates.
 *
 * \note If an \c Updater instance registered with the given \a url is not
 *       found, that \c Updater instance will be initialized automatically
 */
bool QSimpleUpdater::getNotifyOnFinish (const QString& url) const
{
    return getUpdater (url)->notifyOnFinish();
}

/**
 * Returns \c true if the \c Updater instance registered with the given \a url
 * has an update available.
 *
 * \warning You should call \c checkForUpdates() before using this function
 * \note If an \c Updater instance registered with the given \a url is not
 *       found, that \c Updater instance will be initialized automatically
 */
bool QSimpleUpdater::getUpdateAvailable (const QString& url) const
{
    return getUpdater (url)->updateAvailable();
}

/**
 * Returns \c true if the \c Updater instance registered with the given \a url
 * has the integrated downloader enabled.
 *
 * \note If an \c Updater instance registered with the given \a url is not
 *       found, that \c Updater instance will be initialized automatically
 */
bool QSimpleUpdater::getDownloaderEnabled (const QString& url) const
{
    return getUpdater (url)->downloaderEnabled();
}

/**
 * Returns \c true if the \c Updater instance registered with the given \a url
 * shall try to open the downloaded file.
 *
 * If you want to implement your own way to handle the downloaded file, just
 * bind to the \c downloadFinished() signal and disable the integrated
 * downloader with the \c setCustomProcedure() function.
 *
 * \note If an \c Updater instance registered with the given \a url is not
 *       found, that \c Updater instance will be initialized automatically
 */
bool QSimpleUpdater::usesCustomProcedure (const QString& url) const
{
    return getUpdater (url)->customProcedure();
}

/**
 * Returns the URL to open in a web browser of the \c Updater instance
 * registered with the given \a url.
 *
 * \note If the module name is empty, then the \c Updater will use the
 *       application name as its module name.
 * \note If an \c Updater instance registered with the given \a url is not
 *       found, that \c Updater instance will be initialized automatically
 */
QString QSimpleUpdater::getOpenUrl (const QString& url) const
{
    return getUpdater (url)->openUrl();
}

/**
 * Returns the changelog of the \c Updater instance registered with the given
 * \a url.
 *
 * \warning You should call \c checkForUpdates() before using this function
 * \note If an \c Updater instance registered with the given \a url is not
 *       found, that \c Updater instance will be initialized automatically
 */
QString QSimpleUpdater::getChangelog (const QString& url) const
{
    return getUpdater (url)->changelog();
}
// LPub3D Mod
/**
 * Sets the flage to return the changelog of the \c Updater instance
 * registered with the given \a url. The full update check is not performed.
 *
 * \warning You should call \c checkForUpdates() before using this function
 * \note If an \c Updater instance registered with the given \a url is not
 *       found, that \c Updater instance will be initialized automatically
 */
void QSimpleUpdater::setChangelogOnly (const QString& url,
                                          const bool& enabled)
{
    return getUpdater (url)->setChangelogOnly(enabled);
}

bool QSimpleUpdater::getChangelogOnly (const QString& url) const
{
    return getUpdater (url)->getChangeLogOnly();
}

// Mod End

/**
 * Returns the module name of the \c Updater instance registered with the given
 * \a url.
 *
 * \note If the module name is empty, then the \c Updater will use the
 *       application name as its module name.
 * \note If an \c Updater instance registered with the given \a url is not
 *       found, that \c Updater instance will be initialized automatically
 */
QString QSimpleUpdater::getModuleName (const QString& url) const
{
    return getUpdater (url)->moduleName();
}

/**
 * Returns the download URL of the \c Updater instance registered with the given
 * \a url.
 *
 * \warning You should call \c checkForUpdates() before using this function
 * \note If an \c Updater instance registered with the given \a url is not
 *       found, that \c Updater instance will be initialized automatically
 */
QString QSimpleUpdater::getDownloadUrl (const QString& url) const
{
    return getUpdater (url)->downloadUrl();
}

/**
 * Returns the platform key of the \c Updater registered with the given \a url.
 * If you do not define a platform key, the system will assign the following
 * platform key:
 *    - On iOS: \c ios
 *    - On Mac OSX: \c osx
 *    - On Android: \c android
 *    - On GNU/Linux: \c linux
 *    - On Microsoft Windows: \c windows
 *
 * \note If an \c Updater instance registered with the given \a url is not
 *       found, that \c Updater instance will be initialized automatically
 */
QString QSimpleUpdater::getPlatformKey (const QString& url) const
{
    return getUpdater (url)->platformKey();
}

/**
 * Returns the remote module version of the \c Updater instance registered with
 * the given \a url.
 *
 * \warning You should call \c checkForUpdates() before using this function
 * \note If an \c Updater instance registered with the given \a url is not
 *       found, that \c Updater instance will be initialized automatically
 */
QString QSimpleUpdater::getLatestVersion (const QString& url) const
{
    return getUpdater (url)->latestVersion();
}

/**
 * Returns the module version of the \c Updater instance registered with the
 * given \a url.
 *
 * \note If the module version is empty, then the \c Updater will use the
 *       application version as its module version.
 * \note If an \c Updater instance registered with the given \a url is not
 *       found, that \c Updater instance will be initialized automatically
 */
QString QSimpleUpdater::getModuleVersion (const QString& url) const
{
    return getUpdater (url)->moduleVersion();
}

/**
 * Returns the user-agent string used by the updater to communicate with
 * the remote HTTP(S) server.
 *
 * \note If an \c Updater instance registered with the given \a url is not
 *       found, that \c Updater instance will be initialized automatically
 */
QString QSimpleUpdater::getUserAgentString (const QString& url) const
{
    return getUpdater (url)->userAgentString();
}

/**
 * Instructs the \c Updater instance with the registered \c url to download and
 * interpret the update definitions file.
 *
 * \note If an \c Updater instance registered with the given \a url is not
 *       found, that \c Updater instance will be initialized automatically
 */
void QSimpleUpdater::checkForUpdates (const QString& url)
{
    getUpdater (url)->checkForUpdates();
}

/**
 * Changes the module \a name of the \c Updater instance registered at the
 * given \a url.
 *
 * \note If an \c Updater instance registered with the given \a url is not
 *       found, that \c Updater instance will be initialized automatically
 * \note The module name is used on the user prompts. If the module name is
 *       empty, then the prompts will show the name of the application.
 */
void QSimpleUpdater::setModuleName (const QString& url, const QString& name)
{
    getUpdater (url)->setModuleName (name);
}

/**
 * If \a notify is set to \c true, then the \c Updater instance registered with
 * the given \a url will notify the user when an update is available.
 *
 * \note If an \c Updater instance registered with the given \a url is not
 *       found, that \c Updater instance will be initialized automatically
 */
void QSimpleUpdater::setNotifyOnUpdate (const QString& url,
                                        const bool notify)
{
    getUpdater (url)->setNotifyOnUpdate (notify);
}

/**
 * If \a notify is set to \c true, then the \c Updater instance registered with
 * the given \a url will notify the user when it has finished interpreting the
 * update definitions file.
 *
 * \note If an \c Updater instance registered with the given \a url is not
 *       found, that \c Updater instance will be initialized automatically
 */
void QSimpleUpdater::setNotifyOnFinish (const QString& url,
                                        const bool notify)
{
    getUpdater (url)->setNotifyOnFinish (notify);
}

/**
 * Changes the platform key of the \c Updater isntance registered at the given
 * \a url.
 *
 * If the platform key is empty, then the system will use the following keys:
 *    - On iOS: \c ios
 *    - On Mac OSX: \c osx
 *    - On Android: \c android
 *    - On GNU/Linux: \c linux
 *    - On Microsoft Windows: \c windows
 *
 * \note If an \c Updater instance registered with the given \a url is not
 *       found, that \c Updater instance will be initialized automatically
 */
void QSimpleUpdater::setPlatformKey (const QString& url,
                                     const QString& platform)
{
    getUpdater (url)->setPlatformKey (platform);
}

/**
 * Changes the module \version of the \c Updater instance registered at the
 * given \a url.
 *
 * \note The module version is used to compare it with the remove version.
 *       If the module name is empty, then the \c Updater instance will use the
 *       application version.
 */
void QSimpleUpdater::setModuleVersion (const QString& url,
                                       const QString& version)
{
    getUpdater (url)->setModuleVersion (version);
}

/**
 * If the \a enabled parameter is set to \c true, the \c Updater instance
 * registered with the given \a url will open the integrated downloader
 * if the user agrees to install the update (if any).
 *
 * \note If an \c Updater instance registered with the given \a url is not
 *       found, that \c Updater instance will be initialized automatically
 */
void QSimpleUpdater::setDownloaderEnabled (const QString& url,
                                           const bool enabled)
{
    getUpdater (url)->setDownloaderEnabled (enabled);
}

/**
 * Changes the user-agent string used by the updater to communicate
 * with the remote server
 *
 * \note If an \c Updater instance registered with the given \a url is not
 *       found, that \c Updater instance will be initialized automatically
 */
void QSimpleUpdater::setUserAgentString (const QString& url,
                                         const QString& agent)
{
    getUpdater (url)->setUserAgentString (agent);
}

/**
 * If the \a customAppcast parameter is set to \c true, then the \c Updater
 * will not try to read the network reply from the server, instead, it will
 * emit the \c appcastDownloaded() signal, which allows the application to
 * read and interpret the appcast file by itself.
 *
 * \note If an \c Updater instance registered with the given \a url is not
 *       found, that \c Updater instance will be initialized automatically
 */
void QSimpleUpdater::setUseCustomAppcast (const QString& url,
                                          const bool customAppcast)
{
    getUpdater (url)->setUseCustomAppcast (customAppcast);
}

/**
 * If the \a custom parameter is set to \c true, the \c Updater instance
 * registered with the given \a url will not try to open the downloaded file.
 *
 * If you want to implement your own way to handle the downloaded file, just
 * bind to the \c downloadFinished() signal and disable the integrated
 * downloader with the \c setCustomProcedure() function.
 *
 * \note If an \c Updater instance registered with the given \a url is not
 *       found, that \c Updater instance will be initialized automatically
 */
void QSimpleUpdater::setCustomProcedure (const QString& url,
        const bool custom)
{
    getUpdater (url)->setCustomProcedure (custom);
}

/**
 * Returns the \c Updater instance registered with the given \a url.
 *
 * If an \c Updater instance registered with teh given \a url does not exist,
 * this function will create it and configure it automatically.
 */
Updater* QSimpleUpdater::getUpdater (const QString& url) const
{
    if (!URLS.contains (url)) {
        Updater* updater = new Updater;
        updater->setUrl (url);

        URLS.append (url);
        UPDATERS.append (updater);

        connect (updater, SIGNAL (checkingFinished  (QString)),
                 this,    SIGNAL (checkingFinished  (QString)));
        connect (updater, SIGNAL (downloadFinished  (QString, QString)),
                 this,    SIGNAL (downloadFinished  (QString, QString)));
        connect (updater, SIGNAL (appcastDownloaded (QString, QByteArray)),
                 this,    SIGNAL (appcastDownloaded (QString, QByteArray)));
        // LPub3D Mod
        connect (updater, SIGNAL (downloadCancelled ()),
                 this,    SIGNAL (cancel ()));
        connect (updater, SIGNAL (checkingCancelled ()),
                 this,    SIGNAL (cancel ()));
        // Mod End
    }

    return UPDATERS.at (URLS.indexOf (url));
}

// LPub3D Mod
/**
 * Returns the available versions registered for the platform \c url
 */
void QSimpleUpdater::retrieveAvailableVersions (const QString& url) const {
    getUpdater (url)->retrieveAvailableVersions();
}

/**
 * If \a enabled is set to true, then the user will be prompted
 * to download the requeste dcontent with an option to proceed.
 * This is used for in-applicaton content download like library files
 *
 * \note this feature is disabled by default
 * \note you should enable this if you are implementing functionality
 *       to download non versioned content
 */
void QSimpleUpdater::setPromptedDownload (const QString& url,
                                          const bool& enabled) {
    getUpdater (url)->setPromptedDownload (enabled);
}

/**
* \note The module revision is used to compare it with the remove revision.
*/
void QSimpleUpdater::setModuleRevision (const QString& url,
                                      const QString& revision)
{
   getUpdater (url)->setModuleRevision (revision);
}

/**
* \note Sets the flag to display HTTP redirects.
*/
void QSimpleUpdater::setShowRedirects (const QString& url,
                                       const bool& enabled) {
  getUpdater (url)->setShowRedirects (enabled);
}

/**
 * If set to true, the user will be able to directly
 * directly download content without running the version compare logic
 * This is used for direct content download like library files
 *
 * \note this feature is disabled by default
 * \note you should enable this if you are implementing functionality
 *       to download non versioned content
 */
void QSimpleUpdater::setDirectDownload (const QString& url,
                                        const bool& enabled) {
    getUpdater (url)->setDirectDownload (enabled);
}

/**
 * Sets the path of the downloaded content
 */
void QSimpleUpdater::setDownloadDir (const QString& url,
                                           const QString &path) {
  getUpdater (url)->setDownloadDir(path);
}

/**
 * Returns the latest version online
 */
QString QSimpleUpdater::getAvailableVersions (const QString& url) const {
    return getUpdater (url)->getAvailableVersions();
}

/**
 * Compares the two version strings (\a x and \a y).
 *     - If \a x is greater than \y, this function returns \c true.
 *     - If \a y is greater than \x, this function returns \c false.
 *     - If both versions are the same, this function returns \c false.
 */
bool QSimpleUpdater::compareVersionStr (const QString& url, const QString& x, const QString& y) {
    return getUpdater (url)->compareVersionStr(x,y);
}
// Mod End
