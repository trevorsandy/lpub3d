/****************************************************************************
**
** Copyright (C) 2018 Trevor SANDY. All rights reserved.
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

#ifndef LPUBMESSAGES_H
#define LPUBMESSAGES_H

#include <QMessageBox>
#include <QStatusBar>
#include <stdio.h>
#include <QsLog.h>

#include "name.h"

class QObject;
class QString;

class LPubMessages : public QObject
{
    Q_OBJECT

public:
  LPubMessages();
  ~LPubMessages();
public slots:
  void statusMessage(LogType logType, QString message);

signals:
  void messageSig(LogType logType, QString message);
};

extern LPubMessages *alert;

#endif // LPUBMESSAGES_H
