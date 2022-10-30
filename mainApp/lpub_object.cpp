/****************************************************************************
**
** Copyright (C) 2018 - 2022 Trevor SANDY. All rights reserved.
**
** This file may be used under the terms of the
** GNU General Public Liceense (GPL) version 3.0
** which accompanies this distribution, and is
** available at http://www.gnu.org/licenses/gpl.html
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "lpub_object.h"

LPub *lpub;

LPub::LPub()
{
  lpub = this;
}

LPub::~LPub()
{
  lpub = nullptr;
}

/*********************************************
 *
 * Command line support calls
 *
 ********************************************/

int LPub::Process3DViewerCommandLine()
{
    return gApplication->Process3DViewerCommandLine();
}

int LPub::GetStudStyle()
{
    if (gApplication)
        return static_cast<int>(lcGetPiecesLibrary()->GetStudStyle());
    else
        return lcGetProfileInt(LC_PROFILE_STUD_STYLE);
}

bool LPub::GetAutomateEdgeColor()
{
    if (gApplication)
        return lcGetPreferences().mAutomateEdgeColor;
    else
        return lcGetProfileInt(LC_PROFILE_AUTOMATE_EDGE_COLOR);
}

quint32 LPub::GetStudCylinderColor()
{
    if (gApplication)
        return lcGetPreferences().mStudCylinderColor;
    else
        return lcGetProfileInt(LC_PROFILE_STUD_CYLINDER_COLOR);
}

quint32 LPub::GetPartEdgeColor()
{
    if (gApplication)
        return lcGetPreferences().mPartEdgeColor;
    else
        return lcGetProfileInt(LC_PROFILE_PART_EDGE_COLOR);
}

quint32 LPub::GetBlackEdgeColor()
{
    if (gApplication)
        return lcGetPreferences().mBlackEdgeColor;
    else
        return lcGetProfileInt(LC_PROFILE_BLACK_EDGE_COLOR);
}

quint32 LPub::GetDarkEdgeColor()
{
    if (gApplication)
        return lcGetPreferences().mDarkEdgeColor;
    else
        return lcGetProfileInt(LC_PROFILE_DARK_EDGE_COLOR);
}

float LPub::GetPartEdgeContrast()
{
    if (gApplication)
        return lcGetPreferences().mPartEdgeContrast;
    else
        return lcGetProfileFloat(LC_PROFILE_PART_EDGE_CONTRAST);
}

float LPub::GetPartColorLightDarkIndex()
{
    if (gApplication)
        return lcGetPreferences().mPartColorValueLDIndex;
    else
        return lcGetProfileFloat(LC_PROFILE_PART_COLOR_VALUE_LD_INDEX);
}

void LPub::SetStudStyle(const NativeOptions* Options, bool value)
{
    float  PartColorValueLDIndex = lcGetProfileFloat(LC_PROFILE_PART_COLOR_VALUE_LD_INDEX);
    quint32 StudCylinderColor = lcGetProfileInt(LC_PROFILE_STUD_CYLINDER_COLOR);
    quint32 PartEdgeColor = lcGetProfileInt(LC_PROFILE_PART_EDGE_COLOR);
    quint32 BlackEdgeColor = lcGetProfileInt(LC_PROFILE_BLACK_EDGE_COLOR);
    quint32 DarkEdgeColor = lcGetProfileInt(LC_PROFILE_DARK_EDGE_COLOR);
    int StudStyle = lcGetProfileInt(LC_PROFILE_STUD_STYLE);

    if (Options) {
         PartColorValueLDIndex = Options->LightDarkIndex;
         StudCylinderColor = Options->StudCylinderColor;
         PartEdgeColor = Options->PartEdgeColor;
         BlackEdgeColor = Options->BlackEdgeColor;
         DarkEdgeColor = Options->DarkEdgeColor;
         StudStyle = Options->StudStyle;
    }

    bool ColorChanged  = GetStudStyle() != StudStyle;
         ColorChanged |= lcGetPreferences().mPartColorValueLDIndex != PartColorValueLDIndex;
         ColorChanged |= lcGetPreferences().mStudCylinderColor != StudCylinderColor;
         ColorChanged |= lcGetPreferences().mPartEdgeColor != PartEdgeColor;
         ColorChanged |= lcGetPreferences().mBlackEdgeColor != BlackEdgeColor;
         ColorChanged |= lcGetPreferences().mDarkEdgeColor != DarkEdgeColor;

    gApplication->mPreferences.mPartColorValueLDIndex = PartColorValueLDIndex;
    gApplication->mPreferences.mStudCylinderColor = StudCylinderColor;
    gApplication->mPreferences.mPartEdgeColor = PartEdgeColor;
    gApplication->mPreferences.mBlackEdgeColor = BlackEdgeColor;
    gApplication->mPreferences.mDarkEdgeColor = DarkEdgeColor;

    lcGetPiecesLibrary()->SetStudStyle(static_cast<lcStudStyle>(StudStyle), value);

    if (ColorChanged)
        lcGetPiecesLibrary()->LoadColors();
}

