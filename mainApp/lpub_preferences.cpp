/****************************************************************************
**
** Copyright (C) 2007-2009 Kevin Clague. All rights reserved.
** Copyright (C) 2016 Trevor SANDY. All rights reserved.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://www.trolltech.com/products/qt/opensource.html
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/
#include <QDesktopServices>
#include <QSettings>
#include <QFileInfo>
#include <QProcess>
#include <QString>
#include <QStringList>
#include <QDir>
#include <QFileDialog>
#include <QMessageBox>
#include <QDate>

#include "lpub_preferences.h"
#include "ui_preferences.h"
#include "preferencesdialog.h"
#include "updatecheck.h"
#include "resolution.h"
#include "render.h"
#include "pli.h"
#include "version.h"
#include "name.h"
#include "application.h"
//**3D
#include "lc_profile.h"
//**

Preferences preferences;
QDate date = QDate::currentDate();

QString Preferences::ldrawPath                  = "";
QString Preferences::lpub3dLibFile              = "";
QString Preferences::lgeoPath;
QString Preferences::lpub3dPath                 = ".";
QString Preferences::lpubDataPath               = ".";
QString Preferences::lpubExtrasPath             = ".";
QString Preferences::ldgliteExe;
QString Preferences::ldviewExe;
QString Preferences::l3pExe;
QString Preferences::povrayExe;
QString Preferences::preferredRenderer;
QString Preferences::pliFile;
QString Preferences::titleAnnotationsFile;
QString Preferences::freeformAnnotationsFile;
QString Preferences::fadeStepColor              = "Very_Light_Bluish_Gray";
QString Preferences::pliSubstitutePartsFile;
QString Preferences::fadeStepColorPartsFile;
//page attributes dynamic
QString Preferences::defaultAuthor;
QString Preferences::defaultURL;
QString Preferences::defaultEmail;
QString Preferences::documentLogoFile;
QString Preferences::publishDescription;
QString Preferences::ldrawiniFile;
QString Preferences::moduleVersion               = qApp->applicationVersion();
QString Preferences::ldgliteSearchDirs;
QStringList Preferences::ldSearchDirs;
//Static page attributes
QString Preferences::disclaimer                 = QString(QObject::trUtf8("LEGO® is a registered trademark of the LEGO Group, \n"
                                                                          "which does not sponsor, endorse, or authorize these \n"
                                                                          "instructions or the model they depict."));
QString Preferences::copyright                  = QString(QObject::trUtf8("Copyright © %1").arg(date.toString("yyyy")));
QString Preferences::plugImage                  = QString(":/resources/LPub64.png");
QString Preferences::plug                       = QString(QObject::trUtf8("Instructions configured and generated using %1 %2 \n Download %1 at %3")
                                                          .arg(QString::fromLatin1(VER_PRODUCTNAME_STR),
                                                               QString::fromLatin1(VER_FILEVERSION_STR),
                                                               QString::fromLatin1(VER_COMPANYDOMAIN_STR)));

bool    Preferences::enableDocumentLogo         = false;
bool    Preferences::enableLDViewSingleCall     = true;
bool    Preferences::useLDViewSingleCall        = false;
bool    Preferences::displayAllAttributes       = false;
bool    Preferences::generateCoverPages         = false;
bool    Preferences::printDocumentTOC           = false;
//

bool    Preferences::enableFadeStep             = false;
bool    Preferences::preferCentimeters          = true;
bool    Preferences::showAllNotifications       = true;
bool    Preferences::showUpdateNotifications    = true;
bool    Preferences::enableDownloader           = true;
bool    Preferences::ldrawiniFound              = false;
bool    Preferences::fadeStepSettingChanged     = false;
bool    Preferences::fadeStepColorChanged       = false;
bool    Preferences::portableDistribution       = false;
int     Preferences::checkUpdateFrequency       = 2;        //0=Never,1=Daily,2=Weekly,3=Monthly

int     Preferences::pageHeight                 = 800;
int     Preferences::pageWidth                  = 600;

Preferences::Preferences()
{
}

void Preferences::lpubPreferences()
{    
    QDir cwd(QDir::currentPath());

    if (cwd.dirName() == "MacOS") {
        cwd.cdUp(); //MacOS
        cwd.cdUp(); //Contents
        cwd.cdUp(); //LPub3D.app
    }

    lpub3dPath = cwd.absolutePath();

    emit Application::instance()->splashMsgSig("5% - Initialize user data directory...");

    if (QDir(lpub3dPath + "/extras").exists()) { // we have a portable distribution

        portableDistribution = true;

#ifdef Q_OS_WIN                                  // ... portable on Windows (macro)

        bool programFolder = QCoreApplication::applicationDirPath().contains("Program Files") ||
                QCoreApplication::applicationDirPath().contains("Program Files (x86)");

        if (programFolder) {                     // ...installed in Program Folder directory
            // Get the application icon as a pixmap
            QPixmap _icon = QPixmap(":/icons/lpub96.png");
            QMessageBox box;
            box.setWindowIcon(QIcon());
            box.setIconPixmap (_icon);
            box.setTextFormat (Qt::RichText);
            box.setWindowTitle(QMessageBox::tr ("Installation"));
            box.setWindowFlags (Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);

            QString header  = "<b>" + QMessageBox::tr ("Data directory installation folder.") + "</b>";
            QString body = QMessageBox::tr ("Would you like to create a folder outside the Program Files / (x86) directory? \n"
                                            "If you choose No, the data directory will automatically be created in the user's AppData directory.");
            QString detail = QMessageBox::tr ("It looks like this installation is a portable or packaged (i.e. AIOI) distribution \n"
                                              "of LPub3D installed under the system's Program Files/(x86) directory.\n\n"
                                              "Updatable data will not be able to be written to unless you modify\n"
                                              "user account access for this folder which is not recommended.\n\n"
                                              "You should consider changing the installation folder or placing\n"
                                              "the updatable data folder outside the Program Files/(x86) directory\n\n"
                                              "Choose yes to continue and select a data folder outside Program Files/(x86).\n\n"
                                              "If you choose No, the data directory will automatically be created in the user's AppData directory.");
            box.setText (header);
            box.setInformativeText (body);
            box.setDetailedText(detail);
            box.setStandardButtons (QMessageBox::No | QMessageBox::Yes);
            box.setDefaultButton   (QMessageBox::Yes);

            if (box.exec() != QMessageBox::Yes) {   // user choose not to create user data direcory outside program folder, so create automatically

                #if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
                        QStringList dataPathList = QStandardPaths::standardLocations(QStandardPaths::DataLocation);
                        lpubDataPath = dataPathList.first();
                #else
                        lpubDataPath = QDesktopServices::storageLocation(QDesktopServices::DataLocation);
                #endif

            } else {                                // capture user's choice for user data directory

                QStringList dataPathList = QStandardPaths::standardLocations(QStandardPaths::DataLocation);
                lpubDataPath = dataPathList.first();
                QString result = QFileDialog::getExistingDirectory(NULL,
                                                                   QFileDialog::tr("Select Directory"),
                                                                   lpubDataPath,
                                                                   QFileDialog::ShowDirsOnly |
                                                                   QFileDialog::DontResolveSymlinks);
                if (! result.isEmpty())
                    lpubDataPath = QDir::toNativeSeparators(result);
            }

        } else {                                    // ...installed outside Program Folder directory

            lpubDataPath = lpub3dPath;
        }
#else                                               // ...portable on other than Windows (macro)
      lpubDataPath = lpub3dPath;
#endif

    } else {                                        // we havea an installed distribution

#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
        QStringList dataPathList = QStandardPaths::standardLocations(QStandardPaths::DataLocation);
        lpubDataPath = dataPathList.first();
#else
        lpubDataPath = QDesktopServices::storageLocation(QDesktopServices::DataLocation);
#endif

    }

    qDebug() << "LPub3D data path: " << lpubDataPath;

    emit Application::instance()->splashMsgSig("5% - Initialize extras directory...");
    QDir extrasDir(lpubDataPath + "/extras");
    if(!QDir(extrasDir).exists())
        extrasDir.mkpath(".");

    QString location = QString("%1").arg((portableDistribution ? "/extras/" : "/data/"));

    QFileInfo paramFile;
    paramFile.setFile(QString("%1/%2").arg(extrasDir.absolutePath(), VER_FADESTEP_COLORPARTS_FILE));
    if (!paramFile.exists())
        QFile::copy(lpub3dPath + location + paramFile.fileName(), paramFile.absoluteFilePath());
    paramFile.setFile(QString("%1/%2").arg(extrasDir.absolutePath(), VER_FREEFOM_ANNOTATIONS_FILE));
    if (!paramFile.exists())
        QFile::copy(lpub3dPath + location + paramFile.fileName(), paramFile.absoluteFilePath());
    paramFile.setFile(QString("%1/%2").arg(extrasDir.absolutePath(), VER_EXTRAS_LDCONFIG_FILE));
    if (!paramFile.exists())
        QFile::copy(lpub3dPath + location + paramFile.fileName(), paramFile.absoluteFilePath());
    paramFile.setFile(QString("%1/%2").arg(extrasDir.absolutePath(), VER_PDFPRINT_IMAGE_FILE));
    if (!paramFile.exists())
        QFile::copy(lpub3dPath + location + paramFile.fileName(), paramFile.absoluteFilePath());
    paramFile.setFile(QString("%1/%2").arg(extrasDir.absolutePath(), VER_PLI_MPD_FILE));
    if (!paramFile.exists())
        QFile::copy(lpub3dPath + location + paramFile.fileName(), paramFile.absoluteFilePath());
    paramFile.setFile(QString("%1/%2").arg(extrasDir.absolutePath(), VER_PLI_SUBSTITUTE_PARTS_FILE));
    if (!paramFile.exists())
        QFile::copy(lpub3dPath + location + paramFile.fileName(), paramFile.absoluteFilePath());
    paramFile.setFile(QString("%1/%2").arg(extrasDir.absolutePath(), VER_TITLE_ANNOTATIONS_FILE));
    if (!paramFile.exists())
        QFile::copy(lpub3dPath + location + paramFile.fileName(), paramFile.absoluteFilePath());
}

void Preferences::lpub3dLibPreferences(bool force)
{
    emit Application::instance()->splashMsgSig("15% - Locate LDraw archive libraries...");

#ifdef Q_OS_WIN
    QString filter(QFileDialog::tr("Archive (*.zip *.bin);;All Files (*.*)"));
#else
    QString filter(QFileDialog::tr("All Files (*.*)"));
#endif

    QSettings Settings;
    QFileInfo validFile;
    QString const LPub3DLibKey("PartsLibrary");

    if (Settings.contains(QString("%1/%2").arg(SETTINGS,LPub3DLibKey))) {
        lpub3dLibFile = Settings.value(QString("%1/%2").arg(SETTINGS,LPub3DLibKey)).toString();
    } else
    if (portableDistribution) {
        lpub3dLibFile = QString("%1/%2/%3").arg(lpubDataPath, "libraries", VER_LDRAW_OFFICIAL_ARCHIVE);
        Settings.setValue(QString("%1/%2").arg(SETTINGS, LPub3DLibKey), lpub3dLibFile);
    }


    if (!lpub3dLibFile.isEmpty() && ! force) {
        validFile.setFile(lpub3dLibFile);

        if (validFile.exists()) {
            return;
        }
        else {
            lpub3dLibFile.clear();
            Settings.remove(QString("%1/%2").arg(SETTINGS, LPub3DLibKey));
        }
    }

    if (! lpub3dLibFile.isEmpty() && force){

        QString result = QFileDialog::getOpenFileName(NULL,
                                                      QFileDialog::tr("Select LDraw Library Archive"),
                                                      lpub3dLibFile,
                                                      filter);
        if (! result.isEmpty())
            lpub3dLibFile = QDir::toNativeSeparators(result);
    }

    if (! lpub3dLibFile.isEmpty()) {
        Settings.setValue(QString("%1/%2").arg(SETTINGS, LPub3DLibKey), lpub3dLibFile);
    }
    else {
        QPixmap _icon = QPixmap(":/icons/lpub96.png");
        QMessageBox box;
        box.setWindowIcon(QIcon());
        box.setIconPixmap (_icon);
        box.setTextFormat (Qt::RichText);
        box.setWindowTitle(QMessageBox::tr ("Library Selection"));
        box.setWindowFlags (Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);

        QString header = "<b>" + QMessageBox::tr ("No LDraw library defined!") + "</b>";
        QString body   = QMessageBox::tr ("Would you like to select, copy or download the libraries?");
        QString detail = QMessageBox::tr ("You must select or create your LDraw library archive files.\n"
                                          "The location of your official archive file (complete.zip) should "
                                          "also have the unofficial archive file (lpub3dldrawunf.zip).\n"
                                          "LDraw library archive files can be copied, downloaded or selected to your '%1/%2/' folder now.")
                                          .arg(lpubDataPath, "libraries");
        box.setText (header);
        box.setInformativeText (body);
        box.setDetailedText(detail);                                                                                                                                  
        QAbstractButton* copyButton = box.addButton(QMessageBox::tr("Copy"),QMessageBox::YesRole);
        QAbstractButton* downloadButton = box.addButton(QMessageBox::tr("Download"),QMessageBox::YesRole);
        QAbstractButton* selectButton = box.addButton(QMessageBox::tr("Select"),QMessageBox::YesRole);
        box.setStandardButtons (QMessageBox::Cancel);
        box.exec();

        if (box.clickedButton()==copyButton) {
            emit Application::instance()->splashMsgSig("15% - Copying archive libraries...");

            QDir libraryDir(QString("%1/%2").arg(lpubDataPath, "libraries"));
            if (!QDir(libraryDir).exists())
                libraryDir.mkpath(".");

            QString location = QString("%1").arg((portableDistribution ? "/libraries/" : "/data/"));

            validFile.setFile(QString("%1/%2").arg(libraryDir.absolutePath(), VER_LDRAW_OFFICIAL_ARCHIVE));
            if (!validFile.exists())
                QFile::copy(lpub3dPath + location + validFile.fileName(), validFile.absoluteFilePath());

            lpub3dLibFile = validFile.absoluteFilePath();
            Settings.setValue(QString("%1/%2").arg(SETTINGS, LPub3DLibKey), lpub3dLibFile);

            validFile.setFile(QString("%1/%2").arg(libraryDir.absolutePath(), VER_LPUB3D_UNOFFICIAL_ARCHIVE));
            if (!validFile.exists())
                QFile::copy(lpub3dPath + location + validFile.fileName(), validFile.absoluteFilePath());

        } else
          if (box.clickedButton()==selectButton) {
            emit Application::instance()->splashMsgSig("15% - Selecting archive libraries...");

            QFileDialog dlgGetFileName(NULL,QFileDialog::tr("Select LDraw Library Archive "));
            dlgGetFileName.setDirectory(lpubDataPath);
            dlgGetFileName.setNameFilter(filter);

            if (dlgGetFileName.exec() == QFileDialog::Accepted) {
                lpub3dLibFile = dlgGetFileName.selectedFiles().at(0);
                Settings.setValue(QString("%1/%2").arg(SETTINGS, LPub3DLibKey), lpub3dLibFile);
            } else {
                Settings.remove(QString("%1/%2").arg(SETTINGS, LPub3DLibKey));
                body = QMessageBox::tr ("Selection cancelled.");
                body.append("\nThe application will terminate.");

                box.removeButton(copyButton);
                box.removeButton(selectButton);
                box.removeButton(downloadButton);
                box.setStandardButtons (QMessageBox::Close);
                box.setText(header);
                box.setInformativeText(body);

                box.exec();
                lpub3dLibFile.clear();

                exit(-1);
            }

        } else
          if (box.clickedButton()==downloadButton) {

            emit Application::instance()->splashMsgSig("15% - Downloading archive libraries...");

            QDir libraryDir(QString("%1/%2").arg(lpubDataPath, "libraries"));
            if (!QDir(libraryDir).exists())
                libraryDir.mkpath(".");

            UpdateCheck *libraryDownload;
            QEventLoop *wait = new QEventLoop();
            libraryDownload  = new UpdateCheck(NULL, (void*)LDrawOfficialLibraryDirectDownload);
            wait->connect(libraryDownload, SIGNAL(downloadFinished(QString,QString)), wait, SLOT(quit()));
            libraryDownload->requestDownload(libraryDownload->getDEFS_URL(), libraryDir.absolutePath());
            wait->exec();
            libraryDownload  = new UpdateCheck(NULL, (void*)LDrawUnofficialLibraryDirectDownload);
            wait->connect(libraryDownload, SIGNAL(downloadFinished(QString,QString)), wait, SLOT(quit()));
            libraryDownload->requestDownload(libraryDownload->getDEFS_URL(), libraryDir.absolutePath());
            wait->exec();

            lpub3dLibFile = QDir::toNativeSeparators(QString("%1/%2").arg(libraryDir.absolutePath(), VER_LDRAW_OFFICIAL_ARCHIVE));
            validFile.setFile(lpub3dLibFile);
            if (validFile.exists()) {
                Settings.setValue(QString("%1/%2").arg(SETTINGS, LPub3DLibKey), lpub3dLibFile);
            }
            else {
                Settings.remove(QString("%1/%2").arg(SETTINGS, LPub3DLibKey));
                validFile.setFile(QDir::toNativeSeparators(QString("%1/%2").arg(libraryDir.absolutePath(), VER_LDRAW_UNOFFICIAL_ARCHIVE)));
                if (validFile.exists()) {
                    body = QMessageBox::tr ("Files %1 and\n%2 does not exist.").arg(lpub3dLibFile, validFile.absoluteFilePath());
                } else {
                    body = QMessageBox::tr ("File %1 does not exist.").arg(lpub3dLibFile);
                }
                body.append("\nThe application will terminate.");

                box.setStandardButtons (QMessageBox::Close);
                box.setText(header);
                box.setInformativeText(body);

                box.exec();
                lpub3dLibFile.clear();

                exit(-1);
            }
        }
        else {
            exit(-1);
        }
    }

    if (Settings.contains(QString("%1/%2").arg(SETTINGS,LPub3DLibKey)))
        lcSetProfileString(LC_PROFILE_PARTS_LIBRARY, Settings.value(QString("%1/%2").arg(SETTINGS,LPub3DLibKey)).toString());
}

void Preferences::ldrawPreferences(bool force)
{
    emit Application::instance()->splashMsgSig("10% - Locate LDraw directory...");

    QSettings Settings;
    QString const ldrawKey("LDrawDir");

    if (Settings.contains(QString("%1/%2").arg(SETTINGS,ldrawKey))) {
        ldrawPath = Settings.value(QString("%1/%2").arg(SETTINGS,ldrawKey)).toString();
    }

    if (! ldrawPath.isEmpty() && ! force) {
        QDir ldrawDir(ldrawPath);

        if (ldrawDir.exists()) {
            return;
        } else {
            ldrawPath.clear();
            Settings.remove(QString("%1/%2").arg(SETTINGS,ldrawKey));
        }
    }

    if (ldrawPath.isEmpty() && ! force) {

        ldrawPath = "c:\\LDraw";
        QDir guesses;
        guesses.setPath(ldrawPath);
        if ( ! guesses.exists()) {
            ldrawPath = "c:\\Program Files (x86)\\LDraw";
            guesses.setPath(ldrawPath);
            if ( ! guesses.exists()) {

                ldrawPath = QFileDialog::getExistingDirectory(NULL,
                                                              QFileDialog::tr("Locate LDraw Directory"),
                                                              "/",
                                                              QFileDialog::ShowDirsOnly |
                                                              QFileDialog::DontResolveSymlinks);
            }
        }
    }

    if (! ldrawPath.isEmpty() && force){

        QString result = QFileDialog::getExistingDirectory(NULL,
                                                           QFileDialog::tr("Select LDraw Directory"),
                                                           ldrawPath,
                                                           QFileDialog::ShowDirsOnly |
                                                           QFileDialog::DontResolveSymlinks);

        if (! result.isEmpty())
            ldrawPath = QDir::toNativeSeparators(result);
    }

    if (! ldrawPath.isEmpty()) {

        Settings.setValue(QString("%1/%2").arg(SETTINGS,ldrawKey),ldrawPath);

    } else {

        QString question = QMessageBox::tr("You must enter your LDraw directory. \nDo you wish to continue?");
        if (QMessageBox::question(NULL, "LDraw3D", question, QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes)
            exit(-1);
    }
}

void Preferences::lpub3dUpdatePreferences(){

    emit Application::instance()->splashMsgSig("15% - Selecting update settings...");

    QSettings Settings;

    moduleVersion = qApp->applicationVersion();

    if ( ! Settings.contains(QString("%1/%2").arg(UPDATES,"CheckUpdateFrequency"))) {
        checkUpdateFrequency = 0;
        Settings.setValue(QString("%1/%2").arg(UPDATES,"CheckUpdateFrequency"),checkUpdateFrequency);
    } else {
        checkUpdateFrequency = Settings.value(QString("%1/%2").arg(UPDATES,"CheckUpdateFrequency")).toInt();
    }

    if ( ! Settings.contains(QString("%1/%2").arg(UPDATES,"ShowUpdateNotifications"))) {
        QVariant pValue(true);
        showUpdateNotifications = true;
        Settings.setValue(QString("%1/%2").arg(UPDATES,"ShowUpdateNotifications"),pValue);
    } else {
        showUpdateNotifications = Settings.value(QString("%1/%2").arg(UPDATES,"ShowUpdateNotifications")).toBool();
    }

    if ( ! Settings.contains(QString("%1/%2").arg(UPDATES,"EnableDownloader"))) {
        QVariant pValue(true);
        enableDownloader = true;
        Settings.setValue(QString("%1/%2").arg(UPDATES,"EnableDownloader"),pValue);
    } else {
        enableDownloader = Settings.value(QString("%1/%2").arg(UPDATES,"EnableDownloader")).toBool();
    }

    if ( ! Settings.contains(QString("%1/%2").arg(UPDATES,"ShowAllNotifications"))) {
        QVariant pValue(true);
        showAllNotifications = true;
        Settings.setValue(QString("%1/%2").arg(UPDATES,"ShowAllNotifications"),pValue);
    } else {
        showAllNotifications = Settings.value(QString("%1/%2").arg(UPDATES,"ShowAllNotifications")).toBool();
    }
}

void Preferences::lgeoPreferences()
{
    QSettings Settings;
    QString lgeoDirKey("LGEOPath");
    QString lgeoDir;
    if (Settings.contains(QString("%1/%2").arg(POVRAY,lgeoDirKey))){
        lgeoDir = Settings.value(QString("%1/%2").arg(POVRAY,lgeoDirKey)).toString();
        QFileInfo info(lgeoDir);
        if (info.exists()) {
            lgeoPath = lgeoDir;
        } else {
            Settings.remove(QString("%1/%2").arg(POVRAY,lgeoDirKey));
        }
    }
}

void Preferences::renderPreferences()
{
#ifdef Q_OS_WIN
    QFileInfo ldgliteInfo(QString("%1/%2").arg(lpub3dPath).arg("3rdParty/ldglite1.3.1_2g2x_Win/ldglite.exe"));
    QFileInfo l3pInfo(QString("%1/%2").arg(lpub3dPath).arg("3rdParty/l3p1.4WinB/L3P.EXE"));
#else
    //TODO
    QFileInfo ldgliteInfo(QString("%1/%2").arg(lpub3dPath).arg("3rdParty/ldglite1.3.1_2g2x_Mac/ldglite.exe"));
    QFileInfo l3pInfo(QString("%1/%2").arg(lpub3dPath).arg("3rdParty/l3p1.4/L3P.EXE"));
#endif

    QSettings Settings;

    /* Find LDGLite's installation status */

    bool    ldgliteInstalled;
    QString const ldglitePathKey("LDGLite");
    QString ldglitePath;

    if (Settings.contains(QString("%1/%2").arg(SETTINGS,ldglitePathKey))) {
        ldglitePath = Settings.value(QString("%1/%2").arg(SETTINGS,ldglitePathKey)).toString();
        QFileInfo info(ldglitePath);
        if (info.exists()) {
            ldgliteInstalled = true;
            ldgliteExe = ldglitePath;
        } else {
            Settings.remove(QString("%1/%2").arg(SETTINGS,ldglitePathKey));
            ldgliteInstalled = false;
        }
    } else if (ldgliteInfo.exists()) {
        Settings.setValue(QString("%1/%2").arg(SETTINGS,ldglitePathKey),ldgliteInfo.absoluteFilePath());
        ldgliteInstalled = true;
        ldgliteExe = ldgliteInfo.absoluteFilePath();
    } else {
        Settings.remove(QString("%1/%2").arg(SETTINGS,ldglitePathKey));
        ldgliteInstalled = false;
    }

    /* Find LDView's installation status */

    bool    ldviewInstalled;
    QString const ldviewPathKey("LDView");
    QString ldviewPath;

    if (Settings.contains(QString("%1/%2").arg(SETTINGS,ldviewPathKey))) {
        ldviewPath = Settings.value(QString("%1/%2").arg(SETTINGS,ldviewPathKey)).toString();
        QFileInfo info(ldviewPath);
        if (info.exists()) {
            ldviewInstalled = true;
            ldviewExe = ldviewPath;
        } else {
            Settings.remove(QString("%1/%2").arg(SETTINGS,ldviewPathKey));
            ldviewInstalled = false;
        }
    } else {
        ldviewInstalled = false;
    }

    /* Find POV-Ray's installation status */

    bool    povRayInstalled;
    QString const povrayPathKey("POVRayPath");
    QString const l3pPathKey("L3P");
    QString povrayPath, l3pPath;

    if (Settings.contains(QString("%1/%2").arg(POVRAY,povrayPathKey))) {
        povrayPath = Settings.value(QString("%1/%2").arg(POVRAY,povrayPathKey)).toString();
        QFileInfo info(povrayPath);
        if (info.exists()) {
            povRayInstalled = true;
            povrayExe = povrayPath;
        } else {
            Settings.remove(QString("%1/%2").arg(POVRAY,povrayPathKey));
            povRayInstalled = false;
        }
    } else {
        povRayInstalled = false;
    }

    if (Settings.contains(QString("%1/%2").arg(POVRAY,l3pPathKey))) {
        l3pPath = Settings.value(QString("%1/%2").arg(POVRAY,l3pPathKey)).toString();
        QFileInfo info(l3pPath);
        if (info.exists()) {
            povRayInstalled &= true;
            l3pExe = l3pPath;
        } else {
            Settings.remove(QString("%1/%2").arg(POVRAY,l3pPathKey));
            povRayInstalled &= false;
        }
    } else if (l3pInfo.exists()) {
        Settings.setValue(QString("%1/%2").arg(POVRAY,l3pPathKey),l3pInfo.absoluteFilePath());
        povRayInstalled &= true;
        l3pExe = l3pInfo.absoluteFilePath();
    } else {
        Settings.remove(QString("%1/%2").arg(POVRAY,l3pPathKey));
        povRayInstalled &= false;
    }

    /* Find out if we have a valid preferred renderer */

    QString const preferredRendererKey("PreferredRenderer");

    if (Settings.contains(QString("%1/%2").arg(SETTINGS,preferredRendererKey))) {
        preferredRenderer = Settings.value(QString("%1/%2").arg(SETTINGS,preferredRendererKey)).toString();
        if (preferredRenderer == "LDGLite") {
            if ( ! ldgliteInstalled)  {
                preferredRenderer.clear();
                Settings.remove(QString("%1/%2").arg(SETTINGS,preferredRendererKey));
            }
        } else if (preferredRenderer == "LDView") {
            if ( ! ldviewInstalled) {
                preferredRenderer.clear();
                Settings.remove(QString("%1/%2").arg(SETTINGS,preferredRendererKey));
            }
        } else if (preferredRenderer == "POV-Ray") {
            if ( ! povRayInstalled) {
                preferredRenderer.clear();
                Settings.remove(QString("%1/%2").arg(SETTINGS,preferredRendererKey));
            }
        }
    }

    if (preferredRenderer == "") {
        if (ldviewInstalled && ldgliteInstalled) {
            preferredRenderer = povRayInstalled? "POV-Ray" : "LDGLite";
        } else if (povRayInstalled) {
            preferredRenderer = "POV-Ray";
        } else if (ldviewInstalled) {
            preferredRenderer = "LDView";
        } else if (ldgliteInstalled) {
            preferredRenderer = "LDGLite";
        }
    }

    if (preferredRenderer == "") {
        Settings.remove(QString("%1/%2").arg(SETTINGS,preferredRendererKey));
    } else {
        Settings.setValue(QString("%1/%2").arg(SETTINGS,preferredRendererKey),preferredRenderer);
    }

    /* Set use multiple files single call rendering option */
    if (! Settings.contains(QString("%1/%2").arg(SETTINGS,"EnableLDViewSingleCall"))) {
        QVariant eValue(false);
        if (preferredRenderer == "LDView") {
            enableLDViewSingleCall = true;
        } else {
            enableLDViewSingleCall = false;
        }
        Settings.setValue(QString("%1/%2").arg(SETTINGS,"EnableLDViewSingleCall"),eValue);
    } else {
        enableLDViewSingleCall = Settings.value(QString("%1/%2").arg(SETTINGS,"EnableLDViewSingleCall")).toBool();
    }

    if (preferredRenderer == "LDView" && enableLDViewSingleCall) {
        useLDViewSingleCall = true;
    } else {
        useLDViewSingleCall = false;
    }
}

