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
#include "paths.h"

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
    createBuildModAct->setStatusTip(tr("Create a new build modification for this step - Shift+J"));
    createBuildModAct->setShortcut(tr("Shift+J"));
    connect(createBuildModAct, SIGNAL(triggered()), this, SLOT(createBuildModification()));

    QIcon UpdateBuildModIcon;
    UpdateBuildModIcon.addFile(":/resources/buildmodupdate.png");
    UpdateBuildModIcon.addFile(":/resources/buildmodupdate16.png");
    updateBuildModAct = new QAction(UpdateBuildModIcon,tr("Update Build Modification"),this);
    updateBuildModAct->setStatusTip(tr("Commit changes to the current build modification - Shift+K"));
    updateBuildModAct->setShortcut(tr("Shift+K"));
    connect(updateBuildModAct, SIGNAL(triggered()), this, SLOT(updateBuildModification()));

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

    QIcon LoadBuildModIcon;
    LoadBuildModIcon.addFile(":/resources/buildmodload.png");
    LoadBuildModIcon.addFile(":/resources/buildmodload16.png");
    loadBuildModAct = new QAction(LoadBuildModIcon,tr("Load Build Modification..."),this);
    loadBuildModAct->setEnabled(false);
    loadBuildModAct->setStatusTip(tr("Load the step containing the selected build modification"));
    connect(loadBuildModAct, SIGNAL(triggered()), this, SLOT(loadBuildModification()));

    QIcon DeleteBuildModIcon;
    DeleteBuildModIcon.addFile(":/resources/buildmoddelete.png");
    DeleteBuildModIcon.addFile(":/resources/buildmoddelete16.png");
    deleteBuildModAct = new QAction(DeleteBuildModIcon,tr("Delete Build Modification..."),this);
    deleteBuildModAct->setEnabled(false);
    deleteBuildModAct->setStatusTip(tr("Delete selected build modification meta commands"));
    connect(deleteBuildModAct, SIGNAL(triggered()), this, SLOT(deleteBuildModification()));

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

    QIcon ApplyCameraIcon;
    ApplyCameraIcon.addFile(":/resources/applycamerasettings.png");
    ApplyCameraIcon.addFile(":/resources/applycamerasettings_16.png");
    applyCameraAct = new QAction(ApplyCameraIcon,tr("Save Camera Settings"),this);
    applyCameraAct->setStatusTip(tr("Save current camera settings to current step and regenerate image - Shift+A"));
    applyCameraAct->setShortcut(tr("Shift+A"));
    connect(applyCameraAct, SIGNAL(triggered()), this, SLOT(applyCameraSettings()));

    QIcon ApplyLightIcon;
    ApplyLightIcon.addFile(":/resources/applylightsettings.png");
    ApplyLightIcon.addFile(":/resources/applylightsettings_16.png");
    applyLightAct = new QAction(ApplyLightIcon,tr("Save Light Settings"),this);
    applyLightAct->setEnabled(false);
    applyLightAct->setStatusTip(tr("Save light settings to current step - Shift+I"));
    applyLightAct->setShortcut(tr("Shift+I"));
    connect(applyLightAct, SIGNAL(triggered()), this, SLOT(applyLightSettings()));

    useImageSizeAct = new QAction(tr("Use Image Size"),this);
    useImageSizeAct->setStatusTip(tr("Use image width and height - you can also edit width and height in camera Properties tab"));
    useImageSizeAct->setCheckable(true);
    useImageSizeAct->setChecked(lcGetProfileInt(LC_PROFILE_USE_IMAGE_SIZE));
    connect(useImageSizeAct, SIGNAL(triggered()), this, SLOT(useImageSize()));

    autoCenterSelectionAct = new QAction(tr("Look At Selection"),this);
    autoCenterSelectionAct->setStatusTip(tr("Automatically rotate view so selected pieces are at center"));
    autoCenterSelectionAct->setCheckable(true);
    autoCenterSelectionAct->setChecked(lcGetProfileInt(LC_PROFILE_AUTO_CENTER_SELECTION));
    connect(autoCenterSelectionAct, SIGNAL(triggered()), this, SLOT(autoCenterSelection()));

    defaultCameraPropertiesAct = new QAction(tr("Default Properties"),this);
    defaultCameraPropertiesAct->setStatusTip(tr("Display default camera properties in Properties tab"));
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
    lightGroupAct = new QAction(tr("Lights"), this);
    lightGroupAct->setToolTip(tr("Lights - Pointlight"));
    lightGroupAct->setIcon(gMainWindow->mActions[LC_EDIT_ACTION_LIGHT]->icon());
    lightGroupAct->setStatusTip(gMainWindow->mActions[LC_EDIT_ACTION_LIGHT]->statusTip());
    lightGroupAct->setProperty("CommandId", QVariant(LC_EDIT_ACTION_LIGHT));
    gMainWindow->mActions[LC_EDIT_ACTION_LIGHT]->setProperty("CommandId", QVariant(LC_EDIT_ACTION_LIGHT));
    gMainWindow->mActions[LC_EDIT_ACTION_SUNLIGHT]->setProperty("CommandId", QVariant(LC_EDIT_ACTION_SUNLIGHT));
    gMainWindow->mActions[LC_EDIT_ACTION_SPOTLIGHT]->setProperty("CommandId", QVariant(LC_EDIT_ACTION_SPOTLIGHT));
    gMainWindow->mActions[LC_EDIT_ACTION_AREALIGHT]->setProperty("CommandId", QVariant(LC_EDIT_ACTION_AREALIGHT));
    gMainWindow->mActions[LC_EDIT_ACTION_LIGHT]->setProperty("CommandTip", QVariant("Lights - Pointlight"));
    gMainWindow->mActions[LC_EDIT_ACTION_SUNLIGHT]->setProperty("CommandTip", QVariant("Lights - Sunlight"));
    gMainWindow->mActions[LC_EDIT_ACTION_SPOTLIGHT]->setProperty("CommandTip", QVariant("Lights - Spotlight"));
    gMainWindow->mActions[LC_EDIT_ACTION_AREALIGHT]->setProperty("CommandTip", QVariant("Lights - Arealight"));
    connect(lightGroupAct, SIGNAL(triggered()), this, SLOT(groupActionTriggered()));
    connect(gMainWindow->mActions[LC_EDIT_ACTION_LIGHT], SIGNAL(triggered()), this, SLOT(groupActionTriggered()));
    connect(gMainWindow->mActions[LC_EDIT_ACTION_SUNLIGHT], SIGNAL(triggered()), this, SLOT(groupActionTriggered()));
    connect(gMainWindow->mActions[LC_EDIT_ACTION_SPOTLIGHT], SIGNAL(triggered()), this, SLOT(groupActionTriggered()));
    connect(gMainWindow->mActions[LC_EDIT_ACTION_AREALIGHT], SIGNAL(triggered()), this, SLOT(groupActionTriggered()));

    // Viewpoint icons
    viewpointGroupAct = new QAction(tr("Viewpoints"), this);
    viewpointGroupAct->setToolTip(tr("Viewpoints - Home"));
    viewpointGroupAct->setIcon(gMainWindow->mActions[LC_VIEW_VIEWPOINT_HOME]->icon());
    viewpointGroupAct->setStatusTip(gMainWindow->mActions[LC_VIEW_VIEWPOINT_HOME]->statusTip());
    viewpointGroupAct->setProperty("CommandId", QVariant(LC_VIEW_VIEWPOINT_HOME));
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
    connect(viewpointGroupAct, SIGNAL(triggered()), this, SLOT(groupActionTriggered()));
    connect(gMainWindow->mActions[LC_VIEW_VIEWPOINT_FRONT], SIGNAL(triggered()), this, SLOT(groupActionTriggered()));
    connect(gMainWindow->mActions[LC_VIEW_VIEWPOINT_BACK], SIGNAL(triggered()), this, SLOT(groupActionTriggered()));
    connect(gMainWindow->mActions[LC_VIEW_VIEWPOINT_LEFT], SIGNAL(triggered()), this, SLOT(groupActionTriggered()));
    connect(gMainWindow->mActions[LC_VIEW_VIEWPOINT_RIGHT], SIGNAL(triggered()), this, SLOT(groupActionTriggered()));
    connect(gMainWindow->mActions[LC_VIEW_VIEWPOINT_TOP], SIGNAL(triggered()), this, SLOT(groupActionTriggered()));
    connect(gMainWindow->mActions[LC_VIEW_VIEWPOINT_BOTTOM], SIGNAL(triggered()), this, SLOT(groupActionTriggered()));
    connect(gMainWindow->mActions[LC_VIEW_VIEWPOINT_HOME], SIGNAL(triggered()), this, SLOT(groupActionTriggered()));

    viewpointZoomExtAct = new QAction(tr("Fit To View"),this);
    viewpointZoomExtAct->setStatusTip(tr("Specify whether to zoom all items to fit the current view"));
    viewpointZoomExtAct->setCheckable(true);
    viewpointZoomExtAct->setChecked(lcGetProfileInt(LC_PROFILE_VIEWPOINT_ZOOM_EXTENT));
    connect(viewpointZoomExtAct, SIGNAL(triggered()), this, SLOT(groupActionTriggered()));

    viewerZoomSliderAct = new QWidgetAction(nullptr);
    viewerZoomSliderWidget = new QSlider();
    viewerZoomSliderWidget->setSingleStep(1);
    viewerZoomSliderWidget->setTickInterval(10);
    viewerZoomSliderWidget->setTickPosition(QSlider::TicksBelow);
    viewerZoomSliderWidget->setMaximum(150);
    viewerZoomSliderWidget->setMinimum(1);
    viewerZoomSliderWidget->setValue(50);
    connect(viewerZoomSliderWidget, SIGNAL(valueChanged(int)), this, SLOT(ViewerZoomSlider(int)));

    gMainWindow->mActions[LC_PIECE_DELETE]->setIcon(QIcon(":/resources/delete.png"));

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
     buildModMenu->addAction(updateBuildModAct);
     buildModMenu->addSeparator();
     buildModMenu->addAction(applyBuildModAct);
     buildModMenu->addAction(removeBuildModAct);
     buildModMenu->addSeparator();
     buildModMenu->addAction(loadBuildModAct);
     buildModMenu->addAction(deleteBuildModAct);
     createBuildModAct->setMenu(buildModMenu);

     lightMenu = new QMenu(tr("Lights"), this);
     lightMenu->addAction(applyLightAct);
     lightMenu->addSeparator();
     lightMenu->addAction(gMainWindow->mActions[LC_EDIT_ACTION_LIGHT]);
     lightMenu->addAction(gMainWindow->mActions[LC_EDIT_ACTION_SUNLIGHT]);
     lightMenu->addAction(gMainWindow->mActions[LC_EDIT_ACTION_SPOTLIGHT]);
     lightMenu->addAction(gMainWindow->mActions[LC_EDIT_ACTION_AREALIGHT]);
     lightGroupAct->setMenu(lightMenu);

     viewpointGroupAct->setMenu(gMainWindow->GetViewpointMenu());
     gMainWindow->GetViewpointMenu()->insertAction(gMainWindow->mActions[LC_VIEW_VIEWPOINT_FRONT], viewpointZoomExtAct);
     gMainWindow->GetViewpointMenu()->insertSeparator(gMainWindow->mActions[LC_VIEW_VIEWPOINT_FRONT]);

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
     ViewerMenu->addAction(gMainWindow->mActions[LC_EDIT_CUT]);
     ViewerMenu->addAction(gMainWindow->mActions[LC_EDIT_COPY]);
     ViewerMenu->addAction(gMainWindow->mActions[LC_EDIT_PASTE]);
     ViewerMenu->addAction(gMainWindow->mActions[LC_PIECE_DELETE]);
     ViewerMenu->addSeparator();
     ViewerMenu->addAction(gMainWindow->mActions[LC_PIECE_EDIT_SELECTED_SUBMODEL]);
     ViewerMenu->addAction(gMainWindow->mActions[LC_PIECE_EDIT_END_SUBMODEL]);
     ViewerMenu->addSeparator();
     // Show/Hide Piece menus
     ViewerMenu->addAction(gMainWindow->mActions[LC_PIECE_HIDE_SELECTED]);
     ViewerMenu->addAction(gMainWindow->mActions[LC_PIECE_HIDE_UNSELECTED]);
     ViewerMenu->addAction(gMainWindow->mActions[LC_PIECE_UNHIDE_ALL]);
     ViewerMenu->addSeparator();
     // Camera menu
     ViewerMenu->addMenu(gMainWindow->GetCameraMenu());
     // Tools menu
     gMainWindow->GetToolsMenu()->addSeparator();
     gMainWindow->GetToolsMenu()->addAction(gMainWindow->mActions[LC_EDIT_ACTION_SELECT]);
     gMainWindow->GetToolsMenu()->addAction(gMainWindow->mActions[LC_EDIT_ACTION_ROTATE]);
     gMainWindow->GetToolsMenu()->addAction(gMainWindow->mActions[LC_EDIT_ACTION_MOVE]);
     gMainWindow->GetToolsMenu()->addAction(gMainWindow->mActions[LC_EDIT_ACTION_DELETE]);
     gMainWindow->GetToolsMenu()->addAction(gMainWindow->mActions[LC_EDIT_ACTION_PAINT]);
     gMainWindow->GetToolsMenu()->addAction(gMainWindow->mActions[LC_EDIT_ACTION_COLOR_PICKER]);
     gMainWindow->GetToolsMenu()->addSeparator();
     gMainWindow->GetToolsMenu()->addAction(gMainWindow->mActions[LC_EDIT_ACTION_ROTATESTEP]);
     gMainWindow->GetToolsMenu()->addAction(createBuildModAct);
     gMainWindow->GetToolsMenu()->addSeparator();
     gMainWindow->GetToolsMenu()->addAction(gMainWindow->mActions[LC_EDIT_ACTION_INSERT]);
     gMainWindow->GetToolsMenu()->addAction(lightGroupAct);
     gMainWindow->GetToolsMenu()->addAction(gMainWindow->mActions[LC_EDIT_ACTION_CAMERA]);
     gMainWindow->GetToolsMenu()->addSeparator();
     gMainWindow->GetToolsMenu()->addAction(gMainWindow->mActions[LC_VIEW_LOOK_AT]);
     gMainWindow->GetToolsMenu()->addAction(viewpointGroupAct);
