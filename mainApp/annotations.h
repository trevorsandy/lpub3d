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

#ifndef ANNOTATIONS_H
#define ANNOTATIONS_H

#include <QHash>
#include <QString>
#include <QStringList>

class Annotations {
  private:
    static QString     		   empty;
    static QList<QString>	   titleAnnotations;
    static QHash<QString, QString> freeformAnnotations;
  public:
    Annotations();
    static const QString &freeformAnnotation(QString part);
    static const QList<QString> getTitleAnnotations()
    {
        return titleAnnotations;
    }
};

#endif
