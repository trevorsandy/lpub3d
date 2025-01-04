/****************************************************************************
**
** Copyright (C) 2015 - 2025 Trevor SANDY. All rights reserved.
** Copyright (C) 2015 - 2025 Trevor SANDY. All rights reserved.
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
#include "qsimpleupdater.h"

enum updateType{
    SoftwareUpdate = 0,                 //0
    LDViewRendererDownload,
    LDGLiteRendererDownload,
    POVRayRendererDownload,
    LDrawOfficialLibraryDownload,       //1    UI menu initiated download
    LDrawUnofficialLibraryDownload,     //2    UI menu initiated download
    LDrawOfficialLibraryDirectDownload, //3    System load prompt initiated download
    LDrawUnofficialLibraryDirectDownload//4    System load prompt initiated download
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
    bool getCancelled(){return m_cancelled;}

signals:
   void checkingFinished (const QString& url);
   void downloadFinished (const QString& url, const QString& filepath);
   void rendererDownloadFinished (const QString& url);
   void cancel();

public slots:
   void updateChangelog (const QString& url);
   void setCancelled() {
     m_cancelled = true;
     emit cancel();
   }
   void getDownloadReturn(QString &path) {
     path = m_downloadReturnPath;
   }

private slots:
   void downloadReturn(QString url, QString path);

private:
   void applyGeneralSettings (const QString& url);
    bool                     m_cancelled;
    int                      m_option;

    QString                  DEFS_URL;
    QString                  m_changeLog;
    QString                  m_latestVersion;
    QString                  m_downloadReturnUrl;
    QString                  m_downloadReturnPath;
    QSimpleUpdater          *m_updater;
};

class AvailableVersions : public QObject
{
            Q_OBJECT
public:
    AvailableVersions(QObject *parent);
    ~AvailableVersions();
public slots:
    void setAvailableVersions(const QString& url);
private:
    QString                  DEFS_URL;
    QSimpleUpdater          *m_updater;
};
extern AvailableVersions* availableVersions;

#endif // UPDATECHECK_H