//     gMainWindow->GetToolsMenu()->addAction(gMainWindow->mActions[LC_VIEW_ZOOM_EXTENTS]);
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
     cameraMenu->addAction(applyCameraAct);
     cameraMenu->addSeparator();
     cameraMenu->addAction(useImageSizeAct);
     cameraMenu->addAction(autoCenterSelectionAct);
     cameraMenu->addAction(defaultCameraPropertiesAct);
     gMainWindow->mActions[LC_EDIT_ACTION_CAMERA]->setMenu(cameraMenu);
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

    gMainWindow->mToolsToolBar->addSeparator();
    gMainWindow->mToolsToolBar->addAction(gMainWindow->mActions[LC_EDIT_ACTION_SELECT]);
    gMainWindow->mToolsToolBar->addAction(gMainWindow->mActions[LC_EDIT_ACTION_ROTATE]);
    gMainWindow->mToolsToolBar->addAction(gMainWindow->mActions[LC_EDIT_ACTION_MOVE]);
    gMainWindow->mToolsToolBar->addAction(gMainWindow->mActions[LC_EDIT_ACTION_DELETE]);
    gMainWindow->mToolsToolBar->addAction(gMainWindow->mActions[LC_EDIT_ACTION_PAINT]);
    gMainWindow->mToolsToolBar->addAction(gMainWindow->mActions[LC_EDIT_ACTION_COLOR_PICKER]);
    gMainWindow->mToolsToolBar->addSeparator();
    gMainWindow->mToolsToolBar->addAction(gMainWindow->mActions[LC_EDIT_ACTION_ROTATESTEP]);
    gMainWindow->mToolsToolBar->addAction(createBuildModAct);
    gMainWindow->mToolsToolBar->addSeparator();
    gMainWindow->mToolsToolBar->addAction(gMainWindow->mActions[LC_EDIT_ACTION_INSERT]);
    gMainWindow->mToolsToolBar->addAction(lightGroupAct);
    gMainWindow->mToolsToolBar->addAction(gMainWindow->mActions[LC_EDIT_ACTION_CAMERA]);
    gMainWindow->mToolsToolBar->addSeparator();
    gMainWindow->mToolsToolBar->addAction(gMainWindow->mActions[LC_VIEW_LOOK_AT]);
    gMainWindow->mToolsToolBar->addAction(viewpointGroupAct);
//    gMainWindow->mToolsToolBar->addAction(gMainWindow->mActions[LC_VIEW_ZOOM_EXTENTS]);
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
    createBuildModAct->setEnabled(false);
    lightGroupAct->setEnabled(false);
    viewpointGroupAct->setEnabled(false);
}

void Gui::Enable3DActions()
{
    bool enabled = buildModsSize();
    createBuildModAct->setEnabled(buildModRange.first() || enabled);
    applyBuildModAct->setEnabled(enabled);
    removeBuildModAct->setEnabled(enabled);
    loadBuildModAct->setEnabled(enabled);
    updateBuildModAct->setEnabled(enabled);
    deleteBuildModAct->setEnabled(enabled);

    lightGroupAct->setEnabled(true);
    viewpointGroupAct->setEnabled(true);
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

    //Properties
    gMainWindow->mPropertiesToolBar->setWindowTitle(trUtf8("Properties"));
    gMainWindow->mPropertiesToolBar->setObjectName("PropertiesToolbar");
    gMainWindow->mPropertiesToolBar->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    addDockWidget(Qt::RightDockWidgetArea, gMainWindow->mPropertiesToolBar);
    viewMenu->addAction(gMainWindow->mPropertiesToolBar->toggleViewAction());

    tabifyDockWidget(viewerDockWindow/*gMainWindow->mTimelineToolBar*/, gMainWindow->mPropertiesToolBar);

    //Timeline
    gMainWindow->mTimelineToolBar->setWindowTitle(trUtf8("Timeline"));
    gMainWindow->mTimelineToolBar->setObjectName("TimelineToolbar");
    gMainWindow->mTimelineToolBar->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    addDockWidget(Qt::RightDockWidgetArea, gMainWindow->mTimelineToolBar);
    viewMenu->addAction(gMainWindow->mTimelineToolBar->toggleViewAction());

    tabifyDockWidget(viewerDockWindow, gMainWindow->mTimelineToolBar);

    //Part Selection
    gMainWindow->mPartsToolBar->setWindowTitle(trUtf8("Parts"));
    gMainWindow->mPartsToolBar->setObjectName("PartsToolbar");
    gMainWindow->mPartsToolBar->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    addDockWidget(Qt::RightDockWidgetArea, gMainWindow->mPartsToolBar);
    viewMenu->addAction(gMainWindow->mPartsToolBar->toggleViewAction());

    tabifyDockWidget(viewerDockWindow/*gMainWindow->mPropertiesToolBar*/, gMainWindow->mPartsToolBar);

    //Colors Selection
    gMainWindow->mColorsToolBar->setWindowTitle(trUtf8("Colors"));
    gMainWindow->mColorsToolBar->setObjectName("ColorsToolbar");
    gMainWindow->mColorsToolBar->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    addDockWidget(Qt::RightDockWidgetArea, gMainWindow->mColorsToolBar);
    viewMenu->addAction(gMainWindow->mColorsToolBar->toggleViewAction());

    tabifyDockWidget(viewerDockWindow/*gMainWindow->mPartsToolBar*/, gMainWindow->mColorsToolBar);

    connect(viewerDockWindow, SIGNAL (topLevelChanged(bool)), this, SLOT (toggleLCStatusBar(bool)));
}

