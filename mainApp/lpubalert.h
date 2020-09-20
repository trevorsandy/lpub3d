/****************************************************************************
**
** Copyright (C) 2018 - 2020 Trevor SANDY. All rights reserved.
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

#ifndef LPUBALERT_H
#define LPUBALERT_H

#include <QObject>

#include "name.h"

class QString;

class LPubAlert : public QObject
{
    Q_OBJECT

public:
  LPubAlert();
  ~LPubAlert();
signals:
  void messageSig(LogType logType, QString message);
private:
};

extern class LPubAlert *lpubAlert;

#endif // LPUBALERT_H
