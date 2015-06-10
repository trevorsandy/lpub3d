/**************************************************************************** 
**
** Copyright (C) 2007-2009 Kevin Clague. All rights reserved.
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

#include "lpub_preferences.h"
#include "render.h"
#include "ui_preferences.h"
#include "preferencesdialog.h"
#include "name.h"
#include "resolution.h"
#include "pli.h"
//**3D
#include "lc_profile.h"
//**

Preferences preferences;

QString Preferences::ldrawPath = "";
QString Preferences::leocadLibFile = "";
QString Preferences::lgeoPath;
QString Preferences::lpubPath = ".";
QString Preferences::lpubDataPath = ".";
QString Preferences::lpubCachePath = ".";
QString Preferences::lpubExtrasPath = ".";
QString Preferences::ldgliteExe;
QString Preferences::ldviewExe;
QString Preferences::l3pExe;
QString Preferences::povrayExe;
QString Preferences::pliFile;
QString Preferences::preferredRenderer;
QString Preferences::titleAnnotationsFile;
QString Preferences::freeformAnnotationsFile;
QString Preferences::fadeStepColor = "Very_Light_Bluish_Gray";
QString Preferences::fadeStepColorPartsFile;
QString Preferences::defaultAuthor;
QString Preferences::defaultURL;
QString Preferences::defaultEmail;
QString Preferences::documentLogoFile;
QString Preferences::publishDescription;
bool    Preferences::enableDocumentLogo = false;
bool    Preferences::printCopyright = false;
bool    Preferences::preferTitleAnnotation = true;
bool    Preferences::preferFreeformAnnotation = false;
bool    Preferences::titleAndFreeformAnnotation = false;
bool    Preferences::enableFadeStep = false;
bool    Preferences::preferCentimeters = true;
bool    Preferences::silentUpdate = false;
int     Preferences::checkForUpdates = 0;

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
      lpubPath = cwd.absolutePath();

#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
    QStringList cachePathList = QStandardPaths::standardLocations(QStandardPaths::CacheLocation);
    lpubCachePath = cachePathList.first();

    QStringList dataPathList = QStandardPaths::standardLocations(QStandardPaths::DataLocation);
    lpubDataPath = dataPathList.first();
#else
    lpubCachePath = QDesktopServices::storageLocation(QDesktopServices::CacheLocation);
    lpubDataPath  = QDesktopServices::storageLocation(QDesktopServices::DataLocation);
#endif

    QDir extrasDir(lpubDataPath + "/extras");
    if(!QDir(extrasDir).exists())
        extrasDir.mkpath(".");

    QSettings Settings;

    if ( ! Settings.contains(QString("%1/%2").arg(DEFAULTS,"SilentUpdate"))) {
        QVariant pValue(false);
        silentUpdate = false;
        Settings.setValue(QString("%1/%2").arg(DEFAULTS,"SilentUpdate"),pValue);
    } else {
        silentUpdate = Settings.value(QString("%1/%2").arg(DEFAULTS,"SilentUpdate")).toBool();
    }

    if ( ! Settings.contains(QString("%1/%2").arg(DEFAULTS,"CheckForUpdates"))) {
        checkForUpdates = 0;
        Settings.setValue(QString("%1/%2").arg(DEFAULTS,"CheckForUpdates"),checkForUpdates);
    } else {
        checkForUpdates = Settings.value(QString("%1/%2").arg(DEFAULTS,"CheckForUpdates")).toInt();
    }

}

void Preferences::ldrawPreferences(bool force)
{
    QSettings Settings;
    QString const ldrawKey("LDrawDir");

    if (Settings.contains(QString("%1/%2").arg(SETTINGS,ldrawKey))) {
        ldrawPath = Settings.value(QString("%1/%2").arg(SETTINGS,ldrawKey)).toString();
    }

    if (! ldrawPath.isEmpty() && ! force) {
        QDir cwd(ldrawPath);

        if (cwd.exists()) {
            return;
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

void Preferences::leocadLibPreferences(bool force)
{
#ifdef Q_OS_WIN
    QString filter(QFileDialog::tr("Archive (*.zip *.bin);;All Files (*.*)"));
#else
    QString filter(QFileDialog::tr("All Files (*.*)"));
#endif

    QSettings Settings;
    QString const LeocadLibKey("PartsLibrary");

    if (Settings.contains(QString("%1/%2").arg(SETTINGS,LeocadLibKey))) {
        leocadLibFile = Settings.value(QString("%1/%2").arg(SETTINGS,LeocadLibKey)).toString();
    }

    if (! leocadLibFile.isEmpty() && ! force) {
        QDir cwd(leocadLibFile);

        if (cwd.exists()) {
            return;
        }
    }

    if (leocadLibFile.isEmpty() && ! force) {

        leocadLibFile = "c:\\LDraw\\Complete.zip";
        QDir guesses;
        guesses.setPath(leocadLibFile);
        if ( ! guesses.exists()) {
            leocadLibFile = "c:\\Program Files (x86)\\LDraw\\Complete.zip";
            guesses.setPath(leocadLibFile);
            if ( ! guesses.exists()) {
                leocadLibFile = "c:\\Program Files (x86)\\LDraw\\LeoCAD-Libraries\\Complete.zip";
                if (! guesses.exists()){
                    leocadLibFile = QFileDialog::getOpenFileName(NULL,
                         QFileDialog::tr("Locate LeoCad Library Archive"),
                         ldrawPath,
                         filter);
                }
            }
        }
    }

    if (! leocadLibFile.isEmpty() && force){

        QString result = QFileDialog::getOpenFileName(NULL,
                         QFileDialog::tr("Select LeoCad Library Archive"),
                         leocadLibFile,
                         filter);
        if (! result.isEmpty())
            leocadLibFile = QDir::toNativeSeparators(result);
    }

    if (! leocadLibFile.isEmpty()) {
        Settings.setValue(QString("%1/%2").arg(SETTINGS,LeocadLibKey),leocadLibFile);
    } else {
        QString question = QMessageBox::tr("You must select an LDraw library archive file. \nDo you wish to continue?");
        if (QMessageBox::question(NULL, "LDraw3D", question, QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes)
            exit(-1);
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
    QFileInfo ldgliteInfo(QString("%1/%2").arg(lpubPath).arg("3rdParty/ldglite1.2.6Win/ldglite.exe"));
    QFileInfo l3pInfo(QString("%1/%2").arg(lpubPath).arg("3rdParty/l3p1.4WinB/L3P.EXE"));
#else
    //TODO
    QFileInfo ldgliteInfo(QString("%1/%2").arg(lpubPath).arg("3rdParty/ldglite1.2.6Win/ldglite.exe"));
    QFileInfo l3pInfo(QString("%1/%2").arg(lpubPath).arg("3rdParty/l3p1.4WinB/L3P.EXE"));
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
}

void Preferences::pliPreferences()
{
  QSettings Settings;
  pliFile = Settings.value(QString("%1/%2").arg(SETTINGS,"PliControl")).toString();
  
  QFileInfo fileInfo(pliFile);

  if (fileInfo.exists()) {
    return;
  } else {
    Settings.remove(QString("%1/%2").arg(SETTINGS,"PliControl"));
  }

  //QMessageBox::warning(NULL,"LPub3D",lpubPath,QMessageBox::Cancel);
  
#ifdef __APPLE__

  pliFile = QString("%1/%2").arg(lpubDataPath,"extras/pli.mpd");
  
#else

  //30-11-14 Incorrect path string
  //pliFile = "/extras/pli.mpd";
  pliFile = QDir::toNativeSeparators(QString("%1/%2").arg(lpubDataPath,"extras/pli.mpd"));

#endif

  fileInfo.setFile(pliFile);
  if (fileInfo.exists()) {
    Settings.setValue(QString("%1/%2").arg(SETTINGS,"PliControl"),pliFile);
  } else {
    //pliFile = "";
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
    if(! Settings.contains(QString("%1/%2").arg(SETTINGS,"TitleAnnotation"))) {
        QVariant aValue(true);
        preferTitleAnnotation = true;
        Settings.setValue(QString("%1/%2").arg(SETTINGS,"TitleAnnotation"), aValue);
    } else {
        preferTitleAnnotation = Settings.value(QString("%1/%2").arg(SETTINGS,"TitleAnnotation")).toBool();
    }

    if(! Settings.contains(QString("%1/%2").arg(SETTINGS,"FreeFormAnnotation"))) {
        QVariant aValue(false);
        preferFreeformAnnotation = false;
        Settings.setValue(QString("%1/%2").arg(SETTINGS,"FreeFormAnnotation"), aValue);
    } else {
        preferFreeformAnnotation = Settings.value(QString("%1/%2").arg(SETTINGS,"FreeFormAnnotation")).toBool();
    }

    if(! Settings.contains(QString("%1/%2").arg(SETTINGS,"TitleAndFreeformAnnotation"))) {
        QVariant aValue(false);
        titleAndFreeformAnnotation = false;
        Settings.setValue(QString("%1/%2").arg(SETTINGS,"TitleAndFreeformAnnotation"), aValue);
    } else {
        titleAndFreeformAnnotation = Settings.value(QString("%1/%2").arg(SETTINGS,"TitleAndFreeformAnnotation")).toBool();
    }

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
  if ( ! Settings.contains(QString("%1/%2").arg(SETTINGS,"EnableFadeStep"))) {
    QVariant eValue(false);
    enableFadeStep = false;
    Settings.setValue(QString("%1/%2").arg(SETTINGS,"EnableFadeStep"),eValue);
    QVariant cValue("Very_Light_Bluish_Gray");
    fadeStepColor = "Very_Light_Bluish_Gray";
    Settings.setValue(QString("%1/%2").arg(SETTINGS,"FadeStepColor"),cValue);
  } else {
    enableFadeStep = Settings.value(QString("%1/%2").arg(SETTINGS,"EnableFadeStep")).toBool();
    fadeStepColor = Settings.value(QString("%1/%2").arg(SETTINGS,"FadeStepColor")).toString();
  }

  fadeStepColorPartsFile = Settings.value(QString("%1/%2").arg(SETTINGS,"FadeStepColorPartsFile")).toString();
  QFileInfo fadeStepColorFileInfo(fadeStepColorPartsFile);
  if (fadeStepColorFileInfo.exists()) {
    //return;
  } else {
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
 } else {
     //fadeStepColorPartsFile = "";
 }

}

void Preferences::publishingPreferences()
{
    QSettings Settings;

    if ( ! Settings.contains(QString("%1/%2").arg(DEFAULTS,"PrintCopyright"))) {
        QVariant pValue(false);
        printCopyright = false;
        Settings.setValue(QString("%1/%2").arg(DEFAULTS,"PrintCopyright"),pValue);
    } else {
        printCopyright = Settings.value(QString("%1/%2").arg(DEFAULTS,"PrintCopyright")).toBool();
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
    if (Settings.contains(QString("%1/%2").arg(SETTINGS,"PartsLibrary")))
        lcSetProfileString(LC_PROFILE_PARTS_LIBRARY, Settings.value(QString("%1/%2").arg(SETTINGS,"PartsLibrary")).toString());
    if (Settings.contains(QString("%1/%2").arg(POVRAY,"POVRayPath")))
        lcSetProfileString(LC_PROFILE_POVRAY_PATH, Settings.value(QString("%1/%2").arg(POVRAY,"POVRayPath")).toString());
    if (Settings.contains(QString("%1/%2").arg(POVRAY,"LGEOPath")))
        lcSetProfileString(LC_PROFILE_POVRAY_LGEO_PATH, Settings.value(QString("%1/%2").arg(POVRAY,"LGEOPath")).toString());
}

bool Preferences::getPreferences()
{
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

    if (leocadLibFile != dialog->leocadLibFile()) {
      leocadLibFile = dialog->leocadLibFile();
      if (leocadLibFile == "") {
        Settings.remove(QString("%1/%2").arg(SETTINGS,"PartsLibrary"));
      } else {
        Settings.setValue(QString("%1/%2").arg(SETTINGS,"PartsLibrary"),leocadLibFile);
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

    preferCentimeters = dialog->centimeters();
    Settings.setValue(QString("%1/%2").arg(SETTINGS,"Centimeters"),preferCentimeters);
    defaultResolutionType(preferCentimeters);

    preferTitleAnnotation = dialog->titleAnnotation();
    Settings.setValue(QString("%1/%2").arg(SETTINGS,"TitleAnnotation"),preferTitleAnnotation);

    preferFreeformAnnotation = dialog->freeformAnnotation();
    Settings.setValue(QString("%1/%2").arg(SETTINGS,"FreeFormAnnotation"),preferFreeformAnnotation);

    titleAndFreeformAnnotation = dialog->titleAndFreeformAnnotation();
    Settings.setValue(QString("%1/%2").arg(SETTINGS,"TitleAndFreeformAnnotation"),titleAndFreeformAnnotation);

    enableFadeStep = dialog->enableFadeStep();
    Settings.setValue(QString("%1/%2").arg(SETTINGS,"EnableFadeStep"),enableFadeStep);
    if (enableFadeStep && (fadeStepColor != dialog->fadeStepColor()))
    {
        fadeStepColor = dialog->fadeStepColor();
        if (fadeStepColor == "") {
            Settings.remove(QString("%1/%2").arg(SETTINGS,"FadeStepColor"));
        }else {
            Settings.setValue(QString("%1/%2").arg(SETTINGS,"FadeStepColor"),fadeStepColor);
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

    if (silentUpdate != dialog->silentUpdate()) {
        silentUpdate = dialog->silentUpdate();
        Settings.setValue(QString("%1/%2").arg(DEFAULTS,"SilentUpdate"),silentUpdate);
    }

    if (checkForUpdates != dialog->checkForUpdates()) {
        checkForUpdates = dialog->checkForUpdates();
        Settings.setValue(QString("%1/%2").arg(DEFAULTS,"CheckForUpdates"),checkForUpdates);
    }

    if (printCopyright != dialog->printCopyright()) {
        printCopyright = dialog->printCopyright();
        Settings.setValue(QString("%1/%2").arg(DEFAULTS,"PrintCopyright"),printCopyright);
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