void Gui::UpdateViewerUndoRedo(const QString& UndoText, const QString& RedoText)
{
    if (!UndoText.isEmpty())
    {
        viewerUndo = true;
        undoAct->setEnabled(true);
        undoAct->setText(QString(tr("&Undo %1")).arg(UndoText));
    }
    else
    {
        viewerUndo = false;
        undoAct->setEnabled(false);
        undoAct->setText(tr("&Undo"));
    }

    if (!RedoText.isEmpty())
    {
        viewerRedo = true;
        redoAct->setEnabled(true);
        redoAct->setText(QString(tr("&Redo %1")).arg(RedoText));
    }
    else
    {
        viewerRedo = false;
        redoAct->setEnabled(false);
        redoAct->setText(tr("&Redo"));
    }
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

void Gui::enableApplyLightAction()
{
    applyLightAct->setEnabled(lcGetActiveProject()->GetImageType() == Options::Mt::CSI);
}

void Gui::applyLightSettings()
{
    int it = lcGetActiveProject()->GetImageType();
    if (it != Options::Mt::CSI)
        return;

    if (currentStep){

        Meta meta;
        LightData lightData = meta.LeoCad.light.value();
        LightMeta lightMeta = meta.LeoCad.light;
        lightMeta.setValue(lightData);

        View* ActiveView = gMainWindow->GetActiveView();

        lcModel* ActiveModel = ActiveView->GetActiveModel();

        QString metaString;
        bool newCommand = true;
        Where top = currentStep->topOfStep();
        Where bottom = currentStep->bottomOfStep();

        auto notEqual = [] (const float v1, const float v2)
        {
            return qAbs(v1 - v2) > 0.1f;
        };

        beginMacro("LightSettings");

        // Delete existing LIGHT commands starting at the bottom of the current step
        for (Where walk = bottom - 1; walk >= top.lineNumber; --walk)
            if(readLine(walk).startsWith(lightMeta.preamble))
                deleteLine(walk);

        for (lcLight* Light : ActiveModel->mLights) {

            emit messageSig(LOG_INFO, QString("Setting Light [%1]").arg(Light->m_strName));

            QString Type = "Undefined";
            switch(Light->mLightType)
            {
                case LC_POINTLIGHT:
                    Type = "Point";
                    break;
                case LC_SUNLIGHT:
                    Type = "Sun";
                    break;
                case LC_SPOTLIGHT:
                    Type = "Spot";
                    break;
                case LC_AREALIGHT:
                    Type = "Area";
                    break;
            }

            // Populate existing settings
            QString lightKey = QString("%1 %2").arg(Type).arg(Light->m_strName);
            if (currentStep->lightList.contains(lightKey))
                lightMeta.setValue(currentStep->lightList[lightKey]);

            // Type and Name
            lightMeta.lightType.setValue(Type);
            metaString = lightMeta.lightType.format(false,false);
            metaString.append(QString(" NAME \"%1\"").arg(Light->m_strName));
            currentStep->mi(it)->setMetaAlt(top, metaString, newCommand);

            // Position
            if (notEqual(Light->mPosition[0], lightData.position.x()) ||
                notEqual(Light->mPosition[1], lightData.position.y()) ||
                notEqual(Light->mPosition[2], lightData.position.z())) {
                lightMeta.position.setValues(Light->mPosition[0],
                                             Light->mPosition[1],
                                             Light->mPosition[2]);
                metaString = lightMeta.position.format(false/*local*/,false/*global*/);
                currentStep->mi(it)->setMetaAlt(top, metaString, newCommand);
            }

            // Target Position
            if (notEqual(Light->mTargetPosition[0], lightData.target.x()) ||
                notEqual(Light->mTargetPosition[1], lightData.target.y()) ||
                notEqual(Light->mTargetPosition[2], lightData.target.z())) {
                lightMeta.target.setValues(Light->mTargetPosition[0],
                                           Light->mTargetPosition[1],
                                           Light->mTargetPosition[2]);
                metaString = lightMeta.target.format(false,false);
                currentStep->mi(it)->setMetaAlt(top, metaString, newCommand);
            }

            // Colour
            if (notEqual(Light->mLightColor[0], lightData.lightColour.x()) ||
                notEqual(Light->mLightColor[1], lightData.lightColour.y()) ||
                notEqual(Light->mLightColor[2], lightData.lightColour.z())) {
                lightMeta.lightColour.setValues(Light->mLightColor[0],
                                                Light->mLightColor[1],
                                                Light->mLightColor[2]);
                metaString = lightMeta.lightColour.format(false,false);
                currentStep->mi(it)->setMetaAlt(top, metaString, newCommand);
            }

            // Specular
            if (notEqual(Light->mLightSpecular, lightData.lightSpecular.value())) {
                lightMeta.lightSpecular.setValue(Light->mLightSpecular);
                metaString = lightMeta.lightSpecular.format(false,false);
                currentStep->mi(it)->setMetaAlt(top, metaString, newCommand);
            }

            if (Light->mLightType == LC_SUNLIGHT) {
                // Strength
                if (notEqual(Light->mSpotExponent, lightData.strength.value())) {
                    lightMeta.strength.setValue(Light->mSpotExponent);
                    metaString = lightMeta.strength.format(false,false);
                    currentStep->mi(it)->setMetaAlt(top, metaString, newCommand);
                }
                // Angle
                if (notEqual(Light->mLightFactor[0], lightData.angle.value())) {
                    lightMeta.angle.setValue(Light->mLightFactor[0]);
                    metaString = lightMeta.angle.format(false,false);
                    currentStep->mi(it)->setMetaAlt(top, metaString, newCommand);
                }
            } else {
                // Power
                if (notEqual(Light->mSpotExponent, lightData.power.value())) {
                    lightMeta.power.setValue(Light->mSpotExponent);
                    metaString = lightMeta.power.format(false,false);
                    currentStep->mi(it)->setMetaAlt(top, metaString, newCommand);
                }

                // Cutoff Distance
                if (Light->mEnableCutoff &&
                   (notEqual(Light->mSpotCutoff, lightData.spotCutoff.value()))) {
                    lightMeta.power.setValue(Light->mSpotCutoff);
                    metaString = lightMeta.spotCutoff.format(false,false);
                    currentStep->mi(it)->setMetaAlt(top, metaString, newCommand);
                }

                switch (Light->mLightType)
                {
                case LC_POINTLIGHT:
                case LC_SPOTLIGHT:
                    // Radius
                    if (notEqual(Light->mLightFactor[0], lightData.radius.value())) {
                        lightMeta.radius.setValue(Light->mLightFactor[0]);
                        metaString = lightMeta.radius.format(false,false);
                        currentStep->mi(it)->setMetaAlt(top, metaString, newCommand);
                    }
                    if (Light->mLightType == LC_SPOTLIGHT) {
                        // Spot Blend
                        if (notEqual(Light->mLightFactor[1], lightData.spotBlend.value())) {
                            lightMeta.spotBlend.setValue(Light->mLightFactor[1]);
                            metaString = lightMeta.spotBlend.format(false,false);
                            currentStep->mi(it)->setMetaAlt(top, metaString, newCommand);
                        }
                        // Spot Size
                        if (notEqual(Light->mSpotSize, lightData.spotSize.value())) {
                            lightMeta.spotSize.setValue(Light->mSpotSize);
                            metaString = lightMeta.spotSize.format(false,false);
                            currentStep->mi(it)->setMetaAlt(top, metaString, newCommand);
                        }
                    }
                    break;
                case LC_AREALIGHT:
                    if (Light->mLightShape == LC_LIGHT_SHAPE_RECTANGLE || Light->mLightShape == LC_LIGHT_SHAPE_ELLIPSE) {
                        // Width and Height
                        if (notEqual(Light->mLightFactor[0], lightData.width.value()) ||
                            notEqual(Light->mLightFactor[1], lightData.height.value())) {
                            lightMeta.width.setValue(Light->mLightFactor[0]);
                            metaString = lightMeta.width.format(false,false);
                            metaString.append(QString(" HEIGHT %1").arg(double(Light->mLightFactor[1])));
                            currentStep->mi(it)->setMetaAlt(top, metaString, newCommand);
                        }
                    } else {
                        // Size
                        if (notEqual(Light->mLightFactor[0], lightData.size.value())) {
                            lightMeta.size.setValue(Light->mLightFactor[0]);
                            metaString = lightMeta.size.format(false,false);
                            currentStep->mi(it)->setMetaAlt(top, metaString, newCommand);
                        }
                    }

                    // Shape
                    QString Shape = "Undefined";
                    switch(Light->mLightShape) {
                    case LC_LIGHT_SHAPE_SQUARE:
                        Shape = "Square";
                        break;
                    case LC_LIGHT_SHAPE_DISK:
                        Shape = "Disk";
                        break;
                    case LC_LIGHT_SHAPE_RECTANGLE:
                        Shape = "Rectangle";
                        break;
                    case LC_LIGHT_SHAPE_ELLIPSE:
                        Shape = "Ellipse";
                        break;
                    }
                    if (notEqual(Light->mLightShape, lightData.radius.value())) {
                        lightMeta.lightShape.setValue(Shape);
                        metaString = lightMeta.lightShape.format(false,false);
                        currentStep->mi(it)->setMetaAlt(top, metaString, newCommand);
                    }
                    break;
                }
            }
        }

        endMacro();
    }
}

void Gui::applyCameraSettings()
{
    if (currentStep){

        SettingsMeta cameraMeta;

        View* ActiveView = gMainWindow->GetActiveView();

        if (!ActiveView)
            return;
        else if (autoCenterSelectionAct->isChecked())
            ActiveView->LookAt();

        lcCamera* Camera = ActiveView->mCamera;

        auto validCameraFoV = [&cameraMeta, &Camera] ()
        {
            if (Preferences::usingNativeRenderer)
                return qRound(Camera->m_fovy);

                  // e.g.            30.0  +                 0.01         - 30.0
            float result = Camera->m_fovy  + cameraMeta.cameraFoV.value() - gApplication->mPreferences.mCFoV;

            return qRound(result);
        };

        auto notEqual = [] (const float v1, const float v2)
        {
            return qAbs(v1 - v2) > 0.1f;
        };

        emit messageSig(LOG_INFO, QString("Setting %1 Camera").arg(Camera->m_strName[0] == '\0' ? "Default" : Camera->m_strName));

        QString imageFileName;

        int it = lcGetActiveProject()->GetImageType();
        switch(it){
        case Options::Mt::PLI:
            cameraMeta.cameraAngles   = currentStep->pli.pliMeta.cameraAngles;
            cameraMeta.cameraDistance = currentStep->pli.pliMeta.cameraDistance;
            cameraMeta.modelScale     = currentStep->pli.pliMeta.modelScale;
            cameraMeta.cameraFoV      = currentStep->pli.pliMeta.cameraFoV;
            cameraMeta.isOrtho        = currentStep->pli.pliMeta.isOrtho;
            cameraMeta.imageSize      = currentStep->pli.pliMeta.imageSize;
            cameraMeta.target         = currentStep->pli.pliMeta.target;
            break;
        case Options::Mt::SMP:
            cameraMeta.cameraAngles   = currentStep->subModel.subModelMeta.cameraAngles;
            cameraMeta.cameraDistance = currentStep->subModel.subModelMeta.cameraDistance;
            cameraMeta.modelScale     = currentStep->subModel.subModelMeta.modelScale;
            cameraMeta.cameraFoV      = currentStep->subModel.subModelMeta.cameraFoV;
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
        bool clearStepCache = false;
        Where undefined = Where();
        Where top = currentStep->topOfStep();
        Where bottom = currentStep->bottomOfStep();

        float Latitude, Longitude, Distance;
        Camera->GetAngles(Latitude, Longitude, Distance);

        bool applyTarget = !(Camera->mTargetPosition[0] == 0.0f  &&
                             Camera->mTargetPosition[1] == 0.0f  &&
                             Camera->mTargetPosition[2] == 0.0f);

        beginMacro("CameraSettings");

        // execute first in last out
        if (applyTarget) {

            clearStepCache = true;
            if (QFileInfo(imageFileName).exists())
                clearStepCSICache(imageFileName);

            // Switch Y and Z axis with -Y(LC -Z) in the up direction
            cameraMeta.target.setValues(Camera->mTargetPosition[0],
                                        Camera->mTargetPosition[2],
                                        Camera->mTargetPosition[1]);
            metaString = cameraMeta.target.format(true/*local*/,false/*global*/);
            newCommand = cameraMeta.target.here() == undefined;
            currentStep->mi(it)->setMetaAlt(newCommand ? top : cameraMeta.target.here(), metaString, newCommand);
//            currentStep->mi(it)->setMeta(top,bottom,&cameraMeta.target,true/*useTop*/,1/*append*/,true/*local*/,false/*askLocal,global=false*/);
        }

        if (useImageSizeAct->isChecked()) {
            cameraMeta.imageSize.setValues(lcGetActiveProject()->GetImageWidth(),
                                           lcGetActiveProject()->GetImageHeight());
            metaString = cameraMeta.imageSize.format(true,false);
            newCommand = cameraMeta.imageSize.here() == undefined;
            currentStep->mi(it)->setMetaAlt(newCommand ? top : cameraMeta.imageSize.here(), metaString, newCommand);
        }

        if (notEqual(Camera->GetScale(), cameraMeta.modelScale.value())) {
            clearStepCache = true;
            cameraMeta.modelScale.setValue(Camera->GetScale());
            metaString = cameraMeta.modelScale.format(true,false);
            newCommand = cameraMeta.modelScale.here() == undefined;
            currentStep->mi(it)->setMetaAlt(newCommand ? top : cameraMeta.modelScale.here(), metaString, newCommand);
        }

        if (notEqual(qRound(Distance), cameraMeta.cameraDistance.value()) &&
                    !useImageSizeAct->isChecked()) {
            clearStepCache = true;
            cameraMeta.cameraDistance.setValue(qRound(Distance));
            metaString = cameraMeta.cameraDistance.format(true,false);
            newCommand = cameraMeta.cameraDistance.here() == undefined;
            currentStep->mi(it)->setMetaAlt(newCommand ? top : cameraMeta.cameraDistance.here(), metaString, newCommand);
        }

        if (notEqual(qRound(Latitude), cameraMeta.cameraAngles.value(0)) ||
            notEqual(qRound(Longitude),cameraMeta.cameraAngles.value(1))) {
            clearStepCache = true;
            cameraMeta.cameraAngles.setValues(qRound(Latitude), qRound(Longitude));
            metaString = cameraMeta.cameraAngles.format(true,false);
            newCommand = cameraMeta.cameraAngles.here() == undefined;
            currentStep->mi(it)->setMetaAlt(newCommand ? top : cameraMeta.cameraAngles.here(), metaString, newCommand);
        }

        float fovy = validCameraFoV();
        if (notEqual(cameraMeta.cameraFoV.value(), fovy)) {
            clearStepCache = true;
            cameraMeta.cameraFoV.setValue(fovy);
            metaString = cameraMeta.cameraFoV.format(true,false);
            newCommand = cameraMeta.cameraFoV.here() == undefined;
            currentStep->mi(it)->setMetaAlt(newCommand ? top : cameraMeta.cameraFoV.here(), metaString, newCommand);
        }

        if (Camera->IsOrtho() != cameraMeta.isOrtho.value()) {
            cameraMeta.isOrtho.setValue(Camera->IsOrtho());
            metaString = cameraMeta.isOrtho.format(true,false);
            newCommand = cameraMeta.isOrtho.here() == undefined;
            currentStep->mi(it)->setMetaAlt(newCommand ? top : cameraMeta.isOrtho.here(), metaString, newCommand);
        }

        if (Camera->m_strName[0]) {
            cameraMeta.cameraName.setValue(Camera->m_strName);
            metaString = cameraMeta.cameraName.format(true,false);
            newCommand = cameraMeta.cameraName.here() == undefined;
            currentStep->mi(it)->setMetaAlt(newCommand ? top : cameraMeta.cameraName.here(), metaString, newCommand);
        }

        if (clearStepCache && QFileInfo(imageFileName).exists())
            clearStepCSICache(imageFileName);

        endMacro();
    }
}

void Gui::groupActionTriggered()
{
    QAction* Action = static_cast<QAction*>(sender());
    lcCommandId commandId;
    QString commandTip;
    bool ok;

    if (Action == viewpointZoomExtAct)
    {
        lcSetProfileInt(LC_PROFILE_VIEWPOINT_ZOOM_EXTENT, viewpointZoomExtAct->isChecked());
    }
    else if (Action == viewpointGroupAct)
    {
        commandId = lcCommandId(viewpointGroupAct->property("CommandId").toInt(&ok));
        if (ok)
            gMainWindow->mActions[commandId]->trigger();
    }
    else if (Action == lightGroupAct)
    {
        commandId = lcCommandId(lightGroupAct->property("CommandId").toInt(&ok));
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
                lightGroupAct->setIcon(gMainWindow->mActions[commandId]->icon());
                lightGroupAct->setToolTip(tr(commandTip.toLatin1()));
                lightGroupAct->setStatusTip(gMainWindow->mActions[commandId]->statusTip());
                lightGroupAct->setProperty("CommandId", QVariant(commandId));
            } else {
                viewpointGroupAct->setIcon(gMainWindow->mActions[commandId]->icon());
                viewpointGroupAct->setToolTip(tr(commandTip.toLatin1()));
                viewpointGroupAct->setStatusTip(gMainWindow->mActions[commandId]->statusTip());
                viewpointGroupAct->setProperty("CommandId", QVariant(commandId));
            }
        }
    }
}

