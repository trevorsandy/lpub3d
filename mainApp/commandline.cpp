/****************************************************************************
**
** Copyright (C) 2018 - 2022 Trevor SANDY. All rights reserved.
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
  quint32 StudCylinderColor     = GetStudCylinderColor();
  quint32 PartEdgeColor         = GetPartEdgeColor();
  quint32 BlackEdgeColor        = GetBlackEdgeColor();
  quint32 DarkEdgeColor         = GetDarkEdgeColor();
  float   PartEdgeContrast      = GetPartEdgeContrast();
  float   PartColorValueLDIndex = GetPartColorLightDarkIndex();
  bool    AutomateEdgeColor     = GetAutomateEdgeColor();

   int StudStyle             = GetStudStyle();
   int fadeStepsOpacity      = FADE_OPACITY_DEFAULT;
   int highlightLineWidth    = HIGHLIGHT_LINE_WIDTH_DEFAULT;
  bool processExport         = false;
  bool processFile           = false;
  bool fadeSteps             = false;
  bool highlightStep         = false;
// bool imageMatting          = false;
  bool resetSearchDirs       = false;
  bool coloursChanged        = false;
  bool rendererChanged       = false;
  bool studStyleChanged      = false;
  bool autoEdgeColorChanged  = false;
  QString pageRange, exportOption, colourConfigFile,
          preferredRenderer, projection, message,
          fadeStepsColour, highlightStepColour;

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
          commandlineFile = Param;
          continue;
      }

      bool IsExcluded = false;
      for (int i = 0; i < excludedCommands.size(); i++) {
          if (Param.startsWith(excludedCommands.at(i))) {
              IsExcluded = true;
              break;
          }
      }

      if (IsExcluded)
          continue;

      auto InvalidParse = [this, &Param, &Arguments, &ArgIdx, &ParseOK] (const QString& Text, bool Pair)
      {
          QString message = Text.isEmpty() ? QString("Invalid value specified") : Text;
          if (Pair)
              emit gui->messageSig(LOG_ERROR, message.append(QString(" '%1' option: '%2'.").arg(Arguments[ArgIdx - 1]).arg(Param)));
          else
              emit gui->messageSig(LOG_ERROR, message.append(QString(" '%1' option.").arg(Param)));

          ParseOK = false;
      };

      auto ParseString = [&InvalidParse, &ArgIdx, &Arguments, NumArguments](QString& Value, bool Required)
      {
          if (ArgIdx < NumArguments - 1 && Arguments[ArgIdx + 1][0] != '-')
          {
              ArgIdx++;
              Value = Arguments[ArgIdx];
          }
          else if (Required)
          {
              InvalidParse(QString("Not enough parameters for the"), false);
              return false;
          }

          return true;
      };

      auto ParseInteger = [&InvalidParse, &ArgIdx, &Arguments, NumArguments](int& Value, int Min, int Max)
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
                  InvalidParse(QString("Invalid parameter value specified for the"), true);
          }
          else
              InvalidParse(QString("Not enough parameters for the"), false);

          return false;
      };

      auto ParseFloat = [&InvalidParse, &ArgIdx, &Arguments, NumArguments](float& Value, float Min, float Max)
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
                  InvalidParse(QString("Invalid parameter value specified for the"), true);
          }
          else
              InvalidParse(QString("Not enough parameters for the"), false);

          return false;
      };

      auto ParseColor32 = [&InvalidParse, &ArgIdx, &Arguments, NumArguments](quint32& Color)
      {
          if (ArgIdx < NumArguments - 1 && Arguments[ArgIdx + 1][0] != '-')
          {
              ArgIdx++;
              QString Parameter = Arguments[ArgIdx];
              QColor ParsedColor = QColor(Parameter);

              if (ParsedColor.isValid())
              {
                  Color = LC_RGBA(ParsedColor.red(), ParsedColor.green(), ParsedColor.blue(), ParsedColor.alpha());
                  return true;
              }
              else
                  InvalidParse(QString("Not enough parameters for the"), false);
          }
          else
              InvalidParse(QString("Not enough parameters for the"), false);

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
      if (Param == QLatin1String("-ss") || Param == QLatin1String("--stud-style"))
      {
        int Value;
        if (!ParseInteger(Value, 0, 7))
            InvalidParse(QString("Invalid value specified, valid values range from 0 to 7 for the"), true);
        else if ((studStyleChanged = Value != StudStyle))
            StudStyle = Value;
      }
      else if (Param == QLatin1String("-scc") || Param == QLatin1String("--stud-cylinder-color"))
      {
          if (ParseColor32(ColorValue))
          {
              if (StudStyle < 6)
                  InvalidParse(QString("High contrast stud style is required for the"), true);
              else if ((coloursChanged = ColorValue != StudCylinderColor))
                  StudCylinderColor = ColorValue;
          }
      }
      else if (Param == QLatin1String("-ec") || Param == QLatin1String("--edge-color"))
      {
          if (ParseColor32(PartEdgeColor))
          {
              if (StudStyle < 6)
                  InvalidParse(QString("High contrast stud style is required for the"), true);
              else if ((coloursChanged = ColorValue != PartEdgeColor))
                  PartEdgeColor = ColorValue;
          }
      }
      else if (Param == QLatin1String("-bec") || Param == QLatin1String("--black-edge-color"))
      {
          if (ParseColor32(BlackEdgeColor))
          {
              if (StudStyle < 6)
                  InvalidParse(QString("High contrast stud style is required for the"), true);
              else if ((coloursChanged = ColorValue != BlackEdgeColor))
                  BlackEdgeColor = ColorValue;
          }
      }
      else if (Param == QLatin1String("-dec") || Param == QLatin1String("--dark-edge-color"))
      {
          if (ParseColor32(DarkEdgeColor))
          {
              if (StudStyle < 6)
                  InvalidParse(QString("High contrast stud style is required for the"), true);
              else if ((coloursChanged = ColorValue != DarkEdgeColor))
                  DarkEdgeColor = ColorValue;
          }
      }
      else
      if (Param == QLatin1String("-aec") || Param == QLatin1String("--automate-edge-color"))
      {
          if (!AutomateEdgeColor)
              autoEdgeColorChanged = true;
          AutomateEdgeColor = true;
      }
      else
      if (Param == QLatin1String("-cc") || Param == QLatin1String("--color-contrast"))
      {
          if (ParseFloat(PartEdgeContrast, 0.0f, 1.0f))
              if (!AutomateEdgeColor)
                  InvalidParse(QString("Automate edge color is required for the"), true);
      }
      else
      if (Param == QLatin1String("-ldv") || Param == QLatin1String("--light-dark-value"))
      {
          if (ParseFloat(PartColorValueLDIndex, 0.0f, 1.0f))
              if (!AutomateEdgeColor)
                  InvalidParse(QString("Automate edge color is required for the"), true);
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
            InvalidParse(QString("Invalid value specified, valid values range from 1 to 10 for the"), true);
      }
      else
      if (Param == QLatin1String("-ccf") || Param == QLatin1String("--color-config-file"))
      {
        if (ParseString(colourConfigFile, true))
        {
            if (QFileInfo(colourConfigFile).exists())
            {
                coloursChanged = true;
            }
            else
            {
                InvalidParse(QString("Invalid value specified for the "), true);
                colourConfigFile = QString();
            }
        }
    }
    else
      InvalidParse(QString("Unknown %1 command line parameter:").arg(VER_PRODUCTNAME_STR), false);
  }
    
  if (! ParseOK)
  {
      emit gui->messageSig(LOG_ERROR,QString("Parse command line failed: %1.").arg(Arguments.join(" ")));
      return 1;
  }

  if (! QFileInfo(commandlineFile).exists()) {
      emit gui->messageSig(LOG_ERROR,QString("Specified model file was not found: %1.").arg(commandlineFile));
      return 1;
  }

  if (! preferredRenderer.isEmpty()) {
     Gui::savedData.renderer           = Preferences::preferredRenderer;
     Gui::savedData.useLDVSingleCall   = Preferences::enableLDViewSingleCall;
     Gui::savedData.useLDVSnapShotList = Preferences::enableLDViewSnaphsotList;
     Gui::savedData.useNativeGenerator = Preferences::useNativePovGenerator;
     Gui::savedData.usePerspectiveProjection = Preferences::perspectiveProjection;
     rendererChanged = setPreferredRendererFromCommand(preferredRenderer);
  }

  if (projection.toLower() == "p" || projection.toLower() == "perspective") {
      bool applyCARenderer = Preferences::preferredRenderer == RENDERER_LDVIEW;
      message = QString("Camera projection set to Perspective.%1")
                        .arg(applyCARenderer ?
                                 QString(" Apply camera angles locally set to false") : "");
      emit gui->messageSig(LOG_INFO,message);
      Preferences::applyCALocally = !applyCARenderer;
      Preferences::perspectiveProjection = true;
  } else if (projection.toLower() == "o" || projection.toLower() == "orthographic") {
      message = QString("Camera projection set to Orthographic");
      emit gui->messageSig(LOG_INFO,message);
      Preferences::perspectiveProjection = false;
  }

  if (Preferences::sceneGuides)
      Preferences::setSceneGuidesPreference(false);

  if (Preferences::sceneRuler)
      Preferences::setSceneRulerPreference(false);

  if (Preferences::snapToGrid)
      Preferences::setSnapToGridPreference(false);

  if (fadeSteps && fadeSteps != Preferences::enableFadeSteps) {
      Preferences::enableFadeSteps = fadeSteps;
      message = QString("Fade Previous Steps set to ON.");
      emit gui->messageSig(LOG_INFO,message);
      if (fadeStepsColour.isEmpty()) {
          if (Preferences::fadeStepsUseColour){
              Preferences::fadeStepsUseColour = false;
              message = QString("Use Global Fade Color set to OFF.");
              emit gui->messageSig(LOG_INFO,message);
            }
        }
    }

  if ((fadeStepsOpacity != Preferences::fadeStepsOpacity) &&
      Preferences::enableFadeSteps) {
          message = QString("Fade Step Transparency changed from %1 to %2 percent.")
              .arg(Preferences::fadeStepsOpacity)
              .arg(fadeStepsOpacity);
          emit gui->messageSig(LOG_INFO,message);
          Preferences::fadeStepsOpacity = fadeStepsOpacity;
    }

  if (!fadeStepsColour.isEmpty() && Preferences::enableFadeSteps) {
      if (!Preferences::fadeStepsUseColour) {
          Preferences::fadeStepsUseColour = true;
          message = QString("Use Global Fade Color set to ON.");
          emit gui->messageSig(LOG_INFO,message);
          fadeStepsOpacity = 100;
          if (fadeStepsOpacity != Preferences::fadeStepsOpacity ) {
              message = QString("Fade Step Transparency changed from %1 to %2 percent.")
                  .arg(Preferences::fadeStepsOpacity)
                  .arg(fadeStepsOpacity);
              emit gui->messageSig(LOG_INFO,message);
              Preferences::fadeStepsOpacity = fadeStepsOpacity;
            }
        }
      QColor ParsedColor = LDrawColor::color(fadeStepsColour);
      if (ParsedColor.isValid() &&
          fadeStepsColour.toLower() != Preferences::validFadeStepsColour.toLower()) {
          message = QString("Fade Step Color preference changed from %1 to %2.")
              .arg(QString(Preferences::validFadeStepsColour).replace("_"," "))
              .arg(QString(LDrawColor::name(fadeStepsColour)).replace("_"," "));
          emit gui->messageSig(LOG_INFO,message);
          Preferences::validFadeStepsColour = LDrawColor::name(fadeStepsColour);
        }
    }

  /* [Experimental] LDView Image Matting */
