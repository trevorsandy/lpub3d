/****************************************************************************
**
** Copyright (C) 2015 - 2020 Trevor SANDY. All rights reserved.
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

#ifndef ARCHIVEPARTS_H
#define ARCHIVEPARTS_H

#include <QObject>
#include <QsLog.h>
#include "quazip.h"
#include "quazipfile.h"
#include "quazipdir.h"

class ArchiveParts : public QObject
{
    Q_OBJECT

public:
  explicit ArchiveParts(QObject *parent = 0);
  ~ArchiveParts()
  {}
  static bool Archive(const QString &zipArchive,
      const QDir &dir,
            QString &result,
            int &resultSeverity, /* 1=error,2=warning */
      const QString &comment = QString(),
            bool overwriteCustomPart = false);

    static void RecurseAddDir(
      const QDir &dir,
      QStringList &list);

    static bool GetExistingArchiveFileList(
            QStringList &zipDirFileList,
            QStringList &validDirFiles,
            const QString &zipArchive);

public slots:

signals:
};

#endif // ARCHIVEPARTS_H
