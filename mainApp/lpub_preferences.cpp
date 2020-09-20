/****************************************************************************
**
** Copyright (C) 2007-2009 Kevin Clague. All rights reserved.
** Copyright (C) 2015 - 2019 Trevor SANDY. All rights reserved.
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
#include <LDVQt/LDVWidget.h>
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
QString Preferences::ldrawLibPath               = EMPTY_STRING_DEFAULT;
QString Preferences::altLDConfigPath            = EMPTY_STRING_DEFAULT;
QString Preferences::lpub3dLibFile              = EMPTY_STRING_DEFAULT;
QString Preferences::lgeoPath;
QString Preferences::lpub3dPath                 = DOT_PATH_DEFAULT;
QString Preferences::lpub3dExtrasResourcePath   = DOT_PATH_DEFAULT;
QString Preferences::lpub3dDocsResourcePath     = DOT_PATH_DEFAULT;
QString Preferences::lpub3d3rdPartyConfigDir    = DOT_PATH_DEFAULT;
QString Preferences::lpub3d3rdPartyAppDir       = DOT_PATH_DEFAULT;
QString Preferences::lpub3dLDVConfigDir         = DOT_PATH_DEFAULT;
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
QString Preferences::nativeExportIni;
QString Preferences::povrayIniPath;
QString Preferences::povrayIncPath;
QString Preferences::povrayScenePath;
QString Preferences::povrayExe;
QString Preferences::preferredRenderer;
QString Preferences::highlightStepColour        = HIGHLIGHT_COLOUR_DEFAULT;
QString Preferences::ldrawiniFile;
QString Preferences::moduleVersion              = qApp->applicationVersion();
QString Preferences::availableVersions;
QString Preferences::ldgliteSearchDirs;
QString Preferences::loggingLevel               = LOGGING_LEVEL_DEFAULT;
QString Preferences::logPath;
QString Preferences::dataLocation;
QString Preferences::povFileGenerator           = RENDERER_LDVIEW;

QString Preferences::pliControlFile;
QString Preferences::titleAnnotationsFile;
QString Preferences::freeformAnnotationsFile;
QString Preferences::annotationStyleFile;
QString Preferences::pliSubstitutePartsFile;
QString Preferences::excludedPartsFile;
QString Preferences::ldrawColourPartsFile;

QStringList Preferences::ldSearchDirs;
QStringList Preferences::ldgliteParms;

//Dynamic page attributes
QString Preferences::defaultAuthor;
QString Preferences::publishDescription;
QString Preferences::defaultURL;
QString Preferences::defaultEmail;
QString Preferences::documentLogoFile;
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

QString Preferences::validLDrawLibrary          = LEGO_LIBRARY;            // the currently loaded library
QString Preferences::validLDrawLibraryChange    = LEGO_LIBRARY;            // the result of a library test - initialized to the currently loaded library
QString Preferences::validFadeStepsColour       = LEGO_FADE_COLOUR_DEFAULT;
QString Preferences::validLDrawDir              = VER_LEGO_LDRAWDIR_STR;
QString Preferences::validLDrawPart             = VER_LEGO_LDRAWPART_STR;
QString Preferences::validLDrawPartsArchive     = VER_LDRAW_OFFICIAL_ARCHIVE;
QString Preferences::validLDrawCustomArchive    = VER_LPUB3D_UNOFFICIAL_ARCHIVE;
QString Preferences::validLDrawColorParts       = VER_LPUB3D_LEGO_COLOR_PARTS;
QString Preferences::validLDrawPartsLibrary     = LEGO_LIBRARY "® Parts";

QString Preferences::validPliControl            = VER_LEGO_PLI_CONTROL_FILE;
QString Preferences::validTitleAnnotations      = VER_LEGO_TITLE_ANNOTATIONS_FILE;
QString Preferences::validFreeFormAnnotations   = VER_LEGO_FREEFROM_ANNOTATIONS_FILE;
QString Preferences::validAnnotationStyleFile   = VER_LEGO_ANNOTATION_STYLE_FILE;
QString Preferences::validPliSubstituteParts    = VER_LEGO_PLI_SUBSTITUTE_FILE;
QString Preferences::validExcludedPliParts      = VER_LEGO_PLI_EXCLUDED_FILE;

QString Preferences::fadeStepsColourKey         = LEGO_FADE_COLOUR_KEY;
QString Preferences::ldrawSearchDirsKey         = LEGO_SEARCH_DIR_KEY;
QString Preferences::ldrawLibPathKey            = LEGO_LDRAW_LIB_PATH_KEY;

QString Preferences::blCodesFile                = VER_LPUB3D_BLCODES_FILE;
QString Preferences::legoElementsFile           = VER_LPUB3D_LEGOELEMENTS_FILE;
QString Preferences::blColorsFile               = VER_LPUB3D_BLCOLORS_FILE;
QString Preferences::ld2blColorsXRefFile        = VER_LPUB3D_LD2BLCOLORSXREF_FILE;
QString Preferences::ld2blCodesXRefFile         = VER_LPUB3D_LD2BLCODESXREF_FILE;
QString Preferences::ld2rbColorsXRefFile        = VER_LPUB3D_LD2RBCOLORSXREF_FILE;
QString Preferences::ld2rbCodesXRefFile         = VER_LPUB3D_LD2RBCODESXREF_FILE;

QString Preferences::sceneBackgroundColor       = THEME_SCENE_BGCOLOR_DEFAULT;
QString Preferences::sceneGridColor             = THEME_GRID_PEN_DEFAULT;
QString Preferences::sceneRulerTickColor        = THEME_RULER_TICK_PEN_DEFAULT;
QString Preferences::sceneRulerTrackingColor    = THEME_RULER_TRACK_PEN_DEFAULT;
QString Preferences::sceneGuideColor            = THEME_GUIDE_PEN_DEFAULT;

bool    Preferences::usingDefaultLibrary        = true;
bool    Preferences::portableDistribution       = false;
bool    Preferences::perspectiveProjection      = false;
bool    Preferences::saveOnRedraw               = true;
bool    Preferences::saveOnUpdate               = true;

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
bool    Preferences::addLSynthSearchDir         = false;
bool    Preferences::archiveLSynthParts         = false;
bool    Preferences::usingNativeRenderer        = false;
bool    Preferences::skipPartsArchive           = false;
bool    Preferences::loadLastOpenedFile         = false;
bool    Preferences::extendedSubfileSearch      = false;

bool    Preferences::pdfPageImage               = false;
bool    Preferences::ignoreMixedPageSizesMsg    = false;

bool    Preferences::debugLevel                 = false;
bool    Preferences::traceLevel                 = false;
bool    Preferences::noticeLevel                = false;
bool    Preferences::infoLevel                  = false;
bool    Preferences::statusLevel                = false;
bool    Preferences::errorLevel                 = false;
bool    Preferences::fatalLevel                 = false;

bool    Preferences::showSubmodels              = false;
bool    Preferences::showTopModel               = false;
bool    Preferences::showInstanceCount          = false;

bool    Preferences::includeAllLogAttributes    = false;
bool    Preferences::allLogLevels               = false;

bool    Preferences::logLevel                   = false;   // logging level (combo box)
bool    Preferences::logging                    = false;   // logging on/off offLevel (grp box)
bool    Preferences::logLevels                  = false;   // individual logging levels (grp box)

bool    Preferences::applyCALocally             = true;
bool    Preferences::preferCentimeters          = false;   // default is false, to use DPI
bool    Preferences::showAllNotifications       = true;
bool    Preferences::showUpdateNotifications    = true;
bool    Preferences::enableDownloader           = true;
bool    Preferences::showDownloadRedirects      = false;
bool    Preferences::ldrawiniFound              = false;
bool    Preferences::povrayDisplay              = false;
bool    Preferences::povrayAutoCrop             = false;
bool    Preferences::isAppImagePayload          = false;
bool    Preferences::modeGUI                    = true;

bool    Preferences::enableFadeSteps            = false;
bool    Preferences::fadeStepsUseColour         = false;
bool    Preferences::enableHighlightStep        = false;
bool    Preferences::enableImageMatting         = false;

bool    Preferences::sceneRuler                 = false;
bool    Preferences::sceneGuides                = false;
bool    Preferences::snapToGrid                 = false;
bool    Preferences::hidePageBackground         = false;
bool    Preferences::showGuidesCoordinates      = false;
bool    Preferences::showTrackingCoordinates    = false;
bool    Preferences::showParseErrors            = true;
bool    Preferences::showAnnotationMessages     = true;
bool    Preferences::showSaveOnRedraw           = true;
bool    Preferences::showSaveOnUpdate           = true;
bool    Preferences::suppressStdOutToLog        = false;
bool    Preferences::highlightFirstStep         = false;

bool    Preferences::customSceneBackgroundColor = false;
bool    Preferences::customSceneGridColor       = false;
bool    Preferences::customSceneRulerTickColor  = false;
bool    Preferences::customSceneRulerTrackingColor = false;
bool    Preferences::customSceneGuideColor      = false;
bool    Preferences::debugLogging               = false;

#ifdef Q_OS_MAC
bool    Preferences::missingRendererLibs        = false;
#endif

int     Preferences::ldrawFilesLoadMsgs         = NEVER_SHOW;
int     Preferences::sceneRulerTracking         = TRACKING_NONE;
int     Preferences::sceneGuidesPosition        = GUIDES_TOP_LEFT;
int     Preferences::sceneGuidesLine            = SCENE_GUIDES_LINE_DEFAULT;
int     Preferences::povrayRenderQuality        = POVRAY_RENDER_QUALITY_DEFAULT;
int     Preferences::fadeStepsOpacity           = FADE_OPACITY_DEFAULT;              //Default = 50 percent (half opacity)
int     Preferences::highlightStepLineWidth     = HIGHLIGHT_LINE_WIDTH_DEFAULT;      //Default = 1

int     Preferences::checkUpdateFrequency       = UPDATE_CHECK_FREQUENCY_DEFAULT;    //0=Never,1=Daily,2=Weekly,3=Monthly

int     Preferences::gridSizeIndex              = GRID_SIZE_INDEX_DEFAULT;
int     Preferences::pageHeight                 = PAGE_HEIGHT_DEFAULT;
int     Preferences::pageWidth                  = PAGE_WIDTH_DEFAULT;
int     Preferences::rendererTimeout            = RENDERER_TIMEOUT_DEFAULT;          // measured in seconds
int     Preferences::pageDisplayPause           = PAGE_DISPLAY_PAUSE_DEFAULT;        // measured in seconds
int     Preferences::cameraDistFactorNative     = CAMERA_DISTANCE_FACTOR_NATIVE_DEFAULT;

// Native POV file generation settings
QString Preferences::xmlMapPath                 = EMPTY_STRING_DEFAULT;

/*
 * [DATA PATHS]
 * dataLocation            - the data location at install
 * lpubDataPath            - the application user data location after install
 * lpub3d3rdPartyConfigDir - 3rdParty folder at application user data location
 * lpub3d3rdPartyAppDir    - 3rdParty folder at install location
 * lpubExtrasPath          - not used
 */

Preferences::Preferences()
{
}

void Preferences::setStdOutToLogPreference(bool option)
{
    suppressStdOutToLog = option;
}

bool Preferences::checkLDrawLibrary(const QString &libPath) {

    QStringList validLDrawLibs = QStringList() << LEGO_LIBRARY << TENTE_LIBRARY << VEXIQ_LIBRARY;
    QStringList validLDrawParts = QStringList() << VER_LEGO_LDRAWPART_STR << VER_TENTE_LDRAWPART_STR << VER_VEXIQ_LDRAWPART_STR;

    for ( int i = 0; i < NumLibs; i++ )
    {
       if (QFileInfo(QString("%1%2").arg(libPath).arg(validLDrawParts[i])).exists()) {
           validLDrawLibraryChange = validLDrawLibs[i];
           return true;
       }
    }
    return false;
}

void Preferences::setLPub3DAltLibPreferences(const QString &library)
{
    QSettings Settings;
    if (! library.isEmpty()) {
        validLDrawLibrary = library;
        Settings.setValue(QString("%1/%2").arg(SETTINGS,"LDrawLibrary"),validLDrawLibrary);
    } else {
        if ( ! Settings.contains(QString("%1/%2").arg(SETTINGS,"LDrawLibrary"))) {
            Settings.setValue(QString("%1/%2").arg(SETTINGS,"LDrawLibrary"),validLDrawLibrary);
        } else {
            validLDrawLibrary = Settings.value(QString("%1/%2").arg(SETTINGS,"LDrawLibrary")).toString();
        }
    }

    usingDefaultLibrary          = validLDrawLibrary ==  LEGO_LIBRARY;

    if (validLDrawLibrary ==  LEGO_LIBRARY) {
        validFadeStepsColour     = LEGO_FADE_COLOUR_DEFAULT;
        validLDrawPartsLibrary   = LEGO_LIBRARY "® Parts";
        validLDrawLibraryChange  = LEGO_LIBRARY;
        validLDrawDir            = VER_LEGO_LDRAWDIR_STR;
        validLDrawPart           = VER_LEGO_LDRAWPART_STR;
        validLDrawPartsArchive   = VER_LDRAW_OFFICIAL_ARCHIVE;
        validLDrawColorParts     = VER_LPUB3D_LEGO_COLOR_PARTS;
        validLDrawCustomArchive  = VER_LPUB3D_UNOFFICIAL_ARCHIVE;
        validPliControl          = VER_LEGO_PLI_CONTROL_FILE;
        validTitleAnnotations    = VER_LEGO_TITLE_ANNOTATIONS_FILE;
        validFreeFormAnnotations = VER_LEGO_FREEFROM_ANNOTATIONS_FILE;
        validAnnotationStyleFile = VER_LEGO_ANNOTATION_STYLE_FILE;
        validPliSubstituteParts  = VER_LEGO_PLI_SUBSTITUTE_FILE;
        validExcludedPliParts    = VER_LEGO_PLI_EXCLUDED_FILE;

        fadeStepsColourKey       = LEGO_FADE_COLOUR_KEY;
        ldrawSearchDirsKey       = LEGO_SEARCH_DIR_KEY;
        ldrawLibPathKey          = LEGO_LDRAW_LIB_PATH_KEY;
    }
    else
    if (validLDrawLibrary == TENTE_LIBRARY) {
        validFadeStepsColour     = TENTE_FADE_COLOUR_DEFAULT;
        validLDrawPartsLibrary   = TENTE_LIBRARY "® Construction Parts";
        validLDrawLibraryChange  = TENTE_LIBRARY;
        validLDrawDir            = VER_TENTE_LDRAWDIR_STR;
        validLDrawPart           = VER_TENTE_LDRAWPART_STR;
        validLDrawPartsArchive   = VER_LPUB3D_TENTE_ARCHIVE;
        validLDrawColorParts     = VER_LPUB3D_TENTE_COLOR_PARTS;
        validLDrawCustomArchive  = VER_LPUB3D_TENTE_CUSTOM_ARCHIVE;
        validPliControl          = VER_TENTE_PLI_CONTROL_FILE;
        validTitleAnnotations    = VER_TENTE_TITLE_ANNOTATIONS_FILE;
        validFreeFormAnnotations = VER_TENTE_FREEFROM_ANNOTATIONS_FILE;
        validAnnotationStyleFile = VER_TENTE_ANNOTATION_STYLE_FILE;
        validPliSubstituteParts  = VER_TENTE_PLI_SUBSTITUTE_FILE;
        validExcludedPliParts    = VER_TENTE_PLI_EXCLUDED_FILE;

        fadeStepsColourKey       = TENTE_FADE_COLOUR_KEY;
        ldrawSearchDirsKey       = TENTE_SEARCH_DIR_KEY;
        ldrawLibPathKey          = TENTE_LDRAW_LIB_PATH_KEY;
    }
    else
    if (validLDrawLibrary == VEXIQ_LIBRARY) {
        validFadeStepsColour     = VEXIQ_FADE_COLOUR_DEFAULT;
        validLDrawPartsLibrary   = VEXIQ_LIBRARY "® Parts";
        validLDrawLibraryChange  = VEXIQ_LIBRARY;
        validLDrawDir            = VER_VEXIQ_LDRAWDIR_STR;
        validLDrawPart           = VER_VEXIQ_LDRAWPART_STR;
        validLDrawPartsArchive   = VER_LPUB3D_VEXIQ_ARCHIVE;
        validLDrawColorParts     = VER_LPUB3D_VEXIQ_COLOR_PARTS;
        validLDrawCustomArchive  = VER_LPUB3D_VEXIQ_CUSTOM_ARCHIVE;
        validPliControl          = VER_VEXIQ_PLI_CONTROL_FILE;
        validTitleAnnotations    = VER_VEXIQ_TITLE_ANNOTATIONS_FILE;
        validFreeFormAnnotations = VER_VEXIQ_FREEFROM_ANNOTATIONS_FILE;
        validAnnotationStyleFile = VER_VEXIQ_ANNOTATION_STYLE_FILE;
        validPliSubstituteParts  = VER_VEXIQ_PLI_SUBSTITUTE_FILE;
        validExcludedPliParts    = VER_VEXIQ_PLI_EXCLUDED_FILE;

        fadeStepsColourKey       = VEXIQ_FADE_COLOUR_KEY;
        ldrawSearchDirsKey       = VEXIQ_SEARCH_DIR_KEY;
        ldrawLibPathKey          = VEXIQ_LDRAW_LIB_PATH_KEY;
    }
}

