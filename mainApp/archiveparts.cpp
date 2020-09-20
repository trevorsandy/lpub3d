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

#include "archiveparts.h"
#include "lpub_preferences.h"
#include "lpub.h"
#include "paths.h"

#include "lc_application.h"


ArchiveParts::ArchiveParts(QObject *parent) : QObject(parent)
{
}

/*
 * Insert static coloured fade parts into unofficial ldraw library
 *
 */
bool ArchiveParts::Archive(const QString &zipArchive,
    const QDir &dir,
    QString &result,
    int &resultSeverity,
    const QString &comment,
    bool overwriteCustomPart) {

  //qDebug() << QString("\nProcessing %1 with comment: %2").arg(dir.absolutePath()).arg(comment);

  // Check if part directory exist
  if (!dir.exists()) {
      result = QString("Archive directory does not exist: %1").arg(dir.absolutePath());
      return false;
    }

  // We get the list of part directory files and folders recursively
  QStringList dirFileList;
  RecurseAddDir(dir, dirFileList);

  // Check if file list is empty
  if (dirFileList.isEmpty()) {
      result = QString("Directory is empty and will be ignored: %1").arg(dir.absolutePath());
      resultSeverity = 2; // Warning
      return false;
  }

  // Create an array of part file QFileInfo objects
  QFileInfoList filesToArchive;
  foreach (QString fileName, dirFileList) filesToArchive << QFileInfo(fileName);

  // Confirm archive exist or create new instance
  QFileInfo zipFileInfo(zipArchive);

  if (!zipFileInfo.exists()) {
      if (Preferences::usingDefaultLibrary) {
          if (!QFile::copy(QString("%1/%2").arg(Preferences::dataLocation).arg(zipFileInfo.fileName()),zipFileInfo.absoluteFilePath())) {
              emit gui->messageSig(LOG_ERROR, QString("Archive %1 not found and could not copy new instance.").arg(zipFileInfo.fileName()));
          } else {
              emit gui->messageSig(LOG_INFO, QString("Archive %1 not found. New instance will be created.").arg(zipFileInfo.fileName()));
          }
      } else {
          emit gui->messageSig(LOG_INFO, QString("Archive %1 not found. New instance created.").arg(zipFileInfo.fileName()));
      }
  }

  // Initialize the zip file
  QuaZip zip(zipArchive);
  zip.setFileNameCodec("IBM866");

  QFileInfoList zipFiles;

  if (zipFileInfo.exists()){
      if (!zip.open(QuaZip::mdAdd)) {
          result = QString("Could not open archive to add content: %1").arg(zip.getZipError());
          return false;
        }

      // Check if filesToArchive are not in excluded location
      QStringList validDirFiles;
      QStringList excludedPaths = QStringList() << "unofficial/parts" << "unofficial/p" << "parts" << "p";
      foreach(QString dirFile, dirFileList) {
          QString fileDir(QDir::toNativeSeparators(QFileInfo(dirFile).absolutePath()));
          bool isExcludedPath = false;
          foreach(QString excludedPath, excludedPaths) {
              QString excludedDir = QDir::toNativeSeparators(QString("%1/%2/").arg(Preferences::ldrawLibPath).arg(excludedPath));
              if ((isExcludedPath = (fileDir.indexOf(excludedDir,0,Qt::CaseInsensitive)) != -1)) {
                  break;
              }
          }
          if (isExcludedPath) {
              emit gui->messageSig(LOG_NOTICE, QString("Specified path [%1] is excluded from archive").arg(fileDir));
              continue;
          }
          validDirFiles << dirFile;
      }

      // Check if validated file list is not empty
      if (validDirFiles.isEmpty()) {
          result = QString("All specified paths were excluded from archive - nothing to do");
          return false;
      }

      QTime t; t.start();

      // Populate the list of existing zip files
      QStringList zipFileList;
      GetExistingArchiveFileList(zipFileList, validDirFiles, zipArchive);

      emit gui->messageSig(LOG_DEBUG, QString("Get Existing Archive File List %1").arg(gui->elapsedTime(t.elapsed())));

      //Create an array of archive file QFileInfo objects
      foreach (QString zipFile, zipFileList) zipFiles << QFileInfo(zipFile);

  } else {
      if (!zip.open(QuaZip::mdCreate)) {
          result = QString("Could not create archive: %1").arg(zip.getZipError());
          return false;
      }
  }

  // Initialize some variables
  QFile inFile;
  QuaZipFile outFile(&zip);
  int archivedPartCount   = 0;

  char c;
  foreach(QFileInfo fileInfo, filesToArchive) {

      //qDebug() << "Processing Disk File Name: " << fileInfo.absoluteFilePath();
      if (!fileInfo.isFile())
        continue;

      bool alreadyArchived = false;
      QString partStatus = "Archiving";

      foreach (QFileInfo zipFileInfo, zipFiles) {
          if (fileInfo == zipFileInfo) {
              bool okToOverwrite = (zipFileInfo.fileName().contains(QString("%1.dat").arg(FADE_SFX)) ||
                                    zipFileInfo.fileName().contains(QString("%1.dat").arg(HIGHLIGHT_SFX)));
              if (overwriteCustomPart && okToOverwrite) {
                  partStatus = "Overwriting archive";
                  //qDebug() << "FileMatch - Overwriting Fade File !! " << fileInfo.absoluteFilePath();
              } else {
                  alreadyArchived = true;
                  //qDebug() << "FileMatch - Skipping !! " << fileInfo.absoluteFilePath();
              }
              break;
          }
      }

      if (alreadyArchived)
        continue;
      else
         archivedPartCount++;

      // place archive file in appropriate archive subfolder
      QString fileNameWithRelativePath;

      int texDirIndex   = fileInfo.absoluteFilePath().indexOf("/parts/textures/",0,Qt::CaseInsensitive);
      int partsDirIndex = fileInfo.absoluteFilePath().indexOf("/parts/",0,Qt::CaseInsensitive);
      int primDirIndex  = fileInfo.absoluteFilePath().indexOf("/p/",0,Qt::CaseInsensitive);

      bool setTexDir    = texDirIndex   != -1;
      bool setPartsDir  = partsDirIndex != -1;
      bool setPrimDir   = primDirIndex  != -1;

      if (setTexDir){
        fileNameWithRelativePath = fileInfo.absoluteFilePath().remove(0, partsDirIndex + 1);
        qDebug() << "Adjusted Texture fileNameWithRelativePath: " << fileNameWithRelativePath;
      } else if (setPartsDir){
        fileNameWithRelativePath = fileInfo.absoluteFilePath().remove(0, partsDirIndex + 1);
        //qDebug() << "Adjusted Parts fileNameWithRelativePath: " << fileNameWithRelativePath;
      } else if (setPrimDir){
        fileNameWithRelativePath = fileInfo.absoluteFilePath().remove(0, primDirIndex + 1);
        //qDebug() << "Adjusted Primitive fileNameWithRelativePath: " << fileNameWithRelativePath;
      } else {
        fileNameWithRelativePath = fileInfo.fileName();
        //qDebug() << "Adjusted Root fileNameWithRelativePath: " << fileNameWithRelativePath;
      }

      QString fileNameWithCompletePath;

      if (setTexDir || setPartsDir || setPrimDir){
          fileNameWithCompletePath = fileNameWithRelativePath;
          //qDebug() << "fileNameWithCompletePath (ROOT TEXTURE/PART/PRIMITIVE) " << fileNameWithCompletePath;
        } else {
          QString subfolder = fileInfo.suffix().toLower() == "png" ? "parts/textures" : "parts";
          fileNameWithCompletePath = QString("%1/%2").arg(subfolder).arg(fileNameWithRelativePath);
          //qDebug() << "fileNameWithCompletePath (PART - DEFAULT)" << fileNameWithCompletePath;
        }

      emit gui->messageSig(LOG_INFO, QString("%1 part #%2 %3 to %4...")
                                             .arg(partStatus).arg(archivedPartCount)
                                             .arg(fileInfo.fileName()).arg(fileNameWithCompletePath));

      // insert file into archive
      inFile.setFileName(fileInfo.filePath());

      if (!inFile.open(QIODevice::ReadOnly)) {
          result = QString("inFile open error: %1").arg(inFile.errorString().toLocal8Bit().constData());
          return false;
      }

      if (!outFile.open(QIODevice::WriteOnly, QuaZipNewInfo(fileNameWithCompletePath, fileInfo.filePath()))) {
          result = QString("outFile open error: %1").arg(outFile.getZipError());
          return false;
        }

      while (inFile.getChar(&c) && outFile.putChar(c));

      if (outFile.getZipError() != UNZ_OK) {
          result = QString("outFile error: %1").arg(outFile.getZipError());
          return false;
        }

      outFile.close();

      if (outFile.getZipError() != UNZ_OK) {
          result = QString("outFile close error: %1").arg(outFile.getZipError());
          return false;
        }

      inFile.close();
    }

  if (!comment.isEmpty())
    zip.setComment(comment);

  zip.close();

  if (zip.getZipError() != 0) {
      result = QString("Archive error: %1").arg(zip.getZipError());
      return false;
    }

  result = QString::number(archivedPartCount);
  return true;
}

