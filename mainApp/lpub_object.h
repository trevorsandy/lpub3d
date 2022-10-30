/****************************************************************************
**
** Copyright (C) 2018 - 2022 Trevor SANDY. All rights reserved.
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

#ifndef LPUB_OBJECT_H
#define LPUB_OBJECT_H

#include <QObject>

#include "name.h"

class QString;

class LPubAlert : public QObject
{
    Q_OBJECT

public:
  LPubAlert();
  ~LPubAlert();
  static QString elapsedTime(const qint64 &duration);
signals:
  void messageSig(LogType logType, QString message);
private:
};

extern class LPubAlert *lpubAlert;

#endif // LPUB_OBJECT_H
