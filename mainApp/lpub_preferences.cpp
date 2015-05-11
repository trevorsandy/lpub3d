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

QString Preferences::ldrawPath = " ";
QString Preferences::leocadLibPath = " ";
QString Preferences::lgeoPath;
QString Preferences::lpubPath = ".";
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
}

void Preferences::ldrawPreferences(bool force)
{
  QFileInfo fileInfo;
  QSettings Settings;
  QString const ldrawKey("LDrawDir");
  
  if (Settings.contains(QString("%1/%2").arg(SETTINGS,ldrawKey))) {
    ldrawPath = Settings.value(QString("%1/%2").arg(SETTINGS,ldrawKey)).toString();
  }

  if (ldrawPath != "" && ! force) {
    QDir cwd(ldrawPath);

    if (cwd.exists()) {
      return;
    }
  }

  ldrawPath = "c:\\LDraw";

  QDir guesses;
  guesses.setPath(ldrawPath);
  if ( ! guesses.exists()) {
    ldrawPath = "c:\\Program Files\\LDraw";
    guesses.setPath(ldrawPath);
    if ( ! guesses.exists()) {

      ldrawPath = QFileDialog::getExistingDirectory(NULL,
                  QFileDialog::tr("Locate LDraw Directory"),
                  "/",
                  QFileDialog::ShowDirsOnly |
                  QFileDialog::DontResolveSymlinks);
    }
  }

  fileInfo.setFile(ldrawPath);

  if (fileInfo.exists()) {
    Settings.setValue(QString("%1/%2").arg(SETTINGS,ldrawKey),ldrawPath);
  } else {
    exit(-1);
  }  
}

void Preferences::leocadLibPreferences(bool force)
{
  QFileInfo fileInfo;
  QSettings Settings;
  QString const LeocadLibKey("PartsLibrary");

  if (Settings.contains(QString("%1/%2").arg(SETTINGS,LeocadLibKey))) {
    leocadLibPath = Settings.value(QString("%1/%2").arg(SETTINGS,LeocadLibKey)).toString();
  }

  if (leocadLibPath != "" && ! force) {

    fileInfo.setFile(leocadLibPath);
    bool leocadLibExists = fileInfo.exists();

    if (leocadLibExists) {
      return;
    }
  }

  leocadLibPath = "c:\\LDraw\\Complete.zip";

  QDir guesses;
  guesses.setPath(leocadLibPath);
  if ( ! guesses.exists()) {
    leocadLibPath = "c:\\Program Files\\LDraw\\Complete.zip";
    guesses.setPath(leocadLibPath);
    if ( ! guesses.exists()) {
      leocadLibPath = QFileDialog::getOpenFileName(NULL,
                      QFileDialog::tr("Locate LeoCad Library Archive"),
                      ldrawPath,
                      QFileDialog::tr("Archive (*.zip *.bin)"));
    }
  }

  fileInfo.setFile(leocadLibPath);

  if (fileInfo.exists()) {
    Settings.setValue(QString("%1/%2").arg(SETTINGS,LeocadLibKey),leocadLibPath);
  } else {
    exit(-1);
  }
}

void Preferences::lgeoPreferences()
{
    QSettings Settings;
	bool lgeoInstalled;
    QString lgeoDirKey("LGEOPath");
	QString lgeoDir;
    if (Settings.contains(QString("%1/%2").arg(POVRAY,lgeoDirKey))){
        lgeoDir = Settings.value(QString("%1/%2").arg(POVRAY,lgeoDirKey)).toString();
		QFileInfo info(lgeoDir);
		if (info.exists()) {
			lgeoInstalled = true;
			lgeoPath = lgeoDir;
		} else {
            Settings.remove(QString("%1/%2").arg(POVRAY,lgeoDirKey));
			lgeoInstalled = false;
		}
	} else {
		lgeoInstalled = false;
	}
}

void Preferences::renderPreferences()
{
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
  } else {
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
	
	/* Find L3P's installation status */
	
	bool    l3pInstalled;
	QString const l3pPathKey("L3P");
    QString const povrayPathKey("POVRayPath");
	QString l3pPath, povrayPath;
	
    if (Settings.contains(QString("%1/%2").arg(POVRAY,l3pPathKey))) {
        l3pPath = Settings.value(QString("%1/%2").arg(POVRAY,l3pPathKey)).toString();
		QFileInfo info(l3pPath);
		if (info.exists()) {
			l3pInstalled = true;
			l3pExe = l3pPath;
		} else {
            Settings.remove(QString("%1/%2").arg(POVRAY,l3pPathKey));
			l3pInstalled = false;
		}
	} else {
		l3pInstalled = false;
	}
	
    if (Settings.contains(QString("%1/%2").arg(POVRAY,povrayPathKey))) {
        povrayPath = Settings.value(QString("%1/%2").arg(POVRAY,povrayPathKey)).toString();
		QFileInfo info(povrayPath);
		if (info.exists()) {
			l3pInstalled &= true;
			povrayExe = povrayPath;
		} else {
            Settings.remove(QString("%1/%2").arg(POVRAY,povrayPathKey));
			l3pInstalled &= false;
		}
	} else {
		l3pInstalled &= false;
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
    } else if (preferredRenderer == "L3P") {
		if ( ! l3pInstalled) {
			preferredRenderer.clear();
            Settings.remove(QString("%1/%2").arg(SETTINGS,preferredRendererKey));
		}
    }
  }
  if (preferredRenderer == "") {
    if (ldviewInstalled && ldgliteInstalled) {
		preferredRenderer = l3pInstalled? "L3P" : "LDGLite";
    } else if (l3pInstalled) {
      preferredRenderer = "L3P";
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

  pliFile = QString("%1/%2").arg(lpubPath,"extras/pli.mpd");
  
#else

  //30-11-14 Incorrect path string
  //pliFile = "/extras/pli.mpd";
  pliFile = QDir::toNativeSeparators(QString("%1/%2").arg(lpubPath,"extras/pli.mpd"));

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

   titleAnnotationsFile    = QString("%1/%2").arg(lpubPath,"extras/titleAnnotations.lst");
   freeformAnnotationsFile = QString("%1/%2").arg(lpubPath,"extras/freeformAnnotations.lst");

   qDebug() << " Annotation File:  " << titleAnnotationsFile;

#else

    titleAnnotationsFile    = QDir::toNativeSeparators(QString("%1/%2").arg(lpubPath,"extras/titleAnnotations.lst"));
    freeformAnnotationsFile = QDir::toNativeSeparators(QString("%1/%2").arg(lpubPath,"extras/freeformAnnotations.lst"));

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

 fadeStepColorPartsFile    =  QString("%1/%2").arg(lpubPath,"extras/fadeStepColorParts.lst");

#else

  fadeStepColorPartsFile    = QDir::toNativeSeparators(QString("%1/%2").arg(lpubPath,"extras/fadeStepColorParts.lst"));

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

    if (leocadLibPath != dialog->leocadLibPath()) {
      leocadLibPath = dialog->leocadLibPath();
      if (leocadLibPath == "") {
        Settings.remove(QString("%1/%2").arg(SETTINGS,"PartsLibrary"));
      } else {
        Settings.setValue(QString("%1/%2").arg(SETTINGS,"PartsLibrary"),leocadLibPath);
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

    printCopyright = dialog->printCopyright();
    Settings.setValue(QString("%1/%2").arg(DEFAULTS,"PrintCopyright"),printCopyright);
    //SET DEFAULT PRINT STATE HERE AS NEEDED
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



