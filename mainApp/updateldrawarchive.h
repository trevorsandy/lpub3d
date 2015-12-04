/****************************************************************************
**
** Copyright (C) 2015 Trevor SANDY. All rights reserved.
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
#include <QSimpleUpdater>

class UpdateLdrawArchive : public QObject
{
  Q_OBJECT
public:
  explicit UpdateLdrawArchive(QObject *parent = 0);
  ~UpdateLdrawArchive();

  void updateUnoffArchive();

signals:

public slots:

private:
  QSimpleUpdater  *updater;
};

#endif // UPDATELDRAWARCHIVE_H
