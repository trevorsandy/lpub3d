/****************************************************************************
**
** Copyright (C) 2018 - 2024 Trevor SANDY. All rights reserved.
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

#include <QTime>
#include "application.h"
#include "lc_profile.h"
#include "lc_library.h"
#include "lc_application.h"
#include "lpub.h"

int LPub::processCommandLine()
{
  // Visual Editor
  int viewerCommand = Process3DViewerCommandLine();
  if (viewerCommand < 0) /*fail*/
     return 1;
  else
  if (viewerCommand)     /*success*/
    return 0;

  // Declarations
  const QStringList excludedCommands = QStringList()
    // Application::initialize arguments
    << "-ncr" << "--no-console-redirect"
    << "-ns"  << "--no-stdout-log"
    << "-ll"  << "--liblego"
    << "-lt"  << "--libtente"
    << "-lv"  << "--libvexiq"
    << "-ap"  << "--app-paths"
    << "-v"   << "--version"
    << "-?"   << "--help"
    // Visual Editor arguments
    << "-ve"  << "--visual-editor-version"
    << "-l"   << "--libpath"
    << "-i"   << "--image"
    << "-w"   << "--width"
    << "-h"   << "--height"
    << "-f"   << "--from"
    << "-t"   << "--to"
    << "-s"   << "--submodel"
    << "-c"   << "--camera"
    << "-cl"  << "--draw-conditional-lines"
    << "-obj" << "--export-wavefront"
    << "-3ds" << "--export-3ds"
    << "-dae" << "--export-collada"
    << "-html" << "--export-html"
    << "--viewpoint"
    << "--camera-angles"
    << "--camera-position"
    << "--camera-position-ldraw"
    << "--orthographic"
    << "--fov"
    << "--zplanes"
    << "--fade-steps"
    << "--no-fade-steps"
    << "--fade-steps-color"
    << "--highlight"
    << "--no-highlight"
    << "--highlight-color"
    << "--shading"
    << "--line-width"
    << "--aa-samples"
    ;

  connect(gui, SIGNAL(fileLoadedSig(bool)),
          this, SLOT(    fileLoaded(bool)));

  mFileLoaded                   = false;
  mFileLoadFail                 = false;

  quint32 ColorValue            = 0;

   int    StudStyle             = GetStudStyle();
  bool    AutomateEdgeColor     = GetAutomateEdgeColor();
  float   PartEdgeContrast      = GetPartEdgeContrast();
  float   PartColorValueLDIndex = GetPartColorLightDarkIndex();

  quint32 StudCylinderColor     = GetStudCylinderColor();
  quint32 PartEdgeColor         = GetPartEdgeColor();
  quint32 BlackEdgeColor        = GetBlackEdgeColor();
  quint32 DarkEdgeColor         = GetDarkEdgeColor();
  bool    StudCylinderColorEnabled = GetStudCylinderColorEnabled();;
  bool    PartEdgeColorEnabled  = GetPartEdgeColorEnabled();
  bool    BlackEdgeColorEnabled = GetBlackEdgeColorEnabled();
  bool    DarkEdgeColorEnabled  = GetDarkEdgeColorEnabled();
  int     HighContrastStudStyle = static_cast<int>(lcStudStyle::HighContrast);

   int fadeStepsOpacity      = FADE_OPACITY_DEFAULT;
   int highlightLineWidth    = HIGHLIGHT_LINE_WIDTH_DEFAULT;
  bool processExport         = false;
  bool processFile           = false;
  bool fadeSteps             = false;
  bool highlightStep         = false;