/* Confirm specified files exist in the archive */
bool ArchiveParts::GetExistingArchiveFileList(
            QStringList &zipDirFileList,
            QStringList &validDirFiles,
            const QString &zipArchive) {

  QuaZip zip(zipArchive);

  if (! zip.open(QuaZip::mdUnzip)) {
      emit gui->messageSig(LOG_ERROR, QString("Archive open error: %1 @ %2").arg(zip.getZipError()).arg(zipArchive));
      return false;
    }

  zip.setFileNameCodec("IBM866");

  enum EntryPoint
  {
     parts, p, s, t, p8, p48, partsb, sb, num_entryPoints
  };
  QString EntryPointPaths[num_entryPoints] =
  {
      "/parts", "/p", "/parts/s", "/parts/textures", "/p/8", "/p/48", "/parts/b", "/parts/s/b"
  };
  QStringList entryList[num_entryPoints];

  foreach (QString dirFile, validDirFiles) {

     QFileInfo zipFile(dirFile);
     QString fileName = zipFile.fileName();
     QString dirName  = zipFile.dir().dirName();
     bool texture     = zipFile.suffix().toLower() == "png";

     EntryPoint ep = parts;
     if (dirName == "parts") {
          ep = parts;
     } else
       if (dirName == "p") {
          ep = p;
     } else
       if (dirName == "s") {
          ep = s;
     } else
       if (dirName == "textures"){
          ep = t;
     } else
       if (dirName == "8") {
          ep = p8;
     } else
       if (dirName == "48") {
          ep = p48;
     } else
       if (dirName == "b") {
           zipFile.dir().cdUp();
           dirName = zipFile.dir().dirName();
           if (dirName == "parts")
               ep = partsb;
           else
           if (dirName == "s")
               ep = sb;
     } else {
          ep = texture ? t : parts;
     }

     QuaZipDir zipDir(&zip,EntryPointPaths[ep]);

     if (zipDir.exists()) {
         //emit gui->messageSig(LOG_DEBUG, QString("Archive path for [%1] is [%2]").arg(zipFile).arg(zipDir.path()));
         if (entryList[ep].isEmpty())
             entryList[ep] = zipDir.entryList(QDir::NoDotAndDotDot | QDir::Files, QDir::SortByMask);
         if (entryList[ep].contains(fileName))
             zipDirFileList << dirFile;
     } else {
         emit gui->messageSig(LOG_ERROR, QString("Archive entry point not found: %1").arg(EntryPointPaths[ep]));
     }
  }

  zip.close();

  if (zip.getZipError() != UNZ_OK) {
      emit gui->messageSig(LOG_ERROR, QString("Archive close error: %1").arg(zip.getZipError()));
      return false;
    }

  return true;
}