void Preferences::setDistribution(){
#ifdef Q_OS_WIN
    if ((portableDistribution = QDir(QCoreApplication::applicationDirPath()+"/extras").exists())){
        QDir configDir(QCoreApplication::applicationDirPath()+"/config");
        if(!QDir(configDir).exists())
            configDir.mkpath(".");
        QSettings::setDefaultFormat(QSettings::IniFormat);
        QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, configDir.absolutePath());
    }
#endif
}

#ifdef Q_OS_MAC
bool Preferences::validLib(const QString &libName, const QString &libVersion) {

    int waitTime = 60000 ; // 60 secs
    QString scriptFile, scriptCommand;
    QTemporaryDir tempDir;
    if (tempDir.isValid()) {
        scriptFile =  QString("%1/ver.sh").arg(tempDir.path());
        //logDebug() << QString("Script file: [%1]").arg(scriptFile);

        if (libName != "xquartz")
            scriptCommand = QString("echo $(brew info " + libName + ") | sed \"s/^.*stable \\([^(]*\\).*/\\1/\"");
        else
            scriptCommand = QString("echo $(xdpyinfo | grep 'version number') | sed \"s/^.*\\:[ \\t]*\\(.*\\)$/\\1/\" && osascript -e 'quit app \"XQuartz\"'");

        QFile file(scriptFile);
        if(file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream stream(&file);
            stream << "#!/bin/bash" << endl;
            stream << scriptCommand << endl;
            file.close();
        } else {
           logError() << QString("Cannot write library check script file [%1] %2.")
                                 .arg(file.fileName())
                                 .arg(file.errorString());
           return false;
        }
    } else {
       logError() << QString("Cannot create library check temp path.");
       return false;
    }

    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    QStringList envList = env.toStringList();
    envList.replaceInStrings(QRegularExpression("^(?i)PATH=(.*)"), "PATH=/usr/local/Homebrew/bin:/opt/local/bin:/usr/local/bin/:/opt/x11/bin:$HOME/bin:\\1");

    //logDebug() << "SystemEnvironment:  " << envList.join(" ");

    QProcess pr;
    pr.setEnvironment(envList);
    pr.start("/bin/sh",QStringList() << scriptFile);

    if (! pr.waitForStarted()) {
        logError() << QString("Cannot start library check process.");
        return false;
    }

    if (! pr.waitForFinished(waitTime)) {
        if (pr.exitCode() != 0) {
            QByteArray status = pr.readAll();
            QString str;
            str.append(status);
            logError() << QString("Library check process failed with code %1 %2")
                          .arg(pr.exitCode())
                          .arg(str);
            return false;
        }
    }

    QString p_stderr = pr.readAllStandardError();
    if (!p_stderr.isEmpty()) {
        logError() << "Library check returned error: " << p_stderr;
        return false;
    }

    QString v1 = libVersion;
    QString v2 = pr.readAllStandardOutput().trimmed();

    // Compare v1 with v2 and return an integer:
    // Return -1 when v1 < v2
    // Return  0 when v1 = v2
    // Return  1 when v1 > v2

    int vc = QString::compare(v1, v2, Qt::CaseInsensitive);

    logInfo() << QString("Library version check - [%1] minimum :[%2] installed:[%3] vc(%4): %5]")
                         .arg(libName).arg(v1).arg(v2).arg(vc).arg(vc < 0 ? "v1 < v2" : vc == 0 ? "v1 = v2" : "v1 > v2");

    if (vc > 0) {
        logTrace() << QString("Library %1 version [%2] is less than required version [%3]")
                              .arg(libName).arg(v2).arg(libVersion);
        return false;
    }

    return true;
}
#endif

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
    QSettings Settings;

    if (portableDistribution) { // we have a portable distribution

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
    QString _logPath = QString("%1/logs/%2Log.txt").arg(lpubDataPath).arg(VER_PRODUCTNAME_STR);
#ifdef QT_DEBUG_MODE
    qDebug() << QString(QString("LPub3D Executable Path.......(%1)").arg(lpub3dPath));
    qDebug() << QString(QString("LPub3D Log Path..............(%1)").arg(_logPath));
#else
    fprintf(stdout, "%s\n", QString(QString("LPub3D Executable Path.......(%1)").arg(lpub3dPath)).toLatin1().constData());
    fprintf(stdout, "%s\n", QString(QString("LPub3D Log Path..............(%1)").arg(_logPath)).toLatin1().constData());
#endif
#endif


#ifdef Q_OS_WIN
    QString dataDir = "data";
    QString dataPath = lpub3dPath;
    if (portableDistribution) {
        dataDir = "extras";
#ifdef QT_DEBUG_MODE
        qDebug() << QString("LPub3D Portable Distribution.(Yes)");
#else
        fprintf(stdout, "%s\n", QString("LPub3D Portable Distribution.(Yes)").toLatin1().constData());
#endif
    // On Windows installer 'dataLocation' folder defaults to LPub3D install path but can be set with 'DataLocation' reg key
    } else if (Settings.contains(QString("%1/%2").arg(SETTINGS,"DataLocation"))) {
        QString validDataPath = Settings.value(QString("%1/%2").arg(SETTINGS,"DataLocation")).toString();
        QDir validDataDir(QString("%1/%2/").arg(validDataPath,dataDir));
        if(QDir(validDataDir).exists()) {
           dataPath = validDataPath;
#ifdef QT_DEBUG_MODE
           qDebug() << QString(QString("LPub3D Data Location.........(%1)").arg(validDataDir.absolutePath()));
#else
           fprintf(stdout, "%s\n", QString(QString("LPub3D Data Location.........(%1)").arg(validDataDir.absolutePath())).toLatin1().constData());
#endif
        }
    }
    dataLocation = QString("%1/%2/").arg(dataPath,dataDir);

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

    QDir extrasDir(lpubDataPath + QDir::separator() + "extras");
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
    paramFile.setFile(QString("%1/%2").arg(extrasDir.absolutePath(), VER_PLI_CONTROL_FILE));
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
    fprintf(stdout, "%s\n", QString(QString("LPub3D Loaded LDraw Library..(%1 Parts)").arg(validLDrawLibrary)).toLatin1().constData());
    fprintf(stdout, "--------------------------\n");
    fprintf(stdout, "\n");
    fflush(stdout);
#endif
}

