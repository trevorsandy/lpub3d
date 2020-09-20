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
#include "metagui.h"

#include "light.h"
#include "group.h"
#include "camera.h"
#include "project.h"
#include "view.h"
#include "piece.h"
#include "pieceinf.h"
#include "lc_qutils.h"
#include "lc_profile.h"
#include "application.h"
#include "lc_partselectionwidget.h"

void Gui::create3DActions()
{
    QIcon CreateBuildModIcon;
    CreateBuildModIcon.addFile(":/resources/buildmodcreate.png");
    CreateBuildModIcon.addFile(":/resources/buildmodcreate16.png");
    createBuildModAct = new QAction(CreateBuildModIcon,tr("Create Build Modification"),this);
    createBuildModAct->setStatusTip(tr("Create a new build modification for this step - Shift+M"));
    createBuildModAct->setShortcut(tr("Shift+M"));
    connect(createBuildModAct, SIGNAL(triggered()), this, SLOT(createBuildModification()));

    QIcon ApplyBuildModIcon;
    ApplyBuildModIcon.addFile(":/resources/buildmodapply.png");
    ApplyBuildModIcon.addFile(":/resources/buildmodapply16.png");
    applyBuildModAct = new QAction(ApplyBuildModIcon,tr("Apply Build Modification..."),this);
    applyBuildModAct->setStatusTip(tr("Apply existing build modification to this step"));
    connect(applyBuildModAct, SIGNAL(triggered()), this, SLOT(applyBuildModification()));

    QIcon RemoveBuildModIcon;
    RemoveBuildModIcon.addFile(":/resources/buildmodremove.png");
    RemoveBuildModIcon.addFile(":/resources/buildmodremove16.png");
    removeBuildModAct = new QAction(RemoveBuildModIcon,tr("Remove Build Modification..."),this);
    removeBuildModAct->setStatusTip(tr("Remove build modification from this step"));
    connect(removeBuildModAct, SIGNAL(triggered()), this, SLOT(removeBuildModification()));

    enableBuildModAct = new QAction(tr("Build Modifications Enabled"),this);
    enableBuildModAct->setStatusTip(tr("Enable build modification configuration on rotate action"));
    enableBuildModAct->setCheckable(true);
    enableBuildModAct->setChecked(lcGetProfileInt(LC_PROFILE_BUILD_MODIFICATION));
    connect(enableBuildModAct, SIGNAL(triggered()), this, SLOT(enableBuildModification()));

    enableRotstepRotateAct = new QAction(tr("ROTSTEP Rotate Enabled"),this);
    enableRotstepRotateAct->setStatusTip(tr("Enable ROTSTEP configuration on rotate action"));
    enableRotstepRotateAct->setCheckable(true);
    enableRotstepRotateAct->setChecked(!lcGetProfileInt(LC_PROFILE_BUILD_MODIFICATION));
    connect(enableRotstepRotateAct, SIGNAL(triggered()), this, SLOT(enableBuildModification()));

    setTargetPositionAct = new QAction(tr("Set Camera Target"),this);
    setTargetPositionAct->setStatusTip(tr("Set the camera target (Look At) position for this step"));
    setTargetPositionAct->setCheckable(true);
    setTargetPositionAct->setChecked(lcGetProfileInt(LC_PROFILE_SET_TARGET_POSITION));
    connect(setTargetPositionAct, SIGNAL(triggered()), this, SLOT(setTargetPosition()));


    QIcon ApplyCameraIcon;
    if (setTargetPositionAct->isChecked()){
        ApplyCameraIcon.addFile(":/resources/applycamerasettingsposition.png");
        ApplyCameraIcon.addFile(":/resources/applycamerasettingsposition_16.png");
    } else {
        ApplyCameraIcon.addFile(":/resources/applycamerasettings.png");
        ApplyCameraIcon.addFile(":/resources/applycamerasettings_16.png");
    }
    applyCameraAct = new QAction(ApplyCameraIcon,tr("&Apply Camera Settings"),this);
    applyCameraAct->setStatusTip(tr("Apply camera settings to this step - Shift+A"));
    applyCameraAct->setShortcut(tr("Shift+A"));
    connect(applyCameraAct, SIGNAL(triggered()), this, SLOT(applyCameraSettings()));

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

    MoveAction = new QAction(tr("Movement Snap"), this);
    MoveAction->setStatusTip(tr("Snap translations to fixed intervals"));
    MoveAction->setIcon(QIcon(":/resources/edit_snap_move.png"));

    AngleAction = new QAction(tr("Rotation Snap"), this);
    AngleAction->setStatusTip(tr("Snap rotations to fixed intervals"));
    AngleAction->setIcon(QIcon(":/resources/edit_snap_angle.png"));

    QIcon ViewViewPointHomeIcon;
    ViewViewPointHomeIcon.addFile(":/resources/veiw_viewpoint_home.png");
    ViewViewPointHomeIcon.addFile(":/resources/veiw_viewpoint_home_16.png");
    gMainWindow->mActions[LC_VIEW_VIEWPOINT_HOME]->setIcon(ViewViewPointHomeIcon);

    QIcon EditActionRotstepIcon;
    EditActionRotstepIcon.addFile(":/resources/edit_rotatestep.png");
    EditActionRotstepIcon.addFile(":/resources/edit_rotatestep_16.png");
    gMainWindow->mActions[LC_EDIT_ACTION_ROTATESTEP]->setIcon(EditActionRotstepIcon);

    gMainWindow->mActions[LC_EDIT_ACTION_CLEAR_TRANSFORM]->setIcon(QIcon(":/resources/clear_transform.png"));

    // Light icons
    LightGroupAct = new QAction(tr("Lights"), this);
    LightGroupAct->setToolTip(tr("Lights - Pointlight"));
    LightGroupAct->setIcon(gMainWindow->mActions[LC_EDIT_ACTION_LIGHT]->icon());
    LightGroupAct->setStatusTip(gMainWindow->mActions[LC_EDIT_ACTION_LIGHT]->statusTip());
    LightGroupAct->setProperty("CommandId", QVariant(LC_EDIT_ACTION_LIGHT));
    gMainWindow->mActions[LC_EDIT_ACTION_LIGHT]->setProperty("CommandId", QVariant(LC_EDIT_ACTION_LIGHT));
    gMainWindow->mActions[LC_EDIT_ACTION_SUNLIGHT]->setProperty("CommandId", QVariant(LC_EDIT_ACTION_SUNLIGHT));
    gMainWindow->mActions[LC_EDIT_ACTION_SPOTLIGHT]->setProperty("CommandId", QVariant(LC_EDIT_ACTION_SPOTLIGHT));
    gMainWindow->mActions[LC_EDIT_ACTION_AREALIGHT]->setProperty("CommandId", QVariant(LC_EDIT_ACTION_AREALIGHT));
    gMainWindow->mActions[LC_EDIT_ACTION_LIGHT]->setProperty("CommandTip", QVariant("Lights - Pointlight"));
    gMainWindow->mActions[LC_EDIT_ACTION_SUNLIGHT]->setProperty("CommandTip", QVariant("Lights - Sunlight"));
    gMainWindow->mActions[LC_EDIT_ACTION_SPOTLIGHT]->setProperty("CommandTip", QVariant("Lights - Spotlight"));
    gMainWindow->mActions[LC_EDIT_ACTION_AREALIGHT]->setProperty("CommandTip", QVariant("Lights - Arealight"));
    connect(LightGroupAct, SIGNAL(triggered()), this, SLOT(groupActionTriggered()));
    connect(gMainWindow->mActions[LC_EDIT_ACTION_LIGHT], SIGNAL(triggered()), this, SLOT(groupActionTriggered()));
    connect(gMainWindow->mActions[LC_EDIT_ACTION_SUNLIGHT], SIGNAL(triggered()), this, SLOT(groupActionTriggered()));
    connect(gMainWindow->mActions[LC_EDIT_ACTION_SPOTLIGHT], SIGNAL(triggered()), this, SLOT(groupActionTriggered()));
    connect(gMainWindow->mActions[LC_EDIT_ACTION_AREALIGHT], SIGNAL(triggered()), this, SLOT(groupActionTriggered()));

    // Viewpoint icons
    ViewpointGroupAct = new QAction(tr("Viewpoints"), this);
    ViewpointGroupAct->setToolTip(tr("Viewpoints - Home"));
    ViewpointGroupAct->setIcon(gMainWindow->mActions[LC_VIEW_VIEWPOINT_HOME]->icon());
    ViewpointGroupAct->setStatusTip(gMainWindow->mActions[LC_VIEW_VIEWPOINT_HOME]->statusTip());
    ViewpointGroupAct->setProperty("CommandId", QVariant(LC_VIEW_VIEWPOINT_HOME));
    gMainWindow->mActions[LC_VIEW_VIEWPOINT_FRONT]->setIcon(QIcon(":/resources/front.png"));
    gMainWindow->mActions[LC_VIEW_VIEWPOINT_BACK]->setIcon(QIcon(":/resources/back.png"));
    gMainWindow->mActions[LC_VIEW_VIEWPOINT_LEFT]->setIcon(QIcon(":/resources/left.png"));
    gMainWindow->mActions[LC_VIEW_VIEWPOINT_RIGHT]->setIcon(QIcon(":/resources/right.png"));
    gMainWindow->mActions[LC_VIEW_VIEWPOINT_TOP]->setIcon(QIcon(":/resources/top.png"));
    gMainWindow->mActions[LC_VIEW_VIEWPOINT_BOTTOM]->setIcon(QIcon(":/resources/bottom.png"));
    gMainWindow->mActions[LC_VIEW_VIEWPOINT_FRONT]->setProperty("CommandId", QVariant(LC_VIEW_VIEWPOINT_FRONT));
    gMainWindow->mActions[LC_VIEW_VIEWPOINT_BACK]->setProperty("CommandId", QVariant(LC_VIEW_VIEWPOINT_BACK));
    gMainWindow->mActions[LC_VIEW_VIEWPOINT_LEFT]->setProperty("CommandId", QVariant(LC_VIEW_VIEWPOINT_LEFT));
    gMainWindow->mActions[LC_VIEW_VIEWPOINT_RIGHT]->setProperty("CommandId", QVariant(LC_VIEW_VIEWPOINT_RIGHT));
    gMainWindow->mActions[LC_VIEW_VIEWPOINT_TOP]->setProperty("CommandId", QVariant(LC_VIEW_VIEWPOINT_TOP));
    gMainWindow->mActions[LC_VIEW_VIEWPOINT_BOTTOM]->setProperty("CommandId", QVariant(LC_VIEW_VIEWPOINT_BOTTOM));
    gMainWindow->mActions[LC_VIEW_VIEWPOINT_HOME]->setProperty("CommandId", QVariant(LC_VIEW_VIEWPOINT_HOME));
    gMainWindow->mActions[LC_VIEW_VIEWPOINT_FRONT]-> setProperty("CommandTip", QVariant("Viewpoints - Front"));
    gMainWindow->mActions[LC_VIEW_VIEWPOINT_BACK]->  setProperty("CommandTip", QVariant("Viewpoints - Back"));
    gMainWindow->mActions[LC_VIEW_VIEWPOINT_LEFT]->  setProperty("CommandTip", QVariant("Viewpoints - Left"));
    gMainWindow->mActions[LC_VIEW_VIEWPOINT_RIGHT]-> setProperty("CommandTip", QVariant("Viewpoints - Right"));
    gMainWindow->mActions[LC_VIEW_VIEWPOINT_TOP]->   setProperty("CommandTip", QVariant("Viewpoints - Top"));
    gMainWindow->mActions[LC_VIEW_VIEWPOINT_BOTTOM]->setProperty("CommandTip", QVariant("Viewpoints - Bottom"));
    gMainWindow->mActions[LC_VIEW_VIEWPOINT_HOME]->  setProperty("CommandTip", QVariant("Viewpoints - Home"));
    connect(ViewpointGroupAct, SIGNAL(triggered()), this, SLOT(groupActionTriggered()));
    connect(gMainWindow->mActions[LC_VIEW_VIEWPOINT_FRONT], SIGNAL(triggered()), this, SLOT(groupActionTriggered()));
    connect(gMainWindow->mActions[LC_VIEW_VIEWPOINT_BACK], SIGNAL(triggered()), this, SLOT(groupActionTriggered()));
    connect(gMainWindow->mActions[LC_VIEW_VIEWPOINT_LEFT], SIGNAL(triggered()), this, SLOT(groupActionTriggered()));
    connect(gMainWindow->mActions[LC_VIEW_VIEWPOINT_RIGHT], SIGNAL(triggered()), this, SLOT(groupActionTriggered()));
    connect(gMainWindow->mActions[LC_VIEW_VIEWPOINT_TOP], SIGNAL(triggered()), this, SLOT(groupActionTriggered()));
    connect(gMainWindow->mActions[LC_VIEW_VIEWPOINT_BOTTOM], SIGNAL(triggered()), this, SLOT(groupActionTriggered()));
    connect(gMainWindow->mActions[LC_VIEW_VIEWPOINT_HOME], SIGNAL(triggered()), this, SLOT(groupActionTriggered()));

    ViewpointZoomExtAct = new QAction(tr("Fit To View"),this);
    ViewpointZoomExtAct->setStatusTip(tr("Specify whether to zoom all items to fit the current view"));
    ViewpointZoomExtAct->setCheckable(true);
    ViewpointZoomExtAct->setChecked(lcGetProfileInt(LC_PROFILE_VIEWPOINT_ZOOM_EXTENT));
    connect(ViewpointZoomExtAct, SIGNAL(triggered()), this, SLOT(groupActionTriggered()));

    viewerZoomSliderAct = new QWidgetAction(nullptr);
    viewerZoomSliderWidget = new QSlider();
    viewerZoomSliderWidget->setSingleStep(1);
    viewerZoomSliderWidget->setTickInterval(10);
    viewerZoomSliderWidget->setTickPosition(QSlider::TicksBelow);
    viewerZoomSliderWidget->setMaximum(150);
    viewerZoomSliderWidget->setMinimum(1);
    viewerZoomSliderWidget->setValue(50);
    connect(viewerZoomSliderWidget, SIGNAL(valueChanged(int)), this, SLOT(ViewerZoomSlider(int)));

    gMainWindow->mActions[LC_FILE_SAVE_IMAGE]->setIcon(QIcon(":/resources/saveimage.png"));
    gMainWindow->mActions[LC_FILE_SAVE_IMAGE]->setShortcut(tr("Alt+0"));
    gMainWindow->mActions[LC_FILE_SAVE_IMAGE]->setStatusTip(tr("Save an image of the current view - Alt+0"));

    gMainWindow->mActions[LC_FILE_EXPORT_3DS]->setIcon(QIcon(":/resources/3ds32.png"));
    gMainWindow->mActions[LC_FILE_EXPORT_COLLADA]->setIcon(QIcon(":/resources/dae32.png"));
    gMainWindow->mActions[LC_FILE_EXPORT_WAVEFRONT]->setIcon(QIcon(":/resources/obj32.png"));

    QIcon ViewZoomExtentsIcon;
    ViewZoomExtentsIcon.addFile(":/resources/viewzoomextents.png");
    ViewZoomExtentsIcon.addFile(":/resources/viewzoomextents16.png");
    gMainWindow->mActions[LC_VIEW_ZOOM_EXTENTS]->setIcon(ViewZoomExtentsIcon);

    QIcon ViewLookAtIcon;
    ViewLookAtIcon.addFile(":/resources/convertcallout.png");
    ViewLookAtIcon.addFile(":/resources/convertcallout16.png");
    gMainWindow->mActions[LC_VIEW_LOOK_AT]->setIcon(ViewLookAtIcon);
    gMainWindow->mActions[LC_VIEW_LOOK_AT]->setShortcut(tr("Shift+O"));
    gMainWindow->mActions[LC_VIEW_LOOK_AT]->setStatusTip(tr("Rotate view so selected pieces are at center - Shift+O"));

    enableBuildModification();
}

