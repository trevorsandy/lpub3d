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

#include "archiveparts.h"
#include "lpub_preferences.h"

ArchiveParts::ArchiveParts(QObject *parent) : QObject(parent)
{

}

/*
 * Insert static coloured fade parts into unofficial ldraw library
 *
 */
bool ArchiveParts::Archive(const QString &zipArchive, const QDir &dir, const QString &comment = QString(""), ARCHIVE_TYPE option = NORMAL_PART) {

    QString fileNameWithRelativePath;
    QString firstFilePathPiece;

    bool setPrimDir = false;
    bool setPartsDir = false;
    bool setRootDir = false;

    // If input directory is 'p' use 'p' (primitive) else use 'parts'
    if (dir.dirName().toLower() == "p"){
        setPrimDir = true;
      } else if (dir.dirName().toLower() == "parts"){
        setPartsDir = true;
      }

    logWarn() << QString("\nProcessing %1 with comment: %2").arg(dir.absolutePath()).arg(comment);

    // Initialize the zip file
    QuaZip zip(zipArchive);
    zip.setFileNameCodec("IBM866");

    // Check if directory exists
    if (!dir.exists()) {
        logWarn() << QString("! dir.exists(%1)=FALSE").arg(dir.absolutePath());
        return false;
    }

    // We get the list of directory files and folders recursively
    QStringList dirFileList;
    RecurseAddDir(dir, dirFileList);

    // Check if file list is empty
    if (dirFileList.isEmpty()) {
        logWarn() << QString("! dirFileList.isEmpty(): %1").arg(dir.absolutePath());
        return true;
    }

    // Create an array of objects consisting of QFileInfo
    QFileInfoList files;
    foreach (QString fileName, dirFileList) files << QFileInfo(fileName);


    // Check if the zip file exist; if yes, set to add content, and if no create
    QFileInfo zipFileInfo(zipArchive);
    QFileInfoList zipFiles;
    if (zipFileInfo.exists()){
        if (!zip.open(QuaZip::mdAdd)) {
            logWarn() <<  QString("! zip.open()::mdAdd: %1").arg(zip.getZipError());
            return false;
        }

        // We get the list of files already in the archive.
        QStringList zipDirPaths;
        QStringList zipFileList;

        // Test for 'parts' or 'p' sub directory
        int filesTested = 0;
        foreach (QFileInfo fileInfo, files){

            if (!fileInfo.isFile())
              continue;

            filesTested ++;

            fileNameWithRelativePath = fileInfo.filePath().remove(0, dir.absolutePath().length() + 1);
            firstFilePathPiece = fileNameWithRelativePath.section('/',0,0);

            if (firstFilePathPiece.toLower() == "parts" ||
                firstFilePathPiece.toLower() == "p")
              {
                setRootDir = true;
              }

            if (filesTested > 1)
              break;
          }

        switch (option)
          {
          case FADE_COLOUR_PART: {
              if (setPrimDir)
                zipDirPaths << "p";
              else if (setPartsDir)
                zipDirPaths << "parts";
              else
                zipDirPaths << "parts";
              break;
            }
          case NORMAL_PART: {
              if (setPrimDir)
                zipDirPaths << "p";
              else if (setPartsDir)
                zipDirPaths << "parts";
              else if (setRootDir)
                zipDirPaths << "/";
              else
                zipDirPaths << "parts";
              break;
            }
          default: {
              zipDirPaths  << "/" << "parts" << "p";
              break;
            }
          }

        foreach (QString zipDirPath, zipDirPaths){
            RecurseZipArchive(zipFileList, zipDirPath, zipArchive, dir);
          }

        //Create an array of archive file objects consisting of QFileInfo
        foreach (QString zipFile, zipFileList) zipFiles << QFileInfo(zipFile);

    } else {
        if (!zip.open(QuaZip::mdCreate)) {
            logWarn() <<  QString("! zip.open()::mdCreate: %1").arg(zip.getZipError());
            return false;
        }
    }

    // Archive each disk file as necessary
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
                qDebug() << "FileMatch - Skipping !! " << fileInfo.absoluteFilePath();
                break;
            }
        }

        if (alreadyArchived)
            continue;

        logNotice() << "Archived Disk File: " << fileInfo.absoluteFilePath();

      /* If the file is in a subdirectory, then add the name of the subdirectory to filenames
         For example: fileInfo.filePath() = "D:\Work\Sources\SAGO\svn\sago\Release\tmp_DOCSWIN\Folder\123.opn"
         then after removing the absolute path portion of the line will produce the fileNameWithSubFolders "Folder\123.opn", etc.
         For example: QString fileNameWithRelativePath = fileInfo.filePath().remove(0, dir.absolutePath().length() + 1);
         But for part fade, we want to capture the root (fade) and the root's parent (parts) directory to archive
         in the correct directory, so we append the string "parts/fade" to the relative file name path.
         For normal archive, we want to place all unofficial files not in directory P(rimative) in the parts directory */

        fileNameWithRelativePath = fileInfo.filePath().remove(0, dir.absolutePath().length() + 1);
        firstFilePathPiece = fileNameWithRelativePath.section('/',0,0);

        // first part of file path is 'parts' or 'p' use root dir
        if (firstFilePathPiece.toLower() == "parts" ||
            firstFilePathPiece.toLower() == "p")
          {
            setRootDir = true;
          }

        QString fileNameWithCompletePath;

        if (setPartsDir ){
            fileNameWithCompletePath = QString("%1/%2").arg("parts").arg(fileNameWithRelativePath);
            //logInfo() << "fileNameWithCompletePath (PART)" << fileNameWithCompletePath;
          } else if (setPrimDir) {
            fileNameWithCompletePath = QString("%1/%2").arg("p").arg(fileNameWithRelativePath);
            //logInfo() << "fileNameWithCompletePath (PRIMITIVE)" << fileNameWithCompletePath;
          } else if (setRootDir ){
            fileNameWithCompletePath = QString("%1").arg(fileNameWithRelativePath);
            //logInfo() << "fileNameWithCompletePath (ROOT PART/PRIMITIVE) " << fileNameWithCompletePath;
          } else {
            fileNameWithCompletePath = QString("%1/%2").arg("parts").arg(fileNameWithRelativePath);
            //logInfo() << "fileNameWithCompletePath (PART - DEFAULT)" << fileNameWithCompletePath;
          }

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
bool ArchiveParts::RecurseZipArchive(QStringList &zipDirFileList, QString &zipDirPath, const QString &zipArchive, const QDir &dir) {

    QuaZip zip(zipArchive);
    QuaZip *ptrZip = &zip;

    if (!zip.open(QuaZip::mdUnzip)) {
        logWarn() << QString("! zip.open(): %1 @ %2").arg(zip.getZipError()).arg(zipArchive);
        return false;
    }

    zip.setFileNameCodec("IBM866");

    logInfo() << QString("%1 Total Zip Archive Entries: ").arg(zip.getEntriesCount());
    logInfo() << QString("Global Comment: %1").arg(zip.getComment().toLocal8Bit().constData());
    logInfo() << QString("Disk Directory: %1").arg(dir.absolutePath());

    QuaZipDir zipDir(ptrZip,zipDirPath);

    if (zipDir.exists()) {

        zipDir.cd(zipDirPath);

        logInfo() << QString("%1 Zip Directory Entries in '%2'").arg(zipDir.count()).arg(zipDirPath);

        QStringList qsl = zipDir.entryList(QDir::NoDotAndDotDot | QDir::Dirs | QDir::Files, QDir::SortByMask);

        foreach (QString zipFile, qsl) {

                QFileInfo zipFileInfo(QString("%1/%2").arg(dir.absolutePath()).arg(zipFile));
//                logInfo() << QString("zipFileInfo (to compare): %1/%2").arg(dir.absolutePath()).arg(zipFile);
//                logInfo() << QString("zipFile (qsl entry):      %1").arg(zipFile);

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
void ArchiveParts::RecurseAddDir(const QDir &dir, QStringList &list) {

    QString offPartsDir = QDir::toNativeSeparators(QString("%1/%2").arg(Preferences::ldrawPath).arg("PARTS"));
    QString offPrimsDir = QDir::toNativeSeparators(QString("%1/%2").arg(Preferences::ldrawPath).arg("P"));
    QString unoffPartsDir = QDir::toNativeSeparators(QString("%1/%2").arg(Preferences::ldrawPath).arg("Unofficial/parts"));
    QString unoffPrimsDir = QDir::toNativeSeparators(QString("%1/%2").arg(Preferences::ldrawPath).arg("Unofficial/p"));

    QStringList qsl = dir.entryList(QDir::NoDotAndDotDot | QDir::Dirs | QDir::Files);

    foreach (QString file, qsl) {

        QFileInfo finfo(QString("%1/%2").arg(dir.absolutePath()).arg(file));

        if (
            finfo.absolutePath().toLower().contains(offPartsDir.toLower()) ||
            finfo.absolutePath().toLower().contains(offPrimsDir.toLower()) ||
            finfo.absolutePath().toLower().contains(unoffPartsDir.toLower()) ||
            finfo.absolutePath().toLower().contains(unoffPrimsDir.toLower())
            ) {
            //qDebug() << "\nLDRAW EXCLUDED DIR: " << finfo.absolutePath();
            return;
          }

        if (finfo.isSymLink())
            return;

        if (finfo.isDir()) {

            //logInfo() << "FILE INFO DIR PATH: " << finfo.fileName();

            QDir subDir(finfo.filePath());
            RecurseAddDir(subDir, list);

        } else if (finfo.suffix().toLower() == "dat") {

            //logInfo() << "FILE INFO SUFFIX: " << finfo.suffix().toLower();

            list << QDir::toNativeSeparators(finfo.filePath());

        }

    }
}
