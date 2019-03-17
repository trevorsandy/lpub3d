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
#ifndef LPUB_PREFERENCES_H
#define LPUB_PREFERENCES_H

#include <QObject>

enum UpdateFlag { UpdateExisting, SkipExisting };

class QString;
class QStringList;

class Preferences
{
  public:
    Preferences();
    static void lpubPreferences();
    static void ldrawPreferences(bool);
    static void lpub3dLibPreferences(bool);
    static void lpub3dUpdatePreferences();
    static void viewerPreferences();
    static void lgeoPreferences();
    static void rendererPreferences(UpdateFlag);
    static void pliPreferences();
    static void unitsPreferences();
    static void annotationPreferences();
    static void fadestepPreferences();
    static void highlightstepPreferences();
    static void publishingPreferences();
    static void loggingPreferences();
    static void exportPreferences();
    static void userInterfacePreferences();
    static void nativePovGenPreferences();

    static bool extractLDrawLib();
    static void getRequireds();
    static bool getPreferences();
    static void setLPub3DLoaded();
    static void setLDGLiteIniParams();
    static void setDistribution();
    static void updateLDVPOVIniFile(UpdateFlag);
    static void updateLDViewIniFile(UpdateFlag);
    static void updateLDViewPOVIniFile(UpdateFlag);
    static void updatePOVRayConfFile(UpdateFlag);
    static void updatePOVRayIniFile(UpdateFlag);

    static void setLPub3DAltLibPreferences(const QString &);
    static bool checkLDrawLibrary(const QString &);
    static bool setLDViewExtraSearchDirs(const QString &);
    static bool copyRecursively(const QString &,const QString &);
#ifdef Q_OS_MAC
    static bool validLib(const QString &,const QString &);
#endif

    static void setOverwriteCustomParts(bool);
    static void setPageGuidesPreference(bool);
    static void setPageRulerPreference(bool);
    static void setShowParseErrorsPreference(bool);
    static void setStdOutToLogPreference(bool);

    static QString lpub3dAppName;
    static QString ldrawLibPath;
    static QString altLDConfigPath;
    static QString lpub3dLibFile;
    static QString lgeoPath;
    static QString lpubDataPath;
    static QString lpubConfigPath;
    static QString lpubExtrasPath;
    static QString ldgliteExe;
    static QString ldviewExe;
    static QString povrayExe;
    static QString preferredRenderer;
    static QString lpub3dPath;
    static QString lpub3dExtrasResourcePath;
    static QString lpub3dDocsResourcePath;
    static QString lpub3d3rdPartyConfigDir;
    static QString lpub3d3rdPartyAppDir;
    static QString lpub3dLDVConfigDir;
    static QString highlightStepColour;
    static QString defaultAuthor;
    static QString defaultURL;
    static QString defaultEmail;
    static QString documentLogoFile;
    static QString publishDescription;
    static QString disclaimer;
    static QString copyright;
    static QString plugImage;
    static QString plug;
    static QString ldrawiniFile;
    static QString ldgliteSearchDirs;
    static QString moduleVersion;
    static QString logPath;
    static QString loggingLevel;
    static QString availableVersions;
    static QString povrayConf;
    static QString povrayIni;
    static QString ldgliteIni;
    static QString ldviewIni;
    static QString ldviewPOVIni;
    static QString nativePOVIni;
    static QString povrayIniPath;
    static QString povrayIncPath;
    static QString povrayScenePath;
    static QString dataLocation;
    static QString povFileGenerator;
    static QString displayTheme;
    static QString ldvLights;
    static QString xmlMapPath;

    static QString validLDrawLibrary;
    static QString validLDrawDir;
    static QString validLDrawPart;
    static QString validLDrawPartsArchive;
    static QString validLDrawLibraryChange;
    static QString validLDrawCustomArchive;
    static QString validLDrawPartsLibrary;
    static QString validLDrawColorParts;
    static QString validFadeStepsColour;
    static QString validPliControl;
    static QString validTitleAnnotations;
    static QString validFreeFormAnnotations;
    static QString validAnnotationStyleFile;
    static QString validPliSubstituteParts;
    static QString validExcludedPliParts;

    static QString pliSubstitutePartsFile;
    static QString pliControlFile;
    static QString titleAnnotationsFile;
    static QString freeformAnnotationsFile;
    static QString annotationStyleFile;
    static QString excludedPartsFile;
    static QString ldrawColourPartsFile;

    static QString blElementsFile;
    static QString legoElementsFile;
    static QString blColorsFile;
    static QString ld2blColorsXRefFile;
    static QString ld2blCodesXRefFile;

    static QString fadeStepsColourKey;
    static QString ldrawSearchDirsKey;
    static QString ldrawLibPathKey;

    static QStringList ldgliteParms;
    static QStringList ldSearchDirs;

    static bool    usingDefaultLibrary;
    static bool    themeAutoRestart;
    static bool    ldrawiniFound;
    static bool    enableDocumentLogo;
    static bool    enableLDViewSingleCall;
    static bool    enableLDViewSnaphsotList;
    static bool    displayAllAttributes;
    static bool    generateCoverPages;
    static bool    printDocumentTOC;
    static bool    doNotShowPageProcessDlg;
    static bool    applyCALocally;
    static bool    preferCentimeters;
    static bool    showAllNotifications;
    static bool    showUpdateNotifications;
    static bool    showSubmodels;
    static bool    showTopModel;
    static bool    showSubmodelInCallout;
    static bool    enableDownloader;
    static bool    portableDistribution;
    static bool    lpub3dLoaded;
    static bool    lgeoStlLib;
    static bool    addLSynthSearchDir;
    static bool    archiveLSynthParts;
    static bool    usingNativeRenderer;

    static bool    enableFadeSteps;
    static bool    fadeStepsUseColour;
    static int     fadeStepsOpacity;
    static bool    enableImageMatting;
    static bool    enableHighlightStep;
    static int     highlightStepLineWidth;
    static int     cameraDistFactorNative;
    static bool    perspectiveProjection;

    static bool    includeLogLevel;
    static bool    includeTimestamp;
    static bool    includeLineNumber;
    static bool    includeFileName;
    static bool    includeFunction;

    static bool    ignoreMixedPageSizesMsg;

    static bool    debugLevel;
    static bool    traceLevel;
    static bool    noticeLevel;
    static bool    infoLevel;
    static bool    statusLevel;
    static bool    errorLevel;
    static bool    fatalLevel;

    static bool    includeAllLogAttributes;
    static bool    allLogLevels;

    static bool    logging;       // logging on/off offLevel (grp box)
    static bool    logLevel;      // log level combo (grp box)
    static bool    logLevels;     // individual logging levels (grp box)

    static int     checkUpdateFrequency;
    static int     pageWidth;
    static int     pageHeight;
    static int     pageDisplayPause;
    static int     rendererTimeout;
    static bool    povrayDisplay;
    static bool    isAppImagePayload;
    static bool    modeGUI;

    static bool    pageRuler;
    static bool    pageGuides;
    static bool    showParseErrors;
    static bool    suppressStdOutToLog;

#ifdef Q_OS_MAC
    static bool    missingRendererLibs;
#endif

    virtual ~Preferences() {}
};

#endif
