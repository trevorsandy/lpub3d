#include <QtOpenGL>
#include <QSysInfo>

#include <windows.h>
#include <tchar.h>
#include <QtCore/QSysInfo>

#include "aboutdialog.h"
#include "ui_aboutdialog.h"
#include "name.h"
#include "version.h"
 
AboutDialog::AboutDialog(QWidget *parent, void *data) :
	QDialog(parent),
	ui(new Ui::AboutDialog)
{
	ui->setupUi(this);

    ui->version->setTextFormat(Qt::RichText);
    ui->version->setText(tr("<b>%1</b> Version <b>%2</b> Revision %3 Build %4").arg(QString::fromLatin1(VER_PRODUCTNAME_STR))
                                                                          .arg(QString::fromLatin1(VER_PRODUCTVERSION_STR))
                                                                          .arg(QString::fromLatin1(VER_REVISION_STR))
                                                                          .arg(QString::fromLatin1(VER_BUILD_STR)));
    ui->description->setTextFormat(Qt::RichText);
    ui->description->setText(tr("<p><b>%1</b> is a free application that can be used <br>"
                                "to create digital build instructions for "
                                "virtual LEGO(c) models.</p>").arg(QString::fromLatin1(VER_PRODUCTNAME_STR)));

    ui->url->setTextFormat(Qt::RichText);
    ui->url->setText(tr("Homepage: <a href=\"%1\">sourceforge.net/lpub3d</a>").arg(QString::fromLatin1(VER_COMPANYDOMAIN_STR)));

    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    QString AboutFormat = tr("<b>%1</b> is a WYSIWYG tool for creating<br>"
                             "LEGO(c) style building instructions.<br>"
                             "%1 is a significant update and refresh of LPub.<br>"
                             "%1 source code and application can be found on<br><br>"
                             "%3<br>"
                             "Copyright (c) 2000-2011 Kevin Clague<br>"
                             "<table style=""width:100%"">"
                               "<tr>"
                                 "<td>Feedback:</td>"
                                 "<td><a href=\"%4\">%5</a></td>"
                               "</tr>"
                               "<tr>"
                                 "<td>Source:</td>"
                                 "<td><a href=\"%2files\">%2</a></td>"
                               "</tr>"
                             "</table>");
    QString About = AboutFormat.arg(QString::fromLatin1(VER_PRODUCTNAME_STR))
                               .arg(QString::fromLatin1(VER_UPDATE_URL))
                               .arg(QString::fromLatin1(VER_LEGALCOPYRIGHT_STR))
                               .arg(QString::fromLatin1(VER_PUBLISHER_SUPPORT_EMAIL_STR))
                               .arg(QString::fromLatin1(VER_PUBLISHER_EMAIL_STR));

    ui->AppInfo->setTextFormat(Qt::RichText);
    ui->AppInfo->setOpenExternalLinks(true);
    ui->AppInfo->setText(About);

    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    QString BuildInfoFormat = tr("<table style=""width:100%"">"
                                 "<tr>"
                                   "<td>Compiled on:</td>"
                                   "<td>%1</td>"
                                 "</tr>"
                                 "<tr>"
                                   "<td>Compiled for:</td>"
                                   "<td>%2</td>"
                                 "</tr>"
                                 "<tr>"
                                   "<td>Compiled with:</td>"
                                   "<td>%3</td>"
                                 "</tr>"
                                  "<tr>"
                                   "<td>Framework/Library:</td>"
                                   "<td>%4</td>"
                                 "</tr>"
                                 "</tr>"
                                  "<tr>"
                                   "<td>LeoCAD Version:</td>"
                                   "<td>%5</td>"
                                 "</tr>"
                                 "<tr>"
                                   "<td>Build Date</td>"
                                   "<td>%6</td>"
                                 "</tr>"
                               "</table>");
    QString BuildInfo = BuildInfoFormat.arg(QString::fromLatin1(VER_COMPILED_ON))
                                       .arg(QString::fromLatin1(VER_COMPILED_FOR))
                                       .arg(QString::fromLatin1(VER_COMPILED_WITH))
                                       .arg(QString("Qt %1").arg(qVersion()))
                                       .arg(tr("%1 - Revision %2").arg(QString::fromLatin1(LC_VERSION_TEXT))
                                                                  .arg(QString::fromLatin1(LC_VERSION_BUILD)))
                                       .arg(QString::fromLatin1(VER_BUILDDATE_STR));

    ui->BuildInfo->setTextFormat(Qt::RichText);
    ui->BuildInfo->setText(BuildInfo);

    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    QString osPlatform;
    int OS = checkOS();
    if (OS == Win_64)
        osPlatform   = "64-bit system";
    else if(OS == Win_32)
        osPlatform   = "32-bit system";
    else if (OS == OsError)
        osPlatform  = "Error encountered";
    else
        osPlatform = "Cannot determine system";

    qDebug() << "CheckOS():" << checkOS() << " Return " << osPlatform;
    QString OsInfoFormat = tr("<table style=""width:100%"">"
                              "<tr>"
                                "<td>Name:</td>"
                                "<td>%1</td>"
                              "</tr>"
                              "<tr>"
                                "<td>Architecture</td>"
                                "<td>%2</td>"
                              "</tr>"
                              "<tr>"
                                "<td>OpenGL Version</td>"
                                "<td>%3<br>%4 - %5</td>"
                              "</tr>"
                            "</table>");
    QString OsInfo = OsInfoFormat.arg(osName())
                                 .arg(osPlatform)
                                 .arg(QString((const char*)glGetString(GL_VERSION)),
                                      QString((const char*)glGetString(GL_RENDERER)),
                                      QString((const char*)glGetString(GL_VENDOR)));

    ui->OsInfo->setTextFormat(Qt::RichText);
    ui->OsInfo->setText(OsInfo);
}

