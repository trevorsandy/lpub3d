/*
 * Copyright (c) 2014-2016 Alex Spataru <alex_spataru@outlook.com>
 * Copyright (c) 2017 Gilmanov Ildar <https://github.com/gilmanov-ildar>
 * Copyright (C) 2018 - 2020 Trevor SANDY. All rights reserved.
 *
 * This file is part of the QSimpleUpdater library, which is released under
 * the DBAD license, you can read a copy of it below:
 *
 * DON'T BE A DICK PUBLIC LICENSE TERMS AND CONDITIONS FOR COPYING,
 * DISTRIBUTION AND MODIFICATION:
 *
 * Do whatever you like with the original work, just don't be a dick.
 * Being a dick includes - but is not limited to - the following instances:
 *
 * 1a. Outright copyright infringement - Don't just copy this and change the
 *     name.
 * 1b. Selling the unmodified original with no work done what-so-ever, that's
 *     REALLY being a dick.
 * 1c. Modifying the original work to contain hidden harmful content.
 *     That would make you a PROPER dick.
 *
 * If you become rich through modifications, related works/services, or
 * supporting the original work, share the love.
 * Only a dick would make loads off this work and not buy the original works
 * creator(s) a pint.
 *
 * Code is provided with no warranty. Using somebody else's code and bitching
 * when it goes wrong makes you a DONKEY dick.
 * Fix the problem yourself. A non-dick would submit the fix back.
 */

#include <QDir>
#include <QUrl>
#include <QFile>
#include <QThread>
#include <QProcess>
#include <QDateTime>
#include <QFileDialog>
#include <QMessageBox>
#include <QNetworkReply>
#include <QDesktopServices>
#include <QNetworkAccessManager>

#include <math.h>
#include <JlCompress.h>

#include "progress_dialog.h"
#include "threadworkers.h"
#include "downloader.h"
#include "lpubalert.h"
#include "version.h"

static const QString PARTIAL_DOWN (".part");