/* Recursively searches for all files on the disk \ a, and adds to the list of \ b */
void ArchiveParts::RecurseAddDir(const QDir &dir, QStringList &list) {

  QStringList filters = QStringList() << "*";
  QStringList entryList = dir.entryList(filters, QDir::NoDotAndDotDot | QDir::Dirs | QDir::Files);
  QStringList excludedPaths = QStringList() << "unofficial/parts" << "unofficial/p" << "parts" << "p";

  foreach (QString file, entryList) {
      QString filePath = QDir::toNativeSeparators(QString("%1/%2").arg(dir.absolutePath()).arg(file));
      bool isExcludedPath = false;
      foreach(QString excludedPath, excludedPaths) {
          QString excludedDir = QDir::toNativeSeparators(QString("%1/%2/").arg(Preferences::ldrawLibPath).arg(excludedPath));
          if ((isExcludedPath = (filePath.indexOf(excludedDir,0,Qt::CaseInsensitive)) != -1)) {
              break;
          }
      }
      if (isExcludedPath) {
          emit gui->messageSig(LOG_NOTICE, QString("Specified path [%1] is excluded from archive.").arg(filePath));
          return;
      }

      QFileInfo finfo(filePath);
      if (finfo.isSymLink()) {
        emit gui->messageSig(LOG_NOTICE, QString("Encountered a symbolic link: %1").arg(finfo.absoluteFilePath()));
        continue;
      }

      if (finfo.isDir()) {
          //emit gui->messageSig(LOG_INFO, "FILE INFO DIR PATH: " << finfo.fileName());
          QDir subDir(finfo.filePath());
          RecurseAddDir(subDir, list);
       } else if (finfo.suffix().toLower() == "dat" ||
                  finfo.suffix().toLower() == "ldr" ||
                  finfo.suffix().toLower() == "png") {
          list << finfo.filePath();
       }
    }
}