//  if (imageMatting && Preferences::enableFadeSteps &&
//      (Preferences::preferredRenderer == RENDERER_LDVIEW)) {
//      Preferences::enableImageMatting = imageMatting;
//      message = QString("Enable Image matte is ON.");
//      emit gui->messageSig(LOG_INFO,message);
//    } else {
//      QString message;
//      if (imageMatting && !Preferences::enableFadeSteps) {
//          message = QString("Image matte requires fade previous steps set to ON.");
//          emit gui->messageSig(LOG_ERROR,message);
//        }

//      if (imageMatting && (Preferences::preferredRenderer != RENDERER_LDVIEW)) {
//          message = QString("Image matte requires LDView renderer.");
//          emit gui->messageSig(LOG_ERROR,message);
//        }
//      if (imageMatting) {
//          message = QString("Image matte flag will be ignored.");
//          emit gui->messageSig(LOG_ERROR,message);
//        }
//    }

  if (highlightStep && highlightStep != Preferences::enableHighlightStep) {
      Preferences::enableHighlightStep = highlightStep;
      message = QString("Highlight Current Step set to ON.");
      emit gui->messageSig(LOG_INFO,message);
    }

  QColor ParsedColor = LDrawColor::color(fadeStepsColour);
  if (ParsedColor.isValid() &&
      highlightStepColour.toLower() != Preferences::highlightStepColour.toLower() &&
      Preferences::enableHighlightStep) {
      message = QString("Highlight Step Color preference changed from %1 to %2.")
          .arg(Preferences::highlightStepColour)
          .arg(highlightStepColour);
      emit gui->messageSig(LOG_INFO,message);
      Preferences::highlightStepColour = highlightStepColour;
    }

  if ((highlightLineWidth != Preferences::highlightStepLineWidth ) &&
      Preferences::enableHighlightStep) {
      message = QString("Highlight Line Width preference changed from %1 to %2.")
          .arg(Preferences::highlightStepLineWidth)
          .arg(highlightLineWidth);
      emit gui->messageSig(LOG_INFO,message);
      Preferences::highlightStepLineWidth = highlightLineWidth;
    }

  if (resetSearchDirs) {
      message = QString("Reset search directories requested..");
      emit gui->messageSig(LOG_INFO,message);

      // set fade step setting
      if (fadeSteps && fadeSteps != Preferences::enableFadeSteps) {
          Preferences::enableFadeSteps = fadeSteps;
        }
      // set highlight step setting
      if (highlightStep && highlightStep != Preferences::enableHighlightStep) {
          Preferences::enableHighlightStep = highlightStep;
        }
      gui->partWorkerLDSearchDirs.resetSearchDirSettings();
    }

  if (!colourConfigFile.isEmpty())
      Preferences::altLDConfigPath = colourConfigFile;

  if (studStyleChanged || autoEdgeColorChanged) {
      NativeOptions* viewerOptions = new NativeOptions;
      if (studStyleChanged) {
          viewerOptions->StudStyle         = StudStyle;
          viewerOptions->LightDarkIndex    = PartColorValueLDIndex;
          viewerOptions->StudCylinderColor = StudCylinderColor;
          viewerOptions->PartEdgeColor     = PartEdgeColor;
          viewerOptions->BlackEdgeColor    = BlackEdgeColor;
          viewerOptions->DarkEdgeColor     = DarkEdgeColor;
          SetStudStyle(viewerOptions, false);
      } else if (autoEdgeColorChanged) {
          if (AutomateEdgeColor && StudStyle > 5) {
              message = tr("High contrast stud and edge color settings are ignored when -aec or --automate-edge-color is set.");
              emit gui->messageSig(LOG_NOTICE,message);
          }
          viewerOptions->AutoEdgeColor     = AutomateEdgeColor;
          viewerOptions->EdgeContrast      = PartEdgeContrast;
          viewerOptions->EdgeSaturation    = PartColorValueLDIndex;
          SetAutomateEdgeColor(viewerOptions);
      }
  } else if (coloursChanged) {
      lcGetPiecesLibrary()->LoadColors();
  }

  QElapsedTimer commandTimer;
  if (!commandlineFile.isEmpty()) {
      if (processExport)
          Gui::setExporting(true);
      if (processFile)
          Gui::setExporting(false);

      commandTimer.start();

      emit loadFileSig(commandlineFile, true);

      auto wait = []( int millisecondsToWait )
      {
          QTime waitTime = QTime::currentTime().addMSecs( millisecondsToWait );
          while( QTime::currentTime() < waitTime )
              QCoreApplication::processEvents( QEventLoop::AllEvents, 100 );
      };

      int waited = 0;
      while (!LPub::mFileLoaded) {
          if (waited < Preferences::fileLoadWaitTime/*5 minutes*/) {
              waited += 100;
              wait(100);
          } else {
              message = tr("File '%1' load exceeded the allotted time of %2 minutes. %3")
                           .arg(QFileInfo(commandlineFile).fileName())
                           .arg(Preferences::fileLoadWaitTime/60000)
                           .arg(LPub::elapsedTime(commandTimer.elapsed()));
              emit gui->messageSig(LOG_ERROR,message);
              return -1;
          }
      }

      if (LPub::mFileLoadFail)
          return -1;
  }

  int mode = PAGE_PROCESS;

  int result = 0;

  if (Gui::processPageRange(pageRange)) {

      if (processExport) {
          if (exportOption == "pdf")
             mode = EXPORT_PDF;
          else
          if (exportOption == "png")
             mode = EXPORT_PNG;
          else
          if (exportOption == "jpg")
             mode = EXPORT_JPG;
          else
          if (exportOption == "bmp")
             mode = EXPORT_BMP;
          else
          if (exportOption == "3ds")
             mode = EXPORT_3DS_MAX;
          else
          if (exportOption == "dae")
             mode = EXPORT_COLLADA;
          else
          if (exportOption == "obj")
             mode = EXPORT_WAVEFRONT;
          else
          if (exportOption == "stl")
             mode = EXPORT_STL;
          else
          if (exportOption == "pov")
             mode = EXPORT_POVRAY;
          else
          if (exportOption == "bl-xml")
             mode = EXPORT_BRICKLINK;
          else
          if (exportOption == "csv")
             mode = EXPORT_CSV;
          else
          if (exportOption == "htmlparts")
             mode = EXPORT_HTML_PARTS;
          else
          if (exportOption == "htmlsteps")
            mode = EXPORT_HTML_STEPS;
          else
             mode = EXPORT_PDF;
      }

      emit consoleCommandSig(mode, &result);

  } else {
      return 1;
  }

  if (rendererChanged) {
      Preferences::preferredRenderer        = Gui::savedData.renderer;
      Preferences::enableLDViewSingleCall   = Gui::savedData.useLDVSingleCall ;
      Preferences::enableLDViewSnaphsotList = Gui::savedData.useLDVSnapShotList;
      Preferences::useNativePovGenerator    = Gui::savedData.useNativeGenerator;
      Preferences::perspectiveProjection    = Gui::savedData.usePerspectiveProjection;
      Preferences::preferredRendererPreferences(true/*global*/);
  }

  emit gui->messageSig(LOG_INFO,QString("Model file '%1' processed. %2.")
                  .arg(QFileInfo(commandlineFile).fileName())
                  .arg(LPub::elapsedTime(commandTimer.elapsed())));

  disconnect(gui,  SIGNAL(fileLoadedSig(bool)),
             this, SLOT(  fileLoaded(bool)));

  return result;
}
