/****************************************************************************
**
** Copyright (C) 2018 Trevor SANDY. All rights reserved.
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
#include "lpub.h"

int Gui::processCommandLine()
{
  // 3DViewer
  int viewerJob = gApplication->Process3DViewerCommandLine();
  if (viewerJob < 0)
     return 1;
  else
  if (viewerJob > 0)
    return 0;

  // Declarations
  int  fadeStepsOpacity    = FADE_OPACITY_DEFAULT;
  float highlightLineWidth = HIGHLIGHT_LINE_WIDTH_DEFAULT;
  bool processExport       = false;
  bool processFile         = false;
  bool fadeSteps           = false;
  bool highlightStep       = false;
  bool imageMatting        = false;

  QString pageRange, exportOption,
          commandlineFile, preferredRenderer,
          fadeStepsColour, highlightStepColour;

  // Process parameters
  QStringList Arguments = Application::instance()->arguments();

  const int NumArguments = Arguments.size();
  for (int ArgIdx = 1; ArgIdx < NumArguments; ArgIdx++)
    {
      const QString& Param = Arguments[ArgIdx];

      if (Param[0] != '-')
      {
          commandlineFile = Param;
          continue;
      }

      if (Param == QLatin1String("-icr") ||
          Param == QLatin1String("--ignore-console-redirect"))
      {
          continue;
      }

      auto ParseString = [&ArgIdx, &Arguments, NumArguments](QString& Value, bool Required)
      {
          if (ArgIdx < NumArguments - 1 && Arguments[ArgIdx + 1][0] != '-')
            {
              ArgIdx++;
              Value = Arguments[ArgIdx];
            }
          else if (Required)
            printf("Not enough parameters for the '%s' argument.\n", Arguments[ArgIdx].toLatin1().constData());
        };

      auto ParseInteger = [&ArgIdx, &Arguments, NumArguments](int& Value)
      {
          if (ArgIdx < NumArguments - 1 && Arguments[ArgIdx + 1][0] != '-')
          {
              bool Ok = false;
              ArgIdx++;
              int NewValue = Arguments[ArgIdx].toInt(&Ok);

              if (Ok)
                  Value = NewValue;
              else
                  printf("Invalid value specified for the '%s' argument.\n", Arguments[ArgIdx - 1].toLatin1().constData());
          }
          else
              printf("Not enough parameters for the '%s' argument.\n", Arguments[ArgIdx].toLatin1().constData());
      };

      auto ParseFloat = [&ArgIdx, &Arguments, NumArguments](float& Value)
      {
          if (ArgIdx < NumArguments - 1 && Arguments[ArgIdx + 1][0] != '-')
          {
              bool Ok = false;
              ArgIdx++;
              int NewValue = Arguments[ArgIdx].toFloat(&Ok);

	      if (Ok)
		  Value = NewValue;
	      else
		  printf("Invalid value specified for the '%s' argument.\n", Arguments[ArgIdx - 1].toLatin1().constData());
	  }
	  else
	      printf("Not enough parameters for the '%s' argument.\n", Arguments[ArgIdx].toLatin1().constData());
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
        ParseInteger(fadeStepsOpacity);
      else
      if (Param == QLatin1String("-fc") || Param == QLatin1String("--fade-steps-colour"))
        ParseString(fadeStepsColour, false);
      else
      if (Param == QLatin1String("-hs") || Param == QLatin1String("--highlight-step"))
        highlightStep = true;
      else
      if (Param == QLatin1String("-hc") || Param == QLatin1String("--highlight-step-colour"))
        ParseString(highlightStepColour, false);
      else
      if (Param == QLatin1String("-im") || Param == QLatin1String("--image-matte"))
        imageMatting = true;
      else
      if (Param == QLatin1String("-of") || Param == QLatin1String("--pdf-output-file"))
        ParseString(saveFileName, false);
      else
      if (Param == QLatin1String("-x") || Param == QLatin1String("--clear-cache"))
        resetCache = true;
      else
      if (Param == QLatin1String("-p") || Param == QLatin1String("--preferred-renderer"))
        ParseString(preferredRenderer, false);
      else
      if (Param == QLatin1String("-o") || Param == QLatin1String("--export-option"))
        ParseString(exportOption, false);
      else
      if (Param == QLatin1String("-d") || Param == QLatin1String("--image-output-directory"))
        ParseString(saveFileName, false);
      else
      if (Param == QLatin1String("-r") || Param == QLatin1String("--range"))
        ParseString(pageRange, false);
      else
      if (Param == QLatin1String("--line-width"))
        ParseFloat(highlightLineWidth);
      else
        emit messageSig(LOG_INFO,QString("Unknown commandline parameter: '%1'").arg(Param));
    }

  if (!preferredRenderer.isEmpty()){
      //QSettings Settings;
      QString renderer;
      if (preferredRenderer.toLower() == "native"){
          renderer = RENDERER_NATIVE;
      }
      else
      if (preferredRenderer.toLower() == "ldview"){
          renderer = RENDERER_LDVIEW;
      }
      else
      if (preferredRenderer.toLower() == "ldview-sc"){
          renderer = RENDERER_LDVIEW;
          Preferences::useLDViewSingleCall = true;
      }
      else
      if (preferredRenderer.toLower() == "ldglite"){
          renderer = RENDERER_LDGLITE;
      }
      else
      if (preferredRenderer.toLower() == "povray"){
          renderer = RENDERER_POVRAY;
      }
      else {
          emit messageSig(LOG_INFO,QString("Invalid renderer specified: '%1'").arg(renderer));
          return 1;
      }
      if (Preferences::preferredRenderer != renderer) {
          Preferences::preferredRenderer = renderer;
          Render::setRenderer(Preferences::preferredRenderer);
          QString message = QString("Renderer is %1 %2").arg(renderer).arg(Preferences::useLDViewSingleCall ? "(Single Call)" : "");
          emit messageSig(LOG_INFO,message);
      }
    }

  if (Preferences::pageGuides)
      Preferences::setPageGuidesPreference(false);

  if (Preferences::pageRuler)
      Preferences::setPageRulerPreference(false);

  if (fadeSteps && fadeSteps != Preferences::enableFadeSteps) {
      Preferences::enableFadeSteps = fadeSteps;
      QString message = QString("Fade Previous Steps is ON.");
      emit messageSig(LOG_INFO,message);
      if (fadeStepsColour.isEmpty()) {
          if (Preferences::fadeStepsUseColour){
              Preferences::fadeStepsUseColour = false;
              QString message = QString("Use Global Fade Colour is OFF");
              emit messageSig(LOG_INFO,message);
            }
        }
    }

  if ((fadeStepsOpacity != Preferences::fadeStepsOpacity) &&
      Preferences::enableFadeSteps) {
          int previousOpacity = Preferences::fadeStepsOpacity;
          Preferences::fadeStepsOpacity = fadeStepsOpacity;
          QString message = QString("Fade Step Transparency changed from %1 to %2 percent")
              .arg(previousOpacity)
              .arg(Preferences::fadeStepsOpacity);
          emit messageSig(LOG_INFO,message);
    }

  if (!fadeStepsColour.isEmpty() && Preferences::enableFadeSteps) {
      if (!Preferences::fadeStepsUseColour){
          Preferences::fadeStepsUseColour = true;
          QString message = QString("Use Global Fade Colour is ON");
          emit messageSig(LOG_INFO,message);
          fadeStepsOpacity = 100;
          if (fadeStepsOpacity != Preferences::fadeStepsOpacity ) {
              int previousOpacity = Preferences::fadeStepsOpacity;
              Preferences::fadeStepsOpacity = fadeStepsOpacity;
              QString message = QString("Fade Step Transparency changed from %1 to %2 percent")
                  .arg(previousOpacity)
                  .arg(Preferences::fadeStepsOpacity);
              emit messageSig(LOG_INFO,message);
            }
        }
      if (LDrawColor::name(fadeStepsColour) != Preferences::fadeStepsColour) {
          QString previousColour = Preferences::fadeStepsColour;
          Preferences::fadeStepsColour = LDrawColor::name(fadeStepsColour);
          QString message = QString("Fade Step Colour preference changed from %1 to %2")
              .arg(previousColour.replace("_"," "))
              .arg(QString(Preferences::fadeStepsColour).replace("_"," "));
          emit messageSig(LOG_INFO,message);
        }
    }

  /* [Experimental] LDView Image Matting */
  if (imageMatting && Preferences::enableFadeSteps &&
      (Preferences::preferredRenderer == RENDERER_LDVIEW)) {
      Preferences::enableImageMatting = imageMatting;
//      Preferences::updateLDViewIniFile(UpdateExisting);          // strip AutoCrop [Disabled]
      QString message = QString("Enable Image matte is ON.");
      emit messageSig(LOG_INFO,message);
    } else {
      QString message;
      if (imageMatting && !Preferences::enableFadeSteps) {
          message = QString("Image matte requires fade previous steps to be on.");
          emit messageSig(LOG_ERROR,message);
        }

      if (imageMatting && (Preferences::preferredRenderer != RENDERER_LDVIEW)) {
          message = QString("Image matte requires LDView renderer.");
          emit messageSig(LOG_ERROR,message);
        }
      message = QString("Image matte flag will be ignored.");
      emit messageSig(LOG_ERROR,message);
    }

  if (highlightStep && highlightStep != Preferences::enableHighlightStep) {
      Preferences::enableHighlightStep = highlightStep;
      QString message = QString("Highlight Current Step is ON.");
      emit messageSig(LOG_INFO,message);
    }

  if ((highlightStepColour != Preferences::highlightStepColour) &&
      !highlightStepColour.isEmpty() &&
      Preferences::enableHighlightStep) {
      QString previousColour = Preferences::highlightStepColour;
      Preferences::highlightStepColour = highlightStepColour;
      QString message = QString("Highlight Step Colour preference changed from %1 to %2")
          .arg(previousColour)
          .arg(Preferences::highlightStepColour);
      emit messageSig(LOG_INFO,message);
    }

  if ((highlightLineWidth != Preferences::highlightStepLineWidth ) &&
      Preferences::enableHighlightStep) {
      float previousLineWidth = Preferences::highlightStepLineWidth;
      Preferences::highlightStepLineWidth = highlightLineWidth;
      QString message = QString("Highlight Line Width preference changed from %1 to %2")
          .arg(previousLineWidth)
          .arg(Preferences::highlightStepLineWidth);
      emit messageSig(LOG_INFO,message);
    }

  if (!commandlineFile.isEmpty())
    if (!loadFile(commandlineFile))
      return 1;

  if (processPageRange(pageRange)) {
      if (processFile){
          Preferences::pageDisplayPause = 1;
          continuousPageDialog(PAGE_NEXT);
        } else if (processExport) {
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
            exportAsPdfDialog();
        } else {
          continuousPageDialog(PAGE_NEXT);
        }
    } else
       return 1;

  return 0;
}
