#ifndef FILEUTILS_H
#define FILEUTILS_H

#include <QString>

class FileUtils
{
public:
    static bool removeFiles(const QString &dirName);
};

#endif // FILEUTILS_H
