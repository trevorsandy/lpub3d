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

//==============================================================================
// Class includes
//==============================================================================

#include "downloader.h"

//==============================================================================
// System includes
//==============================================================================

#include <math.h>

#include <QDir>
#include <QFile>
#include <QProcess>
#include <QMessageBox>
#include <QNetworkReply>
#include <QDesktopServices>
#include <QNetworkAccessManager>

//==============================================================================
// Downloader::Downloader
//==============================================================================

Downloader::Downloader (QWidget* parent) : QWidget (parent) {
    m_ui = new Ui::Downloader;
    m_ui->setupUi (this);

    // Set the connection user agent
    m_downloadRequest.setRawHeader("User-Agent","Mozilla Firefox");

    /* Initialize private members */
    m_manager = new QNetworkAccessManager();

    /* Initialize internal values */
    m_filePath = "";
    m_startTime = 0;
    m_downloadName = "";
    m_localDownloadPath = "";
    m_isNotSoftwareUpdate = false;
    m_useCustomProcedures = false;
    m_moduleName = qApp->applicationName();
    m_moduleVersion = qApp->applicationVersion();

    /* Make the window look like a modal dialog */
    setWindowIcon (QIcon ());
    setWindowFlags (Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);

    /* Configure the appearance and behavior of the buttons */
    m_ui->openButton->setEnabled (false);
    m_ui->openButton->setVisible (false);
    connect (m_ui->stopButton, SIGNAL (clicked()),
             this,               SLOT (cancelDownload()));
    connect (m_ui->openButton, SIGNAL (clicked()),
             this,               SLOT (installUpdate()));

    /* Resize to fit */
    setFixedSize (minimumSizeHint());
}

//==============================================================================
// Downloader::~Downloader
//==============================================================================

Downloader::~Downloader() {
    delete m_ui;
    delete m_reply;
    delete m_manager;
}

//==============================================================================
// Downloader::useCustomInstallProcedures
//==============================================================================

bool Downloader::useCustomInstallProcedures() const {
    return m_useCustomProcedures;
}

//==============================================================================
// Updater::downloadName
//==============================================================================

QString Downloader::downloadName() const {
    return m_downloadName;
}

//==============================================================================
// Updater::localDownloadPath
//==============================================================================

QString Downloader::localDownloadPath() const {
    return m_localDownloadPath;
}

//==============================================================================
// Updater::isNotSoftwareUpdate
//==============================================================================

bool Downloader::isNotSoftwareUpdate() const {
    return m_isNotSoftwareUpdate;
}

//==============================================================================
// Updater::moduleName
//==============================================================================

QString Downloader::moduleName() const {
    return m_moduleName;
}

//==============================================================================
// Updater::moduleVersion
//==============================================================================

QString Downloader::moduleVersion() const {
    return m_moduleVersion;
}

//==============================================================================
// Updater::setLocalDownloadPath
//==============================================================================

void Downloader::setLocalDownloadPath (const QString& path) {
    m_localDownloadPath = path;
}

//==============================================================================
// Updater::setDownloadName
//==============================================================================

void Downloader::setDownloadName (const QString& name) {
    m_downloadName = name;
}

//==============================================================================
// Updater::setIsNotSoftwareUpdate
//==============================================================================

void Downloader::setIsNotSoftwareUpdate (const bool& enabled) {
    m_isNotSoftwareUpdate = enabled;
}

//==============================================================================
// Downloader::startDownload
//==============================================================================