void Preferences::pliPreferences()
{
    QSettings Settings;
    pliFile = Settings.value(QString("%1/%2").arg(SETTINGS,"PliControl")).toString();
    pliSubstitutePartsFile = Settings.value(QString("%1/%2").arg(SETTINGS,"PliSubstitutePartsFile")).toString();

    QFileInfo fileInfo(pliFile);
    if (fileInfo.exists()) {
        //return;
    } else {
        Settings.remove(QString("%1/%2").arg(SETTINGS,"PliControl"));
    }

    QFileInfo pliSubstituteFileInfo(pliSubstitutePartsFile);
    if (pliSubstituteFileInfo.exists()) {
        return;
    } else {
        Settings.remove(QString("%1/%2").arg(SETTINGS,"PliSubstitutePartsFile"));
    }
#ifdef __APPLE__

    pliFile = QString("%1/%2").arg(lpubDataPath,"extras/pli.mpd");
    pliSubstitutePartsFile =  QString("%1/%2").arg(lpubDataPath,"extras/pliSubstituteParts.lst");

#else

    //30-11-14 Incorrect path string
    //pliFile = "/extras/pli.mpd";
    pliFile = QDir::toNativeSeparators(QString("%1/%2").arg(lpubDataPath,"extras/pli.mpd"));
    pliSubstitutePartsFile = QDir::toNativeSeparators(QString("%1/%2").arg(lpubDataPath,"extras/pliSubstituteParts.lst"));

#endif

    QFileInfo popPliFileInfo(pliFile);
    popPliFileInfo.setFile(pliFile);
    if (popPliFileInfo.exists()) {
        Settings.setValue(QString("%1/%2").arg(SETTINGS,"PliControl"),pliFile);
    } else {
        //pliFile = "";
    }

    QFileInfo popPliSubstituteFileInfo(pliSubstitutePartsFile);
    popPliSubstituteFileInfo.setFile(pliSubstitutePartsFile);
    if (popPliSubstituteFileInfo.exists()) {
        Settings.setValue(QString("%1/%2").arg(SETTINGS,"PliSubstitutePartsFile"),pliSubstitutePartsFile);
    } else {
        //pliSubstitutePartsFile = "";
    }
}