void Gui::create3DMenus()
{
     /*
      * Not used
     FileMenuViewer = menuBar()->addMenu(tr("&Step"));
     FileMenuViewer->addAction(gMainWindow->mActions[LC_FILE_SAVEAS]);
     */

     ViewerExportMenu = new QMenu(tr("&Export As..."), this);
     ViewerExportMenu->setIcon(QIcon(":/resources/exportas.png"));
     ViewerExportMenu->addAction(gMainWindow->mActions[LC_FILE_EXPORT_3DS]);
     ViewerExportMenu->addAction(gMainWindow->mActions[LC_FILE_EXPORT_COLLADA]);
     ViewerExportMenu->addAction(gMainWindow->mActions[LC_FILE_EXPORT_WAVEFRONT]);

     SnapXYMenu = new QMenu(tr("Snap XY"), this);
     for (int actionIdx = LC_EDIT_SNAP_MOVE_XY0; actionIdx <= LC_EDIT_SNAP_MOVE_XY9; actionIdx++)
         SnapXYMenu->addAction(gMainWindow->mActions[actionIdx]);

     SnapZMenu = new QMenu(tr("Snap Z"), this);
     for (int actionIdx = LC_EDIT_SNAP_MOVE_Z0; actionIdx <= LC_EDIT_SNAP_MOVE_Z9; actionIdx++)
         SnapZMenu->addAction(gMainWindow->mActions[actionIdx]);

     gMainWindow->mActions[LC_EDIT_ACTION_SELECT]->setMenu(gMainWindow->mSelectionModeMenu);

     SnapMenu = new QMenu(tr("Snap Menu"), this);
     SnapMenu->addAction(gMainWindow->mActions[LC_EDIT_SNAP_MOVE_TOGGLE]);
     SnapMenu->addSeparator();
     SnapMenu->addMenu(SnapXYMenu);
     SnapMenu->addMenu(SnapZMenu);
     MoveAction->setMenu(SnapMenu);

     SnapAngleMenu = new QMenu(tr("Snap Angle Menu"), this);
     SnapAngleMenu->addAction(gMainWindow->mActions[LC_EDIT_SNAP_ANGLE_TOGGLE]);
     SnapAngleMenu->addSeparator();
     for (int actionIdx = LC_EDIT_SNAP_ANGLE0; actionIdx <= LC_EDIT_SNAP_ANGLE9; actionIdx++)
         SnapAngleMenu->addAction(gMainWindow->mActions[actionIdx]);
     AngleAction->setMenu(SnapAngleMenu);

     buildModMenu = new QMenu(tr("Build Modification"),this);
     buildModMenu->addAction(applyBuildModAct);
     buildModMenu->addAction(removeBuildModAct);
     createBuildModAct->setMenu(buildModMenu);

     LightMenu = new QMenu(tr("Lights"), this);
     LightMenu->addAction(gMainWindow->mActions[LC_EDIT_ACTION_LIGHT]);
     LightMenu->addAction(gMainWindow->mActions[LC_EDIT_ACTION_SUNLIGHT]);
     LightMenu->addAction(gMainWindow->mActions[LC_EDIT_ACTION_SPOTLIGHT]);
     LightMenu->addAction(gMainWindow->mActions[LC_EDIT_ACTION_AREALIGHT]);
     LightGroupAct->setMenu(LightMenu);

     ViewpointGroupAct->setMenu(gMainWindow->GetViewpointMenu());
     gMainWindow->GetViewpointMenu()->insertAction(gMainWindow->mActions[LC_VIEW_VIEWPOINT_FRONT], ViewpointZoomExtAct);

     // Viewer menus
     ViewerMenu = menuBar()->addMenu(tr("&3DViewer"));
     // Render menu
     ViewerMenu->addAction(blenderRenderAct);
     ViewerMenu->addAction(blenderImportAct);
     ViewerMenu->addAction(povrayRenderAct);
     ViewerMenu->addSeparator();
     // Save Image menu
     ViewerMenu->addAction(gMainWindow->mActions[LC_FILE_SAVE_IMAGE]);
     // Export As menu
     ViewerMenu->addMenu(ViewerExportMenu);
     ViewerMenu->addSeparator();
     // Submodel Edit menus
     ViewerMenu->addAction(gMainWindow->mActions[LC_PIECE_EDIT_SELECTED_SUBMODEL]);
     ViewerMenu->addAction(gMainWindow->mActions[LC_PIECE_EDIT_END_SUBMODEL]);
     ViewerMenu->addSeparator();
     // Camera menu
     ViewerMenu->addMenu(gMainWindow->GetCameraMenu());
     // Tools menu
     gMainWindow->GetToolsMenu()->addAction(gMainWindow->mActions[LC_EDIT_UNDO]);
     gMainWindow->GetToolsMenu()->addAction(gMainWindow->mActions[LC_EDIT_REDO]);
     gMainWindow->GetToolsMenu()->addSeparator();
     gMainWindow->GetToolsMenu()->addAction(gMainWindow->mActions[LC_EDIT_ACTION_SELECT]);
     gMainWindow->GetToolsMenu()->addAction(gMainWindow->mActions[LC_EDIT_ACTION_ROTATE]);
     gMainWindow->GetToolsMenu()->addAction(gMainWindow->mActions[LC_EDIT_ACTION_ROTATESTEP]);
     gMainWindow->GetToolsMenu()->addAction(createBuildModAct);
     gMainWindow->GetToolsMenu()->addAction(applyCameraAct);
     gMainWindow->GetToolsMenu()->addAction(gMainWindow->mActions[LC_EDIT_ACTION_CAMERA]);
     gMainWindow->GetToolsMenu()->addAction(LightGroupAct);
     gMainWindow->GetToolsMenu()->addSeparator();
     gMainWindow->GetToolsMenu()->addAction(ViewpointGroupAct);
//     gMainWindow->GetToolsMenu()->addAction(gMainWindow->mActions[LC_VIEW_ZOOM_EXTENTS]);
     gMainWindow->GetToolsMenu()->addAction(gMainWindow->mActions[LC_VIEW_LOOK_AT]);
     gMainWindow->GetToolsMenu()->addAction(gMainWindow->mActions[LC_EDIT_ACTION_ZOOM]);
     gMainWindow->GetToolsMenu()->addAction(gMainWindow->mActions[LC_EDIT_ACTION_PAN]);
     gMainWindow->GetToolsMenu()->addAction(gMainWindow->mActions[LC_EDIT_ACTION_ROTATE_VIEW]);
     gMainWindow->GetToolsMenu()->addAction(gMainWindow->mActions[LC_EDIT_ACTION_ZOOM_REGION]);
     gMainWindow->GetToolsMenu()->addSeparator();
     gMainWindow->GetToolsMenu()->addAction(gMainWindow->mActions[LC_EDIT_TRANSFORM_RELATIVE]);
     gMainWindow->GetToolsMenu()->addAction(MoveAction);
     gMainWindow->GetToolsMenu()->addAction(AngleAction); // Snap Rotations to Fixed Intervals menu item
     ViewerMenu->addMenu(gMainWindow->GetToolsMenu());
     // ViewPoint menu
     ViewerMenu->addMenu(gMainWindow->GetViewpointMenu());
     // Projection menu
     ViewerMenu->addMenu(gMainWindow->GetProjectionMenu());
     // Shading menu
     ViewerMenu->addMenu(gMainWindow->GetShadingMenu());
     ViewerMenu->addSeparator();
     // View menus
     ViewerMenu->addAction(gMainWindow->mActions[LC_VIEW_SPLIT_HORIZONTAL]);
     ViewerMenu->addAction(gMainWindow->mActions[LC_VIEW_SPLIT_VERTICAL]);
     ViewerMenu->addAction(gMainWindow->mActions[LC_VIEW_REMOVE_VIEW]);
     ViewerMenu->addAction(gMainWindow->mActions[LC_VIEW_RESET_VIEWS]);
     ViewerMenu->addSeparator();
     // 3DViewer Preferences menu
     ViewerMenu->addAction(gMainWindow->mActions[LC_VIEW_PREFERENCES]);
     ViewerMenu->addSeparator();

     // Zoom slider menu item
     connect(gMainWindow->mActions[LC_VIEW_VIEWPOINT_HOME], SIGNAL(triggered()), this, SLOT(ResetViewerZoomSlider()));
     viewerZoomSliderAct->setDefaultWidget(viewerZoomSliderWidget);
     ViewerZoomSliderMenu = new QMenu(tr("Zoom Slider"),this);
     ViewerZoomSliderMenu->addAction(viewerZoomSliderAct);
     gMainWindow->mActions[LC_EDIT_ACTION_ZOOM]->setMenu(ViewerZoomSliderMenu);
     ViewerMenu->addSeparator();
     // 3DViewer about menu
     ViewerMenu->addAction(gMainWindow->mActions[LC_HELP_ABOUT]);

     rotateActionMenu = new QMenu(tr("Edit Rotate"),this);
     rotateActionMenu->addAction(enableRotstepRotateAct);
     rotateActionMenu->addAction(enableBuildModAct);
     gMainWindow->mActions[LC_EDIT_ACTION_ROTATE]->setMenu(rotateActionMenu);

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
    exportToolBar->addAction(blenderRenderAct);
    exportToolBar->addAction(blenderImportAct);
    exportToolBar->addAction(povrayRenderAct);
    exportToolBar->addSeparator();
    exportToolBar->addAction(gMainWindow->mActions[LC_FILE_SAVE_IMAGE]);
    exportToolBar->addAction(gMainWindow->mActions[LC_FILE_EXPORT_3DS]);
    exportToolBar->addAction(gMainWindow->mActions[LC_FILE_EXPORT_COLLADA]);
    exportToolBar->addAction(gMainWindow->mActions[LC_FILE_EXPORT_WAVEFRONT]);

    gMainWindow->mToolsToolBar->addAction(gMainWindow->mActions[LC_EDIT_UNDO]);
    gMainWindow->mToolsToolBar->addAction(gMainWindow->mActions[LC_EDIT_REDO]);
    gMainWindow->mToolsToolBar->addSeparator();
    gMainWindow->mToolsToolBar->addAction(gMainWindow->mActions[LC_EDIT_ACTION_SELECT]);
    gMainWindow->mToolsToolBar->addAction(gMainWindow->mActions[LC_EDIT_ACTION_ROTATE]);
    gMainWindow->mToolsToolBar->addAction(gMainWindow->mActions[LC_EDIT_ACTION_ROTATESTEP]);
    gMainWindow->mToolsToolBar->addAction(createBuildModAct);
    gMainWindow->mToolsToolBar->addAction(applyCameraAct);
    gMainWindow->mToolsToolBar->addAction(gMainWindow->mActions[LC_EDIT_ACTION_CAMERA]);
    gMainWindow->mToolsToolBar->addAction(LightGroupAct);
    gMainWindow->mToolsToolBar->addSeparator();
    gMainWindow->mToolsToolBar->addAction(ViewpointGroupAct);
//    gMainWindow->mToolsToolBar->addAction(gMainWindow->mActions[LC_VIEW_ZOOM_EXTENTS]);
    gMainWindow->mToolsToolBar->addAction(gMainWindow->mActions[LC_VIEW_LOOK_AT]);
    gMainWindow->mToolsToolBar->addAction(gMainWindow->mActions[LC_EDIT_ACTION_ZOOM]);
    gMainWindow->mToolsToolBar->addAction(gMainWindow->mActions[LC_EDIT_ACTION_PAN]);
    gMainWindow->mToolsToolBar->addAction(gMainWindow->mActions[LC_EDIT_ACTION_ROTATE_VIEW]);
    gMainWindow->mToolsToolBar->addAction(gMainWindow->mActions[LC_EDIT_ACTION_ZOOM_REGION]);
    gMainWindow->mToolsToolBar->addSeparator();
    gMainWindow->mToolsToolBar->addAction(gMainWindow->mActions[LC_EDIT_TRANSFORM_RELATIVE]);
    gMainWindow->mToolsToolBar->addAction(MoveAction);
    gMainWindow->mToolsToolBar->addAction(AngleAction); // Snap Rotations to Fixed Intervals menu item
    gMainWindow->mPartsToolBar->setWindowTitle("Tools Toolbar");
}