void Gui::restoreLightAndViewpointDefaults(){
    lightGroupAct->setToolTip(tr("Lights - Pointlight"));
    lightGroupAct->setIcon(gMainWindow->mActions[LC_EDIT_ACTION_LIGHT]->icon());
    lightGroupAct->setStatusTip(gMainWindow->mActions[LC_EDIT_ACTION_LIGHT]->statusTip());
    lightGroupAct->setProperty("CommandId", QVariant(LC_EDIT_ACTION_LIGHT));

    viewpointGroupAct->setToolTip(tr("Viewpoints - Home"));
    viewpointGroupAct->setIcon(gMainWindow->mActions[LC_VIEW_VIEWPOINT_HOME]->icon());
    viewpointGroupAct->setStatusTip(gMainWindow->mActions[LC_VIEW_VIEWPOINT_HOME]->statusTip());
    viewpointGroupAct->setProperty("CommandId", QVariant(LC_VIEW_VIEWPOINT_HOME));
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

void Gui::showDefaultCameraProperties()
{
  lcGetPreferences().mDefaultCameraProperties = defaultCameraPropertiesAct->isChecked();
  lcSetProfileInt(LC_PROFILE_DEFAULT_CAMERA_PROPERTIES, defaultCameraPropertiesAct->isChecked());
}

void Gui::useImageSize()
{
  lcSetProfileInt(LC_PROFILE_USE_IMAGE_SIZE, useImageSizeAct->isChecked());
}

void Gui::autoCenterSelection()
{
  lcSetProfileInt(LC_PROFILE_AUTO_CENTER_SELECTION, autoCenterSelectionAct->isChecked());
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
            emit messageSig(LOG_ERROR, QString("Failed to save current model to file [%1]").arg(modelFile));

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
    if (!currentStep)
        return;

    if (lcGetActiveProject()->GetImageType() == Options::Mt::PLI)
        return;

    View* ActiveView = gMainWindow->GetActiveView();
    lcModel* ActiveModel = ActiveView->GetActiveModel();

    if (ActiveModel) {
        QString BuildModKey = buildModChangeKey;
        bool edit = ! BuildModKey.isEmpty();
        QString statusLabel = edit ? "Updating" : "Creating";

        emit progressBarPermInitSig();
        emit progressPermRangeSig(0, 0);   // Busy indicator
        emit progressPermMessageSig(QString("%1 Build Modification...").arg(statusLabel));

        if (buildModRange.first() || edit){

            emit messageSig(LOG_INFO, QString("%1 BuildMod for Step %2...")
                            .arg(statusLabel)
                            .arg(currentStep->stepNumber.number));

            // 'load...' default lines from modelFile and 'save...' buildMod lines from 3DViewer
            lcArray<lcGroup*>  mGroups;
            lcArray<lcCamera*> mCameras;
            lcArray<lcLight*>  mLights;
            lcArray<lcPiece*>  mLPubPieces;                     // Pieces in the buildMod - may include removed viewer pieces
            lcArray<lcGroup*>  mLPubGroups;
            lcArray<lcPiece*>  mViewerPieces;                   // All viewer pieces in the step
            lcArray<lcGroup*>  mViewerGroups;
            lcModelProperties  mViewerProperties;
            lcArray<lcPieceControlPoint> ControlPoints;
            QStringList  mLPubFileLines, mViewerFileLines,
                         LPubModContents, ViewerModContents;

            bool FadeStep         = page.meta.LPub.fadeStep.fadeStep.value();
            bool HighlightStep    = page.meta.LPub.highlightStep.highlightStep.value() && !suppressColourMeta();

            int AddedPieces       = 0;
            int CurrentStep       = 1;
            lcPiece *Piece        = nullptr;
            lcCamera *Camera      = nullptr;
            lcLight  *Light       = nullptr;
            mViewerProperties     = ActiveModel->GetProperties();
            mViewerFileLines      = ActiveModel->mFileLines;
            mViewerPieces         = ActiveModel->GetPieces();
            lcPiecesLibrary *Library = lcGetPiecesLibrary();

            // When edit, initialize BuildMod StepPieces, and Begin and End range with the existing values
            int BuildModBegin     = edit ? getBuildModBeginLineNumber(BuildModKey)  : 0;
            int BuildModAction    = edit ? getBuildModActionLineNumber(BuildModKey) : 0;
            int BuildModEnd       = edit ? getBuildModEndLineNumber(BuildModKey)    : 0;

            int ModBeginLineNum   = edit ? BuildModBegin  : buildModRange.at(BM_BEGIN_LINE_NUM);
            int ModActionLineNum  = edit ? BuildModAction : buildModRange.at(BM_ACTION_LINE_NUM);
            int ModEndLineNum     = edit ? BuildModEnd    : buildModRange.at(BM_BEGIN_LINE_NUM);
            int ModStepPieces     = edit ? getBuildModStepPieces(BuildModKey) : 0;    // All pieces in the previous step
            int ModelIndex        = edit ? getSubmodelIndex(getBuildModModelName(BuildModKey)) : buildModRange.at(BM_MODEL_INDEX);
            int ModStepIndex      = getBuildModStepIndex(currentStep->top);
            int ModDisplayPageNum = displayPageNum;
            buildModChangeKey     = QString();

            QString ModelName     = getSubmodelName(ModelIndex);
            QString ModStepKey    = viewerStepKey;

            // Check if there is an existing build mod in this Step
            QRegExp lineRx("^0 !LPUB BUILD_MOD BEGIN ");
            if (stepContains(currentStep->top, lineRx) && !edit) {

                // Get the application icon as a pixmap
                QPixmap _icon = QPixmap(":/icons/lpub96.png");
                if (_icon.isNull())
                    _icon = QPixmap (":/icons/update.png");

                QMessageBox box;
                box.setWindowIcon(QIcon());
                box.setIconPixmap (_icon);
                box.setTextFormat (Qt::RichText);
                box.setWindowFlags (Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
                QString title = "<b>" + QMessageBox::tr ("You specified create action for an existing Build Modification") + "</b>";
                QString text = QMessageBox::tr("<br>This action will replace the existing Build Modification."
                                               "<br>You can cancel and select 'Update Build Modification...' from the build modification submenu."
                                               "<br>Do you want to continue with this create action ?");
                box.setText (title);
                box.setInformativeText (text);
                box.setStandardButtons (QMessageBox::Cancel | QMessageBox::Ok);
                box.setDefaultButton   (QMessageBox::Cancel);

                if (box.exec() == QMessageBox::Cancel)
                    return;
            }

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

            auto ConfigurePartLine = [&ModelIndex, &FadeStep, &HighlightStep, &LPubModContents] (int LineTypeIndex, QTextStream &Stream)
            {
                if (!FadeStep && !HighlightStep)
                    return;

                if (!Stream.string()->isEmpty()) {

                    QStringList ModLines = Stream.string()->split(QRegExp("(\\r\\n)|\\r|\\n"), QString::SkipEmptyParts);
                    QString PartLine = ModLines.last();

                    QStringList argv;
                    split(PartLine, argv);

                    if (argv.size() == 15 && argv[0] == "1") {

                        QString NameMod, ColourPrefix;
                        if (FadeStep) {
                            NameMod = FADE_SFX;
                            ColourPrefix = LPUB3D_COLOUR_FADE_PREFIX;
                        } else if (HighlightStep) {
                            NameMod = HIGHLIGHT_SFX;
                            ColourPrefix = LPUB3D_COLOUR_HIGHLIGHT_PREFIX;
                        }

                        // Colour code
                        if (argv[1].startsWith(ColourPrefix)) {
                            int LineNumber;
                            QString LPubPartLine, NewColorCode;
                            bool NewLine = !gui->getSelectedLine(ModelIndex, LineTypeIndex, VIEWER_MOD, LineNumber);

                            if (NewLine) {
                                NewColorCode = argv[1];

                            } else {
                                LPubPartLine = LPubModContents.at(LineTypeIndex);
                                QStringList dargv;
                                split(LPubPartLine, dargv);

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

                        ModLines.removeLast();
                        ModLines.append(PartLine);

                        Stream.string()->clear();

                        for (int i = 0; i < ModLines.size(); i++)
                            Stream << ModLines.at(i);
                    }
                }
            };

            auto InsertPiece = [] (lcArray<lcPiece*> &mLPubPieces, lcPiece* Piece, int PieceIdx)
            {
                PieceInfo* Info = Piece->mPieceInfo;

                if (!Info->IsModel())
                {
                    lcMesh* Mesh = Info->IsTemporary() ? gPlaceholderMesh : Info->GetMesh();

                    if (Mesh && Mesh->mVertexCacheOffset == -1)
                        lcGetPiecesLibrary()->mBuffersDirty = true;
                }

                mLPubPieces.InsertAt(PieceIdx, Piece);
            };

            auto AddPiece = [&InsertPiece](lcArray<lcPiece*> &mLPubPieces, lcPiece* Piece)
            {
                for (int PieceIdx = 0; PieceIdx < mLPubPieces.GetSize(); PieceIdx++)
                {
                    if (mLPubPieces[PieceIdx]->GetStepShow() > Piece->GetStepShow())
                    {
                        InsertPiece(mLPubPieces, Piece, PieceIdx);
                        return;
                    }
                }

                InsertPiece(mLPubPieces, Piece, mLPubPieces.GetSize());
            };

            auto GetAddedPieces = [this, &mViewerPieces]()
            {
                Rc rc;
                Where walk = currentStep->top;
                QString line = readLine(walk);
                rc =  page.meta.parse(line, walk, false);
                if (rc == StepRc || rc == RotStepRc)
                    walk++;   // Advance past STEP meta
                int stepParts = 0;
                for ( ;
                      walk.lineNumber < subFileSize(walk.modelName);
                      walk.lineNumber++) {
                    line = readLine(walk);
                    rc =  page.meta.parse(line, walk, false);
                    if (line.toLatin1()[0] == '1')
                        stepParts++;
                    if (rc == StepRc || rc == RotStepRc)
                        break;
                }
                int result = mViewerPieces.GetSize() - stepParts;
                return result;
            };

            // load LPub content
            QByteArray ByteArray;
            for (int i = 0; i < ldrawFile.contents(ModelName).size(); i++){
                if (i > ModActionLineNum)
                    break;
                QString ModLine = ldrawFile.contents(ModelName).at(i);
                if (i >= ModBeginLineNum)
                    LPubModContents.append(ModLine);
                ByteArray.append(ModLine);
                ByteArray.append(QString("\n"));
            }
            QBuffer Buffer(&ByteArray);
            Buffer.open(QIODevice::ReadOnly);
            Buffer.seek(0);

            // load mLPubFileLines, mLPubGroups and mLPubPieces, Camera, Lights and LSynth Control Points with LDraw content using QBA buffer
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

                        if (mViewerProperties.mName != Name)
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
                        mLPubFileLines.append(OriginalLine);
                        continue;
                    }

                    if (Token != QLatin1String("!LPUB"))
                    {
                        mLPubFileLines.append(OriginalLine);
                        continue;
                    }

                    LineStream >> Token;

                    if (Token == QLatin1String("MODEL"))
                    {
                        mViewerProperties.ParseLDrawLine(LineStream);
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
                    else if (Token == QLatin1String("LIGHT"))
                    {
                        if (!Light)
                            Light = new lcLight(0.0f, 0.0f, 0.0f);

                        if (Light->ParseLDrawLine(LineStream))
                        {
                            Light->CreateName(mLights);
                            mLights.Add(Light);
                            Light = nullptr;
                        }
                    }
                    else if (Token == QLatin1String("GROUP"))
                    {
                        LineStream >> Token;

                        if (Token == QLatin1String("BEGIN"))
                        {
                            QString Name = LineStream.readAll().trimmed();
                            lcGroup* Group = GetGroup(Name, true);
                            if (!mLPubGroups.IsEmpty())
                                Group->mGroup = mLPubGroups[mLPubGroups.GetSize() - 1];
                            else
                                Group->mGroup = nullptr;
                            mLPubGroups.Add(Group);
                        }
                        else if (Token == QLatin1String("END"))
                        {
                            if (!mLPubGroups.IsEmpty())
                                mLPubGroups.RemoveIndex(mLPubGroups.GetSize() - 1);
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
                    float IncludeMatrix[12];

                    for (int TokenIdx = 0; TokenIdx < 12; TokenIdx++)
                        LineStream >> IncludeMatrix[TokenIdx];

                    lcMatrix44 IncludeTransform(lcVector4(IncludeMatrix[3], IncludeMatrix[6], IncludeMatrix[9], 0.0f), lcVector4(IncludeMatrix[4], IncludeMatrix[7], IncludeMatrix[10], 0.0f),
                            lcVector4(IncludeMatrix[5], IncludeMatrix[8], IncludeMatrix[11], 0.0f), lcVector4(IncludeMatrix[0], IncludeMatrix[1], IncludeMatrix[2], 1.0f));

                    QString PartId = LineStream.readAll().trimmed();
                    QByteArray CleanId = PartId.toLatin1().toUpper().replace('\\', '/');

                    if (Library->IsPrimitive(CleanId.constData()))
                    {
                        mLPubFileLines.append(OriginalLine);
                    }
                    else
                    {
                        if (!Piece)
                            Piece = new lcPiece(nullptr);

                        if (!mLPubGroups.IsEmpty())
                            Piece->SetGroup(mLPubGroups[mLPubGroups.GetSize() - 1]);

                        PieceInfo* Info = Library->FindPiece(PartId.toLatin1().constData(), lcGetActiveProject(), true, true);

                        float* Matrix = IncludeTransform;
                        lcMatrix44 Transform(lcVector4(Matrix[0], Matrix[2], -Matrix[1], 0.0f), lcVector4(Matrix[8], Matrix[10], -Matrix[9], 0.0f),
                                lcVector4(-Matrix[4], -Matrix[6], Matrix[5], 0.0f), lcVector4(Matrix[12], Matrix[14], -Matrix[13], 1.0f));

                        Piece->SetFileLine(mLPubFileLines.size());
                        Piece->SetPieceInfo(Info, PartId, false);
                        Piece->Initialize(Transform, quint32(CurrentStep));
                        Piece->SetColorCode(quint32(ColorCode));
                        Piece->SetControlPoints(ControlPoints);
                        AddPiece(mLPubPieces, Piece);

                        Piece = nullptr;
                    }
                }
                else
                    mLPubFileLines.append(OriginalLine);

                Library->WaitForLoadQueue();
                Library->mBuffersDirty = true;
                Library->UnloadUnusedParts();

                delete Piece;
                delete Camera;
            }

            Buffer.close();

            // load Viewer content from mViewerFileLines, mViewerGroups and mViewerPieces, Camera, Lights and LSynth Control Points
            bool NewPiece      = false;
            bool SelectedOnly  = false;
            bool PieceInserted = false;
            bool PieceModified = true;
            QLatin1String LineEnding("\r\n");

            QString ViewerModContentsString;
            QTextStream Stream(&ViewerModContentsString, QIODevice::ReadWrite);

            lcStep Step       = 1;
            int CurrentLine   = ModBeginLineNum/*0*/;
            int AddedSteps    = 0;

            int LineIndex     = NEW_PART;
            int LineNumber    = 0;
            int PieceAdjustment = 0;
            int EndModLineNum = ModActionLineNum;

            // Do we have a difference between the number of LPub pieces and Viewer pieces ?
            // If pieces have been added or removed, we capture the delta in PieceAdjustment
            // and AddedPieces.
            PieceAdjustment = AddedPieces = edit ? mViewerPieces.GetSize() - ModStepPieces : GetAddedPieces();

            // Adjust EndModLineNum to accomodate removed pieces
            if (PieceAdjustment < 0)
                EndModLineNum -= PieceAdjustment;

#ifdef QT_DEBUG_MODE
            emit messageSig(LOG_DEBUG, QString("%1Pieces [%2], Viewer Pieces Count [%3], LPub Pieces Count [%4]")
                            .arg(PieceAdjustment == 0 ? "" : PieceAdjustment > 0 ? "Added " : "Removed ")
                            .arg(PieceAdjustment  < 0 ? -PieceAdjustment : PieceAdjustment)
                            .arg(mViewerPieces.GetSize()).arg(ModStepPieces));
#endif

            for (lcPiece* Piece : mViewerPieces)
            {
                LineIndex  = Piece->GetLineTypeIndex();

                // We have a new piece (NewPiece set to true) when the piece LineIndex is -1
                NewPiece   = !getSelectedLine(ModelIndex, LineIndex, VIEWER_MOD, LineNumber);

                // If PieceInserted, we must increment the 'original' line number for the next piece
                // and set PieceInserted to false.
                if (PieceInserted) {
                    LineNumber   += PieceInserted;
                    PieceInserted = false;
                }

                // Added pieces have a line number of 0 so we must reset the LineNumber to EndModLineNum
                // and set PieceInserted to true so we can increment the line number for the next piece.
                // For each NewPiece, we'll decrement PieceAdjustment.
                if ((PieceInserted = NewPiece)) {
                    PieceAdjustment--;
                    LineNumber = EndModLineNum;
                }

                // Set PieceModified. Only modified pieces are added to ViewerModContents
                PieceModified = LineNumber <= ModActionLineNum || Piece->PieceModified() || NewPiece;

#ifdef QT_DEBUG_MODE
                emit messageSig(LOG_DEBUG, QString("Viewer Piece LineIndex [%1], ID [%2], Name [%3], LineNumber [%4], Modified: [%5]")
                                                   .arg(LineIndex < 0 ? "Added Piece #"+QString::number(AddedPieces) : QString::number(LineIndex))
                                                   .arg(Piece->GetID())
                                                   .arg(Piece->GetName())
                                                   .arg(LineNumber)
                                                   .arg(PieceModified ? "Yes" : "No"));
#endif

                // If PieceAdjustment is not 0, we increment EndModLineNum as we process each piece
                if (PieceAdjustment > 0)
                     EndModLineNum = LineNumber + 1;

                // No more modified pieces so we exit the loop
                if (LineNumber > EndModLineNum)
                    break;

                // Process the current piece
                if (LineNumber >= ModBeginLineNum && PieceModified) {

                    // Use LPubFileLines
                    while (LineNumber/*Piece->GetFileLine()*/ > CurrentLine && CurrentLine < mLPubFileLines.size()/*mViewerFileLines.size()*/)
                    {
                        QString Line = mLPubFileLines[CurrentLine]/*mViewerFileLines[CurrentLine]*/;
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
                            Stream << mLPubFileLines[CurrentLine]/*mViewerFileLines[CurrentLine]*/;
                            if (AddedSteps > 0)
                                AddedSteps--;
                        }
                        CurrentLine++;
                    }

                    // Use Viewer Pieces
                    while (Piece->GetStepShow() > Step)
                    {
                        Stream << QLatin1String("0 STEP\r\n");
                        AddedSteps++;
                        Step++;
                    }

                    lcGroup* PieceGroup = Piece->GetGroup();

                    if (PieceGroup)
                    {
                        if (mViewerGroups.IsEmpty() || (!mViewerGroups.IsEmpty() && PieceGroup != mViewerGroups[mViewerGroups.GetSize() - 1]))
                        {
                            lcArray<lcGroup*> PieceParents;

                            for (lcGroup* Group = PieceGroup; Group; Group = Group->mGroup)
                                PieceParents.InsertAt(0, Group);

                            int FoundParent = -1;

                            while (!mViewerGroups.IsEmpty())
                            {
                                lcGroup* Group = mViewerGroups[mViewerGroups.GetSize() - 1];
                                int Index = PieceParents.FindIndex(Group);

                                if (Index == -1)
                                {
                                    mViewerGroups.RemoveIndex(mViewerGroups.GetSize() - 1);
                                    Stream << QLatin1String("0 !LPUB GROUP END\r\n");
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
                                mViewerGroups.Add(Group);
                                Stream << QLatin1String("0 !LPUB GROUP BEGIN ") << Group->mName << LineEnding;
                            }
                        }
                    }
                    else
                    {
                        while (mViewerGroups.GetSize())
                        {
                            mViewerGroups.RemoveIndex(mViewerGroups.GetSize() - 1);
                            Stream << QLatin1String("0 !LPUB GROUP END\r\n");
                        }
                    }

                    if (Piece->mPieceInfo->GetSynthInfo())
                    {
                        Stream << QLatin1String("0 !LPUB SYNTH BEGIN\r\n");

                        const lcArray<lcPieceControlPoint>& ControlPoints = Piece->GetControlPoints();
                        for (int ControlPointIdx = 0; ControlPointIdx < ControlPoints.GetSize(); ControlPointIdx++)
                        {
                            const lcPieceControlPoint& ControlPoint = ControlPoints[ControlPointIdx];

                            Stream << QLatin1String("0 !LPUB SYNTH CONTROL_POINT");

                            const float* FloatMatrix = ControlPoint.Transform;
                            float Numbers[13] = { FloatMatrix[12], -FloatMatrix[14], FloatMatrix[13], FloatMatrix[0], -FloatMatrix[8], FloatMatrix[4], -FloatMatrix[2], FloatMatrix[10], -FloatMatrix[6], FloatMatrix[1], -FloatMatrix[9], FloatMatrix[5], ControlPoint.Scale };

                            for (int NumberIdx = 0; NumberIdx < 13; NumberIdx++)
                                Stream << ' ' << lcFormatValue(Numbers[NumberIdx], NumberIdx < 3 ? 4 : 6);

                            Stream << LineEnding;
                        }
                    }

                    Piece->SaveLDraw(Stream);

                    ConfigurePartLine(Piece->GetLineTypeIndex(), Stream);

                    if (Piece->mPieceInfo->GetSynthInfo())
                        Stream << QLatin1String("0 !LPUB SYNTH END\r\n");
                }
            }

            while (CurrentLine < mLPubFileLines.size()/*mViewerFileLines.size()*/)
            {
                QString Line = mLPubFileLines[CurrentLine]/*mViewerFileLines[CurrentLine]*/;
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
                    Stream << mLPubFileLines[CurrentLine]/*mViewerFileLines[CurrentLine]*/;
                CurrentLine++;
            }

            while (mViewerGroups.GetSize())
            {
                mViewerGroups.RemoveIndex(mViewerGroups.GetSize() - 1);
                Stream << QLatin1String("0 !LPUB GROUP END\r\n");
            }

            for (lcCamera* Camera : mCameras)
                if (!SelectedOnly || Camera->IsSelected())
                    Camera->SaveLDraw(Stream);

            for (lcLight* Light : mLights)
                if (!SelectedOnly || Light->IsSelected())
                    Light->SaveLDraw(Stream);

            ViewerModContents = QString(ViewerModContentsString).split(QRegExp("(\\r\\n)|\\r|\\n"), QString::SkipEmptyParts);

            int BuildModPieces = ViewerModContents.size();

            if (!edit)
                BuildModKey = QString("%1 Mod %2").arg(ModelName).arg(buildModsSize() + 1);

            // Delete meta commands uses the 'original' BuildMod values
            int SaveModBeginLineNum  = edit ? BuildModBegin  : ModBeginLineNum;
            int SaveModActionLineNum = edit ? BuildModAction : ModActionLineNum;
            int SaveModEndLineNum    = edit ? BuildModEnd    : ModEndLineNum;
            int SaveModPieces        = edit ? SaveModEndLineNum - SaveModActionLineNum - 1/*Meta Line*/
                                            : BuildModPieces - (AddedPieces > 0 ? AddedPieces : 0);

#ifdef QT_DEBUG_MODE
            emit messageSig(LOG_TRACE, QString("%1 BuildMod Save LineNumbers "
                                               "Begin: %2, Action: %3, End: %4, ModPieces: %5")
                                               .arg(edit ? "Update" : "Create")
                                               .arg(SaveModBeginLineNum)
                                               .arg(SaveModActionLineNum)
                                               .arg(SaveModEndLineNum)
                                               .arg(SaveModPieces));
#endif

            // BuildMod meta command lines are written in a bottom up manner

            // Set ModBeginLineNum to top of step parts plus the number of parts replacec by BuildMod
            // This is the position for BUILD_MOD END
            ModBeginLineNum += SaveModPieces;

            // Set ModActionLineNum to SaveModBeginLineNum - initial BuildMod insertion line
            // This is the position of BUILD_MOD MOD_END
            ModActionLineNum = SaveModBeginLineNum;

            BuildModData buildModData;

            Where modHere;

            if (BuildModPieces) {

                buildModData = currentStep->buildMod.value();

                QString metaString;
                buildModData.buildModKey = QString();

                QString assemDirName = QDir::currentPath() +  QDir::separator() + Paths::assemDir;
                QFileInfo imageInfo(currentStep->pngName);
                QFile imageFile(assemDirName + QDir::separator() + imageInfo.fileName());

                beginMacro("CreateBuildModContent");

                // Delete old BUILD_MOD END meta command
                Where endMod = Where(ModelName, SaveModEndLineNum);
                QString modLine = readLine(endMod);
                Rc rc = page.meta.parse(modLine, endMod);
                if (rc == BuildModEndRc)
                    deleteLine(endMod);

                // Delete old BUILD_MOD content from bottom up including END_MOD meta command
                endMod = Where(ModelName, SaveModActionLineNum);
                modLine = readLine(endMod);
                rc = page.meta.parse(modLine, endMod);
                if (rc == BuildModEndModRc)
                    for (modHere = endMod; modHere >= SaveModBeginLineNum; --modHere)
                        deleteLine(modHere);

                // Write BUILD_MOD END meta command at the BuildMod insert position
                modHere = Where(ModelName, ModBeginLineNum);
                buildModData.action      = QString("END");
                currentStep->buildMod.setValue(buildModData);
                metaString = currentStep->buildMod.format(false/*local*/,false/*global*/);
                insertLine(modHere, metaString, nullptr);

                // Write BUILD_MOD END_MOD meta command above LPub content first line - last to first
                modHere = Where(ModelName, ModActionLineNum);
                buildModData.action      = QString("END_MOD");
                currentStep->buildMod.setValue(buildModData);
                metaString = currentStep->buildMod.format(false,false);
                insertLine(modHere, metaString, nullptr);

                // Write buildMod content last to first
                for (int i = BuildModPieces - 1; i >= 0; --i) {
                    metaString = ViewerModContents.at(i);
                    insertLine(modHere, metaString, nullptr);
                }

                // Write BUILD_MOD BEGIN meta command above buildMod content first line
                buildModData.action      = QString("BEGIN");
                buildModData.buildModKey = BuildModKey;
                currentStep->buildMod.setValue(buildModData);
                metaString = currentStep->buildMod.format(false,false);
                insertLine(modHere, metaString, nullptr);

                if (!imageFile.remove())
                    emit messageSig(LOG_INFO_STATUS, QString("Unable to remove %1")
                                    .arg(assemDirName +  QDir::separator() + imageFile.fileName()));

                endMacro();
            }

            // BuildMod attribute values are calculated in a top down manner

            // Reset to BUILD_MOD BEGIN
            ModBeginLineNum = SaveModBeginLineNum;

            // Reset to BUILD_MOD END_MOD command line number - add 1 for meta command
            ModActionLineNum = ModBeginLineNum + BuildModPieces + 1;

            // Set to BUILD_MOD END command line number - add 1 for meta command
            ModEndLineNum = ModActionLineNum + SaveModPieces + 1;

            // Set to updated number of pieces in the current Step
            ModStepPieces = mViewerPieces.GetSize();

            QVector<int> ModAttributes = { ModBeginLineNum,   // BM_BEGIN_LINE_NUM
                                           ModActionLineNum,  // BM_ACTION_LINE_NUM
                                           ModEndLineNum,     // BM_END_LINE_NUM
                                           ModDisplayPageNum, // BM_DISPLAY_PAGE_NUM
                                           ModelIndex,        // BM_MODEL_NAME_INDEX
                                           ModStepPieces      // BM_STEP_PIECES
                                         };

            insertBuildMod(BuildModKey,
                           ModStepKey,
                           ModAttributes,
                           BuildModApplyRc,
                           ModStepIndex);                     // Unique ID

#ifdef QT_DEBUG_MODE
          emit messageSig(LOG_DEBUG, QString("%1 BuildMod StepIndx: %2, "
                                             "Attributes: %3 %4 %5 %6 %7 %8, "
                                             "StepKey: %9, "
                                             "ModKey: %10")
                                             .arg(edit ? "Update" : "Create")     // 01
                                             .arg(ModStepIndex)                   // 02
                                             .arg(ModBeginLineNum)                // 03
                                             .arg(ModActionLineNum)               // 04
                                             .arg(ModEndLineNum)                  // 05
                                             .arg(ModDisplayPageNum)              // 06
                                             .arg(ModelIndex)                     // 07
                                             .arg(ModStepPieces)                  // 08
                                             .arg(ModStepKey)                     // 09
                                             .arg(BuildModKey));                  // 10
#endif

            // Reset the build mod range
            buildModRange = { 0/*BM_BEGIN_LINE_NUM*/, 0/*BM_ACTION_LINE_NUM*/, -1/*BM_MODEL_INDEX*/ };
        } // buildModRange || edit

        emit progressPermStatusRemoveSig();
    }
}

void Gui::applyBuildModification()
{
    QStringList buildModKeys;
    BuildModDialogGui *buildModDialogGui =
            new BuildModDialogGui();
    buildModDialogGui->getBuildMod(buildModKeys, BuildModApplyRc);

    if (!buildModKeys.size())
        return;

    if (getBuildModStepKey(buildModKeys.first()) == viewerStepKey)
        return;

    int it = lcGetActiveProject()->GetImageType();
    switch(it) {
    case Options::Mt::CSI:
    case Options::Mt::SMP:
    {
        QString metaString;
        bool newCommand = false;
        Where top = currentStep->topOfStep();
        BuildModData buildModData = currentStep->buildMod.value();

        int stepIndex = getBuildModStepIndex(currentStep->top);
        setBuildModAction(buildModKeys.first(), stepIndex, BuildModApplyRc);

        beginMacro("ApplyBuildModContent");

        buildModData.action      = QString("APPLY");
        buildModData.buildModKey = buildModKeys.first();
        currentStep->buildMod.setValue(buildModData);
        metaString = currentStep->buildMod.format(false/*local*/,false/*global*/);
        newCommand = currentStep->buildMod.here() ==  Where();
        currentStep->mi(it)->setMetaAlt(newCommand ? top : currentStep->buildMod.here(), metaString, newCommand);

        endMacro();
    }
        break;
    default: /*Options::Mt::PLI:*/
        break;
    }
}

void Gui::removeBuildModification()
{
    QStringList buildModKeys;
    BuildModDialogGui *buildModDialogGui =
            new BuildModDialogGui();
    buildModDialogGui->getBuildMod(buildModKeys, BuildModRemoveRc);

    if (!buildModKeys.size())
        return;

    if (getBuildModStepKey(buildModKeys.first()) == viewerStepKey)
        return;

    int it = lcGetActiveProject()->GetImageType();
    switch(it) {
    case Options::Mt::CSI:
    case Options::Mt::SMP:
    {
        QString metaString;
        bool newCommand = false;
        Where top = currentStep->topOfStep();
        BuildModData buildModData = currentStep->buildMod.value();

        int stepIndex = getBuildModStepIndex(currentStep->top);
        setBuildModAction(buildModKeys.first(), stepIndex, BuildModRemoveRc);

        beginMacro("RemoveBuildModContent");

        buildModData.action      = QString("REMOVE");
        buildModData.buildModKey = buildModKeys.first();
        currentStep->buildMod.setValue(buildModData);
        metaString = currentStep->buildMod.format(false/*local*/,false/*global*/);
        newCommand = currentStep->buildMod.here() == Where();
        currentStep->mi(it)->setMetaAlt(newCommand ? top : currentStep->buildMod.here(), metaString, newCommand);

        endMacro();
    }
        break;
    default: /*Options::Mt::PLI:*/
        break;
    }

    bool enabled = buildModsSize();
    applyBuildModAct->setEnabled(enabled);
    removeBuildModAct->setEnabled(enabled);
    loadBuildModAct->setEnabled(enabled);
    updateBuildModAct->setEnabled(enabled);
    deleteBuildModAct->setEnabled(enabled);
}

void Gui::loadBuildModification()
{
    QStringList buildModKeys;
    BuildModDialogGui *buildModDialogGui =
            new BuildModDialogGui();
    buildModDialogGui->getBuildMod(buildModKeys, BM_CHANGE);

    if (!buildModKeys.size())
        return;

    int it = lcGetActiveProject()->GetImageType();
    switch(it) {
    case Options::Mt::CSI:
    case Options::Mt::SMP:
    {

        buildModChangeKey = "";

        int buildModDisplayPageNum = getBuildModDisplayPageNumber(buildModKeys.first());

        QString buildModStepKey = getBuildModStepKey(buildModKeys.first());

        if (buildModDisplayPageNum && ! buildModStepKey.isEmpty()) {

            if (buildModDisplayPageNum != displayPageNum) {
                displayPageNum = buildModDisplayPageNum;
                displayPage();
            }

            bool setBuildModStep = currentStep && currentStep->viewerStepKey != buildModStepKey;

            if (isViewerStepMultiStep(buildModStepKey) && setBuildModStep) {
                if (setCurrentStep(buildModStepKey)) {
                    showLine(currentStep->topOfStep());
                    currentStep->loadTheViewer();
                }
            }

            buildModChangeKey = buildModKeys.first();
        }
    }
        break;
    default: /*Options::Mt::PLI:*/
        break;
    }
}

bool Gui::setBuildModChangeKey()
{
    if (!currentStep)
        return false;

    int it = lcGetActiveProject()->GetImageType();
    switch(it) {
    case Options::Mt::CSI:
    case Options::Mt::SMP:
    {
        Rc rc;
        Where walk = currentStep->top;

        QString line = readLine(walk);
        rc =  page.meta.parse(line,walk,false);
        if (rc == StepRc || rc == RotStepRc)
            walk++;   // Advance past STEP meta

        // Parse the step lines
        for ( ;
              walk.lineNumber < subFileSize(walk.modelName);
              walk.lineNumber++) {
            line = readLine(walk);
            Where here(walk.modelName,walk.lineNumber);
            rc =  page.meta.parse(line,here,false);

            switch (rc) {
            case BuildModBeginRc:
                buildModChangeKey = page.meta.LPub.buildMod.key();
                return true;

            // Search until next step/rotstep meta
            case RotStepRc:
            case StepRc:
                return false;

            default:
                break;
            }
        }
    }
        break;
    default: /*Options::Mt::PLI:*/
        break;
    }

    return false;
}

void Gui::updateBuildModification()
{
    if (buildModChangeKey.isEmpty()) {
        setBuildModChangeKey();
        createBuildModification();
    } else {
        createBuildModification();
    }
}

void Gui::deleteBuildModification()
{
    QStringList buildModKeys;
    BuildModDialogGui *buildModDialogGui =
            new BuildModDialogGui();
    buildModDialogGui->getBuildMod(buildModKeys, BM_DELETE);

    if (!buildModKeys.size())
        return;

    int it = lcGetActiveProject()->GetImageType();
    switch(it) {
    case Options::Mt::CSI:
    case Options::Mt::SMP:
    {
        int modBeginLineNum  = getBuildModBeginLineNumber(buildModKeys.first());
        int modActionLineNum = getBuildModActionLineNumber(buildModKeys.first());
        int modEndLineNum    = getBuildModEndLineNumber(buildModKeys.first());
        QString modelName    = getBuildModModelName(buildModKeys.first());

        if (modelName.isEmpty() || !modBeginLineNum || !modActionLineNum || !modEndLineNum) {
            emit messageSig(LOG_ERROR, QString("There was a problem receiving buld mod attributes for key [%1]").arg(buildModKeys.first()));
            return;
        }

        beginMacro("DeleteBuildModContent");

        // delete existing APPLY/REMOVE (action) commands, starting from the bottom of the step
        Rc rc;
        QString buildModKey, modLine;
        Where here, topOfStep, bottomOfStep;
        QMap<int, int> actionsMap = getBuildModActions(buildModKeys.first());
        QList<int> stepIndexes = actionsMap.keys();
        std::sort(stepIndexes.begin(), stepIndexes.end(), std::greater<int>()); // sort stepIndexes descending
        foreach (int stepIndex, stepIndexes) {
            QMap<int, int>::iterator i = actionsMap.find(stepIndex);
            if (i.key()) {                                                      // skip first step at index 0 - deleted later
                if (getBuildModStepIndexHere(i.key(), topOfStep)){
                    if (! getBuildModStepIndexHere(i.key() + 1, bottomOfStep))  // bottom of step is top of next step                                                    // handle last step
                        bottomOfStep = Where(topOfStep.modelName, subFileSize(topOfStep.modelName));
                    for (Where walk = bottomOfStep; walk > topOfStep.lineNumber; --walk) {
                        here = walk;
                        modLine = readLine(here);
                        rc = page.meta.parse(modLine, here);
                        switch (rc) {
                        case BuildModApplyRc:
                        case BuildModRemoveRc:
                            buildModKey = page.meta.LPub.buildMod.key();
                            if (buildModKey == buildModKeys.first())
                                deleteLine(here);
                            break;
                        default:
                            break;
                        }
                    }
                }
            }
        }

        // delete existing BUILD_MOD commands from bottom up, starting at END
        here = Where(modelName, modEndLineNum);
        modLine = readLine(here);
        rc = page.meta.parse(modLine, here);
        if (rc == BuildModEndRc)
            deleteLine(here);

        // delete existing BUILD_MOD commands from bottom up, starting at END_MOD
        here = Where(modelName, modActionLineNum);
        modLine = readLine(here);
        rc = page.meta.parse(modLine, here);
        if (rc == BuildModEndModRc)
            for (Where walk = here; walk >= modBeginLineNum; --walk)
                deleteLine(walk);

        endMacro();
    }
        break;
    default: /*Options::Mt::PLI:*/
        break;
    }

    bool enabled = buildModsSize();
    applyBuildModAct->setEnabled(enabled);
    removeBuildModAct->setEnabled(enabled);
    loadBuildModAct->setEnabled(enabled);
    updateBuildModAct->setEnabled(enabled);
    deleteBuildModAct->setEnabled(enabled);
}

/*********************************************
 *
 * save viewer model if modified
 *
 ********************************************/

bool Gui::saveBuildModification()
{
    Project* Project = lcGetActiveProject();
    if (Project->GetImageType() == Options::Mt::PLI)
        return true;

    if (!Project->IsModified())
        return true;

    QPixmap _icon = QPixmap(":/icons/lpub96.png");
    if (_icon.isNull())
        _icon = QPixmap (":/icons/update.png");

    QMessageBox box;
    box.setWindowIcon(QIcon());
    box.setIconPixmap (_icon);
    box.setTextFormat (Qt::RichText);
    box.setWindowTitle(tr ("%1 Save Model Change").arg(VER_PRODUCTNAME_STR));
    box.setWindowFlags (Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
    QString title = "<b>" +
            (setBuildModChangeKey()
             ? tr("Save model changes to build modification '%1'?").arg(getBuildModChangeKey())
             : tr("Save model changes as build modification?")) + "</b>";
    QString text = tr("Save changes as a build modification to this step?");
    box.setText (title);
    box.setInformativeText (text);
    box.setStandardButtons (QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
    box.setDefaultButton   (QMessageBox::Cancel);
    switch (box.exec())
    {
    default:
    case QMessageBox::Cancel:
        return false;

    case QMessageBox::Yes:
        updateBuildModification();
        break;

    case QMessageBox::No:
        break;
    }

    return true;
}

/*********************************************
 *
 * set viewer step key
 *
 ********************************************/

void Gui::setViewerStepKey(const QString &stepKey, int notPliPart)
{
    viewerStepKey = stepKey;
    currentStep   = nullptr;
    buildModRange = { 0, 0, -1 };
    if (notPliPart)
        setCurrentStep();
}

/*********************************************
 *
 * split viewer step keys
 *
 ********************************************/

QStringList Gui::getViewerStepKeys(bool modelName, bool pliPart, const QString &key)
{
    // viewerStepKey - 3 elements:
    // CSI: 0=modelNameIndex, 1=lineNumber,   2=stepNumber [_dm (displayModel)]
    // SMP: 0=modelNameIndex, 1=lineNumber,   2=stepNumber [_Preview (Submodel Preview)]
    // PLI: 0=partNameString, 1=colourNumber, 2=stepNumber
    QStringList keys = key.isEmpty() ? viewerStepKey.split(";") : key.split(";");
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

  bool Gui::extractStepKey(Where &here, int &stepNumber, const QString &key)
  {
      // viewerStepKey elements CSI: 0=modelName, 1=lineNumber, 2=stepNumber [,3=_dm (displayModel)]
      QStringList keyArgs = getViewerStepKeys(true/*modelName*/, false/*pliPart*/, key);

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

void Gui::setCurrentStep(Step *step, Where here, int stepNumber, int stepType)
{
    bool stepMatch  = false;
    auto calledOutStep = [this, &here, &stepNumber, &stepType] (Step* step, bool &stepMatch)
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
                                    setCurrentStep(step, here, stepNumber, stepType);
                                }
                            }
                        }
                    }
                }
            }
        }
    };

    if (stepType == BM_CALLOUT_STEP && step){
        calledOutStep(step, stepMatch);
    } else if (stepType == BM_MULTI_STEP) {
        for (int i = 0; i < page.list.size() && !stepMatch; i++){
            Range *range = dynamic_cast<Range *>(page.list[i]);
            for (int j = 0; j < range->list.size(); j++){
                if (range->relativeType == RangeType) {
                    step = dynamic_cast<Step *>(range->list[j]);
                    if (stepType == BM_CALLOUT_STEP)
                        calledOutStep(step, stepMatch);
                    else if (step && step->relativeType == StepType)
                        stepMatch = step->stepNumber.number == stepNumber;
                    if (stepMatch)
                        break;
                }
            }
        }
    } else if (stepType == BM_SINGLE_STEP) {
        Range *range = dynamic_cast<Range *>(page.list[0]);
        if (range->relativeType == RangeType) {
            step = dynamic_cast<Step *>(range->list[0]);
            if (stepType == BM_CALLOUT_STEP)
                calledOutStep(step, stepMatch);
            else if (step && step->relativeType == StepType)
                stepMatch = step->stepNumber.number == stepNumber;
        }
    } else if ((step = gStep)) {
        if (stepType == BM_CALLOUT_STEP)
            calledOutStep(step, stepMatch);
        else if (!(stepMatch = step->stepNumber.number == stepNumber))
            step = nullptr;
    }

    currentStep = step;

    if (Preferences::debugLogging && !stepMatch)
        emit messageSig(LOG_DEBUG, QString("%1 Step number %2 for %3 - modelName [%4] topOfStep [%5]")
                                           .arg(stepMatch ? "Match!" : "Oh oh!")
                                           .arg(QString("%1 %2").arg(stepNumber).arg(stepMatch ? "found" : "not found"))
                                           .arg(stepType == BM_MULTI_STEP  ? "multi step"  :
                                                stepType == BM_CALLOUT_STEP   ? "called out"  :
                                                stepType == BM_SINGLE_STEP ? "single step" : "gStep")
                                           .arg(here.modelName).arg(here.lineNumber));
}