void LPub::SetAutomateEdgeColor(const NativeOptions* Options)
{
    bool  AutomateEdgeColor = lcGetProfileInt(LC_PROFILE_AUTOMATE_EDGE_COLOR);
    float PartEdgeContrast = lcGetProfileFloat(LC_PROFILE_PART_EDGE_CONTRAST);
    float PartColorValueLDIndex = lcGetProfileFloat(LC_PROFILE_PART_COLOR_VALUE_LD_INDEX);

    if (Options) {
         AutomateEdgeColor = Options->AutoEdgeColor;
         PartEdgeContrast = Options->EdgeContrast;
         PartColorValueLDIndex = Options->EdgeSaturation;
    }

    bool ColorChanged  = lcGetPreferences().mAutomateEdgeColor != AutomateEdgeColor;
         ColorChanged |= lcGetPreferences().mPartEdgeContrast != PartEdgeContrast;
         ColorChanged |= lcGetPreferences().mPartColorValueLDIndex != PartColorValueLDIndex;

    gApplication->mPreferences.mAutomateEdgeColor = AutomateEdgeColor;
    gApplication->mPreferences.mPartEdgeContrast = PartEdgeContrast;
    gApplication->mPreferences.mPartColorValueLDIndex = PartColorValueLDIndex;

    if (ColorChanged)
        lcGetPiecesLibrary()->LoadColors();
}

bool LPub::OpenProject(const NativeOptions* Options, int Type/*NATIVE_VIEW*/, bool UseFile/*false*/)
{
    bool Loaded = false;

    Project* Loader = nullptr;

    if (Type != NATIVE_VIEW) // NATIVE_IMAGE or NATIVE_EXPORT
    {
        if (Type == NATIVE_IMAGE)
            Loader = new Project(false/*IsPreview*/, NATIVE_IMAGE);
        else
            Loader = new Project();

        QString FileName, StepKey;
        if (UseFile && !Options->InputFileName.isEmpty())
           FileName = Options->InputFileName;
        else
           StepKey = Options->ViewerStepKey;

        Loaded = Loader->Load(FileName, StepKey, Options->ImageType, false/*ShowErrors*/);
        if (Loaded)
        {
            gApplication->SetProject(Loader);
            lcView::UpdateProjectViews(Loader);
        }
        else
            delete Loader;
    }
    else if (gMainWindow) // NATIVE_VIEW
    {
        if (UseFile && !Options->InputFileName.isEmpty()) {
            Loaded = gMainWindow->OpenProject(Options->InputFileName);
        } else {
            Loader = new Project();
            Loaded = Loader->Load(QString()/*FileName*/, Options->ViewerStepKey, Options->ImageType, true/*ShowErrors*/);
            if (Loaded)
            {
                gApplication->SetProject(Loader);
                lcView::UpdateProjectViews(Loader);
            }
            else
                delete Loader;
        }
    }

    if (Loaded && Type != NATIVE_EXPORT)
        return Render::RenderNativeView(Options, Type == NATIVE_IMAGE);

    return Loaded;
}
/********************************************************************
 *
 *
 *
 * *****************************************************************/

