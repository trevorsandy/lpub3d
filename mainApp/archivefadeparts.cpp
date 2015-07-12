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

#include "archivefadeparts.h"

ArchiveFadeParts::ArchiveFadeParts(QObject *parent) : QObject(parent)
{

}

/*
 * Insert static coloured fade parts into unofficial ldraw library
 *
 */
bool ArchiveFadeParts::Archive(const QString &zipArchive, const QDir &dir, const QString &comment = QString("")) {

    // Initialize the zip file
    QuaZip zip(zipArchive);
    zip.setFileNameCodec("IBM866");

    // Check if directory exists
    if (!dir.exists()) {
        logWarn() << QString("! dir.exists(%1)=FALSE").arg(dir.absolutePath());
        return false;
    }

    // Check if the zip file exist; if yes, set to add content, and if no create
    QFileInfo zipFileInfo(zipArchive);
    QFileInfoList zipFiles;
    if (zipFileInfo.exists()){
        if (!zip.open(QuaZip::mdAdd)) {
            logWarn() <<  QString("! zip.open(): %1").arg(zip.getZipError());
            return false;
        }

        // We get the list of files already in the archive.
        QString dirPath = dir.absolutePath();
        QString zipDirPath = dirPath.section("/",-2,-1);
        QStringList zipFileList;
        RecurseZipArchive(zipFileList, zipDirPath, zipArchive, dir);

        //Create an array of archive file objects consisting of QFileInfo
        foreach (QString zipFile, zipFileList) zipFiles << QFileInfo(zipFile);

    } else {
        if (!zip.open(QuaZip::mdCreate)) {
            logWarn() <<  QString("! zip.open(): %1").arg(zip.getZipError());
            return false;
        }
    }

    // We get the list of directory files and folders recursively
    QStringList dirFileList;
    RecurseAddDir(dir, dirFileList);

    // Create an array of objects consisting of QFileInfo
    QFileInfoList files;
    foreach (QString fileName, dirFileList) files << QFileInfo(fileName);

    QFile inFile;
    QuaZipFile outFile(&zip);

    char c;
    foreach(QFileInfo fileInfo, files) {
        //qDebug() << "Disk File Name: " << fileInfo.absoluteFilePath();
        if (!fileInfo.isFile())
            continue;

        bool alreadyArchived = false;
        foreach (QFileInfo zipFileInfo, zipFiles) {

            if (fileInfo == zipFileInfo) {
                alreadyArchived = true;
                //qDebug() << "FileMatch - Skipping !! " << fileInfo.absoluteFilePath();
            }
        }

        if (alreadyArchived)
            continue;

      /* If the file is in a subdirectory, then add the name of the subdirectory to filenames
         For example: fileInfo.filePath() = "D:\Work\Sources\SAGO\svn\sago\Release\tmp_DOCSWIN\Folder\123.opn"
         then after removing the absolute path portion of the line will produce the fileNameWithSubFolders "Folder\123.opn", etc.
         For example: QString fileNameWithRelativePath = fileInfo.filePath().remove(0, dir.absolutePath().length() + 1);
         But for this application, we want to capture the root (fade) and the root's parent (parts) directory to archive
         in the correct directory, so we append the string "parts/fade" to the relative file name path. */
        QString fileNameWithRelativePath = fileInfo.filePath().remove(0, dir.absolutePath().length() + 1);
        QString fileNameWithCompletePath = QString("%1/%2").arg("parts/fade").arg(fileNameWithRelativePath);

        logTrace() << "Archived Disk File: " << fileInfo.absoluteFilePath();

        inFile.setFileName(fileInfo.filePath());

        if (!inFile.open(QIODevice::ReadOnly)) {
            logWarn() <<  QString("! inFile.open(): %1").arg(inFile.errorString().toLocal8Bit().constData());
            return false;
        }

        if (!outFile.open(QIODevice::WriteOnly, QuaZipNewInfo(fileNameWithCompletePath, fileInfo.filePath()))) {
            logWarn() << QString("! outFile.open(): %1").arg(outFile.getZipError());
            return false;
        }

        while (inFile.getChar(&c) && outFile.putChar(c));

        if (outFile.getZipError() != UNZ_OK) {
            logError() << QString("outFile.putChar() zipError(): %1").arg(outFile.getZipError());
            return false;
        }

        outFile.close();

        if (outFile.getZipError() != UNZ_OK) {
            logError() << QString("outFile.close() zipError(): %1").arg(outFile.getZipError());
            return false;
        }

        inFile.close();
    }

    // + comment
    if (!comment.isEmpty())
        zip.setComment(comment);

    zip.close();

    if (zip.getZipError() != 0) {
        logError() << QString("zip.close() zipError(): %1").arg(zip.getZipError());
        return false;
    }

    return true;
}

/* Recursively searches files in the archive for a given directory \ a, and adds to the list of \ b */
bool ArchiveFadeParts::RecurseZipArchive(QStringList &zipDirFileList, QString &zipDirPath, const QString &zipArchive, const QDir &dir) {

    QuaZip zip(zipArchive);
    QuaZip *ptrZip = &zip;

    if (!zip.open(QuaZip::mdUnzip)) {
        logWarn() << QString("! zip.open(): %1 @ %2").arg(zip.getZipError()).arg(zipArchive);
        return false;
    }

    zip.setFileNameCodec("IBM866");

    logInfo() << QString("%1 entries").arg(zip.getEntriesCount());
    logInfo() << QString("Global comment: %1").arg(zip.getComment().toLocal8Bit().constData());

    QuaZipDir zipDir(ptrZip,zipDirPath);

    if (zipDir.exists()) {

        zipDir.cd(zipDirPath);

        logInfo() << QString("%1 Fade zipDir entries at %2").arg(zipDir.count()).arg(zipDirPath);

        QStringList qsl = zipDir.entryList(QDir::NoDotAndDotDot | QDir::Dirs | QDir::Files, QDir::SortByMask);

        foreach (QString zipFile, qsl) {

                QFileInfo zipFileInfo(QString("%1/%2").arg(dir.absolutePath()).arg(zipFile));

                if (zipFileInfo.isSymLink())
                    return false;

                if(zipFileInfo.isDir()){

                    QString subDirPath = QString("%1/%2").arg(zipDirPath).arg(zipFile);
                    QDir subDir(zipFileInfo.filePath());
                    RecurseZipArchive(zipDirFileList, subDirPath, zipArchive, subDir);

                } else
                    zipDirFileList << zipFileInfo.filePath();
            }
        }

    zip.close();

    if (zip.getZipError() != UNZ_OK) {
        logError() << QString("zip.close() zipError(): %1").arg(zip.getZipError());
        return false;
    }

    return true;
}

/* Recursively searches for all files on the disk \ a, and adds to the list of \ b */
void ArchiveFadeParts::RecurseAddDir(const QDir &dir, QStringList &list) {

    QStringList qsl = dir.entryList(QDir::NoDotAndDotDot | QDir::Dirs | QDir::Files);

    foreach (QString file, qsl) {

        QFileInfo finfo(QString("%1/%2").arg(dir.absolutePath()).arg(file));

        if (finfo.isSymLink())
            return;

        if (finfo.isDir()) {

            QDir subDir(finfo.filePath());
            RecurseAddDir(subDir, list);

        } else
            list << QDir::toNativeSeparators(finfo.filePath());

    }
}