void Downloader::startDownload (const QUrl& url) {
    /* Reset UI */
    m_ui->progressBar->setValue (0);
    m_ui->stopButton->setText (tr ("Stop"));

    if (isNotSoftwareUpdate()) {
        this->setWindowTitle(tr ("Library Update"));
        m_ui->downloadLabel->setText (tr ("Downloading %1...").arg(downloadName()));
    } else {
        this->setWindowTitle(tr ("Software Update"));
        m_ui->downloadLabel->setText (tr ("Downloading %1 update").arg(moduleName()));
    }

    /* Start download */
    m_ui->timeLabel->setText (tr ("Time remaining") + ": " + tr ("unknown"));
    m_startTime = QDateTime::currentDateTime().toTime_t();

    m_downloadRequest.setUrl(url);
    m_reply = m_manager->get (m_downloadRequest);

    /* Update UI when download progress changes or download finishes */
    connect (m_reply, SIGNAL (downloadProgress (qint64, qint64)),
             this,      SLOT (updateProgress   (qint64, qint64)));
    connect (m_reply, SIGNAL (finished()),
             this,      SLOT (onDownloadFinished()));

    showNormal();
}

//==============================================================================
// Downloader::openDownload
//==============================================================================

void Downloader::openDownload() {
    if (!m_filePath.isEmpty())
        QDesktopServices::openUrl (QUrl::fromLocalFile (m_filePath));

    else {
        QMessageBox::critical (this,
                               tr ("Error"),
                               tr ("Cannot find downloaded update!"),
                               QMessageBox::Close);
    }
}

//==============================================================================
// Downloader::installUpdate
//==============================================================================

void Downloader::installUpdate() {
    if (useCustomInstallProcedures())
        return;

    QMessageBox box;
    box.setIcon (QMessageBox::Question);
    box.setDefaultButton   (QMessageBox::Ok);
    box.setStandardButtons (QMessageBox::Ok | QMessageBox::Cancel);
    box.setText ("<b>" +
                 tr ("To install the update, you may need to "
                     "quit %1.").arg(moduleName())
                 + "</b>");
    box.setInformativeText (tr ("Click \"OK\" to begin installing the %1 update").arg(moduleName()));

    if (box.exec() == QMessageBox::Ok) {
        if (!useCustomInstallProcedures()) {
            openDownload();
            qApp->closeAllWindows();
        }
    }

    else {
        m_ui->openButton->setEnabled (true);
        m_ui->openButton->setVisible (true);
        m_ui->timeLabel->setText (tr ("Click the \"Open\" button to "
                                      "apply the update"));
    }
}

//==============================================================================
// Downloader::cancelDownload
//==============================================================================

void Downloader::cancelDownload() {
    if (!m_reply->isFinished()) {
        QMessageBox box;
        box.setWindowTitle (tr ("Software Update"));
        box.setIcon (QMessageBox::Question);
        box.setStandardButtons (QMessageBox::Yes | QMessageBox::No);
        box.setText (tr ("Are you sure you want to cancel the download?"));

        if (box.exec() == QMessageBox::Yes) {
            hide();
            m_reply->abort();
        }
    }

    else
        hide();
}

//==============================================================================
// Downloader::onDownloadFinished
//==============================================================================

void Downloader::onDownloadFinished() {
    m_ui->stopButton->setText    (tr ("Close"));
    m_ui->downloadLabel->setText (tr ("Download %1 complete!").arg(downloadName()));
    if (isNotSoftwareUpdate()) {
        this->setWindowTitle(tr ("Library Update"));
        m_ui->timeLabel->setText (tr ("Library archive written to disc."));
    } else {
        this->setWindowTitle(tr ("Software Update"));
        m_ui->timeLabel->setText (tr ("The installer will open in a separate window..."));
    }

    QByteArray data = m_reply->readAll();

    if (!data.isEmpty()) {
        QStringList list = m_reply->url().toString().split ("/");
        QString downloadContent = QString("%1/%2").arg(localDownloadPath(),downloadName());
        QFile file (isNotSoftwareUpdate() ? downloadContent : QDir::tempPath() + "/" + list.at (list.count() - 1));

        if (file.open (QIODevice::WriteOnly)) {
            file.write (data);
            file.close();

            m_filePath = file.fileName();
            emit downloadFinished (m_reply->url().toString(), m_filePath);
        } else {
            QMessageBox box;
            box.setWindowTitle (tr (isNotSoftwareUpdate() ? "Library Update" : "Software Update" ));
            box.setIcon (QMessageBox::Critical);
            box.setText (tr ("Could not save\n%1\nPlease try again.") .arg(file.fileName()));
            box.setStandardButtons (QMessageBox::Close);
        }

        if (!isNotSoftwareUpdate())
            installUpdate();
    }

}