Downloader::Downloader (QWidget* parent) : QWidget (parent)
{
    m_ui = new Ui::Downloader;
    m_ui->setupUi (this);

    /* Initialize private members */
    m_manager = new QNetworkAccessManager();

    /* Initialize internal values */
    m_url = "";
    m_fileName = "";
    m_startTime = 0;
    m_customProcedure = false;

    // LPub3D Mod
    m_portableDistro = false;
    m_showRedirects = false;
    //m_moduleName = qApp->applicationName(); // Can't use this again cuz its set to the exe name.
    m_moduleName = QString("%1").arg(VER_PRODUCTNAME_STR);
    // Mod End

    // LPub3D Mod
    /* Set default download directory */
    QStringList pathList = QStandardPaths::standardLocations(QStandardPaths::TempLocation);
    QString tempPath = pathList.first();
    m_downloadDir = tempPath + "/";
    //m_downloadDir = QDir::homePath() + "/Downloads/";
    // Mod End

    /* Make the window look like a modal dialog */
    setWindowIcon (QIcon());
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

Downloader::~Downloader()
{
    delete m_ui;
    delete m_reply;
    delete m_manager;
}

/**
 * Returns \c true if the updater shall not intervene when the download has
 * finished (you can use the \c QSimpleUpdater signals to know when the
 * download is completed).
 */
bool Downloader::customProcedure() const
{
    return m_customProcedure;
}

/**
 * Changes the URL, which is used to indentify the downloader dialog
 * with an \c Updater instance
 *
 * \note the \a url parameter is not the download URL, it is the URL of
 *       the AppCast file
 */
void Downloader::setUrlId (const QString& url)
{
    m_url = url;
}

/**
 * Begins downloading the file at the given \a url
 */
void Downloader::startDownload (const QUrl& url)
{
    /* Set URL string */
    m_downloadUrl = url.toString();

    /* Reset UI */
    m_ui->progressBar->setValue (0);
    m_ui->stopButton->setText (tr ("Stop"));
    // LPub3D Mod
    if (customProcedure()) {
        this->setWindowTitle(tr ("Library Update"));
        m_ui->downloadLabel->setText (tr ("Downloading LDraw library archive %1...").arg(m_fileName));
    } else {
        this->setWindowTitle(tr ("Software Update"));
        m_ui->downloadLabel->setText (tr ("Downloading %1 software update...").arg(m_moduleName));
    }
    // Mod End
    m_ui->timeLabel->setText (tr ("Time remaining") + ": " + tr ("unknown"));

    /* Ensure that downloads directory exists */
    if (!m_downloadDir.exists())
        m_downloadDir.mkpath (".");

    /* Remove old downloads */
    QString fullFilePath = m_downloadDir.filePath (m_fileName + PARTIAL_DOWN);
    QFile::remove (m_downloadDir.filePath (m_fileName));
    QFile::remove (fullFilePath);

    /* Create new download file */
    m_file = new QFile(fullFilePath);
    if (!m_file->open(QIODevice::WriteOnly)) {
        QMessageBox::information(this, tr("HTTP"),
                      tr("Unable to save the file %1: %2.")
                      .arg(fullFilePath).arg(m_file->errorString()));
        delete m_file;
        m_file = nullptr;
        emit downloadCancelled();
    }

    /* Start the download request */
    startRequest(url);

    showNormal();
}

/**
 * Start the dowonload request for the given \a url
 */
void Downloader::startRequest(const QUrl &url)
{
    /* Configure the network request */
    QNetworkRequest request (url);
    if (!m_userAgentString.isEmpty())
        request.setRawHeader ("User-Agent", m_userAgentString.toUtf8());

    // get() method posts a request
    // to obtain the contents of the target request
    // and returns a new QNetworkReply object
    // opened for reading which emits
    // the readyRead() signal whenever new data arrives.
    m_reply = m_manager->get (request);
    m_startTime = QDateTime::currentDateTime().toTime_t();

    // Whenever more data is received from the network,
    // this readyRead() signal is emitted
    connect(m_reply, SIGNAL(readyRead()),
            this, SLOT(httpReadyRead()));

    // Also, downloadProgress() signal is emitted when data is received
    connect (m_reply, SIGNAL (downloadProgress (qint64, qint64)),
             this,      SLOT (updateProgress   (qint64, qint64)));

    // This signal is emitted when the reply has finished processing.
    // After this signal is emitted,
    // there will be no more updates to the reply's data or metadata.
    connect (m_reply, SIGNAL (finished ()),
             this,      SLOT (finished ()));

#if QT_VERSION >= QT_VERSION_CHECK(5,6,0)
    connect (m_reply, SIGNAL (redirected    (QUrl)),
             this,      SLOT (startDownload (QUrl)));
#endif
}

/**
 * Changes the name of the downloaded file
 */
void Downloader::setFileName (const QString& file)
{
    m_fileName = file;

    if (m_fileName.isEmpty())
        m_fileName = "QSU_Update.bin";
}

/**
 * Changes the user-agent string used to communicate with the remote HTTP server
 */
void Downloader::setUserAgentString (const QString& agent)
{
    m_userAgentString = agent;
}

void Downloader::finished()
{
    /* Download finished normally */
    m_file->flush();
    m_file->close();

    /* Setup redirection MessageBox */
    QMessageBox box;
    box.setWindowTitle (tr ("HTTP Redirect"));

    /* Get redirection url */
    QVariant redirectionTarget = m_reply->attribute(QNetworkRequest::RedirectionTargetAttribute);
    if (m_reply->error()) {
        m_file->remove();

        box.setIcon (QMessageBox::Critical);
        box.setStandardButtons (QMessageBox::Ok);
        box.setText (tr ("Download redirect failed: %1.")
                     .arg(m_reply->errorString()));

        m_ui->stopButton->setText (tr ("Close"));
        emit downloadCancelled();
    } else if (!redirectionTarget.isNull()) {
        QUrl url(m_downloadUrl);
        QUrl newUrl = url.resolved(redirectionTarget.toUrl());

        bool redirect = true;
        if (m_showRedirects) {
            box.setIcon (QMessageBox::Question);
            box.setStandardButtons (QMessageBox::Yes | QMessageBox::No);
            box.setText (tr("Follow redirect to %1 ?").arg(newUrl.toString()));
            redirect = box.exec() == QMessageBox::Yes;
        }

        if (redirect) {
            url = newUrl;
            m_reply->deleteLater();
            m_file->open(QIODevice::WriteOnly);
            m_file->resize(0);
            startRequest(url);
            return;
        }
    } else {
        /* Rename file */
        QFile::rename (m_file->fileName(),
                       m_downloadDir.filePath (m_fileName));

        /* Notify application */
        emit downloadFinished (m_url, m_downloadDir.filePath (m_fileName));
        m_ui->downloadLabel->setText (tr ("Download complete!"));
        m_ui->stopButton->setText    (tr ("Close"));
    }

    m_reply->close();
    m_reply->deleteLater();
    m_reply = nullptr;
    delete m_file;
    m_file = nullptr;
    m_manager = nullptr;

    /* Install the update */
    if (!customProcedure())
      installUpdate();
    hide();
}

/**
 * Opens the downloaded file.
 * \note If the downloaded file is not found, then the function will alert the
 *       user about the error.
 */
void Downloader::openDownload()
{
    if (!m_fileName.isEmpty())
        // LPub3D Mod
        if (m_portableDistro) {
            // Automatically extract portable distribution
            QEventLoop  *wait   = new QEventLoop();
            QString newarchive  = m_downloadDir.filePath (m_fileName);
            QString destination = QDir::toNativeSeparators(tr("%1").arg(m_extractPath));
            QString message     = tr("Extracting %1. Please wait...")
                                     .arg(QFileInfo(newarchive).fileName());
            emit lpubAlert->messageSig(LOG_STATUS,message);

            QStringList items = JlCompress::getFileList(newarchive);
            ProgressDialog  *m_progressDialog;
            m_progressDialog  = new ProgressDialog(nullptr);
            m_progressDialog->setWindowFlags(m_progressDialog->windowFlags() & ~Qt::WindowCloseButtonHint);
            m_progressDialog->setWindowTitle(QString("%1 Software Update").arg(m_moduleName));
            m_progressDialog->progressBarSetLabelText(QString("Extracting %1...")
                                                      .arg(QFileInfo(newarchive).fileName()));
            m_progressDialog->progressBarSetRange(0,items.count());
            m_progressDialog->setAutoHide(false);
            m_progressDialog->setModal(true);
            m_progressDialog->show();

            QThread *thread    = new QThread(this);
            ExtractWorker *job = new ExtractWorker(newarchive,destination);
            job->moveToThread(thread);
            wait = new QEventLoop();

            connect(thread, SIGNAL(started()),
                    job, SLOT(doWork()));
            connect(thread, SIGNAL(finished()),
                    thread, SLOT(deleteLater()));
            connect(job, SIGNAL(progressSetValue(int)),
                    m_progressDialog, SLOT(progressBarSetValue(int)));
            disconnect(m_progressDialog, SIGNAL (cancelClicked()),
                    this, SLOT (cancelExporting()));
            connect(m_progressDialog, SIGNAL(cancelClicked()),
                    job, SLOT(requestEndWorkNow()));
            connect(job, SIGNAL(result(int)),
                    this, SLOT(workerJobResult(int)));
            connect(this, SIGNAL(requestEndThreadNowSig()),
                    job, SLOT(requestEndWorkNow()));
            connect(job, SIGNAL(finished()),
                    thread, SLOT(quit()));
            connect(job, SIGNAL(finished()),
                    job, SLOT(deleteLater()));
            wait->connect(job, SIGNAL(finished()),
                    wait, SLOT(quit()));

            workerJobResult(0);
            thread->start();
            wait->exec();

            m_progressDialog->progressBarSetValue(items.count());
            connect (m_progressDialog, SIGNAL (cancelClicked()),
                     this, SLOT (cancelExporting()));

            if (m_workerJobResult) {
                message = tr("Done, %1 of %2 files extracted to %3")
                             .arg(m_workerJobResult)
                             .arg(items.count())
                             .arg(destination);
                emit lpubAlert->messageSig(LOG_INFO,message);
            } else {
                message = tr("Failed to extract %1 library files")
                             .arg(QFileInfo(newarchive).fileName());
                emit lpubAlert->messageSig(LOG_ERROR,message);
            }
            m_progressDialog->progressBarSetLabelText(message);
            m_progressDialog->setBtnToClose();

        } else {
            QDesktopServices::openUrl (QUrl::fromLocalFile (m_downloadDir.filePath (
                                                                m_fileName)));
        }
        // Mod End
    else {
        QMessageBox::critical (this,
                               tr ("Error"),
                               tr ("Cannot find downloaded update!"),
                               QMessageBox::Close);
    }
}

/**
 * Instructs the OS to open the downloaded file.
 *
 * \note If \c customProcedure() returns \c true, the function will
 *       not instruct the OS to open the downloaded file. You can use the
 *       signals fired by the \c QSimpleUpdater to install the update with your
 *       own implementations/code.
 */
void Downloader::installUpdate()
{
    if (customProcedure())
        return;

    // LPub3D Mod
    this->setWindowTitle(tr ("Software Update"));
    if (m_portableDistro)
        m_ui->timeLabel->setText (tr ("%1 Windows portable distribution zip archive").arg(m_moduleName));
    else
        m_ui->timeLabel->setText (tr ("The %1 installer will open in a separate window").arg(m_moduleName)
                              + "...");
    // Mod End

    // Get the application icon as a pixmap
    QPixmap _icon = QPixmap(":/icons/setup96.png");
    if (_icon.isNull())
        _icon = QPixmap (":/icons/update.png");

    /* Ask the user to install the download */
    QMessageBox box;
    box.setWindowIcon(QIcon());
    box.setIconPixmap (_icon);
    box.setDefaultButton   (QMessageBox::Ok);
    box.setStandardButtons (QMessageBox::Ok | QMessageBox::Cancel);
    // LPub3D Mod
    box.setWindowTitle(tr ("%1 Installer").arg(m_moduleName));
    if (m_portableDistro) {
        box.setInformativeText (tr ("Click \"OK\" to extract the %1 Windows portable distribution archvie").arg(m_moduleName));
        box.setText ("<h3>" + tr ("Select a folder to extract the %1 archive.").arg(m_moduleName) + "</h3>");
    } else {
        box.setInformativeText (tr ("Click \"OK\" to begin installing the %1 update.").arg(m_moduleName));
        box.setText ("<h3>" + tr ("To install the update, %1 will quit.").arg(m_moduleName) + "</h3>");
    }
    // Mod End

    /* User wants to install the download */
    if (box.exec() == QMessageBox::Ok) {
        if (!customProcedure())
        {
            // LPub3D Mod
            if (m_portableDistro) {
                QString showMessage;
                QDir cwd(QCoreApplication::applicationDirPath());
                cwd.cdUp();
                if (cwd.dirName().contains("LPub3D_x86_"))
                    cwd.cdUp();
                QString samplePath = cwd.absolutePath();
                QString result = QFileDialog::getExistingDirectory(nullptr,
                                                                   QFileDialog::tr("Select Directory"),
                                                                   samplePath,
                                                                   QFileDialog::ShowDirsOnly |
                                                                   QFileDialog::DontResolveSymlinks);
                if (! result.isEmpty()) {
                    m_extractPath = QDir::toNativeSeparators(result);
                } else {
                    QMessageBox::warning (this,
                                           tr ("Warning"),
                                           tr (QString(QString("Empty directory path - %1 will be extracted to %2")
                                                               .arg(m_moduleName)
                                                               .arg(cwd.dirName())).toLatin1().constData()),
                                           QMessageBox::Close);
                    m_extractPath = QDir::toNativeSeparators(cwd.dirName());
                }
            } else {
                qApp->closeAllWindows();
            }
            // Mod End
            openDownload();
        }
    }

    /* Wait */
    else {
        m_ui->openButton->setEnabled (true);
        m_ui->openButton->setVisible (true);
        // LPub3D Mod
        m_ui->timeLabel->setText (tr ("Click \"Open\" apply the %1 update or "
                                      "\"Cancel to abort.\"").arg(m_moduleName));
        // Mod End
    }
}

/**
 * Prompts the user if he/she wants to cancel the download and cancels the
 * download if the user agrees to do that.
 */
void Downloader::cancelDownload()
{
    if (!m_reply->isFinished()) {
        QMessageBox box;
        box.setWindowTitle (tr ("Software Update"));
        box.setIcon (QMessageBox::Question);
        box.setStandardButtons (QMessageBox::Yes | QMessageBox::No);
        box.setText (tr ("Are you sure you want to cancel the download?"));

        if (box.exec() == QMessageBox::Yes) {
            disconnect(m_reply, SIGNAL (finished()),
                     this,      SLOT (finished()));

            if (m_file) {
                m_file->close();
                m_file->remove();
                delete m_file;
                m_file = nullptr;
            }

            m_reply->abort();
            m_reply->deleteLater();
            emit downloadCancelled();
            hide();
        }
    }
    else
        hide();
}

/**
 * Writes the downloaded data to the disk
 */
void Downloader::httpReadyRead()
{
    // this slot gets called every time the QNetworkReply has new data.
    // We read all of its new data and write it into the file.
    // That way we use less RAM than when reading it at the finished()
    // signal of the QNetworkReply
    if (m_file)
        m_file->write(m_reply->readAll());
}

/**
 * Calculates the appropiate size units (bytes, KB or MB) for the received
 * data and the total download size. Then, this function proceeds to update the
 * dialog controls/UI.
 */
void Downloader::calculateSizes (qint64 received, qint64 total)
{
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

    // LPub3D Mod
    if (customProcedure()) {
        m_ui->downloadLabel->setText (tr ("Downloading %1").arg(m_fileName)
                                      + " (" + receivedSize + " " + tr ("of")
                                      + " " + totalSize + ")");
    } else {
        m_ui->downloadLabel->setText (tr ("Downloading %1 update").arg(m_moduleName)
                                      + " (" + receivedSize + " " + tr ("of")
                                      + " " + totalSize + ")");
    }
    // Mod End
}

/**
 * Uses the \a received and \a total parameters to get the download progress
 * and update the progressbar value on the dialog.
 */
void Downloader::updateProgress (qint64 received, qint64 total)
{
    if (total > 0) {
        m_ui->progressBar->setMinimum (0);
        m_ui->progressBar->setMaximum (100);
        m_ui->progressBar->setValue (int((received * 100) / total));

        calculateSizes (received, total);
        calculateTimeRemaining (received, total);
//        saveFile (received, total);
    }

    else {
        m_ui->progressBar->setMinimum (0);
        m_ui->progressBar->setMaximum (0);
        m_ui->progressBar->setValue (-1);
        // LPub3D Mod
        if (customProcedure())
            m_ui->downloadLabel->setText (tr ("Downloading LDraw library archive %1").arg(m_fileName) + "...");
        else
            m_ui->downloadLabel->setText (tr ("Downloading %1 software update").arg(m_moduleName) + "...");
        // Mod End
        m_ui->timeLabel->setText (QString ("%1: %2")
                                  .arg (tr ("Time Remaining"))
                                  .arg (tr ("Unknown")));
    }
}

/**
 * Uses two time samples (from the current time and a previous sample) to
 * calculate how many bytes have been downloaded.
 *
 * Then, this function proceeds to calculate the appropiate units of time
 * (hours, minutes or seconds) and constructs a user-friendly string, which
 * is displayed in the dialog.
 */
void Downloader::calculateTimeRemaining (qint64 received, qint64 total)
{
    uint difference = QDateTime::currentDateTime().toTime_t() - m_startTime;

    if (difference > 0) {
        QString timeString;
        qreal timeRemaining = total / (received / difference);

        if (timeRemaining > 7200) {
            timeRemaining /= 3600;
            int hours = int (timeRemaining + 0.5);

            if (hours > 1)
                timeString = tr ("about %1 hours").arg (hours);
            else
                timeString = tr ("about one hour");
        }

        else if (timeRemaining > 60) {
            timeRemaining /= 60;
            int minutes = int (timeRemaining + 0.5);

            if (minutes > 1)
                timeString = tr ("%1 minutes").arg (minutes);
            else
                timeString = tr ("1 minute");
        }

        else if (timeRemaining <= 60) {
            int seconds = int (timeRemaining + 0.5);

            if (seconds > 1)
                timeString = tr ("%1 seconds").arg (seconds);
            else
                timeString = tr ("1 second");
        }

        m_ui->timeLabel->setText (tr ("Time remaining") + ": " + timeString);
    }
}

/**
 * Rounds the given \a input to two decimal places
 */
qreal Downloader::round (const qreal& input)
{
    return roundf (input * 100) / 100;
}

QString Downloader::downloadDir() const
{
    return m_downloadDir.absolutePath();
}

void Downloader::setDownloadDir(const QString& downloadDir)
{
    if(m_downloadDir.absolutePath() != downloadDir) {
        m_downloadDir = downloadDir;
    }
}

/**
 * If the \a custom parameter is set to \c true, then the \c Downloader will not
 * attempt to open the downloaded file.
 *
 * Use the signals fired by the \c QSimpleUpdater to implement your own install
 * procedures.
 */
void Downloader::setCustomProcedure (const bool custom)
{
    m_customProcedure = custom;
}

// LPub3D Mod
/**
 * Sets the flag to display HTTP redirects
 */
void Downloader::setShowRedirects (const bool& enabled) {
    m_showRedirects = enabled;
}

/**
 * Sets the flag to indicate download is a portable distribution
 */
void Downloader::setPortableDistro (const bool& enabled) {
    m_portableDistro = enabled;
}

// Mod End
