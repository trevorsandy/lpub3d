/****************************************************************************
**
** Copyright (C) 2015 - 2022 Trevor SANDY. All rights reserved.
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

#include "QsLog.h"

#define LDRAW_UNOFFICIAL_ARCHIVE   "http://www.ldraw.org/library/unofficial/ldrawunf.zip"
#define LDRAW_OFFICIAL_ARCHIVE     "http://www.ldraw.org/library/updates/complete.zip"

UpdateLdrawArchive::UpdateLdrawArchive(QObject *parent) : QObject(parent)
{

  bool isLdrawDownload = true;

  // Set the ldraw archive librayr path
  ldrawArchivePath = QDir::toNativeSeparators(QFileInfo(Preferences::viewerLibFile).absolutePath());

  // Initialize the updater
  updater = new QSimpleUpdater (this, isLdrawDownload);

  // Download and refresh LDraw Unofficial Archive
  updateLdrawArchive(true);

}

UpdateLdrawArchive::~UpdateLdrawArchive(){

    if(updater)
        updater->deleteLater();

}

void UpdateLdrawArchive::updateLdrawArchive(bool unoff){

  // Tell the updater where to download the update, its recommended to use direct links
  QString url = unoff ? LDRAW_UNOFFICIAL_ARCHIVE : LDRAW_OFFICIAL_ARCHIVE;
  updater->setDownloadUrl (url);

  // Tell the updater where to install the archive file
  updater->setLdrawArchivePath (ldrawArchivePath);

  // Finally, check for updates...
  updater->updateLdrawArchive();

}
