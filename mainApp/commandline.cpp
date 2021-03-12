/****************************************************************************
**
** Copyright (C) 2018 - 2021 Trevor SANDY. All rights reserved.
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

#include "application.h"
#include "lc_profile.h"
#include "lc_application.h"
#include "lpub.h"

int Gui::processCommandLine()
{
  // 3DViewer
  int viewerCommand = Process3DViewerCommandLine();
  if (viewerCommand < 0)
     return 1;
  else
  if (viewerCommand)
    return 0;

  // Declarations
  QStringList excludedCommands = QStringList()
    // Application::initialize arguments
    << "-ns" << "--no-stdout-log"
    << "-ll" << "--liblego"
    << "-lt" << "--libtente"
    << "-lv" << "--libvexiq"
    // 3DViewer arguments
    << "-l" << "--libpath"
    << "-i" << "--image"
    << "-w" << "--width"
    << "-h" << "--height"
    << "-f" << "--from"
    << "-t" << "--to"
    << "-s" << "--submodel"
    << "-c" << "--camera"
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
    << "-obj" << "--export-wavefront"
    << "-3ds" << "--export-3ds"
    << "-dae" << "--export-collada"
    << "-html" << "--export-html"
    ;

  quint32 ColorValue            = 0;
  quint32 StudCylinderColor     = gui->GetStudCylinderColor();
  quint32 PartEdgeColor         = gui->GetPartEdgeColor();
  quint32 BlackEdgeColor        = gui->GetBlackEdgeColor();
  quint32 DarkEdgeColor         = gui->GetDarkEdgeColor();
  float   PartEdgeContrast      = gui->GetPartEdgeContrast();
  float   PartColorValueLDIndex = gui->GetPartColorLightDarkIndex();
  bool    AutomateEdgeColor     = gui->GetAutomateEdgeColor();

   int StudStyle             = GetStudStyle();
   int fadeStepsOpacity      = FADE_OPACITY_DEFAULT;
   int highlightLineWidth    = HIGHLIGHT_LINE_WIDTH_DEFAULT;
  bool processExport         = false;
  bool processFile           = false;
  bool fadeSteps             = false;
  bool highlightStep         = false;
// bool imageMatting          = false;
  bool resetSearchDirs       = false;
  bool useLDVSingleCall      = false;
  bool useLDVSnapShotList    = false;
  bool useNativeRenderer     = false;
  bool coloursChanged        = false;
  bool studStyleChanged      = false;
  bool autoEdgeColorChanged  = false;

  QString generator          = RENDERER_NATIVE;

  QString pageRange, exportOption, colourConfigFile,
          commandlineFile, preferredRenderer, projection,
          fadeStepsColour, highlightStepColour, message;

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
              emit messageSig(LOG_ERROR, message.append(QString(" '%1' option: '%2'.").arg(Arguments[ArgIdx - 1]).arg(Param)));
          else
              emit messageSig(LOG_ERROR, message.append(QString(" '%1' option.").arg(Param)));

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
      if (Param == QLatin1String("-of") || Param == QLatin1String("--pdf-output-file"))
        ParseString(saveFileName, true);
      else
      if (Param == QLatin1String("-rs") || Param == QLatin1String("--reset-search-dirs"))
          resetSearchDirs = true;
      else
      if (Param == QLatin1String("-x") || Param == QLatin1String("--clear-cache"))
        resetCache = true;
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
      if (Param == QLatin1String("-d") || Param == QLatin1String("--image-output-directory"))
        ParseString(saveFileName, true);
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
    
  if (!ParseOK)
  {
      return 1;
  }

  if (!preferredRenderer.isEmpty()){
      //QSettings Settings;
      QString renderer;
      if (preferredRenderer.toLower() == "native"){
          renderer = RENDERER_NATIVE;
          useNativeRenderer = true;
      }
      else
      if (preferredRenderer.toLower() == "ldview"){
          renderer = RENDERER_LDVIEW;
      }
      else
      if (preferredRenderer.toLower() == "ldview-sc"){
          renderer = RENDERER_LDVIEW;
          useLDVSingleCall = true;
      }
      else
      if (preferredRenderer.toLower() == "ldview-scsl"){
          renderer = RENDERER_LDVIEW;
          useLDVSingleCall = true;
          useLDVSnapShotList = true;
      }
      else
      if (preferredRenderer.toLower() == "ldglite"){
          renderer = RENDERER_LDGLITE;
      }
      else
      if (preferredRenderer.toLower() == "povray"){
          renderer = RENDERER_POVRAY;
      }
      else
      if (preferredRenderer.toLower() == "povray-ldv"){
          renderer = RENDERER_POVRAY;
          generator = RENDERER_LDVIEW;
      }
      else {
          emit messageSig(LOG_INFO,QString("Invalid renderer option specified: '%1'.").arg(renderer));
          return 1;
      }
      if (Preferences::preferredRenderer != renderer) {
          if (preferredRenderer.toLower().contains("ldview")) {
              if (Preferences::enableLDViewSingleCall != useLDVSingleCall) {
                  message = QString("Renderer preference use LDView Single Call changed from %1 to %2.")
                      .arg(Preferences::enableLDViewSingleCall ? "Yes" : "No")
                      .arg(useLDVSingleCall ? "Yes" : "No");
                  emit messageSig(LOG_INFO,message);
                  Preferences::enableLDViewSingleCall = useLDVSingleCall;
              }
              if (Preferences::enableLDViewSnaphsotList != useLDVSnapShotList) {
                  message = QString("Renderer preference use LDView Snapshot List changed from %1 to %2.")
                      .arg(Preferences::enableLDViewSnaphsotList ? "Yes" : "No")
                      .arg(useLDVSingleCall ? "Yes" : "No");
                  emit messageSig(LOG_INFO,message);
                  Preferences::enableLDViewSnaphsotList = useLDVSnapShotList;
              }
          }
          else
          if (preferredRenderer.toLower().contains("povray")) {
              if (Preferences::povFileGenerator != generator) {
                  message = QString("Renderer preference POV file generator changed from %1 to %2.")
                      .arg(Preferences::povFileGenerator)
                      .arg(generator);
                  emit messageSig(LOG_INFO,message);
                  Preferences::povFileGenerator = generator;
              }
          }
          message = QString("Renderer preference changed from %1 to %2%3.")
              .arg(Preferences::preferredRenderer)
              .arg(renderer)
              .arg(renderer == RENDERER_POVRAY ? QString(" (POV file generator is %1)").arg(generator) :
                   renderer == RENDERER_LDVIEW ? useLDVSingleCall ?
                                                 useLDVSnapShotList ? " (Single Call w/ SnapShot List)" : " (Single Call)" :
                                                 QString() : QString());
          emit messageSig(LOG_INFO,message);
          Preferences::preferredRenderer   = renderer;
          Preferences::usingNativeRenderer = useNativeRenderer;
          Render::setRenderer(Preferences::preferredRenderer);
          Preferences::updatePOVRayConfigFiles();
      }
  }

  if (projection.toLower() == "p" || projection.toLower() == "perspective") {
      bool applyCARenderer = Preferences::preferredRenderer == RENDERER_LDVIEW;
      message = QString("Camera projection set to Perspective.%1")
                        .arg(applyCARenderer ?
                                 QString(" Apply camera angles locally set to false") : "");
      emit messageSig(LOG_INFO,message);
      Preferences::applyCALocally = !applyCARenderer;
      Preferences::perspectiveProjection = true;
  } else if (projection.toLower() == "o" || projection.toLower() == "orthographic") {
      message = QString("Camera projection set to Orthographic");
      emit messageSig(LOG_INFO,message);
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
      emit messageSig(LOG_INFO,message);
      if (fadeStepsColour.isEmpty()) {
          if (Preferences::fadeStepsUseColour){
              Preferences::fadeStepsUseColour = false;
              message = QString("Use Global Fade Color set to OFF.");
              emit messageSig(LOG_INFO,message);
            }
        }
    }

  if ((fadeStepsOpacity != Preferences::fadeStepsOpacity) &&
      Preferences::enableFadeSteps) {
          message = QString("Fade Step Transparency changed from %1 to %2 percent.")
              .arg(Preferences::fadeStepsOpacity)
              .arg(fadeStepsOpacity);
          emit messageSig(LOG_INFO,message);
          Preferences::fadeStepsOpacity = fadeStepsOpacity;
    }

  if (!fadeStepsColour.isEmpty() && Preferences::enableFadeSteps) {
      if (!Preferences::fadeStepsUseColour){
          Preferences::fadeStepsUseColour = true;
          message = QString("Use Global Fade Color set to ON.");
          emit messageSig(LOG_INFO,message);
          fadeStepsOpacity = 100;
          if (fadeStepsOpacity != Preferences::fadeStepsOpacity ) {
              message = QString("Fade Step Transparency changed from %1 to %2 percent.")
                  .arg(Preferences::fadeStepsOpacity)
                  .arg(fadeStepsOpacity);
              emit messageSig(LOG_INFO,message);
              Preferences::fadeStepsOpacity = fadeStepsOpacity;
            }
        }
      if (LDrawColor::name(fadeStepsColour) != Preferences::validFadeStepsColour) {
          message = QString("Fade Step Color preference changed from %1 to %2.")
              .arg(QString(Preferences::validFadeStepsColour).replace("_"," "))
              .arg(QString(LDrawColor::name(fadeStepsColour)).replace("_"," "));
          emit messageSig(LOG_INFO,message);
          Preferences::validFadeStepsColour = LDrawColor::name(fadeStepsColour);
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

  if (highlightStep && highlightStep != Preferences::enableHighlightStep) {
      Preferences::enableHighlightStep = highlightStep;
      message = QString("Highlight Current Step set to ON.");
      emit messageSig(LOG_INFO,message);
    }

  if ((highlightStepColour != Preferences::highlightStepColour) &&
      !highlightStepColour.isEmpty() &&
      Preferences::enableHighlightStep) {
      message = QString("Highlight Step Color preference changed from %1 to %2.")
          .arg(Preferences::highlightStepColour)
          .arg(highlightStepColour);
      emit messageSig(LOG_INFO,message);
      Preferences::highlightStepColour = highlightStepColour;
    }

  if ((highlightLineWidth != Preferences::highlightStepLineWidth ) &&
      Preferences::enableHighlightStep) {
      message = QString("Highlight Line Width preference changed from %1 to %2.")
          .arg(Preferences::highlightStepLineWidth)
          .arg(highlightLineWidth);
      emit messageSig(LOG_INFO,message);
      Preferences::highlightStepLineWidth = highlightLineWidth;
    }

  if (resetSearchDirs) {
      message = QString("Reset search directories requested..");
      emit messageSig(LOG_INFO,message);

      // set fade step setting
      if (fadeSteps && fadeSteps != Preferences::enableFadeSteps) {
          Preferences::enableFadeSteps = fadeSteps;
        }
      // set highlight step setting
      if (highlightStep && highlightStep != Preferences::enableHighlightStep) {
          Preferences::enableHighlightStep = highlightStep;
        }
      partWorkerLDSearchDirs.resetSearchDirSettings();
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
              emit messageSig(LOG_NOTICE,message);
          }
          viewerOptions->AutoEdgeColor     = AutomateEdgeColor;
          viewerOptions->EdgeContrast      = PartEdgeContrast;
          viewerOptions->EdgeSaturation    = PartColorValueLDIndex;
          SetAutomateEdgeColor(viewerOptions);
      }
  } else if (coloursChanged) {
      LoadColors();
  }

  QElapsedTimer commandTimer;
  if (!commandlineFile.isEmpty()) {
      if (processExport)
          setExporting(true);
      if (processFile)
          setExporting(false);
      if(resetCache) {
          emit messageSig(LOG_INFO,QString("Reset parts cache specified."));
          resetModelCache(QFileInfo(commandlineFile).absoluteFilePath(), true/*commandLine*/);
      }
      commandTimer.start();
      if (!loadFile(commandlineFile)) {
          return 1;
      }
  }

  if (processPageRange(pageRange)) {
      if (processFile){
          Preferences::pageDisplayPause = 1;
          continuousPageDialog(PAGE_NEXT);
        } else
        if (processExport) {
            if (exportOption == "pdf")
               exportAsPdfDialog();
            else
            if (exportOption == "png")
               exportAsPngDialog();
            else
            if (exportOption == "jpg")
               exportAsJpgDialog();
            else
            if (exportOption == "bmp")
               exportAsBmpDialog();
            else
            if (exportOption == "stl")
               exportAsStlDialog();
            else
            if (exportOption == "3ds")
               exportAs3dsDialog();
            else
            if (exportOption == "pov")
               exportAsPovDialog();
            else
            if (exportOption == "dae")
               exportAsColladaDialog();
            else
            if (exportOption == "obj")
               exportAsObjDialog();
            else
               exportAsPdfDialog();
          } else {
            continuousPageDialog(PAGE_NEXT);
          }
    } else {
       return 1;
    }

  emit messageSig(LOG_INFO,QString("Model file '%1' processed. %2.")
                  .arg(QFileInfo(commandlineFile).fileName())
                  .arg(gui->elapsedTime(commandTimer.elapsed())));
  return 0;
}
