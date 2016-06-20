/*
 * (C) Copyright 2014 Alex Spataru
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the GNU Lesser General Public License
 * (LGPL) version 2.1 which accompanies this distribution, and is available at
 * http://www.gnu.org/licenses/lgpl-2.1.html
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 */

#ifndef DOWNLOAD_DIALOG_H
#define DOWNLOAD_DIALOG_H

#include <QDialog>
#include <ui_downloader.h>
#include <QNetworkReply>
#include <QNetworkAccessManager>

namespace Ui {
class Downloader;
}

class QNetworkReply;
class QNetworkAccessManager;

class Downloader : public QWidget {
    Q_OBJECT

  public:
    explicit Downloader (QWidget* parent = 0);
    ~Downloader();

    ///
    /// Returns \c true if the downloader will not attempt to install the
    /// downloaded file.
    ///
    /// This can be useful if you want to use the \c downloadFinished() signal
    /// to implement your own install procedures.
    ///
    bool useCustomInstallProcedures() const;

    ///
    /// Returns the download content name,
    ///
    QString downloadName() const;

    ///
    /// Returns the download local path
    ///
    QString localDownloadPath() const;

    ///
    /// Returns the type of content update or download
    ///
    bool isNotSoftwareUpdate() const;

    ///
    /// Returns the application name, which can be set manually or
    /// automatically using the \c qApp->applicationName() function.
    ///
    QString moduleName() const;

    ///
    /// Returns the application version, which can be set manually or
    /// automatically using the \c qApp->applicationVersion() function.
    ///
    QString moduleVersion() const;


  public slots:
    ///
    /// Begins downloading the update
    ///
    void startDownload (const QUrl& url);

    ///
    /// If \c custom is set to true, then the Downloader will not attempt to
    /// open or install the downloaded updates. This can be useful if you want
    /// to implement your own install procedures using the \c downloadFinished()
    /// signal.
    ///
    void setUseCustomInstallProcedures (const bool& custom);

    ///
    /// Changes the name of the download content.
    ///
    void setDownloadName (const QString& name);

    ///
    /// Sets the path for download content
    ///
    void setLocalDownloadPath (const QString& path);

    ///
    /// Sets the type of content update or download
    ///
    void setIsNotSoftwareUpdate (const bool& enabled);


  private slots:

    void openDownload();
    void installUpdate();
    void cancelDownload();
    void onDownloadFinished();
    void calculateSizes (qint64 received, qint64 total);
    void updateProgress (qint64 received, qint64 total);
    void calculateTimeRemaining (qint64 received, qint64 total);

  private:
    ///
    /// Rounds the \a input to the nearest integer
    ///
    float round (const float& input);

  signals:
    ///
    /// Emitted when the download has finished.
    /// You can use this to implement your own procedures to install the
    /// downloaded updates.
    ///
    void downloadFinished (const QString& url, const QString& filepath);

  private:
    uint m_startTime;
    QString m_filePath;
    QString m_moduleName;
    QString m_downloadName;
    QString m_moduleVersion;
    QString m_localDownloadPath;
    bool m_isNotSoftwareUpdate;
    bool m_useCustomProcedures;

    Ui::Downloader* m_ui;
    QNetworkReply* m_reply;
    QNetworkRequest m_downloadRequest;
    QNetworkAccessManager* m_manager;
};

#endif
