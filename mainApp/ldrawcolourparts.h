/****************************************************************************
**
** Copyright (C) 2015 - 2018 Trevor SANDY. All rights reserved.
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

#ifndef LDRAWCOLOURPARTS_H
#define LDRAWCOLOURPARTS_H

#include <QHash>
#include <QString>
#include <QStringList>

class LDrawColourParts
{
  private:
    static QHash<QString, QString>   ldrawColourParts;
  public:
    LDrawColourParts();
    bool isLDrawColourPart(QString part);
    static QString getLDrawColourPartInfo(QString part);
};

#endif // LDRAWCOLOURPARTS_H
