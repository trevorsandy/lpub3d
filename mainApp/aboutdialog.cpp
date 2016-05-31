/****************************************************************************
**
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

#include <QtOpenGL>
#include <QSysInfo>

#include <windows.h>
#include <tchar.h>
#include <QtCore/QSysInfo>

#include "aboutdialog.h"
#include "ui_aboutdialog.h"
#include "name.h"
#include "version.h"
#include "lc_global.h"
#include "lpub_preferences.h"
 
AboutDialog::AboutDialog(QWidget *parent, void *data) :
	QDialog(parent),
	ui(new Ui::AboutDialog)
{
	ui->setupUi(this);

    ui->contentGroupBox->hide();

    ui->version->setTextFormat(Qt::RichText);
    ui->version->setText(tr("Version <b>%1</b> Revision <b>%2</b> Build <b>%3</b>").arg(QString::fromLatin1(VER_PRODUCTVERSION_STR))
                                                                                   .arg(QString::fromLatin1(VER_REVISION_STR))
                                                                                   .arg(QString::fromLatin1(VER_BUILD_STR)));
    ui->description->setTextFormat(Qt::RichText);
    ui->description->setText(tr("<p><b>%1</b> is an LDraw&trade; building instruction editor.<br><br>"
                                "%2</p>").arg(QString::fromLatin1(VER_PRODUCTNAME_STR))
                                             .arg(QString::fromLatin1(VER_LEGALCOPYRIGHT_STR)));

    ui->url->setTextFormat(Qt::RichText);
    ui->url->setText(tr("Homepage: <a href=\"%1\">%1</a>").arg(QString::fromLatin1(VER_COMPANYDOMAIN_STR)));

    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    QString AboutFormat = tr("<table style=\"width:100%\">"
                             "<tr>"
                               "<td>"
                                  "<table>"
                                     "<tr>"
                                       "<td>"
                                         "<b>%1</b> is a free WYSIWYG editing application for<br>"
                                         "creating LEGO&copy; style digital building instructions<br>"
                                         "for LDraw&trade; based digital models.<br><br>"
                                         "%1 is a major refresh of Kevin Clague's LPub&copy;<br>"
                                         "incorporating LeoCAD&copy; to render 3D visualization.<br>"
                                       "</td>"
                                       "<td valign=\"middle\">"
                                         "<a href=\"%5\"><img src=\":/resources/gplv3.png\"></a>"
                                       "</td>"
                                    "</tr>"
                                  "</table>"
                                "</td>"
                             "</tr>"
                             "<tr>"
                                "<td>"
                                   "<table>"
                                     "<tr>"
                                       "<td>Feedback:</td>"
                                       "<td><a href=\"%3\">%4</a></td>"
                                     "</tr>"
                                     "<tr>"
                                       "<td>Source:</td>"
                                       "<td><a href=\"%2\">%2</a></td>"
                                     "</tr>"
                                  "</table>"
                                "</td>"
                             "</tr>"
                             "</table>");
    QString About = AboutFormat.arg(QString::fromLatin1(VER_PRODUCTNAME_STR))
                               .arg(QString::fromLatin1(VER_SOURCE_URL))
                               .arg(QString::fromLatin1(VER_PUBLISHER_SUPPORT_EMAIL_STR))
                               .arg(QString::fromLatin1(VER_PUBLISHER_EMAIL_STR))
                               .arg(QString("http://www.gnu.org/licenses/gpl-3.0.html"));

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
                                   "<td>Framework/Library:&nbsp;</td>"
                                   "<td>%4</td>"
                                 "</tr>"
                                 "<tr>"
                                   "<td>Dev Environment:</td>"
                                   "<td>%5</td>"
                                 "</tr>"
                                 "<tr>"
                                   "<td>LeoCAD Version:</td>"
                                   "<td>%6</td>"
                                 "</tr>"
                                 "<tr>"
                                   "<td>Build Date:</td>"
                                   "<td>%7</td>"
                                 "</tr>"
                               "</table>");
    QString BuildInfo = BuildInfoFormat.arg(QString::fromLatin1(VER_COMPILED_ON))
                                       .arg(QString::fromLatin1(VER_COMPILED_FOR))
                                       .arg(QString::fromLatin1(VER_COMPILED_WITH))
                                       .arg(QString("Qt %1").arg(qVersion()))
                                       .arg(QString::fromLatin1(VER_IDE))
                                       .arg(tr("%1 - Revision %2").arg(QString::fromLatin1(LC_VERSION_TEXT))
                                                                  .arg(QString::fromLatin1(LC_VERSION_BUILD)))
                                       .arg(QString::fromLatin1(VER_BUILDDATE_STR));

    ui->BuildInfo->setTextFormat(Qt::RichText);
    ui->BuildInfo->setText(BuildInfo);

    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    QString osPlatform;
#ifdef Q_OS_WIN
    int OS = checkOS();
    if (OS == Win_64)
        osPlatform   = "64-bit system";
    else if(OS == Win_32)
        osPlatform   = "32-bit system";
    else if (OS == OsOther)
        osPlatform  = "Other Operating System";
    else if (OS == OsError)
        osPlatform  = "Error encountered";
    else
        osPlatform = "Cannot determine system";
#endif
#ifdef Q_OS_MAC
    osPlatform = QSysInfo::currentCpuArchitecture();
#endif
    //TODO add Linus check

    QString OsInfoFormat = tr("<table style=""width:100%"">"
                              "<tr>"
                                "<td>Name:</td>"
                                "<td>%1</td>"
                              "</tr>"
                              "<tr>"
                                "<td>Architecture:</td>"
                                "<td>%2</td>"
                              "</tr>"
                              "<tr>"
                                "<td>OpenGL Version:&nbsp;</td>"
                                "<td>%3<br>%4 - %5</td>"
                              "</tr>"
                            "</table>");
    QString OsInfo = OsInfoFormat.arg(osName())
                                 .arg(osPlatform)
                                 .arg(QString((const char*)glGetString(GL_VERSION)),
                                      QString((const char*)glGetString(GL_RENDERER)).replace("(R)", "&reg;"),
                                      QString((const char*)glGetString(GL_VENDOR)));

    ui->OsInfo->setTextFormat(Qt::RichText);
    ui->OsInfo->setText(OsInfo);

    //buttonBar additions
    detailsButton = new QPushButton(tr("Version Details..."));
    detailsButton->setDefault(true);
    ui->buttonBox->addButton(detailsButton,QDialogButtonBox::ActionRole);

    connect(detailsButton,SIGNAL(clicked(bool)),
            this,SLOT(showReadmeDetails(bool)));

    creditsButton = new QPushButton(tr("Credits..."));
    creditsButton->setDefault(true);
    ui->buttonBox->addButton(creditsButton,QDialogButtonBox::ActionRole);

    connect(creditsButton,SIGNAL(clicked(bool)),
            this,SLOT(showCreditDetails(bool)));
}

AboutDialog::~AboutDialog()
{
	delete ui;
}

void AboutDialog::showReadmeDetails(bool clicked){
    clicked = clicked;
    //populate readme
    QString readmeFile = QString("%1/%2").arg(Preferences::lpub3dPath).arg("README.txt");
    QFile file(readmeFile);
    if (! file.open(QFile::ReadOnly | QFile::Text)) {
        content = QString("Failed to open Readme file: \n%1:\n%2")
                            .arg(readmeFile)
                            .arg(file.errorString());
        ui->contentEdit->setPlainText(content);
    } else {
        QTextStream in(&file);
        while (! in.atEnd()){
            content = in.readAll();
        }
        ui->contentEdit->setPlainText(content);
    }

    if (ui->contentGroupBox->isHidden()){
        ui->contentGroupBox->show();
        this->adjustSize();
    }
    else{
        ui->contentGroupBox->hide();
        this->adjustSize();
    }
}

void AboutDialog::showCreditDetails(bool clicked){
    clicked = clicked;
    //populate credits
    QString creditsFile = QString("%1/%2").arg(Preferences::lpub3dPath).arg("/docs/CREDITS.txt");
    QFile file(creditsFile);
    if (! file.open(QFile::ReadOnly | QFile::Text)) {
        content = QString("Failed to open Credits file: \n%1:\n%2")
                            .arg(creditsFile)
                            .arg(file.errorString());
        ui->contentEdit->setPlainText(content);
    } else {
        QTextStream in(&file);
        while (! in.atEnd()){
            content = in.readAll();
        }
        ui->contentEdit->setPlainText(content);
    }

    if (ui->contentGroupBox->isHidden()){
        ui->contentGroupBox->show();
        this->adjustSize();
    }
    else{
        ui->contentGroupBox->hide();
        this->adjustSize();
    }
}

QString AboutDialog::osName()
{
#ifdef Q_OS_WIN
  #if QT_VERSION >= QT_VERSION_CHECK(5,5,0)
    switch(QSysInfo::windowsVersion())
    {
      case QSysInfo::WV_5_1: return "Windows XP";
      case QSysInfo::WV_5_2: return "Windows 2003";
      case QSysInfo::WV_6_0: return "Windows Vista";
      case QSysInfo::WV_6_1: return "Windows 7";
      case QSysInfo::WV_6_2: return "Windows 8";
      case QSysInfo::WV_6_3: return "Windows 8.1";
      case QSysInfo::WV_10_0: return "Windows 10";
      default: return "Windows";
    }
  #else
  switch(QSysInfo::windowsVersion())
  {
    case QSysInfo::WV_5_1: return "Windows XP";
    case QSysInfo::WV_5_2: return "Windows 2003";
    case QSysInfo::WV_6_0: return "Windows Vista";
    case QSysInfo::WV_6_1: return "Windows 7";
    case QSysInfo::WV_6_2: return "Windows 8";
    case QSysInfo::WV_6_3: return "Windows 8.1 or later";
    default: return "Windows";
  }
  #endif
#endif
#ifdef Q_OS_MAC
  #if QT_VERSION >= QT_VERSION_CHECK(5,5,0)
    switch(QSysInfo::MacintoshVersion)
    {
      case QSysInfo::MV_LION:         return "OS X Lion";           // Version 10.7
      case QSysInfo::MV_MOUNTAINLION: return "OS X Mountain Lion";  // Version 10.8
      case QSysInfo::MV_MAVERICKS:    return "OS X Mavericks";      // version 10.9
      case QSysInfo::MW_YOSEMITE:     return "OS X Yosemite";       // Version 10.10
      case QSysInfo::MV_ELCAPITAN:    return "OS X El Capitan";     // Version 10.11
      default: return "OS X";
    }
  #else
  switch(QSysInfo::MacintoshVersion)
  {
    case QSysInfo::MV_LION:         return "OS X Lion";                    // Version 10.7
    case QSysInfo::MV_MOUNTAINLION: return "OS X Mountain Lion";           // Version 10.8
    case QSysInfo::MV_MAVERICKS:    return "OS X Mavericks";               // version 10.9
    case QSysInfo::MW_YOSEMITE:     return "OS X Yosemite or later";       // Version 10.10
    default: return "OS X";
  }
  #endif
#endif
 //TODO add Linux Check
}

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