// bool imageMatting          = false;
  bool resetSearchDirs       = false;
  bool StudStyleColourChanged= false;
  bool AutoEdgeColorChanged  = false;
  bool ColourConfigChanged   = false;
  bool rendererChanged       = false;
  bool StudStyleChanged      = false;
  QString pageRange, exportOption, colourConfigFile,
          preferredRenderer, projection, message,
          fadeStepsColour, highlightStepColour,
          metaCommandsFile;
  QString const StudStyleMessage = tr("High contrast stud style is required for the");
  QString const AutomateEdgeColourMessage = tr("Automate edge color is required for the");
  QColor ParsedColor;

  // Parse parameters
  QStringList Arguments = Application::instance()->arguments();

  bool ParseOK = true;

  const int NumArguments = Arguments.size();
  
  for (int ArgIdx = 1; ArgIdx < NumArguments; ArgIdx++)
  {
      const QString& Param = Arguments[ArgIdx];

      if (Param.isEmpty())
          continue;

      if (Param[0] != '-')
      {
          LPub::commandlineFile = Param;
          continue;
      }

      bool IsExcluded = false;
      for (int i = 0; i < excludedCommands.size(); i++)
      {
          if (Param.startsWith(excludedCommands.at(i)))
          {
              IsExcluded = true;
              break;
          }
      }

      if (IsExcluded)
          continue;

      auto InvalidParse = [&] (const QString& Text, bool Pair, bool SetParse = true)
      {
          QString message = Text.isEmpty() ? tr("Invalid value specified") : Text;
          if (Pair)
              emit messageSig(LOG_ERROR, message.append(tr(" '%1' option: '%2'.").arg(Arguments[ArgIdx - 1]).arg(Param)));
          else
              emit messageSig(LOG_ERROR, message.append(tr(" '%1' option.").arg(Param)));

          ParseOK = SetParse ? false : true;
      };

      auto ParseString = [&](QString& Value, bool Required)
      {
          if (ArgIdx < NumArguments - 1 && Arguments[ArgIdx + 1][0] != '-')
          {
              ArgIdx++;
              Value = Arguments[ArgIdx];
          }
          else if (Required)
          {
              InvalidParse(tr("Not enough parameters for the"), false);
              return false;
          }

          return true;
      };

      auto ParseInteger = [&](int& Value, int Min, int Max)
      {
          if (ArgIdx < NumArguments - 1 && Arguments[ArgIdx + 1][0] != '-')
          {
              bool Ok = false;
              ArgIdx++;
              int NewValue = Arguments[ArgIdx].toInt(&Ok);

              if (Ok && NewValue >= Min && NewValue <= Max)
              {
                  Value = NewValue;
                  return true;
              }
              else
                  InvalidParse(tr("Invalid parameter value specified for the"), true);
          }
          else
              InvalidParse(tr("Not enough parameters for the"), false);

          return false;
      };

      auto ParseFloat = [&](float& Value, float Min, float Max)
      {
          if (ArgIdx < NumArguments - 1 && Arguments[ArgIdx + 1][0] != '-')
          {
              bool Ok = false;
              ArgIdx++;
              int NewValue = Arguments[ArgIdx].toFloat(&Ok);

              if (Ok && NewValue >= Min && NewValue <= Max)
              {
                  Value = NewValue;
                  return true;
              }
              else
                  InvalidParse(tr("Invalid parameter value specified for the"), true);
          }
          else
              InvalidParse(tr("Not enough parameters for the"), false);

          return false;
      };

      auto ParseColor32 = [&](quint32& Color)
      {
          if (ArgIdx < NumArguments - 1 && Arguments[ArgIdx + 1][0] != '-')
          {
              ArgIdx++;
              QString Parameter = Arguments[ArgIdx];
              ParsedColor = QColor(Parameter);

              if (ParsedColor.isValid())
              {
                  Color = LC_RGBA(ParsedColor.red(), ParsedColor.green(), ParsedColor.blue(), ParsedColor.alpha());
                  return true;
              }
              else
                  InvalidParse(tr("Not enough parameters for the"), false);
          }
          else
              InvalidParse(tr("Not enough parameters for the"), false);

          return false;
      };

      if (Param == QLatin1String("-pf") || Param == QLatin1String("--process-file"))
        processFile = true;
      else
      if (Param == QLatin1String("-pe") || Param == QLatin1String("--process-export"))
        processExport = true;
      else
      if (Param == QLatin1String("-fs") || Param == QLatin1String("--fade-steps"))
        fadeSteps = true;
      else
      if (Param == QLatin1String("-fo") || Param == QLatin1String("--fade-step-opacity"))
      {
         if (ParseInteger(fadeStepsOpacity, 0, 100))
           fadeSteps = true;
      }
      else
      if (Param == QLatin1String("-fc") || Param == QLatin1String("--fade-steps-color"))
      {
          if (ParseString(fadeStepsColour, true))
            fadeSteps = true;
      }
      else
      if (Param == QLatin1String("-hs") || Param == QLatin1String("--highlight-step"))
          highlightStep = true;
      else
      if (Param == QLatin1String("-hc") || Param == QLatin1String("--highlight-step-color"))
      {
        if (ParseString(highlightStepColour, true))
            highlightStep = true;
      }
      else
      if (Param == QLatin1String("-emc") || Param == QLatin1String("--export-meta-commands"))
        ParseString(metaCommandsFile, true);
      else
      if (Param == QLatin1String("-ss") || Param == QLatin1String("--stud-style"))
      {
        int Value;
        if (!ParseInteger(Value, static_cast<int>(lcStudStyle::Plain), HighContrastStudStyle))
            InvalidParse(tr("Invalid value specified, valid values range from 0 to 7 for the"), true);
        else if ((StudStyleChanged = Value != StudStyle))
            StudStyle = Value;
      }
      else
      if (Param == QLatin1String("-nscc") || Param == QLatin1String("--disable-stud-cylinder-color"))
      {
        if (StudStyle < HighContrastStudStyle && !Arguments.contains("-ss") && !Arguments.contains("--stud-style"))
            InvalidParse(StudStyleMessage, true, false);
        else if (StudCylinderColorEnabled) {
            StudStyleChanged = true;
            StudCylinderColorEnabled = false;
        }
      }
      else
      if (Param == QLatin1String("-scc") || Param == QLatin1String("--stud-cylinder-color"))
      {
          if (ParseColor32(ColorValue))
          {
              if (StudStyle < HighContrastStudStyle && !Arguments.contains("-ss") && !Arguments.contains("--stud-style"))
                  InvalidParse(StudStyleMessage, true, false);
              else if (ColorValue != StudCylinderColor) {
                  StudStyleChanged = true;
                  StudCylinderColor = ColorValue;
              }
          }
      }
      else
      if (Param == QLatin1String("-nec") || Param == QLatin1String("--disable-edge-color"))
      {
          if (StudStyle < HighContrastStudStyle && !Arguments.contains("-ss") && !Arguments.contains("--stud-style"))
              InvalidParse(StudStyleMessage, true, false);
          else if (PartEdgeColorEnabled)
          {
              StudStyleColourChanged = true;
              PartEdgeColorEnabled = false;
          }
      }
      else
      if (Param == QLatin1String("-ec") || Param == QLatin1String("--edge-color"))
      {
          if (ParseColor32(PartEdgeColor))
          {
              if (StudStyle < HighContrastStudStyle && !Arguments.contains("-ss") && !Arguments.contains("--stud-style"))
                  InvalidParse(StudStyleMessage, true, false);
              else if (ColorValue != PartEdgeColor)
              {
                  StudStyleColourChanged = true;
                  PartEdgeColor = ColorValue;
              }
          }
      }
      else
      if (Param == QLatin1String("-nbec") || Param == QLatin1String("--disable-black-edge-color"))
      {
          if (StudStyle < HighContrastStudStyle && !Arguments.contains("-ss") && !Arguments.contains("--stud-style"))
              InvalidParse(StudStyleMessage, true, false);
          else if (BlackEdgeColorEnabled)
          {
              StudStyleColourChanged = true;
              BlackEdgeColorEnabled = false;
          }
      }
      else
      if (Param == QLatin1String("-bec") || Param == QLatin1String("--black-edge-color"))
      {
          if (ParseColor32(BlackEdgeColor))
          {
              if (StudStyle < HighContrastStudStyle && !Arguments.contains("-ss") && !Arguments.contains("--stud-style"))
                  InvalidParse(StudStyleMessage, true, false);
              else if (ColorValue != BlackEdgeColor)
              {
                  StudStyleColourChanged = true;
                  BlackEdgeColor = ColorValue;
              }
          }
      }
      else
      if (Param == QLatin1String("-ndec") || Param == QLatin1String("--disable-dark-edge-color"))
      {
          if (StudStyle < HighContrastStudStyle && !Arguments.contains("-ss") && !Arguments.contains("--stud-style"))
              InvalidParse(StudStyleMessage, true, false);
          else if (DarkEdgeColorEnabled)
          {
              StudStyleColourChanged = true;
              DarkEdgeColorEnabled = false;
          }
      }
      else
      if (Param == QLatin1String("-dec") || Param == QLatin1String("--dark-edge-color"))
      {
          if (ParseColor32(DarkEdgeColor))
          {
              if (StudStyle < HighContrastStudStyle && !Arguments.contains("-ss") && !Arguments.contains("--stud-style"))
                  InvalidParse(StudStyleMessage, true, false);
              else if (ColorValue != DarkEdgeColor)
              {
                  StudStyleColourChanged = true;
                  DarkEdgeColor = ColorValue;
              }
          }
      }
      else
      if (Param == QLatin1String("-aec") || Param == QLatin1String("--automate-edge-color"))
      {
          if (!AutomateEdgeColor)
          {
              AutoEdgeColorChanged = true;
              AutomateEdgeColor = true;
          }
      }
      else
      if (Param == QLatin1String("-cc") || Param == QLatin1String("--color-contrast"))
      {
          float EdgeContrast = 0;
          if (ParseFloat(EdgeContrast, 0.0f, 1.0f))
          {
              if (!AutomateEdgeColor && !Arguments.contains("-aec") && !Arguments.contains("--automate-edge-color"))
                  InvalidParse(AutomateEdgeColourMessage, true, false);
              else if (EdgeContrast != PartEdgeContrast)
              {
                  AutoEdgeColorChanged = true;
                  PartEdgeContrast = EdgeContrast;
              }
          }
      }
      else
      if (Param == QLatin1String("-ldv") || Param == QLatin1String("--light-dark-value"))
      {
          float ColorValueLDIndex = 0;
          if (ParseFloat(PartColorValueLDIndex, 0.0f, 1.0f))
          {
              if ((!AutomateEdgeColor && !Arguments.contains("-aec") && !Arguments.contains("--automate-edge-color")) ||
                  (StudStyle < HighContrastStudStyle && !Arguments.contains("-ss") && !Arguments.contains("--stud-style"))) {
                  InvalidParse(tr("Automate edge color or High contrast stud style is required for the"), true, false);
              }
              else if (ColorValueLDIndex != PartColorValueLDIndex)
              {
                  AutoEdgeColorChanged = true;
                  PartColorValueLDIndex = ColorValueLDIndex;
              }
          }
      }
      else
//      if (Param == QLatin1String("-im") || Param == QLatin1String("--image-matte"))
//        imageMatting = true;
//      else
      if (Param == QLatin1String("-of") || Param == QLatin1String("--output-file") || Param == QLatin1String("--pdf-output-file"))
        ParseString(Gui::saveFileName, true);
      else
      if (Param == QLatin1String("-rs") || Param == QLatin1String("--reset-search-dirs"))
          resetSearchDirs = true;
      else
      if (Param == QLatin1String("-x") || Param == QLatin1String("--clear-cache"))
        Gui::resetCache = true;
      else
      if (Param == QLatin1String("-p") || Param == QLatin1String("--preferred-renderer"))
        ParseString(preferredRenderer, true);
      else
      if (Param == QLatin1String("-pr") || Param == QLatin1String("--projection"))
        ParseString(projection, true);
      else
      if (Param == QLatin1String("-o") || Param == QLatin1String("--export-option"))
        ParseString(exportOption, true);
      else
      if (Param == QLatin1String("-od") || Param == QLatin1String("--export-directory"))
        ParseString(Gui::m_saveDirectoryName, true);
      else
      if (Param == QLatin1String("-r") || Param == QLatin1String("--range"))
        ParseString(pageRange, true);
      else
      if (Param == QLatin1String("-hw") || Param == QLatin1String("--highlight-line-width"))
      {
        if (!ParseInteger(highlightLineWidth, 1, 10))
            InvalidParse(tr("Invalid value specified, valid values range from 1 to 10 for the"), true);
        else if (Preferences::preferredRenderer != RENDERER_LDGLITE && (!Arguments.contains("-p") || !Arguments.contains("--preferred-renderer")))
            InvalidParse(tr("Renderer LDGlite is required for the"), true, false);
      }
      else
      if (Param == QLatin1String("-ccf") || Param == QLatin1String("--color-config-file"))
      {
        if (ParseString(colourConfigFile, true))
        {
            colourConfigFile = QDir::toNativeSeparators(colourConfigFile);
            if (QFileInfo(colourConfigFile).exists())
            {
                if (QDir::toNativeSeparators(Preferences::altLDConfigPath.toLower()) != colourConfigFile.toLower())
                    ColourConfigChanged = true;
            }
            else
            {
                InvalidParse(tr("Invalid value specified for the "), true);
                colourConfigFile = QString();
            }
        }
      }
      else
        InvalidParse(tr("Unknown %1 command line parameter:").arg(VER_PRODUCTNAME_STR), false, false);
  }

  auto restoreRendererAndLibrary = [&] ()
  {
      if (rendererChanged)
      {
          Preferences::preferredRenderer        = Gui::savedRendererData.renderer;
          Preferences::enableLDViewSingleCall   = Gui::savedRendererData.useLDVSingleCall ;
          Preferences::enableLDViewSnaphsotList = Gui::savedRendererData.useLDVSnapShotList;
          Preferences::useNativePovGenerator    = Gui::savedRendererData.useNativeGenerator;
          Preferences::perspectiveProjection    = Gui::savedRendererData.usePerspectiveProjection;
          Preferences::preferredRendererPreferences(true/*global*/);
      }

      if (! Preferences::currentLibrarySave.isEmpty())
      {
          QSettings Settings;
          // set library directly in setings
          Settings.setValue(QString("%1/%2").arg(SETTINGS,"LDrawLibrary"),Preferences::currentLibrarySave);
          // clear current library save
          Preferences::currentLibrarySave.clear();
          // call with empty library argument
          Preferences::setLPub3DAltLibPreferences(QString());
          // re-initialize directories
          Preferences::lpubPreferences();
      }
  };

  if (! ParseOK)
  {
      emit messageSig(LOG_ERROR,tr("Parse command line failed: %1.").arg(Arguments.join(" ")));

      disconnect(gui,  SIGNAL(fileLoadedSig(bool)), this, SLOT(  fileLoaded(bool)));

      restoreRendererAndLibrary();

      return 1;
  }

  int result = 0;

  if (!metaCommandsFile.isEmpty())
  {
      QString ignored;
      loadCommandCollection();
      if (!exportMetaCommands(metaCommandsFile, ignored))
          result = 1;
  }

  if (! preferredRenderer.isEmpty())
  {
     Gui::savedRendererData.renderer           = Preferences::preferredRenderer;
     Gui::savedRendererData.useLDVSingleCall   = Preferences::enableLDViewSingleCall;
     Gui::savedRendererData.useLDVSnapShotList = Preferences::enableLDViewSnaphsotList;
     Gui::savedRendererData.useNativeGenerator = Preferences::useNativePovGenerator;
     Gui::savedRendererData.usePerspectiveProjection = Preferences::perspectiveProjection;
     rendererChanged = setPreferredRendererFromCommand(preferredRenderer);
  }

  if (projection.toLower() == "p" || projection.toLower() == "perspective")
  {
      bool applyCARenderer = Preferences::preferredRenderer == RENDERER_LDVIEW;
      message = tr("Camera projection set to Perspective.%1")
                   .arg(applyCARenderer ?
                            tr(" Apply camera angles locally set to false") : "");
      emit messageSig(LOG_INFO,message);
      Preferences::applyCALocally = !applyCARenderer;
      Preferences::perspectiveProjection = true;
  }
  else if (projection.toLower() == "o" || projection.toLower() == "orthographic")
  {
      message = tr("Camera projection set to Orthographic");
      emit messageSig(LOG_INFO,message);
      Preferences::perspectiveProjection = false;
  }

  if (Preferences::sceneGuides)
      Preferences::setSceneGuidesPreference(false);

  if (Preferences::sceneRuler)
      Preferences::setSceneRulerPreference(false);

  if (Preferences::snapToGrid)
      Preferences::setSnapToGridPreference(false);

  if (fadeSteps && fadeSteps != Preferences::enableFadeSteps)
  {
      Preferences::enableFadeSteps = fadeSteps;
      message = tr("Fade Steps set to ON.");
      emit messageSig(LOG_INFO,message);
      if (fadeStepsColour.isEmpty())
      {
          if (Preferences::fadeStepsUseColour)
          {
              Preferences::fadeStepsUseColour = false;
              message = tr("Use Global Fade Color set to OFF.");
              emit messageSig(LOG_INFO,message);
          }
      }
  }

  if ((fadeStepsOpacity != Preferences::fadeStepsOpacity) && Preferences::enableFadeSteps)
  {
      message = tr("Fade Steps Transparency changed from %1 to %2 percent.")
          .arg(Preferences::fadeStepsOpacity)
          .arg(fadeStepsOpacity);
      emit messageSig(LOG_INFO,message);
      Preferences::fadeStepsOpacity = fadeStepsOpacity;
  }

  if (!fadeStepsColour.isEmpty() && Preferences::enableFadeSteps)
  {
      if (!Preferences::fadeStepsUseColour)
      {
          Preferences::fadeStepsUseColour = true;
          message = tr("Use Global Fade Color set to ON.");
          emit messageSig(LOG_INFO,message);
          fadeStepsOpacity = 100;
          if (fadeStepsOpacity != Preferences::fadeStepsOpacity )
          {
              message = tr("Fade Steps Transparency changed from %1 to %2 percent.")
                  .arg(Preferences::fadeStepsOpacity)
                  .arg(fadeStepsOpacity);
              emit messageSig(LOG_INFO,message);
              Preferences::fadeStepsOpacity = fadeStepsOpacity;
          }
      }
      ParsedColor = LDrawColor::color(fadeStepsColour);
      if (ParsedColor.isValid() &&
          fadeStepsColour.toLower() != Preferences::validFadeStepsColour.toLower())
      {
          bool isHexRGB = fadeStepsColour.contains(QRegExp("\\s*(0x|#)([\\da-fA-F]+)\\s*$"));
          QString const validColourName = isHexRGB ? LDrawColor::name(fadeStepsColour) : fadeStepsColour;
          message = tr("Fade Steps Color preference changed from %1 to %2.")
              .arg(QString(Preferences::validFadeStepsColour).replace("_"," "))
              .arg(QString(validColourName).replace("_"," "));
          emit messageSig(LOG_INFO,message);
          Preferences::validFadeStepsColour = validColourName;
      }
  }

  /* [Experimental] LDView Image Matting */