bool Application::setFadeStepsFromCommand()
{
  QString result;
  Where topLevelModel(ldrawFile.topLevelFile(),0);
  QRegExp fadeRx = QRegExp("FADE_STEP ENABLED\\s*(GLOBAL)?\\s*TRUE");
  if (!Preferences::enableFadeSteps) {
    Preferences::enableFadeSteps = Gui::stepContains(topLevelModel,fadeRx,result,1);
    if (Preferences::enableFadeSteps && result != "GLOBAL") {
      emit gui->messageSig(LOG_ERROR,QString("Top level FADE_STEP ENABLED meta command must be GLOBAL"));
      Preferences::enableFadeSteps = false;
    }
  }

  bool setupFadeSteps = false;
  if (!Preferences::enableFadeSteps) {
    result.clear();
    fadeRx.setPattern("FADE_STEP SETUP\\s*(GLOBAL)?\\s*TRUE");
    setupFadeSteps = Gui::stepContains(topLevelModel,fadeRx,result,1);
    if (setupFadeSteps && result != "GLOBAL") {
      emit gui->messageSig(LOG_ERROR,QString("Top level FADE_STEP SETUP meta command must be GLOBAL"));
      setupFadeSteps = false;
    }
  }

  if (Preferences::enableFadeSteps != Preferences::initEnableFadeSteps)
    emit gui->messageSig(LOG_INFO_STATUS,QString("Fade Previous Steps is %1 - Set from meta command.")
                                            .arg(Preferences::enableFadeSteps ? "ON" : "OFF"));
  if (setupFadeSteps)
     emit gui->messageSig(LOG_INFO_STATUS,QString("Fade Previous Steps Setup is ENABLED."));

  if (!Preferences::enableFadeSteps && !setupFadeSteps)
    return false;

  result.clear();
  fadeRx.setPattern("FADE_STEP OPACITY\\s*(?:GLOBAL)?\\s*(\\d+)");
  Gui::stepContains(topLevelModel,fadeRx,result,1);
  if (!result.isEmpty()) {
    bool ok = result.toInt(&ok);
    int fadeStepsOpacityCompare = Preferences::fadeStepsOpacity;
    Preferences::fadeStepsOpacity = ok ? result.toInt() : FADE_OPACITY_DEFAULT;
    bool fadeStepsOpacityChanged = Preferences::fadeStepsOpacity != fadeStepsOpacityCompare;
    if (fadeStepsOpacityChanged)
      emit gui->messageSig(LOG_INFO,QString("Fade Step Transparency changed from %1 to %2 percent - Set from meta command")
                                       .arg(fadeStepsOpacityCompare)
                                       .arg(Preferences::fadeStepsOpacity));
  }

  result.clear();
  fadeRx.setPattern("FADE_STEP COLOR\\s*(?:GLOBAL)?\\s*\"(\\w+)\"");
  Gui::stepContains(topLevelModel,fadeRx,result,1);
  if (!result.isEmpty()) {
    QColor ParsedColor = LDrawColor::color(result);
    bool fadeStepsUseColorCompare = Preferences::fadeStepsUseColour;
    Preferences::fadeStepsUseColour = ParsedColor.isValid();
    if (Preferences::fadeStepsUseColour != fadeStepsUseColorCompare)
      emit gui->messageSig(LOG_INFO,QString("Use Fade Color is %1 - Set from meta command")
                                       .arg(Preferences::fadeStepsUseColour ? "ON" : "OFF"));
    QString fadeStepsColourCompare = Preferences::validFadeStepsColour;
    Preferences::validFadeStepsColour = ParsedColor.isValid() ? result : Preferences::validFadeStepsColour;
    if (QString(Preferences::validFadeStepsColour).toLower() != fadeStepsColourCompare.toLower())
      emit gui->messageSig(LOG_INFO,QString("Fade Step Color preference changed from %1 to %2 - Set from meta command")
                                       .arg(fadeStepsColourCompare.replace("_"," "))
                                       .arg(QString(Preferences::validFadeStepsColour).replace("_"," ")));
  }

  return setupFadeSteps;
}

