/****************************************************************************
**
** Copyright (C) 2019 - 2020 Trevor SANDY. All rights reserved.
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

/****************************************************************************
 *
 * This class encapsulates the native renderer routines.
 *
 * Please see lpub.h for an overall description of how the files in LPub
 * make up the LPub program.
 *
 ***************************************************************************/

#include "lpub.h"
#include "lpub_preferences.h"
#include "step.h"

#include "camera.h"
#include "project.h"
#include "view.h"
#include "piece.h"
#include "lc_profile.h"
#include "application.h"
#include "lc_partselectionwidget.h"

void Gui::create3DActions()
{
    applyCameraAct = new QAction(QIcon(":/resources/applycamerasettings.png"),tr("&Apply Camera Settings"),this);
    applyCameraAct->setShortcut(tr("Alt+0"));
    applyCameraAct->setStatusTip(tr("Apply camera settings to this step - Alt+0"));
    connect(applyCameraAct, SIGNAL(triggered()), this, SLOT(applyCameraSettings()));

    setTargetPositionAct = new QAction(tr("Set Camera Target"),this);
    setTargetPositionAct->setStatusTip(tr("Set the camera target position for this assembly"));
    setTargetPositionAct->setCheckable(true);
    setTargetPositionAct->setChecked(lcGetProfileInt(LC_PROFILE_SET_TARGET_POSITION));
    connect(setTargetPositionAct, SIGNAL(triggered()), this, SLOT(setTargetPosition()));

    useImageSizeAct = new QAction(tr("Custom Image Size"),this);
    useImageSizeAct->setStatusTip(tr("Maintain initial image width and height or enter custom image size"));
    useImageSizeAct->setCheckable(true);
    useImageSizeAct->setChecked(lcGetProfileInt(LC_PROFILE_USE_IMAGE_SIZE));
    connect(useImageSizeAct, SIGNAL(triggered()), this, SLOT(useImageSize()));

    defaultCameraPropertiesAct = new QAction(tr("Display Properties"),this);
    defaultCameraPropertiesAct->setStatusTip(tr("Display camera propeties in Properties window"));
    defaultCameraPropertiesAct->setCheckable(true);
    defaultCameraPropertiesAct->setChecked(lcGetPreferences().mDefaultCameraProperties);
    connect(defaultCameraPropertiesAct, SIGNAL(triggered()), this, SLOT(showDefaultCameraProperties()));

    viewerZoomSliderAct = new QWidgetAction(nullptr);
    viewerZoomSliderWidget = new QSlider();
    viewerZoomSliderWidget->setSingleStep(1);
    viewerZoomSliderWidget->setTickInterval(10);
    viewerZoomSliderWidget->setTickPosition(QSlider::TicksBelow);
    viewerZoomSliderWidget->setMaximum(150);
    viewerZoomSliderWidget->setMinimum(1);
    viewerZoomSliderWidget->setValue(50);
    connect(viewerZoomSliderWidget, SIGNAL(valueChanged(int)), this, SLOT(ViewerZoomSlider(int)));
}

