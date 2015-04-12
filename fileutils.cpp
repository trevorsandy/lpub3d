#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QFileInfoList>

#include "fileutils.h"

/*
   Delete directory contents.

   \param dirName Path of directory to remove.
   \return true on success; false on error.
*/
bool FileUtils::removeFiles(const QString &dirName)
{
    bool result = true;
    QDir dir(dirName);
    qDebug() << "Directory Name: " << dirName;

    if (dir.exists(dirName)) {
        Q_FOREACH(QFileInfo info, dir.entryInfoList(QDir::NoDotAndDotDot | QDir::System | QDir::Hidden  | QDir::AllDirs | QDir::Files, QDir::DirsFirst)) {
            if (info.isDir()) {
                result = removeFiles(info.absoluteFilePath());
            }
            else {
                result = QFile::remove(info.absoluteFilePath());
            }

            if (!result) {
                return result;
            }
        }
    }

    return result;
}