void Preferences::unitsPreferences()
{
    QSettings Settings;
    if ( ! Settings.contains(QString("%1/%2").arg(SETTINGS,"Centimeters"))) {
        QVariant uValue(true);
        preferCentimeters = true;
        Settings.setValue(QString("%1/%2").arg(SETTINGS,"Centimeters"),uValue);
    } else {
        preferCentimeters = Settings.value(QString("%1/%2").arg(SETTINGS,"Centimeters")).toBool();
    }
}

void Preferences::annotationPreferences()
{
    QSettings Settings;
    titleAnnotationsFile = Settings.value(QString("%1/%2").arg(SETTINGS,"TitleAnnotationFile")).toString();
    freeformAnnotationsFile = Settings.value(QString("%1/%2").arg(SETTINGS,"FreeFormAnnotationsFile")).toString();

    QFileInfo titleFileInfo(titleAnnotationsFile);
    if (titleFileInfo.exists()) {
        //return;
    } else {
        Settings.remove(QString("%1/%2").arg(SETTINGS,"TitleAnnotationFile"));
    }

    QFileInfo freeformFileInfo(freeformAnnotationsFile);
    if (freeformFileInfo.exists()) {
        return;
    } else {
        Settings.remove(QString("%1/%2").arg(SETTINGS,"FreeFormAnnotationsFile"));
    }
#ifdef __APPLE__

    titleAnnotationsFile    = QString("%1/%2").arg(lpubDataPath,"extras/titleAnnotations.lst");
    freeformAnnotationsFile = QString("%1/%2").arg(lpubDataPath,"extras/freeformAnnotations.lst");

    qDebug() << " Annotation File:  " << titleAnnotationsFile;

#else

    titleAnnotationsFile    = QDir::toNativeSeparators(QString("%1/%2").arg(lpubDataPath,"extras/titleAnnotations.lst"));
    freeformAnnotationsFile = QDir::toNativeSeparators(QString("%1/%2").arg(lpubDataPath,"extras/freeformAnnotations.lst"));

#endif
    QFileInfo popTitleFileInfo(titleAnnotationsFile);
    popTitleFileInfo.setFile(titleAnnotationsFile);
    if (popTitleFileInfo.exists()) {
        Settings.setValue(QString("%1/%2").arg(SETTINGS,"TitleAnnotationFile"),titleAnnotationsFile);
    } else {
        //titleAnnotationsFile = "";
    }

    QFileInfo popFreeFormFileInfo(freeformAnnotationsFile);
    popFreeFormFileInfo.setFile(freeformAnnotationsFile);
    if (popFreeFormFileInfo.exists()) {
        Settings.setValue(QString("%1/%2").arg(SETTINGS,"FreeFormAnnotationsFile"),freeformAnnotationsFile);
    } else {
        //freeformAnnotationsFile = "";
    }
}

