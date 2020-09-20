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

    static bool extractLDrawLib();
    static void getRequireds();
    static bool getPreferences();
    static void setLPub3DLoaded();
    static void setLDGLiteIniParams();
    static void setDistribution();
    static void updateLDVExportIniFile(UpdateFlag);
    static void updateLDViewIniFile(UpdateFlag);
    static void updateLDViewPOVIniFile(UpdateFlag);
    static void updatePOVRayConfFile(UpdateFlag);
    static void updatePOVRayIniFile(UpdateFlag);
    static void updatePOVRayConfigFiles();

    static void setLPub3DAltLibPreferences(const QString &);
    static bool checkLDrawLibrary(const QString &);
    static bool setLDViewExtraSearchDirs(const QString &);
    static bool copyRecursively(const QString &,const QString &);
#ifdef Q_OS_MAC
    static bool validLib(const QString &,const QString &);
#endif

    static void setOverwriteCustomParts(bool);  // NO SOURCE
    static void setSceneGuidesPreference(bool);
    static void setSceneGuidesPositionPreference(int);
    static void setSceneGuidesLinePreference(int);
    static void setSnapToGridPreference(bool);
    static void setHidePageBackgroundPreference(bool);
    static void setShowGuidesCoordinatesPreference(bool);
    static void setShowTrackingCoordinatesPreference(bool);
    static void setGridSizeIndexPreference(int);
    static void setSceneRulerPreference(bool);
    static void setSceneRulerTrackingPreference(int);
    static void setShowParseErrorsPreference(bool);
    static void setShowSaveOnRedrawPreference(bool);
    static void setShowSaveOnUpdatePreference(bool);
    static void setStdOutToLogPreference(bool);

    static void setCustomSceneBackgroundColorPreference(bool = true);
    static void setCustomSceneGridColorPreference(bool = true);
    static void setCustomSceneRulerTickColorPreference(bool = true);
    static void setCustomSceneRulerTrackingColorPreference(bool = true);
    static void setCustomSceneGuideColorPreference(bool = true);
    static void setSceneBackgroundColorPreference(QString);
    static void setSceneGridColorPreference(QString);
    static void setSceneRulerTickColorPreference(QString);
    static void setSceneRulerTrackingColorPreference(QString);
    static void setSceneGuideColorPreference(QString);
    static void setDebugLogging(bool);

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
    static QString nativeExportIni;
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

    static QString blCodesFile;
    static QString legoElementsFile;
    static QString blColorsFile;
    static QString ld2blColorsXRefFile;
    static QString ld2blCodesXRefFile;
    static QString ld2rbColorsXRefFile;
    static QString ld2rbCodesXRefFile;

    static QString fadeStepsColourKey;
    static QString ldrawSearchDirsKey;
    static QString ldrawLibPathKey;

    static QString sceneBackgroundColor;
    static QString sceneGridColor;
    static QString sceneRulerTickColor;
    static QString sceneRulerTrackingColor;
    static QString sceneGuideColor;

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
    static bool    showInstanceCount;
    static bool    enableDownloader;
    static bool    showDownloadRedirects;
    static bool    portableDistribution;
    static bool    lpub3dLoaded;
    static bool    lgeoStlLib;
    static bool    addLSynthSearchDir;
    static bool    archiveLSynthParts;
    static bool    usingNativeRenderer;
    static bool    skipPartsArchive;
    static bool    loadLastOpenedFile;
    static bool    extendedSubfileSearch;

    static bool    enableFadeSteps;
    static bool    fadeStepsUseColour;

    static bool    enableImageMatting;
    static bool    enableHighlightStep;
    static int     highlightStepLineWidth;
    static bool    highlightFirstStep;
    static bool    perspectiveProjection;
    static bool    saveOnRedraw;
    static bool    saveOnUpdate;

    static bool    includeLogLevel;
    static bool    includeTimestamp;
    static bool    includeLineNumber;
    static bool    includeFileName;
    static bool    includeFunction;

    static bool    pdfPageImage;
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

    static bool    povrayDisplay;
    static bool    povrayAutoCrop;
    static bool    isAppImagePayload;
    static bool    modeGUI;

    static bool    sceneRuler;
    static int     sceneRulerTracking;
    static bool    sceneGuides;
    static bool    snapToGrid;
    static bool    hidePageBackground;
    static bool    showGuidesCoordinates;
    static bool    showTrackingCoordinates;
    static bool    showParseErrors;
    static bool    showSaveOnRedraw;
    static bool    showSaveOnUpdate;
    static bool    suppressStdOutToLog;

    static bool    customSceneBackgroundColor;
    static bool    customSceneGridColor;
    static bool    customSceneRulerTickColor;
    static bool    customSceneRulerTrackingColor;
    static bool    customSceneGuideColor;
    static bool    debugLogging;


#ifdef Q_OS_MAC
    static bool    missingRendererLibs;
#endif

    static int     cameraDistFactorNative;
    static int     fadeStepsOpacity;
    static int     checkUpdateFrequency;
    static int     pageWidth;
    static int     pageHeight;
    static int     gridSizeIndex;
    static int     pageDisplayPause;
    static int     rendererTimeout;
    static int     sceneGuidesLine;
    static int     sceneGuidesPosition;
    static int     povrayRenderQuality;
    static int     ldrawFilesLoadMsgs;

    virtual ~Preferences() {}
};

#endif
