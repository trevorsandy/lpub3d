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

#include "updateldrawarchive.h"
#include "version.h"
#include "lpub_preferences.h"
#include "name.h"

#include "QsLog.h"

UpdateLdrawArchive::UpdateLdrawArchive(QObject *parent, void *data) : QObject(parent)
{

  bool isLdrawUpdate = true;

  // Set the ldraw archive library path
  ldrawArchivePath = QDir::toNativeSeparators(QFileInfo(Preferences::lpub3dLibFile).absolutePath());

  // Initialize the updater - this bool is static @true
  updater = new SimpleUpdater (this, isLdrawUpdate);

  // Download and refresh LDraw Archive
  updateLdrawArchive((bool)data);

}

UpdateLdrawArchive::~UpdateLdrawArchive(){

    if(updater)
        updater->deleteLater();

}

void UpdateLdrawArchive::updateLdrawArchive(bool unofficial){

  // Tell the updater where to download the update, its recommended to use direct links
  QString url = unofficial ? URL_LDRAW_UNOFFICIAL_ARCHIVE : URL_LDRAW_OFFICIAL_ARCHIVE;
  updater->setDownloadUrl (url);

  // Tell the updated if the archive is unofficial - or offical
  updater->setIsUnofficialArchive(unofficial);

  // Tell the updater where to install the archive file
  updater->setLdrawArchivePath (ldrawArchivePath);

  // Finally, check for updates...
  updater->updateLdrawArchive();

}