void Preferences::fadestepPreferences()
{
    QSettings Settings;
    if (! Settings.contains(QString("%1/%2").arg(SETTINGS,"EnableFadeStep"))) {
        QVariant eValue(false);
        enableFadeStep = false;
        Settings.setValue(QString("%1/%2").arg(SETTINGS,"EnableFadeStep"),eValue);
    } else {
        enableFadeStep = Settings.value(QString("%1/%2").arg(SETTINGS,"EnableFadeStep")).toBool();
    }

    if (! Settings.contains(QString("%1/%2").arg(SETTINGS,"FadeStepColor"))) {
        QVariant cValue("Very_Light_Bluish_Gray");
        fadeStepColor = "Very_Light_Bluish_Gray";
        Settings.setValue(QString("%1/%2").arg(SETTINGS,"FadeStepColor"),cValue);
    } else {
        fadeStepColor = Settings.value(QString("%1/%2").arg(SETTINGS,"FadeStepColor")).toString();
    }

    fadeStepColorPartsFile = Settings.value(QString("%1/%2").arg(SETTINGS,"FadeStepColorPartsFile")).toString();
    QFileInfo fadeStepColorFileInfo(fadeStepColorPartsFile);
    if (!fadeStepColorFileInfo.exists()) {
        Settings.remove(QString("%1/%2").arg(SETTINGS,"FadeStepColorPartsFile"));
    }
#ifdef __APPLE__

    fadeStepColorPartsFile    =  QString("%1/%2").arg(lpubDataPath,"extras/fadeStepColorParts.lst");

#else

    fadeStepColorPartsFile    = QDir::toNativeSeparators(QString("%1/%2").arg(lpubDataPath,"extras/fadeStepColorParts.lst"));

#endif
    QFileInfo popFadeStepColorFileInfo(fadeStepColorPartsFile);
    popFadeStepColorFileInfo.setFile(fadeStepColorPartsFile);
    if (popFadeStepColorFileInfo.exists()) {
        Settings.setValue(QString("%1/%2").arg(SETTINGS,"FadeStepColorPartsFile"),fadeStepColorPartsFile);
    }
}