void Gui::create3DMenus()
{
     /*
      * Not used
     FileMenuViewer = menuBar()->addMenu(tr("&Step"));
     FileMenuViewer->addAction(gMainWindow->mActions[LC_FILE_SAVEAS]);
     */

     gMainWindow->mActions[LC_FILE_SAVE_IMAGE]->setIcon(QIcon(":/resources/saveimage.png"));
     gMainWindow->mActions[LC_FILE_SAVE_IMAGE]->setShortcut(tr("Alt+0"));
     gMainWindow->mActions[LC_FILE_SAVE_IMAGE]->setStatusTip(tr("Save an image of the current view - Alt+0"));

     gMainWindow->mActions[LC_FILE_EXPORT_3DS]->setIcon(QIcon(":/resources/3ds32.png"));
     gMainWindow->mActions[LC_FILE_EXPORT_COLLADA]->setIcon(QIcon(":/resources/dae32.png"));
     gMainWindow->mActions[LC_FILE_EXPORT_WAVEFRONT]->setIcon(QIcon(":/resources/obj32.png"));

     ViewerExportMenu = new QMenu(tr("&Export As..."), this);
     ViewerExportMenu->setIcon(QIcon(":/resources/exportas.png"));
     ViewerExportMenu->addAction(gMainWindow->mActions[LC_FILE_EXPORT_3DS]);
     ViewerExportMenu->addAction(gMainWindow->mActions[LC_FILE_EXPORT_COLLADA]);
     ViewerExportMenu->addAction(gMainWindow->mActions[LC_FILE_EXPORT_WAVEFRONT]);

     ViewerMenu = menuBar()->addMenu(tr("&3DViewer"));
     ViewerMenu->addAction(povrayRenderAct);
     ViewerMenu->addSeparator();
     ViewerMenu->addAction(gMainWindow->mActions[LC_FILE_SAVE_IMAGE]);
     ViewerMenu->addMenu(ViewerExportMenu);
     ViewerMenu->addSeparator();
     ViewerMenu->addAction(gMainWindow->mActions[LC_PIECE_EDIT_SELECTED_SUBMODEL]);
     ViewerMenu->addAction(gMainWindow->mActions[LC_PIECE_EDIT_END_SUBMODEL]);
     ViewerMenu->addSeparator();
     ViewerMenu->addAction(gMainWindow->mActions[LC_VIEW_ZOOM_EXTENTS]);
     ViewerMenu->addAction(gMainWindow->mActions[LC_VIEW_LOOK_AT]);
     gMainWindow->mActions[LC_VIEW_LOOK_AT]->setIcon(QIcon(":/resources/lookat.png"));

     ViewerMenu->addMenu(gMainWindow->GetCameraMenu());
     gMainWindow->GetToolsMenu()->addAction(applyCameraAct);
     ViewerMenu->addMenu(gMainWindow->GetToolsMenu());
     ViewerMenu->addMenu(gMainWindow->GetViewpointMenu());
     gMainWindow->mActions[LC_VIEW_VIEWPOINT_FRONT]->setIcon(QIcon(":/resources/front.png"));
     gMainWindow->mActions[LC_VIEW_VIEWPOINT_BACK]->setIcon(QIcon(":/resources/back.png"));
     gMainWindow->mActions[LC_VIEW_VIEWPOINT_LEFT]->setIcon(QIcon(":/resources/left.png"));
     gMainWindow->mActions[LC_VIEW_VIEWPOINT_RIGHT]->setIcon(QIcon(":/resources/right.png"));
     gMainWindow->mActions[LC_VIEW_VIEWPOINT_TOP]->setIcon(QIcon(":/resources/top.png"));
     gMainWindow->mActions[LC_VIEW_VIEWPOINT_BOTTOM]->setIcon(QIcon(":/resources/bottom.png"));

     ViewerMenu->addMenu(gMainWindow->GetProjectionMenu());
     ViewerMenu->addMenu(gMainWindow->GetShadingMenu());

     ViewerMenu->addSeparator();
     ViewerMenu->addAction(gMainWindow->mActions[LC_VIEW_SPLIT_HORIZONTAL]);
     ViewerMenu->addAction(gMainWindow->mActions[LC_VIEW_SPLIT_VERTICAL]);
     ViewerMenu->addAction(gMainWindow->mActions[LC_VIEW_REMOVE_VIEW]);
     ViewerMenu->addAction(gMainWindow->mActions[LC_VIEW_RESET_VIEWS]);

     ViewerMenu->addSeparator();
     ViewerMenu->addAction(gMainWindow->mActions[LC_VIEW_PREFERENCES]);
     ViewerMenu->addSeparator();

     connect(gMainWindow->mActions[LC_VIEW_VIEWPOINT_HOME], SIGNAL(triggered()), this, SLOT(ResetViewerZoomSlider()));

     viewerZoomSliderAct->setDefaultWidget(viewerZoomSliderWidget);
     ViewerZoomSliderMenu = new QMenu(tr("Zoom Slider"),this);
     ViewerZoomSliderMenu->addAction(viewerZoomSliderAct);
     gMainWindow->mActions[LC_EDIT_ACTION_ZOOM]->setMenu(ViewerZoomSliderMenu);

     ((QToolButton*)gMainWindow->mToolsToolBar->widgetForAction(gMainWindow->mActions[LC_EDIT_ACTION_ZOOM]))->setPopupMode(QToolButton::InstantPopup);

     ViewerMenu->addAction(gMainWindow->mToolsToolBar->toggleViewAction());
     ViewerMenu->addSeparator();
     ViewerMenu->addAction(gMainWindow->mActions[LC_HELP_ABOUT]);

     cameraMenu = new QMenu(tr("Camera Settings"),this);
     cameraMenu->addAction(defaultCameraPropertiesAct);
     cameraMenu->addSeparator();
     cameraMenu->addAction(setTargetPositionAct);
     cameraMenu->addAction(useImageSizeAct);
     applyCameraAct->setMenu(cameraMenu);
}

