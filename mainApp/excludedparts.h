/****************************************************************************
**
** Copyright (C) 2015 - 2024 Trevor SANDY. All rights reserved.
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

#ifndef EXCLUDEDPARTS_H
#define EXCLUDEDPARTS_H

#include <QString>

class ExcludedParts
{
  public:
    ExcludedParts();
    static void loadExcludedParts(QByteArray &Buffer);
    static bool exportExcludedParts();
    static bool overwriteFile(const QString &file);
    static bool isExcludedPart(const QString &part);
    static bool isExcludedPart(const QString &part, bool &helperPart);
    static int isExcludedSupportPart(const QString &part);
    static bool lineHasExcludedPart(const QString &line);
    enum ExcludedPartType {
        EP_STANDARD,
        EP_HELPER,
        EP_LSYNTH,
        EP_HELPER_AND_LSYNTH
    };
  private:
    struct Part {
        QString id;
        int type;
        Part() : type(EP_STANDARD) {}
        Part(const QString _id, const int _type) : id(_id), type(_type) {}
    };
    static QList<Part> excludedParts;
};

#endif // EXCLUDEDPARTS_H