//  if (imageMatting && Preferences::enableFadeSteps &&
//      (Preferences::preferredRenderer == RENDERER_LDVIEW)) {
//      Preferences::enableImageMatting = imageMatting;
//      message = QString("Enable Image matte is ON.");
//      emit messageSig(LOG_INFO,message);
//    } else {
//      QString message;
//      if (imageMatting && !Preferences::enableFadeSteps) {
//          message = QString("Image matte requires fade previous steps set to ON.");
//          emit messageSig(LOG_ERROR,message);
//        }

//      if (imageMatting && (Preferences::preferredRenderer != RENDERER_LDVIEW)) {
//          message = QString("Image matte requires LDView renderer.");
//          emit messageSig(LOG_ERROR,message);
//        }
//      if (imageMatting) {
//          message = QString("Image matte flag will be ignored.");
//          emit messageSig(LOG_ERROR,message);
//        }
//    }

  if (highlightStep && highlightStep != Preferences::enableHighlightStep)
  {
      Preferences::enableHighlightStep = highlightStep;
      message = tr("Highlight Step set to ON.");
      emit messageSig(LOG_INFO,message);
  }

  if (!highlightStepColour.isEmpty() && Preferences::enableHighlightStep)
  {
      ParsedColor = LDrawColor::color(highlightStepColour);
      if (ParsedColor.isValid() &&
          highlightStepColour.toLower() != Preferences::highlightStepColour.toLower())
      {
          message = tr("Highlight Step Color preference changed from %1 to %2.")
              .arg(Preferences::highlightStepColour)
              .arg(highlightStepColour);
          emit messageSig(LOG_INFO,message);
          Preferences::highlightStepColour = highlightStepColour;
      }
  }

  if ((highlightLineWidth != Preferences::highlightStepLineWidth ) && Preferences::enableHighlightStep)
  {
      message = tr("Highlight Line Width preference changed from %1 to %2.")
          .arg(Preferences::highlightStepLineWidth)
          .arg(highlightLineWidth);
      emit messageSig(LOG_INFO,message);
      Preferences::highlightStepLineWidth = highlightLineWidth;
  }

  if (resetSearchDirs)
  {
      message = tr("Reset search directories requested..");
      emit messageSig(LOG_INFO,message);

      // set fade step setting
      if (fadeSteps && fadeSteps != Preferences::enableFadeSteps)
          Preferences::enableFadeSteps = fadeSteps;
      // set highlight step setting
      if (highlightStep && highlightStep != Preferences::enableHighlightStep)
          Preferences::enableHighlightStep = highlightStep;
      gui->partWorkerLDSearchDirs.resetSearchDirSettings();
  }

  if (!colourConfigFile.isEmpty())
      Preferences::altLDConfigPath = colourConfigFile;

  if (StudStyleChanged || StudStyleColourChanged || AutoEdgeColorChanged || ColourConfigChanged)
  {
      NativeOptions* viewerOptions = new NativeOptions;
      if (StudStyleChanged || StudStyleColourChanged)
      {
          viewerOptions->StudStyle                = StudStyle;
          viewerOptions->StudCylinderColorEnabled = StudCylinderColorEnabled;
          viewerOptions->StudCylinderColor        = StudCylinderColor;
          viewerOptions->PartEdgeColorEnabled     = PartEdgeColorEnabled;
          viewerOptions->PartEdgeColor            = PartEdgeColor;
          viewerOptions->BlackEdgeColorEnabled    = BlackEdgeColorEnabled;
          viewerOptions->BlackEdgeColor           = BlackEdgeColor;
          viewerOptions->DarkEdgeColorEnabled     = DarkEdgeColorEnabled;
          viewerOptions->DarkEdgeColor            = DarkEdgeColor;
          viewerOptions->LightDarkIndex           = PartColorValueLDIndex;
          if (StudStyleChanged)
              SetStudStyle(viewerOptions, false/*reload*/, true/*change*/);
          else if (StudStyleColourChanged)
          {
              lcPreferences& Preferences = lcGetPreferences();
              Preferences.mPartColorValueLDIndex = PartColorValueLDIndex;
              Preferences.mPartEdgeColorEnabled  = PartEdgeColorEnabled;
              Preferences.mPartEdgeColor         = PartEdgeColor;
              Preferences.mBlackEdgeColorEnabled = BlackEdgeColorEnabled;
              Preferences.mBlackEdgeColor        = BlackEdgeColor;
              Preferences.mDarkEdgeColorEnabled  = DarkEdgeColorEnabled;
              Preferences.mDarkEdgeColor         = DarkEdgeColor;
              lcGetPiecesLibrary()->LoadColors();
          }
      }
      else if (AutoEdgeColorChanged)
      {
          if (AutomateEdgeColor && StudStyle >= HighContrastStudStyle)
          {
              message = tr("High contrast stud and edge color settings are ignored when -aec or --automate-edge-color is set.");
              emit messageSig(LOG_NOTICE,message);
          }
          viewerOptions->AutoEdgeColor  = AutomateEdgeColor;
          viewerOptions->LightDarkIndex = PartColorValueLDIndex;
          viewerOptions->EdgeContrast   = PartEdgeContrast;
          viewerOptions->EdgeSaturation = PartColorValueLDIndex;
          SetAutomateEdgeColor(viewerOptions, true/*change*/);
      }
      else if (ColourConfigChanged)
          lcGetPiecesLibrary()->LoadColors();
  }

  if (!LPub::commandlineFile.isEmpty() )
  {
      if (QFileInfo(LPub::commandlineFile).isFile())
      {
          QElapsedTimer commandTimer;
          commandTimer.start();

          if (processExport)
              Gui::setExporting(true);
          if (processFile)
              Gui::setExporting(false);

          emit loadFileSig(commandlineFile, true/*CommandLine*/);

          auto wait = []( int millisecondsToWait )
          {
              QTime waitTime = QTime::currentTime().addMSecs( millisecondsToWait );
              QTime currentTime = QTime::currentTime();
              while( currentTime < waitTime )
                  QCoreApplication::processEvents( QEventLoop::AllEvents, millisecondsToWait );
          };

          int waited = 0;
          while (!LPub::mFileLoaded)
          {
              if (waited < Preferences::fileLoadWaitTime/*5 minutes*/)
              {
                  waited += 100;
                  wait(100);
              }
              else
              {
                  message = tr("File '%1' load exceeded the allotted time of %2 minutes. %3")
                               .arg(QFileInfo(LPub::commandlineFile).fileName())
                               .arg(Preferences::fileLoadWaitTime/60000)
                               .arg(LPub::elapsedTime(commandTimer.elapsed()));
                  emit messageSig(LOG_ERROR,message);
                  LPub::mFileLoadFail = true;
                  break;
              }
          }

          if (!LPub::mFileLoadFail && Gui::processPageRange(pageRange)) {

              int mode = PAGE_PROCESS;

              if (processExport)
              {
                  if (exportOption == QLatin1String("pdf"))
                     mode = EXPORT_PDF;
                  else
                  if (exportOption == QLatin1String("png"))
                     mode = EXPORT_PNG;
                  else
                  if (exportOption == QLatin1String("jpg"))
                     mode = EXPORT_JPG;
                  else
                  if (exportOption == QLatin1String("bmp"))
                     mode = EXPORT_BMP;
                  else
                  if (exportOption == QLatin1String("3ds"))
                     mode = EXPORT_3DS_MAX;
                  else
                  if (exportOption == QLatin1String("dae"))
                     mode = EXPORT_COLLADA;
                  else
                  if (exportOption == QLatin1String("obj"))
                     mode = EXPORT_WAVEFRONT;
                  else
                  if (exportOption == QLatin1String("stl"))
                     mode = EXPORT_STL;
                  else
                  if (exportOption == QLatin1String("pov"))
                     mode = EXPORT_POVRAY;
                  else
                  if (exportOption == QLatin1String("bl-xml"))
                     mode = EXPORT_BRICKLINK;
                  else
                  if (exportOption == QLatin1String("csv"))
                     mode = EXPORT_CSV;
                  else
                  if (exportOption == QLatin1String("htmlparts"))
                     mode = EXPORT_HTML_PARTS;
                  else
                  if (exportOption == QLatin1String("htmlsteps"))
                    mode = EXPORT_HTML_STEPS;
                  else
                     mode = EXPORT_PDF;
              }
              emit consoleCommandSig(mode, &result);
          }
          else
          {
              result = 1;
          }
          emit messageSig(LOG_INFO,tr("Model file '%1' process %2 (result code %3). %4.")
                          .arg(QFileInfo(LPub::commandlineFile).fileName())
                          .arg(result ? tr("failed") : tr("succeeded"))
                          .arg(result)
                          .arg(LPub::elapsedTime(commandTimer.elapsed())));
      }
      else
      {
          emit messageSig(LOG_ERROR,tr("Specified model file is not valid: %1.").arg(LPub::commandlineFile));
          result = 1;
      }
  }

  disconnect(gui,  SIGNAL(fileLoadedSig(bool)),
             this, SLOT(  fileLoaded(bool)));

  restoreRendererAndLibrary();

  return result;
}