void Gui::create3DToolBars()
{
    exportToolBar->addSeparator();
    exportToolBar->addAction(povrayRenderAct);
    exportToolBar->addSeparator();
    exportToolBar->addAction(gMainWindow->mActions[LC_FILE_SAVE_IMAGE]);
    exportToolBar->addAction(gMainWindow->mActions[LC_FILE_EXPORT_3DS]);
    exportToolBar->addAction(gMainWindow->mActions[LC_FILE_EXPORT_COLLADA]);
    exportToolBar->addAction(gMainWindow->mActions[LC_FILE_EXPORT_WAVEFRONT]);

    gMainWindow->mToolsToolBar->addAction(applyCameraAct);

    gMainWindow->mPartsToolBar->setWindowTitle("Tools Toolbar");
}

void Gui::Disable3DActions()
{
    applyCameraAct->setEnabled(false);
}

void Gui::Enable3DActions()
{
    applyCameraAct->setEnabled(true);
}

void Gui::create3DDockWindows()
{
    viewerDockWindow = new QDockWidget(trUtf8("3DViewer"), this);
    viewerDockWindow->setObjectName("ModelDockWindow");
    viewerDockWindow->setAllowedAreas(
                Qt::TopDockWidgetArea  | Qt::BottomDockWidgetArea |
                Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    viewerDockWindow->setWidget(gMainWindow);
    addDockWidget(Qt::RightDockWidgetArea, viewerDockWindow);
    viewMenu->addAction(viewerDockWindow->toggleViewAction());

    tabifyDockWidget(viewerDockWindow, fileEditDockWindow);

    //timeline
    gMainWindow->mTimelineToolBar->setWindowTitle(trUtf8("Timeline"));
    gMainWindow->mTimelineToolBar->setObjectName("TimelineToolbar");
    gMainWindow->mTimelineToolBar->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    gMainWindow->mTimelineToolBar->setAcceptDrops(true);
    addDockWidget(Qt::RightDockWidgetArea, gMainWindow->mTimelineToolBar);
    viewMenu->addAction(gMainWindow->mTimelineToolBar->toggleViewAction());

    tabifyDockWidget(viewerDockWindow, gMainWindow->mTimelineToolBar);

    //Properties
    gMainWindow->mPropertiesToolBar->setWindowTitle(trUtf8("Properties"));
    gMainWindow->mPropertiesToolBar->setObjectName("PropertiesToolbar");
    gMainWindow->mPropertiesToolBar->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    addDockWidget(Qt::RightDockWidgetArea, gMainWindow->mPropertiesToolBar);
    viewMenu->addAction(gMainWindow->mPropertiesToolBar->toggleViewAction());

    tabifyDockWidget(viewerDockWindow/*gMainWindow->mTimelineToolBar*/, gMainWindow->mPropertiesToolBar);

    //Part Selection
    QSettings Settings;
    bool viewable = false;
    if (Settings.contains(QString("%1/%2").arg(SETTINGS,VIEW_PARTS_WIDGET_KEY)))
        viewable = Settings.value(QString("%1/%2").arg(SETTINGS,VIEW_PARTS_WIDGET_KEY)).toBool();
    gMainWindow->mPartsToolBar->toggleViewAction()->setChecked(viewable);
    gMainWindow->mPartsToolBar->setWindowTitle(trUtf8("Parts"));
    gMainWindow->mPartsToolBar->setObjectName("PartsToolbar");
    gMainWindow->mPartsToolBar->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    addDockWidget(Qt::RightDockWidgetArea, gMainWindow->mPartsToolBar);
    viewMenu->addAction(gMainWindow->mPartsToolBar->toggleViewAction());
    connect (gMainWindow->mPartsToolBar, SIGNAL (visibilityChanged(bool)),
                                   this, SLOT (partsWidgetVisibilityChanged(bool)));

    tabifyDockWidget(viewerDockWindow/*gMainWindow->mPropertiesToolBar*/, gMainWindow->mPartsToolBar);

    //Colors Selection
    viewable = false;
    if (Settings.contains(QString("%1/%2").arg(SETTINGS,VIEW_COLORS_WIDGET_KEY)))
        viewable = Settings.value(QString("%1/%2").arg(SETTINGS,VIEW_COLORS_WIDGET_KEY)).toBool();
    gMainWindow->mColorsToolBar->toggleViewAction()->setChecked(viewable);
    gMainWindow->mColorsToolBar->setWindowTitle(trUtf8("Colors"));
    gMainWindow->mColorsToolBar->setObjectName("ColorsToolbar");
    gMainWindow->mColorsToolBar->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    addDockWidget(Qt::RightDockWidgetArea, gMainWindow->mColorsToolBar);
    viewMenu->addAction(gMainWindow->mColorsToolBar->toggleViewAction());
    connect (gMainWindow->mColorsToolBar, SIGNAL (visibilityChanged(bool)),
                                    this, SLOT (coloursWidgetVisibilityChanged(bool)));

    tabifyDockWidget(viewerDockWindow/*gMainWindow->mPartsToolBar*/, gMainWindow->mColorsToolBar);

    connect(viewerDockWindow, SIGNAL (topLevelChanged(bool)), this, SLOT (toggleLCStatusBar(bool)));
}

void Gui::partsWidgetVisibilityChanged(bool visible)
{
    QSettings Settings;
    Settings.setValue(QString("%1/%2").arg(SETTINGS,VIEW_PARTS_WIDGET_KEY),visible);
}

void Gui::coloursWidgetVisibilityChanged(bool visible)
{
    QSettings Settings;
    Settings.setValue(QString("%1/%2").arg(SETTINGS,VIEW_COLORS_WIDGET_KEY),visible);
}

void Gui::showLCStatusMessage()
{

    if(!viewerDockWindow->isFloating())
    statusBarMsg(gMainWindow->mLCStatusBar->currentMessage());
}

void Gui::toggleLCStatusBar(bool topLevel)
{

    Q_UNUSED(topLevel);

    if(viewerDockWindow->isFloating())
        gMainWindow->statusBar()->show();
    else
        gMainWindow->statusBar()->hide();
}

void Gui::ViewerZoomSlider(int value)
{
    float z = value;
    if (value > mViewerZoomLevel)
        z = 10.0f;
    else
        z = -10.0f;
    gMainWindow->GetActiveView()->Zoom(z);
    mViewerZoomLevel = value;
}

void Gui::ResetViewerZoomSlider()
{
   viewerZoomSliderWidget->setValue(50);
}

void Gui::applyCameraSettings()
{
    SettingsMeta cameraMeta;

    View* ActiveView = gMainWindow->GetActiveView();

    lcCamera* Camera = ActiveView->mCamera;

    auto validCameraValue = [&cameraMeta, &Camera] (const CamFlag flag)
    {
        if (Preferences::usingNativeRenderer)
            return qRound(flag == DefFoV ?
                               Camera->m_fovy :
                          flag == DefZNear ?
                               Camera->m_zNear : Camera->m_zFar);

        float result;
        switch (flag)
        {
        case DefFoV:
            // e.g.            30.0  +                 0.01         - 30.0
            result = Camera->m_fovy  + cameraMeta.cameraFoV.value() - CAMERA_FOV_NATIVE_DEFAULT;
            break;
        case DefZNear:
            // e.g.            25.0  +             10.0         - 25.0
            result = Camera->m_zNear + cameraMeta.znear.value() - CAMERA_ZNEAR_NATIVE_DEFAULT;
            break;
        case DefZFar:
            // e.g.         50000.0  +         4000.0          - 50000.0
            result = Camera->m_zFar  + cameraMeta.zfar.value() - CAMERA_ZFAR_NATIVE_DEFAULT;
            break;
        }

        return qRound(result);
    };

    auto notEqual = [] (const float v1, const float v2)
    {
        return qAbs(v1 - v2) > 0.1f;
    };

    emit messageSig(LOG_STATUS,QString("Setting Camera %1").arg(Camera->m_strName));

    QString imageFileName;

    if (currentStep){
        int it = lcGetActiveProject()->GetImageType();
        switch(it){
        case Options::Mt::PLI:
            cameraMeta.cameraAngles   = currentStep->pli.pliMeta.cameraAngles;
            cameraMeta.cameraDistance = currentStep->pli.pliMeta.cameraDistance;
            cameraMeta.modelScale     = currentStep->pli.pliMeta.modelScale;
            cameraMeta.cameraFoV      = currentStep->pli.pliMeta.cameraFoV;
            cameraMeta.zfar           = currentStep->pli.pliMeta.zfar;
            cameraMeta.znear          = currentStep->pli.pliMeta.znear;
            cameraMeta.isOrtho        = currentStep->pli.pliMeta.isOrtho;
            cameraMeta.imageSize      = currentStep->pli.pliMeta.imageSize;
            cameraMeta.target         = currentStep->pli.pliMeta.target;
            break;
        case Options::Mt::SMP:
            cameraMeta.cameraAngles   = currentStep->subModel.subModelMeta.cameraAngles;
            cameraMeta.cameraDistance = currentStep->subModel.subModelMeta.cameraDistance;
            cameraMeta.modelScale     = currentStep->subModel.subModelMeta.modelScale;
            cameraMeta.cameraFoV      = currentStep->subModel.subModelMeta.cameraFoV;
            cameraMeta.zfar           = currentStep->subModel.subModelMeta.zfar;
            cameraMeta.znear          = currentStep->subModel.subModelMeta.znear;
            cameraMeta.isOrtho        = currentStep->subModel.subModelMeta.isOrtho;
            cameraMeta.imageSize      = currentStep->subModel.subModelMeta.imageSize;
            cameraMeta.target         = currentStep->subModel.subModelMeta.target;
            break;
        default: /*Options::Mt::CSI:*/
            cameraMeta                = currentStep->csiStepMeta;
            imageFileName             = currentStep->pngName;
            break;
        }

        QString metaString;
        bool newCommand = false;
        Where undefined = Where();
        Where top = currentStep->topOfStep();
        Where bottom = currentStep->bottomOfStep();

        float Latitude, Longitude, Distance;
        Camera->GetAngles(Latitude, Longitude, Distance);

        // get target position

        bool applyTarget = !(Camera->mTargetPosition[0] == 0.0f  &&
                             Camera->mTargetPosition[1] == 0.0f  &&
                             Camera->mTargetPosition[2] == 0.0f) &&
                             setTargetPositionAct->isChecked();
        if (applyTarget) {
            lcModel* ActiveModel = ActiveView->GetActiveModel();
            if (ActiveModel && ActiveModel->AnyPiecesSelected()) {
                lcVector3 Min(FLT_MAX, FLT_MAX, FLT_MAX), Max(-FLT_MAX, -FLT_MAX, -FLT_MAX);
                if (ActiveModel->GetPiecesBoundingBox(Min, Max))
                {
                   lcVector3 Target = (Min + Max) / 2.0f;
                   Camera->SetAngles(Latitude,Longitude,Distance,Target);
                }
            }
        }

        beginMacro("CameraSettings");

        // execute first in last out

        // LeoCAD flips Y an Z axis so that Z is up and Y represents depth
        if (applyTarget) {
            if (!imageFileName.isEmpty())
                gui->clearStepCSICache(imageFileName);
            cameraMeta.target.setValues(Camera->mTargetPosition[0],
                                        Camera->mTargetPosition[2],
                                        Camera->mTargetPosition[1]);
            metaString = cameraMeta.target.format(true/*local*/,false/*global*/);
            newCommand = cameraMeta.target.here() == undefined;
            currentStep->mi(it)->setMetaAlt(top, metaString, newCommand);
//            currentStep->mi(it)->setMeta(top,bottom,&cameraMeta.target,true/*useTop*/,1/*append*/,true/*local*/,false/*askLocal,global=false*/);
        }

        if (useImageSizeAct->isChecked()) {
            cameraMeta.imageSize.setValues(lcGetActiveProject()->GetImageWidth(),
                                           lcGetActiveProject()->GetImageHeight());
            metaString = cameraMeta.imageSize.format(true,false);
            newCommand = cameraMeta.imageSize.here() == undefined;
            currentStep->mi(it)->setMetaAlt(top, metaString, newCommand);
        }

        if (notEqual(Camera->GetScale(), cameraMeta.modelScale.value())) {
            cameraMeta.modelScale.setValue(Camera->GetScale());
            metaString = cameraMeta.modelScale.format(true,false);
            newCommand = cameraMeta.modelScale.here() == undefined;
            currentStep->mi(it)->setMetaAlt(top, metaString, newCommand);
        }

        if (notEqual(qRound(Distance), cameraMeta.cameraDistance.value()) &&
            !useImageSizeAct->isChecked()) {
            cameraMeta.cameraDistance.setValue(qRound(Distance));
            metaString = cameraMeta.cameraDistance.format(true,false);
            newCommand = cameraMeta.cameraDistance.here() == undefined;
            currentStep->mi(it)->setMetaAlt(top, metaString, newCommand);
        }

        if (notEqual(qRound(Latitude), cameraMeta.cameraAngles.value(0)) ||
            notEqual(qRound(Longitude),cameraMeta.cameraAngles.value(1))) {
            cameraMeta.cameraAngles.setValues(qRound(Latitude), qRound(Longitude));
            metaString = cameraMeta.cameraAngles.format(true,false);
            newCommand = cameraMeta.cameraAngles.here() == undefined;
            currentStep->mi(it)->setMetaAlt(top, metaString, newCommand);
        }

        float zFar = validCameraValue(DefZFar);
        if (notEqual(cameraMeta.zfar.value(),zFar)) {
            cameraMeta.zfar.setValue(zFar);
            metaString = cameraMeta.zfar.format(true,false);
            newCommand = cameraMeta.zfar.here() == undefined;
            currentStep->mi(it)->setMetaAlt(top, metaString, newCommand);
        }

        float zNear = validCameraValue(DefZNear);
        if (notEqual(cameraMeta.znear.value(), zNear)) {
            cameraMeta.znear.setValue(zNear);
            metaString = cameraMeta.znear.format(true,false);
            newCommand = cameraMeta.znear.here() == undefined;
            currentStep->mi(it)->setMetaAlt(top, metaString, newCommand);
        }

        float fovy = validCameraValue(DefFoV);
        if (notEqual(cameraMeta.cameraFoV.value(), fovy)) {
            cameraMeta.cameraFoV.setValue(fovy);
            metaString = cameraMeta.cameraFoV.format(true,false);
            newCommand = cameraMeta.cameraFoV.here() == undefined;
            currentStep->mi(it)->setMetaAlt(top, metaString, newCommand);
        }

        bool isOrtho = Camera->IsOrtho();
        cameraMeta.isOrtho.setValue(isOrtho);
        if (cameraMeta.isOrtho.value()) {
            metaString = cameraMeta.isOrtho.format(true,false);
            newCommand = cameraMeta.isOrtho.here() == undefined;
            currentStep->mi(it)->setMetaAlt(top, metaString, newCommand);
        }

        if (!QString(Camera->m_strName).isEmpty()) {
            cameraMeta.cameraName.setValue(Camera->m_strName);
            metaString = cameraMeta.cameraName.format(true,false);
            newCommand = cameraMeta.cameraName.here() == undefined;
            currentStep->mi(it)->setMetaAlt(top, metaString, newCommand);
        }

        endMacro();
    }
}

void Gui::setTargetPosition()
{
  lcSetProfileInt(LC_PROFILE_SET_TARGET_POSITION, setTargetPositionAct->isChecked());
  if (setTargetPositionAct->isChecked())
      applyCameraAct->setIcon(QIcon(":/resources/applycamerasettingsposition.png"));
  else
      applyCameraAct->setIcon(QIcon(":/resources/applycamerasettings.png"));
}

void Gui::useImageSize()
{
   lcSetProfileInt(LC_PROFILE_USE_IMAGE_SIZE, useImageSizeAct->isChecked());
}

void Gui::showDefaultCameraProperties()
{
  lcGetPreferences().mDefaultCameraProperties = defaultCameraPropertiesAct->isChecked();
  lcSetProfileInt(LC_PROFILE_DEFAULT_CAMERA_PROPERTIES, defaultCameraPropertiesAct->isChecked());
}

void Gui::createStatusBar()
{
  statusBar()->showMessage(tr("Ready"));
  connect(gMainWindow->mLCStatusBar, SIGNAL(messageChanged(QString)), this, SLOT(showLCStatusMessage()));
}

void Gui::SetActiveModel(const QString &fileName,bool newSubmodel)
{
    if (fileName != VIEWER_MODEL_DEFAULT) {
        displayFile(&ldrawFile,
                    newSubmodel ? fileName :
                                  getSubmodelName(QString(viewerStepKey[0]).toInt()));
    }
}

/*********************************************
 *
 * split viewer step keys
 *
 ********************************************/

QStringList Gui::getViewerStepKeys(bool modelName, bool pliPart)
{
    // viewerStepKey - 3 elements:
    // CSI: 0=modelNameIndex, 1=lineNumber,   2=stepNumber [_fm (finalModel)]
    // SMP: 0=modelNameIndex, 1=lineNumber,   2=stepNumber [_Preview (Submodel Preview)]
    // PLI: 0=partNameString, 1=colourNumber, 2=stepNumber
    QStringList keys = viewerStepKey.split(";");
    // confirm keys has at least 3 elements
    if (keys.size() < 3) {
        if (Preferences::debugLogging)
            emit messageSig(LOG_DEBUG, QString("Parse stepKey [%1] failed").arg(viewerStepKey));
        return QStringList();
    }

    if (!pliPart) {
        bool ok;
        int modelNameIndex = keys[0].toInt(&ok);
        if (!ok) {
            if (Preferences::debugLogging)
                emit messageSig(LOG_DEBUG, QString("Parse stepKey failed. Expected model name index integer got [%1]").arg(keys[0]));
            return QStringList();
        }

        if (modelName)
            keys.replace(0,getSubmodelName(modelNameIndex));
    }

    return keys;
}

/*********************************************
 *
 * extract stepKey - callled for CSI and SMP only
 *
 ********************************************/

  bool Gui::extractStepKey(Where &here, int &stepNumber)
  {
      // viewerStepKey elements CSI: 0=modelName, 1=lineNumber, 2=stepNumber [,3=_fm (finalModel)]
      QStringList keyArgs = getViewerStepKeys();

      if (!keyArgs.size())
          return false;

      QString modelName  = keyArgs[0]; // Converted to modelName in getViewerStepKeys()

      bool ok[2];
      int lineNumber = keyArgs[1].toInt(&ok[0]);
      if (modelName.isEmpty()) {
          emit messageSig(LOG_ERROR,QString("Model name was not found for index [%1]").arg(keyArgs[1]));
          return false;
      } else if (!ok[0]) {
          emit messageSig(LOG_ERROR,QString("Line number is not an integer [%1]").arg(keyArgs[1]));
          return false;
      } else {
          here = Where(modelName,lineNumber);
      }

      stepNumber = 0;
      if (page.modelDisplayOnlyStep) {
          stepNumber = QStringList(keyArgs[2].split("_")).first().toInt(&ok[1]);
      } else {
          stepNumber = keyArgs[2].toInt(&ok[1]);
      }
      if (!ok[1]) {
          emit messageSig(LOG_NOTICE,QString("Step number is not an integer [%1]").arg(keyArgs[2]));
          return false;
      }

//      if (Preferences::debugLogging) {
//          QString messsage = QString("Step Key parse OK, modelName: %1, lineNumber: %2, stepNumber: %3")
//                                     .arg(modelName).arg(lineNumber).arg(stepNumber);
//          if (!stepNumber && page.pli.tsize() && page.pli.bom)
//              messsage = QString("Step Key parse OK but this is a BOM page, step pageNumber: %1")
//                                 .arg(stepPageNum);
//          emit messageSig(LOG_DEBUG, messsage);
//      }

      return true;
  }

/*********************************************
 *
 * current step - called for CSI and SMP only
 *
 ********************************************/

void Gui::setCurrentStep(Step *step, Where here, int stepNumber)
{
    bool stepMatch = false;
    bool multiStep = false;
    bool calledOut = false;

    if (step && (calledOut = isViewerStepCalledOut(viewerStepKey))) {
        stepMatch = step->stepNumber.number == stepNumber;
        if (! stepMatch) {
            for (int k = 0; k < step->list.size() && !stepMatch; k++) {
                if (step->list[k]->relativeType == CalloutType) {
                    Callout *callout = dynamic_cast<Callout *>(step->list[k]);
                    for (int l = 0; l < callout->list.size() && !stepMatch; l++){
                        Range *range = dynamic_cast<Range *>(callout->list[l]);
                        for (int m = 0; m < range->list.size() && !stepMatch; m++){
                            if (range->relativeType == RangeType) {
                                Step *step = dynamic_cast<Step *>(range->list[m]);
                                if (step && step->relativeType == StepType){
                                    setCurrentStep(step, here, stepNumber);
                                }
                            }
                        }
                    }
                }
            }
        }
    } else if ((multiStep = isViewerStepMultiStep(viewerStepKey))) {
        for (int i = 0; i < page.list.size() && !stepMatch; i++){
            Range *range = dynamic_cast<Range *>(page.list[i]);
            for (int j = 0; j < range->list.size(); j++){
                if (range->relativeType == RangeType) {
                    step = dynamic_cast<Step *>(range->list[j]);
                    if (step && step->relativeType == StepType){
                        stepMatch = step->stepNumber.number == stepNumber;
                        if (stepMatch)
                            break;
                    }
                }
            }
        }
    } else if (gStep) {
        step = (stepMatch = gStep->stepNumber.number == stepNumber) ? gStep : nullptr;
    }

    if (Preferences::debugLogging && !stepMatch)
        emit messageSig(LOG_DEBUG, QString("%1 Step number %2 for %3 - modelName [%4] topOfStep [%5]")
                                           .arg(stepMatch ? "Match!" : "Oh oh!")
                                           .arg(QString("%1 %2").arg(stepNumber).arg(stepMatch ? "found" : "not found"))
                                           .arg(multiStep ? "multi step" : calledOut ? "called out" : "single step")
                                           .arg(here.modelName).arg(here.lineNumber));
    currentStep = step;
}

void Gui::setCurrentStep()
{
    Step *step = nullptr;
    Where here;
    int stepNumber;

    extractStepKey(here,stepNumber);

    if (!stepNumber) {
        currentStep = step;
        return;
    }

    setCurrentStep(step, here, stepNumber);
}

/*********************************************
 *
 * slelcted Line
 *
 ********************************************/

bool Gui::getSelectedLine(int modelIndex, int lineIndex, int source, int &lineNumber) {

    lineNumber        = 0;
    bool currentModel = modelIndex == -1;
    bool fromViewer   = source > EDITOR_LINE;

    if (currentModel) {

        if (!currentStep)
            return false;

        if (Preferences::debugLogging) {
            emit messageSig(LOG_TRACE, QString("Step lineIndex size: %1 item(s)")
                                                .arg(currentStep->lineTypeIndexes.size()));
            for (int i = 0; i < currentStep->lineTypeIndexes.size(); ++i)
                emit messageSig(LOG_TRACE, QString("-Part lineNumber: [%1] at step line lineIndex [%2] - specified lineIndex [%3]")
                                                   .arg(currentStep->lineTypeIndexes.at(i)).arg(i).arg(lineIndex));
        }

        if (fromViewer)      // input relativeIndes
            lineNumber = currentStep->getLineTypeRelativeIndex(lineIndex);
        else                 // input lineTypeIndex
            lineNumber = currentStep->getLineTypeIndex(lineIndex);

    } else {

        if (fromViewer)      // input relativeIndes
            lineNumber = getLineTypeRelativeIndex(modelIndex,lineIndex); // return lineTypeIndex - part lineNumber
        else                 // input lineTypeIndex
            lineNumber = getLineTypeIndex(modelIndex,lineIndex);         // return relativeIndex - step line lineIndex
    }

    return lineNumber != -1;
}

/*********************************************
 *
 * Selected part index(es)
 *
 ********************************************/

void Gui::SelectedPartLines(QVector<TypeLine> &indexes, PartSource source){
    if (! exporting()) {

        if (!currentStep)
            return;

        QVector<int> lines;
        bool fromViewer   = source > EDITOR_LINE;
        bool validLine    = false;
        int lineIndex     = -1;
        int lineNumber    = -1;;
        QString stepModel = viewerStepKey.isEmpty() ? "undefined" : gui->getSubmodelName(QString(viewerStepKey[0]).toInt());
        QString modelName = indexes.size() ? getSubmodelName(indexes.at(0).modelIndex) : "undefined";
        int modelIndex    = modelName == "undefined" ? -1 : indexes.at(0).modelIndex;

        if (Preferences::debugLogging && modelIndex != -1) {
            emit messageSig(LOG_TRACE, QString("Submodel lineIndex size: %1 item(s)")
                                               .arg(ldrawFile.getLineTypeRelativeIndexes(modelIndex)->size()));
        }

        for (int i = 0; i < indexes.size(); ++i) {

            lineIndex  = indexes.at(i).lineIndex;
            validLine  = getSelectedLine(modelIndex, lineIndex, source, lineNumber);

            if (validLine) {
                lines.append(lineNumber);
                if (fromViewer) {
                    Where here = Where(modelName, lineNumber);
                    if (!modifiedParts.contains(here))
                        modifiedParts.append(here);
                }
            }

            if (Preferences::debugLogging) {
                QString Message;
                if (!validLine) {
                    Message = tr("Invalid part lineNumber [%1] for step line index [%2]")
                                 .arg(fromViewer ? lineNumber : lineIndex)
                                 .arg(fromViewer ? lineIndex : lineNumber);
                } else if (fromViewer) {
                    Message = tr("Selected part modelName [%1] lineNumber: [%2] at step line index [%3]")
                                 .arg(modelName).arg(lineNumber).arg(lineIndex);
                } else {
                    Message = tr("Selected part modelName [%1] lineNumber: [%2] at step line index [%3]")
                                 .arg(modelName).arg(lineIndex).arg(lineNumber);
                }
                emit messageSig(LOG_TRACE, Message);
            }
        }

        if (fromViewer)
            emit highlightSelectedLinesSig(lines);
        else
            emit setSelectedPiecesSig(lines);
    }
}
