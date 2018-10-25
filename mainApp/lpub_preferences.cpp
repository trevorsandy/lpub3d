/****************************************************************************
**
** Copyright (C) 2007-2009 Kevin Clague. All rights reserved.
** Copyright (C) 2015 - 2018 Trevor SANDY. All rights reserved.
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
#include <JlCompress.h>
#include "lpub_preferences.h"
#include "ui_preferences.h"
#include "preferencesdialog.h"
#include "updatecheck.h"
#include "resolution.h"
#include "render.h"
#include "pli.h"
#include "version.h"
#include "name.h"
#include "paths.h"
#include "application.h"
#include "messageboxresizable.h"
//**3D
#include "lc_profile.h"
//**

Preferences preferences;
QDate date = QDate::currentDate();

QString Preferences::lpub3dAppName              = EMPTY_STRING_DEFAULT;
QString Preferences::ldrawPath                  = EMPTY_STRING_DEFAULT;
QString Preferences::altLDConfigPath            = EMPTY_STRING_DEFAULT;
QString Preferences::lpub3dLibFile              = EMPTY_STRING_DEFAULT;
QString Preferences::lgeoPath;
QString Preferences::lpub3dPath                 = DOT_PATH_DEFAULT;
QString Preferences::lpub3dExtrasResourcePath   = DOT_PATH_DEFAULT;
QString Preferences::lpub3dDocsResourcePath     = DOT_PATH_DEFAULT;
QString Preferences::lpub3d3rdPartyConfigDir    = DOT_PATH_DEFAULT;
QString Preferences::lpub3d3rdPartyAppDir       = DOT_PATH_DEFAULT;
QString Preferences::lpubDataPath               = DOT_PATH_DEFAULT;
QString Preferences::lpubConfigPath             = DOT_PATH_DEFAULT;
QString Preferences::lpubExtrasPath             = DOT_PATH_DEFAULT;
QString Preferences::ldgliteExe;
QString Preferences::ldviewExe;
QString Preferences::povrayConf;
QString Preferences::povrayIni;
QString Preferences::ldgliteIni;
QString Preferences::ldviewIni;
QString Preferences::ldviewPOVIni;
QString Preferences::nativePOVIni;
QString Preferences::povrayIniPath;
QString Preferences::povrayIncPath;
QString Preferences::povrayScenePath;
QString Preferences::povrayExe;
QString Preferences::preferredRenderer;
QString Preferences::pliFile;
QString Preferences::titleAnnotationsFile;
QString Preferences::freeformAnnotationsFile;
QString Preferences::fadeStepsColour            = FADE_COLOUR_DEFAULT;
QString Preferences::highlightStepColour        = HIGHLIGHT_COLOUR_DEFAULT;
QString Preferences::pliSubstitutePartsFile;
QString Preferences::ldrawColourPartsFile;
QString Preferences::excludedPartsFile;
//page attributes dynamic
QString Preferences::defaultAuthor;
QString Preferences::defaultURL;
QString Preferences::defaultEmail;
QString Preferences::documentLogoFile;
QString Preferences::publishDescription;
QString Preferences::ldrawiniFile;
QString Preferences::moduleVersion              = qApp->applicationVersion();
QString Preferences::availableVersions;
QString Preferences::ldgliteSearchDirs;
QString Preferences::loggingLevel               = LOGGING_LEVEL_DEFAULT;
QString Preferences::logPath;
QString Preferences::dataLocation;
QString Preferences::povFileGenerator           = RENDERER_LDVIEW;

QStringList Preferences::ldSearchDirs;
QStringList Preferences::ldgliteParms;

//Static page attributes
QString Preferences::disclaimer                 = QString(QObject::trUtf8("LEGO® is a registered trademark of the LEGO Group, \n"
                                                                          "which does not sponsor, endorse, or authorize these \n"
                                                                          "instructions or the model they depict."));
QString Preferences::copyright                  = QString(QObject::trUtf8("Copyright © %1").arg(date.toString("yyyy")));
QString Preferences::plugImage                  = QString(":/resources/LPub64.png");
QString Preferences::plug                       = QString(QObject::trUtf8("Instructions configured and generated using %1 %2 \n Download %1 at %3")
                                                          .arg(QString::fromLatin1(VER_PRODUCTNAME_STR),
                                                               QString::fromLatin1(VER_FILEVERSION_STR).replace("\"",""),
                                                               QString::fromLatin1(VER_COMPANYDOMAIN_STR)));
QString Preferences::displayTheme               = THEME_DEFAULT;

QString Preferences::validLDrawDir              = LDRAWDIR_STR;
QString Preferences::validLDrawPart             = LDRAWLEGOPART_STR;
QString Preferences::validLDrawArchive          = VER_LDRAW_OFFICIAL_ARCHIVE;
QString Preferences::validLDrawCustomArchive    = VER_LPUB3D_UNOFFICIAL_ARCHIVE;
QString Preferences::validLDrawColorParts       = VER_LPUB3D_LEGO_COLOR_PARTS;

QString Preferences::ldrawLibrary               = LEGO_LIBRARY;
QString Preferences::validLDrawPartsLibrary     = LEGO_LIBRARY "® Parts";

bool    Preferences::usingDefaultLibrary        = true;

bool    Preferences::themeAutoRestart           = false;
bool    Preferences::lgeoStlLib                 = false;
bool    Preferences::lpub3dLoaded               = false;
bool    Preferences::enableDocumentLogo         = false;
bool    Preferences::enableLDViewSingleCall     = true;
bool    Preferences::enableLDViewSnaphsotList   = false;
bool    Preferences::displayAllAttributes       = false;
bool    Preferences::generateCoverPages         = false;
bool    Preferences::printDocumentTOC           = false;
bool    Preferences::doNotShowPageProcessDlg    = false;

bool    Preferences::includeLogLevel            = false;
bool    Preferences::includeTimestamp           = false;
bool    Preferences::includeLineNumber          = false;
bool    Preferences::includeFileName            = false;
bool    Preferences::includeFunction            = false;

bool    Preferences::ignoreMixedPageSizesMsg    = false;

bool    Preferences::debugLevel                 = false;
bool    Preferences::traceLevel                 = false;
bool    Preferences::noticeLevel                = false;
bool    Preferences::infoLevel                  = false;
bool    Preferences::statusLevel                = false;
bool    Preferences::errorLevel                 = false;
bool    Preferences::fatalLevel                 = false;

bool    Preferences::includeAllLogAttributes    = false;
bool    Preferences::allLogLevels               = false;

bool    Preferences::logLevel                   = false;
bool    Preferences::logging                    = false;   // logging on/off offLevel (grp box)
bool    Preferences::logLevels                  = false;   // individual logging levels (grp box)

bool    Preferences::applyCALocally             = true;
bool    Preferences::preferCentimeters          = false;   // default is false, to use DPI
bool    Preferences::showAllNotifications       = true;
bool    Preferences::showUpdateNotifications    = true;
bool    Preferences::enableDownloader           = true;
bool    Preferences::ldrawiniFound              = false;
bool    Preferences::portableDistribution       = false;
bool    Preferences::povrayDisplay              = false;
bool    Preferences::isAppImagePayload          = false;
bool    Preferences::modeGUI                    = true;

bool    Preferences::enableFadeSteps            = false;
bool    Preferences::fadeStepsUseColour         = false;
bool    Preferences::enableHighlightStep        = false;
bool    Preferences::enableImageMatting         = false;

bool    Preferences::pageRuler                  = false;
bool    Preferences::pageGuides                 = false;
bool    Preferences::showParseErrors            = true;

#ifdef Q_OS_MAC
bool    Preferences::ldviewMissingLibs          = true;
bool    Preferences::povrayMissingLibs          = true;
#endif

int     Preferences::fadeStepsOpacity           = FADE_OPACITY_DEFAULT;              //Default = 50 percent (half opacity)
int     Preferences::highlightStepLineWidth     = HIGHLIGHT_LINE_WIDTH_DEFAULT;      //Default = 1

int     Preferences::checkUpdateFrequency       = UPDATE_CHECK_FREQUENCY_DEFAULT;    //0=Never,1=Daily,2=Weekly,3=Monthly

int     Preferences::pageHeight                 = PAGE_HEIGHT_DEFAULT;
int     Preferences::pageWidth                  = PAGE_WIDTH_DEFAULT;
int     Preferences::rendererTimeout            = RENDERER_TIMEOUT_DEFAULT;          // measured in seconds
int     Preferences::pageDisplayPause           = PAGE_DISPLAY_PAUSE_DEFAULT;        // measured in seconds
int     Preferences::cameraDistFactorNative     = CAMERA_DISTANCE_FACTOR_NATIVE_DEFAULT;

// Native POV file generation settings
QString Preferences::ldvLights                  = LIGHTS_COMBO_DEFAULT;
QString Preferences::xmlMapPath                 = XML_MAP_PATH_DEFAULT;

Preferences::Preferences()
{
}

bool Preferences::checkLDrawLibrary(const QString &libPath) {

    QString foo = "foo";
    QStringList validLDrawLibs = QStringList() << LEGO_LIBRARY << TENTE_LIBRARY << VEXIQ_LIBRARY;
    QStringList validLDrawParts = QStringList() << LDRAWLEGOPART_STR << LDRAWTENTEPART_STR << LDRAWVEXIQPART_STR;

    for ( int i = 1; i < NumLibs; i++ )
    {
       if (QFileInfo(QString("%1%2").arg(libPath).arg(validLDrawParts[i])).exists()) {
           lpub3dAltLibPreferences(validLDrawLibs[i]);
           return true;
       }
    }
    return false;
}

void Preferences::lpub3dAltLibPreferences(const QString &library)
{
    QSettings Settings;
    if (! library.isEmpty()) {
        ldrawLibrary = library;
        Settings.setValue(QString("%1/%2").arg(SETTINGS,"LDrawLibrary"),ldrawLibrary);
    } else {
        if ( ! Settings.contains(QString("%1/%2").arg(SETTINGS,"LDrawLibrary"))) {
            Settings.setValue(QString("%1/%2").arg(SETTINGS,"LDrawLibrary"),ldrawLibrary);
        } else {
            ldrawLibrary = Settings.value(QString("%1/%2").arg(SETTINGS,"LDrawLibrary")).toString();
        }
    }

    usingDefaultLibrary = ldrawLibrary == LEGO_LIBRARY;

    if (ldrawLibrary == TENTE_LIBRARY) {
        validLDrawDir     = LDRAWTENTEDIR_STR;
        validLDrawPart    = LDRAWTENTEPART_STR;
        validLDrawArchive = VER_LPUB3D_TENTE_ARCHIVE;
        validLDrawColorParts = VER_LPUB3D_TENTE_COLOR_PARTS;
        validLDrawCustomArchive = VER_LPUB3D_TENTE_CUSTOM_ARCHIVE;
        validLDrawPartsLibrary = TENTE_LIBRARY "® Construction Parts";
    }
    else
    if (ldrawLibrary == VEXIQ_LIBRARY) {
        validLDrawDir     = LDRAWVEXIQDIR_STR;
        validLDrawPart    = LDRAWVEXIQPART_STR;
        validLDrawArchive = VER_LPUB3D_VEXIQ_ARCHIVE;
        validLDrawColorParts = VER_LPUB3D_VEXIQ_COLOR_PARTS;
        validLDrawCustomArchive = VER_LPUB3D_VEXIQ_CUSTOM_ARCHIVE;
        validLDrawPartsLibrary = VEXIQ_LIBRARY "® Parts";
    }
}

void Preferences::lpubPreferences()
{
    lpub3dAppName = QCoreApplication::applicationName();
    modeGUI = Application::instance()->modeGUI();
    QDir cwd(QCoreApplication::applicationDirPath());
#ifdef QT_DEBUG_MODE
    qDebug() << "";
    qDebug() << "--------------------------";
#else
    fprintf(stdout, "\n");
    fprintf(stdout, "--------------------------\n");
#endif
#ifdef Q_OS_MAC

    //qDebug() << QString("macOS Binary Directory (%1), AbsPath (%2)").arg(cwd.dirName()).arg(cwd.absolutePath());
#ifdef QT_DEBUG_MODE
    qDebug() << QString(QString("macOS Binary Directory.......(%1)").arg(cwd.dirName()));
#else
    fprintf(stdout, "%s\n", QString(QString("macOS Binary Directory.......(%1)").arg(cwd.dirName())).toLatin1().constData());
#endif
    if (cwd.dirName() == "MacOS") {   // MacOS/         (app bundle executable folder)
        cwd.cdUp();                   // Contents/      (app bundle contents folder)
        cwd.cdUp();                   // LPub3D.app/    (app bundle folder)
        cwd.cdUp();                   // Applications/  (app bundle installation folder)
    }
    //qDebug() << QString("macOS Base Directory (%1), AbsPath (%2)").arg(cwd.dirName()).arg(cwd.absolutePath());
#ifdef QT_DEBUG_MODE
    qDebug() << QString(QString("macOS Base Directory.........(%1)").arg(cwd.dirName()));
#else
    fprintf(stdout, "%s\n", QString(QString("macOS Base Directory.........(%1)").arg(cwd.dirName())).toLatin1().constData());
#endif

    lpub3dExtrasResourcePath = QString("%1/%2.app/Contents/Resources").arg(cwd.absolutePath(),lpub3dAppName);
    lpub3dDocsResourcePath   = lpub3dExtrasResourcePath;

    if (QCoreApplication::applicationName() != QString(VER_PRODUCTNAME_STR))
    {
#ifdef QT_DEBUG_MODE
        qDebug() << QString(QString("macOS Info.plist update......(%1)").arg(lpub3dAppName));
#else
        fprintf(stdout, "%s\n", QString(QString("macOS Info.plist update......(%1)").arg(lpub3dAppName)).toLatin1().constData());
#endif
        QFileInfo plbInfo("/usr/libexec/PlistBuddy");
        QString plistCmd = QString("%1 -c").arg(plbInfo.absoluteFilePath());
        QString infoPlistFile = QString("%1/%2.app/Contents/Info.plist").arg(cwd.absolutePath(),lpub3dAppName);
        if (plbInfo.exists())
        {
            QProcess::execute(QString("%1 \"Set :CFBundleExecutable %2\" \"%3\"").arg(plistCmd,lpub3dAppName,infoPlistFile));
            QProcess::execute(QString("%1 \"Set :CFBundleName %2\" \"%3\"").arg(plistCmd,lpub3dAppName,infoPlistFile));
            QProcess::execute(QString("%1 \"Set :CFBundleDisplayName %2\" \"%3\"").arg(plistCmd,lpub3dAppName,infoPlistFile));
            QProcess::execute(QString("%1 \"Set :CFBundleIdentifier com.trevorsandy.%2\" \"%3\"").arg(plistCmd,lpub3dAppName.toLower(),infoPlistFile));
            QProcess::execute(QString("%1 \"Set :UTExportedTypeDeclarations:0:UTTypeIdentifier com.trevorsandy.%2\" \"%3\"").arg(plistCmd,lpub3dAppName.toLower(),infoPlistFile));
            QProcess::execute(QString("%1 \"Set :UTExportedTypeDeclarations:0:UTTypeIdentifier com.trevorsandy.%2\" \"%3\"").arg(plistCmd,lpub3dAppName.toLower(),infoPlistFile));
            QProcess::execute(QString("%1 \"Set :UTExportedTypeDeclarations:0:UTTypeIdentifier com.trevorsandy.%2\" \"%3\"").arg(plistCmd,lpub3dAppName.toLower(),infoPlistFile));
        } else {
#ifdef QT_DEBUG_MODE
            qDebug() << QString(QString("ERROR - %1 not found, cannot update Info.Plist").arg(plbInfo.absoluteFilePath()));
#else
            fprintf(stdout, "%s\n", QString(QString("ERROR - %1 not found, cannot update Info.Plist").arg(plbInfo.absoluteFilePath())).toLatin1().constData());
#endif
        }
    }

#elif defined Q_OS_LINUX

    // Check if running as AppImage payload
    // Check for the presence of $ORIGIN/../AppRun *or* $ORIGIN/../../AppRun
    // (can be a file or a symlink) (with $ORIGIN being the application itself)
    QDir appRunDir(QString("%1/../../").arg(cwd.absolutePath()));
    QFileInfo appRunInfo(QString("%1/AppRun").arg(appRunDir.absolutePath()));
    if (appRunInfo.exists() && (appRunInfo.isFile() || appRunInfo.isSymLink())) {
        isAppImagePayload = true;
    } else {
        appRunDir.setPath(QString("%1/../").arg(cwd.absolutePath()));
        appRunInfo.setFile(QString("%1/AppRun").arg(appRunDir.absolutePath()));
        if (appRunInfo.exists() && (appRunInfo.isFile() || appRunInfo.isSymLink()))
            isAppImagePayload = true;
    }

    QDir progDir(QString("%1/../share").arg(cwd.absolutePath()));

    // This is a shameless hack until I figure out a better way to get the application name folder
    QStringList fileFilters;
    fileFilters << "lpub3d*";

    QDir contentsDir(progDir.absolutePath() + "/");
    QStringList shareContents = contentsDir.entryList(fileFilters);

    if (shareContents.size() > 0)
    {
        // Because the QCoreApplication::applicationName() is not the same as the LPub3D
        // executable name in an AppImage and the executable name is not the same as the
        // application folder, we set 'lpub3dAppName' to the value of the lpub3d application folder.
        // The application folder value is set with the DIST_TARGET variable in mainApp.pro
        lpub3dAppName = shareContents.at(0);
#ifdef QT_DEBUG_MODE
        qDebug() << QString(QString("LPub3D Application Folder....(%1)").arg(lpub3dAppName));
#else
        fprintf(stdout, "%s\n", QString(QString("LPub3D Application Folder....(%1)").arg(lpub3dAppName)).toLatin1().constData());
#endif
    } else {
#ifdef QT_DEBUG_MODE
        qDebug() << QString(QString("ERROR - Application Folder Not Found."));
#else
        fprintf(stdout, "%s\n", QString(QString("ERROR - Application Folder Not Found.")).toLatin1().constData());
#endif
    }

#ifdef X11_BINARY_BUILD                                               // Standard User Rights Install

    lpub3dDocsResourcePath   = QString("doc");

#else                                                                 // Elevated User Rights Install

    lpub3dDocsResourcePath   = QString("%1/doc/%2").arg(progDir.absolutePath(),lpub3dAppName);
    lpub3dExtrasResourcePath = QString("%1/%2").arg(progDir.absolutePath(),lpub3dAppName);

#endif

#elif defined Q_OS_WIN

    lpub3dDocsResourcePath   = QString("docs");

#endif

    lpub3dPath = cwd.absolutePath();

    // Default configuration path
    QStringList configPathList = QStandardPaths::standardLocations(QStandardPaths::ConfigLocation);
    lpubConfigPath = configPathList.first();

#ifdef Q_OS_WIN //... Windows portable or installed

    portableDistribution = QDir(lpub3dPath + "/extras").exists();

    if (portableDistribution) { // we have a portable distribution

        // Write settings to ini file at <LPub3D Path>\config\LPub3D Software\LPub3D<ver>.ini
        QDir configDir(lpub3dPath + "/config");
        if(!QDir(configDir).exists())
            configDir.mkpath(".");
        QSettings::setDefaultFormat(QSettings::IniFormat);
        QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, configDir.absolutePath());

        bool programFolder = QCoreApplication::applicationDirPath().contains("Program Files") ||
                QCoreApplication::applicationDirPath().contains("Program Files (x86)");

        if (programFolder) {                     // ...installed in Program Folder directory

            if (modeGUI) {
                // Get the application icon as a pixmap
                QPixmap _icon = QPixmap(":/icons/lpub96.png");
                QMessageBoxResizable box;
                box.setWindowIcon(QIcon());
                box.setIconPixmap (_icon);
                box.setTextFormat (Qt::RichText);
                box.setWindowTitle(QMessageBox::tr ("Installation"));
                box.setWindowFlags (Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);

                QString header  = "<b> " + QMessageBox::tr ("Data directory installation folder.") + "</b>";
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

                QStringList dataPathList = QStandardPaths::standardLocations(QStandardPaths::DataLocation);
                lpubDataPath = dataPathList.first();

                if (box.exec() == QMessageBox::Yes) {   // capture user's choice for user data directory

                    QString result = QFileDialog::getExistingDirectory(nullptr,
                                                                       QFileDialog::tr("Select Directory"),
                                                                       lpubDataPath,
                                                                       QFileDialog::ShowDirsOnly |
                                                                       QFileDialog::DontResolveSymlinks);
                    if (! result.isEmpty()) {
                        lpubDataPath = QDir::toNativeSeparators(result);
                    }
                }

            } else {                                          // console mode create automatically

#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
                QStringList dataPathList = QStandardPaths::standardLocations(QStandardPaths::DataLocation);
                lpubDataPath = dataPathList.first();
#endif
            }

        } else {                                 // ...installed outside Program Folder directory

            lpubDataPath = lpub3dPath;
        }


    } else {                    // we have an installed distribution

        QSettings Settings;
        QString const LPub3DDataPathKey("LPub3DDataPath");

        if (Settings.contains(QString("%1/%2").arg(SETTINGS,LPub3DDataPathKey))) {

            lpubDataPath = Settings.value(QString("%1/%2").arg(SETTINGS,LPub3DDataPathKey)).toString();

        } else {

#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
            QStringList dataPathList = QStandardPaths::standardLocations(QStandardPaths::DataLocation);
            lpubDataPath = dataPathList.first();
#endif
            Settings.setValue(QString("%1/%2").arg(SETTINGS, LPub3DDataPathKey), lpubDataPath);
        }
    }

#else  // ...Linux or OSX

#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
    QStringList dataPathList = QStandardPaths::standardLocations(QStandardPaths::DataLocation);

    // Note that for AppImage distros, lpubDataPath folder name will be the name of the AppImage file.
    lpubDataPath = dataPathList.first();
#endif

#endif

    // applications paths:
#ifdef QT_DEBUG_MODE
    qDebug() << QString(QString("LPub3D App Data Path.........(%1)").arg(lpubDataPath));
#else
    fprintf(stdout, "%s\n", QString(QString("LPub3D App Data Path.........(%1)").arg(lpubDataPath)).toLatin1().constData());
#endif
#ifdef Q_OS_MAC
#ifdef QT_DEBUG_MODE
    qDebug() << QString(QString("LPub3D Bundle App Path.......(%1)").arg(lpub3dPath));
#else
    fprintf(stdout, "%s\n", QString(QString("LPub3D Bundle App Path.......(%1)").arg(lpub3dPath)).toLatin1().constData());
#endif
#else
#ifdef QT_DEBUG_MODE
    qDebug() << QString(QString("LPub3D Executable Path.......(%1)").arg(lpub3dPath));
#else
    fprintf(stdout, "%s\n", QString(QString("LPub3D Executable Path.......(%1)").arg(lpub3dPath)).toLatin1().constData());
#endif
#endif


#ifdef Q_OS_WIN
    QString dataDir = "data";
    if (portableDistribution) {
        dataDir = "extras";
#ifdef QT_DEBUG_MODE
        qDebug() << QString("LPub3D Portable Distribution.(Yes)");
#else
        fprintf(stdout, "%s\n", QString("LPub3D Portable Distribution.(Yes)").toLatin1().constData());
#endif
    }
    dataLocation = QString("%1/%2/").arg(lpub3dPath,dataDir);
#else
#ifdef QT_DEBUG_MODE
    qDebug() << QString(QString("LPub3D Extras Resource Path..(%1)").arg(lpub3dExtrasResourcePath));
#else
    fprintf(stdout, "%s\n", QString(QString("LPub3D Extras Resource Path..(%1)").arg(lpub3dExtrasResourcePath)).toLatin1().constData());
#endif
  // On Linux 'dataLocation' folder is /usr/share/lpub3d
  // On macOS 'dataLocation' folder is /Applications/LPub3D.app/Contents/Resources
  dataLocation = QString("%1/").arg(lpub3dExtrasResourcePath);
#if defined Q_OS_LINUX
    QDir rendererDir(QString("%1/../../opt/%2").arg(lpub3dPath).arg(lpub3dAppName));
#ifdef QT_DEBUG_MODE
    qDebug() << QString(QString("LPub3D Renderers Exe Path....(%1/3rdParty)").arg(rendererDir.absolutePath()));
#else
    fprintf(stdout, "%s\n", QString(QString("LPub3D Renderers Exe Path....(%1/3rdParty)").arg(rendererDir.absolutePath())).toLatin1().constData());
#endif
#endif
#endif

    // dataLocation   - the data location at install
    // lpubDataPath   - the application use data location after install
    // lpubExtrasPath - not used

    QDir extrasDir(lpubDataPath + "/extras");
    if(!QDir(extrasDir).exists())
        extrasDir.mkpath(".");

    QFileInfo paramFile;
    paramFile.setFile(QString("%1/%2").arg(extrasDir.absolutePath(), validLDrawColorParts));
    if (!paramFile.exists())
        QFile::copy(dataLocation + paramFile.fileName(), paramFile.absoluteFilePath());
    paramFile.setFile(QString("%1/%2").arg(extrasDir.absolutePath(), VER_FREEFOM_ANNOTATIONS_FILE));
    if (!paramFile.exists())
        QFile::copy(dataLocation + paramFile.fileName(), paramFile.absoluteFilePath());
    paramFile.setFile(QString("%1/%2").arg(extrasDir.absolutePath(), VER_EXTRAS_LDCONFIG_FILE));
    if (!paramFile.exists())
        QFile::copy(dataLocation + paramFile.fileName(), paramFile.absoluteFilePath());
    paramFile.setFile(QString("%1/%2").arg(extrasDir.absolutePath(), VER_PDFPRINT_IMAGE_FILE));
    if (!paramFile.exists())
        QFile::copy(dataLocation + paramFile.fileName(), paramFile.absoluteFilePath());
    paramFile.setFile(QString("%1/%2").arg(extrasDir.absolutePath(), VER_PLI_MPD_FILE));
    if (!paramFile.exists())
        QFile::copy(dataLocation + paramFile.fileName(), paramFile.absoluteFilePath());
    paramFile.setFile(QString("%1/%2").arg(extrasDir.absolutePath(), VER_PLI_SUBSTITUTE_PARTS_FILE));
    if (!paramFile.exists())
        QFile::copy(dataLocation + paramFile.fileName(), paramFile.absoluteFilePath());
    paramFile.setFile(QString("%1/%2").arg(extrasDir.absolutePath(), VER_TITLE_ANNOTATIONS_FILE));
    if (!paramFile.exists())
        QFile::copy(dataLocation + paramFile.fileName(), paramFile.absoluteFilePath());
    paramFile.setFile(QString("%1/%2").arg(extrasDir.absolutePath(), VER_EXCLUDED_PARTS_FILE));
    if (!paramFile.exists())
        QFile::copy(dataLocation + paramFile.fileName(), paramFile.absoluteFilePath());
#ifdef QT_DEBUG_MODE
    qDebug() << QString(QString("LPub3D Loaded LDraw Library..(%1)").arg(validLDrawPartsLibrary));
    qDebug() << "--------------------------";
    qDebug() << "";
#else
    fprintf(stdout, "%s\n", QString(QString("LPub3D Loaded LDraw Library..(%1 Parts)").arg(ldrawLibrary)).toLatin1().constData());
    fprintf(stdout, "--------------------------\n");
    fprintf(stdout, "\n");
    fflush(stdout);
#endif
}

void Preferences::loggingPreferences()
{
    // define log path
    QString lpubDataPath = Preferences::lpubDataPath;
    QDir logDir(lpubDataPath+"/logs");
    if(!QDir(logDir).exists())
        logDir.mkpath(".");
    Preferences::logPath = QDir(logDir).filePath(QString("%1Log.txt").arg(VER_PRODUCTNAME_STR));

    QSettings Settings;
    if ( ! Settings.contains(QString("%1/%2").arg(LOGGING,"IncludeLogLevel"))) {
        QVariant uValue(true);
        includeLogLevel = true;
        Settings.setValue(QString("%1/%2").arg(LOGGING,"IncludeLogLevel"),uValue);
    } else {
        includeLogLevel = Settings.value(QString("%1/%2").arg(LOGGING,"IncludeLogLevel")).toBool();
    }

    if ( ! Settings.contains(QString("%1/%2").arg(LOGGING,"IncludeTimestamp"))) {
        QVariant uValue(false);
        includeTimestamp = false;
        Settings.setValue(QString("%1/%2").arg(LOGGING,"IncludeTimestamp"),uValue);
    } else {
        includeTimestamp = Settings.value(QString("%1/%2").arg(LOGGING,"IncludeTimestamp")).toBool();
    }

    if ( ! Settings.contains(QString("%1/%2").arg(LOGGING,"IncludeFileName"))) {
        QVariant uValue(false);
        includeFileName = false;
        Settings.setValue(QString("%1/%2").arg(LOGGING,"IncludeFileName"),uValue);
    } else {
        includeFileName = Settings.value(QString("%1/%2").arg(LOGGING,"IncludeFileName")).toBool();
    }

    if ( ! Settings.contains(QString("%1/%2").arg(LOGGING,"IncludeLineNumber"))) {
        QVariant uValue(true);
        includeLineNumber = true;
        Settings.setValue(QString("%1/%2").arg(LOGGING,"IncludeLineNumber"),uValue);
    } else {
        includeLineNumber = Settings.value(QString("%1/%2").arg(LOGGING,"IncludeLineNumber")).toBool();
    }

    if ( ! Settings.contains(QString("%1/%2").arg(LOGGING,"IncludeFunction"))) {
        QVariant uValue(true);
        includeFunction = true;
        Settings.setValue(QString("%1/%2").arg(LOGGING,"IncludeFunction"),uValue);
    } else {
        includeFunction = Settings.value(QString("%1/%2").arg(LOGGING,"IncludeFunction")).toBool();
    }

    if ( ! Settings.contains(QString("%1/%2").arg(LOGGING,"IncludeAllLogAttributes"))) {
        QVariant uValue(false);
        includeAllLogAttributes = false;
        Settings.setValue(QString("%1/%2").arg(LOGGING,"IncludeAllLogAttributes"),uValue);
    } else {
        includeAllLogAttributes = Settings.value(QString("%1/%2").arg(LOGGING,"IncludeAllLogAttributes")).toBool();
    }

    if ( ! Settings.contains(QString("%1/%2").arg(LOGGING,"Logging "))) {
        QVariant uValue(true);
        logging = true;
        Settings.setValue(QString("%1/%2").arg(LOGGING,"Logging "),uValue);
    } else {
        logging = Settings.value(QString("%1/%2").arg(LOGGING,"Logging ")).toBool();
    }

    // log levels combo
    if ( ! Settings.contains(QString("%1/%2").arg(LOGGING,"LoggingLevel"))) {
        QVariant uValue(LOGGING_LEVEL_DEFAULT);
        loggingLevel = LOGGING_LEVEL_DEFAULT;
        Settings.setValue(QString("%1/%2").arg(LOGGING,"LoggingLevel"),uValue);
    } else {
        loggingLevel = Settings.value(QString("%1/%2").arg(LOGGING,"LoggingLevel")).toString();
    }
    // log levels group box
    if ( ! Settings.contains(QString("%1/%2").arg(LOGGING,"LogLevel"))) {
        QVariant uValue(true);
        logLevel = true;
        Settings.setValue(QString("%1/%2").arg(LOGGING,"LogLevel"),uValue);
    } else {
        logLevel = Settings.value(QString("%1/%2").arg(LOGGING,"LogLevel")).toBool();
    }

    if ( ! Settings.contains(QString("%1/%2").arg(LOGGING,"LogLevels"))) {
        QVariant uValue(false);
        logLevels = false;
        Settings.setValue(QString("%1/%2").arg(LOGGING,"LogLevels"),uValue);
    } else {
        logLevels = Settings.value(QString("%1/%2").arg(LOGGING,"LogLevels")).toBool();
    }

    if ( ! Settings.contains(QString("%1/%2").arg(LOGGING,"DebugLevel"))) {
        QVariant uValue(false);
        debugLevel = false;
        Settings.setValue(QString("%1/%2").arg(LOGGING,"DebugLevel"),uValue);
    } else {
        debugLevel = Settings.value(QString("%1/%2").arg(LOGGING,"DebugLevel")).toBool();
    }

    if ( ! Settings.contains(QString("%1/%2").arg(LOGGING,"TraceLevel"))) {
        QVariant uValue(false);
        traceLevel = false;
        Settings.setValue(QString("%1/%2").arg(LOGGING,"TraceLevel"),uValue);
    } else {
        traceLevel = Settings.value(QString("%1/%2").arg(LOGGING,"TraceLevel")).toBool();
    }

    if ( ! Settings.contains(QString("%1/%2").arg(LOGGING,"NoticeLevel"))) {
        QVariant uValue(false);
        noticeLevel = false;
        Settings.setValue(QString("%1/%2").arg(LOGGING,"NoticeLevel"),uValue);
    } else {
        noticeLevel = Settings.value(QString("%1/%2").arg(LOGGING,"NoticeLevel")).toBool();
    }

    if ( ! Settings.contains(QString("%1/%2").arg(LOGGING,"InfoLevel"))) {
        QVariant uValue(false);
        infoLevel = false;
        Settings.setValue(QString("%1/%2").arg(LOGGING,"InfoLevel"),uValue);
    } else {
        infoLevel = Settings.value(QString("%1/%2").arg(LOGGING,"InfoLevel")).toBool();
    }

    if ( ! Settings.contains(QString("%1/%2").arg(LOGGING,"StatusLevel"))) {
        QVariant uValue(false);
        statusLevel = false;
        Settings.setValue(QString("%1/%2").arg(LOGGING,"StatusLevel"),uValue);
    } else {
        statusLevel = Settings.value(QString("%1/%2").arg(LOGGING,"StatusLevel")).toBool();
    }

    if ( ! Settings.contains(QString("%1/%2").arg(LOGGING,"ErrorLevel"))) {
        QVariant uValue(false);
        errorLevel = false;
        Settings.setValue(QString("%1/%2").arg(LOGGING,"ErrorLevel"),uValue);
    } else {
        errorLevel = Settings.value(QString("%1/%2").arg(LOGGING,"ErrorLevel")).toBool();
    }

    if ( ! Settings.contains(QString("%1/%2").arg(LOGGING,"FatalLevel"))) {
        QVariant uValue(false);
        fatalLevel = false;
        Settings.setValue(QString("%1/%2").arg(LOGGING,"FatalLevel"),uValue);
    } else {
        fatalLevel = Settings.value(QString("%1/%2").arg(LOGGING,"FatalLevel")).toBool();
    }

    if ( ! Settings.contains(QString("%1/%2").arg(LOGGING,"AllLogLevels"))) {
        QVariant uValue(false);
        allLogLevels = false;
        Settings.setValue(QString("%1/%2").arg(LOGGING,"AllLogLevels"),uValue);
    } else {
        allLogLevels = Settings.value(QString("%1/%2").arg(LOGGING,"AllLogLevels")).toBool();
    }

}

void Preferences::lpub3dLibPreferences(bool force)
{
    emit Application::instance()->splashMsgSig("10% - Locate LDraw archive libraries...");

#ifdef Q_OS_WIN
    QString filter(QFileDialog::tr("Archive (*.zip *.bin);;All Files (*.*)"));
#else
    QString filter(QFileDialog::tr("All Files (*.*)"));
#endif

    QSettings Settings;

    QString latestToValidate = QString("%1/%2/%3").arg(lpubDataPath, "libraries", validLDrawArchive);

    QFileInfo validFile;
    QString const LPub3DLibKey("PartsLibrary");

    // Start by checking the registry value, if not exist set the user data path
    if (Settings.contains(QString("%1/%2").arg(SETTINGS,LPub3DLibKey))) {
        lpub3dLibFile = Settings.value(QString("%1/%2").arg(SETTINGS,LPub3DLibKey)).toString();
    } else {
        lpub3dLibFile = latestToValidate;
        Settings.setValue(QString("%1/%2").arg(SETTINGS, LPub3DLibKey), lpub3dLibFile);
    }
    if (lpub3dLibFile != latestToValidate) {
        lpub3dLibFile = latestToValidate;
        Settings.setValue(QString("%1/%2").arg(SETTINGS, LPub3DLibKey), lpub3dLibFile);
    }

    // If we have a valid library archive file, update the 3DViewer parts_library variable, else clear the registry value
    if (! lpub3dLibFile.isEmpty() && ! force) {
        validFile.setFile(lpub3dLibFile);

        if (validFile.exists()) {
            lcSetProfileString(LC_PROFILE_PARTS_LIBRARY, Settings.value(QString("%1/%2").arg(SETTINGS,LPub3DLibKey)).toString());
            return;
        }
        else {
            lpub3dLibFile.clear();
            Settings.remove(QString("%1/%2").arg(SETTINGS, LPub3DLibKey));
        }
    }

    // Request to open a dialogue to select library path
    if (! lpub3dLibFile.isEmpty() && force){

        QString result = QFileDialog::getOpenFileName(nullptr,
                                                      QFileDialog::tr("Select LDraw Library Archive"),
                                                      lpub3dLibFile,
                                                      filter);
        if (! result.isEmpty())
            lpub3dLibFile = QDir::toNativeSeparators(result);
    }

    // Archive library exist, so set registry value
    if (! lpub3dLibFile.isEmpty()) {

        Settings.setValue(QString("%1/%2").arg(SETTINGS, LPub3DLibKey), lpub3dLibFile);

    } else {

        // lets go look for the archive files...

        validFile.setFile(dataLocation + validLDrawArchive);
        bool archivesExist = validFile.exists();

        // DEBUG DEBUG DEBUG
        //        QMessageBox::information(nullptr,
        //                                 QMessageBox::tr("LPub3D"),
        //                                 QMessageBox::tr("lpub3dPath: (%1)\n\n"
        //                                                 "lpub3dExtrasResourcePath [dataLocation]: (%2)\n\n"
        //                                                 "ArchivesExist: (%3)\n\n"
        //                                                 "Full Archives Path: (%4)")
        //                                                 .arg(lpub3dPath)
        //                                                 .arg(lpub3dExtrasResourcePath)
        //                                                 .arg(archivesExist ? "Yes" : "No")
        //                                                 .arg(validFile.absoluteFilePath()));

        if (archivesExist) {  // This condition should always fire as archive files are deposited at installation

            emit Application::instance()->splashMsgSig("10% - Copying archive libraries...");

            QDir libraryDir(QString("%1/%2").arg(lpubDataPath).arg(Paths::libraryDir));
            if (!QDir(libraryDir).exists())
                libraryDir.mkpath(".");

            validFile.setFile(QString("%1/%2").arg(libraryDir.absolutePath(), validLDrawArchive));
            if (!validFile.exists())
                QFile::copy(dataLocation + validFile.fileName(), validFile.absoluteFilePath());

            lpub3dLibFile = validFile.absoluteFilePath();
            Settings.setValue(QString("%1/%2").arg(SETTINGS, LPub3DLibKey), lpub3dLibFile);

            if (usingDefaultLibrary)
                validFile.setFile(QString("%1/%2").arg(libraryDir.absolutePath(), VER_LPUB3D_UNOFFICIAL_ARCHIVE));
            if (usingDefaultLibrary && !validFile.exists())
                QFile::copy(dataLocation + validFile.fileName(), validFile.absoluteFilePath());

        } else if (modeGUI) { // This condition should never fire - left over old code that offers to select or download the archive libraries (in case the user removes them)

#ifdef Q_OS_MAC
            if (! lpub3dLoaded && modeGUI && Application::instance()->splash->isVisible())
                Application::instance()->splash->hide();
#endif
            QPixmap _icon = QPixmap(":/icons/lpub96.png");
            QMessageBoxResizable box;
            box.setWindowIcon(QIcon());
            box.setIconPixmap (_icon);
            box.setTextFormat (Qt::RichText);
            box.setWindowTitle(QMessageBox::tr ("Library Selection"));
            box.setWindowFlags (Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);

            QAbstractButton* downloadButton = box.addButton(QMessageBox::tr("Download"),QMessageBox::YesRole);
            QAbstractButton* selectButton   = box.addButton(QMessageBox::tr("Select"),QMessageBox::YesRole);

            QString header = "<b> " + QMessageBox::tr ("No LDraw library archive defined!") + "</b>";
            QString body = QMessageBox::tr ("Note: The LDraw library archives are not provided and <u>must be downloaded</u> - or selected.\n"
                                            "Would you like to download or select the library archives?");
            QString detail = QMessageBox::tr ("You must select or create your LDraw library archive files.\n"
                                              "The location of your official archive file (%1) should "
                                              "also have the unofficial archive file (%2).\n"
                                              "LDraw library archive files can be copied, downloaded or selected to your '%1/%2/' folder now.")
                                              .arg(validLDrawArchive)
                                              .arg(usingDefaultLibrary ? VER_LPUB3D_UNOFFICIAL_ARCHIVE : "")
                                              .arg(lpubDataPath, "libraries");
            box.setText (header);
            box.setInformativeText (body);
            box.setDetailedText(detail);
            box.setStandardButtons (QMessageBox::Cancel);
            box.exec();

            if (box.clickedButton()==selectButton) {
                emit Application::instance()->splashMsgSig("10% - Selecting archive libraries...");

                QFileDialog dlgGetFileName(nullptr,QFileDialog::tr("Select LDraw Library Archive "));
                dlgGetFileName.setDirectory(lpubDataPath);
                dlgGetFileName.setNameFilter(filter);

                if (dlgGetFileName.exec() == QFileDialog::Accepted) {
                    lpub3dLibFile = dlgGetFileName.selectedFiles().at(0);
                    Settings.setValue(QString("%1/%2").arg(SETTINGS, LPub3DLibKey), lpub3dLibFile);

                } else {

                    Settings.remove(QString("%1/%2").arg(SETTINGS, LPub3DLibKey));
                    body = QMessageBox::tr ("<u>Selection cancelled</u>.\n"
                                            "The application will terminate.");
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

#ifdef Q_OS_MAC
                    if (! lpub3dLoaded && modeGUI && Application::instance()->splash->isHidden())
                        Application::instance()->splash->show();
#endif
                    emit Application::instance()->splashMsgSig("10% - Downloading archive libraries...");

                    QDir libraryDir(QString("%1/%2").arg(lpubDataPath, "libraries"));
                    if (!QDir(libraryDir).exists())
                        libraryDir.mkpath(".");

                    UpdateCheck *libraryDownload;
                    QEventLoop  *wait = new QEventLoop();
                    libraryDownload  = new UpdateCheck(nullptr, (void*)LDrawOfficialLibraryDirectDownload);
                    wait->connect(libraryDownload, SIGNAL(downloadFinished(QString,QString)), wait, SLOT(quit()));
                    wait->connect(libraryDownload, SIGNAL(cancel()),                          wait, SLOT(quit()));
                    libraryDownload->requestDownload(libraryDownload->getDEFS_URL(), libraryDir.absolutePath());
                    wait->exec();
                    if (libraryDownload->getCancel()) {
                        body = QMessageBox::tr ("<u>Official LDraw archive download cancelled</u>.\n"
                                                "The application will terminate.");
                        box.removeButton(selectButton);
                        box.removeButton(downloadButton);
                        box.setStandardButtons (QMessageBox::Close);
                        box.setText(header);
                        box.setInformativeText(body);
                        box.exec();

                        exit(-1);
                    }
                    libraryDownload  = new UpdateCheck(nullptr, (void*)LDrawUnofficialLibraryDirectDownload);
                    wait->connect(libraryDownload, SIGNAL(downloadFinished(QString,QString)), wait, SLOT(quit()));
                    wait->connect(libraryDownload, SIGNAL(cancel()),                          wait, SLOT(quit()));
                    libraryDownload->requestDownload(libraryDownload->getDEFS_URL(), libraryDir.absolutePath());
                    wait->exec();
                    if (libraryDownload->getCancel()) {
                        body = QMessageBox::tr ("<u>Unofficial LDraw archive download cancelled</u>.\n"
                                                "The application will continue; however, your LDraw archives\n"
                                                "will not contain unofficial LDraw parts or primitives.");
                        box.removeButton(selectButton);
                        box.removeButton(downloadButton);
                        box.setStandardButtons (QMessageBox::Close);
                        box.setText(header);
                        box.setInformativeText(body);
                        box.exec();
                    }

                    // validate downloaded files
                    lpub3dLibFile = QDir::toNativeSeparators(QString("%1/%2").arg(libraryDir.absolutePath(), validLDrawArchive));
                    validFile.setFile(lpub3dLibFile);
                    if (!validFile.exists()) {

                        Settings.remove(QString("%1/%2").arg(SETTINGS, LPub3DLibKey));
                        if (usingDefaultLibrary)
                            validFile.setFile(QDir::toNativeSeparators(QString("%1/%2").arg(libraryDir.absolutePath(), VER_LDRAW_UNOFFICIAL_ARCHIVE)));
                        if (usingDefaultLibrary && !validFile.exists()) {
                            body = QMessageBox::tr ("Required archive files\n%1\n%2\ndoes not exist.").arg(lpub3dLibFile, validFile.absoluteFilePath());
                        } else {
                            body = QMessageBox::tr ("Required archive file %1 does not exist.").arg(lpub3dLibFile);
                        }
                        body.append("\nThe application will terminate.");

                        box.setStandardButtons (QMessageBox::Close);
                        box.setText(header);
                        box.setInformativeText(body);

                        box.exec();
                        lpub3dLibFile.clear();

                        exit(-1);
                    }

                    // extract downloaded files
                    extractLDrawLib();

                } else {

                    exit(-1);

                }
#ifdef Q_OS_MAC
            if (! lpub3dLoaded && modeGUI && Application::instance()->splash->isHidden())
                Application::instance()->splash->show();
#endif
        } else {              // If we get here, inform the user that required archive libraries do not exist (performing build check or they were probably removed)

            QString officialArchive = validFile.absoluteFilePath();
            if (usingDefaultLibrary)
                validFile.setFile(dataLocation + VER_LPUB3D_UNOFFICIAL_ARCHIVE);
            if (usingDefaultLibrary && !validFile.exists()) {
                fprintf(stderr, "Required archive files\n%s\n%s\ndoes not exist.\n", officialArchive.toLatin1().constData(), validFile.absoluteFilePath().toLatin1().constData());
            } else {
                fprintf(stderr, "Required archive file %s does not exist.\n", lpub3dLibFile.toLatin1().constData());
            }
            fprintf(stderr, "Launching %s in GUI mode offers a dialogue to download, and extract archive libraries.\n",lpub3dAppName.toLatin1().constData());
        }
    }

    if (Settings.contains(QString("%1/%2").arg(SETTINGS,LPub3DLibKey)))
        lcSetProfileString(LC_PROFILE_PARTS_LIBRARY, Settings.value(QString("%1/%2").arg(SETTINGS,LPub3DLibKey)).toString());
}

void Preferences::ldrawPreferences(bool force)
{
    emit Application::instance()->splashMsgSig("10% - Locate LDraw directory...");

    QSettings Settings;

    QString latestToValidate = QDir::toNativeSeparators(QFileInfo(ldrawPath).dir().absolutePath()+"/"+validLDrawDir);
    QString const ldrawKey("LDrawDir");    
                                                    // Registry check
    if (Settings.contains(QString("%1/%2").arg(SETTINGS,ldrawKey))) {
        ldrawPath = QDir::toNativeSeparators(Settings.value(QString("%1/%2").arg(SETTINGS,ldrawKey)).toString());
    }
                                                    // Registry folder against latest...
    if (ldrawPath != latestToValidate){
       ldrawPath = latestToValidate;
    }

    QDir ldrawDir(ldrawPath);
    if (! QFileInfo(ldrawDir.absolutePath()+validLDrawPart).exists() || force) {      // first check

        QString returnMessage = QString();

        if (! force ) {
            ldrawPath.clear();
            Settings.remove(QString("%1/%2").arg(SETTINGS,ldrawKey));

            char* EnvPath = getenv("LDRAWDIR");         // check environment variable LDRAWDIR

            if (EnvPath && EnvPath[0])
                ldrawPath = QString(EnvPath);

            QDir ldrawDir(ldrawPath);
        }

        if (! QFileInfo(ldrawDir.absolutePath()+validLDrawPart).exists() || force) {  // second check
            if (! force) {                                                            // third check - force
                ldrawPath.clear();

                /* Path Checks */
                QString homePath,userLocalDataPath,userDocumentsPath;
                QStringList dataPathList;
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
                dataPathList = QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
                homePath = dataPathList.first();