bool Application::setHighlightStepFromCommand()
{
  QString result;
  Where topLevelModel(gui->topLevelFile(),0);
  QRegExp highlightRx = QRegExp("HIGHLIGHT_STEP ENABLED\\s*(GLOBAL)?\\s*TRUE");
  if (!Preferences::enableHighlightStep) {
    Preferences::enableHighlightStep = Gui::stepContains(topLevelModel,highlightRx,result,1);
    if (Preferences::enableHighlightStep && result != "GLOBAL") {
      emit gui->messageSig(LOG_ERROR,QString("Top level HIGHLIGHT_STEP ENABLED meta command must be GLOBAL"));
      Preferences::enableHighlightStep = false;
    }
  }

  bool setupHighlightStep = false;
  if (!Preferences::enableHighlightStep) {
    result.clear();
    highlightRx.setPattern("HIGHLIGHT_STEP SETUP\\s*(GLOBAL)?\\s*TRUE");
    setupHighlightStep = Gui::stepContains(topLevelModel,highlightRx,result,1);
    if (setupHighlightStep && result != "GLOBAL") {
      emit gui->messageSig(LOG_ERROR,QString("Top level HIGHLIGHT_STEP SETUP meta command must be GLOBAL"));
      setupHighlightStep = false;
    }
  }

  if (Preferences::enableHighlightStep != Preferences::initEnableHighlightStep)
    emit gui->messageSig(LOG_INFO_STATUS,QString("Highlight Current Step is %1 - Set from meta command.")
                                            .arg(Preferences::enableHighlightStep ? "ON" : "OFF"));
  if (setupHighlightStep)
     emit gui->messageSig(LOG_INFO_STATUS,QString("Highlight Current Step Setup is ENABLED."));

  if (!Preferences::enableHighlightStep && !setupHighlightStep)
    return false;

  result.clear();
  highlightRx.setPattern("HIGHLIGHT_STEP COLOR\\s*(?:GLOBAL)?\\s*\"(0x|#)([\\da-fA-F]+)\"");
  if (Gui::stepContains(topLevelModel,highlightRx,result)) {
    result = QString("%1%2").arg(highlightRx.cap(1),highlightRx.cap(2));
    QColor ParsedColor = QColor(result);
    QString highlightStepColourCompare = Preferences::highlightStepColour;
    Preferences::highlightStepColour = ParsedColor.isValid() ? result : Preferences::validFadeStepsColour;
    bool highlightStepColorChanged = QString(Preferences::highlightStepColour).toLower() != highlightStepColourCompare.toLower();
    if (highlightStepColorChanged)
      emit gui->messageSig(LOG_INFO,QString("Highlight Step Color preference changed from %1 to %2 - Set from meta command")
                                  .arg(highlightStepColourCompare)
                                  .arg(Preferences::highlightStepColour));
  }

  return setupHighlightStep;
}