void Preferences::loggingPreferences()
{
    // define log path
    QDir logDir(lpubDataPath+"/logs");
    if(!QDir(logDir).exists())
        logDir.mkpath(".");
    logPath = QDir(logDir).filePath(QString("%1Log.txt").arg(VER_PRODUCTNAME_STR));

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

    if ( ! Settings.contains(QString("%1/%2").arg(LOGGING,"Logging"))) {
        QVariant uValue(true);
        logging = true;
        Settings.setValue(QString("%1/%2").arg(LOGGING,"Logging"),uValue);
    } else {
        logging = Settings.value(QString("%1/%2").arg(LOGGING,"Logging")).toBool();
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
    emit Application::instance()->splashMsgSig("5% - Locate LDraw archive libraries...");

#ifdef Q_OS_WIN
    QString filter(QFileDialog::tr("Archive (*.zip *.bin);;All Files (*.*)"));
#else
    QString filter(QFileDialog::tr("All Files (*.*)"));
#endif

    QFileInfo fileInfo;
    QString const PartsLibraryKey("PartsLibrary");

    QSettings Settings;

    // check if skip parts archive selected
    if (Settings.contains(QString("%1/%2").arg(DEFAULTS,SAVE_SKIP_PARTS_ARCHIVE_KEY))) {
        skipPartsArchive = Settings.value(QString("%1/%2").arg(DEFAULTS,SAVE_SKIP_PARTS_ARCHIVE_KEY)).toBool();
        Settings.remove(QString("%1/%2").arg(DEFAULTS,SAVE_SKIP_PARTS_ARCHIVE_KEY));
    }

    lpub3dLibFile = Settings.value(QString("%1/%2").arg(SETTINGS,PartsLibraryKey)).toString();

    // Set archive library path, also check alternate location (e.g. AIOI uses C:\Users\Public\Documents\LDraw)
    QString validFile = QString("%1/libraries/%2").arg(lpubDataPath, validLDrawPartsArchive);

    bool altLibLocation = false;
    if (Settings.contains(QString("%1/%2").arg(SETTINGS,"AltLibLocation")))
        altLibLocation = Settings.value(QString("%1/%2").arg(SETTINGS,"AltLibLocation")).toBool();

    // Start by checking the registry value, if not exist set to valid file path
    if (Settings.contains(QString("%1/%2").arg(SETTINGS,PartsLibraryKey))) {
        lpub3dLibFile = Settings.value(QString("%1/%2").arg(SETTINGS,PartsLibraryKey)).toString();
    } else {
        lpub3dLibFile = QDir::toNativeSeparators(validFile);
        Settings.setValue(QString("%1/%2").arg(SETTINGS, PartsLibraryKey), lpub3dLibFile);
    }

    // Confirm archive library location is valid
    if (lpub3dLibFile != validFile && !altLibLocation) {
        lpub3dLibFile = QDir::toNativeSeparators(validFile);
        Settings.setValue(QString("%1/%2").arg(SETTINGS, PartsLibraryKey), lpub3dLibFile);
    }

    // If we have a valid library archive file, update the 3DViewer parts_library variable, else clear the registry value
    if (! lpub3dLibFile.isEmpty() && ! force) {
        fileInfo.setFile(lpub3dLibFile);

        if (fileInfo.exists()) {
            QString partsLibrary = Settings.value(QString("%1/%2").arg(SETTINGS,PartsLibraryKey)).toString();
            return;
        }
        else {
            lpub3dLibFile.clear();
            Settings.remove(QString("%1/%2").arg(SETTINGS, PartsLibraryKey));
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

        Settings.setValue(QString("%1/%2").arg(SETTINGS, PartsLibraryKey), lpub3dLibFile);

    } else {

        // lets go look for the archive files...

        fileInfo.setFile(dataLocation + validLDrawPartsArchive);
        bool archivesExist = fileInfo.exists();

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
        //                                                 .arg(fileInfo.absoluteFilePath()));

        if (archivesExist) {  // This condition should always fire as archive files are deposited at installation

            emit Application::instance()->splashMsgSig("10% - Copying archive libraries...");

            QDir libraryDir(QString("%1/%2").arg(lpubDataPath).arg(Paths::libraryDir));
            if (!QDir(libraryDir).exists())
                libraryDir.mkpath(".");

            fileInfo.setFile(QString("%1/%2").arg(libraryDir.absolutePath(), validLDrawPartsArchive));
            if (!fileInfo.exists())
                QFile::copy(dataLocation + fileInfo.fileName(), fileInfo.absoluteFilePath());

            lpub3dLibFile = QDir::toNativeSeparators(fileInfo.absoluteFilePath());
            Settings.setValue(QString("%1/%2").arg(SETTINGS, PartsLibraryKey), lpub3dLibFile);

            if (usingDefaultLibrary)
                fileInfo.setFile(QString("%1/%2").arg(libraryDir.absolutePath(), VER_LPUB3D_UNOFFICIAL_ARCHIVE));
            if (usingDefaultLibrary && !fileInfo.exists())
                QFile::copy(dataLocation + fileInfo.fileName(), fileInfo.absoluteFilePath());

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
                                              .arg(validLDrawPartsArchive)
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
                    lpub3dLibFile = QDir::toNativeSeparators(dlgGetFileName.selectedFiles().at(0));
                    Settings.setValue(QString("%1/%2").arg(SETTINGS, PartsLibraryKey), lpub3dLibFile);

                } else {

                    Settings.remove(QString("%1/%2").arg(SETTINGS, PartsLibraryKey));
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
                    if (libraryDownload->getCancelled()) {
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
                    if (libraryDownload->getCancelled()) {
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
                    lpub3dLibFile = QDir::toNativeSeparators(QString("%1/%2").arg(libraryDir.absolutePath(), validLDrawPartsArchive));
                    fileInfo.setFile(lpub3dLibFile);
                    if (!fileInfo.exists()) {

                        Settings.remove(QString("%1/%2").arg(SETTINGS, PartsLibraryKey));
                        if (usingDefaultLibrary)
                            fileInfo.setFile(QDir::toNativeSeparators(QString("%1/%2").arg(libraryDir.absolutePath(), VER_LDRAW_UNOFFICIAL_ARCHIVE)));
                        if (usingDefaultLibrary && !fileInfo.exists()) {
                            body = QMessageBox::tr ("Required archive files\n%1\n%2\ndoes not exist.").arg(lpub3dLibFile, fileInfo.absoluteFilePath());
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

            QString officialArchive = fileInfo.absoluteFilePath();
            if (usingDefaultLibrary)
                fileInfo.setFile(dataLocation + VER_LPUB3D_UNOFFICIAL_ARCHIVE);
            if (usingDefaultLibrary && !fileInfo.exists()) {
                fprintf(stderr, "Required archive files\n%s\n%s\ndoes not exist.\n", officialArchive.toLatin1().constData(), fileInfo.absoluteFilePath().toLatin1().constData());
            } else {
                fprintf(stderr, "Required archive file %s does not exist.\n", lpub3dLibFile.toLatin1().constData());
            }
            fprintf(stderr, "Launching %s in GUI mode offers a dialogue to download, and extract archive libraries.\n",lpub3dAppName.toLatin1().constData());
        }
    }
}

void Preferences::ldrawPreferences(bool force)
{
    emit Application::instance()->splashMsgSig("10% - Locate LDraw directory...");

    QSettings Settings;
    ldrawLibPath = Settings.value(QString("%1/%2").arg(SETTINGS,ldrawLibPathKey)).toString();

    QDir ldrawDir(ldrawLibPath);
    if (! QFileInfo(ldrawDir.absolutePath()+validLDrawPart).exists() || force) {      // first check

        QString returnMessage = QString();

        if (! force ) {                                                               // second check - no force
            ldrawLibPath.clear();
            Settings.remove(QString("%1/%2").arg(SETTINGS,ldrawLibPathKey));

            char* EnvPath = getenv("LDRAWDIR");                                       // check environment variable LDRAWDIR

            if (EnvPath && EnvPath[0]) {
                ldrawLibPath = QString(EnvPath);
                ldrawDir.setPath(ldrawLibPath);
            }


            if (portableDistribution &&
               (ldrawLibPath.isEmpty() ||
               ! QFileInfo(ldrawDir.absolutePath()+validLDrawPart).exists())){       // third check - no force
                ldrawLibPath = QDir::toNativeSeparators(QString("%1/%2").arg(lpubDataPath).arg(validLDrawDir));
                ldrawDir.setPath(ldrawLibPath);
            }
        }

        if (! QFileInfo(ldrawDir.absolutePath()+validLDrawPart).exists() || force) {  // second check - force
            if (! force) {                                                            // fourth check - no force
                ldrawLibPath.clear();

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
                ldrawLibPath = QDir::toNativeSeparators(QString("%1/%2").arg(homePath).arg(validLDrawDir));

                if ( ! QFileInfo(ldrawLibPath+validLDrawPart).exists()) {     // check user documents path

                    ldrawLibPath = QDir::toNativeSeparators(QString("%1/%2").arg(userDocumentsPath).arg(validLDrawDir));

                    if ( ! QFileInfo(ldrawLibPath+validLDrawPart).exists()) { // check system data path

                        dataPathList = QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation);

#if defined Q_OS_WIN || defined Q_OS_MAC
                        ldrawLibPath = QDir::toNativeSeparators(QString("%1/%2").arg(dataPathList.at(1)).arg(validLDrawDir)); /* C:/Users/<user>/AppData/Local/LPub3D Software/LPub3Dd/<LDraw library>,  ~/Library/Application Support/LPub3D Software/LPub3D/<LDraw library> */
#else
                        ldrawLibPath = QDir::toNativeSeparators(QString("%1/%2").arg(dataPathList.at(2)).arg(validLDrawDir)); /* ~/.local/share/LPub3D Software/lpub3d<ver_suffix>/<LDraw library>" */
#endif

                        if ( ! QFileInfo(ldrawLibPath+validLDrawPart).exists()) {     // check user data path

                            ldrawLibPath = QDir::toNativeSeparators(QString("%1/%2").arg(userLocalDataPath).arg(validLDrawDir));

                            if ( ! QFileInfo(ldrawLibPath+validLDrawPart).exists()) { // manual prompt for LDraw Library location
                                ldrawLibPath.clear();

                                QString searchDetail;
                                searchDetail = QMessageBox::tr ("\t%1\n\t%2\n\t%3\n\t%4")
                                        .arg(portableDistribution ?
                                            QString("%1\n\t%2")
                                                 .arg(QDir::toNativeSeparators(QString("%1/%2").arg(lpubDataPath).arg(validLDrawDir)))
                                                 .arg(QDir::toNativeSeparators(QString("%1/%2").arg(homePath).arg(validLDrawDir))) :
                                            QDir::toNativeSeparators(QString("%1/%2").arg(homePath).arg(validLDrawDir)))
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

                                    if (isAppImagePayload ) { // For AppImage, automatically install LDraw library (and notify user) if library not found

                                        if (extractLDrawLib()) {

                                            header = "<b> " + QMessageBox::tr ("%1 LDraw library installed").arg(validLDrawLibrary) + "</b>";
                                            body = QMessageBox::tr ("LDraw library was not found. The bundled library archives were installed at\n"
                                                                    "%1").arg(ldrawLibPath);
                                            detail = QMessageBox::tr ("The following locations were searched for the LDraw library.\n%1\n"
                                                                      "You can change the library path in the Preferences dialogue.").arg(searchDetail);
                                            box.setText (header);
                                            box.setInformativeText (body);
                                            box.setDetailedText(detail);
                                            box.setStandardButtons (QMessageBox::Close);
                                            box.exec();

                                        }

                                    } else { // For all except AppImage, prompt user to select or install LDraw library if not found

                                        QAbstractButton* extractButton = box.addButton(QMessageBox::tr("Extract Archive"),QMessageBox::YesRole);
                                        QAbstractButton* selectButton  = box.addButton(QMessageBox::tr("Select Folder"),QMessageBox::YesRole);

                                        header = "<b> " + QMessageBox::tr ("%1 LDraw library folder not found!").arg(validLDrawLibrary) + "</b>";
                                        body = QMessageBox::tr ("You may select your LDraw folder or extract it from the bundled %1 %2.\n"
                                                                "Would you like to extract the library or select the LDraw folder?")
                                                                 .arg(validLDrawLibrary).arg(usingDefaultLibrary ? "archives" : "archive");
                                        detail = QMessageBox::tr ("The following locations were searched for the LDraw library:\n%1\n"
                                                                  "You must select an LDraw library folder or extract the library.\n"
                                                                  "It is possible to create your library folder from the %2 file (%3) "
                                                                  "and the %4 parts archive file %5. The extracted library folder will "
                                                                  "be located at '%6'").arg(searchDetail)
                                                                   .arg(usingDefaultLibrary ? "official LDraw LEGO archive" : "LDraw " + validLDrawLibrary + " archive.")
                                                                   .arg(validLDrawPartsArchive).arg(usingDefaultLibrary ? "unofficial" : "custom")
                                                                   .arg(validLDrawCustomArchive).arg(ldrawLibPath);
                                        box.setText (header);
                                        box.setInformativeText (body);
                                        box.setDetailedText(detail);
                                        box.setStandardButtons (QMessageBox::Cancel);
                                        box.exec();

                                        if (box.clickedButton()==selectButton) {
                                            emit Application::instance()->splashMsgSig("10% - Selecting LDraw folder...");

                                            ldrawLibPath = QFileDialog::getExistingDirectory(nullptr,
                                                                                          QFileDialog::tr("Select LDraw library folder"),
                                                                                          "/",
                                                                                          QFileDialog::ShowDirsOnly |
                                                                                          QFileDialog::DontResolveSymlinks);

                                            if (! ldrawLibPath.isEmpty()) {
                                                if (checkLDrawLibrary(ldrawLibPath)) {
                                                    Settings.setValue(QString("%1/%2").arg(SETTINGS,ldrawLibPathKey),ldrawLibPath);
                                                } else {
                                                    ldrawLibPath.clear();
                                                    returnMessage = QMessageBox::tr ("The selected path [%1] does not "
                                                                                     "appear to be a valid LDraw Library.")
                                                                                     .arg(ldrawLibPath);
                                                }
                                            }

                                        } else {
                                            if (box.clickedButton()==extractButton) {
                                                extractLDrawLib();
                                            } else {
                                                ldrawLibPath.clear();
                                            }
                                        }
                                    }

                                } else {                  // Console mode so extract and install LDraw Library automatically if not exist in searched paths.
                                    QString message = QString("The %1 LDraw library was not found.\n"
                                                              "The following locations were searched for the LDraw library:\n%2.\n").arg(validLDrawLibrary).arg(searchDetail);
                                    fprintf(stdout,"%s\n",message.toLatin1().constData());
                                    if (extractLDrawLib()) {
                                        message = QString("The bundled %1 LDraw archive library was extracted to:\n%2\n"
                                                          "You can edit the library path in the Preferences dialogue.\n").arg(validLDrawLibrary).arg(ldrawLibPath);
                                        fprintf(stdout,"%s\n",message.toLatin1().constData());
                                    }
                                    fflush(stdout);
                                }
                            }
                        }
                    }
                }
            }

            if (! ldrawLibPath.isEmpty() && force && modeGUI) {
#ifdef Q_OS_MAC
                if (! lpub3dLoaded && modeGUI && Application::instance()->splash->isVisible())
                    Application::instance()->splash->hide();
#endif
                QString result = QFileDialog::getExistingDirectory(nullptr,
                                                                   QFileDialog::tr("Select LDraw Directory"),
                                                                   ldrawLibPath,
                                                                   QFileDialog::ShowDirsOnly |
                                                                   QFileDialog::DontResolveSymlinks);
                if (! result.isEmpty()) {

                    if (checkLDrawLibrary(result)) {
                        ldrawLibPath = QDir::toNativeSeparators(result);
                    } else {
                        returnMessage = QMessageBox::tr ("The specified path is not a valid LPub3D-supported LDraw Library.\n"
                                                          "%1").arg(ldrawLibPath);
                    }
                }
            }

            if (! ldrawLibPath.isEmpty()) {

                Settings.setValue(QString("%1/%2").arg(SETTINGS,ldrawLibPathKey),ldrawLibPath);

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
            Settings.setValue(QString("%1/%2").arg(SETTINGS,ldrawLibPathKey),ldrawLibPath);
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
                                                                   ldrawLibPath,
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

    // LSynth settings
    QString const addLSynthSearchDirKey("AddLSynthSearchDir");
    if ( ! Settings.contains(QString("%1/%2").arg(SETTINGS,addLSynthSearchDirKey))) {
        QVariant uValue(false);
        addLSynthSearchDir = false;
        Settings.setValue(QString("%1/%2").arg(SETTINGS,addLSynthSearchDirKey),uValue);
    } else {
        addLSynthSearchDir = Settings.value(QString("%1/%2").arg(SETTINGS,addLSynthSearchDirKey)).toBool();
    }

    QString const archiveLSynthPartsKey("ArchiveLSynthParts");
    if ( ! Settings.contains(QString("%1/%2").arg(SETTINGS,archiveLSynthPartsKey))) {
        QVariant uValue(false);
        archiveLSynthParts = false;
        Settings.setValue(QString("%1/%2").arg(SETTINGS,archiveLSynthPartsKey),uValue);
    } else {
        archiveLSynthParts = Settings.value(QString("%1/%2").arg(SETTINGS,archiveLSynthPartsKey)).toBool();
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

    if ( ! Settings.contains(QString("%1/%2").arg(SETTINGS,"ShowDownloadRedirects"))) {
        QVariant uValue(true);
        showDownloadRedirects = false;
        Settings.setValue(QString("%1/%2").arg(SETTINGS,"ShowDownloadRedirects"),uValue);
    } else {
        showDownloadRedirects = Settings.value(QString("%1/%2").arg(SETTINGS,"ShowDownloadRedirects")).toBool();
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
        lgeoDir = QDir::toNativeSeparators(ldrawLibPath + "/lgeo");
    }
    QDir lgeoDirInfo(lgeoDir);
    if (lgeoDirInfo.exists()) {
        lgeoPath = lgeoDir;
        logInfo() << QString("LGEO library path  : %1").arg(lgeoDirInfo.absolutePath());
        /* Durat's lgeo stl library Check */
        QDir lgeoStlLibInfo(QDir::toNativeSeparators(lgeoPath + "/stl"));
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
    emit Application::instance()->splashMsgSig(QString("25% - %1 macOS Required Library Check...").arg(VER_PRODUCTNAME_STR));
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
            missingRendererLibs = true;
            QVariant eValue(missingRendererLibs);
            Settings.setValue(QString("%1/%2").arg(SETTINGS,"LDViewMissingLibs"),eValue);
        } else {
            missingRendererLibs = Settings.value(QString("%1/%2").arg(SETTINGS,"LDViewMissingLibs")).toBool();
        }

        if (missingRendererLibs) {
            missingLibs.clear();
            QFileInfo libInfo("/opt/X11/lib/libOSMesa.dylib");
            if (!libInfo.exists()){
                missingLibs << libInfo.absoluteFilePath() + " - not found.";
            } else {
                QString xquartz = "xquartz";
                if (!validLib(xquartz, LIBXQUARTZ_MACOS_VERSION))
                    missingLibs << libInfo.absoluteFilePath() + " - invalid version.";
            }
            libInfo.setFile("/usr/local/opt/libpng/lib/libpng.dylib");
            if (!libInfo.exists()){
                missingLibs << libInfo.absoluteFilePath() + " - not found.";
            } else {
                if (!validLib(libInfo.completeBaseName(), LIBPNG_MACOS_VERSION))
                    missingLibs << libInfo.absoluteFilePath() + " - invalid version.";
            }
            libInfo.setFile("/usr/local/opt/gl2ps/lib/libgl2ps.dylib");
            if (!libInfo.exists()){
                missingLibs << libInfo.absoluteFilePath() + " - not found.";
            } else {
                if (!validLib(libInfo.completeBaseName().replace("lib",""), LIBGL2PS_MACOS_VERSION))
                    missingLibs << libInfo.absoluteFilePath() + " - invalid version.";
            }
            libInfo.setFile("/usr/local/opt/jpeg/lib/libjpeg.dylib");
            if (!libInfo.exists()){
                missingLibs << libInfo.absoluteFilePath() + " - not found.";
            } else {
                if (!validLib(libInfo.completeBaseName().replace("lib",""), LIBJPEG_MACOS_VERSION))
                    missingLibs << libInfo.absoluteFilePath() + " - invalid version.";
            }
            libInfo.setFile("/usr/local/opt/tinyxml/lib/libtinyxml.dylib");
            if (!libInfo.exists()){
                missingLibs << libInfo.absoluteFilePath() + " - not found.";
            } else {
                if (!validLib(libInfo.completeBaseName().replace("lib",""), LIBXML_MACOS_VERSION))
                    missingLibs << libInfo.absoluteFilePath() + " - invalid version.";
            }
            libInfo.setFile("/usr/local/opt/minizip/lib/libminizip.dylib");
            if (!libInfo.exists()){
                missingLibs << libInfo.absoluteFilePath() + " - not found.";
            } else {
                if (!validLib(libInfo.completeBaseName().replace("lib",""), LIBMINIZIP_MACOS_VERSION))
                    missingLibs << libInfo.absoluteFilePath() + " - invalid version.";
            }

            missingRendererLibs = missingLibs.size() > 0;
            QVariant eValue(missingRendererLibs);
            if (!missingRendererLibs) {
                Settings.setValue(QString("%1/%2").arg(SETTINGS,"LDViewMissingLibs"),eValue);
            }
            else
            {
                QString header = QMessageBox::tr ("<b>Required libraries were not found!</b>");
                QString body = QMessageBox::tr ("The following LDView libraries were not found:%2%2-%1%2%2"
                                                "See /Applications/LPub3D.app/Contents/Resources/README_macOS.txt for details.")
                                                .arg(missingLibs.join("\n -").arg(modeGUI ? "<br>" : "\n"));
                box.setText (header);
                box.setInformativeText (body);

                if (modeGUI) {
                    if (! lpub3dLoaded && Application::instance()->splash->isVisible())
                        Application::instance()->splash->hide();
                    if (box.exec() == QMessageBox::Close) {
                        if (! lpub3dLoaded && Application::instance()->splash->isHidden())
                            Application::instance()->splash->show();
                    }
                } else {
                    fprintf(stdout,"%s\n",body.toLatin1().constData());
                    fflush(stdout);

                    logDebug() << QString("LDView Missing Libs: %1").arg(missingLibs.join("\n -"));
                }
            }
        }
#endif
    } else {
        logError() << QString("LDView   : %1 not found").arg(ldviewInfo.absoluteFilePath());
    }

    if (povrayInfo.exists()) {
        povRayInstalled = true;
        povrayExe = QDir::toNativeSeparators(povrayInfo.absoluteFilePath());
        logInfo() << QString("POVRay  : %1").arg(povrayExe);

#ifdef Q_OS_MAC
// Check POVRay libraries on macOS

        if (! Settings.contains(QString("%1/%2").arg(SETTINGS,"POVRayMissingLibs"))) {
          missingRendererLibs = true;
          QVariant eValue(missingRendererLibs);
          Settings.setValue(QString("%1/%2").arg(SETTINGS,"POVRayMissingLibs"),eValue);
        } else {
          missingRendererLibs = Settings.value(QString("%1/%2").arg(SETTINGS,"POVRayMissingLibs")).toBool();
        }

        if (missingRendererLibs) {
            missingLibs.clear();
            QFileInfo libInfo("/opt/X11/lib/libX11.dylib");
            if (!libInfo.exists()){
                missingLibs << libInfo.absoluteFilePath() + " - not found.";
            } else {
                QString xquartz = "xquartz";
                if (!validLib(xquartz, LIBXQUARTZ_MACOS_VERSION))
                    missingLibs << libInfo.absoluteFilePath() + " - invalid version.";
            }
            libInfo.setFile("/usr/local/opt/libtiff/lib/libtiff.dylib");
            if (!libInfo.exists()){
                missingLibs << libInfo.absoluteFilePath() + " - not found.";
            } else {
                if (!validLib(libInfo.completeBaseName(), LIBTIFF_MACOS_VERSION))
                    missingLibs << libInfo.absoluteFilePath() + " - invalid version.";
            }
            libInfo.setFile("/usr/local/opt/openexr/lib/libIlmImf.dylib");
            if (!libInfo.exists()){
                missingLibs << libInfo.absoluteFilePath() + " - not found.";
            } else {
                QString openexr = "openexr";
                if (!validLib(openexr, LIBOPENEXR_MACOS_VERSION))
                    missingLibs << libInfo.absoluteFilePath() + " - invalid version.";
            }
            libInfo.setFile("/usr/local/opt/ilmbase/lib/libHalf.dylib");
            if (!libInfo.exists()){
                missingLibs << libInfo.absoluteFilePath() + " - not found.";
            } else {
                QString ilmbase = "ilmbase";
                if (!validLib(ilmbase, LIBILMBASE_MACOS_VERSION))
                    missingLibs << libInfo.absoluteFilePath() + " - invalid version.";
            }
            libInfo.setFile("/usr/local/opt/sdl2/lib/libSDL2.dylib");
            if (!libInfo.exists()){
                missingLibs << libInfo.absoluteFilePath() + " - not found.";
            } else {
                if (!validLib(libInfo.completeBaseName().replace("lib","").toLower(), LIBSDL_MACOS_VERSION))
                    missingLibs << libInfo.absoluteFilePath() + " - invalid version.";
            }

            missingRendererLibs = missingLibs.size() > 0;
            QVariant eValue(missingRendererLibs);
            if (!missingRendererLibs) {
                Settings.setValue(QString("%1/%2").arg(SETTINGS,"POVRayMissingLibs"),eValue);
            }
            else
            {
              QString header = QMessageBox::tr ("<b>Required libraries were not found!</b>");
              QString body = QMessageBox::tr ("The following required POVRay libraries were not found:%2%2-%1%2%2"
                                              "See /Applications/LPub3D.app/Contents/Resources/README_macOS.txt for details.")
                                              .arg(missingLibs.join("\n -").arg(modeGUI ? "<br>" : "\n"));
              box.setText (header);
              box.setInformativeText (body);

              if (modeGUI) {
                if (!lpub3dLoaded && Application::instance()->splash->isVisible())
                  Application::instance()->splash->hide();
                if (box.exec() == QMessageBox::Close) {
                    if (! lpub3dLoaded && Application::instance()->splash->isHidden())
                      Application::instance()->splash->show();
                }
              } else {
                fprintf(stdout,"%s\n",body.toLatin1().constData());
                fflush(stdout);

                logDebug() << QString("POVRay Missing Libraries: %1").arg(missingLibs.join("\n -"));
              }
           }
        }
        emit Application::instance()->splashMsgSig(QString("25% - %1 window defaults loading...").arg(VER_PRODUCTNAME_STR));
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
//        if (!missingRendererLibs)
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

    // using native Renderer flag
    usingNativeRenderer = preferredRenderer == RENDERER_NATIVE;

    // Default projection
    QString const perspectiveProjectionKey("PerspectiveProjection");

    if ( ! Settings.contains(QString("%1/%2").arg(SETTINGS,perspectiveProjectionKey))) {
        QVariant uValue(perspectiveProjection);
        Settings.setValue(QString("%1/%2").arg(SETTINGS,perspectiveProjectionKey),uValue);
    } else {
        perspectiveProjection = Settings.value(QString("%1/%2").arg(SETTINGS,perspectiveProjectionKey)).toBool();
    }

    // LDView multiple files single call rendering
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

    //  LDView single call snapshot list
    if ( ! Settings.contains(QString("%1/%2").arg(SETTINGS,"EnableLDViewSnapshotsList"))) {
        QVariant uValue(false);
        enableLDViewSnaphsotList = false;
        Settings.setValue(QString("%1/%2").arg(SETTINGS,"EnableLDViewSnapshotsList"),uValue);
    } else {
        enableLDViewSnaphsotList = Settings.value(QString("%1/%2").arg(SETTINGS,"EnableLDViewSnapshotsList")).toBool();
    }

    // Renderer timeout
    if ( ! Settings.contains(QString("%1/%2").arg(SETTINGS,"RendererTimeout"))) {
        rendererTimeout = RENDERER_TIMEOUT_DEFAULT;
        Settings.setValue(QString("%1/%2").arg(SETTINGS,"RendererTimeout"),rendererTimeout);
    } else {
        rendererTimeout = Settings.value(QString("%1/%2").arg(SETTINGS,"RendererTimeout")).toInt();
    }

    // Native renderer camera distance factor - Do not add Settings if not exist
    if ( ! Settings.contains(QString("%1/%2").arg(SETTINGS,"CameraDistFactorNative"))) {
        cameraDistFactorNative = CAMERA_DISTANCE_FACTOR_NATIVE_DEFAULT;
    } else {
        cameraDistFactorNative = Settings.value(QString("%1/%2").arg(SETTINGS,"CameraDistFactorNative")).toInt();
    }

    // povray generation renderer
    if ( ! Settings.contains(QString("%1/%2").arg(SETTINGS,"POVFileGenerator"))) {
        QVariant cValue(RENDERER_NATIVE);
        povFileGenerator = RENDERER_NATIVE;
        Settings.setValue(QString("%1/%2").arg(SETTINGS,"POVFileGenerator"),cValue);
    } else {
        povFileGenerator = Settings.value(QString("%1/%2").arg(SETTINGS,"POVFileGenerator")).toString();
    }

    // Display povray image during rendering [experimental]
    QString const povrayDisplayKey("POVRayDisplay");
    if ( ! Settings.contains(QString("%1/%2").arg(SETTINGS,povrayDisplayKey))) {
        QVariant uValue(false);
        povrayDisplay = false;
        Settings.setValue(QString("%1/%2").arg(SETTINGS,povrayDisplayKey),uValue);
    } else {
        povrayDisplay = Settings.value(QString("%1/%2").arg(SETTINGS,povrayDisplayKey)).toBool();
    }

    // Set POV-Ray render quality
    QString const povrayRenderQualityKey("PovrayRenderQuality");
    if ( ! Settings.contains(QString("%1/%2").arg(SETTINGS,povrayRenderQualityKey))) {
        povrayRenderQuality = POVRAY_RENDER_QUALITY_DEFAULT;
        Settings.setValue(QString("%1/%2").arg(SETTINGS,povrayRenderQualityKey),povrayRenderQuality);
    } else {
        povrayRenderQuality = Settings.value(QString("%1/%2").arg(SETTINGS,povrayRenderQualityKey)).toInt();
    }

    // Automatically crop POV-Ray rendered images
    QString const povrayAutoCropKey("PovrayAutoCrop");
    if ( ! Settings.contains(QString("%1/%2").arg(SETTINGS,povrayAutoCropKey))) {
        QVariant uValue(true);
        povrayAutoCrop = true;
        Settings.setValue(QString("%1/%2").arg(SETTINGS,povrayAutoCropKey),uValue);
    } else {
        povrayAutoCrop = Settings.value(QString("%1/%2").arg(SETTINGS,povrayAutoCropKey)).toBool();
    }

    // Apply latitude and longitude camera angles locally
    if ( ! Settings.contains(QString("%1/%2").arg(SETTINGS,"ApplyCALocally"))) {
        applyCALocally = !(perspectiveProjection && preferredRenderer == RENDERER_LDVIEW);
        QVariant uValue(applyCALocally);
        Settings.setValue(QString("%1/%2").arg(SETTINGS,"ApplyCALocally"),uValue);
    } else {
        applyCALocally = Settings.value(QString("%1/%2").arg(SETTINGS,"ApplyCALocally")).toBool();
    }

    // Image matting [future use]
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
    lpub3dLDVConfigDir      = QString("%1/ldv").arg(lpubDataPath);
    setLDGLiteIniParams();
    updateLDVExportIniFile(updateFlag);
    updateLDViewIniFile(updateFlag);
    updateLDViewPOVIniFile(updateFlag);
    updatePOVRayConfFile(updateFlag);
    updatePOVRayIniFile(updateFlag);

    // Populate POVRay Library paths
    QFileInfo resourceFile;
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
                                  QString("(%1 POV file generator)").arg(povFileGenerator) :
                              preferredRenderer == RENDERER_LDVIEW ?
                              enableLDViewSingleCall ?
                                  QString("(Single Call%1)")
                                          .arg(enableLDViewSnaphsotList ? " - SnapshotsList" : "") : "" : "");
}

void Preferences::setLDGLiteIniParams()
{
    QString inFileName;
    QFileInfo resourceFile;
    QFile confFileIn, confFileOut;

    resourceFile.setFile(QString("%1/%2/config/%3").arg(lpub3d3rdPartyConfigDir, VER_LDGLITE_STR, VER_LDGLITE_INI_FILE));
    if (!resourceFile.exists()) {
        logInfo() << QString("Initializing %1...").arg(resourceFile.absoluteFilePath());
        inFileName = QString("%1/%2").arg(dataLocation, resourceFile.fileName());
        if (!resourceFile.absoluteDir().exists())
            resourceFile.absoluteDir().mkpath(".");
        confFileIn.setFileName(QDir::toNativeSeparators(inFileName));;
        confFileOut.setFileName(QString("%1/%2/config/%3").arg(lpub3d3rdPartyConfigDir, VER_LDGLITE_STR, resourceFile.fileName()));
        if (confFileIn.open(QIODevice::ReadOnly) && confFileOut.open(QIODevice::WriteOnly | QIODevice::Text)){
            QTextStream input(&confFileIn);
            QTextStream output(&confFileOut);
            while (!input.atEnd())
            {
                QString line = input.readLine();
                // Remove Template note from used instance
                if (line.contains(QRegExp("^__NOTE:"))){
                    continue;
                }
                output << line << endl;
            }
            confFileIn.close();
            confFileOut.close();
        } else {
            QString confFileError;
            if (!confFileIn.errorString().isEmpty())
                confFileError.append(QString(" confFileInError: %1").arg(confFileIn.errorString()));
            if (!confFileOut.errorString().isEmpty())
                confFileError.append(QString(" confFileOutError: %1").arg(confFileOut.errorString()));
            logError() << QString("Could not open ldglite.ini input or output file: %1").arg(confFileError);
        }
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
        logError() << QString("Could not open ldglite.ini input file: %1").arg(confFileError);
    }
    if (preferredRenderer == RENDERER_LDGLITE)
        logInfo() << QString("LDGLite Parameters :%1").arg((ldgliteParms.isEmpty() ? "No parameters" : ldgliteParms.join(" ")));
    if (resourceFile.exists())
        ldgliteIni = resourceFile.absoluteFilePath(); // populate ldglite ini file
    logInfo() << QString("LDGLite.ini file   : %1").arg(ldgliteIni.isEmpty() ? "Not found" : ldgliteIni);
}

void Preferences::updateLDVExportIniFile(UpdateFlag updateFlag)
{
    QString inFileName;
    QFileInfo resourceFile;
    QFile confFileIn, confFileOut, oldFile;
    QDateTime timeStamp = QDateTime::currentDateTime();

    resourceFile.setFile(QString("%1/%2").arg(lpub3dLDVConfigDir, VER_NATIVE_EXPORT_INI_FILE));
    if (resourceFile.exists())
    {
        if (updateFlag == SkipExisting) {
           nativeExportIni = resourceFile.absoluteFilePath(); // populate Native Export file file
           logInfo() << QString("Native Export file : %1").arg(nativeExportIni);
           return;
        }
        logInfo() << QString("Updating %1...").arg(resourceFile.absoluteFilePath());
        inFileName = QString("%1.%2").arg(resourceFile.absoluteFilePath(),timeStamp.toString("ddMMyyhhmmss"));
        oldFile.setFileName(resourceFile.absoluteFilePath());
        oldFile.rename(inFileName);
    } else {
       logInfo() << QString("Initializing %1...").arg(resourceFile.absoluteFilePath());
       inFileName = QString("%1/%2").arg(dataLocation, resourceFile.fileName());
       if (!resourceFile.absoluteDir().exists())
           resourceFile.absoluteDir().mkpath(".");
    }
    confFileIn.setFileName(QDir::toNativeSeparators(inFileName));
    confFileOut.setFileName(QString("%1/%2").arg(lpub3dLDVConfigDir, resourceFile.fileName()));
    if (confFileIn.open(QIODevice::ReadOnly) && confFileOut.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QTextStream input(&confFileIn);
        QTextStream output(&confFileOut);
        while (!input.atEnd())
        {
            QString line = input.readLine();
            // Remove Template note from used instance
            if (line.contains(QRegExp("^__NOTE:"))){
                continue;
            }
            // strip EdgeThickness because set in renderer parameters
            if (line.contains(QRegExp("^EdgeThickness="))){
                continue;
            }
            //logDebug() << QString("Line INPUT: %1").arg(line);
            // set ldraw dir
            if (line.contains(QRegExp("^LDrawDir=")))
            {
                line.clear();
                line = QString("LDrawDir=%1").arg(QDir::toNativeSeparators(ldrawLibPath));
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
        nativeExportIni = resourceFile.absoluteFilePath(); // populate native POV ini file
    if (oldFile.exists())
        oldFile.remove();                               // delete old file
    logInfo() << QString("NativePOV ini file : %1").arg(nativeExportIni.isEmpty() ? "Not found" : nativeExportIni);
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
           ldviewIni = QDir::toNativeSeparators(resourceFile.absoluteFilePath()); // populate ldview ini file
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
    confFileOut.setFileName(QString("%1/%2/config/%3").arg(lpub3d3rdPartyConfigDir, VER_LDVIEW_STR, resourceFile.fileName()));
    if (confFileIn.open(QIODevice::ReadOnly) && confFileOut.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QTextStream input(&confFileIn);
        QTextStream output(&confFileOut);
        while (!input.atEnd())
        {
            QString line = input.readLine();
            // strip EdgeThickness because set in renderer parameters
            if (line.contains(QRegExp("^EdgeThickness="))){
                continue;
            }
            //logDebug() << QString("Line INPUT: %1").arg(line);
            // set ldraw dir
            if (line.contains(QRegExp("^LDrawDir=")))
            {
                line.clear();
                line = QString("LDrawDir=%1").arg(QDir::toNativeSeparators(ldrawLibPath));
            }
            // set AutoCrop=0
//            if (line.contains(QRegExp("^AutoCrop="))) {
//                line.clear();
//                line = QString("AutoCrop=%1").arg((enableFadeSteps && enableImageMatting) ? 0 : 1);
//            }
            logInfo() << QString("LDView.ini OUT: %1").arg(line);
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
        logError() << QString("Could not open LDView.ini input or output file: %1").arg(confFileError);
    }
    if (resourceFile.exists())
        ldviewIni = QDir::toNativeSeparators(resourceFile.absoluteFilePath()); // populate ldview ini file
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
           ldviewPOVIni = resourceFile.absoluteFilePath(); // populate ldview POV ini file
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
    confFileOut.setFileName(QString("%1/%2/config/%3").arg(lpub3d3rdPartyConfigDir, VER_LDVIEW_STR, resourceFile.fileName()));
    if (confFileIn.open(QIODevice::ReadOnly) && confFileOut.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QTextStream input(&confFileIn);
        QTextStream output(&confFileOut);
        while (!input.atEnd())
        {
            QString line = input.readLine();
            // strip EdgeThickness because set in renderer parameters
            if (line.contains(QRegExp("^EdgeThickness="))){
              continue;
            }
            //logDebug() << QString("Line INPUT: %1").arg(line);
            // set ldraw dir
            if (line.contains(QRegExp("^LDrawDir=")))
            {
                line.clear();
                line = QString("LDrawDir=%1").arg(QDir::toNativeSeparators(ldrawLibPath));
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
        ldviewPOVIni = resourceFile.absoluteFilePath(); // populate ldview POV ini file
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
           updatePOVRayConfigFiles();
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
    confFileOut.setFileName(QString("%1/%2/config/%3").arg(lpub3d3rdPartyConfigDir, VER_POVRAY_STR, resourceFile.fileName()));
    if (confFileIn.open(QIODevice::ReadOnly) && confFileOut.open(QIODevice::WriteOnly | QIODevice::Text))
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
                QString locations = QString("You can use %HOME%, %INSTALLDIR% and the working directory (e.g. %1) as the origin to define permitted paths:")
                                            .arg(QDir::toNativeSeparators(QDir::homePath()+"/MOCs/myModel"));
                QString homedir = QString("%HOME% is hard-coded to the %1 environment variable (%2).")
#if defined Q_OS_WIN
                                          .arg("%USERPROFILE%")
#else
                                          .arg("$USER")
#endif
                                          .arg(QDir::toNativeSeparators(QDir::homePath()));
                QString workingdir = QString("The working directory (e.g. %1) is where LPub3D-Trace is called from.")
                                             .arg(QDir::toNativeSeparators(QDir::homePath()+"/MOCs/myModel"));

                // set application 3rd party renderers path
                line.replace(QString("__USEFUL_LOCATIONS_COMMENT__"),locations);
                line.replace(QString("__HOMEDIR_COMMENT__"),homedir);
                line.replace(QString("__WORKINGDIR_COMMENT__"),workingdir);
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

    updatePOVRayConfigFiles();
}

void Preferences::updatePOVRayConfigFiles(){
#if defined Q_OS_WIN
    if (preferredRenderer == RENDERER_POVRAY) {
        QString targetFolder, destFolder, dataPath, targetFile, destFile, saveFile;
        QStringList dataPathList = QStandardPaths::standardLocations(QStandardPaths::DataLocation);
        QFileInfo fileInfo, saveFileInfo;
        dataPath     = dataPathList.first();
        destFolder   = QString("%1/%2").arg(dataPath).arg("3rdParty/" VER_POVRAY_STR "/config");
        targetFolder = QString(lpub3dPath +"/3rdParty/" VER_POVRAY_STR "/config");
        if (portableDistribution) {
            targetFile = QString("%1/povray.conf").arg(targetFolder);
            destFile   = QString("%1/povray.conf").arg(destFolder);
            saveFile   = QString("%1/povray_install.conf").arg(destFolder);
            fileInfo.setFile(destFile);
            saveFileInfo.setFile(saveFile);
            if (! saveFileInfo.exists()) {
                if (fileInfo.exists()) {
                    QFile file(fileInfo.absoluteFilePath());
                    if (! file.rename(saveFile) || ! QFile::copy(targetFile,destFile)) {
                        logError() << QString("Could not copy %1 to %2").arg(targetFile).arg(destFile);
                    } else {
                        logInfo() << QString("Copied 'portable' POVRay conf file %1").arg(destFile);
                    }
                } else if (! QFile::copy(targetFile,destFile)) {
                    logError() << QString("Could not copy %1 to %2").arg(targetFile).arg(destFile);
                }
            }
            targetFile = QString("%1/povray.ini").arg(targetFolder);
            destFile   = QString("%1/povray.ini").arg(destFolder);
            saveFile   = QString("%1/povray_install.ini").arg(destFolder);
            fileInfo.setFile(destFile);
            saveFileInfo.setFile(saveFile);
            if (! saveFileInfo.exists()) {
                if (fileInfo.exists()) {
                    QFile file(fileInfo.absoluteFilePath());
                    if (! file.rename(saveFile) || ! QFile::copy(targetFile,destFile)) {
                        logError() << QString("Could not copy %1 to %2").arg(targetFile).arg(destFile);
                    } else {
                        logInfo() << QString("Copied 'portable' POVRay ini file %1").arg(destFile);
                    }
                } else if (! QFile::copy(targetFile,destFile)) {
                    logError() << QString("Could not copy %1 to %2").arg(targetFile).arg(destFile);
                }
            }
        } else {
            saveFile = QString("%1/povray_install.conf").arg(destFolder);
            saveFileInfo.setFile(saveFile);
            if (saveFileInfo.exists()) {
                destFile = QString("%1/povray.conf").arg(destFolder);
                fileInfo.setFile(destFile);
                QFile file2(saveFileInfo.absoluteFilePath());
                if (fileInfo.exists()) {
                    QFile file(fileInfo.absoluteFilePath());
                    if (! file.remove(destFile) || ! file2.rename(destFile)) {
                        logError() << QString("Could not rename %1 to %2").arg(targetFile).arg(destFile);
                    } else {
                        logInfo() << QString("Renamed 'installed' POVRay conf file %1").arg(destFile);
                    }
                } else if (! file2.rename(destFile)) {
                    logError() << QString("Could not rename %1 to %2").arg(targetFile).arg(destFile);
                }
            }
            saveFile = QString("%1/povray_install.ini").arg(destFolder);
            saveFileInfo.setFile(saveFile);
            if (saveFileInfo.exists()) {
                destFile = QString("%1/povray.ini").arg(destFolder);
                fileInfo.setFile(destFile);
                QFile file2(saveFileInfo.absoluteFilePath());
                if (fileInfo.exists()) {
                    QFile file(fileInfo.absoluteFilePath());
                    if (! file.remove(destFile) || ! file2.rename(destFile)) {
                        logError() << QString("Could not rename %1 to %2").arg(targetFile).arg(destFile);
                    } else {
                        logInfo() << QString("Renamed 'installed' POVRay ini file %1").arg(destFile);
                    }
                } else if (! file2.rename(destFile)) {
                    logError() << QString("Could not rename %1 to %2").arg(targetFile).arg(destFile);
                }
            }

        }
    }
#endif
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
    confFileOut.setFileName(QString("%1/%2/config/%3").arg(lpub3d3rdPartyConfigDir, VER_POVRAY_STR, resourceFile.fileName()));
    if (confFileIn.open(QIODevice::ReadOnly) && confFileOut.open(QIODevice::WriteOnly | QIODevice::Text))
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
    bool pliOk[3] = { true, true, true };
    QFileInfo pliInfo;
    QSettings Settings;
    pliControlFile = Settings.value(QString("%1/%2").arg(SETTINGS,"PliControlFile")).toString();
    pliSubstitutePartsFile = Settings.value(QString("%1/%2").arg(SETTINGS,"PliSubstitutePartsFile")).toString();
    excludedPartsFile = Settings.value(QString("%1/%2").arg(SETTINGS,"ExcludedPartsFile")).toString();

    pliInfo.setFile(pliControlFile);
    if (! pliInfo.exists()) {
        Settings.remove(QString("%1/%2").arg(SETTINGS,"PliControlFile"));
        pliOk[0] = false;
    }

    pliInfo.setFile(pliSubstitutePartsFile);
    if (! pliInfo.exists()) {
        Settings.remove(QString("%1/%2").arg(SETTINGS,"PliSubstitutePartsFile"));
        pliOk[1] = false;
    }

    pliInfo.setFile(excludedPartsFile);
    if (! pliInfo.exists()) {
        Settings.remove(QString("%1/%2").arg(SETTINGS,"ExcludedPliPartsFile"));
        pliOk[2] = false;
    }

    if (pliOk[0] && pliOk[1] && pliOk[2])
        return;

    if (! pliOk[0]) {
        pliControlFile = QString("%1/extras/%2").arg(lpubDataPath,validPliControl);
        pliInfo.setFile(pliControlFile);
        if (pliInfo.exists()) {
            Settings.setValue(QString("%1/%2").arg(SETTINGS,"PliControlFile"),pliControlFile);
        } else {
            pliControlFile = pliInfo.absolutePath()+"/"+VER_PLI_CONTROL_FILE;
            Settings.setValue(QString("%1/%2").arg(SETTINGS,"PliControlFile"),pliControlFile);
        }
    }

    if (! pliOk[1]) {
        pliSubstitutePartsFile = QString("%1/extras/%2").arg(lpubDataPath,validPliSubstituteParts);
        pliInfo.setFile(pliSubstitutePartsFile);
        if (pliInfo.exists()) {
            Settings.setValue(QString("%1/%2").arg(SETTINGS,"PliSubstitutePartsFile"),pliSubstitutePartsFile);
        } else {
            pliSubstitutePartsFile = pliInfo.absolutePath()+"/"+VER_PLI_SUBSTITUTE_PARTS_FILE;
            Settings.setValue(QString("%1/%2").arg(SETTINGS,"PliSubstitutePartsFile"),pliSubstitutePartsFile);
        }
    }

    if (! pliOk[2]) {
        excludedPartsFile = QString("%1/extras/%2").arg(lpubDataPath,validExcludedPliParts);
        pliInfo.setFile(excludedPartsFile);
        if (pliInfo.exists()) {
            Settings.setValue(QString("%1/%2").arg(SETTINGS,"ExcludedPartsFile"),excludedPartsFile);
        } else {
            excludedPartsFile = pliInfo.absolutePath()+"/"+VER_EXCLUDED_PARTS_FILE;
            Settings.setValue(QString("%1/%2").arg(SETTINGS,"ExcludedPartsFile"),excludedPartsFile);
        }
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

  QString const displayThemeKey("DisplayTheme");
  if ( ! Settings.contains(QString("%1/%2").arg(SETTINGS,displayThemeKey))) {
          displayTheme = THEME_DEFAULT;
          Settings.setValue(QString("%1/%2").arg(SETTINGS,displayThemeKey),displayTheme);
  } else {
          displayTheme = Settings.value(QString("%1/%2").arg(SETTINGS,displayThemeKey)).toString();
  }

  QString const themeAutoRestartKey("ThemeAutoRestart");
  if ( ! Settings.contains(QString("%1/%2").arg(SETTINGS,themeAutoRestartKey))) {
          QVariant uValue(false);
          themeAutoRestart = false;
          Settings.setValue(QString("%1/%2").arg(SETTINGS,themeAutoRestartKey),uValue);
  } else {
          themeAutoRestart = Settings.value(QString("%1/%2").arg(SETTINGS,themeAutoRestartKey)).toBool();
  }

  QString const sceneRulerKey("SceneRuler");
  if ( ! Settings.contains(QString("%1/%2").arg(SETTINGS,sceneRulerKey))) {
          QVariant uValue(false);
          sceneRuler = false;
          Settings.setValue(QString("%1/%2").arg(SETTINGS,sceneRulerKey),uValue);
  } else {
          sceneRuler = Settings.value(QString("%1/%2").arg(SETTINGS,sceneRulerKey)).toBool();
  }

  QString const sceneRulerTrackingKey("SceneRulerTracking");
  if ( ! Settings.contains(QString("%1/%2").arg(SETTINGS,sceneRulerTrackingKey))) {
          sceneRulerTracking = TRACKING_NONE;
          Settings.setValue(QString("%1/%2").arg(SETTINGS,sceneRulerTrackingKey),sceneRulerTracking);
  } else {
          sceneRulerTracking = Settings.value(QString("%1/%2").arg(SETTINGS,sceneRulerTrackingKey)).toInt();
  }

  QString const sceneGuidesKey("SceneGuides");
  if ( ! Settings.contains(QString("%1/%2").arg(SETTINGS,sceneGuidesKey))) {
          QVariant uValue(false);
          sceneGuides = false;
          Settings.setValue(QString("%1/%2").arg(SETTINGS,sceneGuidesKey),uValue);
  } else {
          sceneGuides = Settings.value(QString("%1/%2").arg(SETTINGS,sceneGuidesKey)).toBool();
  }

  QString const sceneGuidesLineKey("SceneGuidesLine");
  if ( ! Settings.contains(QString("%1/%2").arg(SETTINGS,sceneGuidesLineKey))) {
          sceneGuidesLine = int(Qt::DashLine);
          Settings.setValue(QString("%1/%2").arg(SETTINGS,sceneGuidesLineKey),sceneGuidesLine);
  } else {
          sceneGuidesLine = Settings.value(QString("%1/%2").arg(SETTINGS,sceneGuidesLineKey)).toInt();
  }

  QString const sceneGuidesPositionKey("SceneGuidesPosition");
  if ( ! Settings.contains(QString("%1/%2").arg(SETTINGS,sceneGuidesPositionKey))) {
      sceneGuidesPosition = int(GUIDES_TOP_LEFT);
      Settings.setValue(QString("%1/%2").arg(SETTINGS,sceneGuidesPositionKey),sceneGuidesPosition);
  } else {
      sceneGuidesPosition = Settings.value(QString("%1/%2").arg(SETTINGS,sceneGuidesPositionKey)).toInt();
  }

  QString const customSceneBackgroundColorKey("CustomSceneBackgroundColor");
  if ( ! Settings.contains(QString("%1/%2").arg(SETTINGS,customSceneBackgroundColorKey))) {
          QVariant uValue(false);
          customSceneBackgroundColor = false;
          Settings.setValue(QString("%1/%2").arg(SETTINGS,customSceneBackgroundColorKey),uValue);
  } else {
          customSceneBackgroundColor = Settings.value(QString("%1/%2").arg(SETTINGS,customSceneBackgroundColorKey)).toBool();
  }

  QString const customSceneGridColorKey("CustomSceneGridColor");
  if ( ! Settings.contains(QString("%1/%2").arg(SETTINGS,customSceneGridColorKey))) {
          QVariant uValue(false);
          customSceneGridColor = false;
          Settings.setValue(QString("%1/%2").arg(SETTINGS,customSceneGridColorKey),uValue);
  } else {
          customSceneGridColor = Settings.value(QString("%1/%2").arg(SETTINGS,customSceneGridColorKey)).toBool();
  }

  QString const customSceneRulerTickColorKey("CustomSceneRulerTickColor");
  if ( ! Settings.contains(QString("%1/%2").arg(SETTINGS,customSceneRulerTickColorKey))) {
          QVariant uValue(false);
          customSceneRulerTickColor = false;
          Settings.setValue(QString("%1/%2").arg(SETTINGS,customSceneRulerTickColorKey),uValue);
  } else {
          customSceneRulerTickColor = Settings.value(QString("%1/%2").arg(SETTINGS,customSceneRulerTickColorKey)).toBool();
  }

  QString const customSceneRulerTrackingColorKey("CustomSceneRulerTrackingColor");
  if ( ! Settings.contains(QString("%1/%2").arg(SETTINGS,customSceneRulerTrackingColorKey))) {
          QVariant uValue(false);
          customSceneRulerTrackingColor = false;
          Settings.setValue(QString("%1/%2").arg(SETTINGS,customSceneRulerTrackingColorKey),uValue);
  } else {
          customSceneRulerTrackingColor = Settings.value(QString("%1/%2").arg(SETTINGS,customSceneRulerTrackingColorKey)).toBool();
  }

  QString const customSceneGuideColorKey("CustomSceneGuideColor");
  if ( ! Settings.contains(QString("%1/%2").arg(SETTINGS,customSceneGuideColorKey))) {
          QVariant uValue(false);
          customSceneGuideColor = false;
          Settings.setValue(QString("%1/%2").arg(SETTINGS,customSceneGuideColorKey),uValue);
  } else {
          customSceneGuideColor = Settings.value(QString("%1/%2").arg(SETTINGS,customSceneGuideColorKey)).toBool();
  }

  QString const sceneBackgroundColorKey("SceneBackgroundColor");
  if ( ! Settings.contains(QString("%1/%2").arg(SETTINGS,sceneBackgroundColorKey))) {
          displayTheme == THEME_DARK ?
          sceneBackgroundColor = THEME_SCENE_BGCOLOR_DARK :
          sceneBackgroundColor = THEME_SCENE_BGCOLOR_DEFAULT;
          Settings.setValue(QString("%1/%2").arg(SETTINGS,sceneBackgroundColorKey),sceneBackgroundColor);
  } else {
          sceneBackgroundColor = Settings.value(QString("%1/%2").arg(SETTINGS,sceneBackgroundColorKey)).toString();
  }

  QString const sceneGridColorKey("SceneGridColor");
  if ( ! Settings.contains(QString("%1/%2").arg(SETTINGS,sceneGridColorKey))) {
          displayTheme == THEME_DARK ?
          sceneGridColor = THEME_GRID_PEN_DARK :
          sceneGridColor = THEME_GRID_PEN_DEFAULT;
          Settings.setValue(QString("%1/%2").arg(SETTINGS,sceneGridColorKey),sceneGridColor);
  } else {
          sceneGridColor = Settings.value(QString("%1/%2").arg(SETTINGS,sceneGridColorKey)).toString();
  }

  QString const sceneRulerTickColorKey("SceneRulerTickColor");
  if ( ! Settings.contains(QString("%1/%2").arg(SETTINGS,sceneRulerTickColorKey))) {
          displayTheme == THEME_DARK ?
          sceneRulerTickColor = THEME_RULER_TICK_PEN_DARK :
          sceneRulerTickColor = THEME_RULER_TICK_PEN_DEFAULT;
          Settings.setValue(QString("%1/%2").arg(SETTINGS,sceneRulerTickColorKey),sceneRulerTickColor);
  } else {
          sceneRulerTickColor = Settings.value(QString("%1/%2").arg(SETTINGS,sceneRulerTickColorKey)).toString();
  }

  QString const sceneRulerTrackingColorKey("SceneRulerTrackingColor");
  if ( ! Settings.contains(QString("%1/%2").arg(SETTINGS,sceneRulerTrackingColorKey))) {
          displayTheme == THEME_DARK ?
          sceneRulerTrackingColor = THEME_RULER_TRACK_PEN_DARK :
          sceneRulerTrackingColor = THEME_RULER_TRACK_PEN_DEFAULT;
          Settings.setValue(QString("%1/%2").arg(SETTINGS,sceneRulerTrackingColorKey),sceneRulerTrackingColor);
  } else {
          sceneRulerTrackingColor = Settings.value(QString("%1/%2").arg(SETTINGS,sceneRulerTrackingColorKey)).toString();
  }

  QString const sceneGuideColorKey("SceneGuideColor");
  if ( ! Settings.contains(QString("%1/%2").arg(SETTINGS,sceneGuideColorKey))) {
          displayTheme == THEME_DARK ?
          sceneGuideColor = THEME_GUIDE_PEN_DARK :
          sceneGuideColor = THEME_GUIDE_PEN_DEFAULT;
          Settings.setValue(QString("%1/%2").arg(SETTINGS,sceneGuideColorKey),sceneGuideColor);
  } else {
          sceneGuideColor = Settings.value(QString("%1/%2").arg(SETTINGS,sceneGuideColorKey)).toString();
  }

  QString const snapToGridKey("SnapToGrid");
  if ( ! Settings.contains(QString("%1/%2").arg(SETTINGS,snapToGridKey))) {
          QVariant uValue(false);
          snapToGrid = false;
          Settings.setValue(QString("%1/%2").arg(SETTINGS,snapToGridKey),uValue);
  } else {
          snapToGrid = Settings.value(QString("%1/%2").arg(SETTINGS,snapToGridKey)).toBool();
  }

  QString const hidePageBackgroundKey("HidePageBackground");
  if ( ! Settings.contains(QString("%1/%2").arg(SETTINGS,hidePageBackgroundKey))) {
          QVariant uValue(false);
          hidePageBackground = false;
          Settings.setValue(QString("%1/%2").arg(SETTINGS,hidePageBackgroundKey),uValue);
  } else {
          hidePageBackground = Settings.value(QString("%1/%2").arg(SETTINGS,hidePageBackgroundKey)).toBool();
  }

  QString const showGuidesCoordinatesKey("ShowGuidesCoordinates");
  if ( ! Settings.contains(QString("%1/%2").arg(SETTINGS,showGuidesCoordinatesKey))) {
          QVariant uValue(false);
          showGuidesCoordinates = false;
          Settings.setValue(QString("%1/%2").arg(SETTINGS,showGuidesCoordinatesKey),uValue);
  } else {
          showGuidesCoordinates = Settings.value(QString("%1/%2").arg(SETTINGS,showGuidesCoordinatesKey)).toBool();
  }

  QString const showTrackingCoordinatesKey("ShowTrackingCoordinates");
  if ( ! Settings.contains(QString("%1/%2").arg(SETTINGS,showTrackingCoordinatesKey))) {
          QVariant uValue(false);
          showTrackingCoordinates = false;
          Settings.setValue(QString("%1/%2").arg(SETTINGS,showTrackingCoordinatesKey),uValue);
  } else {
          showTrackingCoordinates = Settings.value(QString("%1/%2").arg(SETTINGS,showTrackingCoordinatesKey)).toBool();
  }

  QString const gridSizeIndexKey("GridSizeIndex");
  if ( ! Settings.contains(QString("%1/%2").arg(SETTINGS,gridSizeIndexKey))) {
      gridSizeIndex = GRID_SIZE_INDEX_DEFAULT;
      Settings.setValue(QString("%1/%2").arg(SETTINGS,gridSizeIndexKey),gridSizeIndex);
  } else {
      gridSizeIndex = Settings.value(QString("%1/%2").arg(SETTINGS,gridSizeIndexKey)).toInt();
  }

  QString const showParseErrorsKey("ShowParseErrors");
  if ( ! Settings.contains(QString("%1/%2").arg(SETTINGS,showParseErrorsKey))) {
          QVariant uValue(true);
          showParseErrors = true;
          Settings.setValue(QString("%1/%2").arg(SETTINGS,showParseErrorsKey),uValue);
  } else {
          showParseErrors = Settings.value(QString("%1/%2").arg(SETTINGS,showParseErrorsKey)).toBool();
  }

  QString const showAnnotationMessagesKey("ShowAnnotationMessages");
  if ( ! Settings.contains(QString("%1/%2").arg(SETTINGS,showAnnotationMessagesKey))) {
          QVariant uValue(true);
          showAnnotationMessages = true;
          Settings.setValue(QString("%1/%2").arg(SETTINGS,showAnnotationMessagesKey),uValue);
  } else {
          showAnnotationMessages = Settings.value(QString("%1/%2").arg(SETTINGS,showAnnotationMessagesKey)).toBool();
  }

  QString const showSaveOnRedrawKey("ShowSaveOnRedraw");
  if ( ! Settings.contains(QString("%1/%2").arg(SETTINGS,showSaveOnRedrawKey))) {
          QVariant uValue(true);
          showSaveOnRedraw = true;
          Settings.setValue(QString("%1/%2").arg(SETTINGS,showSaveOnRedrawKey),uValue);
  } else {
          showSaveOnRedraw = Settings.value(QString("%1/%2").arg(SETTINGS,showSaveOnRedrawKey)).toBool();
  }

  QString const showSaveOnUpdateKey("ShowSaveOnUpdate");
  if ( ! Settings.contains(QString("%1/%2").arg(SETTINGS,showSaveOnUpdateKey))) {
          QVariant uValue(true);
          showSaveOnUpdate = true;
          Settings.setValue(QString("%1/%2").arg(SETTINGS,showSaveOnUpdateKey),uValue);
  } else {
          showSaveOnUpdate = Settings.value(QString("%1/%2").arg(SETTINGS,showSaveOnUpdateKey)).toBool();
  }

  QString const showSubmodelsKey("ShowSubmodels");
  if (Settings.contains(QString("%1/%2").arg(SETTINGS,showSubmodelsKey))) {
      showSubmodels = Settings.value(QString("%1/%2").arg(SETTINGS,showSubmodelsKey)).toBool();
  }

  QString const showTopModelKey("ShowTopModel");
  if (Settings.contains(QString("%1/%2").arg(SETTINGS,showTopModelKey))) {
      showTopModel = Settings.value(QString("%1/%2").arg(SETTINGS,showTopModelKey)).toBool();
  }

  QString const showInstanceCountKey("ShowInstanceCount");
  if (Settings.contains(QString("%1/%2").arg(SETTINGS,showInstanceCountKey))) {
      showInstanceCount = Settings.value(QString("%1/%2").arg(SETTINGS,showInstanceCountKey)).toBool();
  }

  QString const loadLastOpenedFileKey("LoadLastOpenedFile");
  if ( ! Settings.contains(QString("%1/%2").arg(SETTINGS,loadLastOpenedFileKey))) {
      QVariant uValue(false);
      loadLastOpenedFile = false;
      Settings.setValue(QString("%1/%2").arg(SETTINGS,loadLastOpenedFileKey),uValue);
  } else {
      loadLastOpenedFile = Settings.value(QString("%1/%2").arg(SETTINGS,loadLastOpenedFileKey)).toBool();
  }

  QString const povrayFileGeneratorKey("ExtendedSubfileSearch");
  if ( ! Settings.contains(QString("%1/%2").arg(SETTINGS,povrayFileGeneratorKey))) {
      QVariant uValue(false);
      extendedSubfileSearch = false;
      Settings.setValue(QString("%1/%2").arg(SETTINGS,povrayFileGeneratorKey),uValue);
  } else {
      extendedSubfileSearch = Settings.value(QString("%1/%2").arg(SETTINGS,povrayFileGeneratorKey)).toBool();
  }

  QString const ldrawFilesLoadMsgsKey("LdrawFilesLoadMsgs");
  if ( ! Settings.contains(QString("%1/%2").arg(SETTINGS,ldrawFilesLoadMsgsKey))) {
      ldrawFilesLoadMsgs = NEVER_SHOW;
      Settings.setValue(QString("%1/%2").arg(SETTINGS,ldrawFilesLoadMsgsKey),ldrawFilesLoadMsgs);
  } else {
      ldrawFilesLoadMsgs = Settings.value(QString("%1/%2").arg(SETTINGS,ldrawFilesLoadMsgsKey)).toInt();
  }

  QString const saveOnRedrawKey("SaveOnRedraw");
  if ( ! Settings.contains(QString("%1/%2").arg(SETTINGS,saveOnRedrawKey))) {
      QVariant uValue(true);
      saveOnRedraw = true;
      Settings.setValue(QString("%1/%2").arg(SETTINGS,saveOnRedrawKey),uValue);
  } else {
      saveOnRedraw = Settings.value(QString("%1/%2").arg(SETTINGS,saveOnRedrawKey)).toBool();
  }

  QString const saveOnUpdateKey("SaveOnUpdate");
  if ( ! Settings.contains(QString("%1/%2").arg(SETTINGS,saveOnUpdateKey))) {
      QVariant uValue(true);
      saveOnUpdate = true;
      Settings.setValue(QString("%1/%2").arg(SETTINGS,saveOnUpdateKey),uValue);
  } else {
      saveOnUpdate = Settings.value(QString("%1/%2").arg(SETTINGS,saveOnUpdateKey)).toBool();
  }
}

void Preferences::setShowParseErrorsPreference(bool b)
{
  QSettings Settings;
  showParseErrors = b;
  QVariant uValue(b);
  QString const showParseErrorsKey("ShowParseErrors");
  Settings.setValue(QString("%1/%2").arg(SETTINGS,showParseErrorsKey),uValue);
}

void Preferences::setShowAnnotationMessagesPreference(bool b)
{
QSettings Settings;
showAnnotationMessages = b;
QVariant uValue(b);
QString const showAnnotationMessagesKey("ShowAnnotationMessages");
Settings.setValue(QString("%1/%2").arg(SETTINGS,showAnnotationMessagesKey),uValue);
}

void Preferences::setShowSaveOnRedrawPreference(bool b)
{
  QSettings Settings;
  showSaveOnRedraw = b;
  QVariant uValue(b);
  QString const showSaveOnRedrawKey("ShowSaveOnRedraw");
  Settings.setValue(QString("%1/%2").arg(SETTINGS,showSaveOnRedrawKey),uValue);
}

void Preferences::setShowSaveOnUpdatePreference(bool b)
{
  QSettings Settings;
  showSaveOnUpdate = b;
  QVariant uValue(b);
  QString const showSaveOnUpdateKey("ShowSaveOnUpdate");
  Settings.setValue(QString("%1/%2").arg(SETTINGS,showSaveOnUpdateKey),uValue);
}

void Preferences::setSnapToGridPreference(bool b)
{
  QSettings Settings;
  snapToGrid = b;
  QVariant uValue(b);
  QString const snapToGridKey("SnapToGrid");
  Settings.setValue(QString("%1/%2").arg(SETTINGS,snapToGridKey),uValue);
}

void Preferences::setHidePageBackgroundPreference(bool b)
{
  QSettings Settings;
  hidePageBackground = b;
  QVariant uValue(b);
  QString const hidePageBackgroundKey("HidePageBackground");
  Settings.setValue(QString("%1/%2").arg(SETTINGS,hidePageBackgroundKey),uValue);
}

void Preferences::setShowGuidesCoordinatesPreference(bool b)
{
  QSettings Settings;
  showGuidesCoordinates = b;
  QVariant uValue(b);
  QString const showGuidesCoordinatesKey("ShowGuidesCoordinates");
  Settings.setValue(QString("%1/%2").arg(SETTINGS,showGuidesCoordinatesKey),uValue);
}

void Preferences::setShowTrackingCoordinatesPreference(bool b)
{
  QSettings Settings;
  showTrackingCoordinates = b;
  QVariant uValue(b);
  QString const showTrackingCoordinatesKey("ShowTrackingCoordinates");
  Settings.setValue(QString("%1/%2").arg(SETTINGS,showTrackingCoordinatesKey),uValue);
}

void Preferences::setGridSizeIndexPreference(int i)
{
  QSettings Settings;
  gridSizeIndex = i;
  QVariant uValue(i);
  QString const gridSizeIndex("GridSizeIndex");
  Settings.setValue(QString("%1/%2").arg(SETTINGS,gridSizeIndex),uValue);
}

void Preferences::setSceneGuidesPreference(bool b)
{
  QSettings Settings;
  sceneGuides = b;
  QVariant uValue(b);
  QString const sceneGuidesKey("SceneGuides");
  Settings.setValue(QString("%1/%2").arg(SETTINGS,sceneGuidesKey),uValue);
}

void Preferences::setSceneGuidesLinePreference(int i)
{
  QSettings Settings;
  sceneGuidesLine = i;
  QVariant uValue(i);
  QString const sceneGuidesLineKey("SceneGuidesLine");
  Settings.setValue(QString("%1/%2").arg(SETTINGS,sceneGuidesLineKey),uValue);
}

void Preferences::setSceneGuidesPositionPreference(int i)
{
  QSettings Settings;
  sceneGuidesPosition = i;
  QVariant uValue(i);
  QString const sceneGuidesPositionKey("SceneGuidesPosition");
  Settings.setValue(QString("%1/%2").arg(SETTINGS,sceneGuidesPositionKey),uValue);
}

void Preferences::setSceneRulerPreference(bool b)
{
  QSettings Settings;
  sceneRuler = b;
  QVariant uValue(b);
  QString const sceneRulerKey("SceneRuler");
  Settings.setValue(QString("%1/%2").arg(SETTINGS,sceneRulerKey),uValue);
}

void Preferences::setSceneRulerTrackingPreference(int i)
{
  QSettings Settings;
  sceneRulerTracking = i;
  QString const sceneRulerTrackingKey("SceneRulerTracking");
  Settings.setValue(QString("%1/%2").arg(SETTINGS,sceneRulerTrackingKey),sceneRulerTracking);
}

void Preferences::setCustomSceneBackgroundColorPreference(bool b)
{
  QSettings Settings;
  customSceneBackgroundColor = b;
  QVariant uValue(b);
  QString const sceneBackgroundColorKey("CustomSceneBackgroundColor");
  Settings.setValue(QString("%1/%2").arg(SETTINGS,sceneBackgroundColorKey),uValue);
}

void Preferences::setCustomSceneGridColorPreference(bool b)
{
  QSettings Settings;
  customSceneGridColor = b;
  QVariant uValue(b);
  QString const sceneGridColorKey("CustomSceneGridColor");
  Settings.setValue(QString("%1/%2").arg(SETTINGS,sceneGridColorKey),uValue);
}

void Preferences::setCustomSceneRulerTickColorPreference(bool b)
{
  QSettings Settings;
  customSceneRulerTickColor = b;
  QVariant uValue(b);
  QString const sceneRulerTickColorKey("CustomSceneRulerTickColor");
  Settings.setValue(QString("%1/%2").arg(SETTINGS,sceneRulerTickColorKey),uValue);
}

void Preferences::setCustomSceneRulerTrackingColorPreference(bool b)
{
  QSettings Settings;
  customSceneRulerTrackingColor = b;
  QVariant uValue(b);
  QString const sceneRulerTrackingColorKey("CustomSceneRulerTrackingColor");
  Settings.setValue(QString("%1/%2").arg(SETTINGS,sceneRulerTrackingColorKey),uValue);
}

void Preferences::setCustomSceneGuideColorPreference(bool b)
{
  QSettings Settings;
  customSceneGuideColor = b;
  QVariant uValue(b);
  QString const sceneGuideColorKey("CustomSceneGuideColor");
  Settings.setValue(QString("%1/%2").arg(SETTINGS,sceneGuideColorKey),uValue);
}

void Preferences::setSceneBackgroundColorPreference(QString s)
{
  QSettings Settings;
  sceneBackgroundColor = s;
  QVariant uValue(s);
  QString const sceneBackgroundColorKey("SceneBackgroundColor");
  Settings.setValue(QString("%1/%2").arg(SETTINGS,sceneBackgroundColorKey),uValue);
}

void Preferences::setSceneGridColorPreference(QString s)
{
  QSettings Settings;
  sceneGridColor = s;
  QVariant uValue(s);
  QString const sceneGridColorKey("SceneGridColor");
  Settings.setValue(QString("%1/%2").arg(SETTINGS,sceneGridColorKey),uValue);
}

void Preferences::setSceneRulerTickColorPreference(QString s)
{
  QSettings Settings;
  sceneRulerTickColor = s;
  QVariant uValue(s);
  QString const sceneRulerTickColorKey("SceneRulerTickColor");
  Settings.setValue(QString("%1/%2").arg(SETTINGS,sceneRulerTickColorKey),uValue);
}

void Preferences::setSceneRulerTrackingColorPreference(QString s)
{
  QSettings Settings;
  sceneRulerTrackingColor = s;
  QVariant uValue(s);
  QString const sceneRulerTrackingColorKey("SceneRulerTrackingColor");
  Settings.setValue(QString("%1/%2").arg(SETTINGS,sceneRulerTrackingColorKey),uValue);
}

void Preferences::setSceneGuideColorPreference(QString s)
{
  QSettings Settings;
  sceneGuideColor = s;
  QVariant uValue(s);
  QString const sceneGuideColorKey("SceneGuideColor");
  Settings.setValue(QString("%1/%2").arg(SETTINGS,sceneGuideColorKey),uValue);
}

void Preferences::setDebugLogging(bool b){
    debugLogging = b;
}

void Preferences::annotationPreferences()
{
    bool annoOk[10] = { true, true, true, true, true, true, true, true, true, true };
    QFileInfo annoInfo;
    QSettings Settings;
    titleAnnotationsFile    = Settings.value(QString("%1/%2").arg(SETTINGS,"TitleAnnotationFile")).toString();
    freeformAnnotationsFile = Settings.value(QString("%1/%2").arg(SETTINGS,"FreeFormAnnotationsFile")).toString();
    annotationStyleFile     = Settings.value(QString("%1/%2").arg(SETTINGS,"AnnotationStyleFile")).toString();

    blCodesFile             = Settings.value(QString("%1/%2").arg(SETTINGS,"BLCodesFile")).toString();
    legoElementsFile        = Settings.value(QString("%1/%2").arg(SETTINGS,"LEGOElementsFile")).toString();
    blColorsFile            = Settings.value(QString("%1/%2").arg(SETTINGS,"BLColorsFile")).toString();
    ld2blColorsXRefFile     = Settings.value(QString("%1/%2").arg(SETTINGS,"LD2BLColorsXRefFile")).toString();
    ld2blCodesXRefFile      = Settings.value(QString("%1/%2").arg(SETTINGS,"LD2BLCodesXRefFile")).toString();
    ld2rbColorsXRefFile     = Settings.value(QString("%1/%2").arg(SETTINGS,"LD2RBColorsXRefFile")).toString();
    ld2rbCodesXRefFile      = Settings.value(QString("%1/%2").arg(SETTINGS,"LD2RBCodesXRefFile")).toString();

    annoInfo.setFile(titleAnnotationsFile);
    if (! annoInfo.exists()) {
        Settings.remove(QString("%1/%2").arg(SETTINGS,"TitleAnnotationFile"));
        annoOk[0] = false;
    }

    annoInfo.setFile(freeformAnnotationsFile);
    if (! annoInfo.exists()) {
        Settings.remove(QString("%1/%2").arg(SETTINGS,"FreeFormAnnotationsFile"));
        annoOk[1] = false;
    }

    annoInfo.setFile(annotationStyleFile);
    if (! annoInfo.exists()) {
        Settings.remove(QString("%1/%2").arg(SETTINGS,"AnnotationStyleFile"));
        annoOk[2] = false;
    }

    annoInfo.setFile(blCodesFile);
    if (! annoInfo.exists()) {
        Settings.remove(QString("%1/%2").arg(SETTINGS,"BLCodesFile"));
        annoOk[3] = false;
    }

    annoInfo.setFile(legoElementsFile);
    if (! annoInfo.exists()) {
        Settings.remove(QString("%1/%2").arg(SETTINGS,"LEGOElementsFile"));
        annoOk[4] = false;
    }

    annoInfo.setFile(blColorsFile);
    if (! annoInfo.exists()) {
        Settings.remove(QString("%1/%2").arg(SETTINGS,"BLColorsFile"));
        annoOk[5] = false;
    }

    annoInfo.setFile(ld2blColorsXRefFile);
    if (! annoInfo.exists()) {
        Settings.remove(QString("%1/%2").arg(SETTINGS,"LD2BLColorsXRefFile"));
        annoOk[6] = false;
    }

    annoInfo.setFile(ld2blCodesXRefFile);
    if (! annoInfo.exists()) {
        Settings.remove(QString("%1/%2").arg(SETTINGS,"LD2BLCodesXRefFile"));
        annoOk[7] = false;
    }

    annoInfo.setFile(ld2rbColorsXRefFile);
    if (! annoInfo.exists()) {
        Settings.remove(QString("%1/%2").arg(SETTINGS,"LD2RBColorsXRefFile"));
        annoOk[8] = false;
    }

    annoInfo.setFile(ld2rbCodesXRefFile);
    if (! annoInfo.exists()) {
        Settings.remove(QString("%1/%2").arg(SETTINGS,"LD2RBCodesXRefFile"));
        annoOk[9] = false;
    }

    if (annoOk[0] && annoOk[1] && annoOk[2] && annoOk[3] &&
        annoOk[4] && annoOk[5] && annoOk[6] && annoOk[7] &&
        annoOk[8] && annoOk[9])
        return;

    if (! annoOk[3]) {
        blCodesFile = QString("%1/extras/%2").arg(lpubDataPath,VER_LPUB3D_BLCODES_FILE);
        annoInfo.setFile(blCodesFile);
        if (annoInfo.exists()) {
            Settings.setValue(QString("%1/%2").arg(SETTINGS,"BLCodesFile"),blCodesFile);
        } else {
            blCodesFile = QString();
        }
        annoOk[3] = true;
    }

    if (! annoOk[4]) {
        legoElementsFile = QString("%1/extras/%2").arg(lpubDataPath,VER_LPUB3D_LEGOELEMENTS_FILE);
        annoInfo.setFile(legoElementsFile);
        if (annoInfo.exists()) {
            Settings.setValue(QString("%1/%2").arg(SETTINGS,"LEGOElementsFile"),legoElementsFile);
        } else {
            legoElementsFile = QString();
        }
        annoOk[4] = true;
    }

    if (! annoOk[5]) {
        blColorsFile = QString("%1/extras/%2").arg(lpubDataPath,VER_LPUB3D_BLCOLORS_FILE);
        annoInfo.setFile(blColorsFile);
        if (annoInfo.exists()) {
            Settings.setValue(QString("%1/%2").arg(SETTINGS,"BLColorsFile"),blColorsFile);
        } else {
            blColorsFile = QString();
        }
        annoOk[5] = true;
    }

    if (! annoOk[6]) {
        ld2blColorsXRefFile = QString("%1/extras/%2").arg(lpubDataPath,VER_LPUB3D_LD2BLCOLORSXREF_FILE);
        annoInfo.setFile(ld2blColorsXRefFile);
        if (annoInfo.exists()) {
            Settings.setValue(QString("%1/%2").arg(SETTINGS,"LD2BLColorsXRefFile"),ld2blColorsXRefFile);
        } else {
            ld2blColorsXRefFile = QString();
        }
        annoOk[6] = true;
    }

    if (! annoOk[7]) {
        ld2blCodesXRefFile = QString("%1/extras/%2").arg(lpubDataPath,VER_LPUB3D_LD2BLCODESXREF_FILE);
        annoInfo.setFile(ld2blCodesXRefFile);
        if (annoInfo.exists()) {
            Settings.setValue(QString("%1/%2").arg(SETTINGS,"LD2BLCodesXRefFile"),ld2blCodesXRefFile);
        } else {
            ld2blCodesXRefFile = QString();
        }
        annoOk[7] = true;
    }

    if (! annoOk[8]) {
        ld2rbColorsXRefFile = QString("%1/extras/%2").arg(lpubDataPath,VER_LPUB3D_LD2RBCOLORSXREF_FILE);
        annoInfo.setFile(ld2rbColorsXRefFile);
        if (annoInfo.exists()) {
            Settings.setValue(QString("%1/%2").arg(SETTINGS,"LD2RBColorsXRefFile"),ld2rbColorsXRefFile);
        } else {
            ld2rbColorsXRefFile = QString();
        }
        annoOk[8] = true;
    }

    if (! annoOk[9]) {
        ld2rbCodesXRefFile = QString("%1/extras/%2").arg(lpubDataPath,VER_LPUB3D_LD2RBCODESXREF_FILE);
        annoInfo.setFile(ld2rbCodesXRefFile);
        if (annoInfo.exists()) {
            Settings.setValue(QString("%1/%2").arg(SETTINGS,"LD2RBCodesXRefFile"),ld2rbCodesXRefFile);
        } else {
            ld2rbCodesXRefFile = QString();
        }
        annoOk[9] = true;
    }

    if (annoOk[0] && annoOk[1] && annoOk[2])
        return;

    if (! annoOk[0]) {
        titleAnnotationsFile = QString("%1/extras/%2").arg(lpubDataPath,validTitleAnnotations);
        annoInfo.setFile(titleAnnotationsFile);
        if (annoInfo.exists()) {
            Settings.setValue(QString("%1/%2").arg(SETTINGS,"TitleAnnotationFile"),titleAnnotationsFile);
        } else {
            titleAnnotationsFile = annoInfo.absolutePath()+"/"+VER_TITLE_ANNOTATIONS_FILE;
            Settings.setValue(QString("%1/%2").arg(SETTINGS,"TitleAnnotationFile"),titleAnnotationsFile);
        }
    }

    if (! annoOk[1]) {
        freeformAnnotationsFile = QString("%1/extras/%2").arg(lpubDataPath,validFreeFormAnnotations);
        annoInfo.setFile(freeformAnnotationsFile);
        if (annoInfo.exists()) {
            Settings.setValue(QString("%1/%2").arg(SETTINGS,"FreeFormAnnotationsFile"),freeformAnnotationsFile);
        } else {
            freeformAnnotationsFile = annoInfo.absolutePath()+"/"+VER_FREEFOM_ANNOTATIONS_FILE;
            Settings.setValue(QString("%1/%2").arg(SETTINGS,"FreeFormAnnotationsFile"),freeformAnnotationsFile);
        }
    }

    if (! annoOk[2]) {
        annotationStyleFile = QString("%1/extras/%2").arg(lpubDataPath,validAnnotationStyleFile);
        annoInfo.setFile(annotationStyleFile);
        if (annoInfo.exists()) {
            Settings.setValue(QString("%1/%2").arg(SETTINGS,"AnnotationStyleFile"),annotationStyleFile);
        } else {
            annotationStyleFile = annoInfo.absolutePath()+"/"+VER_ANNOTATION_STYLE_FILE;
            if (annoInfo.exists()) {
                Settings.setValue(QString("%1/%2").arg(SETTINGS,"AnnotationStyleFile"),annotationStyleFile);
            } else {
                annotationStyleFile = QString();
            }
        }
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

    if (! Settings.contains(QString("%1/%2").arg(SETTINGS,fadeStepsColourKey))) {
        Settings.setValue(QString("%1/%2").arg(SETTINGS,fadeStepsColourKey),validFadeStepsColour);
    } else {
        validFadeStepsColour = Settings.value(QString("%1/%2").arg(SETTINGS,fadeStepsColourKey)).toString();
    }

    if (! Settings.contains(QString("%1/%2").arg(SETTINGS,"FadeStepsOpacity"))) {
        QVariant cValue(FADE_OPACITY_DEFAULT);
        fadeStepsOpacity = FADE_OPACITY_DEFAULT;
        Settings.setValue(QString("%1/%2").arg(SETTINGS,"FadeStepsOpacity"),cValue);
    } else {
        fadeStepsOpacity = Settings.value(QString("%1/%2").arg(SETTINGS,"FadeStepsOpacity")).toInt();
    }

    QString const LDrawColourPartsKey("LDrawColourPartsFile");
    ldrawColourPartsFile = Settings.value(QString("%1/%2").arg(SETTINGS,LDrawColourPartsKey)).toString();
    QFileInfo ldrawColorFileInfo(ldrawColourPartsFile);
    if (!ldrawColorFileInfo.exists()) {
        Settings.remove(QString("%1/%2").arg(SETTINGS,LDrawColourPartsKey));
    }

    ldrawColourPartsFile    = QDir::toNativeSeparators(QString("%1/extras/%2")
                                                         .arg(lpubDataPath)
                                                         .arg(validLDrawColorParts));
    ldrawColorFileInfo.setFile(ldrawColourPartsFile);
    if (ldrawColorFileInfo.exists()) {
        Settings.setValue(QString("%1/%2").arg(SETTINGS,LDrawColourPartsKey),ldrawColourPartsFile);
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

    if ( ! Settings.contains(QString("%1/%2").arg(SETTINGS,"HighlightFirstStep"))) {
        QVariant uValue(false);
        highlightFirstStep = false;
        Settings.setValue(QString("%1/%2").arg(SETTINGS,"HighlightFirstStep"),uValue);
    } else {
        highlightFirstStep = Settings.value(QString("%1/%2").arg(SETTINGS,"HighlightFirstStep")).toBool();
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
      if (modeGUI)
        ignoreMixedPageSizesMsg = Settings.value(QString("%1/%2").arg(DEFAULTS,"IgnoreMixedPageSizesMsg")).toBool();
      else
        ignoreMixedPageSizesMsg = false;
    }

    if ( ! Settings.contains(QString("%1/%2").arg(DEFAULTS,"PdfPageImage"))) {
      QVariant uValue(false);
      pdfPageImage = false;
      Settings.setValue(QString("%1/%2").arg(DEFAULTS,"PdfPageImage"),uValue);
    } else {
      pdfPageImage = Settings.value(QString("%1/%2").arg(DEFAULTS,"PdfPageImage")).toBool();
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
        if (modeGUI)
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

    if (!altLDConfigPath.isEmpty())
        lcSetProfileString(LC_PROFILE_COLOR_CONFIG, Preferences::altLDConfigPath);

    if (!defaultAuthor.isEmpty())
        lcSetProfileString(LC_PROFILE_DEFAULT_AUTHOR_NAME, defaultAuthor);

    if (!lpub3dLibFile.isEmpty())
        lcSetProfileString(LC_PROFILE_PARTS_LIBRARY, lpub3dLibFile);

    lcSetProfileInt(LC_PROFILE_NATIVE_PROJECTION, perspectiveProjection ? 0 : 1);

    lcSetProfileInt(LC_PROFILE_ANTIALIASING_SAMPLES, 8);

    QSettings Settings;
    if (Settings.contains(QString("%1/%2").arg(SETTINGS,"ProjectsPath")))
        lcSetProfileString(LC_PROFILE_PROJECTS_PATH, Settings.value(QString("%1/%2").arg(SETTINGS,"ProjectsPath")).toString());

    lcSetProfileInt(LC_PROFILE_SET_TARGET_POSITION, false);
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

        if (ldrawLibPath != dialog->ldrawLibPath()) {
            ldrawLibPath = dialog->ldrawLibPath();
            if (ldrawLibPath.isEmpty()) {
                Settings.remove(QString("%1/%2").arg(SETTINGS,ldrawLibPathKey));
            } else {
                Settings.setValue(QString("%1/%2").arg(SETTINGS,ldrawLibPathKey),ldrawLibPath);
            }
            // update LDView ini files
            updateLDVExportIniFile(UpdateExisting);
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

        if (pliControlFile != dialog->pliControlFile()) {
            pliControlFile = dialog->pliControlFile();
            if (pliControlFile.isEmpty()) {
                Settings.remove(QString("%1/%2").arg(SETTINGS,"PliControlFile"));
            } else {
                Settings.setValue(QString("%1/%2").arg(SETTINGS,"PliControlFile"),pliControlFile);
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

        if (povrayRenderQuality != dialog->povrayRenderQuality())
        {
            povrayRenderQuality = dialog->povrayRenderQuality();
            Settings.setValue(QString("%1/%2").arg(SETTINGS,"PovrayRenderQuality"),povrayRenderQuality);
        }

        if (povrayAutoCrop != dialog->povrayAutoCrop())
        {
            povrayAutoCrop = dialog->povrayAutoCrop();
            Settings.setValue(QString("%1/%2").arg(SETTINGS,"PovrayAutoCrop"),povrayAutoCrop);
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
                        QString excludedDir = QDir::toNativeSeparators(QString("%1/%2/").arg(ldrawLibPath).arg(excludedPath));
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
                    if (modeGUI) {
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
                    Settings.setValue(QString("%1/%2").arg(SETTINGS,ldrawSearchDirsKey),ldSearchDirs);
                else
                    Settings.remove(QString("%1/%2").arg(SETTINGS,ldrawSearchDirsKey));
            } else {
                Settings.remove(QString("%1/%2").arg(SETTINGS,ldrawSearchDirsKey));
            }
            // update LDView ExtraSearchDirs in ini files
            if (!setLDViewExtraSearchDirs(ldviewIni))
               logError() << QString("Could not update %1").arg(ldviewIni);
            if (!setLDViewExtraSearchDirs(ldviewPOVIni))
               logError() << QString("Could not update %1").arg(ldviewPOVIni);
            if (!setLDViewExtraSearchDirs(nativeExportIni))
               logError() << QString("Could not update %1").arg(nativeExportIni);
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

        if (showAnnotationMessages != dialog->showAnnotationMessages())
        {
            showAnnotationMessages = dialog->showAnnotationMessages();
            Settings.setValue(QString("%1/%2").arg(SETTINGS,"ShowAnnotationMessages"),showAnnotationMessages);
        }

        if (showSaveOnRedraw != dialog->showSaveOnRedraw())
        {
            showSaveOnRedraw = dialog->showSaveOnRedraw();
            Settings.setValue(QString("%1/%2").arg(SETTINGS,"ShowSaveOnRedraw"),showSaveOnRedraw);
        }

        if (showSaveOnUpdate != dialog->showSaveOnUpdate())
        {
            showSaveOnUpdate = dialog->showSaveOnUpdate();
            Settings.setValue(QString("%1/%2").arg(SETTINGS,"ShowSaveOnUpdate"),showSaveOnUpdate);
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

        if (validFadeStepsColour != dialog->fadeStepsColour())
        {
            validFadeStepsColour = dialog->fadeStepsColour();
            Settings.setValue(QString("%1/%2").arg(SETTINGS,fadeStepsColourKey),validFadeStepsColour);
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

        if (highlightFirstStep != dialog->highlightFirstStep())
        {
            highlightFirstStep = dialog->highlightFirstStep();
            Settings.setValue(QString("%1/%2").arg(SETTINGS,"HighlightFirstStep"),highlightFirstStep);
        }

        if (enableImageMatting != dialog->enableImageMatting())
        {
            enableImageMatting = dialog->enableImageMatting();
            Settings.setValue(QString("%1/%2").arg(SETTINGS,"EnableImageMatting"),enableImageMatting);
            if (enableImageMatting)
                updateLDViewIniFile(UpdateExisting);       // strip AutoCrop [disabled]
        }

        if (preferCentimeters != dialog->centimeters())
        {
            preferCentimeters = dialog->centimeters();
            Settings.setValue(QString("%1/%2").arg(SETTINGS,"Centimeters"),preferCentimeters);
            defaultResolutionType(preferCentimeters);
        }

        if (addLSynthSearchDir != dialog->addLSynthSearchDir())
        {
            addLSynthSearchDir = dialog->addLSynthSearchDir();
            Settings.setValue(QString("%1/%2").arg(SETTINGS,"AddLSynthSearchDir"),addLSynthSearchDir);
        }

        if (archiveLSynthParts != dialog->archiveLSynthParts())
        {
            archiveLSynthParts = dialog->archiveLSynthParts();
            Settings.setValue(QString("%1/%2").arg(SETTINGS,"ArchiveLSynthParts"),archiveLSynthParts);
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

        if (showDownloadRedirects != dialog->showDownloadRedirects()) {
            showDownloadRedirects = dialog->showDownloadRedirects();
            Settings.setValue(QString("%1/%2").arg(SETTINGS,"ShowDownloadRedirects"),showDownloadRedirects);
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

        if (sceneBackgroundColor != dialog->sceneBackgroundColor()){
            sceneBackgroundColor = dialog->sceneBackgroundColor();
            Settings.setValue(QString("%1/%2").arg(SETTINGS,"SceneBackgroundColor"),sceneBackgroundColor);
            setCustomSceneBackgroundColorPreference(true);
        }

        if (sceneGridColor != dialog->sceneGridColor()){
            sceneGridColor = dialog->sceneGridColor();
            Settings.setValue(QString("%1/%2").arg(SETTINGS,"SceneGridColor"),sceneGridColor);
            setCustomSceneGridColorPreference(true);
        }

        if (sceneRulerTickColor != dialog->sceneRulerTickColor()){
            sceneRulerTickColor = dialog->sceneRulerTickColor();
            Settings.setValue(QString("%1/%2").arg(SETTINGS,"SceneRulerTickColor"),sceneRulerTickColor);
            setCustomSceneRulerTickColorPreference(true);
        }

        if (sceneRulerTrackingColor != dialog->sceneRulerTrackingColor()){
            sceneRulerTrackingColor = dialog->sceneRulerTrackingColor();
            Settings.setValue(QString("%1/%2").arg(SETTINGS,"SceneRulerTrackingColor"),sceneRulerTrackingColor);
            setCustomSceneRulerTrackingColorPreference(true);
        }

        if (sceneGuideColor != dialog->sceneGuideColor()){
            sceneGuideColor = dialog->sceneGuideColor();
            Settings.setValue(QString("%1/%2").arg(SETTINGS,"SceneGuideColor"),sceneGuideColor);
            setCustomSceneGuideColorPreference(true);
        }

        if (dialog->resetSceneColors()) {
            if (displayTheme == THEME_DARK) {
                sceneBackgroundColor = THEME_SCENE_BGCOLOR_DARK;
                sceneGridColor = THEME_GRID_PEN_DARK;
                sceneRulerTickColor = THEME_RULER_TICK_PEN_DARK;
                sceneRulerTrackingColor = THEME_RULER_TRACK_PEN_DARK;
                sceneGuideColor = THEME_GUIDE_PEN_DARK;
            } else {
                sceneBackgroundColor = THEME_SCENE_BGCOLOR_DEFAULT;
                sceneGridColor = THEME_GRID_PEN_DEFAULT;
                sceneRulerTickColor = THEME_RULER_TICK_PEN_DEFAULT;
                sceneRulerTrackingColor = THEME_RULER_TRACK_PEN_DEFAULT;
                sceneGuideColor = THEME_GUIDE_PEN_DEFAULT;
            }
            Settings.setValue(QString("%1/%2").arg(SETTINGS,"SceneBackgroundColor"),sceneBackgroundColor);
            Settings.setValue(QString("%1/%2").arg(SETTINGS,"SceneGridColor"),sceneGridColor);
            Settings.setValue(QString("%1/%2").arg(SETTINGS,"SceneRulerTickColor"),sceneRulerTickColor);
            Settings.setValue(QString("%1/%2").arg(SETTINGS,"SceneRulerTrackingColor"),sceneRulerTrackingColor);
            Settings.setValue(QString("%1/%2").arg(SETTINGS,"SceneGuideColor"),sceneGuideColor);
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

        if (perspectiveProjection != dialog->perspectiveProjection())
        {
            perspectiveProjection = dialog->perspectiveProjection();
            Settings.setValue(QString("%1/%2").arg(SETTINGS,"PerspectiveProjection"),perspectiveProjection);
        }

        if (saveOnRedraw != dialog->saveOnRedraw())
        {
            saveOnRedraw = dialog->saveOnRedraw();
            Settings.setValue(QString("%1/%2").arg(SETTINGS,"SaveOnRedraw"),saveOnRedraw);
        }

        if (saveOnUpdate != dialog->saveOnUpdate())
        {
            saveOnUpdate = dialog->saveOnUpdate();
            Settings.setValue(QString("%1/%2").arg(SETTINGS,"SaveOnUpdate"),saveOnUpdate);
        }

        if (loadLastOpenedFile != dialog->loadLastOpenedFile())
        {
            loadLastOpenedFile = dialog->loadLastOpenedFile();
            Settings.setValue(QString("%1/%2").arg(SETTINGS,"LoadLastOpenedFile"),loadLastOpenedFile);
        }

        if (extendedSubfileSearch != dialog->extendedSubfileSearch())
        {
            extendedSubfileSearch = dialog->extendedSubfileSearch();
            Settings.setValue(QString("%1/%2").arg(SETTINGS,"ExtendedSubfileSearch"),extendedSubfileSearch);
        }

        if (ldrawFilesLoadMsgs != dialog->ldrawFilesLoadMsgs())
        {
            ldrawFilesLoadMsgs = dialog->ldrawFilesLoadMsgs();
            Settings.setValue(QString("%1/%2").arg(SETTINGS,"LdrawFilesLoadMsgs"),ldrawFilesLoadMsgs);
        }

        usingNativeRenderer = preferredRenderer == RENDERER_NATIVE;
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
    QFileInfo confFileInfo(iniFile);
    QStringList contentList;
    QRegExp prefSetRx("^(Native POV|Native STL|Native 3DS|Native Part List|POV-Ray Render)",Qt::CaseInsensitive);
    if (preferredRenderer == RENDERER_LDVIEW)
      logInfo() << QString("Updating ExtraSearchDirs in %1").arg(iniFile);
    if (confFile.open(QIODevice::ReadOnly))
    {
        int dirNum = 0;
        bool foundExtraSearchDirs = false;
        bool inExtraSearchDirsSection = false;
        QString prefSet;
        QTextStream input(&confFile);
        while (!input.atEnd())
        {
            QString line = input.readLine();
            if (line.left(17) == "[ExtraSearchDirs]") {
                foundExtraSearchDirs = true;
                inExtraSearchDirsSection = true;
                if (!contentList.contains(line,Qt::CaseInsensitive))
                    contentList.append( line);
            } else if (inExtraSearchDirsSection) {                  // in ExtraSearchDirs section
                if (line.left(1) == "[" || line.isEmpty()) {        // at next section or empty line, insert search dirs
                    dirNum = 0;
                    QString nativePath;
                    foreach (QString searchDir, ldSearchDirs) {
                       dirNum++;
                       if (dirNum <= ldSearchDirs.count()) {
#ifdef Q_OS_WIN
                          nativePath = searchDir.replace("\\","\\\\");
#else
                          nativePath = QDir::toNativeSeparators(searchDir);
#endif
                          if (!contentList.contains(nativePath, Qt::CaseInsensitive)) {
                              QString formattedSearchDir = QString("Dir%1=%2").arg(dirNum, 3, 10, QChar('0')).arg(nativePath);
                              contentList += formattedSearchDir;
                              if (preferredRenderer == RENDERER_LDVIEW || confFileInfo.completeBaseName().toLower() == "ldvexport")
                                  logInfo() << QString("ExtraSearchDirs OUT: %1").arg(formattedSearchDir);
                          }
                       }
                    }
                    if ( !line.isEmpty())
                        contentList.append( line);
                    inExtraSearchDirsSection = false;
                }
            } else if (line.contains(prefSetRx)) {               // session preference set
                if (line.contains("_SessionPlaceholder")) {      // insert search dirs before session placeholder
                    dirNum = 0;
                    QString nativePath;
                    foreach (QString searchDir, ldSearchDirs) {
                       dirNum++;
                       if (dirNum <= ldSearchDirs.count()) {
#ifdef Q_OS_WIN
                          nativePath = searchDir.replace("\\","\\\\");
#else
                          nativePath = QDir::toNativeSeparators(searchDir);
#endif
                          if (!contentList.contains(nativePath, Qt::CaseInsensitive)) {
                              QString formattedSearchDir = QString("%1/ExtraSearchDirs/Dir%2=%3").arg(prefSetRx.cap(1)).arg(dirNum, 3, 10, QChar('0')).arg(nativePath);
                              contentList += formattedSearchDir;
                              if (preferredRenderer == RENDERER_LDVIEW || confFileInfo.completeBaseName().toLower() == "ldvexport")
                                  logInfo() << QString("ExtraSearchDirs OUT: %1").arg(formattedSearchDir);
                          }
                       }
                    }
                    contentList.append( line);
                } else if ( !line.isEmpty() && !line.contains("/ExtraSearchDirs/",Qt::CaseInsensitive)) {   // remove old ExtraSearchDirs lines
                    contentList.append( line);
                }
            } else if (!contentList.contains(line,Qt::CaseInsensitive) && !line.isEmpty()) {
                contentList.append( line);
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

    QSettings Settings;
    QFileInfo fileInfo;
    QString message;
    bool r = true;

    message = QMessageBox::tr("Extracting %1 LDraw library, please wait...").arg(validLDrawLibrary);

    emit Application::instance()->splashMsgSig("10% - " + message);

    if (!modeGUI) {
      fprintf(stdout,"%s\n",message.toLatin1().constData());
      fflush(stdout);
    }

    // if ldraw directory path is empty use the default location (datapath)
    if (ldrawLibPath.isEmpty()) {
        ldrawLibPath = QString("%1/%2").arg(lpubDataPath, validLDrawDir);
    }
    // set ldraw parent directory to extract archive.zip
    bool parentDirNotValid = false;
    QDir ldrawDir(ldrawLibPath);
    //logInfo() << QString("LDraw directory: %1").arg(ldrawDir.absolutePath()));
    logDebug() << QString("LDraw directory: %1").arg(ldrawDir.absolutePath());
    if (ldrawDir.dirName().toLower() != validLDrawDir.toLower())
        parentDirNotValid = true;
    if (!ldrawDir.isRoot())
        ldrawDir.cdUp();            // ldraw path parent directory
    //logInfo() << QString("LDraw parent directory (%1), AbsPath (%2)").arg(ldrawDir.dirName()).arg(ldrawDir.absolutePath())));

    // set the archive library path
    QDir libraryDir(QString("%1/%2").arg(lpubDataPath, "libraries"));
    fileInfo.setFile(QDir::toNativeSeparators(QString("%1/%2").arg(libraryDir.absolutePath(), validLDrawPartsArchive)));

    // archive library exist so let's proceed...
    if (fileInfo.exists()) {

        // extract archive.zip
        QString destination = ldrawDir.absolutePath();
        QStringList result = JlCompress::extractDir(fileInfo.absoluteFilePath(),destination);
        if (result.isEmpty()){
            logError() << QString("Failed to extract %1 to %2/%3").arg(fileInfo.absoluteFilePath()).arg(destination).arg(validLDrawDir);
            r = false;
         } else {
            if (! usingDefaultLibrary) {
                // Rename extracted library to ldraw for compatibility with lc_library and renderers
                QFile library(QString("%1/%2").arg(destination).arg(validLDrawDir));
                QFile extract(QString("%1/ldraw").arg(destination));

                if (! library.exists() || library.remove()) {
                    if (! extract.rename(library.fileName())) {
                        logError() << QString("Failed to rename %1 to %2").arg(extract.fileName()).arg(library.fileName());
                    } else {
                        message = QMessageBox::tr("%1 %2 Library files extracted to %3/%4")
                                .arg(result.size()).arg(validLDrawLibrary).arg(destination).arg(validLDrawDir);
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
            fileInfo.setFile(QDir::toNativeSeparators(QString("%1/%2").arg(libraryDir.absolutePath(),validLDrawCustomArchive)));
            if (fileInfo.exists()) {
                QString destination = QString("%1/unofficial").arg(ldrawLibPath);
                QStringList result = JlCompress::extractDir(fileInfo.absoluteFilePath(),destination);
                if (result.isEmpty()){
                    logError() << QString("Failed to extract %1 to %2").arg(fileInfo.absoluteFilePath()).arg(destination);
                } else {
                    message = QMessageBox::tr("%1 Unofficial Library files extracted to %2").arg(result.size()).arg(destination);
                    logInfo() << QString(message);
                }
            } else {
                message = QMessageBox::tr ("Unofficial Library archive file %1 does not exist.").arg(fileInfo.absoluteFilePath());
                logError() << QString(message);
            }
        }

        // copy extracted contents to ldraw directory and delete extract dir if needed
        if (parentDirNotValid) {
            QDir extractDir(QString("%1/%2").arg(ldrawDir.absolutePath()).arg(validLDrawDir));
            if (!copyRecursively(extractDir.absolutePath(),ldrawLibPath)) {
                message = QMessageBox::tr("Unable to copy %1 to %2").arg(extractDir.absolutePath(),ldrawLibPath);
                logInfo() << QString(message);
                r = false;
            }
            extractDir.removeRecursively();
        }

        // if no errors, (re)set ldrawLibPath in registry
        if (r)
            Settings.setValue(QString("%1/%2").arg(SETTINGS, ldrawLibPathKey),ldrawLibPath);

    } else {

        QString body;
        if (usingDefaultLibrary)
            fileInfo.setFile(QDir::toNativeSeparators(QString("%1/%2").arg(libraryDir.absolutePath(), VER_LDRAW_UNOFFICIAL_ARCHIVE)));
        if (usingDefaultLibrary && !fileInfo.exists()) {
            body = QMessageBox::tr ("LPub3D attempted to extract the LDraw library however the required archive files\n%1\n%2\ndoes not exist.\n").arg(lpub3dLibFile, fileInfo.absoluteFilePath());
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

        // remove registry setting and clear ldrawLibPath
        fileInfo.setFile(QDir::toNativeSeparators(QString("%1%2").arg(ldrawLibPath).arg(validLDrawPart)));
        if (!fileInfo.exists()) {
            Settings.remove(QString("%1/%2").arg(SETTINGS,ldrawLibPathKey));
            ldrawLibPath.clear();
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
