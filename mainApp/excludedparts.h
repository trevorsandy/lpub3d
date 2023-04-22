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

#ifndef EXCLUDEDPARTS_H
#define EXCLUDEDPARTS_H

#include <QString>

class ExcludedParts
{
  private:
    enum Type {
        EP_STANDARD,
        EP_HELPER
    };
    struct Part {
        QString id;
        Type type;
        Part() : type(EP_STANDARD) {}
        Part(const QString _id, const Type _type) :
            id(_id), type(_type) {}
    };
    static QList<Part> excludedParts;
  public:
    ExcludedParts();
    static void loadExcludedParts(QByteArray &Buffer);
    static bool exportExcludedParts();
    static bool overwriteFile(const QString &file);
    static bool isExcludedPart(const QString &part);
    static bool isExcludedPart(const QString &part, bool &helperPart);
    static bool isExcludedHelperPart(const QString &part);
    static bool lineHasExcludedPart(const QString &line);
};

#endif // EXCLUDEDPARTS_H