#if (QT_VERSION >= QT_VERSION_CHECK(5, 4, 0))
                dataPathList = QStandardPaths::standardLocations(QStandardPaths::AppLocalDataLocation);
#else
                dataPathList = QStandardPaths::standardLocations(QStandardPaths::DataLocation);
#endif
                userLocalDataPath = dataPathList.first();
                dataPathList = QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation);
                userDocumentsPath = dataPathList.first();
#endif
                ldrawPath = QDir::toNativeSeparators(QString("%1/%2").arg(homePath).arg(validLDrawDir));

                if ( ! QDir(ldrawPath).exists()) {     // check user documents path

                    ldrawPath = QDir::toNativeSeparators(QString("%1/%2").arg(userDocumentsPath).arg(validLDrawDir));

                    if ( ! QDir(ldrawPath).exists()) { // check system data path

                        dataPathList = QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation);

#if defined Q_OS_WIN || defined Q_OS_MAC
                        ldrawPath = QDir::toNativeSeparators(QString("%1/%2").arg(dataPathList.at(1)).arg(validLDrawDir)); /* C:/ProgramData/LDraw, /Library/Application Support/LDraw */
#else
                        ldrawPath = QDir::toNativeSeparators(QString("%1/%2").arg(dataPathList.at(2)).arg(validLDrawDir)); /* /usr/share/LDRAW" */
#endif

                        if ( ! QDir(ldrawPath).exists()) {     // check user data path

                            ldrawPath = QDir::toNativeSeparators(QString("%1/%2").arg(userLocalDataPath).arg(validLDrawDir));


                            if ( ! QDir(ldrawPath).exists()) { // manual prompt for LDraw Library location

                                QString searchDetail;
                                searchDetail = QMessageBox::tr ("%1\n %2\n %3\n %4")
                                        .arg(QDir::toNativeSeparators(QString("%1/%2").arg(homePath).arg(validLDrawDir)))
                                        .arg(QDir::toNativeSeparators(QString("%1/%2").arg(userDocumentsPath).arg(validLDrawDir)))
#if defined Q_OS_WIN || defined Q_OS_MAC
                                        .arg(QDir::toNativeSeparators(QString("%1/%2").arg(dataPathList.at(1)).arg(validLDrawDir)))
#else
                                        .arg(QDir::toNativeSeparators(QString("%1/%2").arg(dataPathList.at(2)).arg(validLDrawDir)))
#endif
                                        .arg(QDir::toNativeSeparators(QString("%1/%2").arg(userLocalDataPath).arg(validLDrawDir)));


                                if (modeGUI) {
#ifdef Q_OS_MAC
                                    if (! lpub3dLoaded && modeGUI && Application::instance()->splash->isVisible())
                                        Application::instance()->splash->hide();
#endif
                                    QPixmap _icon = QPixmap(":/icons/lpub96.png");
                                    QMessageBoxResizable box;
                                    box.setWindowIcon(QIcon());
                                    box.setIconPixmap (_icon);
                                    box.setTextFormat (Qt::RichText);
                                    box.setWindowTitle(QMessageBox::tr ("LDraw Library"));
                                    box.setWindowFlags (Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);

                                    QString header;
                                    QString body;
                                    QString detail;

                                    if (isAppImagePayload ) { // For AppImage, automatically install LDraw library if not found

                                        if (extractLDrawLib()) {

                                            header = "<b> " + ldrawLibrary + QMessageBox::tr (" LDraw library installed") + "</b>";
                                            body = QMessageBox::tr ("LDraw library was not found. The bundled library archives were installed at\n"
                                                                    "%1").arg(ldrawPath);
                                            detail = QMessageBox::tr ("The following locations were searched for the LDraw library.\n%1\n"
                                                                      "You can change the library path in the Preferences dialogue.").arg(searchDetail);
                                            box.setText (header);
                                            box.setInformativeText (body);
                                            box.setDetailedText(detail);
                                            box.setStandardButtons (QMessageBox::Close);
                                            box.exec();

                                        }

                                    } else {                  // For all except AppImage, prompt user to select or install LDraw library if not found

                                        QAbstractButton* extractButton = box.addButton(QMessageBox::tr("Extract Archive"),QMessageBox::YesRole);
                                        QAbstractButton* selectButton  = box.addButton(QMessageBox::tr("Select Folder"),QMessageBox::YesRole);

                                        header = "<b> " + QMessageBox::tr ("%1 LDraw library folder not found!").arg(ldrawLibrary) + "</b>";
                                        body = QMessageBox::tr ("You may select your LDraw folder or extract it from the bundled %1 %2.\n"
                                                                "Would you like to extract the library or select the LDraw folder?")
                                                                 .arg(ldrawLibrary).arg(usingDefaultLibrary ? "archives" : "archive");
                                        detail = QMessageBox::tr ("The following locations were searched for the LDraw library:\n%1\n"
                                                                  "You must select an LDraw library folder or extract the library.\n"
                                                                  "It is possible to create your library folder from the %2 file (%3) "
                                                                  "and the %4 parts archive file %5. The extracted library folder will "
                                                                  "be located at '%6'").arg(searchDetail)
                                                                   .arg(usingDefaultLibrary ? "official LDraw LEGO archive" : "LDraw " + ldrawLibrary + " archive.")
                                                                   .arg(validLDrawArchive).arg(usingDefaultLibrary ? "unofficial" : "custom")
                                                                   .arg(validLDrawCustomArchive).arg(ldrawPath);
                                        box.setText (header);
                                        box.setInformativeText (body);
                                        box.setDetailedText(detail);
                                        box.setStandardButtons (QMessageBox::Cancel);
                                        box.exec();

                                        if (box.clickedButton()==selectButton) {
                                            emit Application::instance()->splashMsgSig("10% - Selecting LDraw folder...");

                                            ldrawPath = QFileDialog::getExistingDirectory(nullptr,
                                                                                          QFileDialog::tr("Select LDraw library folder"),
                                                                                          "/",
                                                                                          QFileDialog::ShowDirsOnly |
                                                                                          QFileDialog::DontResolveSymlinks);

                                            if (! ldrawPath.isEmpty()) {
                                                if (checkLDrawLibrary(ldrawPath)) {
                                                    Settings.setValue(QString("%1/%2").arg(SETTINGS,ldrawKey),ldrawPath);
                                                } else {
                                                    ldrawPath.clear();
                                                    returnMessage = QMessageBox::tr ("The specified path is not a valid LPub3D-supported LDraw Library.\n"
                                                                                     "%1").arg(ldrawPath);
                                                }
                                            }

                                        } else {
                                            if (box.clickedButton()==extractButton) {
                                                extractLDrawLib();
                                            } else {
                                                ldrawPath.clear();
                                            }
                                        }
                                    }

                                } else {                  // Console mode so extract and install LDraw Library automatically if not exist in searched paths.
                                    QString message = QString("LDraw library was not found. The following locations were searched for the LDraw library:\n%1.\n").arg(searchDetail);
                                    fprintf(stdout,"%s\n",message.toLatin1().constData());
                                    if (extractLDrawLib()) {
                                        message = QString("The bundled library archives were installed at:\n%1\n"
                                                          "You can change the library path in the Preferences dialogue.\n").arg(ldrawPath);
                                        fprintf(stdout,"%s\n",message.toLatin1().constData());
                                    }
                                    fflush(stdout);
                                }
                            }
                        }
                    }
                }
            }

            if (! ldrawPath.isEmpty() && force && modeGUI) {
#ifdef Q_OS_MAC
                if (! lpub3dLoaded && modeGUI && Application::instance()->splash->isVisible())
                    Application::instance()->splash->hide();
#endif
                QString result = QFileDialog::getExistingDirectory(nullptr,
                                                                   QFileDialog::tr("Select LDraw Directory"),
                                                                   ldrawPath,
                                                                   QFileDialog::ShowDirsOnly |
                                                                   QFileDialog::DontResolveSymlinks);
                if (! result.isEmpty()) {

                    if (checkLDrawLibrary(result)) {
                        ldrawPath = QDir::toNativeSeparators(result);
                    } else {
                        returnMessage = QMessageBox::tr ("The specified path is not a valid LPub3D-supported LDraw Library.\n"
                                                          "%1").arg(ldrawPath);
                    }
                }
            }

            if (! ldrawPath.isEmpty()) {

                Settings.setValue(QString("%1/%2").arg(SETTINGS,ldrawKey),ldrawPath);

            } else {

                if (modeGUI) {
#ifdef Q_OS_MAC
                    if (! lpub3dLoaded && modeGUI && Application::instance()->splash->isVisible())
                        Application::instance()->splash->hide();
#endif
                    QPixmap _icon = QPixmap(":/icons/lpub96.png");
                    QMessageBoxResizable box;
                    box.setWindowIcon(QIcon());
                    box.setIconPixmap (_icon);
                    box.setTextFormat (Qt::RichText);
                    box.setWindowTitle(QMessageBox::tr ("LDraw Directory"));
                    box.setWindowFlags (Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);

                    if (returnMessage.isEmpty())
                        returnMessage = "You must enter your LDraw library path.";
                    QString header = "<b> " + QMessageBox::tr ("No LDraw library defined!") + "</b>";
                    QString body = QMessageBox::tr ("%1\nDo you wish to continue?")
                                                    .arg(returnMessage);
                    QString detail = QMessageBox::tr ("The LDraw library is required by the LPub3D renderer(s). "
                                                      "If an LDraw library is not defined, the renderer will not "
                                                      "be able to find the parts and primitives needed to render images.");

                    box.setText (header);
                    box.setInformativeText (body);
                    box.setDetailedText(detail);
                    box.setStandardButtons (QMessageBox::Yes | QMessageBox::No);

                    if (box.exec() != QMessageBox::Yes) {

                        exit(-1);

                    } else {
                        ldrawPreferences(false);
                    }

                } else {
                    fprintf(stderr, "No LDraw library defined! The application will terminate.\n");
                    exit(-1);
                }
            }

        } else {                                 // second check successful - return
            Settings.setValue(QString("%1/%2").arg(SETTINGS,ldrawKey),ldrawPath);
            QString PartsLibrary = lcGetProfileString(LC_PROFILE_PARTS_LIBRARY);
            if (PartsLibrary.isEmpty())
                lcSetProfileString(LC_PROFILE_PARTS_LIBRARY, Settings.value(QString("%1/%2").arg(SETTINGS,ldrawKey)).toString());
        }
    }                                             // first check successful - return


    // Check for and set alternate LDConfig file if specified
    QString const altLDConfigPathKey("AltLDConfigPath");

    if (Settings.contains(QString("%1/%2").arg(SETTINGS,altLDConfigPathKey))) {
        altLDConfigPath = Settings.value(QString("%1/%2").arg(SETTINGS,altLDConfigPathKey)).toString();
    }

    if (! altLDConfigPath.isEmpty()) {

        QFileInfo altLDConfigFile(altLDConfigPath);

        if (altLDConfigFile.exists()) {
            return;
        } else {

            if (modeGUI) {
#ifdef Q_OS_MAC
                if (! lpub3dLoaded && modeGUI && Application::instance()->splash->isVisible())
                    Application::instance()->splash->hide();
#endif
                QPixmap _icon = QPixmap(":/icons/lpub96.png");
                QMessageBoxResizable box;
                box.setWindowIcon(QIcon());
                box.setIconPixmap (_icon);
                box.setTextFormat (Qt::RichText);
                box.setWindowTitle(QMessageBox::tr ("Alternate LDraw LDConfig"));
                box.setWindowFlags (Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);

                QAbstractButton* selectButton  = box.addButton(QMessageBox::tr("Select File"),QMessageBox::YesRole);

                QString header = "<b> " + QMessageBox::tr ("Alternate LDraw LDConfig file not detected!") + "</b>";
                QString body = QMessageBox::tr ("The alternate LDraw LDConfig file:\n"
                                                "%1 does not exist.\n"
                                                "Would you like to select the alternate LDConfig file?").arg(altLDConfigFile.absoluteFilePath());;
                QString detail = QMessageBox::tr ("The alternate LDraw LDConfig file is optional.\n"
                                                  "You can use 'Select File' to select or 'Cancel' to\n"
                                                  "abandon the alternate LDraw LDConfig file.\n");
                box.setText (header);
                box.setInformativeText (body);
                box.setDetailedText(detail);
                box.setStandardButtons (QMessageBox::Cancel);
                box.exec();

                if (box.clickedButton()==selectButton) {
#ifdef Q_OS_WIN
                    QString filter(QMessageBox::tr("LDraw (*.ldr);;All Files (*.*)"));
#else
                    QString filter(QMessageBox::tr("All Files (*.*)"));
#endif
                    altLDConfigPath = QFileDialog::getOpenFileName(nullptr,
                                                                   QFileDialog::tr("Select LDRaw LDConfig file"),
                                                                   ldrawPath,
                                                                   filter);

                    if (! altLDConfigPath.isEmpty()) {

                        Settings.setValue(QString("%1/%2").arg(SETTINGS,"AltLDConfigPath"),altLDConfigPath);

                    }

                } else {
                    altLDConfigPath.clear();
                }

            } else {
                fprintf(stdout, "The alternate LDraw LDConfig file %s does not exist. Setting ignored.\n", altLDConfigFile.absoluteFilePath().toLatin1().constData());
                fflush(stdout);
            }
        }
    }

