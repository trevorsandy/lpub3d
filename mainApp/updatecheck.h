/****************************************************************************
**
** Copyright (C) 2015 - 2017 Trevor SANDY. All rights reserved.
** Copyright (C) 2015 - 2017 Trevor SANDY. All rights reserved.
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
#include "qsimpleupdater.h"

enum updateType{
    SoftwareUpdate = 0,                 //0
    LDrawOfficialLibraryDownload,       //1
    LDrawUnofficialLibraryDownload,     //2
    LDrawOfficialLibraryDirectDownload, //3
    LDrawUnofficialLibraryDirectDownload//4
};

void DoInitialUpdateCheck();

class UpdateCheck : public QObject
{
    Q_OBJECT
public:
    explicit UpdateCheck(QObject *parent, void *data);
    UpdateCheck(){}
    ~UpdateCheck();

    void requestDownload(const QString& url, const QString& localPath);
    QString getDEFS_URL(){return DEFS_URL;}
    bool getCancel(){return m_cancel;}

signals:
   void checkingFinished (const QString& url);
   void downloadFinished (const QString& url, const QString& filepath);
   void cancel();

public slots:
   void updateChangelog (const QString& url);
   void setCancel(){
     m_cancel = true;
     emit cancel();
   }

private:
   void applyGeneralSettings (const QString& url);

    bool                     m_cancel;
    int                      m_option;

    QString                  DEFS_URL;
    QString                  m_changeLog;
    QString                  m_latestVersion;
    QSimpleUpdater          *m_updater;
};

class AvailableVersions : public QObject
{
            Q_OBJECT
public:
    AvailableVersions();
    ~AvailableVersions();
private:
    QString                  DEFS_URL;
    QSimpleUpdater          *m_updater;
};
#endif // UPDATECHECK_H
