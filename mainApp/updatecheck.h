/****************************************************************************
**
** Copyright (C) 2016 Trevor SANDY. All rights reserved.
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

#ifndef UPDATECHECK_H
#define UPDATECHECK_H

#include <QObject>
#include <QNetworkReply>
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
#include "SimpleUpdater.h"
#else
#include <SimpleUpdater>
#endif

void DoInitialUpdateCheck();

class UpdateCheck : public QObject
{
    Q_OBJECT
public:
    explicit UpdateCheck(QObject *parent, void *data);
    ~UpdateCheck();

    void checkForUpdates();

signals:

public slots:

private:
    bool                    initialUpdate;
    SimpleUpdater          *updater;
};

#endif // UPDATECHECK_H