bool Application::setPreferredRendererFromCommand(const QString &preferredRenderer)
{
  if (preferredRenderer.isEmpty())
      return false;

  int renderer = Preferences::preferredRenderer;
  bool useLDVSingleCall = false;
  bool useLDVSnapShotList = false;
  bool useNativeGenerator = true;
  bool rendererChanged = false;
  bool renderFlagChanged = false;

  QString message;
  QString command = preferredRenderer.toLower();
  if (command == "native") {
    renderer = RENDERER_NATIVE;
  } else if (command == "ldview") {
    renderer = RENDERER_LDVIEW;
  } else if ((useLDVSingleCall = command == "ldview-sc")) {
    renderer = RENDERER_LDVIEW;
  } else if ((useLDVSnapShotList = command == "ldview-scsl")) {
    renderer = RENDERER_LDVIEW;
  } else if (command == "ldglite") {
    renderer = RENDERER_LDGLITE;
  } else if (command == "povray") {
    renderer = RENDERER_POVRAY;
  } else if (command == "povray-ldv") {
    renderer = RENDERER_POVRAY;
    useNativeGenerator = false;
  } else {
    emit gui->messageSig(LOG_ERROR,QString("Invalid renderer console command option specified: '%1'.").arg(renderer));
    return rendererChanged;
  }

  if (renderer == RENDERER_LDVIEW) {
    if (Preferences::enableLDViewSingleCall != useLDVSingleCall) {
      message = QString("Renderer preference use LDView Single Call changed from %1 to %2.")
                        .arg(Preferences::enableLDViewSingleCall ? "Yes" : "No")
                        .arg(useLDVSingleCall ? "Yes" : "No");
      emit gui->messageSig(LOG_INFO,message);
      Preferences::enableLDViewSingleCall = useLDVSingleCall;
      renderFlagChanged = true;
    }
    if (Preferences::enableLDViewSnaphsotList != useLDVSnapShotList) {
      message = QString("Renderer preference use LDView Snapshot List changed from %1 to %2.")
                        .arg(Preferences::enableLDViewSnaphsotList ? "Yes" : "No")
                        .arg(useLDVSingleCall ? "Yes" : "No");
      emit gui->messageSig(LOG_INFO,message);
      Preferences::enableLDViewSnaphsotList = useLDVSnapShotList;
      if (useLDVSnapShotList)
          Preferences::enableLDViewSingleCall = true;
      if (!renderFlagChanged)
        renderFlagChanged = true;
    }
  } else if (renderer == RENDERER_POVRAY) {
    if (Preferences::useNativePovGenerator != useNativeGenerator) {
      message = QString("Renderer preference POV file generator changed from %1 to %2.")
                        .arg(Preferences::useNativePovGenerator ? rendererNames[RENDERER_NATIVE] : rendererNames[RENDERER_LDVIEW])
                        .arg(useNativeGenerator ? rendererNames[RENDERER_NATIVE] : rendererNames[RENDERER_LDVIEW]);
      emit gui->messageSig(LOG_INFO,message);
      Preferences::useNativePovGenerator = useNativeGenerator;
      if (!renderFlagChanged)
        renderFlagChanged = true;
    }
  }

  rendererChanged = renderer != Preferences::preferredRenderer;

  if (rendererChanged) {
    Preferences::preferredRenderer = renderer;
    Render::setRenderer(Preferences::preferredRenderer);
    Preferences::preferredRendererPreferences(true/*global*/);
    Preferences::updatePOVRayConfigFiles();
  }

  if (rendererChanged || renderFlagChanged) {
    message = QString("Renderer preference changed from %1 to %2%3.")
                      .arg(rendererNames[Preferences::preferredRenderer])
                      .arg(rendererNames[renderer])
                      .arg(renderer == RENDERER_POVRAY ? QString(" (POV file generator is %1)")
                                                                 .arg(Preferences::useNativePovGenerator ? rendererNames[RENDERER_NATIVE] : rendererNames[RENDERER_LDVIEW]) :
                           renderer == RENDERER_LDVIEW ? useLDVSingleCall ?
                                                         useLDVSnapShotList ? QString(" (Single Call using Export File List)") :
                                                                              QString(" (Single Call)") :
                                                                              QString() : QString());
    emit gui->messageSig(LOG_INFO,message);
  }

  return rendererChanged;
}

/********************************************************************
 *
 *
 *
 * *****************************************************************/

// flip orientation for landscape
int LPub::pageSize(PageMeta &meta, int which){
  int _which;
  if (meta.orientation.value() == Landscape){
      which == 0 ? _which = 1 : _which = 0;
    } else {
      _which = which;
    }
  return meta.size.valuePixels(_which);
}

/*********************************************
 *
 * split viewer step keys
 *
 ********************************************/

QStringList LPub::getViewerStepKeys(bool modelName, bool pliPart, const QString &key)
{
    // viewerStepKey - 3 elements:
    // CSI: 0=modelNameIndex, 1=lineNumber,   2=stepNumber [_dm (displayModel)]
    // SMP: 0=modelNameIndex, 1=lineNumber,   2=stepNumber [_Preview (Submodel Preview)]
    // PLI: 0=partNameString, 1=colourNumber, 2=stepNumber
    QStringList keys = key.isEmpty() ? viewerStepKey.split(";") : key.split(";");
    // confirm keys has at least 3 elements
    if (keys.size() < 3) {
#ifdef QT_DEBUG_MODE
        emit gui->messageSig(LOG_DEBUG, QString("Parse stepKey [%1] failed").arg(viewerStepKey));
#endif
        return QStringList();
    } else if (keys.at(2).count("_")) {
        QStringList displayStepKeys = keys.at(2).split("_");
        keys.removeLast();
        keys.append(displayStepKeys);
    }

    if (!pliPart) {
        bool ok;
        int modelNameIndex = keys[0].toInt(&ok);
        if (!ok) {
#ifdef QT_DEBUG_MODE
            emit gui->messageSig(LOG_DEBUG, QString("Parse stepKey failed. Expected model name index integer got [%1]").arg(keys[0]));
#endif
            return QStringList();
        }

        if (modelName)
            keys.replace(0,ldrawFile.getSubmodelName(modelNameIndex));
    }

    return keys;
}

/****************************************************************************
 *
 * Download with progress monotor
 *
 ***************************************************************************/

