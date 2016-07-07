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
#ifndef LPUB_PREFERENCES_H
#define LPUB_PREFERENCES_H

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
    static void renderPreferences();
    static void pliPreferences();
    static void unitsPreferences();
    static void annotationPreferences();
    static void fadestepPreferences();
    static void publishingPreferences();
    static void getRequireds();
    static bool getPreferences();

    static QString ldrawPath;
    static QString lpub3dLibFile;
    static QString lgeoPath;
    static QString lpubDataPath;
    static QString lpubExtrasPath;
    static QString ldgliteExe;
    static QString ldviewExe;
    static QString l3pExe;
    static QString povrayExe;
    static QString preferredRenderer;
    static QString lpub3dPath;
    static QString fadeStepColor;
    static QString pliSubstitutePartsFile;
    static QString fadeStepColorPartsFile;
    static QString pliFile;
    static QString titleAnnotationsFile;
    static QString freeformAnnotationsFile;
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
    static QStringList ldSearchDirs;
    static bool    ldrawiniFound;
    static bool    enableDocumentLogo;
    static bool    enableLDViewSingleCall;
    static bool    useLDViewSingleCall;
    static bool    displayAllAttributes;
    static bool    generateCoverPages;
    static bool    printDocumentTOC;
    static bool    preferCentimeters;
    static bool    enableFadeStep;
    static bool    fadeStepSettingChanged;
    static bool    fadeStepColorChanged;
    static bool    showAllNotifications;
    static bool    showUpdateNotifications;
    static bool    enableDownloader;
    static bool    portableDistribution;
    static int     checkUpdateFrequency;
    static int     pageWidth;
    static int     pageHeight;

    virtual ~Preferences() {}
};

#endif
