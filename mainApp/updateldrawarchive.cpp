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

#include "updateldrawarchive.h"
#include "version.h"
#include "lpub_preferences.h"

#include "QsLog.h"

#define LDRAW_UNOFFICIAL_ARCHIVE   "http://www.ldraw.org/library/unofficial/ldrawunf.zip"
#define LDRAW_OFFICIAL_ARCHIVE

UpdateLdrawArchive::UpdateLdrawArchive(QObject *parent) : QObject(parent)
{

  bool isLdrawDownload = true;

  // Initialize the updater
  updater = new QSimpleUpdater (this, isLdrawDownload);

  // Download and refresh LDraw Unofficial Archive
  updateUnoffArchive();

}

UpdateLdrawArchive::~UpdateLdrawArchive(){

    if(updater)
        updater->deleteLater();

}

void UpdateLdrawArchive::updateUnoffArchive(){

  //set initial update status

  // Tell the updater where to download the update, its recommended to use direct links
  updater->setDownloadUrl (LDRAW_UNOFFICIAL_ARCHIVE);

  // Tell the updater where to install the archive file
  updater->setLdrawArchivePath (QDir::toNativeSeparators(Preferences::ldrawPath + "/LPub3DViewer-Library"));

  // Finally, check for updates...
  updater->updateUnoffArchive();

}
