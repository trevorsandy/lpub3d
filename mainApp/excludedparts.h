/****************************************************************************
**
** Copyright (C) 2016 Trevor SANDY. All rights reserved.
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
#include <QStringList>

class ExcludedParts
{
  private:
    static bool     				result;
    static QString     				empty;
    static QList<QString>               excludedParts;
  public:
    ExcludedParts();
    static const bool &hasExcludedPart(QString part);
};

#endif // EXCLUDEDPARTS_H
