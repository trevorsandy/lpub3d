/****************************************************************************
**
** Copyright (C) 2015 - 2024 Trevor SANDY. All rights reserved.
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

#ifndef UPDATELDRAWARCHIVE_H
#define UPDATELDRAWARCHIVE_H

#include <QObject>
#include <QNetworkReply>
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
#include "qsimpleupdater.h"
#else
#include <QSimpleUpdater>
#endif

class UpdateLdrawArchive : public QObject
{
  Q_OBJECT

public:
  explicit UpdateLdrawArchive(QObject *parent = 0);
  ~UpdateLdrawArchive();

  void updateLdrawArchive(bool unoff = true);

signals:

public slots:

private:
  QSimpleUpdater  *updater;
  QString ldrawArchivePath;
};

#endif // UPDATELDRAWARCHIVE_H