#ifdef Q_OS_MAC
    if (! lpub3dLoaded && modeGUI && Application::instance()->splash->isHidden())
        Application::instance()->splash->show();
#endif
}

void Preferences::lpub3dUpdatePreferences(){

    emit Application::instance()->splashMsgSig("15% - Selecting update settings...");

    QSettings Settings;

    moduleVersion = qApp->applicationVersion();

    if ( ! Settings.contains(QString("%1/%2").arg(UPDATES,"CheckUpdateFrequency"))) {
        checkUpdateFrequency = UPDATE_CHECK_FREQUENCY_DEFAULT;
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
    logInfo() << QString("LGEO library status...");
    QSettings Settings;
    QString const lgeoDirKey("LGEOPath");
    QString lgeoDir = "";
    if (Settings.contains(QString("%1/%2").arg(POVRAY,lgeoDirKey))){
        lgeoDir = Settings.value(QString("%1/%2").arg(POVRAY,lgeoDirKey)).toString();
    } else { // check in ldraw directory path for lgeo
        lgeoDir = ldrawPath + "/lgeo";
    }
    QDir lgeoDirInfo(lgeoDir);
    if (lgeoDirInfo.exists()) {
        lgeoPath = QDir::toNativeSeparators(lgeoDir);
        logInfo() << QString("LGEO library path  : %1").arg(lgeoDirInfo.absolutePath());
        /* Durat's lgeo stl library Check */
        QDir lgeoStlLibInfo(lgeoPath + "/stl");
        lgeoStlLib = lgeoStlLibInfo.exists();
        if (lgeoStlLib)
            logInfo() << QString("Durat's Stl library: %1").arg(lgeoStlLibInfo.absolutePath());
    } else {
        Settings.remove(QString("%1/%2").arg(POVRAY,lgeoDirKey));
        logInfo() << QString("LGEO library path  : Not found");
        lgeoPath.clear();
    }
}

void Preferences::rendererPreferences(UpdateFlag updateFlag)
{
    QSettings Settings;

    /* Set 3rdParty application locations */

    logInfo() << QString("Image renderers...");
#ifdef Q_OS_WIN
    lpub3d3rdPartyAppDir = QString("%1/3rdParty").arg(lpub3dPath);

    QFileInfo ldgliteInfo(QString("%1/%2/bin/ldglite.exe").arg(lpub3d3rdPartyAppDir, VER_LDGLITE_STR));
#if defined __i386__ || defined _M_IX86
    QFileInfo ldviewInfo(QString("%1/%2/bin/LDView.exe").arg(lpub3d3rdPartyAppDir, VER_LDVIEW_STR));
    QFileInfo povrayInfo(QString("%1/%2/bin/lpub3d_trace_cui32.exe").arg(lpub3d3rdPartyAppDir, VER_POVRAY_STR));
#elif defined __x86_64__ || defined _M_X64
    QFileInfo ldviewInfo(QString("%1/%2/bin/LDView64.exe").arg(lpub3d3rdPartyAppDir, VER_LDVIEW_STR));
    QFileInfo povrayInfo(QString("%1/%2/bin/lpub3d_trace_cui64.exe").arg(lpub3d3rdPartyAppDir, VER_POVRAY_STR));
#endif
#elif defined Q_OS_MAC
    lpub3d3rdPartyAppDir = QString("%1/%2.app/Contents/3rdParty").arg(lpub3dPath).arg(lpub3dAppName);

    QFileInfo ldgliteInfo(QString("%1/%2/bin/ldglite").arg(lpub3d3rdPartyAppDir, VER_LDGLITE_STR));
    QFileInfo ldviewInfo(QString("%1/%2/bin/LDView").arg(lpub3d3rdPartyAppDir, VER_LDVIEW_STR));
    QFileInfo povrayInfo(QString("%1/%2/bin/lpub3d_trace_cui").arg(lpub3d3rdPartyAppDir, VER_POVRAY_STR));
#else
    QDir appDir(QString("%1/../share").arg(lpub3dPath));
    lpub3d3rdPartyAppDir = QString("%1/%2/3rdParty").arg(appDir.absolutePath(), lpub3dAppName);

    appDir.setPath(QString("%1/../../opt").arg(lpub3dPath));
    QString lpub3d3rdPartyAppExeDir = QString("%1/%2/3rdParty").arg(appDir.absolutePath(), lpub3dAppName);

    QFileInfo ldgliteInfo(QString("%1/%2/bin/ldglite").arg(lpub3d3rdPartyAppExeDir, VER_LDGLITE_STR));
    QFileInfo ldviewInfo(QString("%1/%2/bin/ldview").arg(lpub3d3rdPartyAppExeDir, VER_LDVIEW_STR));
    QFileInfo povrayInfo(QString("%1/%2/bin/lpub3d_trace_cui").arg(lpub3d3rdPartyAppExeDir, VER_POVRAY_STR));
#endif

    /* 3rd Party application installation status */

    bool    ldgliteInstalled = false;
    bool    ldviewInstalled = false;
    bool    povRayInstalled = false;

    // LDGLite EXE
    if (ldgliteInfo.exists()) {
        ldgliteInstalled = true;
        ldgliteExe = QDir::toNativeSeparators(ldgliteInfo.absoluteFilePath());
        logInfo() << QString("LDGLite : %1").arg(ldgliteExe);
    } else {
        logError() << QString("LDGLite : %1 not found").arg(ldgliteInfo.absoluteFilePath());
    }

#ifdef Q_OS_MAC
    QStringList missingLibs;
    QPixmap _icon = QPixmap(":/icons/lpub96.png");
    QMessageBoxResizable box;
    box.setWindowIcon(QIcon());
    box.setIconPixmap (_icon);
    box.setTextFormat (Qt::RichText);
    box.setWindowTitle(QMessageBox::tr ("Missing Libraries"));
    box.setWindowFlags (Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
    box.setStandardButtons (QMessageBox::Close);
#endif

    if (ldviewInfo.exists()) {
        ldviewInstalled = true;
        ldviewExe = QDir::toNativeSeparators(ldviewInfo.absoluteFilePath());
        logInfo() << QString("LDView  : %1").arg(ldviewExe);

#ifdef Q_OS_MAC

        // Check macOS LDView Libraries

        if (! Settings.contains(QString("%1/%2").arg(SETTINGS,"LDViewMissingLibs"))) {
            QVariant eValue(ldviewMissingLibs);
            Settings.setValue(QString("%1/%2").arg(SETTINGS,"LDViewMissingLibs"),eValue);
        } else {
            ldviewMissingLibs = Settings.value(QString("%1/%2").arg(SETTINGS,"LDViewMissingLibs")).toBool();
        }

        if (ldviewMissingLibs) {
            ldviewMissingLibs = false;
            QFileInfo libInfo("/usr/local/opt/libpng/lib/libpng.dylib");

            if (!libInfo.exists()){
                if (!ldviewMissingLibs)
                    ldviewMissingLibs = true;
                missingLibs << libInfo.absoluteFilePath();
            }
            libInfo.setFile("/usr/local/opt/gl2ps/lib/libgl2ps.dylib");
            if (!libInfo.exists()){
                if (!ldviewMissingLibs)
                    ldviewMissingLibs = true;
                missingLibs << libInfo.absoluteFilePath();
            }
            libInfo.setFile("/usr/local/opt/libjpg/lib/libjpeg.dylib");
            if (!libInfo.exists()){
                if (!ldviewMissingLibs)
                    ldviewMissingLibs = true;
                missingLibs << libInfo.absoluteFilePath();
            }
            libInfo.setFile("/usr/local/opt/tinyxml/lib/libtinyxml.dylib");
            if (!libInfo.exists()){
                if (!ldviewMissingLibs)
                    ldviewMissingLibs = true;
                missingLibs << libInfo.absoluteFilePath();
            }
            libInfo.setFile("/usr/local/opt/minizip/lib/libminizip.dylib");
            if (!libInfo.exists()){
                if (!ldviewMissingLibs)
                    ldviewMissingLibs = true;
                missingLibs << libInfo.absoluteFilePath();
            }

            QVariant eValue(ldviewMissingLibs);
            if (!ldviewMissingLibs)
                Settings.setValue(QString("%1/%2").arg(SETTINGS,"LDViewMissingLibs"),eValue);

            if (ldviewMissingLibs){
                QString header = "<b> " + QMessageBox::tr ("Required libraries for the LDView renderer were not found!!") + "</b>";
                QString body = QMessageBox::tr ("The following libraries were not found:\n\n -%1\n\n"
                                                "See %2/assets/docs/lpub3d/macOS_libs.html for install details.")
                        .arg(missingLibs.join("\n -"))
                        .arg(VER_COMPANYDOMAIN_STR);
                box.setText (header);
                box.setInformativeText (body);

                if (modeGUI) {
                    if (!lpub3dLoaded && Application::instance()->splash->isVisible())
                        Application::instance()->splash->hide();
                    if (box.exec() == QMessageBox::Close) {
                        if (! lpub3dLoaded && modeGUI && Application::instance()->splash->isHidden())
                            Application::instance()->splash->show();
                    }
                } else {
                    fprintf(stdout,"%s\n",body.toLatin1().constData());
                    fflush(stdout);
                }
            }
        }
#endif
    } else {
        logError() << QString("LDView   : %1 not found").arg(ldviewInfo.absoluteFilePath());
    }

    // POV-Ray EXE
    if (povrayInfo.exists()) {
        povRayInstalled = true;
        povrayExe = QDir::toNativeSeparators(povrayInfo.absoluteFilePath());
        logInfo() << QString("POVRay  : %1").arg(povrayExe);

#ifdef Q_OS_MAC

        // Check POVRay libraries on macOS
        if (! Settings.contains(QString("%1/%2").arg(SETTINGS,"POVRayMissingLibs"))) {
          QVariant eValue(povrayMissingLibs);
          Settings.setValue(QString("%1/%2").arg(SETTINGS,"POVRayMissingLibs"),eValue);
        } else {
          povrayMissingLibs = Settings.value(QString("%1/%2").arg(SETTINGS,"POVRayMissingLibs")).toBool();
        }

        if (povrayMissingLibs) {
            missingLibs.clear();
            povrayMissingLibs = false;
            QFileInfo libInfo("/usr/local/opt/libtiff/lib/libtiff.dylib");
            if (!libInfo.exists()){
                if (!povrayMissingLibs)
                  povrayMissingLibs = true;
                missingLibs << libInfo.absoluteFilePath();
            }
            libInfo.setFile("/usr/local/opt/openexr/lib/libIlmImf.dylib");
            if (!libInfo.exists()){
                if (!povrayMissingLibs)
                  povrayMissingLibs = true;
                missingLibs << libInfo.absoluteFilePath();
            }
            libInfo.setFile("/usr/local/opt/ilmbase/lib/libHalf.dylib");
            if (!libInfo.exists()){
                if (!povrayMissingLibs)
                  povrayMissingLibs = true;
                missingLibs << libInfo.absoluteFilePath();
            }
            libInfo.setFile("/usr/local/opt/sdl2/lib/libSDL2.dylib");
            if (!libInfo.exists()){
                if (!povrayMissingLibs)
                  povrayMissingLibs = true;
                missingLibs << libInfo.absoluteFilePath();
            }

            QVariant eValue(ldviewMissingLibs);
            if (!ldviewMissingLibs)
              Settings.setValue(QString("%1/%2").arg(SETTINGS,"POVRayMissingLibs"),eValue);

            if (povrayMissingLibs){
              QString header = "<b> " + QMessageBox::tr ("Required libraries for the POVRay renderer were not found!\n\n") + "</b>";
              QString body = QMessageBox::tr ("The following libraries were not found:\n\n -%1\n\n"
                                              "See %2/assets/docs/lpub3d/macOS_libs.html for install details.")
                                              .arg(missingLibs.join("\n -"))
                                              .arg(VER_COMPANYDOMAIN_STR);
              box.setText (header);
              box.setInformativeText (body);

              if (modeGUI) {
                if (!lpub3dLoaded && Application::instance()->splash->isVisible())
                  Application::instance()->splash->hide();
                if (box.exec() == QMessageBox::Close) {
                    if (! lpub3dLoaded && modeGUI && Application::instance()->splash->isHidden())
                      Application::instance()->splash->show();
                }
              } else {
                fprintf(stdout,"%s\n",body.toLatin1().constData());
                fflush(stdout);
              }
           }
        }
#endif
    } else {
        logError() << QString("POVRay  : %1 not found").arg(povrayInfo.absoluteFilePath());
    }

    /* Find out if we have a valid preferred renderer */

    QString const preferredRendererKey("PreferredRenderer");

    // Get preferred renderer from Registry
    if (Settings.contains(QString("%1/%2").arg(SETTINGS,preferredRendererKey))) {
        preferredRenderer = Settings.value(QString("%1/%2").arg(SETTINGS,preferredRendererKey)).toString();
        if (preferredRenderer == RENDERER_LDGLITE) {
            if ( ! ldgliteInstalled)  {
                preferredRenderer.clear();
                Settings.remove(QString("%1/%2").arg(SETTINGS,preferredRendererKey));
            }
        } else if (preferredRenderer == RENDERER_LDVIEW) {
            if ( ! ldviewInstalled) {
                preferredRenderer.clear();
                Settings.remove(QString("%1/%2").arg(SETTINGS,preferredRendererKey));
            }
        } else if (preferredRenderer == RENDERER_POVRAY) {
            if ( ! povRayInstalled) {
                preferredRenderer.clear();
                Settings.remove(QString("%1/%2").arg(SETTINGS,preferredRendererKey));
            }
        }

    } else { // No Registry setting so set preferred renderer if installed...

        preferredRenderer = RENDERER_NATIVE;

// -- previous setting default
//#ifdef Q_OS_MAC
//        if (!ldviewMissingLibs)
//          preferredRenderer = RENDERER_LDVIEW;
//#else
//        preferredRenderer = RENDERER_LDVIEW;
//#endif

// -- old setting default
//        if (ldgliteInstalled && povRayInstalled) {
//            preferredRenderer = ldviewInstalled  ? RENDERER_LDVIEW : RENDERER_LDGLITE;
//        } else if (povRayInstalled) {
//            preferredRenderer = RENDERER_POVRAY;
//        } else if (ldviewInstalled) {
//            preferredRenderer = RENDERER_LDVIEW;
//        } else if (ldgliteInstalled) {
//            preferredRenderer = RENDERER_LDGLITE;
//        }

        if (!preferredRenderer.isEmpty()) {
            Settings.setValue(QString("%1/%2").arg(SETTINGS,preferredRendererKey),preferredRenderer);
        }
    }

    /* Set use multiple files single call rendering option */

    if (! Settings.contains(QString("%1/%2").arg(SETTINGS,"EnableLDViewSingleCall"))) {
        QVariant eValue(false);
        if (preferredRenderer == RENDERER_LDVIEW) {
            enableLDViewSingleCall = true;
        } else {
            enableLDViewSingleCall = false;
        }
        Settings.setValue(QString("%1/%2").arg(SETTINGS,"EnableLDViewSingleCall"),eValue);
    } else {
        enableLDViewSingleCall = Settings.value(QString("%1/%2").arg(SETTINGS,"EnableLDViewSingleCall")).toBool();
    }

    if ( ! Settings.contains(QString("%1/%2").arg(SETTINGS,"EnableLDViewSnapshotsList"))) {
        QVariant uValue(false);
        enableLDViewSnaphsotList = false;
        Settings.setValue(QString("%1/%2").arg(SETTINGS,"EnableLDViewSnapshotsList"),uValue);
    } else {
        enableLDViewSnaphsotList = Settings.value(QString("%1/%2").arg(SETTINGS,"EnableLDViewSnapshotsList")).toBool();
    }

    //Renderer Timeout
    if ( ! Settings.contains(QString("%1/%2").arg(SETTINGS,"RendererTimeout"))) {
        rendererTimeout = RENDERER_TIMEOUT_DEFAULT;
        Settings.setValue(QString("%1/%2").arg(SETTINGS,"RendererTimeout"),rendererTimeout);
    } else {
        rendererTimeout = Settings.value(QString("%1/%2").arg(SETTINGS,"RendererTimeout")).toInt();
    }

    // Native renderer camera distance factor
    if (Settings.contains(QString("%1/%2").arg(SETTINGS,"cameraDistFactorNative"))) {
        cameraDistFactorNative = Settings.value(QString("%1/%2").arg(SETTINGS,"cameraDistFactorNative")).toInt();
    } else {
        cameraDistFactorNative = CAMERA_DISTANCE_FACTOR_NATIVE_DEFAULT;
    }

    // povray generation renderer
    if ( ! Settings.contains(QString("%1/%2").arg(SETTINGS,"POVFileGenerator"))) {
        QVariant cValue(RENDERER_NATIVE);
        povFileGenerator = RENDERER_NATIVE;
        Settings.setValue(QString("%1/%2").arg(SETTINGS,"POVFileGenerator"),cValue);
    } else {
        povFileGenerator = Settings.value(QString("%1/%2").arg(SETTINGS,"POVFileGenerator")).toString();
    }

    // display povray image during rendering
    QString const povrayDisplayKey("POVRayDisplay");
    if ( ! Settings.contains(QString("%1/%2").arg(SETTINGS,povrayDisplayKey))) {
        QVariant uValue(false);
        povrayDisplay = false;
        Settings.setValue(QString("%1/%2").arg(SETTINGS,povrayDisplayKey),uValue);
    } else {
        povrayDisplay = Settings.value(QString("%1/%2").arg(SETTINGS,povrayDisplayKey)).toBool();
    }

    if ( ! Settings.contains(QString("%1/%2").arg(SETTINGS,"ApplyCALocally"))) {
        QVariant uValue(true);
        applyCALocally = true;
        Settings.setValue(QString("%1/%2").arg(SETTINGS,"ApplyCALocally"),uValue);
    } else {
        applyCALocally = Settings.value(QString("%1/%2").arg(SETTINGS,"ApplyCALocally")).toBool();
    }

    QString const enableImageMattingKey("EnableImageMatting");
    if ( ! Settings.contains(QString("%1/%2").arg(SETTINGS,enableImageMattingKey))) {
        QVariant uValue(false);
        enableImageMatting = false;
        Settings.setValue(QString("%1/%2").arg(SETTINGS,enableImageMattingKey),uValue);
    } else {
        enableImageMatting = Settings.value(QString("%1/%2").arg(SETTINGS,enableImageMattingKey)).toBool();
    }

    // Write config files
    logInfo() << QString("Processing renderer configuration files...");

    lpub3d3rdPartyConfigDir = QString("%1/3rdParty").arg(lpubDataPath);
    setLDGLiteIniParams();
    updateNativePOVIniFile(updateFlag);
    updateLDViewIniFile(updateFlag);
    updateLDViewPOVIniFile(updateFlag);
    updatePOVRayConfFile(updateFlag);
    updatePOVRayIniFile(updateFlag);

    QFileInfo resourceFile;

    // Populate POVRay Library paths
    resourceFile.setFile(QString("%1/%2/resources/ini/%3").arg(lpub3d3rdPartyAppDir, VER_POVRAY_STR, VER_POVRAY_INI_FILE));
    if (resourceFile.exists())
        povrayIniPath = resourceFile.absolutePath();
    logInfo() << QString("POVRay ini path    : %1").arg(povrayIniPath.isEmpty() ? "Not found" : povrayIniPath);

    resourceFile.setFile(QString("%1/%2/resources/include/%3").arg(lpub3d3rdPartyAppDir, VER_POVRAY_STR, VER_POVRAY_INC_FILE));
    if (resourceFile.exists())
        povrayIncPath = resourceFile.absolutePath();
    logInfo() << QString("POVRay include path: %1").arg(povrayIncPath.isEmpty() ? "Not found" : povrayIncPath);

    resourceFile.setFile(QString("%1/%2/resources/scenes/%3").arg(lpub3d3rdPartyAppDir, VER_POVRAY_STR, VER_POVRAY_SCENE_FILE));
    if (resourceFile.exists())
        povrayScenePath = resourceFile.absolutePath();
    logInfo() << QString("POVRay scene path  : %1").arg(povrayScenePath.isEmpty() ? "Not found" : povrayScenePath);

    logInfo() << QString("Renderer is %1 %2")
                         .arg(preferredRenderer)
                         .arg(preferredRenderer == RENDERER_POVRAY ?
                                  QString("(%1 POV file generator)").arg(Preferences::povFileGenerator) :
                              preferredRenderer == RENDERER_LDVIEW ?
                              Preferences::enableLDViewSingleCall ?
                                  QString("(Single Call%1)")
                                          .arg(Preferences::enableLDViewSnaphsotList ? " - SnapshotsList" : "") : "" : "");
}

void Preferences::setLDGLiteIniParams()
{
    QFileInfo resourceFile;
    QFile confFileIn;

    resourceFile.setFile(QString("%1/%2/config/%3").arg(lpub3d3rdPartyConfigDir, VER_LDGLITE_STR, VER_LDGLITE_INI_FILE));
    if (!resourceFile.exists()) {
        logInfo() << QString("Reading %1...").arg(resourceFile.absoluteFilePath());
        if (!resourceFile.absoluteDir().exists())
            resourceFile.absoluteDir().mkpath(".");
        QFile::copy(dataLocation + resourceFile.fileName(), resourceFile.absoluteFilePath());
    }
    confFileIn.setFileName(resourceFile.absoluteFilePath());
    if (confFileIn.open(QIODevice::ReadOnly))
    {
        ldgliteParms.clear();
        QTextStream input(&confFileIn);
        while (!input.atEnd())
        {
            QString line = input.readLine();
            if (line.contains(QRegExp("^-.*")))
            {
                //logDebug() << QString("Line PARAM: %1").arg(line);
                ldgliteParms << line;
            }
        }
        confFileIn.close();
    } else {
        QString confFileError;
        if (!confFileIn.errorString().isEmpty())
            confFileError.append(QString(" confFileInError: %1").arg(confFileIn.errorString()));
        logError() << QString("Could not open input file: %1").arg(confFileError);
    }
    if (preferredRenderer == RENDERER_LDGLITE)
        logInfo() << QString("LDGLite Parameters :%1").arg((ldgliteParms.isEmpty() ? "No parameters" : ldgliteParms.join(" ")));
    if (resourceFile.exists())
        ldgliteIni = resourceFile.absoluteFilePath(); // populate ldglite ini file
    logInfo() << QString("LDGLite.ini file   : %1").arg(ldgliteIni.isEmpty() ? "Not found" : ldgliteIni);
}

void Preferences::updateNativePOVIniFile(UpdateFlag updateFlag)
{
    QString inFileName;
    QFileInfo resourceFile;
    QFile confFileIn, confFileOut, oldFile;
    QDateTime timeStamp = QDateTime::currentDateTime();

    resourceFile.setFile(QString("%1/extras/%2").arg(lpubDataPath, VER_NATIVE_POV_INI_FILE));
    if (resourceFile.exists())
    {
        if (updateFlag == SkipExisting) {
           nativePOVIni = resourceFile.absoluteFilePath(); // populate native pov ini file
           logInfo() << QString("NativePOV ini file : %1").arg(nativePOVIni);
           return;
        }
        logInfo() << QString("Updating %1...").arg(resourceFile.absoluteFilePath());
        inFileName = QString("%1.%2").arg(resourceFile.absoluteFilePath(),timeStamp.toString("ddMMyyhhmmss"));
        oldFile.setFileName(resourceFile.absoluteFilePath());
        oldFile.rename(inFileName);
                confFileIn.setFileName(QDir::toNativeSeparators(inFileName));
    } else {
       logInfo() << QString("Initializing %1...").arg(resourceFile.absoluteFilePath());
       if (!resourceFile.absoluteDir().exists())
           resourceFile.absoluteDir().mkpath(".");
       confFileIn.setFileName(dataLocation + resourceFile.fileName());
    }
    confFileOut.setFileName(QString("%1/extras/%2").arg(lpubDataPath, VER_NATIVE_POV_INI_FILE));
    if (confFileIn.open(QIODevice::ReadOnly) && confFileOut.open(QIODevice::WriteOnly))
    {
        QTextStream input(&confFileIn);
        QTextStream output(&confFileOut);
        while (!input.atEnd())
        {
            QString line = input.readLine();
            // strip EdgeThickness because set in renderer parms
            if (line.contains(QRegExp("^EdgeThickness="))){
              continue;
            }
            //logDebug() << QString("Line INPUT: %1").arg(line);
            // set ldraw dir
            if (line.contains(QRegExp("^LDrawDir=")))
            {
                line.clear();
                line = QString("LDrawDir=%1").arg(QDir::toNativeSeparators(ldrawPath));
            }
            // set lgeo paths as required
            if (line.contains(QRegExp("^XmlMapPath=")))
            {
                line.clear();
                if (lgeoPath.isEmpty())
                {
                    line = QString("XmlMapPath=");
                } else {
                    line = QString("XmlMapPath=%1").arg(QDir::toNativeSeparators(QString("%1/%2").arg(lgeoPath).arg(VER_LGEO_XML_FILE)));
                }
            }
            logInfo() << QString("NativePOV.ini OUT: %1").arg(line);
            output << line << endl;
        }
        confFileIn.close();
        confFileOut.close();
    } else {
        QString confFileError;
        if (!confFileIn.errorString().isEmpty())
            confFileError.append(QString(" confFileInError: %1\n").arg(confFileIn.errorString()));
        if (!confFileOut.errorString().isEmpty())
            confFileError.append(QString(" confFileOutError: %1").arg(confFileOut.errorString()));
        logError() << QString("Could not open NativePOV.ini input or output file: %1").arg(confFileError);
    }
    if (resourceFile.exists())
        nativePOVIni = resourceFile.absoluteFilePath(); // populate native POV ini file
    if (oldFile.exists())
        oldFile.remove();                               // delete old file
    logInfo() << QString("NativePOV ini file : %1").arg(nativePOVIni.isEmpty() ? "Not found" : nativePOVIni);
}

void Preferences::updateLDViewIniFile(UpdateFlag updateFlag)
{
    QString inFileName;
    QFileInfo resourceFile;
    QFile confFileIn, confFileOut, oldFile;
    QDateTime timeStamp = QDateTime::currentDateTime();

    resourceFile.setFile(QString("%1/%2/config/%3").arg(lpub3d3rdPartyConfigDir, VER_LDVIEW_STR, VER_LDVIEW_INI_FILE));
    if (resourceFile.exists())
    {
       if (updateFlag == SkipExisting) {
           ldviewIni = resourceFile.absoluteFilePath(); // populate ldview ini file
           logInfo() << QString("LDView ini file    : %1").arg(ldviewIni);
           return;
       }
       logInfo() << QString("Updating %1...").arg(resourceFile.absoluteFilePath());
       inFileName = QString("%1.%2").arg(resourceFile.absoluteFilePath(),timeStamp.toString("ddMMyyhhmmss"));
       oldFile.setFileName(resourceFile.absoluteFilePath());
       oldFile.rename(inFileName);
    } else {
       logInfo() << QString("Initializing %1...").arg(resourceFile.absoluteFilePath());
       inFileName = QString("%1/%2/resources/config/%3").arg(lpub3d3rdPartyAppDir, VER_LDVIEW_STR, resourceFile.fileName());
       if (!resourceFile.absoluteDir().exists())
           resourceFile.absoluteDir().mkpath(".");
    }
    confFileIn.setFileName(QDir::toNativeSeparators(inFileName));
    confFileOut.setFileName(QString("%1/%2/config/%3").arg(lpub3d3rdPartyConfigDir, VER_LDVIEW_STR, VER_LDVIEW_INI_FILE));
    if (confFileIn.open(QIODevice::ReadOnly) && confFileOut.open(QIODevice::WriteOnly))
    {
        QTextStream input(&confFileIn);
        QTextStream output(&confFileOut);
        while (!input.atEnd())
        {
            QString line = input.readLine();
            // strip EdgeThickness because set in renderer parms
            if (line.contains(QRegExp("^EdgeThickness="))){
                continue;
            }
            //logDebug() << QString("Line INPUT: %1").arg(line);
            // set ldraw dir
            if (line.contains(QRegExp("^LDrawDir=")))
            {
                line.clear();
                line = QString("LDrawDir=%1").arg(QDir::toNativeSeparators(ldrawPath));
            }
            // set AutoCrop=0
//            if (line.contains(QRegExp("^AutoCrop="))) {
//                line.clear();
//                line = QString("AutoCrop=%1").arg((Preferences::enableFadeSteps && Preferences::enableImageMatting) ? 0 : 1);
//            }
            logInfo() << QString("LDView.ini OUT: %1").arg(line);
            output << line << endl;
        }
        confFileIn.close();
        confFileOut.flush();
        confFileOut.close();
    } else {
        QString confFileError;
        if (!confFileIn.errorString().isEmpty())
            confFileError.append(QString(" confFileInError: %1\n").arg(confFileIn.errorString()));
        if (!confFileOut.errorString().isEmpty())
            confFileError.append(QString(" confFileOutError: %1").arg(confFileOut.errorString()));
        logError() << QString("Could not open LDView.ini input or output file: %1").arg(confFileError);
    }
    if (resourceFile.exists())
        ldviewIni = resourceFile.absoluteFilePath(); // populate ldview ini file
    if (oldFile.exists())
        oldFile.remove();                            // delete old file
    logInfo() << QString("LDView ini file    : %1").arg(ldviewIni.isEmpty() ? "Not found" : ldviewIni);
}

void Preferences::updateLDViewPOVIniFile(UpdateFlag updateFlag)
{
    QString inFileName;
    QFileInfo resourceFile;
    QFile confFileIn, confFileOut, oldFile;
    QDateTime timeStamp = QDateTime::currentDateTime();

    resourceFile.setFile(QString("%1/%2/config/%3").arg(lpub3d3rdPartyConfigDir, VER_LDVIEW_STR, VER_LDVIEW_POV_INI_FILE));
    if (resourceFile.exists())
    {
        if (updateFlag == SkipExisting) {
           ldviewPOVIni = resourceFile.absoluteFilePath(); // populate ldview pov ini file
           logInfo() << QString("LDViewPOV ini file : %1").arg(ldviewPOVIni);
           return;
        }
        logInfo() << QString("Updating %1...").arg(resourceFile.absoluteFilePath());
        inFileName = QString("%1.%2").arg(resourceFile.absoluteFilePath(),timeStamp.toString("ddMMyyhhmmss"));
        oldFile.setFileName(resourceFile.absoluteFilePath());
        oldFile.rename(inFileName);
    } else {
       logInfo() << QString("Initializing %1...").arg(resourceFile.absoluteFilePath());
       inFileName = QString("%1/%2/resources/config/%3").arg(lpub3d3rdPartyAppDir, VER_LDVIEW_STR, resourceFile.fileName());
       if (!resourceFile.absoluteDir().exists())
           resourceFile.absoluteDir().mkpath(".");
    }
    confFileIn.setFileName(QDir::toNativeSeparators(inFileName));
    confFileOut.setFileName(QString("%1/%2/config/%3").arg(lpub3d3rdPartyConfigDir, VER_LDVIEW_STR, VER_LDVIEW_POV_INI_FILE));
    if (confFileIn.open(QIODevice::ReadOnly) && confFileOut.open(QIODevice::WriteOnly))
    {
        QTextStream input(&confFileIn);
        QTextStream output(&confFileOut);
        while (!input.atEnd())
        {
            QString line = input.readLine();
            // strip EdgeThickness because set in renderer parms
            if (line.contains(QRegExp("^EdgeThickness="))){
              continue;
            }
            //logDebug() << QString("Line INPUT: %1").arg(line);
            // set ldraw dir
            if (line.contains(QRegExp("^LDrawDir=")))
            {
                line.clear();
                line = QString("LDrawDir=%1").arg(QDir::toNativeSeparators(ldrawPath));
            }
            // set lgeo paths as required
            if (line.contains(QRegExp("^XmlMapPath=")))
            {
                line.clear();
                if (lgeoPath.isEmpty())
                {
                    line = QString("XmlMapPath=");
                } else {
                    line = QString("XmlMapPath=%1").arg(QDir::toNativeSeparators(QString("%1/%2").arg(lgeoPath).arg(VER_LGEO_XML_FILE)));
                }
            }
            logInfo() << QString("LDViewPOV.ini OUT: %1").arg(line);
            output << line << endl;
        }
        confFileIn.close();
        confFileOut.close();
    } else {
        QString confFileError;
        if (!confFileIn.errorString().isEmpty())
            confFileError.append(QString(" confFileInError: %1\n").arg(confFileIn.errorString()));
        if (!confFileOut.errorString().isEmpty())
            confFileError.append(QString(" confFileOutError: %1").arg(confFileOut.errorString()));
        logError() << QString("Could not open LDViewPOV.ini input or output file: %1").arg(confFileError);
    }
    if (resourceFile.exists())
        ldviewPOVIni = resourceFile.absoluteFilePath(); // populate ldview pov ini file
    if (oldFile.exists())
        oldFile.remove();                               // delete old file
    logInfo() << QString("LDViewPOV ini file : %1").arg(ldviewPOVIni.isEmpty() ? "Not found" : ldviewPOVIni);
}

void Preferences::updatePOVRayConfFile(UpdateFlag updateFlag)
{
    QString inFileName;
    QFileInfo resourceFile;
    QFile confFileIn, confFileOut, oldFile;
    QDateTime timeStamp = QDateTime::currentDateTime();

    // POV-Ray Conf
    resourceFile.setFile(QString("%1/%2/config/%3").arg(lpub3d3rdPartyConfigDir, VER_POVRAY_STR ,VER_POVRAY_CONF_FILE));
    if (resourceFile.exists())
    {
        if (updateFlag == SkipExisting) {
           povrayConf = resourceFile.absoluteFilePath();  // populate povray conf file
           logInfo() << QString("POVRay conf file   : %1").arg(povrayConf);
           return;
        }
        logInfo() << QString("Updating %1...").arg(resourceFile.absoluteFilePath());
        inFileName = QString("%1.%2").arg(resourceFile.absoluteFilePath(),timeStamp.toString("ddMMyyhhmmss"));
        oldFile.setFileName(resourceFile.absoluteFilePath());
        oldFile.rename(inFileName);
    } else {
       logInfo() << QString("Initializing %1...").arg(resourceFile.absoluteFilePath());
       inFileName = QString("%1/%2/resources/config/%3").arg(lpub3d3rdPartyAppDir, VER_POVRAY_STR, resourceFile.fileName());
       if (!resourceFile.absoluteDir().exists())
           resourceFile.absoluteDir().mkpath(".");
    }
    confFileIn.setFileName(QDir::toNativeSeparators(inFileName));
    confFileOut.setFileName(QString("%1/%2/config/%3").arg(lpub3d3rdPartyConfigDir, VER_POVRAY_STR, VER_POVRAY_CONF_FILE));
    if (confFileIn.open(QIODevice::ReadOnly) && confFileOut.open(QIODevice::WriteOnly))
    {
        QTextStream input(&confFileIn);
        QTextStream output(&confFileOut);
        while (!input.atEnd())
        {
            QString line = input.readLine();
            //logDebug() << QString("Line INPUT: %1").arg(line);
            if (line.contains(QRegExp("^read* =")) && oldFile.exists())
            {
                if (lgeoPath != "")
                {
                    if (line.contains(QRegExp("[\\/|\\\\]ar")))
                    {
                        line.clear();
                        line = QString("read* = \"%1\"").arg(QDir::toNativeSeparators(QString("%1/ar").arg(lgeoPath)));
                    }
                    if (line.contains(QRegExp("[\\/|\\\\]lg")))
                    {
                        line.clear();
                        line = QString("read* = \"%1\"").arg(QDir::toNativeSeparators(QString("%1/lg").arg(lgeoPath)));
                    }
                    if (lgeoStlLib && line.contains(QRegExp("[\\/|\\\\]stl")))
                    {
                        line.clear();
                        line = QString("read* = \"%1\"").arg(QDir::toNativeSeparators(QString("%1/stl").arg(lgeoPath)));
                    }
                }
            } else {
                // set application 3rd party renderers path
                line.replace(QString("__POVSYSDIR__"), QDir::toNativeSeparators(QString("%1/3rdParty/%2").arg(lpub3dPath, VER_POVRAY_STR)));
                // set lgeo paths as required
                if (lgeoPath != "")
                {
                    line.replace(QString("; read* = \"__LGEOARDIR__"), QString("read* = \"%1").arg(QDir::toNativeSeparators(lgeoPath)));
                    line.replace(QString("; read* = \"__LGEOLGDIR__"), QString("read* = \"%1").arg(QDir::toNativeSeparators(lgeoPath)));
                    if (lgeoStlLib){
                        line.replace(QString("; read* = \"__LGEOSTLDIR__"), QString("read* = \"%1").arg(QDir::toNativeSeparators(lgeoPath)));
                    }
                }
            }
            logInfo() << QString("POV-Ray.conf OUT: %1").arg(line);
            output << line << endl;
        }
        confFileIn.close();
        confFileOut.close();
    } else {
        QString confFileError;
        if (!confFileIn.errorString().isEmpty())
            confFileError.append(QString(" confFileInError: %1\n").arg(confFileIn.errorString()));
        if (!confFileOut.errorString().isEmpty())
            confFileError.append(QString(" confFileOutError: %1").arg(confFileOut.errorString()));
        logError() << QString("Could not open POVRay.conf input or output file: %1").arg(confFileError);
    }
    if (resourceFile.exists())
        povrayConf = resourceFile.absoluteFilePath();  // populate povray conf file
    if (oldFile.exists())
        oldFile.remove();                              // delete old file
    logInfo() << QString("POVRay conf file   : %1").arg(povrayConf.isEmpty() ? "Not found" : povrayConf);
}

void Preferences::updatePOVRayIniFile(UpdateFlag updateFlag)
{
    QString inFileName;
    QFileInfo resourceFile;
    QFile confFileIn, confFileOut, oldFile;
    QDateTime timeStamp = QDateTime::currentDateTime();

    resourceFile.setFile(QString("%1/%2/%3").arg(lpub3d3rdPartyConfigDir, VER_POVRAY_STR "/config" ,VER_POVRAY_INI_FILE));
    if (resourceFile.exists())
    {
        if (updateFlag == SkipExisting) {
            povrayIni = resourceFile.absoluteFilePath();     // populate povray ini file
            logInfo() << QString("POVRay ini file    : %1").arg(povrayIni);
           return;
        }
        logInfo() << QString("Updating %1...").arg(resourceFile.absoluteFilePath());
        inFileName = QString("%1.%2").arg(resourceFile.absoluteFilePath(),timeStamp.toString("ddMMyyhhmmss"));
        oldFile.setFileName(resourceFile.absoluteFilePath());
        oldFile.rename(inFileName);
    } else {
       logInfo() << QString("Initializing %1...").arg(resourceFile.absoluteFilePath());
       inFileName = QString("%1/%2/resources/config/%3").arg(lpub3d3rdPartyAppDir, VER_POVRAY_STR, resourceFile.fileName());
       if (!resourceFile.absoluteDir().exists())
           resourceFile.absoluteDir().mkpath(".");
    }
    confFileIn.setFileName(QDir::toNativeSeparators(inFileName));
    confFileOut.setFileName(QString("%1/%2/config/%3").arg(lpub3d3rdPartyConfigDir, VER_POVRAY_STR, VER_POVRAY_INI_FILE));
    if (confFileIn.open(QIODevice::ReadOnly) && confFileOut.open(QIODevice::WriteOnly))
    {
        QTextStream input(&confFileIn);
        QTextStream output(&confFileOut);
        while (!input.atEnd())
        {
            // set the application 3rd party renderers path
            QString line = input.readLine();
            if (! oldFile.exists())
              line.replace(QString("__POVSYSDIR__"), QDir::toNativeSeparators(QString("%1/3rdParty/%2").arg(lpub3dPath, VER_POVRAY_STR)));
            logInfo() << QString("POV-Ray.ini OUT: %1").arg(line);
            output << line << endl;
        }
        confFileIn.close();
        confFileOut.close();
    } else {
        QString confFileError;
        if (!confFileIn.errorString().isEmpty())
            confFileError.append(QString(" confFileInError: %1\n").arg(confFileIn.errorString()));
        if (!confFileOut.errorString().isEmpty())
            confFileError.append(QString(" confFileOutError: %1").arg(confFileOut.errorString()));
        logError() << QString("Could not open POVRay.ini input or output file: %1").arg(confFileError);
    }
    if (resourceFile.exists())
        povrayIni = resourceFile.absoluteFilePath();     // populate povray ini file
    if (oldFile.exists())
        oldFile.remove();                              // delete old file
    logInfo() << QString("POVRay ini file    : %1").arg(povrayIni.isEmpty() ? "Not found" : povrayIni);
}

void Preferences::pliPreferences()
{
    bool allIsWell = true;
    QSettings Settings;
    pliFile = Settings.value(QString("%1/%2").arg(SETTINGS,"PliControl")).toString();
    pliSubstitutePartsFile = Settings.value(QString("%1/%2").arg(SETTINGS,"PliSubstitutePartsFile")).toString();
    excludedPartsFile = Settings.value(QString("%1/%2").arg(SETTINGS,"ExcludedPartsFile")).toString();

    QFileInfo fileInfo(pliFile);
    if (! fileInfo.exists()) {
        Settings.remove(QString("%1/%2").arg(SETTINGS,"PliControl"));
        allIsWell = false;
    }

    QFileInfo pliSubstituteFileInfo(pliSubstitutePartsFile);
    if (! pliSubstituteFileInfo.exists()) {
        Settings.remove(QString("%1/%2").arg(SETTINGS,"PliSubstitutePartsFile"));
        allIsWell = false;
    }

    QFileInfo excludeFileInfo(excludedPartsFile);
    if (! excludeFileInfo.exists()) {
        Settings.remove(QString("%1/%2").arg(SETTINGS,"ExcludedPartsFile"));
        allIsWell = false;
    }

    if (allIsWell)
        return;

    pliFile = QDir::toNativeSeparators(QString("%1/%2").arg(lpubDataPath,"extras/pli.mpd"));
    pliSubstitutePartsFile = QDir::toNativeSeparators(QString("%1/%2").arg(lpubDataPath,"extras/pliSubstituteParts.lst"));
    excludedPartsFile = QDir::toNativeSeparators(QString("%1/%2").arg(lpubDataPath,"extras/excludedParts.lst"));

    QFileInfo popPliFileInfo(pliFile);
    popPliFileInfo.setFile(pliFile);
    if (popPliFileInfo.exists()) {
        Settings.setValue(QString("%1/%2").arg(SETTINGS,"PliControl"),pliFile);
    }

    QFileInfo popPliSubstituteFileInfo(pliSubstitutePartsFile);
    popPliSubstituteFileInfo.setFile(pliSubstitutePartsFile);
    if (popPliSubstituteFileInfo.exists()) {
        Settings.setValue(QString("%1/%2").arg(SETTINGS,"PliSubstitutePartsFile"),pliSubstitutePartsFile);
    }

    QFileInfo popExlcudedFileInfo(excludedPartsFile);
    popExlcudedFileInfo.setFile(excludedPartsFile);
    if (popExlcudedFileInfo.exists()) {
        Settings.setValue(QString("%1/%2").arg(SETTINGS,"ExcludedPartsFile"),excludedPartsFile);
    }
}

void Preferences::unitsPreferences()
{
    QSettings Settings;
    if ( ! Settings.contains(QString("%1/%2").arg(SETTINGS,"Centimeters"))) {
        QVariant uValue(false);
        preferCentimeters = false;
        Settings.setValue(QString("%1/%2").arg(SETTINGS,"Centimeters"),uValue);
    } else {
        preferCentimeters = Settings.value(QString("%1/%2").arg(SETTINGS,"Centimeters")).toBool();
    }
}

void Preferences::userInterfacePreferences()
{
  QSettings Settings;

  QString const themeAutoRestartKey("ThemeAutoRestart");
  if ( ! Settings.contains(QString("%1/%2").arg(SETTINGS,themeAutoRestartKey))) {
          QVariant uValue(false);
          themeAutoRestart = false;
          Settings.setValue(QString("%1/%2").arg(SETTINGS,themeAutoRestartKey),uValue);
  } else {
          themeAutoRestart = Settings.value(QString("%1/%2").arg(SETTINGS,themeAutoRestartKey)).toBool();
  }
  QString const pageRulerKey("PageRuler");
  if ( ! Settings.contains(QString("%1/%2").arg(SETTINGS,pageRulerKey))) {
          QVariant uValue(false);
          pageRuler = false;
          Settings.setValue(QString("%1/%2").arg(SETTINGS,pageRulerKey),uValue);
  } else {
          pageRuler = Settings.value(QString("%1/%2").arg(SETTINGS,pageRulerKey)).toBool();
  }

  QString const pageGuidesKey("PageGuides");
  if ( ! Settings.contains(QString("%1/%2").arg(SETTINGS,pageGuidesKey))) {
          QVariant uValue(false);
          pageGuides = false;
          Settings.setValue(QString("%1/%2").arg(SETTINGS,pageGuidesKey),uValue);
  } else {
          pageGuides = Settings.value(QString("%1/%2").arg(SETTINGS,pageGuidesKey)).toBool();
  }

  QString const displayThemeKey("DisplayTheme");
  if ( ! Settings.contains(QString("%1/%2").arg(SETTINGS,displayThemeKey))) {
          displayTheme = THEME_DEFAULT;
          Settings.setValue(QString("%1/%2").arg(SETTINGS,displayThemeKey),displayTheme);
  } else {
          displayTheme = Settings.value(QString("%1/%2").arg(SETTINGS,displayThemeKey)).toString();
  }

  QString const showParseErrorsKey("ShowParseErrors");
  if ( ! Settings.contains(QString("%1/%2").arg(SETTINGS,showParseErrorsKey))) {
          QVariant uValue(true);
          showParseErrors = true;
          Settings.setValue(QString("%1/%2").arg(SETTINGS,showParseErrorsKey),uValue);
  } else {
          showParseErrors = Settings.value(QString("%1/%2").arg(SETTINGS,showParseErrorsKey)).toBool();
  }
}


void Preferences::setPageGuidesPreference(bool b)
{
  QSettings Settings;
  pageGuides = b;
  QVariant uValue(b);
  QString const pageGuidesKey("PageGuides");
  Settings.setValue(QString("%1/%2").arg(SETTINGS,pageGuidesKey),uValue);
}

void Preferences::setPageRulerPreference(bool b)
{
  QSettings Settings;
  pageRuler = b;
  QVariant uValue(b);
  QString const pageRulerKey("PageRuler");
  Settings.setValue(QString("%1/%2").arg(SETTINGS,pageRulerKey),uValue);
}

void Preferences::setShowParseErrorsPreference(bool b)
{
  QSettings Settings;
  showParseErrors = b;
  QVariant uValue(b);
  QString const showParseErrorsKey("ShowParseErrors");
  Settings.setValue(QString("%1/%2").arg(SETTINGS,showParseErrorsKey),uValue);
}

void Preferences::annotationPreferences()
{
    bool allIsWell = true;
    QSettings Settings;
    titleAnnotationsFile = Settings.value(QString("%1/%2").arg(SETTINGS,"TitleAnnotationFile")).toString();
    freeformAnnotationsFile = Settings.value(QString("%1/%2").arg(SETTINGS,"FreeFormAnnotationsFile")).toString();

    QFileInfo titleFileInfo(titleAnnotationsFile);
    if (! titleFileInfo.exists()) {
        Settings.remove(QString("%1/%2").arg(SETTINGS,"TitleAnnotationFile"));
        allIsWell = false;
    }

    QFileInfo freeformFileInfo(freeformAnnotationsFile);
    if (! freeformFileInfo.exists()) {
        Settings.remove(QString("%1/%2").arg(SETTINGS,"FreeFormAnnotationsFile"));
        allIsWell = false;
    }

    if (allIsWell)
        return;

    titleAnnotationsFile    = QDir::toNativeSeparators(QString("%1/%2").arg(lpubDataPath,"extras/titleAnnotations.lst"));
    freeformAnnotationsFile = QDir::toNativeSeparators(QString("%1/%2").arg(lpubDataPath,"extras/freeformAnnotations.lst"));

    QFileInfo popTitleFileInfo(titleAnnotationsFile);
    popTitleFileInfo.setFile(titleAnnotationsFile);
    if (popTitleFileInfo.exists()) {
        Settings.setValue(QString("%1/%2").arg(SETTINGS,"TitleAnnotationFile"),titleAnnotationsFile);
    }

    QFileInfo popFreeFormFileInfo(freeformAnnotationsFile);
    popFreeFormFileInfo.setFile(freeformAnnotationsFile);
    if (popFreeFormFileInfo.exists()) {
        Settings.setValue(QString("%1/%2").arg(SETTINGS,"FreeFormAnnotationsFile"),freeformAnnotationsFile);
    }
}

void Preferences::fadestepPreferences()
{
    QSettings Settings;
    if (! Settings.contains(QString("%1/%2").arg(SETTINGS,"EnableFadeSteps"))) {
        QVariant eValue(false);
        enableFadeSteps = false;
        Settings.setValue(QString("%1/%2").arg(SETTINGS,"EnableFadeSteps"),eValue);
    } else {
        enableFadeSteps = Settings.value(QString("%1/%2").arg(SETTINGS,"EnableFadeSteps")).toBool();
    }

    if (! Settings.contains(QString("%1/%2").arg(SETTINGS,"FadeStepsUseColour"))) {
        QVariant eValue(false);
        fadeStepsUseColour = false;
        Settings.setValue(QString("%1/%2").arg(SETTINGS,"FadeStepsUseColour"),eValue);
    } else {
        fadeStepsUseColour = Settings.value(QString("%1/%2").arg(SETTINGS,"FadeStepsUseColour")).toBool();
    }

    if (! Settings.contains(QString("%1/%2").arg(SETTINGS,"FadeStepColor"))) {
        fadeStepsColour = FADE_COLOUR_DEFAULT;
        Settings.setValue(QString("%1/%2").arg(SETTINGS,"FadeStepColor"),fadeStepsColour);
    } else {
        if (fadeStepsColour.isEmpty())
            fadeStepsColour = FADE_COLOUR_DEFAULT;
        fadeStepsColour = Settings.value(QString("%1/%2").arg(SETTINGS,"FadeStepColor")).toString();
    }

    if (! Settings.contains(QString("%1/%2").arg(SETTINGS,"FadeStepsOpacity"))) {
        QVariant cValue(FADE_OPACITY_DEFAULT);
        fadeStepsOpacity = FADE_OPACITY_DEFAULT;
        Settings.setValue(QString("%1/%2").arg(SETTINGS,"FadeStepsOpacity"),cValue);
    } else {
        fadeStepsOpacity = Settings.value(QString("%1/%2").arg(SETTINGS,"FadeStepsOpacity")).toInt();
    }

    ldrawColourPartsFile = Settings.value(QString("%1/%2").arg(SETTINGS,"LDrawColourPartsFile")).toString();
    QFileInfo ldrawColorFileInfo(ldrawColourPartsFile);
    if (!ldrawColorFileInfo.exists()) {
        Settings.remove(QString("%1/%2").arg(SETTINGS,"LDrawColourPartsFile"));
    }

    ldrawColourPartsFile    = QDir::toNativeSeparators(QString("%1/extras/%2")
                                                         .arg(lpubDataPath)
                                                         .arg(validLDrawColorParts));
    ldrawColorFileInfo.setFile(ldrawColourPartsFile);
    if (ldrawColorFileInfo.exists()) {
        Settings.setValue(QString("%1/%2").arg(SETTINGS,"LDrawColourPartsFile"),ldrawColourPartsFile);
    }
}

void Preferences::highlightstepPreferences()
{
    QSettings Settings;
    if (! Settings.contains(QString("%1/%2").arg(SETTINGS,"EnableHighlightStep"))) {
        QVariant eValue(false);
        enableHighlightStep = false;
        Settings.setValue(QString("%1/%2").arg(SETTINGS,"EnableHighlightStep"),eValue);
    } else {
        enableHighlightStep = Settings.value(QString("%1/%2").arg(SETTINGS,"EnableHighlightStep")).toBool();
    }

    if (! Settings.contains(QString("%1/%2").arg(SETTINGS,"HighlightStepColor"))) {
        QVariant cValue(HIGHLIGHT_COLOUR_DEFAULT);
        highlightStepColour = HIGHLIGHT_COLOUR_DEFAULT;
        Settings.setValue(QString("%1/%2").arg(SETTINGS,"HighlightStepColor"),cValue);
    } else {
        highlightStepColour = Settings.value(QString("%1/%2").arg(SETTINGS,"HighlightStepColor")).toString();
    }

    if (! Settings.contains(QString("%1/%2").arg(SETTINGS,"HighlightStepLineWidth"))) {
        QVariant cValue(HIGHLIGHT_LINE_WIDTH_DEFAULT);
        highlightStepLineWidth = HIGHLIGHT_LINE_WIDTH_DEFAULT;
        Settings.setValue(QString("%1/%2").arg(SETTINGS,"HighlightStepLineWidth"),cValue);
    } else {
        highlightStepLineWidth = Settings.value(QString("%1/%2").arg(SETTINGS,"HighlightStepLineWidth")).toInt();
    }
}

void Preferences::exportPreferences()
{
    QSettings Settings;
    if ( ! Settings.contains(QString("%1/%2").arg(DEFAULTS,"IgnoreMixedPageSizesMsg"))) {
        QVariant uValue(false);
        ignoreMixedPageSizesMsg = false;
        Settings.setValue(QString("%1/%2").arg(DEFAULTS,"IgnoreMixedPageSizesMsg"),uValue);
      } else {
        if (Preferences::modeGUI)
          ignoreMixedPageSizesMsg = Settings.value(QString("%1/%2").arg(DEFAULTS,"IgnoreMixedPageSizesMsg")).toBool();
        else
          ignoreMixedPageSizesMsg = false;
      }
}

void Preferences::publishingPreferences()
{
    QSettings Settings;

    //Page Display Pause
    if ( ! Settings.contains(QString("%1/%2").arg(SETTINGS,"PageDisplayPause"))) {
        pageDisplayPause = PAGE_DISPLAY_PAUSE_DEFAULT;
        Settings.setValue(QString("%1/%2").arg(SETTINGS,"PageDisplayPause"),pageDisplayPause);
    } else {
        pageDisplayPause = Settings.value(QString("%1/%2").arg(SETTINGS,"PageDisplayPause")).toInt();
    }

    if ( ! Settings.contains(QString("%1/%2").arg(DEFAULTS,"DoNotShowPageProcessDlg"))) {
        QVariant pValue(false);
        doNotShowPageProcessDlg = false;
        Settings.setValue(QString("%1/%2").arg(DEFAULTS,"DoNotShowPageProcessDlg"),pValue);
    } else {
        if (Preferences::modeGUI)
          doNotShowPageProcessDlg = Settings.value(QString("%1/%2").arg(DEFAULTS,"DoNotShowPageProcessDlg")).toBool();
        else
          doNotShowPageProcessDlg = true;
    }

    if ( ! Settings.contains(QString("%1/%2").arg(DEFAULTS,"DisplayAllAttributes"))) {
        QVariant pValue(false);
        displayAllAttributes = false;
        Settings.setValue(QString("%1/%2").arg(DEFAULTS,"DisplayAllAttributes"),pValue);
    } else {
        displayAllAttributes = Settings.value(QString("%1/%2").arg(DEFAULTS,"DisplayAllAttributes")).toBool();
    }

    if ( ! Settings.contains(QString("%1/%2").arg(DEFAULTS,"GenerateCoverPages"))) {
        QVariant pValue(false);
        generateCoverPages = false;
        Settings.setValue(QString("%1/%2").arg(DEFAULTS,"GenerateCoverPages"),pValue);
    } else {
        generateCoverPages = Settings.value(QString("%1/%2").arg(DEFAULTS,"GenerateCoverPages")).toBool();
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
    if (!povrayExe.isEmpty())
        lcSetProfileString(LC_PROFILE_POVRAY_PATH, povrayExe);
    if (!lgeoPath.isEmpty())
        lcSetProfileString(LC_PROFILE_POVRAY_LGEO_PATH, lgeoPath);
    if (!defaultAuthor.isEmpty())
        lcSetProfileString(LC_PROFILE_DEFAULT_AUTHOR_NAME, defaultAuthor);

    QSettings Settings;
    if (Settings.contains(QString("%1/%2").arg(SETTINGS,"ProjectsPath")))
        lcSetProfileString(LC_PROFILE_PROJECTS_PATH, Settings.value(QString("%1/%2").arg(SETTINGS,"ProjectsPath")).toString());
}

bool Preferences::getPreferences()
{
#ifdef Q_OS_MAC
    if (! lpub3dLoaded && modeGUI && Application::instance()->splash->isVisible())
        Application::instance()->splash->hide();
#endif

    bool updateLDViewConfigFiles   = false;

    PreferencesDialog *dialog      = new PreferencesDialog();

    QSettings Settings;

    if (dialog->exec() == QDialog::Accepted) {

        if (ldrawPath != dialog->ldrawPath()) {
            ldrawPath = dialog->ldrawPath();
            if (ldrawPath.isEmpty()) {
                Settings.remove(QString("%1/%2").arg(SETTINGS,"LDrawDir"));
            } else {
                Settings.setValue(QString("%1/%2").arg(SETTINGS,"LDrawDir"),ldrawPath);
            }
            // update LDView ini files
            updateNativePOVIniFile(UpdateExisting);
            updateLDViewIniFile(UpdateExisting);       //ldraw path changed
            updateLDViewPOVIniFile(UpdateExisting);    //ldraw or lgeo paths changed
            updateLDViewConfigFiles = true;            //set flag to true
        }

        if (altLDConfigPath != dialog->altLDConfigPath())
        {
            altLDConfigPath = dialog->altLDConfigPath();
            if (altLDConfigPath.isEmpty()) {
                Settings.remove(QString("%1/%2").arg(SETTINGS,"AltLDConfigPath"));
            } else {
                Settings.setValue(QString("%1/%2").arg(SETTINGS,"AltLDConfigPath"),altLDConfigPath);
            }
        }

        if (pliFile != dialog->pliFile()) {
            pliFile = dialog->pliFile();
            if (pliFile.isEmpty()) {
                Settings.remove(QString("%1/%2").arg(SETTINGS,"PliControl"));
            } else {
                Settings.setValue(QString("%1/%2").arg(SETTINGS,"PliControl"),pliFile);
            }
        }

        if (preferredRenderer != dialog->preferredRenderer()) {
            preferredRenderer = dialog->preferredRenderer();
            if (preferredRenderer.isEmpty()) {
                Settings.remove(QString("%1/%2").arg(SETTINGS,"PreferredRenderer"));
            } else {
                Settings.setValue(QString("%1/%2").arg(SETTINGS,"PreferredRenderer"),preferredRenderer);
            }
        }

        if (enableLDViewSingleCall != dialog->enableLDViewSingleCall()) {
            enableLDViewSingleCall = dialog->enableLDViewSingleCall();
            Settings.setValue(QString("%1/%2").arg(SETTINGS,"EnableLDViewSingleCall"),enableLDViewSingleCall);
        }

        if (enableLDViewSnaphsotList != dialog->enableLDViewSnaphsotList()) {
            enableLDViewSnaphsotList = dialog->enableLDViewSnaphsotList();
            Settings.setValue(QString("%1/%2").arg(SETTINGS,"EnableLDViewSnapshotsList"),enableLDViewSnaphsotList);
        }

        if (lgeoPath != dialog->lgeoPath()) {
            lgeoPath = dialog->lgeoPath();
            if(lgeoPath.isEmpty()) {
                Settings.remove(QString("%1/%2").arg(POVRAY,"LGEOPath"));
            } else {
                Settings.setValue(QString("%1/%2").arg(POVRAY,"LGEOPath"),lgeoPath);
            }
            // update LDView ini files
            if (!updateLDViewConfigFiles) {
                updateLDViewPOVIniFile(UpdateExisting);    //ldraw or lgeo paths changed
            }
            updatePOVRayConfFile(UpdateExisting);          //lgeo path changed
        }

        if (povFileGenerator != dialog->povFileGenerator())
        {
            povFileGenerator = dialog->povFileGenerator();
            Settings.setValue(QString("%1/%2").arg(SETTINGS,"POVFileGenerator"),povFileGenerator);
        }

        if (povrayDisplay != dialog->povrayDisplay())
        {
            povrayDisplay = dialog->povrayDisplay();
            Settings.setValue(QString("%1/%2").arg(SETTINGS,"POVRayDisplay"),povrayDisplay);
        }

        if (ldSearchDirs != dialog->searchDirSettings()) {
            if (! dialog->searchDirSettings().isEmpty()) {
                ldSearchDirs.clear();
                QStringList excludedEntries;
                QStringList excludedPaths = QStringList() << "unofficial/parts" << "unofficial/p" << "parts" << "p";
                foreach (QString searchDir, dialog->searchDirSettings()) {
                    bool entryHasExcludedPath = false;
                    QString formattedDir = QDir::toNativeSeparators(searchDir);
                    foreach(QString excludedPath, excludedPaths) {
                        QString excludedDir = QDir::toNativeSeparators(QString("%1/%2/").arg(Preferences::ldrawPath).arg(excludedPath));
                        if ((entryHasExcludedPath = (formattedDir.indexOf(excludedDir,0,Qt::CaseInsensitive)) != -1)) {
                            break;
                        }
                    }
                    if (entryHasExcludedPath){
                        excludedEntries << formattedDir;
                    } else {
                        ldSearchDirs << searchDir;
                    }
                }
                if (! excludedEntries.isEmpty()) {
                    QString message =
                    QString("The search directory list contains paths excluded from search."
                            "%1 will not be saved. %2")
                            .arg(excludedEntries.size() > 1 ? "These paths" : "This path")
                            .arg(excludedEntries.join(" "));
                    if (Preferences::modeGUI) {
                        QMessageBox box;
                        box.setMinimumSize(40,20);
                        box.setIcon (QMessageBox::Information);
                        box.setDefaultButton   (QMessageBox::Ok);
                        box.setStandardButtons (QMessageBox::Ok);
                        box.setText (message);
                        box.exec();
                    } else {
                        fprintf(stdout,"%s\n",message.toLatin1().constData());
                        fflush(stdout);
                    }

                }
                if (! ldSearchDirs.isEmpty())
                    Settings.setValue(QString("%1/%2").arg(SETTINGS,"LDSearchDirs"),ldSearchDirs);
                else
                    Settings.remove(QString("%1/%2").arg(SETTINGS,"LDSearchDirs"));
            } else {
                Settings.remove(QString("%1/%2").arg(SETTINGS,"LDSearchDirs"));
            }
            // update LDView ExtraSearchDirs in ini files
            if (!setLDViewExtraSearchDirs(Preferences::ldviewIni))
               logError() << QString("Could not update %1").arg(ldviewIni);
            if (!setLDViewExtraSearchDirs(Preferences::ldviewPOVIni))
               logError() << QString("Could not update %1").arg(ldviewPOVIni);
            if (!setLDViewExtraSearchDirs(Preferences::nativePOVIni))
               logError() << QString("Could not update %1").arg(nativePOVIni);
        }

        if (rendererTimeout != dialog->rendererTimeout()) {
            rendererTimeout = dialog->rendererTimeout();
            Settings.setValue(QString("%1/%2").arg(SETTINGS,"RendererTimeout"),rendererTimeout);
        }

        if (pageDisplayPause != dialog->pageDisplayPause()) {
            pageDisplayPause = dialog->pageDisplayPause();
            Settings.setValue(QString("%1/%2").arg(SETTINGS,"PageDisplayPause"),pageDisplayPause);
        }

        if (documentLogoFile != dialog->documentLogoFile()) {
            documentLogoFile = dialog->documentLogoFile();
            if (documentLogoFile.isEmpty()) {
                Settings.remove(QString("%1/%2").arg(DEFAULTS,"DocumentLogoFile"));
            } else {
                Settings.setValue(QString("%1/%2").arg(DEFAULTS,"DocumentLogoFile"),documentLogoFile);
            }
        }

        if (defaultAuthor != dialog->defaultAuthor()) {
            defaultAuthor = dialog->defaultAuthor();
            if (defaultAuthor.isEmpty()) {
                Settings.remove(QString("%1/%2").arg(DEFAULTS,"Author"));
            } else {
                Settings.setValue(QString("%1/%2").arg(DEFAULTS,"Author"),defaultAuthor);
            }
        }

        if (defaultURL != dialog->defaultURL()) {
            defaultURL = dialog->defaultURL();
            if (!defaultURL.isEmpty()) {
                Settings.remove(QString("%1/%2").arg(DEFAULTS,"URL"));
            } else {
                Settings.setValue(QString("%1/%2").arg(DEFAULTS,"URL"),defaultURL);
            }
        }

        if (defaultEmail != dialog->defaultEmail()) {
            defaultEmail = dialog->defaultEmail();
            if (defaultEmail.isEmpty()) {
                Settings.remove(QString("%1/%2").arg(DEFAULTS,"Email"));
            } else {
                Settings.setValue(QString("%1/%2").arg(DEFAULTS,"Email"),defaultEmail);
            }
        }

        if (publishDescription != dialog->publishDescription()) {
            publishDescription = dialog->publishDescription();
            if (publishDescription.isEmpty()) {
                Settings.remove(QString("%1/%2").arg(DEFAULTS,"PublishDescription"));
            } else {
                Settings.setValue(QString("%1/%2").arg(DEFAULTS,"PublishDescription"),publishDescription);
            }
        }

        if (enableFadeSteps != dialog->enableFadeSteps())
        {
            enableFadeSteps = dialog->enableFadeSteps();
            Settings.setValue(QString("%1/%2").arg(SETTINGS,"EnableFadeSteps"),enableFadeSteps);
        }

        if (showParseErrors != dialog->showParseErrors())
        {
            showParseErrors = dialog->showParseErrors();
            Settings.setValue(QString("%1/%2").arg(SETTINGS,"ShowParseErrors"),showParseErrors);
        }

        if (fadeStepsOpacity != dialog->fadeStepsOpacity())
        {
            fadeStepsOpacity = dialog->fadeStepsOpacity();
            Settings.setValue(QString("%1/%2").arg(SETTINGS,"FadeStepsOpacity"),fadeStepsOpacity);
        }

        if (fadeStepsUseColour != dialog->fadeStepsUseColour())
        {
            fadeStepsUseColour = dialog->fadeStepsUseColour();
            Settings.setValue(QString("%1/%2").arg(SETTINGS,"FadeStepsUseColour"),fadeStepsUseColour);
        }

        if (fadeStepsColour != dialog->fadeStepsColour())
        {
            if (dialog->fadeStepsColour().isEmpty())
                fadeStepsColour = FADE_COLOUR_DEFAULT;
            else
                fadeStepsColour = dialog->fadeStepsColour();
            Settings.setValue(QString("%1/%2").arg(SETTINGS,"FadeStepColor"),fadeStepsColour);
        }

        if (highlightStepColour != dialog->highlightStepColour())
        {
            highlightStepColour = dialog->highlightStepColour();
            Settings.setValue(QString("%1/%2").arg(SETTINGS,"HighlightStepColor"),highlightStepColour);
        }

        if (enableHighlightStep != dialog->enableHighlightStep())
        {
            enableHighlightStep = dialog->enableHighlightStep();
            Settings.setValue(QString("%1/%2").arg(SETTINGS,"EnableHighlightStep"),enableHighlightStep);
        }

        if (enableHighlightStep && (highlightStepLineWidth != dialog->highlightStepLineWidth()))
        {
            highlightStepLineWidth = dialog->highlightStepLineWidth();
            Settings.setValue(QString("%1/%2").arg(SETTINGS,"HighlightStepLineWidth"),highlightStepLineWidth);
        }

        if (enableImageMatting != dialog->enableImageMatting())
        {
            enableImageMatting = dialog->enableImageMatting();
            Settings.setValue(QString("%1/%2").arg(SETTINGS,"EnableImageMatting"),enableImageMatting);
            if (enableImageMatting)
                updateLDViewIniFile(UpdateExisting);       // strip AutoCrop
        }

        if (preferCentimeters != dialog->centimeters())
        {
            preferCentimeters = dialog->centimeters();
            Settings.setValue(QString("%1/%2").arg(SETTINGS,"Centimeters"),preferCentimeters);
            defaultResolutionType(preferCentimeters);
        }

        if (applyCALocally != dialog->applyCALocally())
        {
            applyCALocally = dialog->applyCALocally();
            Settings.setValue(QString("%1/%2").arg(SETTINGS,"ApplyCALocally"),applyCALocally);
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
            Settings.setValue(QString("%1/%2").arg(DEFAULTS,"GenerateCoverPages"),generateCoverPages);
        }

        if (printDocumentTOC != dialog->printDocumentTOC()) {
            printDocumentTOC = dialog->printDocumentTOC();
            Settings.setValue(QString("%1/%2").arg(DEFAULTS,"PrintDocumentTOC"),printDocumentTOC);
        }

        if (doNotShowPageProcessDlg != dialog->doNotShowPageProcessDlg()) {
            doNotShowPageProcessDlg = dialog->doNotShowPageProcessDlg();
            Settings.setValue(QString("%1/%2").arg(DEFAULTS,"DoNotShowPageProcessDlg"),doNotShowPageProcessDlg);
        }

        if (displayTheme != dialog->displayTheme()){
            displayTheme = dialog->displayTheme();
            Settings.setValue(QString("%1/%2").arg(SETTINGS,"DisplayTheme"),displayTheme);
        }

        if (themeAutoRestart != dialog->themeAutoRestart())
        {
            themeAutoRestart = dialog->themeAutoRestart();
            Settings.setValue(QString("%1/%2").arg(SETTINGS,"ThemeAutoRestart"),themeAutoRestart);
        }

        if (moduleVersion != dialog->moduleVersion()){
            moduleVersion = dialog->moduleVersion();
        }

        if (includeLogLevel != dialog->includeLogLevel())
        {
            includeLogLevel = dialog->includeLogLevel();
            Settings.setValue(QString("%1/%2").arg(LOGGING,"IncludeLogLevel"),includeLogLevel);
        }

        if (includeTimestamp != dialog->includeTimestamp())
        {
            includeTimestamp = dialog->includeTimestamp();
            Settings.setValue(QString("%1/%2").arg(LOGGING,"IncludeTimestamp"),includeTimestamp);
        }

        if (includeLineNumber != dialog->includeLineNumber())
        {
            includeLineNumber = dialog->includeLineNumber();
            Settings.setValue(QString("%1/%2").arg(LOGGING,"IncludeLineNumber"),includeLineNumber);
        }

        if (includeFileName != dialog->includeFileName())
        {
            includeFileName = dialog->includeFileName();
            Settings.setValue(QString("%1/%2").arg(LOGGING,"IncludeFileName"),includeFileName);
        }

        if (includeAllLogAttributes != dialog->includeAllLogAttrib())
        {
            includeAllLogAttributes = dialog->includeAllLogAttrib();
            Settings.setValue(QString("%1/%2").arg(LOGGING,"IncludeAllLogAttributes"),includeAllLogAttributes);
        }

        if (logging != dialog->loggingGrpBox())
        {
            logging = dialog->loggingGrpBox();
            Settings.setValue(QString("%1/%2").arg(LOGGING,"Logging"),logging);
        }

        if (loggingLevel != dialog->logLevelCombo())
        {
            loggingLevel = dialog->logLevelCombo();
            Settings.setValue(QString("%1/%2").arg(LOGGING,"LoggingLevel"),loggingLevel);
        }

        if (logLevel != dialog->logLevelGrpBox())
        {
            logLevel = dialog->logLevelGrpBox();
            Settings.setValue(QString("%1/%2").arg(LOGGING,"LogLevel"),logLevel);
        }

        if (logLevels != dialog->logLevelsGrpBox())
        {
            logLevels = dialog->logLevelsGrpBox();
            Settings.setValue(QString("%1/%2").arg(LOGGING,"LogLevels"),logLevels);
        }

        if (debugLevel != dialog->debugLevel())
        {
            debugLevel = dialog->debugLevel();
            Settings.setValue(QString("%1/%2").arg(LOGGING,"DebugLevel"),debugLevel);
        }

        if (traceLevel != dialog->traceLevel())
        {
            traceLevel = dialog->traceLevel();
            Settings.setValue(QString("%1/%2").arg(LOGGING,"TraceLevel"),traceLevel);
        }

        if (noticeLevel != dialog->noticeLevel())
        {
            noticeLevel = dialog->noticeLevel();
            Settings.setValue(QString("%1/%2").arg(LOGGING,"NoticeLevel"),noticeLevel);
        }

        if (infoLevel != dialog->infoLevel())
        {
            infoLevel = dialog->infoLevel();
            Settings.setValue(QString("%1/%2").arg(LOGGING,"InfoLevel"),infoLevel);
        }

        if (statusLevel != dialog->statusLevel())
        {
            statusLevel = dialog->statusLevel();
            Settings.setValue(QString("%1/%2").arg(LOGGING,"StatusLevel"),statusLevel);
        }

        if (errorLevel != dialog->errorLevel())
        {
            errorLevel = dialog->errorLevel();
            Settings.setValue(QString("%1/%2").arg(LOGGING,"ErrorLevel"),errorLevel);
        }

        if (fatalLevel != dialog->fatalLevel())
        {
            fatalLevel = dialog->fatalLevel();
            Settings.setValue(QString("%1/%2").arg(LOGGING,"FatalLevel"),fatalLevel);
        }

        if (allLogLevels != dialog->allLogLevels())
        {
            allLogLevels = dialog->allLogLevels();
            Settings.setValue(QString("%1/%2").arg(LOGGING,"AllLogLevels"),allLogLevels);
        }

        return true;
    } else {
        return false;
    }
#ifdef Q_OS_MAC
    if (! lpub3dLoaded && modeGUI && Application::instance()->splash->isHidden())
        Application::instance()->splash->show();
#endif
}

void Preferences::getRequireds()
{
    // this call will most likely not get past the preferredRenderer == "" statement as
    // the preferred renderer is set on application launch before getRequireds() is called.
    if (preferredRenderer == "" && ! getPreferences()) {
        exit (-1);
    }
}

void Preferences::setLPub3DLoaded(){
    lpub3dLoaded = true;
}

/*
 * Set the LDView extra search directories. This function is called on the following occasions
 * - LPub3D Initialize: Gui::initialize() -> Preferences::setLDViewExtraSearchDirs(...)
 * - Preference Dialogue if search directories change: Preferences::getPreferences()
 * - Archiving Custom Color parts where Custom dirs were not in search dirs list: PartWorker::processCustomColourParts()
 */
bool Preferences::setLDViewExtraSearchDirs(const QString &iniFile) {
    bool retVal = true;
    QFile confFile(iniFile);
    QStringList contentList;
    if (Preferences::preferredRenderer == RENDERER_LDVIEW)
      logInfo() << QString("Updating ExtraSearchDirs in %1").arg(iniFile);
    if (confFile.open(QIODevice::ReadOnly))
    {
        bool foundExtraSearchDirs = false;
        bool inExtraSearchDirsSection = false;
        QTextStream input(&confFile);
        while (!input.atEnd())
        {
            QString line = input.readLine();
            if (line.left(17) == "[ExtraSearchDirs]") {
                foundExtraSearchDirs = true;
                inExtraSearchDirsSection = true;
                if (!contentList.contains(line,Qt::CaseSensitivity::CaseInsensitive))
                    contentList += line;
            } else if (inExtraSearchDirsSection) {                  // in ExtraSearchDirs section
                int dirNum = 0;
                QString nativePath;
                if (line.left(1) == "[" || line.isEmpty()) {        // at next section or empty line, insert search dirs
                    foreach (QString searchDir, ldSearchDirs) {
                       dirNum++;
                       if (dirNum <= ldSearchDirs.count()) {
#ifdef Q_OS_WIN
                          nativePath = searchDir.replace("\\","\\\\");
#else
                          nativePath = QDir::toNativeSeparators(searchDir);
#endif
                          if (!contentList.contains(nativePath, Qt::CaseSensitivity::CaseInsensitive)) {
                              QString formattedSearchDir = QString("Dir%1=%2").arg(dirNum, 3, 10, QChar('0')).arg(nativePath);
                              contentList += formattedSearchDir;
                              if (Preferences::preferredRenderer == RENDERER_LDVIEW)
                                  logInfo() << QString("ExtraSearchDirs OUT: %1").arg(formattedSearchDir);
                          }
                       }
                    }
                    if ( !line.isEmpty())
                        contentList += line;
                    inExtraSearchDirsSection = false;
                }
            } else if (!contentList.contains(line,Qt::CaseSensitivity::CaseInsensitive) && !line.isEmpty()) {
                contentList += line;
            }
        }  // atEnd
        confFile.close();
        if (!foundExtraSearchDirs) {
            logError() << QString("Did not find [ExtraSearchDirs] section in %1 "
                                  "The inf file %1 may be malformed or corrupt.").arg(confFile.fileName());
            retVal = false;
         }
    } else {
        QString confFileError;
        if (!confFile.errorString().isEmpty())
            confFileError.append(QString(" confFileInError: %1").arg(confFile.errorString()));
        logError() << QString("Could not open input: %1").arg(confFileError);
        retVal = false;
    }
    // write search dir to ini files
    if (confFile.open(QIODevice::WriteOnly))
    {
        QTextStream output(&confFile);
        foreach (QString line, contentList) {
           output << line << endl;
        }
        confFile.flush();
        confFile.close();
    } else {
        QString confFileError;
        if (!confFile.errorString().isEmpty())
            confFileError.append(QString(" confFileOutError: %1").arg(confFile.errorString()));
        logError() << QString("Could not open input or output file: %1").arg(confFileError);
        retVal = false;
    }
    return retVal;
}

bool Preferences::extractLDrawLib() {

#ifdef Q_OS_MAC
    if (! lpub3dLoaded && modeGUI && Application::instance()->splash->isHidden())
        Application::instance()->splash->show();
#endif

    QString const ldrawKey("LDrawDir");
    QSettings Settings;
    QFileInfo validFile;
    QString message;
    bool r = true;

    message = QMessageBox::tr("Extracting %1 LDraw library, please wait...").arg(ldrawLibrary);

    emit Application::instance()->splashMsgSig(message.prepend("10% - "));

    if (!modeGUI) {
      fprintf(stdout,"%s\n",message.toLatin1().constData());
      fflush(stdout);
    }

    // if ldraw directory path is empty use the default location (datapath)
    if (ldrawPath.isEmpty()) {
        ldrawPath = QString("%1/%2/").arg(lpubDataPath, validLDrawDir);
    }

    // set ldraw parent directory to extract archive.zip
    bool parentDirNotValid = false;
    QDir ldrawDir(ldrawPath);
    //logInfo() << QString("LDraw directory: %1").arg(ldrawDir.absolutePath()));
    logDebug() << QString("LDraw directory: %1").arg(ldrawDir.absolutePath());
    if (ldrawDir.dirName().toLower() != validLDrawDir.toLower())
        parentDirNotValid = true;
    if (!ldrawDir.isRoot())
        ldrawDir.cdUp();            // ldraw path parent directory
    //logInfo() << QString("LDraw parent directory (%1), AbsPath (%2)").arg(ldrawDir.dirName()).arg(ldrawDir.absolutePath())));

    // set the archive library path
    QDir libraryDir(QString("%1/%2").arg(lpubDataPath, "libraries"));
    validFile.setFile(QDir::toNativeSeparators(QString("%1/%2").arg(libraryDir.absolutePath(), validLDrawArchive)));

    // archive library exist so let's proceed...
    if (validFile.exists()) {

        // extract archive.zip
        QString destination = ldrawDir.absolutePath();
        QStringList result = JlCompress::extractDir(validFile.absoluteFilePath(),destination);
        if (result.isEmpty()){
            logError() << QString("Failed to extract %1 to %2/%3").arg(validFile.absoluteFilePath()).arg(destination).arg(validLDrawDir);
            r = false;
         } else {
            if (! usingDefaultLibrary) {
                // Rename extracted library (defaults to ldraw)
                QFile library(QString("%1/%2").arg(destination).arg(validLDrawDir));
                QFile extract(QString("%1/%2").arg(destination).arg(QString(LDRAWDIR_STR).toLower()));

                if (! library.exists() || library.remove()) {
                    if (! extract.rename(library.fileName())) {
                        logError() << QString("Failed to rename %1 to %2").arg(extract.fileName()).arg(library.fileName());
                    } else {
                        message = QMessageBox::tr("%1 %2 Library files extracted to %3/%4")
                                .arg(result.size()).arg(ldrawLibrary).arg(destination).arg(validLDrawDir);
                        logInfo() << QString(message);
                    }
                }  else {
                    message = QMessageBox::tr("Could not remove old library %1").arg(library.remove());
                    logError() << QString(message);
                }
            } else {
             message = QMessageBox::tr("%1 Official Library files extracted to %2/%3").arg(result.size()).arg(destination).arg(validLDrawDir);
             logInfo() << QString(message);
            }
         }

        // extract lpub3dldrawunf.zip - for LEGO library only
        if (usingDefaultLibrary) {
            validFile.setFile(QDir::toNativeSeparators(QString("%1/%2").arg(libraryDir.absolutePath(), VER_LPUB3D_UNOFFICIAL_ARCHIVE)));
            if (validFile.exists()) {
                QString destination = QString("%1/unofficial").arg(Preferences::ldrawPath);
                QStringList result = JlCompress::extractDir(validFile.absoluteFilePath(),destination);
                if (result.isEmpty()){
                    logError() << QString("Failed to extract %1 to %2").arg(validFile.absoluteFilePath()).arg(destination);
                } else {
                    message = QMessageBox::tr("%1 Unofficial Library files extracted to %2").arg(result.size()).arg(destination);
                    logInfo() << QString(message);
                }
            } else {
                message = QMessageBox::tr ("Unofficial Library archive file %1 does not exist.").arg(validFile.absoluteFilePath());
                logError() << QString(message);
            }
        }

        // copy extracted contents to ldraw directory and delete extract dir if needed
        if (parentDirNotValid) {
            QDir extractDir(QString("%1/%2").arg(ldrawDir.absolutePath()).arg(validLDrawDir));
            if (!copyRecursively(extractDir.absolutePath(),ldrawPath)) {
                message = QMessageBox::tr("Unable to copy %1 to %2").arg(extractDir.absolutePath(),ldrawPath);
                logInfo() << QString(message);
                r = false;
            }
            extractDir.removeRecursively();
        }

        // if no errors, (re)set ldrawPath in registry
        if (r)
            Settings.setValue(QString("%1/%2").arg(SETTINGS, ldrawKey), ldrawPath);

    } else {

        QString body;
        if (usingDefaultLibrary)
            validFile.setFile(QDir::toNativeSeparators(QString("%1/%2").arg(libraryDir.absolutePath(), VER_LDRAW_UNOFFICIAL_ARCHIVE)));
        if (usingDefaultLibrary && !validFile.exists()) {
            body = QMessageBox::tr ("LPub3D attempted to extract the LDraw library however the required archive files\n%1\n%2\ndoes not exist.\n").arg(lpub3dLibFile, validFile.absoluteFilePath());
        } else {
            body = QMessageBox::tr ("LPub3D attempted to extract the LDraw library however the required archive file\n%1\ndoes not exist.\n").arg(lpub3dLibFile);
        }

        if (modeGUI) {
            QPixmap _icon = QPixmap(":/icons/lpub96.png");
            QMessageBox box;
            box.setWindowIcon(QIcon());
            box.setIconPixmap (_icon);
            box.setTextFormat (Qt::RichText);
            box.setWindowTitle(QMessageBox::tr ("LDraw Library"));
            box.setWindowFlags (Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
            QString header = "<b> " + QMessageBox::tr ("LDraw library archive not found!") + "</b>";
            box.setStandardButtons (QMessageBox::Close);
            box.setText(header);
            box.setInformativeText(body);
            box.exec();
        } else {
            fprintf(stderr,"%s",body.toLatin1().constData());
        }

        r = false;

        // remove registry setting and clear ldrawPath
        validFile.setFile(QDir::toNativeSeparators(QString("%1%2").arg(ldrawPath).arg(validLDrawPart)));
        if (!validFile.exists()) {
            Settings.remove(QString("%1/%2").arg(SETTINGS, ldrawKey));
            ldrawPath.clear();
        }

    }

    return r;
}


bool Preferences::copyRecursively(const QString &srcFilePath,
                            const QString &tgtFilePath)
{
    QFileInfo srcFileInfo(srcFilePath);
    if (srcFileInfo.isDir()) {
        QDir targetDir(tgtFilePath);
        targetDir.cdUp();
        if (!targetDir.mkdir(QFileInfo(tgtFilePath).fileName()))
            return false;
        QDir sourceDir(srcFilePath);
        QStringList fileNames = sourceDir.entryList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot | QDir::Hidden | QDir::System);
        foreach (const QString &fileName, fileNames) {
            const QString newSrcFilePath
                    = srcFilePath + QLatin1Char('/') + fileName;
            const QString newTgtFilePath
                    = tgtFilePath + QLatin1Char('/') + fileName;
            if (!copyRecursively(newSrcFilePath, newTgtFilePath))
                return false;
        }
    } else {
        if (!QFile::copy(srcFilePath, tgtFilePath))
            return false;
    }
    return true;
}

void Preferences::nativePovGenPreferences()
{

    emit Application::instance()->splashMsgSig("25% - NativePOV settings...");

    QSettings Settings;

    ldvLights = LIGHTS_COMBO_DEFAULT;
    if ( ! Settings.contains(QString("%1/%2").arg(POVRAY,"LDVLights"))) {
        Settings.setValue(QString("%1/%2").arg(POVRAY,"LDVLights"),ldvLights);
    } else {
        ldvLights = Settings.value(QString("%1/%2").arg(POVRAY,"LDVLights")).toString();
        if (ldvLights.isEmpty()) {
            ldvLights = LIGHTS_COMBO_DEFAULT;
            Settings.setValue(QString("%1/%2").arg(POVRAY,"LDVLights"),ldvLights);
        }
    }
}
