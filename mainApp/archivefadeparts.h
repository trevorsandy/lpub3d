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
