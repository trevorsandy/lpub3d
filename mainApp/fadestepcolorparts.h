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

#ifndef FADESTEPCOLORPARTS_H
#define FADESTEPCOLORPARTS_H

#include <QString>
#include <QStringList>

class FadeStepColorParts
{
  private:
    static bool     				result;
    static QString     				empty;
    static QString                  path;
    static QMap<QString, QString>   fadeStepStaticColorParts;
  public:
    FadeStepColorParts();
    static const bool &isStaticColorPart(QString part);
    static const bool &getStaticColorPartInfo(QString &part);
    static const QString &staticColorPartPath(QString part);
};

#endif // FADESTEPCOLORPARTS_H