AboutDialog::~AboutDialog()
{
	delete ui;
}

QString AboutDialog::osName()
{
#ifdef Q_WS_WIN
    switch(QSysInfo::windowsVersion())
    {
      case QSysInfo::WV_6_1: return "Windows 7";
      case QSysInfo::WV_6_2: return "Windows 8";
      case QSysInfo::WV_6_3: return "Windows 8.1";
      default: return "Windows";
    }
#endif
#ifdef Q_OS_MAC
    switch(QSysInfo::MacintoshVersion)
    {
      case QSysInfo::MV_LION:         return "OS X Lion";           // Version 10.7
      case QSysInfo::MV_MOUNTAINLION: return "OS X Mountain Lion";  // Version 10.8
      case QSysInfo::MV_MAVERICKS:    return "OS X Mavericks";      // version 10.9
      case QSysInfo::MW_YOSEMITE:     return "OS X Yosemite";       // Version 10.10
      default: return "OS X";
    }
#endif

}

//int AboutDialog::checkOS()
OsType AboutDialog::checkOS()
{
#ifndef Q_OS_WIN32
    return OsOther;
#else
    // An application compiled for 64 bits can only run on a 64 bit os,
    // so no need to check any further.
    if (QSysInfo::WordSize == 64) return Win_64;
    // A 32 bit application may be running on a 64 bit OS.
    BOOL is64 = FALSE;
    // IsWow64Process may not be available in kernel32 on all Windows versions, so we bind to it
    // at runtime.
    LPFN_ISWOW64PROCESS fnIsWow64Process;
    fnIsWow64Process = (LPFN_ISWOW64PROCESS)
            GetProcAddress(GetModuleHandle(TEXT("kernel32")),"IsWow64Process");
    // No way it's a 64 bit OS if it doesn't have this API.
    if (fnIsWow64Process == NULL) return Win_32;
    // Note that GetCurrentProcess() can't fail.
    if (!IsWow64Process(GetCurrentProcess(), &is64)) return OsError; // The check has failed.
    return is64 ? Win_64 : Win_32;
#endif

}