bool Gui::setCurrentStep(const QString &key)
{
    Step *step     = nullptr;
    currentStep    = step;
    Where here     = Where();
    int stepNumber = 0;
    int stepType   = 0; /*None*/

    extractStepKey(here, stepNumber, key);

    if (!stepNumber)
        return currentStep;

    QString stepKey = key.isEmpty() ? viewerStepKey : key;

    if (isViewerStepCalledOut(stepKey))
        stepType = BM_CALLOUT_STEP;
    else if (isViewerStepMultiStep(stepKey))
        stepType = BM_MULTI_STEP;
    else if (page.relativeType == SingleStepType && page.list.size())
        stepType = BM_SINGLE_STEP;

    if (stepType || gStep)
        setCurrentStep(step, here, stepNumber, stepType);
    else if (Preferences::debugLogging)
        emit messageSig(LOG_DEBUG, QString("Could not determine step for %1 at step number %2.")
                                           .arg(here.modelName).arg(stepNumber));

    return currentStep;
}

/*********************************************
 *
 * slelcted Line
 *
 ********************************************/

bool Gui::getSelectedLine(int modelIndex, int lineIndex, int source, int &lineNumber) {

    lineNumber        = 0;
    bool currentModel = modelIndex == QString(viewerStepKey[0]).toInt();
    bool newLine      = lineIndex == NEW_PART;
    bool fromViewer   = source > EDITOR_LINE;

    if (newLine) {
        emit messageSig(LOG_TRACE, QString("New viewer part modelName [%1]")
                                           .arg(getSubmodelName(modelIndex)));
        return false;

    } else if (currentModel) {

        if (!currentStep)
            return false;

        if (Preferences::debugLogging) {
            emit messageSig(LOG_TRACE, QString("LPub Step lineIndex size: %1 item(s)")
                                                .arg(currentStep->lineTypeIndexes.size()));
            for (int i = 0; i < currentStep->lineTypeIndexes.size(); ++i)
                emit messageSig(LOG_TRACE, QString(" -LPub Part lineNumber [%1] at step line lineIndex [%2] - specified lineIndex [%3]")
                                                   .arg(currentStep->lineTypeIndexes.at(i)).arg(i).arg(lineIndex));
        }

        if (fromViewer)      // input relativeIndes
            lineNumber = currentStep->getLineTypeRelativeIndex(lineIndex);
        else                 // input lineTypeIndex
            lineNumber = currentStep->getLineTypeIndex(lineIndex);

    } else if (modelIndex != NEW_MODEL) {

        if (fromViewer)      // input relativeIndes
            lineNumber = getLineTypeRelativeIndex(modelIndex,lineIndex); // return lineTypeIndex - part lineNumber
        else                 // input lineTypeIndex
            lineNumber = getLineTypeIndex(modelIndex,lineIndex);         // return relativeIndex - step line lineIndex
    } else
        return false;

    return lineNumber;
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
        int lineNumber    = 0;
        int lineIndex     = NEW_PART;
        int modelIndex    = NEW_MODEL;
        QString modelName = "undefined";
        if (indexes.size()) {
            modelName  = getSubmodelName(indexes.at(0).modelIndex);
            modelIndex = indexes.at(0).modelIndex;
        } else if (!viewerStepKey.isEmpty()) {
            modelName  = getSubmodelName(QString(viewerStepKey[0]).toInt());
            modelIndex = getSubmodelIndex(modelName);
        }

        if (Preferences::debugLogging) {
            if (modelIndex != NEW_MODEL && source == VIEWER_MOD)
                emit messageSig(LOG_TRACE, QString("Submodel lineIndex size: %1 item(s)")
                                .arg(ldrawFile.getLineTypeRelativeIndexes(modelIndex)->size()));
        }

        for (int i = 0; i < indexes.size(); ++i) {

            lineIndex = indexes.at(i).lineIndex;
            // New part lines are added in createBuildModification() routine
            if (lineIndex != NEW_PART) {
                validLine = getSelectedLine(modelIndex, lineIndex, source, lineNumber);
                lines.append(lineNumber);
            }

            if (validLine) {
                if (fromViewer && source == VIEWER_MOD) {
                    if (buildModRange.first()) {
                        if (lineNumber < buildModRange.first())
                            buildModRange[BM_BEGIN_LINE_NUM] = lineNumber;
                        else if (lineNumber > buildModRange.last())
                            buildModRange[BM_ACTION_LINE_NUM] = lineNumber;
                        buildModRange[BM_MODEL_INDEX] = modelIndex;
                    } else {
                        buildModRange = { lineNumber, lineNumber, modelIndex };
                    }
                    createBuildModAct->setEnabled(true);
                }
            }

            if (Preferences::debugLogging) {
                QString Message;
                if (fromViewer) {
                    if (lineIndex == NEW_PART) {
                        Message = tr("New viewer part specified at step %1, modelName: [%2]")
                                     .arg(currentStep->stepNumber.number)
                                     .arg(modelName);
                    } else if (validLine) {
                        Message = tr("Selected part modelName [%1] lineNumber: [%2] at step line index [%3]")
                                     .arg(modelName).arg(lineNumber).arg(lineIndex);
                    } else {
                        Message = tr("Invalid part lineNumber [%1] for step line index [%2]")
                                     .arg(lineNumber).arg(lineIndex);
                    }
                } else if (validLine) { // valid and not from viewer
                    Message = tr("Selected part modelName [%1] lineNumber: [%2] at step line index [%3]")
                                 .arg(modelName).arg(lineIndex).arg(lineNumber);
                } else {                // invalid and not from viewer
                    Message = tr("Invalid part lineNumber [%1] for step line index [%2]") // index and number flipped
                                 .arg(lineIndex).arg(lineNumber);
                }
                emit messageSig(LOG_TRACE, Message);
            }
        }

        if (fromViewer) {
            if (source == VIEWER_MOD) {
                emit highlightSelectedLinesSig(lines);
            } else if (source == VIEWER_DEL) {
                createBuildModAct->setEnabled(true);
                updateBuildModAct->setEnabled(buildModsSize());
                emit messageSig(LOG_TRACE, tr("Delete viewer part(s) specified at step %1, modelName: [%2]")
                                              .arg(currentStep->stepNumber.number)
                                              .arg(modelName));
            }
        } else {
            emit setSelectedPiecesSig(lines);
        }
    }
}
