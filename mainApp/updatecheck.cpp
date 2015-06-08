/****************************************************************************
**
** Copyright (C) 2015 Trevor SANDY. All rights reserved.
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

#include "updatecheck.h"
#include "version.h"
#include "lpub_preferences.h"

#include "QsLog.h"

void DoInitialUpdateCheck()
{
    int updateFrequency = Preferences::checkForUpdates;

    if (updateFrequency == 0)
        return;

    QSettings Settings;
    QDateTime checkTime = Settings.value("Updates/LastCheck", QDateTime()).toDateTime();

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

    new UpdateCheck(NULL, (void*)1);
}

UpdateCheck::UpdateCheck(QObject *parent, void *data) : QObject(parent)
{
    initialUpdate = (bool)data;

    // Initialize the updater
    updater = new QSimpleUpdater (this);

    // Check for updates
    checkForUpdates();
}

UpdateCheck::~UpdateCheck(){

    if(updater)
        updater->deleteLater();

}

void UpdateCheck::checkForUpdates(){

    //set initial update status
    updater->setInitialUpdate(initialUpdate);

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
    initialUpdate ? updater->setSilent (initialUpdate) :
                    updater->setSilent(Preferences::silentUpdate);

    // Show messages when checking is finished
    updater->setShowNewestVersionMessage (true);

    // Finally, check for updates...
    updater->checkForUpdates();
}