void Preferences::publishingPreferences()
{
    QSettings Settings;

    if ( ! Settings.contains(QString("%1/%2").arg(DEFAULTS,"DisplayAllAttributes"))) {
        QVariant pValue(false);
        displayAllAttributes = false;
        Settings.setValue(QString("%1/%2").arg(DEFAULTS,"DisplayAllAttributes"),pValue);
    } else {
        displayAllAttributes = Settings.value(QString("%1/%2").arg(DEFAULTS,"DisplayAllAttributes")).toBool();
    }

    if ( ! Settings.contains(QString("%1/%2").arg(DEFAULTS,"generateCoverPages"))) {
        QVariant pValue(false);
        generateCoverPages = false;
        Settings.setValue(QString("%1/%2").arg(DEFAULTS,"generateCoverPages"),pValue);
    } else {
        generateCoverPages = Settings.value(QString("%1/%2").arg(DEFAULTS,"generateCoverPages")).toBool();
    }

    if ( ! Settings.contains(QString("%1/%2").arg(DEFAULTS,"PrintDocumentTOC"))) {
        QVariant pValue(false);
        printDocumentTOC = false;
        Settings.setValue(QString("%1/%2").arg(DEFAULTS,"PrintDocumentTOC"),pValue);
    } else {
        printDocumentTOC = Settings.value(QString("%1/%2").arg(DEFAULTS,"PrintDocumentTOC")).toBool();
    }

    if ( ! Settings.contains(QString("%1/%2").arg(DEFAULTS,"Email"))) {
        QVariant eValue("");
        defaultEmail = "";
        Settings.setValue(QString("%1/%2").arg(DEFAULTS,"Email"),eValue);
    } else {
        defaultEmail = Settings.value(QString("%1/%2").arg(DEFAULTS,"Email")).toString();
    }

    if ( ! Settings.contains(QString("%1/%2").arg(DEFAULTS,"URL"))) {
        QVariant uValue("");
        defaultURL = "";
        Settings.setValue(QString("%1/%2").arg(DEFAULTS,"URL"),uValue);
    } else {
        defaultURL = Settings.value(QString("%1/%2").arg(DEFAULTS,"URL")).toString();
    }

    if ( ! Settings.contains(QString("%1/%2").arg(DEFAULTS,"Author"))) {
        QVariant eValue("");
        defaultAuthor = "";
        Settings.setValue(QString("%1/%2").arg(DEFAULTS,"Author"),eValue);
    } else {
        defaultAuthor = Settings.value(QString("%1/%2").arg(DEFAULTS,"Author")).toString();
    }

    if ( ! Settings.contains(QString("%1/%2").arg(DEFAULTS,"PublishDescription"))) {
        QVariant eValue("");
        publishDescription = "";
        Settings.setValue(QString("%1/%2").arg(DEFAULTS,"PublishDescription"),eValue);
    } else {
        publishDescription = Settings.value(QString("%1/%2").arg(DEFAULTS,"PublishDescription")).toString();
    }

    if (Settings.contains(QString("%1/%2").arg(DEFAULTS,"DocumentLogoFile"))) {
        documentLogoFile = Settings.value(QString("%1/%2").arg(DEFAULTS,"DocumentLogoFile")).toString();
        QFileInfo fileInfo(documentLogoFile);
        if (fileInfo.exists()) {
            return;
        } else {
            Settings.remove(QString("%1/%2").arg(DEFAULTS,"DocumentLogoFile"));
        }
    }
}

