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

#ifndef PLISUBSTITUTEPARTS_H
#define PLISUBSTITUTEPARTS_H

#include <QMap>
#include <QString>
#include <QStringList>
class LPubMessages;

class PliSubstituteParts
{
  private:
    static bool     				result;
    static QString     				empty;
    static QMap<QString, QString>               substituteParts;
  public:
    PliSubstituteParts();
    static bool exportSubstitutePartsHeader();
    static bool overwriteFile(const QString &file);
    static const bool &hasSubstitutePart(QString part);
    static const bool &getSubstitutePart(QString &part);
};

#endif // PLISUBSTITUTEPARTS_H
