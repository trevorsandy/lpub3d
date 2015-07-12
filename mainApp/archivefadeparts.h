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

#ifndef ARCHIVEFADEPARTS_H
#define ARCHIVEFADEPARTS_H

#include <QObject>

#include "quazip.h"
#include "quazipfile.h"
#include "quazipdir.h"

#include "QsLog.h"

class ArchiveFadeParts : public QObject
{
    Q_OBJECT
public:
    explicit ArchiveFadeParts(QObject *parent = 0);

    static bool Archive(
      const QString &zipArchive,
      const QDir &dir,
      const QString &comment);

    static void RecurseAddDir(
      const QDir &dir,
      QStringList &list);

    static bool RecurseZipArchive(
            QStringList &zipDirFileList,
            QString &zipDirPath,
      const QString &zipArchive,
      const QDir &dir);

signals:

public slots:
};

#endif // ARCHIVEFADEPARTS_H