void LPub::downloadFile(QString URL, QString title, bool promptRedirect)
{
    mTitle = title;
    mPromptRedirect = promptRedirect;
    mProgressDialog = new QProgressDialog(nullptr);
    connect(mProgressDialog, SIGNAL(canceled()), this, SLOT(cancelDownload()));

    mProgressDialog->setWindowTitle(tr("Downloading"));
    mProgressDialog->setLabelText(tr("Downloading %1").arg(mTitle));
    mProgressDialog->show();

    mHttpManager   = new QNetworkAccessManager(this);

    mUrl = URL;

    mHttpRequestAborted = false;

    startRequest(mUrl);

    while (mHttpReply)
        QLPub::processEvents();
}

void LPub::updateDownloadProgress(qint64 bytesRead, qint64 totalBytes)
{
    if (mHttpRequestAborted)
        return;

    mProgressDialog->setMaximum(int(totalBytes));
    mProgressDialog->setValue(int(bytesRead));
}

void LPub::startRequest(QUrl url)
{
    QNetworkRequest request(url);
#if (QT_VERSION >= QT_VERSION_CHECK(5, 6, 0))
    if (!mPromptRedirect)
        request.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
#endif
    mHttpReply = mHttpManager->get(request);

    connect(mHttpReply, SIGNAL(downloadProgress(qint64,qint64)),
            this, SLOT(updateDownloadProgress(qint64,qint64)));

    connect(mHttpReply, SIGNAL(finished()),
            this, SLOT(httpDownloadFinished()));
}

void LPub::cancelDownload()
{
    mHttpRequestAborted = true;
    mHttpReply->abort();
}

void LPub::httpDownloadFinished()
{
    if (mHttpRequestAborted) {
        mByteArray.clear();
        mHttpReply->deleteLater();
        mHttpReply = nullptr;
        mProgressDialog->close();
        return;
    }

    QString message;
    QVariant redirectionTarget = mHttpReply->attribute(QNetworkRequest::RedirectionTargetAttribute);
    if (mHttpReply->error()) {
        mByteArray.clear();
        message = QString("%1 Download failed: %2.")
                          .arg(mTitle).arg(mHttpReply->errorString());
        if (Preferences::modeGUI){
            QMessageBox::warning(nullptr,QMessageBox::tr("LPub3D"),  message);
        } else {
            logError() << message;
        }
    } else if (!redirectionTarget.isNull()) {
        // This block should only trigger for redirects when Qt
        // is less than 5.6.0 or if prompt redirect set to True
        QUrl newUrl = mUrl.resolved(redirectionTarget.toUrl());
        bool proceedToRedirect = true;
        if (mPromptRedirect && Preferences::modeGUI) {
            proceedToRedirect = QMessageBox::question(nullptr, tr("HTTP"),
                                              tr("Download redirect to %1 ?").arg(newUrl.toString()),
                                              QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes;
        } else {
            logNotice() << QString("Download redirect to %1 ?").arg(newUrl.toString());
        }
        if (proceedToRedirect) {
            mUrl = newUrl;
            mHttpReply->deleteLater();
            mByteArray.resize(0);
            startRequest(mUrl);
            return;
        }
    } else {
        mByteArray = mHttpReply->readAll();
    }

    mProgressDialog->close();

    mHttpReply->deleteLater();
    mHttpReply = nullptr;
    mHttpManager = nullptr;
}

QString LPub::elapsedTime(const qint64 &duration) {

    qint64 elapsed = duration;
    int milliseconds = int(elapsed % 1000);
    elapsed /= 1000;
    int seconds = int(elapsed % 60);
    elapsed /= 60;
    int minutes = int(elapsed % 60);
    elapsed /= 60;
    int hours = int(elapsed % 24);

    return QString("Elapsed time: %1%2%3")
                   .arg(hours   >   0 ?
                                  QString("%1 %2 ")
                                          .arg(hours)
                                          .arg(hours   > 1 ? "hours"   : "hour") :
                                  QString())
                   .arg(minutes > 0 ?
                                  QString("%1 %2 ")
                                          .arg(minutes)
                                          .arg(minutes > 1 ? "minutes" : "minute") :
                                  QString())
                   .arg(QString("%1.%2 %3")
                                .arg(seconds)
                                .arg(milliseconds,3,10,QLatin1Char('0'))
                                .arg(seconds > 1 ? "seconds" : "second"));

}
