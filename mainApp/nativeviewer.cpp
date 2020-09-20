/****************************************************************************
**
** Copyright (C) 2019 Trevor SANDY. All rights reserved.
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

    tabifyDockWidget(gMainWindow->mTimelineToolBar, gMainWindow->mPropertiesToolBar);

    //Part Selection
    gMainWindow->mPartsToolBar->setWindowTitle(trUtf8("Parts"));
    gMainWindow->mPartsToolBar->setObjectName("PartsToolbar");
    gMainWindow->mPartsToolBar->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    addDockWidget(Qt::RightDockWidgetArea, gMainWindow->mPartsToolBar);

    QSettings Settings;
    bool viewable = false;
    if (Settings.contains(QString("%1/%2").arg(SETTINGS,VIEW_PARTS_WIDGET_KEY)))
        viewable = Settings.value(QString("%1/%2").arg(SETTINGS,VIEW_PARTS_WIDGET_KEY)).toBool();
    viewMenu->addAction(gMainWindow->mPartsToolBar->toggleViewAction());
    gMainWindow->mPartsToolBar->toggleViewAction()->setChecked(viewable);
    connect (gMainWindow->mPartsToolBar, SIGNAL (visibilityChanged(bool)),
                                   this, SLOT (partsWidgetVisibilityChanged(bool)));

    tabifyDockWidget(gMainWindow->mPropertiesToolBar, gMainWindow->mPartsToolBar);

    //Colors Selection
    gMainWindow->mColorsToolBar->setWindowTitle(trUtf8("Colors"));
    gMainWindow->mColorsToolBar->setObjectName("ColorsToolbar");
    gMainWindow->mColorsToolBar->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    addDockWidget(Qt::RightDockWidgetArea, gMainWindow->mColorsToolBar);
    viewable = false;
    if (Settings.contains(QString("%1/%2").arg(SETTINGS,VIEW_COLORS_WIDGET_KEY)))
        viewable = Settings.value(QString("%1/%2").arg(SETTINGS,VIEW_COLORS_WIDGET_KEY)).toBool();
    viewMenu->addAction(gMainWindow->mColorsToolBar->toggleViewAction());
    gMainWindow->mColorsToolBar->toggleViewAction()->setChecked(viewable);
    connect (gMainWindow->mColorsToolBar, SIGNAL (visibilityChanged(bool)),
                                    this, SLOT (coloursWidgetVisibilityChanged(bool)));

    tabifyDockWidget(gMainWindow->mPartsToolBar, gMainWindow->mColorsToolBar);

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
    Step *currentStep = gui->getCurrentStep();

    if (currentStep){
        int it = lcGetActiveProject()->GetImageType();
        switch(it){
        case Render::Mt::PLI:
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
        case Render::Mt::SMP:
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
        default: /*Render::Mt::CSI:*/
            cameraMeta                = currentStep->csiCameraMeta;
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