//==============================================================================
// Downloader::calculateSizes
//==============================================================================

void Downloader::calculateSizes (qint64 received, qint64 total) {
    QString totalSize;
    QString receivedSize;

    if (total < 1024)
        totalSize = tr ("%1 bytes").arg (total);

    else if (total < 1048576)
        totalSize = tr ("%1 KB").arg (round (total / 1024));

    else
        totalSize = tr ("%1 MB").arg (round (total / 1048576));

    if (received < 1024)
        receivedSize = tr ("%1 bytes").arg (received);

    else if (received < 1048576)
        receivedSize = tr ("%1 KB").arg (received / 1024);

    else
        receivedSize = tr ("%1 MB").arg (received / 1048576);

    if (isNotSoftwareUpdate()) {
        m_ui->downloadLabel->setText (tr ("Downloading %1").arg(downloadName())
                                      + " (" + receivedSize + " " + tr ("of")
                                      + " " + totalSize + ")");
    } else {
        m_ui->downloadLabel->setText (tr ("Downloading %1 update").arg(moduleName())
                                      + " (" + receivedSize + " " + tr ("of")
                                      + " " + totalSize + ")");
    }
}

//==============================================================================
// Downloader::updateProgress
//==============================================================================

void Downloader::updateProgress (qint64 received, qint64 total) {
    if (total > 0) {
        m_ui->progressBar->setMinimum (0);
        m_ui->progressBar->setMaximum (100);
        m_ui->progressBar->setValue ((received * 100) / total);

        calculateSizes (received, total);
        calculateTimeRemaining (received, total);
    }

    else {
        m_ui->progressBar->setMinimum (0);
        m_ui->progressBar->setMaximum (0);
        m_ui->progressBar->setValue (-1);
        if (isNotSoftwareUpdate())
            m_ui->downloadLabel->setText (tr ("Downloading %1").arg(downloadName()) + "...");
        else
            m_ui->downloadLabel->setText (tr ("Downloading %1 update").arg(moduleName()) + "...");
        m_ui->timeLabel->setText (QString ("%1: %2")
                                  .arg (tr ("Time Remaining"))
                                  .arg (tr ("Unknown")));
    }
}

//==============================================================================
// Downloader::calculateTimeRemaining
//==============================================================================

void Downloader::calculateTimeRemaining (qint64 received, qint64 total) {
    uint difference = QDateTime::currentDateTime().toTime_t() - m_startTime;

    if (difference > 0) {
        QString timeString;
        float timeRemaining = total / (received / difference);

        if (timeRemaining > 7200) {
            timeRemaining /= 3600;
            timeString = tr ("About %1 hours").arg (int (timeRemaining + 0.5));
        }

        else if (timeRemaining > 60) {
            timeRemaining /= 60;
            timeString = tr ("About %1 minutes").arg (int (timeRemaining + 0.5));
        }

        else if (timeRemaining <= 60)
            timeString = tr ("%1 seconds").arg (int (timeRemaining + 0.5));

        m_ui->timeLabel->setText (tr ("Time remaining") + ": " + timeString);
    }
}

//==============================================================================
// Downloader::round
//==============================================================================

float Downloader::round (const float& input) {
    return roundf (input * 100) / 100;
}

//==============================================================================
// Downloader::setUseCustomInstallProcedures
//==============================================================================

void Downloader::setUseCustomInstallProcedures (const bool& custom) {
    m_useCustomProcedures = custom;
}