void Preferences::viewerPreferences()
{
    QSettings Settings;

    if (Settings.contains(QString("%1/%2").arg(DEFAULTS,"Author")))
        lcSetProfileString(LC_PROFILE_DEFAULT_AUTHOR_NAME, Settings.value(QString("%1/%2").arg(DEFAULTS,"Author")).toString());
    if (Settings.contains(QString("%1/%2").arg(SETTINGS,"ProjectsPath")))
        lcSetProfileString(LC_PROFILE_PROJECTS_PATH, Settings.value(QString("%1/%2").arg(SETTINGS,"ProjectsPath")).toString());
    if (Settings.contains(QString("%1/%2").arg(POVRAY,"POVRayPath")))
        lcSetProfileString(LC_PROFILE_POVRAY_PATH, Settings.value(QString("%1/%2").arg(POVRAY,"POVRayPath")).toString());
    if (Settings.contains(QString("%1/%2").arg(POVRAY,"LGEOPath")))
        lcSetProfileString(LC_PROFILE_POVRAY_LGEO_PATH, Settings.value(QString("%1/%2").arg(POVRAY,"LGEOPath")).toString());
}

bool Preferences::getPreferences()
{
    fadeStepSettingChanged     = false;
    fadeStepColorChanged       = false;

    PreferencesDialog *dialog = new PreferencesDialog();

    QSettings Settings;

    if (dialog->exec() == QDialog::Accepted) {

        if (ldrawPath != dialog->ldrawPath()) {
            ldrawPath = dialog->ldrawPath();
            if (ldrawPath == "") {
                Settings.remove(QString("%1/%2").arg(SETTINGS,"LDrawDir"));
            } else {
                Settings.setValue(QString("%1/%2").arg(SETTINGS,"LDrawDir"),ldrawPath);
            }
        }

        if (pliFile != dialog->pliFile()) {
            pliFile = dialog->pliFile();
            if (pliFile == "") {
                Settings.remove(QString("%1/%2").arg(SETTINGS,"PliControl"));
            } else {
                Settings.setValue(QString("%1/%2").arg(SETTINGS,"PliControl"),pliFile);
            }
        }

        if (l3pExe != dialog->l3pExe()) {
            l3pExe = dialog->l3pExe();
            if (l3pExe == "") {
                Settings.remove(QString("%1/%2").arg(POVRAY,"L3P"));
            } else {
                Settings.setValue(QString("%1/%2").arg(POVRAY,"L3P"),l3pExe);
            }
        }

        if (povrayExe != dialog->povrayExe()) {
            povrayExe = dialog->povrayExe();
            if (povrayExe == "") {
                Settings.remove(QString("%1/%2").arg(POVRAY,"POVRayPath"));
            } else {
                Settings.setValue(QString("%1/%2").arg(POVRAY,"POVRayPath"),povrayExe);
            }
        }

        if (lgeoPath != dialog->lgeoPath()) {
            lgeoPath = dialog->lgeoPath();
            if(lgeoPath == "") {
                Settings.remove(QString("%1/%2").arg(POVRAY,"LGEOPath"));
            } else {
                Settings.setValue(QString("%1/%2").arg(POVRAY,"LGEOPath"),lgeoPath);
            }
        }

        if (ldgliteExe != dialog->ldgliteExe()) {
            ldgliteExe = dialog->ldgliteExe();
            if (ldgliteExe == "") {
                Settings.remove(QString("%1/%2").arg(SETTINGS,"LDGLite"));
            } else {
                Settings.setValue(QString("%1/%2").arg(SETTINGS,"LDGLite"),ldgliteExe);
            }
        }

        if (ldviewExe != dialog->ldviewExe()) {
            ldviewExe = dialog->ldviewExe();
            if (ldviewExe == "") {
                Settings.remove(QString("%1/%2").arg(SETTINGS,"LDView"));
            } else {
                Settings.setValue(QString("%1/%2").arg(SETTINGS,"LDView"),ldviewExe);
            }
        }

        if (preferredRenderer != dialog->preferredRenderer()) {
            preferredRenderer = dialog->preferredRenderer();
            if (preferredRenderer == "") {
                Settings.remove(QString("%1/%2").arg(SETTINGS,"PreferredRenderer"));
            } else {
                Settings.setValue(QString("%1/%2").arg(SETTINGS,"PreferredRenderer"),preferredRenderer);
            }
        }

        if (ldSearchDirs != dialog->searchDirSettings()) {
            if (! dialog->searchDirSettings().isEmpty()){
                ldSearchDirs.clear();
                QString unoffDirPath = QDir::toNativeSeparators(QString("%1/%2").arg(Preferences::ldrawPath).arg("Unofficial"));
                QString modelsDirPath = QDir::toNativeSeparators(QString("%1/%2").arg(Preferences::ldrawPath).arg("MODELS"));
                foreach (QString dirPath, dialog->searchDirSettings()) {
                    QDir searchDir(dirPath);
                    if (!searchDir.exists() || (dirPath.size() > 1 && !dirPath.toLower().contains(unoffDirPath.toLower()) && dirPath.toLower() != modelsDirPath.toLower())){
                        QMessageBox::warning(NULL,
                                             QMessageBox::tr("LPub3D"),
                                             QMessageBox::tr("%1 is not a valid directory.\nAdded directories must be under the Unofficial directory. This path will not be saved.")
                                             .arg(dirPath));
                        continue;
                    } else {
                        ldSearchDirs << dirPath;
                    }
                }
                if (! ldSearchDirs.isEmpty())
                    Settings.setValue(QString("%1/%2").arg(SETTINGS,"LDSearchDirs"),ldSearchDirs);
                else
                    Settings.remove(QString("%1/%2").arg(SETTINGS,"LDSearchDirs"));
            } else {
                Settings.remove(QString("%1/%2").arg(SETTINGS,"LDSearchDirs"));
            }
        }

        if (documentLogoFile != dialog->documentLogoFile()) {
            documentLogoFile = dialog->documentLogoFile();
            if (documentLogoFile == "") {
                Settings.remove(QString("%1/%2").arg(DEFAULTS,"DocumentLogoFile"));
            } else {
                Settings.setValue(QString("%1/%2").arg(DEFAULTS,"DocumentLogoFile"),documentLogoFile);
            }
        }

        if (defaultAuthor != dialog->defaultAuthor()) {
            defaultAuthor = dialog->defaultAuthor();
            if (defaultAuthor == "") {
                Settings.remove(QString("%1/%2").arg(DEFAULTS,"Author"));
            } else {
                Settings.setValue(QString("%1/%2").arg(DEFAULTS,"Author"),defaultAuthor);
            }
        }

        if (defaultURL != dialog->defaultURL()) {
            defaultURL = dialog->defaultURL();
            if (defaultURL == "") {
                Settings.remove(QString("%1/%2").arg(DEFAULTS,"URL"));
            } else {
                Settings.setValue(QString("%1/%2").arg(DEFAULTS,"URL"),defaultURL);
            }
        }

        if (defaultEmail != dialog->defaultEmail()) {
            defaultEmail = dialog->defaultEmail();
            if (defaultEmail == "") {
                Settings.remove(QString("%1/%2").arg(DEFAULTS,"Email"));
            } else {
                Settings.setValue(QString("%1/%2").arg(DEFAULTS,"Email"),defaultEmail);
            }
        }

        if (publishDescription != dialog->publishDescription()) {
            publishDescription = dialog->publishDescription();
            if (publishDescription == "") {
                Settings.remove(QString("%1/%2").arg(DEFAULTS,"PublishDescription"));
            } else {
                Settings.setValue(QString("%1/%2").arg(DEFAULTS,"PublishDescription"),publishDescription);
            }
        }

        if (enableFadeStep != dialog->enableFadeStep())
        {
            fadeStepSettingChanged = true;
            enableFadeStep = dialog->enableFadeStep();
            Settings.setValue(QString("%1/%2").arg(SETTINGS,"EnableFadeStep"),enableFadeStep);
        }

        if (enableFadeStep && (fadeStepColor != dialog->fadeStepColor()))
        {
            fadeStepColorChanged = true;
            fadeStepColor = dialog->fadeStepColor();
            Settings.setValue(QString("%1/%2").arg(SETTINGS,"FadeStepColor"),fadeStepColor);
        }

        if (preferCentimeters != dialog->centimeters())
        {
            preferCentimeters = dialog->centimeters();
            Settings.setValue(QString("%1/%2").arg(SETTINGS,"Centimeters"),preferCentimeters);
            defaultResolutionType(preferCentimeters);
        }

        if (enableLDViewSingleCall != dialog->enableLDViewSingleCall()) {
            enableLDViewSingleCall = dialog->enableLDViewSingleCall();
            Settings.setValue(QString("%1/%2").arg(SETTINGS,"EnableLDViewSingleCall"),enableLDViewSingleCall);
        }

        if (enableDownloader != dialog->enableDownloader()) {
            enableDownloader = dialog->enableDownloader();
            Settings.setValue(QString("%1/%2").arg(UPDATES,"EnableDownloader"),enableDownloader);
        }

        if (showUpdateNotifications != dialog->showUpdateNotifications()) {
            showUpdateNotifications = dialog->showUpdateNotifications();
            Settings.setValue(QString("%1/%2").arg(UPDATES,"ShowUpdateNotifications"),showUpdateNotifications);
        }

        if (showAllNotifications != dialog->showAllNotifications()) {
            showAllNotifications = dialog->showAllNotifications();
            Settings.setValue(QString("%1/%2").arg(UPDATES,"ShowAllNotifications"),showAllNotifications);
        }

        if (checkUpdateFrequency != dialog->checkUpdateFrequency()) {
            checkUpdateFrequency = dialog->checkUpdateFrequency();
            Settings.setValue(QString("%1/%2").arg(UPDATES,"CheckUpdateFrequency"),checkUpdateFrequency);
        }

        if (displayAllAttributes != dialog->displayAllAttributes()) {
            displayAllAttributes = dialog->displayAllAttributes();
            Settings.setValue(QString("%1/%2").arg(DEFAULTS,"DisplayAllAttributes"),displayAllAttributes);
        }

        if (generateCoverPages != dialog->generateCoverPages()) {
            generateCoverPages = dialog->generateCoverPages();
            Settings.setValue(QString("%1/%2").arg(DEFAULTS,"generateCoverPages"),generateCoverPages);
        }

        if (printDocumentTOC != dialog->printDocumentTOC()) {
            printDocumentTOC = dialog->printDocumentTOC();
            Settings.setValue(QString("%1/%2").arg(DEFAULTS,"PrintDocumentTOC"),printDocumentTOC);
        }

        if (preferredRenderer == "LDView" && enableLDViewSingleCall) {
            useLDViewSingleCall = true;
        } else {
            useLDViewSingleCall = false;
        }

        if (moduleVersion != dialog->moduleVersion()){
            moduleVersion = dialog->moduleVersion();
        }

        return true;
    } else {
        return false;
    }
}

void Preferences::getRequireds()
{
    if (preferredRenderer == "" && ! getPreferences()) {
        exit (-1);
    }
}