void Gui::Disable3DActions()
{
    applyCameraAct->setEnabled(false);
    createBuildModAct->setEnabled(false);
    LightGroupAct->setEnabled(false);
    ViewpointGroupAct->setEnabled(false);
}

void Gui::Enable3DActions()
{
    applyCameraAct->setEnabled(true);
    createBuildModAct->setEnabled(buildModRange.first() || hasBuildMods());
    applyBuildModAct->setEnabled(hasBuildMods());
    removeBuildModAct->setEnabled(hasBuildMods());
    LightGroupAct->setEnabled(true);
    ViewpointGroupAct->setEnabled(true);
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
                   Camera->SetAngles(Latitude,Longitude,Distance,Target, ActiveModel->GetCurrentStep(), false);
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

void Gui::groupActionTriggered()
{
    QAction* Action = static_cast<QAction*>(sender());
    lcCommandId commandId;
    QString commandTip;
    bool ok;

    if (Action == ViewpointZoomExtAct)
    {
        lcSetProfileInt(LC_PROFILE_VIEWPOINT_ZOOM_EXTENT, ViewpointZoomExtAct->isChecked());
    }
    else if (Action == ViewpointGroupAct)
    {
        commandId = lcCommandId(ViewpointGroupAct->property("CommandId").toInt(&ok));
        if (ok)
            gMainWindow->mActions[commandId]->trigger();
    }
    else if (Action == LightGroupAct)
    {
        commandId = lcCommandId(LightGroupAct->property("CommandId").toInt(&ok));
        if (ok)
            gMainWindow->mActions[commandId]->trigger();
    }
    else {
        commandId = lcCommandId(Action->property("CommandId").toInt(&ok));
        commandTip = Action->property("CommandTip").toString();
        if (ok)
        {
            if (commandId == LC_EDIT_ACTION_LIGHT ||
                commandId == LC_EDIT_ACTION_SUNLIGHT ||
                commandId == LC_EDIT_ACTION_SPOTLIGHT ||
                commandId == LC_EDIT_ACTION_AREALIGHT) {
                LightGroupAct->setIcon(gMainWindow->mActions[commandId]->icon());
                LightGroupAct->setToolTip(tr(commandTip.toLatin1()));
                LightGroupAct->setStatusTip(gMainWindow->mActions[commandId]->statusTip());
                LightGroupAct->setProperty("CommandId", QVariant(commandId));
            } else {
                ViewpointGroupAct->setIcon(gMainWindow->mActions[commandId]->icon());
                ViewpointGroupAct->setToolTip(tr(commandTip.toLatin1()));
                ViewpointGroupAct->setStatusTip(gMainWindow->mActions[commandId]->statusTip());
                ViewpointGroupAct->setProperty("CommandId", QVariant(commandId));
            }
        }
    }
}

void Gui::restoreLightAndViewpointDefaults(){
    LightGroupAct->setToolTip(tr("Lights - Pointlight"));
    LightGroupAct->setIcon(gMainWindow->mActions[LC_EDIT_ACTION_LIGHT]->icon());
    LightGroupAct->setStatusTip(gMainWindow->mActions[LC_EDIT_ACTION_LIGHT]->statusTip());
    LightGroupAct->setProperty("CommandId", QVariant(LC_EDIT_ACTION_LIGHT));

    ViewpointGroupAct->setToolTip(tr("Viewpoints - Home"));
    ViewpointGroupAct->setIcon(gMainWindow->mActions[LC_VIEW_VIEWPOINT_HOME]->icon());
    ViewpointGroupAct->setStatusTip(gMainWindow->mActions[LC_VIEW_VIEWPOINT_HOME]->statusTip());
    ViewpointGroupAct->setProperty("CommandId", QVariant(LC_VIEW_VIEWPOINT_HOME));
}

void Gui::enableBuildModification()
{
    if (sender() == enableBuildModAct)
      lcSetProfileInt(LC_PROFILE_BUILD_MODIFICATION, enableBuildModAct->isChecked());
    else
      lcSetProfileInt(LC_PROFILE_BUILD_MODIFICATION, !enableRotstepRotateAct->isChecked());

    QIcon RotateIcon;
    if (lcGetProfileInt(LC_PROFILE_BUILD_MODIFICATION)) {
        enableBuildModAct->setChecked(true);
        enableRotstepRotateAct->setChecked(false);
        RotateIcon.addFile(":/resources/rotatebuildmod.png");
        RotateIcon.addFile(":/resources/rotatebuildmod16.png");
    } else {
        enableBuildModAct->setChecked(false);
        enableRotstepRotateAct->setChecked(true);
        RotateIcon.addFile(":/resources/rotaterotstep.png");
        RotateIcon.addFile(":/resources/rotaterotstep16.png");
    }
    gMainWindow->mActions[LC_EDIT_ACTION_ROTATE]->setIcon(RotateIcon);
    gMainWindow->mActions[LC_EDIT_ACTION_ROTATESTEP]->setEnabled(enableRotstepRotateAct->isChecked());
}

void Gui::setTargetPosition()
{
  lcSetProfileInt(LC_PROFILE_SET_TARGET_POSITION, setTargetPositionAct->isChecked());
  QIcon ApplyCameraIcon;
  if (setTargetPositionAct->isChecked()) {
      ApplyCameraIcon.addFile(":/resources/applycamerasettingsposition.png");
      ApplyCameraIcon.addFile(":/resources/applycamerasettingsposition_16.png");
      applyCameraAct->setIcon(ApplyCameraIcon);
  } else {
      ApplyCameraIcon.addFile(":/resources/applycamerasettings.png");
      ApplyCameraIcon.addFile(":/resources/applycamerasettings_16.png");
      applyCameraAct->setIcon(ApplyCameraIcon);
  }
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

int Gui::GetImageWidth()
{
    return lcGetActiveProject()->GetImageWidth();
}

int Gui::GetImageHeight()
{
    return lcGetActiveProject()->GetImageHeight();
}

void Gui::saveCurrent3DViewerModel(const QString &modelFile)
{
    View* ActiveView     = gMainWindow->GetActiveView();
    lcModel* ActiveModel = ActiveView->GetActiveModel();

    if (ActiveModel){
        // Create a copy of the current camera and add it cameras
        lcCamera* Camera = ActiveView->mCamera;
        Camera->CreateName(ActiveModel->mCameras);
        Camera->SetSelected(true);
        ActiveModel->mCameras.Add(ActiveView->mCamera);

        // Get the created camera name
        char cameraName[81];
        const char* Prefix = "Camera ";
        sprintf(cameraName, "%s %d", Prefix, ActiveModel->GetCameras().GetSize());

        // Set the created camera
        ActiveView->SetCamera(cameraName/*cameraName.toLatin1().constData()*/);

        // Save the current model
        if (!lcGetActiveProject()->Save(modelFile))
            emit gui->messageSig(LOG_ERROR, QString("Failed to save current model to file [%1]").arg(modelFile));

        // Reset the camera
        bool RemovedCamera = false;
        for (int CameraIdx = 0; CameraIdx < ActiveModel->GetCameras().GetSize(); )
        {
            if (!strcmp(ActiveModel->mCameras[CameraIdx]->m_strName, cameraName))
            {
                RemovedCamera = true;
                ActiveModel->mCameras.RemoveIndex(CameraIdx);
            }
            else
                CameraIdx++;
        }

        ActiveView->SetCamera(Camera, true);
        strcpy(ActiveView->mCamera->m_strName, "");

        if (RemovedCamera)
            gMainWindow->UpdateCameraMenu();

        Camera = nullptr;
    }
}
/*********************************************
 *
 * build modificaitons
 *
 ********************************************/

void Gui::createBuildModification()
{
    View* ActiveView = gMainWindow->GetActiveView();
    lcModel* ActiveModel = ActiveView->GetActiveModel();

    if (ActiveModel) {
        if (buildModRange.first()){

            lcArray<lcGroup*>  mGroups;
            lcArray<lcCamera*> mCameras;
            lcArray<lcPiece*>  mLoadPieces;
            lcArray<lcGroup*>  mLoadGroups;
            lcArray<lcPiece*>  mSavePieces;
            lcArray<lcLight*>  mSaveLights;
            lcArray<lcGroup*>  mSaveGroups;
            lcModelProperties  mSaveProperties;
            lcArray<lcPieceControlPoint> ControlPoints; // empty placeholder
            QStringList  mLoadFileLines, mSaveFileLines,
                         DefaultContents, BuildModContents;

            int CurrentStep    = 1;
            int ModActionLineNum = 0;
            bool BuildModPart  = false;
            lcPiece *Piece     = nullptr;
            lcCamera *Camera   = nullptr;
            mSaveProperties    = ActiveModel->GetProperties();
            mSaveFileLines     = ActiveModel->mFileLines;
            mSavePieces        = ActiveModel->GetPieces();
            mSaveLights        = ActiveModel->mLights;
            lcPiecesLibrary *Library = lcGetPiecesLibrary();

            int ModelIndex      = buildModRange.at(BM_MODEL_INDEX);
            int ModBeginLineNum = buildModRange.at(BM_BEGIN_LINE);
            int ModEndLineNum   = buildModRange.at(BM_END_LINE);
            QString ModelName   = getSubmodelName(ModelIndex);

            bool FadeStep = page.meta.LPub.fadeStep.fadeStep.value();
            bool HighlightStep = page.meta.LPub.highlightStep.highlightStep.value() && !suppressColourMeta();

            auto GetGroup = [&mGroups](const QString& Name, bool CreateIfMissing)
            {
                lcGroup* nullGroup = nullptr;
                for (lcGroup* Group : mGroups)
                    if (Group->mName == Name)
                        return Group;

                if (CreateIfMissing)
                {
                    lcGroup* Group = new lcGroup();
                    Group->mName = Name;
                    mGroups.Add(Group);

                    return Group;
                }

                return nullGroup;
            };

            auto ReconfigurePart = [&ModelIndex, &FadeStep, &HighlightStep, &DefaultContents] (int LineTypeIndex, QString &PartLine)
            {
                QString NameMod, ColourPrefix;
                if (FadeStep || HighlightStep){

                    if (FadeStep) {
                        NameMod = FADE_SFX;
                        ColourPrefix = LPUB3D_COLOUR_FADE_PREFIX;
                    } else if (HighlightStep) {
                        NameMod = HIGHLIGHT_SFX;
                        ColourPrefix = LPUB3D_COLOUR_HIGHLIGHT_PREFIX;
                    }

                    QStringList argv;
                    split(PartLine, argv);
                    if (argv.size() == 15 && argv[0] == "1") {

                        // Colour code
                        if (argv[1].startsWith(ColourPrefix)) {
                            int LineNumber;
                            QString DefaultPartLine, NewColorCode;
                            bool NewLine = !gui->getSelectedLine(ModelIndex, LineTypeIndex, VIEWER_MOD, LineNumber);

                            if (NewLine) {
                                NewColorCode = argv[1];

                            } else {
                                DefaultPartLine = DefaultContents.at(LineTypeIndex);
                                QStringList dargv;
                                split(DefaultPartLine, dargv);

                                if (dargv.size() == 15 && dargv[0] == "1") {
                                    NewColorCode = dargv[1];

                                } else {
                                    NewColorCode = argv[1].right(argv[1].size() - ColourPrefix.size());
                                    if (FadeStep && Preferences::fadeStepsUseColour)
                                        NewColorCode = LDRAW_MAIN_MATERIAL_COLOUR;
                                }
                            }

                            argv[1] = NewColorCode;
                        }

                        // fileName
                        QString FileName = QString(argv[argv.size()-1]).toLower();
                        if (FileName.contains(NameMod)) {

                            QString NewFileName = FileName.replace(NameMod,"");
                            argv[argv.size()-1] = NewFileName;
                        }

                        PartLine = argv.join(" ");
                    }
                }
            };

            auto AddPart = [&BuildModPart, &ReconfigurePart] (int LineTypeIndex, QTextStream &Stream)
            {
                QString PartLine;

                Stream >> PartLine;

                if (BuildModPart) {
                    ReconfigurePart(LineTypeIndex, PartLine);

                    Stream << PartLine;
                }
            };

            auto InsertPiece = [&mLoadPieces] (lcPiece* Piece, int Index)
            {
                PieceInfo* Info = Piece->mPieceInfo;

                if (!Info->IsModel())
                {
                    lcMesh* Mesh = Info->IsTemporary() ? gPlaceholderMesh : Info->GetMesh();

                    if (Mesh && Mesh->mVertexCacheOffset == -1)
                        lcGetPiecesLibrary()->mBuffersDirty = true;
                }

                mLoadPieces.InsertAt(Index, Piece);
            };

            auto AddPiece = [&mLoadPieces, &InsertPiece](lcPiece* Piece)
            {
                for (int PieceIdx = 0; PieceIdx < mLoadPieces.GetSize(); PieceIdx++)
                {
                    if (mLoadPieces[PieceIdx]->GetStepShow() > Piece->GetStepShow())
                    {
                        InsertPiece(Piece, PieceIdx);
                        return;
                    }
                }

                InsertPiece(Piece, mLoadPieces.GetSize());
            };

            // capture default content for ldrawFile.buildMod and QBA
            QByteArray QBA;
            for (int i = 0; i < ldrawFile.contents(ModelName).size(); i++){
                if (i > ModEndLineNum)
                    break;
                QString ModLine = ldrawFile.contents(ModelName).at(i);
                if (i >= ModBeginLineNum) {
                    DefaultContents.append(ModLine);
                }
                QBA.append(ModLine);
                QBA.append(QString("\n"));
            }
            QBuffer Buffer(&QBA);
            Buffer.open(QIODevice::ReadOnly);
            Buffer.seek(0);

            // load mLoadFileLines, mLoadGroups and mLoadPieces with LDraw content using QBA buffer
            while (!Buffer.atEnd())
            {
                qint64 Pos = Buffer.pos();
                QString OriginalLine = Buffer.readLine();
                QString Line = OriginalLine.trimmed();
                QTextStream LineStream(&Line, QIODevice::ReadOnly);

                QString Token;
                LineStream >> Token;

                if (Token == QLatin1String("0"))
                {
                    LineStream >> Token;
                    if (Token == QLatin1String("FILE"))
                    {
                        QString Name = LineStream.readAll().trimmed();

                        if (mSaveProperties.mName != Name)
                        {
                            Buffer.seek(Pos);
                            break;
                        }

                        continue;
                    }
                    else if (Token == QLatin1String("NOFILE"))
                    {
                        break;
                    }
                    else if (Token == QLatin1String("STEP"))
                    {
                        CurrentStep++;
                        mLoadFileLines.append(OriginalLine);
                        continue;
                    }

                    if (Token != QLatin1String("!LEOCAD"))
                    {
                        mLoadFileLines.append(OriginalLine);
                        continue;
                    }

                    LineStream >> Token;

                    if (Token == QLatin1String("MODEL"))
                    {
                        mSaveProperties.ParseLDrawLine(LineStream);
                    }
                    else if (Token == QLatin1String("PIECE"))
                    {
                        if (!Piece)
                            Piece = new lcPiece(nullptr);

                        Piece->ParseLDrawLine(LineStream);
                    }
                    else if (Token == QLatin1String("CAMERA"))
                    {
                        if (!Camera)
                            Camera = new lcCamera(false);

                        if (Camera->ParseLDrawLine(LineStream))
                        {
                            Camera->CreateName(mCameras);
                            mCameras.Add(Camera);
                            Camera = nullptr;
                        }
                    }
                    else if (Token == QLatin1String("GROUP"))
                    {
                        LineStream >> Token;

                        if (Token == QLatin1String("BEGIN"))
                        {
                            QString Name = LineStream.readAll().trimmed();
                            lcGroup* Group = GetGroup(Name, true);
                            if (!mLoadGroups.IsEmpty())
                                Group->mGroup = mLoadGroups[mLoadGroups.GetSize() - 1];
                            else
                                Group->mGroup = nullptr;
                            mLoadGroups.Add(Group);
                        }
                        else if (Token == QLatin1String("END"))
                        {
                            if (!mLoadGroups.IsEmpty())
                                mLoadGroups.RemoveIndex(mLoadGroups.GetSize() - 1);
                        }
                    }
                    else if (Token == QLatin1String("SYNTH"))
                    {
                        LineStream >> Token;

                        if (Token == QLatin1String("BEGIN"))
                        {
                            ControlPoints.RemoveAll();
                        }
                        else if (Token == QLatin1String("END"))
                        {
                            ControlPoints.RemoveAll();
                        }
                        else if (Token == QLatin1String("CONTROL_POINT"))
                        {
                            float Numbers[13];
                            for (int TokenIdx = 0; TokenIdx < 13; TokenIdx++)
                                LineStream >> Numbers[TokenIdx];

                            lcPieceControlPoint& PieceControlPoint = ControlPoints.Add();
                            PieceControlPoint.Transform = lcMatrix44(lcVector4(Numbers[3], Numbers[9], -Numbers[6], 0.0f), lcVector4(Numbers[5], Numbers[11], -Numbers[8], 0.0f),
                                                                     lcVector4(-Numbers[4], -Numbers[10], Numbers[7], 0.0f), lcVector4(Numbers[0], Numbers[2], -Numbers[1], 1.0f));
                            PieceControlPoint.Scale = Numbers[12];
                        }
                    }

                    continue;

                }
                else if (Token == QLatin1String("1"))
                {
                    int ColorCode;
                    LineStream >> ColorCode;
                    BuildModPart = false;
                    QString EmptyLine = QString();
                    QTextStream EmptyStream(&EmptyLine, QIODevice::ReadOnly);

                    float IncludeMatrix[12];
                    for (int TokenIdx = 0; TokenIdx < 12; TokenIdx++)
                        LineStream >> IncludeMatrix[TokenIdx];

                    lcMatrix44 IncludeTransform(lcVector4(IncludeMatrix[3], IncludeMatrix[6], IncludeMatrix[9], 0.0f), lcVector4(IncludeMatrix[4], IncludeMatrix[7], IncludeMatrix[10], 0.0f),
                            lcVector4(IncludeMatrix[5], IncludeMatrix[8], IncludeMatrix[11], 0.0f), lcVector4(IncludeMatrix[0], IncludeMatrix[1], IncludeMatrix[2], 1.0f));

                    QString PartId = LineStream.readAll().trimmed();
                    QByteArray CleanId = PartId.toLatin1().toUpper().replace('\\', '/');

                    if (Library->IsPrimitive(CleanId.constData()))
                    {
                        mLoadFileLines.append(OriginalLine);
                    }
                    else
                    {
                        if (!Piece)
                            Piece = new lcPiece(nullptr);

                        if (!mLoadGroups.IsEmpty())
                            Piece->SetGroup(mLoadGroups[mLoadGroups.GetSize() - 1]);

                        PieceInfo* Info = Library->FindPiece(PartId.toLatin1().constData(), lcGetActiveProject(), true, true);

                        float* Matrix = IncludeTransform;
                        lcMatrix44 Transform(lcVector4(Matrix[0], Matrix[2], -Matrix[1], 0.0f), lcVector4(Matrix[8], Matrix[10], -Matrix[9], 0.0f),
                                lcVector4(-Matrix[4], -Matrix[6], Matrix[5], 0.0f), lcVector4(Matrix[12], Matrix[14], -Matrix[13], 1.0f));

                        Piece->SetFileLine(mLoadFileLines.size());
                        Piece->SetPieceInfo(Info, PartId, false);
                        Piece->Initialize(Transform, quint32(CurrentStep));
                        Piece->SetColorCode(quint32(ColorCode));
                        Piece->SetControlPoints(ControlPoints);
                        AddPiece(Piece);
                        AddPart(Piece->GetLineTypeIndex(), EmptyStream);

                        Piece = nullptr;
                    }
                }
                else
                    mLoadFileLines.append(OriginalLine);

                Library->WaitForLoadQueue();
                Library->mBuffersDirty = true;
                Library->UnloadUnusedParts();

                delete Piece;
                delete Camera;
            }

            // Save 3DViewer content to buildModContent
            bool SelectedOnly = false;
            QLatin1String LineEnding("\r\n");

            QBA.clear();
            QTextStream Stream(&QBA);

            lcStep Step        = 1;
            int CurrentLine    = ModBeginLineNum/*0*/;
            int AddedSteps     = 0;

            int LineIndex      = -1;
            int LineNumber     = 0;
            int SaveLineNumber = 0;
            bool NewLine       = false;

            for (lcPiece* Piece : mSavePieces)
            {
                BuildModPart   = true;
                SaveLineNumber = LineNumber;
                LineIndex      = Piece->GetLineTypeIndex();
                NewLine        = !getSelectedLine(ModelIndex, LineIndex, VIEWER_MOD, LineNumber);
                Q_UNUSED(NewLine);

                if (LineNumber > ModEndLineNum)
                    break;

                if (LineNumber >= ModBeginLineNum) {

                    while (LineNumber/*Piece->GetFileLine()*/ > CurrentLine && CurrentLine < mLoadFileLines.size()/*mSaveFileLines.size()*/)
                    {
                        QString Line = mLoadFileLines[CurrentLine]/*mSaveFileLines[CurrentLine]*/;
                        QTextStream LineStream(&Line, QIODevice::ReadOnly);

                        QString Token;
                        LineStream >> Token;
                        bool Skip = false;

                        if (Token == QLatin1String("0"))
                        {
                            LineStream >> Token;

                            if (Token == QLatin1String("STEP"))
                            {
                                if (Piece->GetStepShow() > Step)
                                    Step++;
                                else
                                    Skip = true;
                            }
                        }

                        if (!Skip)
                        {
                            Stream << mLoadFileLines[CurrentLine]/*mSaveFileLines[CurrentLine]*/;
                            if (AddedSteps > 0)
                                AddedSteps--;
                        }
                        CurrentLine++;
                        QString foo = QString();
                    }

                    while (Piece->GetStepShow() > Step)
                    {
                        Stream << QLatin1String("0 STEP\r\n");
                        AddedSteps++;
                        Step++;
                    }

                    lcGroup* PieceGroup = Piece->GetGroup();

                    if (PieceGroup)
                    {
                        if (mSaveGroups.IsEmpty() || (!mSaveGroups.IsEmpty() && PieceGroup != mSaveGroups[mSaveGroups.GetSize() - 1]))
                        {
                            lcArray<lcGroup*> PieceParents;

                            for (lcGroup* Group = PieceGroup; Group; Group = Group->mGroup)
                                PieceParents.InsertAt(0, Group);

                            int FoundParent = -1;

                            while (!mSaveGroups.IsEmpty())
                            {
                                lcGroup* Group = mSaveGroups[mSaveGroups.GetSize() - 1];
                                int Index = PieceParents.FindIndex(Group);

                                if (Index == -1)
                                {
                                    mSaveGroups.RemoveIndex(mSaveGroups.GetSize() - 1);
                                    Stream << QLatin1String("0 !LEOCAD GROUP END\r\n");
                                }
                                else
                                {
                                    FoundParent = Index;
                                    break;
                                }
                            }

                            for (int ParentIdx = FoundParent + 1; ParentIdx < PieceParents.GetSize(); ParentIdx++)
                            {
                                lcGroup* Group = PieceParents[ParentIdx];
                                mSaveGroups.Add(Group);
                                Stream << QLatin1String("0 !LEOCAD GROUP BEGIN ") << Group->mName << LineEnding;
                            }
                        }
                    }
                    else
                    {
                        while (mSaveGroups.GetSize())
                        {
                            mSaveGroups.RemoveIndex(mSaveGroups.GetSize() - 1);
                            Stream << QLatin1String("0 !LEOCAD GROUP END\r\n");
                        }
                    }

                    if (Piece->mPieceInfo->GetSynthInfo())
                    {
                        Stream << QLatin1String("0 !LEOCAD SYNTH BEGIN\r\n");

                        const lcArray<lcPieceControlPoint>& ControlPoints = Piece->GetControlPoints();
                        for (int ControlPointIdx = 0; ControlPointIdx < ControlPoints.GetSize(); ControlPointIdx++)
                        {
                            const lcPieceControlPoint& ControlPoint = ControlPoints[ControlPointIdx];

                            Stream << QLatin1String("0 !LEOCAD SYNTH CONTROL_POINT");

                            const float* FloatMatrix = ControlPoint.Transform;
                            float Numbers[13] = { FloatMatrix[12], -FloatMatrix[14], FloatMatrix[13], FloatMatrix[0], -FloatMatrix[8], FloatMatrix[4], -FloatMatrix[2], FloatMatrix[10], -FloatMatrix[6], FloatMatrix[1], -FloatMatrix[9], FloatMatrix[5], ControlPoint.Scale };

                            for (int NumberIdx = 0; NumberIdx < 13; NumberIdx++)
                                Stream << ' ' << lcFormatValue(Numbers[NumberIdx], NumberIdx < 3 ? 4 : 6);

                            Stream << LineEnding;
                        }
                    }

                    AddPart(Piece->GetLineTypeIndex(), Stream);

                    if (Piece->mPieceInfo->GetSynthInfo())
                        Stream << QLatin1String("0 !LEOCAD SYNTH END\r\n");
                }
            }

            while (CurrentLine < mLoadFileLines.size()/*mSaveFileLines.size()*/)
            {
                QString Line = mLoadFileLines[CurrentLine]/*mSaveFileLines[CurrentLine]*/;
                QTextStream LineStream(&Line, QIODevice::ReadOnly);

                QString Token;
                LineStream >> Token;
                bool Skip = false;

                if (Token == QLatin1String("0"))
                {
                    LineStream >> Token;

                    if (Token == QLatin1String("STEP") && AddedSteps-- > 0)
                        Skip = true;
                }

                if (!Skip)
                    Stream << mLoadFileLines[CurrentLine]/*mSaveFileLines[CurrentLine]*/;
                CurrentLine++;
            }

            while (mSaveGroups.GetSize())
            {
                mSaveGroups.RemoveIndex(mSaveGroups.GetSize() - 1);
                Stream << QLatin1String("0 !LEOCAD GROUP END\r\n");
            }

            for (lcCamera* Camera : mCameras)
                if (!SelectedOnly || Camera->IsSelected())
                    Camera->SaveLDraw(Stream);

            for (lcLight* Light : mSaveLights)
                if (!SelectedOnly || Light->IsSelected())
                    Light->SaveLDraw(Stream);

            Stream.flush();

            BuildModContents = QString(QBA).split(QRegExp("(\\r\\n)|\\r|\\n"), QString::SkipEmptyParts);

            QString BuildModKey = QString("%1 Mod %2").arg(ModelName).arg(getBuildModNextIndex(ModelName));

            Where modHere;

            if (currentStep && BuildModContents.size()) {
                int it = lcGetActiveProject()->GetImageType();
                switch(it){
                case Options::Mt::CSI:
                case Options::Mt::SMP:
                {
                    QString metaString;
                    BuildModData buildModData;
                    modHere = Where(ModelName, ModEndLineNum);

                    beginMacro("CreateBuildModContent");

                    // write end meta command below default content last line (append line)
                    buildModData.action      = QString("END");
                    buildModData.buildModKey = QString();
                    currentStep->buildMod.setValue(buildModData);
                    metaString = currentStep->buildMod.format(true,false);
                    gui->appendLine(modHere, metaString, nullptr);

                    // write action meta command above default content first line - last to first
                    modHere = Where(ModelName, ModBeginLineNum);
                    buildModData.action      = QString("END_MOD");
                    buildModData.buildModKey = QString();
                    currentStep->buildMod.setValue(buildModData);
                    metaString = currentStep->buildMod.format(true,false);
                    gui->insertLine(modHere, metaString, nullptr);

                    // write buildMod content last to first
                    modHere.lineNumber = BuildModContents.size();
                    while (modHere.lineNumber > 0) {
                       gui->insertLine(modHere, metaString, nullptr);
                       modHere--;
                    }

                    // write begin meta command above buildMod content first line
                    ModBeginLineNum          = modHere.lineNumber;
                    buildModData.action      = QString("BEGIN");
                    buildModData.buildModKey = BuildModKey;
                    currentStep->buildMod.setValue(buildModData);
                    metaString = currentStep->buildMod.format(true,false);
                    gui->insertLine(modHere, metaString, nullptr);

                    endMacro();
                }
                    break;
                default: /*Options::Mt::PLI:*/
                    break;
                }
            }

            QVector<int> ModAttributes = { ModBeginLineNum,
                                           ModActionLineNum,
                                           ModEndLineNum,
                                           ModelIndex};

            insertBuildMod(BuildModKey,
                           ModAttributes,
                           BuildModApplyRc,
                           currentStep ? currentStep->stepNumber.number : 0);

            // reset the modified parts list
            buildModRange = { 0, -1, 0 };
        }
    }
}

void Gui::applyBuildModification()
{
    QStringList buildModKeys;
    BuildModDialogGui *buildModDialogGui =
            new BuildModDialogGui();
    buildModDialogGui->getBuildMod(buildModKeys);

    if (buildModKeys.size() && currentStep){
        if (lcGetActiveProject()->GetImageType() != Options::Mt::PLI) {
            QString metaString;
            BuildModData buildModData;
            Where top = currentStep->topOfStep();
            int stepNumber = currentStep->stepNumber.number;

            gui->setBuildModAction(buildModKeys.first(), stepNumber, BuildModApplyRc);

            beginMacro("ApplyBuildModContent");

            buildModData.action      = QString("APPLY");
            buildModData.buildModKey = buildModKeys.first();
            currentStep->buildMod.setValue(buildModData);
            metaString = currentStep->buildMod.format(true,false);
            gui->insertLine(top, metaString, nullptr);

            if (currentStep->buildMod.here() == Where())
                gui->insertLine(top, metaString, nullptr);
            else
                gui->replaceLine(top, metaString, nullptr);

            endMacro();
        }
    }
}

void Gui::removeBuildModification()
{
    QStringList buildModKeys;
    BuildModDialogGui *buildModDialogGui =
            new BuildModDialogGui();
    buildModDialogGui->getBuildMod(buildModKeys,false);

    if (buildModKeys.size() && currentStep){
        if (lcGetActiveProject()->GetImageType() != Options::Mt::PLI) {
            QString metaString;
            BuildModData buildModData;
            Where top = currentStep->topOfStep();
            int stepNumber = currentStep->stepNumber.number;

            gui->setBuildModAction(buildModKeys.first(), stepNumber, BuildModRemoveRc);

            beginMacro("RemoveBuildModContent");

            buildModData.action      = QString("REMOVE");
            buildModData.buildModKey = buildModKeys.first();
            currentStep->buildMod.setValue(buildModData);
            metaString = currentStep->buildMod.format(true,false);
            gui->insertLine(top, metaString, nullptr);

            if (currentStep->buildMod.here() == Where())
                gui->insertLine(top, metaString, nullptr);
            else
                gui->replaceLine(top, metaString, nullptr);

            endMacro();
        }
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
    // CSI: 0=modelNameIndex, 1=lineNumber,   2=stepNumber [_dm (displayModel)]
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
      // viewerStepKey elements CSI: 0=modelName, 1=lineNumber, 2=stepNumber [,3=_dm (displayModel)]
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
    bool stepMatch  = false;
    bool singleStep = false;
    bool multiStep  = isViewerStepMultiStep(viewerStepKey);
    bool calledOut  = isViewerStepCalledOut(viewerStepKey);

    auto calledOutStep = [this, &here, &stepNumber] (Step* step, bool &stepMatch)
    {
        if (! (stepMatch = step->stepNumber.number == stepNumber)) {
            for (int k = 0; k < step->list.size(); k++) {
                if (step->list[k]->relativeType == CalloutType) {
                    Callout *callout = dynamic_cast<Callout *>(step->list[k]);
                    for (int l = 0; l < callout->list.size(); l++){
                        Range *range = dynamic_cast<Range *>(callout->list[l]);
                        for (int m = 0; m < range->list.size(); m++){
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
    };

    if (calledOut && step){
        calledOutStep(step, stepMatch);
    } else {
        if ((multiStep = isViewerStepMultiStep(viewerStepKey))) {
            for (int i = 0; i < page.list.size() && !stepMatch; i++){
                Range *range = dynamic_cast<Range *>(page.list[i]);
                for (int j = 0; j < range->list.size(); j++){
                    if (range->relativeType == RangeType) {
                        step = dynamic_cast<Step *>(range->list[j]);
                        if (calledOut)
                            calledOutStep(step, stepMatch);
                        else if (step && step->relativeType == StepType)
                            stepMatch = step->stepNumber.number == stepNumber;
                        if (stepMatch)
                            break;
                    }
                }
            }
        } else if ((singleStep = page.relativeType == SingleStepType && page.list.size())) {
            Range *range = dynamic_cast<Range *>(page.list[0]);
            if (range->relativeType == RangeType) {
                step = dynamic_cast<Step *>(range->list[0]);
                if (calledOut)
                    calledOutStep(step, stepMatch);
                else if (step && step->relativeType == StepType)
                    stepMatch = step->stepNumber.number == stepNumber;
            }
        } else if ((step = gStep)) {
            if (calledOut)
                calledOutStep(step, stepMatch);
            else if (!(stepMatch = step->stepNumber.number == stepNumber))
                step = nullptr;
        }
    }

    currentStep = step;

    if (Preferences::debugLogging && !stepMatch)
        emit messageSig(LOG_DEBUG, QString("%1 Step number %2 for %3 - modelName [%4] topOfStep [%5]")
                                           .arg(stepMatch ? "Match!" : "Oh oh!")
                                           .arg(QString("%1 %2").arg(stepNumber).arg(stepMatch ? "found" : "not found"))
                                           .arg(multiStep ? "multi step" : calledOut ? "called out" : singleStep ? "single step" : "gStep")
                                           .arg(here.modelName).arg(here.lineNumber));
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
        int lineNumber    = 0;
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
                if (fromViewer && source == VIEWER_MOD) {
                    QVector<int> modAttributes = { lineNumber, modelIndex, lineNumber};
                    if (buildModRange.first()) {
                        buildModRange[BM_MODEL_INDEX] = modelIndex;
                        if (lineNumber < buildModRange.first())
                            buildModRange[BM_BEGIN_LINE] = lineNumber;
                        else if (lineNumber > buildModRange.last())
                            buildModRange[BM_END_LINE] = lineNumber;     // NOT USED - Can remove
                    } else {
                        buildModRange = { lineNumber, modelIndex, lineNumber};
                    }
                    createBuildModAct->setEnabled(true);
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
