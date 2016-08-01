/****************************************************************************
**
** Copyright (C) 2016 Trevor SANDY. All rights reserved.
**
** This file may be used under the terms of the
** GNU General Public Liceense (GPL) version 3.0
** which accompanies this distribution, and is
** available at http://www.gnu.org/licenses/gpl.html
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QSettings>
#include <QApplication>
#include "updatecheck.h"
#include "lpub_preferences.h"
#include "version.h"
#include "name.h"

UpdateCheck::UpdateCheck(QObject *parent, void *data) : QObject(parent)
{

    DEFS_URL        = "";
    m_latestVersion = "";
    m_changeLog     = "";
    m_option = (intptr_t)data;

    m_updater = QSimpleUpdater::getInstance();

    connect (m_updater, SIGNAL (checkingFinished (QString)),
             this,        SLOT (updateChangelog  (QString)));

    connect (m_updater, SIGNAL (checkingFinished (QString)),
             this,      SIGNAL (checkingFinished (QString)));

    connect (m_updater, SIGNAL (downloadFinished (QString, QString)),
             this,      SIGNAL (downloadFinished (QString, QString)));

    /* Run check for updates if sofware update */
    if (m_option == SoftwareUpdate) {
//        DEFS_URL = QString(VER_UPDATE_CHECK_JSON_URL).arg(Preferences::moduleVersion);
        DEFS_URL = VER_UPDATE_CHECK_JSON_URL;
        applyGeneralSettings (DEFS_URL);
        m_updater->checkForUpdates (DEFS_URL);      
    }

}

UpdateCheck::~UpdateCheck(){

    if(m_updater)
        m_updater->deleteLater();

}

void UpdateCheck::applyGeneralSettings(const QString &url){
    if(url == DEFS_URL){
        QString moduleVersion = Preferences::moduleVersion;
        bool enableDownloader = Preferences::enableDownloader;
        bool showAllNotifications = Preferences::showAllNotifications;
        bool showUpdateNotifications = Preferences::showUpdateNotifications;

        if (m_updater->getModuleVersion(DEFS_URL) != moduleVersion)
            m_updater->setModuleVersion(DEFS_URL, moduleVersion);
        m_updater->setEnableDownloader(DEFS_URL, enableDownloader);
        m_updater->setShowAllNotifications(DEFS_URL, showAllNotifications);
        m_updater->setShowUpdateNotifications (DEFS_URL, showUpdateNotifications);
    }
}

void UpdateCheck::requestDownload(const QString &url, const QString &localPath)
{
    if (url == DEFS_URL) {
        bool enabled  = true;
        switch (m_option){
        case LDrawOfficialLibraryDownload:
            DEFS_URL = VER_OFFICIAL_LIBRARY_JSON_URL;
            applyGeneralSettings(DEFS_URL);
            m_updater->setPromptedDownload(DEFS_URL,enabled);
            break;
        case LDrawUnofficialLibraryDownload:
            DEFS_URL = VER_UNOFFICIAL_LIBRARY_JSON_URL;
            applyGeneralSettings(DEFS_URL);
            m_updater->setPromptedDownload(DEFS_URL,enabled);
            break;
        case LDrawOfficialLibraryDirectDownload:
            DEFS_URL = VER_OFFICIAL_LIBRARY_JSON_URL;
            applyGeneralSettings(DEFS_URL);
            m_updater->setDirectDownload(DEFS_URL,enabled);
            break;
        case LDrawUnofficialLibraryDirectDownload:
            DEFS_URL = VER_UNOFFICIAL_LIBRARY_JSON_URL;
            applyGeneralSettings(DEFS_URL);
            m_updater->setDirectDownload(DEFS_URL,enabled);
            break;
        }
        m_updater->setIsNotSoftwareUpdate(DEFS_URL,enabled);
        m_updater->setLocalDownloadPath(DEFS_URL,localPath);

        m_updater->checkForUpdates (DEFS_URL);
    }
}

void UpdateCheck::updateChangelog (const QString &url) {
    if (url == DEFS_URL) {
        m_latestVersion = m_updater->getLatestVersion (url);
        m_changeLog = m_updater->getChangelog (url);
    }
}

void DoInitialUpdateCheck()
{
    int updateFrequency = Preferences::checkUpdateFrequency;;

    if (updateFrequency == 0)           //0=Never,1=Daily,2=Weekly,3=Monthly
        return;

    QSettings Settings;
    QDateTime checkTime = Settings.value(QString("%1/%2").arg(UPDATES,"LastCheck"), QDateTime()).toDateTime();

    if (!checkTime.isNull())
    {

        switch(updateFrequency)
        {
        case 1:
            checkTime = checkTime.addDays(1);
            break;
        case 2:
            checkTime = checkTime.addDays(7);
            break;
        case 3:
            checkTime = checkTime.addDays(30);
            break;
        }

        if (checkTime > QDateTime::currentDateTimeUtc())
            return;
    }

    new UpdateCheck(NULL, (void*)SoftwareUpdate);
}
