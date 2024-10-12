/****************************************************************************
**
** Copyright (C) 2015 - 2023 Trevor SANDY. All rights reserved.
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

#include <quazip.h>
#include <quazipfile.h>
#include <quazipdir.h>
#include <QsLog.h>

#include "archiveparts.h"
#include "lpub_preferences.h"
#include "lpub.h"

ArchiveParts::ArchiveParts(QObject *parent) : QObject(parent)
{
}

/*
 * Insert static coloured fade parts into unofficial ldraw library
 *
 */
bool ArchiveParts::Archive(
    const QString &zipArchive,
    const QDir &dir,
    QString &result,
    int &resultSeverity,
    const QString &comment,
    bool overwriteCustomPart) {

  //qDebug() << QString("\nProcessing %1 with comment: %2").arg(dir.absolutePath()).arg(comment);

  // Check if part directory exist
  if (!dir.exists()) {
      result = tr("Archive directory does not exist: %1").arg(dir.absolutePath());
      return false;
  }

  // We get the list of part directory files and folders recursively
  QStringList dirFileList;
  RecurseAddDir(dir, dirFileList);

  // Check if file list is empty
  if (dirFileList.isEmpty()) {
      result = tr("Directory is empty and will be ignored: %1").arg(dir.absolutePath());
      resultSeverity = 2; // Warning
      return false;
  }

  // Create an array of part file QFileInfo objects
  QFileInfoList filesToArchive;
  Q_FOREACH (QString const &fileName, dirFileList) filesToArchive << QFileInfo(fileName);

  // Confirm archive exist or create new instance
  QFileInfo zipFileInfo(zipArchive);

  if (!zipFileInfo.exists()) {
      if (Preferences::usingDefaultLibrary) {
          if (!QFile::copy(QString("%1/%2").arg(Preferences::dataLocation).arg(zipFileInfo.fileName()),zipFileInfo.absoluteFilePath())) {
              emit gui->messageSig(LOG_ERROR, tr("Archive %1 not found and could not copy new instance.").arg(zipFileInfo.fileName()));
          } else {
              emit gui->messageSig(LOG_INFO, tr("Archive %1 not found. New instance will be created.").arg(zipFileInfo.fileName()));
          }
      } else {
          emit gui->messageSig(LOG_INFO, tr("Archive %1 not found. New instance created.").arg(zipFileInfo.fileName()));
      }
  }

  // Initialize the zip file
  QuaZip zip(zipArchive);
  zip.setFileNameCodec("IBM866");

  QFileInfoList zipFiles;

  if (zipFileInfo.exists()){
      if (!zip.open(QuaZip::mdAdd)) {
          result = tr("Could not open archive to add content. Return code %1.<br>"
                      "Archive file %2 may be open in another program.")
                       .arg(zip.getZipError()).arg(QFileInfo(zipArchive).fileName());
          return false;
        }

      // Check if filesToArchive are not in excluded location
      QStringList validDirFiles;
        QStringList excludedPaths = QStringList()
                                    << QLatin1String("unofficial/parts")
                                    << QLatin1String("unofficial/p")
                                    << QLatin1String("parts")
                                    << QLatin1String("p");
      Q_FOREACH (QString const &dirFile, dirFileList) {
          QString fileDir(QDir::toNativeSeparators(QFileInfo(dirFile).absolutePath()));
          bool isExcludedPath = false;
          Q_FOREACH (QString const &excludedPath, excludedPaths) {
              QString excludedDir = QDir::toNativeSeparators(QString("%1/%2/").arg(Preferences::ldrawLibPath).arg(excludedPath));
              if ((isExcludedPath = (fileDir.indexOf(excludedDir,0,Qt::CaseInsensitive)) != -1)) {
                  break;
              }
          }
          if (isExcludedPath) {
              emit gui->messageSig(LOG_NOTICE, tr("Specified path [%1] is excluded from archive").arg(fileDir));
              continue;
          }
          validDirFiles << dirFile;
      }

      // Check if validated file list is not empty
      if (validDirFiles.isEmpty()) {
          result = tr("All specified paths were excluded from archive - nothing to do");
          return false;
      }

      QElapsedTimer t; t.start();

      // Populate the list of existing zip files
      QStringList zipFileList;
      GetExistingArchiveFileList(zipFileList, validDirFiles, zipArchive);

      emit gui->messageSig(LOG_DEBUG, tr("Get Existing Archive File List %1").arg(gui->elapsedTime(t.elapsed())));

      //Create an array of archive file QFileInfo objects
      Q_FOREACH (QString const &zipFile, zipFileList) zipFiles << QFileInfo(zipFile);

  } else {
      if (!zip.open(QuaZip::mdCreate)) {
          result = tr("Could not create archive. Return code %1.").arg(zip.getZipError());
          return false;
      }
  }

  // Initialize some variables
  QFile inFile;
  QuaZipFile outFile(&zip);
  int archivedPartCount   = 0;

  char c;
  Q_FOREACH (QFileInfo const &fileInfo, filesToArchive) {

      //qDebug() << "Processing Disk File Name: " << fileInfo.absoluteFilePath();
      if (!fileInfo.isFile())
        continue;

      bool alreadyArchived = false;
      QString partStatus = "Archiving";

      Q_FOREACH (QFileInfo const &zipFileInfo, zipFiles) {
          if (fileInfo == zipFileInfo) {
              bool okToOverwrite = (zipFileInfo.fileName().endsWith(QString("%1.dat").arg(QLatin1String(FADE_SFX)),Qt::CaseInsensitive) ||
                                    zipFileInfo.fileName().endsWith(QString("%1.dat").arg(QLatin1String(HIGHLIGHT_SFX)),Qt::CaseInsensitive));
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
        //qDebug() << "Adjusted Texture fileNameWithRelativePath: " << fileNameWithRelativePath;
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
          QString const subfolder = fileInfo.suffix().toLower() == QLatin1String("png")
                                        ? QLatin1String("parts/textures")
                                        : QLatin1String("parts");
          fileNameWithCompletePath = QString("%1/%2").arg(subfolder).arg(fileNameWithRelativePath);
          //qDebug() << "fileNameWithCompletePath (PART - DEFAULT)" << fileNameWithCompletePath;
        }

      emit gui->messageSig(LOG_INFO, QString("%1 part #%2 %3 to %4...")
                                             .arg(partStatus).arg(archivedPartCount)
                                             .arg(fileInfo.fileName()).arg(fileNameWithCompletePath));

      // insert file into archive
      inFile.setFileName(fileInfo.filePath());

      if (!inFile.open(QIODevice::ReadOnly)) {
          result = tr("inFile open error: %1").arg(inFile.errorString().toLocal8Bit().constData());
          return false;
      }

      if (!outFile.open(QIODevice::WriteOnly, QuaZipNewInfo(fileNameWithCompletePath, fileInfo.filePath()))) {
          result = tr("outFile open error. Return code %1.").arg(outFile.getZipError());
          return false;
        }

      while (inFile.getChar(&c) && outFile.putChar(c));

      if (outFile.getZipError() != UNZ_OK) {
          result = tr("outFile error. Return code %1.").arg(outFile.getZipError());
          return false;
        }

      outFile.close();

      if (outFile.getZipError() != UNZ_OK) {
          result = tr("outFile close error. Return code %1.").arg(outFile.getZipError());
          return false;
        }

      inFile.close();
    }

  if (!comment.isEmpty())
    zip.setComment(comment);

  zip.close();

  if (zip.getZipError() != 0) {
      result = QString("Archive error. Return code %1.").arg(zip.getZipError());
      return false;
    }

  result = QString::number(archivedPartCount);
  return true;
}

/* Confirm specified files exist in the archive */
bool ArchiveParts::GetExistingArchiveFileList(
    QStringList &zipDirFileList,
    QStringList &validDirFiles,
    const QString &zipArchive)
{
  QuaZip zip(zipArchive);

  if (! zip.open(QuaZip::mdUnzip)) {
      QString const error = tr("Could not open archive to add content. Return code %1.<br>"
                               "Archive file %2 may be open in another program.")
                                .arg(zip.getZipError()).arg(QFileInfo(zipArchive).fileName());
      emit gui->messageSig(LOG_ERROR, error);
      return false;
  }

  zip.setFileNameCodec("IBM866");

  enum EntryPoint
  {
     parts, p, s, t, p8, p48, partsb, sb, num_entryPoints
  };
  QString EntryPointPaths[num_entryPoints] =
  {
    QLatin1String("/parts"),
    QLatin1String("/p"),
    QLatin1String("/parts/s"),
    QLatin1String("/parts/textures"),
    QLatin1String("/p/8"),
    QLatin1String("/p/48"),
    QLatin1String("/parts/b"),
    QLatin1String("/parts/s/b")
  };

  QStringList entryList[num_entryPoints];

  Q_FOREACH (QString const &dirFile, validDirFiles) {

    QFileInfo zipFile(dirFile);
    QString const fileName = zipFile.fileName();
    QString dirName  = zipFile.dir().dirName();
    bool texture     = zipFile.suffix().toLower() == "png";

    EntryPoint ep = parts;
    if (dirName == QLatin1String("parts"))
      ep = parts;
    else
    if (dirName == QLatin1String("p"))
      ep = p;
    else
    if (dirName == QLatin1String("s"))
        ep = s;
    else
    if (dirName == QLatin1String("textures"))
      ep = t;
    else
    if (dirName == QLatin1String("8"))
      ep = p8;
    else
    if (dirName == QLatin1String("48"))
      ep = p48;
    else
    if (dirName == QLatin1String("b")) {
      zipFile.dir().cdUp();
      dirName = zipFile.dir().dirName();
      if (dirName == QLatin1String("parts"))
        ep = partsb;
      else
      if (dirName == QLatin1String("s"))
        ep = sb;
    }
    else
      ep = texture ? t : parts;

    QuaZipDir zipDir(&zip,EntryPointPaths[ep]);

    if (zipDir.exists()) {
      //emit gui->messageSig(LOG_DEBUG, QString("Archive path for [%1] is [%2]").arg(zipFile).arg(zipDir.path()));
      if (entryList[ep].isEmpty())
        entryList[ep] = zipDir.entryList(QDir::NoDotAndDotDot | QDir::Files, QDir::SortByMask);
      if (entryList[ep].contains(fileName))
        zipDirFileList << dirFile;
    } else {
      emit gui->messageSig(LOG_WARNING, tr("Archive entry point not found: %1").arg(EntryPointPaths[ep]));
    }
  }

  zip.close();

  if (zip.getZipError() != UNZ_OK) {
    emit gui->messageSig(LOG_ERROR, tr("Archive close errorReturn code %1.").arg(zip.getZipError()));
    return false;
  }

  return true;
}

/* Recursively searches for all files on the disk \ a, and adds to the list of \ b */
void ArchiveParts::RecurseAddDir(const QDir &dir, QStringList &list) {

  QStringList filters = QStringList() << "*";
  QStringList entryList = dir.entryList(filters, QDir::NoDotAndDotDot | QDir::Dirs | QDir::Files);
  QStringList excludedPaths = QStringList()
                              << QLatin1String("unofficial/parts")
                              << QLatin1String("unofficial/p")
                              << QLatin1String("parts")
                              << QLatin1String("p");

  Q_FOREACH (QString const &file, entryList) {
    QString filePath = QDir::toNativeSeparators(QString("%1/%2").arg(dir.absolutePath()).arg(file));
    bool isExcludedPath = false;
    Q_FOREACH (QString const &excludedPath, excludedPaths) {
      QString excludedDir = QDir::toNativeSeparators(QString("%1/%2/").arg(Preferences::ldrawLibPath).arg(excludedPath));
      if ((isExcludedPath = (filePath.indexOf(excludedDir,0,Qt::CaseInsensitive)) != -1)) {
        break;
      }
    }
    if (isExcludedPath) {
      emit gui->messageSig(LOG_NOTICE, tr("Specified path [%1] is excluded from archive.").arg(filePath));
      return;
    }

    QFileInfo finfo(filePath);
    if (finfo.isSymLink()) {
      emit gui->messageSig(LOG_NOTICE, tr("Encountered a symbolic link: %1").arg(finfo.absoluteFilePath()));
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
