/****************************************************************************
**
** Copyright (C) 2020 - 2021 Trevor SANDY. All rights reserved.
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

#ifndef STICKERPARTS_H
#define STICKERPARTS_H

#include <QString>
#include <QStringList>

class StickerParts
{
  private:
    static bool                     result;
    static QString                  empty;
    static QList<QString>           stickerParts;
  public:
    StickerParts();
    static void loadStickerParts(QByteArray &Buffer);
    static bool exportStickerParts();
    static bool overwriteFile(const QString &file);
    static const bool &hasStickerPart(QString part);
    static const bool &lineHasStickerPart(const QString &line);
};

#endif // STICKERPARTS_H
