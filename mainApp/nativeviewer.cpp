/****************************************************************************
**
** Copyright (C) 2019 - 2021 Trevor SANDY. All rights reserved.
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
#include "editwindow.h"
#include "lpub_preferences.h"
#include "messageboxresizable.h"
#include "step.h"
#include "metagui.h"
#include "paths.h"

#include "lc_viewwidget.h"
#include "lc_previewwidget.h"
#include "pieceinf.h"
#include "lc_library.h"
#include "project.h"
#include "lc_mainwindow.h"

#include "lc_qutils.h"
#include "lc_profile.h"
#include "light.h"
#include "camera.h"
#include "piece.h"
#include "group.h"
#include "lc_view.h"
#include "application.h"
#include "lc_partselectionwidget.h"
#include "lc_setsdatabasedialog.h"

const QStringList BuildModChangeTriggers = QStringList()
        << "Deleting" << "Adding Piece"
        << "Move"     << "Rotate"
        << "Moving"   << "Rotating"
        << "Painting" << "New Model"
        << "Cutting"  << "Pasting"
        << "Removing Step" << "Inserting Step"
        << "Duplicating Pieces" << "New Model"
        << "Ungrouping" << "Grouping" << "Inlining"
           ;

void Gui::create3DActions()
{
    QIcon CreateBuildModIcon;
    CreateBuildModIcon.addFile(":/resources/buildmodcreate.png");
    CreateBuildModIcon.addFile(":/resources/buildmodcreate16.png");
    createBuildModAct = new QAction(CreateBuildModIcon,tr("Create Build Modification - Shift+J"),this);
    createBuildModAct->setStatusTip(tr("Create a new build modification for this step - Shift+J"));
    createBuildModAct->setShortcut(tr("Shift+J"));
    connect(createBuildModAct, SIGNAL(triggered()), this, SLOT(createBuildModification()));

    QIcon UpdateBuildModIcon;
    UpdateBuildModIcon.addFile(":/resources/buildmodupdate.png");
    UpdateBuildModIcon.addFile(":/resources/buildmodupdate16.png");
    updateBuildModAct = new QAction(UpdateBuildModIcon,tr("Update Build Modification - Shift+K"),this);
    updateBuildModAct->setEnabled(false);
    updateBuildModAct->setStatusTip(tr("Commit changes to the current build modification - Shift+K"));
    updateBuildModAct->setShortcut(tr("Shift+K"));
    connect(updateBuildModAct, SIGNAL(triggered()), this, SLOT(updateBuildModification()));

    QIcon ApplyBuildModIcon;
    ApplyBuildModIcon.addFile(":/resources/buildmodapply.png");
    ApplyBuildModIcon.addFile(":/resources/buildmodapply16.png");
    applyBuildModAct = new QAction(ApplyBuildModIcon,tr("Apply Build Modification..."),this);
    applyBuildModAct->setEnabled(false);
    applyBuildModAct->setStatusTip(tr("Apply existing build modification to this step"));
    connect(applyBuildModAct, SIGNAL(triggered()), this, SLOT(applyBuildModification()));

    QIcon RemoveBuildModIcon;
    RemoveBuildModIcon.addFile(":/resources/buildmodremove.png");
    RemoveBuildModIcon.addFile(":/resources/buildmodremove16.png");
    removeBuildModAct = new QAction(RemoveBuildModIcon,tr("Remove Build Modification..."),this);
    removeBuildModAct->setEnabled(false);
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

    defaultCameraPropertiesAct = new QAction(tr("Display Properties"),this);
    defaultCameraPropertiesAct->setStatusTip(tr("Display default camera properties in Properties tab"));
    defaultCameraPropertiesAct->setCheckable(true);
    defaultCameraPropertiesAct->setChecked(GetPreferences().mDefaultCameraProperties);
    connect(defaultCameraPropertiesAct, SIGNAL(triggered()), this, SLOT(showDefaultCameraProperties()));

    TransformAction = new QAction(tr("Transform"), this);
    TransformAction->setStatusTip(tr("Transform Options"));
    TransformAction->setIcon(QIcon(":/resources/edit_transform_relative.png"));

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
    ViewLookAtIcon.addFile(":/resources/lookat.png");
    ViewLookAtIcon.addFile(":/resources/lookat.png");
    gMainWindow->mActions[LC_VIEW_LOOK_AT]->setIcon(ViewLookAtIcon);
    gMainWindow->mActions[LC_VIEW_LOOK_AT]->setShortcut(tr("Shift+O"));
    gMainWindow->mActions[LC_VIEW_LOOK_AT]->setStatusTip(tr("Position camera so selection is placed at the viewport center - Shift+O"));

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

     gMainWindow->mActions[LC_EDIT_ACTION_SELECT]->setMenu(gMainWindow->GetSelectionModeMenu());

     TransformAction->setMenu(gMainWindow->GetTransformMenu());

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
     // 3DViewer Preferences menu
     ViewerMenu->addAction(gMainWindow->mActions[LC_VIEW_PREFERENCES]);
     ViewerMenu->addSeparator();
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
     ViewerMenu->addAction(gMainWindow->mActions[LC_PIECE_PAINT_SELECTED]);
     ViewerMenu->addAction(gMainWindow->mActions[LC_PIECE_EDIT_SELECTED_SUBMODEL]);
     ViewerMenu->addAction(gMainWindow->mActions[LC_PIECE_EDIT_END_SUBMODEL]);
     ViewerMenu->addSeparator();
     // Show/Hide Piece menus
     ViewerMenu->addAction(gMainWindow->mActions[LC_PIECE_HIDE_SELECTED]);
     ViewerMenu->addAction(gMainWindow->mActions[LC_PIECE_UNHIDE_SELECTED]);
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
     gMainWindow->GetToolsMenu()->addAction(gMainWindow->mActions[LC_EDIT_ACTION_ZOOM]);
     gMainWindow->GetToolsMenu()->addAction(gMainWindow->mActions[LC_VIEW_ZOOM_EXTENTS]);
     gMainWindow->GetToolsMenu()->addAction(gMainWindow->mActions[LC_EDIT_ACTION_ZOOM_REGION]);
     gMainWindow->GetToolsMenu()->addAction(gMainWindow->mActions[LC_EDIT_ACTION_ROTATE_VIEW]);
     gMainWindow->GetToolsMenu()->addAction(gMainWindow->mActions[LC_EDIT_ACTION_PAN]);
     gMainWindow->GetToolsMenu()->addAction(viewpointGroupAct);
     gMainWindow->GetToolsMenu()->addSeparator();
     gMainWindow->GetToolsMenu()->addAction(TransformAction);
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
     cameraMenu->addMenu(gMainWindow->GetCameraMenu());
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

    gMainWindow->GetToolsToolBar()->addSeparator();
    gMainWindow->GetToolsToolBar()->addAction(gMainWindow->mActions[LC_EDIT_ACTION_SELECT]);
    gMainWindow->GetToolsToolBar()->addAction(gMainWindow->mActions[LC_EDIT_ACTION_ROTATE]);
    gMainWindow->GetToolsToolBar()->addAction(gMainWindow->mActions[LC_EDIT_ACTION_MOVE]);
    gMainWindow->GetToolsToolBar()->addAction(gMainWindow->mActions[LC_EDIT_ACTION_DELETE]);
    gMainWindow->GetToolsToolBar()->addAction(gMainWindow->mActions[LC_EDIT_ACTION_PAINT]);
    gMainWindow->GetToolsToolBar()->addAction(gMainWindow->mActions[LC_EDIT_ACTION_COLOR_PICKER]);
    gMainWindow->GetToolsToolBar()->addSeparator();
    gMainWindow->GetToolsToolBar()->addAction(gMainWindow->mActions[LC_EDIT_ACTION_ROTATESTEP]);
    gMainWindow->GetToolsToolBar()->addAction(createBuildModAct);
    gMainWindow->GetToolsToolBar()->addSeparator();
    gMainWindow->GetToolsToolBar()->addAction(gMainWindow->mActions[LC_EDIT_ACTION_INSERT]);
    gMainWindow->GetToolsToolBar()->addAction(lightGroupAct);
    gMainWindow->GetToolsToolBar()->addAction(gMainWindow->mActions[LC_EDIT_ACTION_CAMERA]);
    gMainWindow->GetToolsToolBar()->addSeparator();
    gMainWindow->GetToolsToolBar()->addAction(gMainWindow->mActions[LC_VIEW_LOOK_AT]);
    gMainWindow->GetToolsToolBar()->addAction(gMainWindow->mActions[LC_EDIT_ACTION_ZOOM]);
    gMainWindow->GetToolsToolBar()->addAction(gMainWindow->mActions[LC_VIEW_ZOOM_EXTENTS]);
    gMainWindow->GetToolsToolBar()->addAction(gMainWindow->mActions[LC_EDIT_ACTION_ZOOM_REGION]);
    gMainWindow->GetToolsToolBar()->addAction(gMainWindow->mActions[LC_EDIT_ACTION_ROTATE_VIEW]);
    gMainWindow->GetToolsToolBar()->addAction(gMainWindow->mActions[LC_EDIT_ACTION_PAN]);
    gMainWindow->GetToolsToolBar()->addAction(viewpointGroupAct);
    gMainWindow->GetToolsToolBar()->addSeparator();
    gMainWindow->GetToolsToolBar()->addAction(TransformAction);
    gMainWindow->GetToolsToolBar()->addAction(MoveAction);
    gMainWindow->GetToolsToolBar()->addAction(AngleAction); // Snap Rotations to Fixed Intervals menu item
    gMainWindow->GetPartsToolBar()->setWindowTitle("Tools Toolbar");
}

void Gui::initiaizeNativeViewer()
{
    connect(this,        SIGNAL(clearViewerWindowSig()),                   gMainWindow, SLOT(NewProject()));
    connect(this,        SIGNAL(setSelectedPiecesSig(QVector<int>&)),      gMainWindow, SLOT(SetSelectedPieces(QVector<int>&)));

    connect(gMainWindow, SIGNAL(SetRotStepMeta()),                         this,        SLOT(SetRotStepMeta()));
    connect(gMainWindow, SIGNAL(SetRotStepAngleX(float,bool)),             this,        SLOT(SetRotStepAngleX(float,bool)));
    connect(gMainWindow, SIGNAL(SetRotStepAngleY(float,bool)),             this,        SLOT(SetRotStepAngleY(float,bool)));
    connect(gMainWindow, SIGNAL(SetRotStepAngleZ(float,bool)),             this,        SLOT(SetRotStepAngleZ(float,bool)));
    connect(gMainWindow, SIGNAL(SetRotStepTransform(QString&,bool)),       this,        SLOT(SetRotStepTransform(QString&,bool)));
    connect(gMainWindow, SIGNAL(GetRotStepMeta()),                         this,        SLOT(GetRotStepMeta()));
    connect(gMainWindow, SIGNAL(SetActiveModelSig(const QString&,bool)),   this,        SLOT(SetActiveModel(const QString&,bool)));
    connect(gMainWindow, SIGNAL(SelectedPartLinesSig(QVector<TypeLine>&,PartSource)),this,SLOT(SelectedPartLines(QVector<TypeLine>&,PartSource)));
    connect(gMainWindow, SIGNAL(UpdateUndoRedoSig(const QString&,const QString&)),   this,SLOT(UpdateViewerUndoRedo(const QString&,const QString&)));
    connect(gMainWindow, SIGNAL(TogglePreviewWidgetSig(bool)),             this,        SLOT(togglePreviewWidget(bool)));

    enable3DActions(false);
}

void Gui::enable3DActions(bool enable)
{
    if (enable)
        enableBuildModMenuAndActions();
    else
        createBuildModAct->setEnabled(enable);

    lightGroupAct->setEnabled(enable);
    viewpointGroupAct->setEnabled(enable);

    blenderRenderAct->setEnabled(enable);
    blenderImportAct->setEnabled(enable);
    povrayRenderAct->setEnabled(enable);
    ViewerExportMenu->setEnabled(enable);

    GetToolsToolBar()->setEnabled(enable);
    GetTimelineToolBar()->setEnabled(enable);
    GetPropertiesToolBar()->setEnabled(enable);
    GetPartsToolBar()->setEnabled(enable);
    GetColorsToolBar()->setEnabled(enable);

    GetCameraMenu()->setEnabled(enable);
    GetToolsMenu()->setEnabled(enable);
    GetViewpointMenu()->setEnabled(enable);
    GetProjectionMenu()->setEnabled(enable);
    GetShadingMenu()->setEnabled(enable);

    //Window
    gMainWindow->mActions[LC_VIEW_SPLIT_HORIZONTAL]->setEnabled(enable);
    gMainWindow->mActions[LC_VIEW_SPLIT_VERTICAL]->setEnabled(enable);
    gMainWindow->mActions[LC_VIEW_REMOVE_VIEW]->setEnabled(enable);
    gMainWindow->mActions[LC_VIEW_RESET_VIEWS]->setEnabled(enable);
    //File
    gMainWindow->mActions[LC_FILE_RENDER]->setEnabled(enable);
    gMainWindow->mActions[LC_FILE_SAVE_IMAGE]->setEnabled(enable);
    gMainWindow->mActions[LC_FILE_SAVE_IMAGE]->setEnabled(enable);
    //Export
    gMainWindow->mActions[LC_FILE_EXPORT_3DS]->setEnabled(enable);
    gMainWindow->mActions[LC_FILE_EXPORT_BRICKLINK]->setEnabled(enable);
    gMainWindow->mActions[LC_FILE_EXPORT_COLLADA]->setEnabled(enable);
    gMainWindow->mActions[LC_FILE_EXPORT_CSV]->setEnabled(enable);
    gMainWindow->mActions[LC_FILE_EXPORT_HTML]->setEnabled(enable);
    gMainWindow->mActions[LC_FILE_EXPORT_POVRAY]->setEnabled(enable);
    gMainWindow->mActions[LC_FILE_EXPORT_WAVEFRONT]->setEnabled(enable);
    //Tools
    //mActions[LC_EDIT_ACTION_ROTATESTEP]->setEnabled(enable);
    gMainWindow->mActions[LC_EDIT_ACTION_CAMERA]->setEnabled(enable);
    gMainWindow->mActions[LC_EDIT_ACTION_CLEAR_TRANSFORM]->setEnabled(enable);
    gMainWindow->mActions[LC_EDIT_ACTION_SELECT]->setEnabled(enable);
    gMainWindow->mActions[LC_EDIT_ACTION_ROTATE]->setEnabled(enable);
    gMainWindow->mActions[LC_EDIT_ACTION_PAN]->setEnabled(enable);
    gMainWindow->mActions[LC_EDIT_ACTION_ROTATE_VIEW]->setEnabled(enable);
    gMainWindow->mActions[LC_EDIT_ACTION_ROLL]->setEnabled(enable);
    gMainWindow->mActions[LC_EDIT_ACTION_ZOOM_REGION]->setEnabled(enable);
    gMainWindow->mActions[LC_EDIT_ACTION_MOVE]->setEnabled(enable);
    gMainWindow->mActions[LC_EDIT_ACTION_INSERT]->setEnabled(enable);
    gMainWindow->mActions[LC_EDIT_ACTION_PAINT]->setEnabled(enable);
    gMainWindow->mActions[LC_EDIT_ACTION_COLOR_PICKER]->setEnabled(enable);
    gMainWindow->mActions[LC_EDIT_ACTION_DELETE]->setEnabled(enable);
    //Shading
    gMainWindow->mActions[LC_VIEW_SHADING_WIREFRAME]->setEnabled(enable);
    gMainWindow->mActions[LC_VIEW_SHADING_FLAT]->setEnabled(enable);
    gMainWindow->mActions[LC_VIEW_SHADING_DEFAULT_LIGHTS]->setEnabled(enable);
    //View
    gMainWindow->mActions[LC_EDIT_ACTION_ZOOM]->setEnabled(enable);
    gMainWindow->mActions[LC_VIEW_ZOOM_EXTENTS]->setEnabled(enable);
    gMainWindow->mActions[LC_VIEW_LOOK_AT]->setEnabled(enable);
    gMainWindow->mActions[LC_VIEW_SPLIT_HORIZONTAL]->setEnabled(enable);
    gMainWindow->mActions[LC_VIEW_SPLIT_VERTICAL]->setEnabled(enable);
    gMainWindow->mActions[LC_VIEW_REMOVE_VIEW]->setEnabled(enable);
    gMainWindow->mActions[LC_VIEW_RESET_VIEWS]->setEnabled(enable);
    gMainWindow->mActions[LC_VIEW_VIEWPOINT_FRONT]->setEnabled(enable);
    gMainWindow->mActions[LC_VIEW_VIEWPOINT_BACK]->setEnabled(enable);
    gMainWindow->mActions[LC_VIEW_VIEWPOINT_LEFT]->setEnabled(enable);
    gMainWindow->mActions[LC_VIEW_VIEWPOINT_RIGHT]->setEnabled(enable);
    gMainWindow->mActions[LC_VIEW_VIEWPOINT_TOP]->setEnabled(enable);
    gMainWindow->mActions[LC_VIEW_VIEWPOINT_BOTTOM]->setEnabled(enable);
    gMainWindow->mActions[LC_VIEW_VIEWPOINT_HOME]->setEnabled(enable);
    gMainWindow->mActions[LC_VIEW_CAMERA_NONE]->setEnabled(enable);
    gMainWindow->mActions[LC_VIEW_PROJECTION_PERSPECTIVE]->setEnabled(enable);
    gMainWindow->mActions[LC_VIEW_PROJECTION_ORTHO]->setEnabled(enable);
}

void Gui::halt3DViewer(bool enable)
{
    if (!gMainWindow)
        return;

    if(enable){
        GetToolsToolBar()->setEnabled(false);
        gMainWindow->menuBar()->setEnabled(false);
    } else {
        GetToolsToolBar()->setEnabled(true);
        gMainWindow->menuBar()->setEnabled(true);
    }

    messageSig(LOG_INFO, QString("3D Viewer %1").arg(enable ? "stopped" :"resumed"));
}

void Gui::create3DDockWindows()
{
    viewerDockWindow = new QDockWidget(trUtf8("3DViewer"), this);
    viewerDockWindow->setObjectName("ModelDockWindow");
    viewerDockWindow->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    viewerDockWindow->setWidget(gMainWindow);
    addDockWidget(Qt::RightDockWidgetArea, viewerDockWindow);
    viewMenu->addAction(viewerDockWindow->toggleViewAction());

    tabifyDockWidget(viewerDockWindow, fileEditDockWindow);

    //Part Selection
    gMainWindow->GetPartsToolBar()->setWindowTitle(trUtf8("Parts"));
    gMainWindow->GetPartsToolBar()->setObjectName("PartsToolbar");
    gMainWindow->GetPartsToolBar()->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    addDockWidget(Qt::RightDockWidgetArea, gMainWindow->GetPartsToolBar());
    viewMenu->addAction(gMainWindow->GetPartsToolBar()->toggleViewAction());

    tabifyDockWidget(viewerDockWindow/*gMainWindow->GetPropertiesToolBar()*/, gMainWindow->GetPartsToolBar());

    //Colors Selection
    gMainWindow->GetColorsToolBar()->setWindowTitle(trUtf8("Colors"));
    gMainWindow->GetColorsToolBar()->setObjectName("ColorsToolbar");
    gMainWindow->GetColorsToolBar()->setAllowedAreas(
                Qt::TopDockWidgetArea  | Qt::BottomDockWidgetArea |
                Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    addDockWidget(Qt::RightDockWidgetArea, gMainWindow->GetColorsToolBar());
    viewMenu->addAction(gMainWindow->GetColorsToolBar()->toggleViewAction());

    tabifyDockWidget(viewerDockWindow/*gMainWindow->GetPartsToolBar()*/, gMainWindow->GetColorsToolBar());

    //Properties
    gMainWindow->GetPropertiesToolBar()->setWindowTitle(trUtf8("Properties"));
    gMainWindow->GetPropertiesToolBar()->setObjectName("PropertiesToolbar");
    gMainWindow->GetPropertiesToolBar()->setAllowedAreas(
                Qt::TopDockWidgetArea  | Qt::BottomDockWidgetArea |
                Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    addDockWidget(Qt::RightDockWidgetArea, gMainWindow->GetPropertiesToolBar());
    viewMenu->addAction(gMainWindow->GetPropertiesToolBar()->toggleViewAction());

    tabifyDockWidget(viewerDockWindow/*gMainWindow->GetTimelineToolBar()*/, gMainWindow->GetPropertiesToolBar());

    //Timeline
    gMainWindow->GetTimelineToolBar()->setWindowTitle(trUtf8("Timeline"));
    gMainWindow->GetTimelineToolBar()->setObjectName("TimelineToolbar");
    gMainWindow->GetTimelineToolBar()->setAllowedAreas(
                Qt::TopDockWidgetArea  | Qt::BottomDockWidgetArea |
                Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    addDockWidget(Qt::RightDockWidgetArea, gMainWindow->GetTimelineToolBar());
    viewMenu->addAction(gMainWindow->GetTimelineToolBar()->toggleViewAction());

    tabifyDockWidget(viewerDockWindow, gMainWindow->GetTimelineToolBar());

    // Preview
    if (GetPreferences().mPreviewPosition == lcPreviewPosition::Dockable)
        createPreviewWidget();

    // Status Bar and Window Flags
    connect(viewerDockWindow,                    SIGNAL (topLevelChanged(bool)), this, SLOT (toggleLCStatusBar(bool)));
    connect(viewerDockWindow,                    SIGNAL (topLevelChanged(bool)), this, SLOT (enableWindowFlags(bool)));
    connect(gMainWindow->GetTimelineToolBar(),   SIGNAL (topLevelChanged(bool)), this, SLOT (enableWindowFlags(bool)));
    connect(gMainWindow->GetPropertiesToolBar(), SIGNAL (topLevelChanged(bool)), this, SLOT (enableWindowFlags(bool)));
    connect(gMainWindow->GetColorsToolBar(),     SIGNAL (topLevelChanged(bool)), this, SLOT (enableWindowFlags(bool)));
    connect(gMainWindow->GetPartsToolBar(),      SIGNAL (topLevelChanged(bool)), this, SLOT (enableWindowFlags(bool)));
}

bool Gui::createPreviewWidget()
{
    gMainWindow->CreatePreviewWidget();

    if (gMainWindow->GetPreviewWidget()) {
        previewDockWindow = new QDockWidget(tr("3DPreview"), this);
        previewDockWindow->setWindowTitle(tr("3DPreview"));
        previewDockWindow->setObjectName("PreviewDockWindow");
        previewDockWindow->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
        previewDockWindow->setWidget(gMainWindow->GetPreviewWidget());
        addDockWidget(Qt::RightDockWidgetArea, previewDockWindow);
        viewMenu->addAction(previewDockWindow->toggleViewAction());

        tabifyDockWidget(viewerDockWindow, previewDockWindow);

        connect(previewDockWindow, SIGNAL (topLevelChanged(bool)), this, SLOT (enableWindowFlags(bool)));

        return true;
    } else {
        messageSig(LOG_ERROR, QString("Preview failed."));
    }
    return false;
}

void Gui::previewPiece(const QString &partType, int colorCode, bool dockable, QRect parentRect, QPoint position)
{
    if (dockable) {
        if (gMainWindow)
            gMainWindow->PreviewPiece(partType, colorCode, false);
        return;
    } else {
        preview = new lcPreview();
        lcViewWidget* viewWidget = new lcViewWidget(nullptr, preview);

        if (preview && viewWidget)
        {
            viewWidget->setAttribute(Qt::WA_DeleteOnClose, true);

            if (preview->SetCurrentPiece(partType, colorCode))
            {
                viewWidget->SetPreviewPosition(parentRect, position);
                return;
            }
        }
    }

    QMessageBox::information(this, tr("Error"), tr("Part preview for '%1' failed.").arg(partType));
}

void Gui::updatePreview()
{
    if (previewDockWindow) {
        gMainWindow->GetPreviewWidget()->UpdatePreview();
    } else if (preview) {
        preview->UpdatePreview();
    }
}

void Gui::togglePreviewWidget(bool visible)
{
    if (previewDockWindow) {
        if (visible)
            previewDockWindow->show();
        else
            previewDockWindow->hide();
        QList<QAction*> viewActions = viewMenu->actions();
        Q_FOREACH (QAction *viewAct, viewActions) {
            if (viewAct->text() == "3DPreview") {
                viewAct->setChecked(visible);
                viewAct->setVisible(visible);
                messageSig(LOG_DEBUG, QString("%1 window %2.")
                           .arg(viewAct->text()).arg(visible ? "Displayed" : "Hidden"));
                break;
            }
        }
    } else if (visible) {
        createPreviewWidget();
    }
}

void Gui::enableWindowFlags(bool detached)
{
    if (detached) {
        QDockWidget *dockWidget = qobject_cast<QDockWidget *>(sender());
        dockWidget->setWindowFlags(Qt::CustomizeWindowHint |
                                   Qt::Window |
                                   Qt::WindowMinimizeButtonHint |
                                   Qt::WindowMaximizeButtonHint |
                                   Qt::WindowCloseButtonHint);
        dockWidget->show();
    }
}

void Gui::ClearPreviewWidget()
{
    if (gMainWindow && gMainWindow->GetPreviewWidget())
         gMainWindow->GetPreviewWidget()->ClearPreview();
}

void Gui::UpdateViewerUndoRedo(const QString& UndoText, const QString& RedoText)
{
    undoAct->setData(UndoText);
    if (!UndoText.isEmpty())
    {
        viewerUndo = true;
        undoAct->setEnabled(true);
        undoAct->setText(QString(tr("&Undo %1")).arg(UndoText));
    }
    else
    {
        viewerUndo = false;
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
    applyLightAct->setEnabled(lcGetActiveProject()->GetImageType() == Options::CSI);
}

void Gui::applyLightSettings()
{
    int it = lcGetActiveProject()->GetImageType();
    if (it != Options::CSI)
        return;

    if (currentStep){

        Meta meta;
        LightData lightData = meta.LeoCad.light.value();
        LightMeta lightMeta = meta.LeoCad.light;
        lightMeta.setValue(lightData);

        lcView* ActiveView = gMainWindow->GetActiveView();

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

        for (lcLight* Light : ActiveModel->GetLights()) {

            emit messageSig(LOG_INFO, QString("Setting Light [%1]").arg(Light->mName));

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
            QString lightKey = QString("%1 %2").arg(Type).arg(Light->mName);
            if (currentStep->lightList.contains(lightKey))
                lightMeta.setValue(currentStep->lightList[lightKey]);

            // Type and Name
            lightMeta.lightType.setValue(Type);
            metaString = lightMeta.lightType.format(false,false);
            metaString.append(QString(" NAME \"%1\"").arg(Light->mName));
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

        lcView* ActiveView = gMainWindow->GetActiveView();

        if (!ActiveView)
            return;

        SettingsMeta cameraMeta;

        if (autoCenterSelectionAct->isChecked())
            ActiveView->LookAt();

        lcCamera* Camera = ActiveView->GetCamera();

        auto validCameraFoV = [this, &cameraMeta, &Camera] ()
        {
            float result = qRound(Camera->m_fovy);

            if (Preferences::preferredRenderer != RENDERER_NATIVE)
                  // e.g.            30.0  +                 0.01         - 30.0
                   result = Camera->m_fovy + cameraMeta.cameraFoV.value() - GetPreferences().mCFoV;

            return qRound(result);
        };

        auto notEqual = [] (const float v1, const float v2)
        {
            return qAbs(v1 - v2) > 0.1f;
        };

        emit messageSig(LOG_INFO, QString("Setting %1 Camera").arg(Camera->GetName().isEmpty() ? "Default" : Camera->GetName()));

        QString imageFileName;

        int it = lcGetActiveProject()->GetImageType();
        switch(it){
        case Options::PLI:
            cameraMeta.cameraAngles   = currentStep->pli.pliMeta.cameraAngles;
            cameraMeta.cameraDistance = currentStep->pli.pliMeta.cameraDistance;
            cameraMeta.modelScale     = currentStep->pli.pliMeta.modelScale;
            cameraMeta.cameraFoV      = currentStep->pli.pliMeta.cameraFoV;
            cameraMeta.cameraZNear    = currentStep->pli.pliMeta.cameraZNear;
            cameraMeta.cameraZFar     = currentStep->pli.pliMeta.cameraZFar;
            cameraMeta.isOrtho        = currentStep->pli.pliMeta.isOrtho;
            cameraMeta.imageSize      = currentStep->pli.pliMeta.imageSize;
            cameraMeta.target         = currentStep->pli.pliMeta.target;
            cameraMeta.position       = currentStep->pli.pliMeta.position;
            cameraMeta.upvector       = currentStep->pli.pliMeta.upvector;
            break;
        case Options::SMP:
            cameraMeta.cameraAngles   = currentStep->subModel.subModelMeta.cameraAngles;
            cameraMeta.cameraDistance = currentStep->subModel.subModelMeta.cameraDistance;
            cameraMeta.modelScale     = currentStep->subModel.subModelMeta.modelScale;
            cameraMeta.cameraFoV      = currentStep->subModel.subModelMeta.cameraFoV;
            cameraMeta.cameraZNear    = currentStep->subModel.subModelMeta.cameraZNear;
            cameraMeta.cameraZFar     = currentStep->subModel.subModelMeta.cameraZFar;
            cameraMeta.isOrtho        = currentStep->subModel.subModelMeta.isOrtho;
            cameraMeta.imageSize      = currentStep->subModel.subModelMeta.imageSize;
            cameraMeta.target         = currentStep->subModel.subModelMeta.target;
            cameraMeta.position       = currentStep->subModel.subModelMeta.position;
            cameraMeta.upvector       = currentStep->subModel.subModelMeta.upvector;
            break;
        default: /*Options::CSI:*/
            cameraMeta                = currentStep->csiStepMeta;
            imageFileName             = currentStep->pngName;
            break;
        }

        QString metaString;
        bool newCommand = false;
        bool clearStepCache = false;
        lcVector3 ldrawVector;
        Where undefined = Where();
        Where top = currentStep->topOfStep();
        Where bottom = currentStep->bottomOfStep();

        float Latitude, Longitude, Distance;
        Camera->GetAngles(Latitude, Longitude, Distance);

        bool applyTarget = !(Camera->mTargetPosition[0] == 0.0f  &&
                             Camera->mTargetPosition[1] == 0.0f  &&
                             Camera->mTargetPosition[2] == 0.0f);
        bool applyPosition = !(Camera->mPosition[0] == 0.0f &&
                             Camera->mPosition[1] == -1.0f &&
                             Camera->mPosition[2] == 0.0);
        bool applyUpVector = applyPosition && !(Camera->mUpVector[0] == 0.0f &&
                             Camera->mUpVector[1] == 0.0f &&
                             Camera->mUpVector[2] == 1.0);
        bool applyZPlanes = applyUpVector;


        beginMacro("CameraSettings");

        // execute first in last out

        if (applyUpVector) {

            clearStepCache = true;
            if (QFileInfo(imageFileName).exists())
                clearStepCSICache(imageFileName);

            // Camera Globe, Switch Y and Z axis with -Y(LC -Z) in the up direction
            ldrawVector = lcVector3LeoCADToLDraw(Camera->mUpVector);
            cameraMeta.upvector.setValues(ldrawVector[0], ldrawVector[1], ldrawVector[2]);
            metaString = cameraMeta.upvector.format(true/*local*/,false/*global*/);
            newCommand = cameraMeta.upvector.here() == undefined;
            currentStep->mi(it)->setMetaAlt(newCommand ? top : cameraMeta.upvector.here(), metaString, newCommand);
        }

        if (applyTarget) {

            clearStepCache = true;
            if (QFileInfo(imageFileName).exists())
                clearStepCSICache(imageFileName);

            // Camera Globe, Switch Y and Z axis with -Y(LC -Z) in the up direction
            lcVector3 ldrawTarget = lcVector3LeoCADToLDraw(Camera->mTargetPosition);
            cameraMeta.target.setValues(ldrawTarget[0], ldrawTarget[1], ldrawTarget[2]);
            metaString = cameraMeta.target.format(true/*local*/,false/*global*/);
            newCommand = cameraMeta.target.here() == undefined;
            currentStep->mi(it)->setMetaAlt(newCommand ? top : cameraMeta.target.here(), metaString, newCommand);
        }

        if (applyPosition) {

            clearStepCache = true;
            if (QFileInfo(imageFileName).exists())
                clearStepCSICache(imageFileName);

            // Camera Globe, Switch Y and Z axis with -Y(LC -Z) in the up direction
            ldrawVector = lcVector3LeoCADToLDraw(Camera->mPosition);
            cameraMeta.position.setValues(ldrawVector[0], ldrawVector[1], ldrawVector[2]);
            metaString = cameraMeta.position.format(true/*local*/,false/*global*/);
            newCommand = cameraMeta.position.here() == undefined;
            currentStep->mi(it)->setMetaAlt(newCommand ? top : cameraMeta.position.here(), metaString, newCommand);
        }

        if (useImageSizeAct->isChecked()) {
            cameraMeta.imageSize.setFormats(3,0,"###9");
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
            cameraMeta.cameraAngles.setFormats(5,2,"###9");
            cameraMeta.cameraAngles.setValues(qRound(Latitude), qRound(Longitude));
            metaString = cameraMeta.cameraAngles.format(true,false);
            newCommand = cameraMeta.cameraAngles.here() == undefined;
            currentStep->mi(it)->setMetaAlt(newCommand ? top : cameraMeta.cameraAngles.here(), metaString, newCommand);
        }

        if (applyZPlanes && notEqual(cameraMeta.cameraZNear.value(), Camera->m_zNear)) {
            clearStepCache = true;
            cameraMeta.cameraZNear.setValue(Camera->m_zNear);
            metaString = cameraMeta.cameraZNear.format(true,false);
            newCommand = cameraMeta.cameraZNear.here() == undefined;
            currentStep->mi(it)->setMetaAlt(newCommand ? top : cameraMeta.cameraZNear.here(), metaString, newCommand);
        }

        if (applyZPlanes && notEqual(cameraMeta.cameraZFar.value(), Camera->m_zFar)) {
            clearStepCache = true;
            cameraMeta.cameraZFar.setValue(Camera->m_zFar);
            metaString = cameraMeta.cameraZFar.format(true,false);
            newCommand = cameraMeta.cameraZFar.here() == undefined;
            currentStep->mi(it)->setMetaAlt(newCommand ? top : cameraMeta.cameraZFar.here(), metaString, newCommand);
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

        if (!Camera->GetName().isEmpty()) {
            cameraMeta.cameraName.setValue(Camera->GetName());
            metaString = cameraMeta.cameraName.format(true,false);
            newCommand = cameraMeta.cameraName.here() == undefined;
            currentStep->mi(it)->setMetaAlt(newCommand ? top : cameraMeta.cameraName.here(), metaString, newCommand);
        }

        if (clearStepCache && QFileInfo(imageFileName).exists())
            clearStepCSICache(imageFileName);

        endMacro();
    }
}

bool Gui::installExportBanner(const int &type, const QString &printFile, const QString &imageFile){

    QList<QString> bannerData;
    bannerData << "0 Print Banner";
    bannerData << "0 Name: printbanner.ldr";
    bannerData << "0 Author: Trevor SANDY";
    bannerData << "0 Unofficial Model";
    bannerData << "0 !LPUB MODEL NAME Printbanner";
    bannerData << "0 !LPUB MODEL AUTHOR LPub3D";
    bannerData << "0 !LPUB MODEL DESCRIPTION Graphic displayed during pdf printing";
    bannerData << "0 !LPUB MODEL BACKGROUND IMAGE NAME " + imageFile;
    bannerData << "1 71 0 0 0 1 0 0 0 1 0 0 0 1 3020.dat";
    bannerData << "1 71 30 -8 10 1 0 0 0 1 0 0 0 1 3024.dat";
    bannerData << "1 71 30 -16 10 1 0 0 0 1 0 0 0 1 3024.dat";
    bannerData << "1 71 -30 -8 10 1 0 0 0 1 0 0 0 1 3024.dat";
    bannerData << "1 71 -30 -16 10 1 0 0 0 1 0 0 0 1 3024.dat";
    bannerData << "1 71 -30 -32 10 1 0 0 0 1 0 0 0 1 6091.dat";
    bannerData << "1 71 30 -32 10 1 0 0 0 1 0 0 0 1 6091.dat";
    bannerData << "1 71 30 -32 10 1 0 0 0 1 0 0 0 1 30039.dat";
    bannerData << "1 2 -30 -32 10 1 0 0 0 1 0 0 0 1 30039.dat";
    bannerData << "1 71 0 -24 10 1 0 0 0 1 0 0 0 1 3937.dat";
    bannerData << "1 72 0 -8 -10 1 0 0 0 1 0 0 0 1 3023.dat";
    bannerData << "1 72 0 -8 -10 -1 0 0 0 1 0 0 0 -1 85984.dat";
    bannerData << "1 71 0 -23.272 6.254 -1 0 0 0 0.927 0.375 0 0.375 -0.927 3938.dat";
    bannerData << "1 72 0 -45.524 -2.737 -1 0 0 0 0.927 0.375 0 0.375 -0.927 4865a.dat";
    switch (type) {
    case EXPORT_PNG:
        bannerData << "1 25 -22 -4 -32 1 0 0 0 0.423 -0.906 0 0.906 0.423 3070bptp.dat";
        bannerData << "1 25 -2 -4 -32 1 0 0 0 0.423 -0.906 0 0.906 0.423 3070bptn.dat";
        bannerData << "1 25 18 -4 -32 1 0 0 0 0.423 -0.906 0 0.906 0.423 3070bptg.dat";
        break;
    case EXPORT_JPG:
        bannerData << "1 92 -22 -4 -32 1 0 0 0 0.423 -0.906 0 0.906 0.423 3070bptj.dat";
        bannerData << "1 92 -2 -4 -32 1 0 0 0 0.423 -0.906 0 0.906 0.423 3070bptp.dat";
        bannerData << "1 92 18 -4 -32 1 0 0 0 0.423 -0.906 0 0.906 0.423 3070bptg.dat";
        break;
    case EXPORT_BMP:
        bannerData << "1 73 -22 -4 -32 1 0 0 0 0.423 -0.906 0 0.906 0.423 3070bptb.dat";
        bannerData << "1 73 -2 -4 -32 1 0 0 0 0.423 -0.906 0 0.906 0.423 3070bptm.dat";
        bannerData << "1 73 18 -4 -32 1 0 0 0 0.423 -0.906 0 0.906 0.423 3070bptp.dat";
        break;
    case EXPORT_WAVEFRONT:
        bannerData << "1 73 -22 -4 -32 1 0 0 0 0.423 -0.906 0 0.906 0.423 3070bpto.dat";
        bannerData << "1 73 -2 -4 -32 1 0 0 0 0.423 -0.906 0 0.906 0.423 3070bptb.dat";
        bannerData << "1 73 18 -4 -32 1 0 0 0 0.423 -0.906 0 0.906 0.423 3070bptj.dat";
        break;
    case EXPORT_COLLADA:
        bannerData << "1 73 -22 -4 -32 1 0 0 0 0.423 -0.906 0 0.906 0.423 3070bptd.dat";
        bannerData << "1 73 -2 -4 -32 1 0 0 0 0.423 -0.906 0 0.906 0.423 3070bpta.dat";
        bannerData << "1 73 18 -4 -32 1 0 0 0 0.423 -0.906 0 0.906 0.423 3070bpte.dat";
        break;
    case EXPORT_3DS_MAX:
        bannerData << "1 73 -22 -4 -32 1 0 0 0 0.423 -0.906 0 0.906 0.423 3070bptx.dat";
        bannerData << "1 73 -2 -4 -32 1 0 0 0 0.423 -0.906 0 0.906 0.423 3070bptd.dat";
        bannerData << "1 73 18 -4 -32 1 0 0 0 0.423 -0.906 0 0.906 0.423 3070bpts.dat";
        break;
    case EXPORT_STL:
        bannerData << "1 73 -22 -4 -32 1 0 0 0 0.423 -0.906 0 0.906 0.423 3070bpts.dat";
        bannerData << "1 73 -2 -4 -32 1 0 0 0 0.423 -0.906 0 0.906 0.423 3070bptt.dat";
        bannerData << "1 73 18 -4 -32 1 0 0 0 0.423 -0.906 0 0.906 0.423 3070bptl.dat";
        break;
    case EXPORT_POVRAY:
        bannerData << "1 73 -22 -4 -32 1 0 0 0 0.423 -0.906 0 0.906 0.423 3070bptp.dat";
        bannerData << "1 73 -2 -4 -32 1 0 0 0 0.423 -0.906 0 0.906 0.423 3070bpto.dat";
        bannerData << "1 73 18 -4 -32 1 0 0 0 0.423 -0.906 0 0.906 0.423 3070bptv.dat";
        break;
    case EXPORT_HTML_PARTS:
        bannerData << "1 73 -22 -4 -32 1 0 0 0 0.423 -0.906 0 0.906 0.423 3070bpth.dat";
        bannerData << "1 73 -2 -4 -32 1 0 0 0 0.423 -0.906 0 0.906 0.423 3070bptt.dat";
        bannerData << "1 73 18 -4 -32 1 0 0 0 0.423 -0.906 0 0.906 0.423 3070bptm.dat";
        break;
    case EXPORT_CSV:
        bannerData << "1 73 -22 -4 -32 1 0 0 0 0.423 -0.906 0 0.906 0.423 3070bptc.dat";
        bannerData << "1 73 -2 -4 -32 1 0 0 0 0.423 -0.906 0 0.906 0.423 3070bpts.dat";
        bannerData << "1 73 18 -4 -32 1 0 0 0 0.423 -0.906 0 0.906 0.423 3070bptv.dat";
        break;
    case EXPORT_BRICKLINK:
        bannerData << "1 73 -22 -4 -32 1 0 0 0 0.423 -0.906 0 0.906 0.423 3070bptb.dat";
        bannerData << "1 73 -2 -4 -32 1 0 0 0 0.423 -0.906 0 0.906 0.423 3070bptl.dat";
        bannerData << "1 73 18 -4 -32 1 0 0 0 0.423 -0.906 0 0.906 0.423 3070bptk.dat";
        break;
    default:
        bannerData << "1 216 -22 -4 -32 1 0 0 0 0.423 -0.906 0 0.906 0.423 3070bptp.dat";
        bannerData << "1 216 -2 -4 -32 1 0 0 0 0.423 -0.906 0 0.906 0.423 3070bptd.dat";
        bannerData << "1 216 18 -4 -32 1 0 0 0 0.423 -0.906 0 0.906 0.423 3070bptf.dat";
    }
    bannerData << "0";
    bannerData << "0 NOFILE";

    QFile bannerFile(printFile);
    if ( ! bannerFile.open(QIODevice::ReadWrite | QIODevice::Truncate | QIODevice::Text)) {
        emit gui->messageSig(LOG_ERROR,tr("Cannot open Export Banner file %1 for writing:\n%2")
                             .arg(printFile)
                             .arg(bannerFile.errorString()));
        return false;
    }
    QTextStream out(&bannerFile);
    for (int i = 0; i < bannerData.size(); i++) {
        QString fileLine = bannerData[i];
        out << fileLine << endl;
    }
    bannerFile.close();

    if (!gMainWindow->OpenProject(bannerFile.fileName()))
    {
        emit gui->messageSig(LOG_ERROR, tr("Could not load banner'%1'.").arg(bannerFile.fileName()));
        return false;
    }

    return true;
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
        lcSetProfileInt(LC_PROFILE_BUILD_MODIFICATION, Preferences::buildModEnabled && enableBuildModAct->isChecked());
    else if (sender() == enableRotstepRotateAct)
        lcSetProfileInt(LC_PROFILE_BUILD_MODIFICATION, !enableRotstepRotateAct->isChecked());
    else
        lcSetProfileInt(LC_PROFILE_BUILD_MODIFICATION, Preferences::buildModEnabled);

    bool buildModEnabled = Preferences::buildModEnabled    &&
            lcGetProfileInt(LC_PROFILE_BUILD_MODIFICATION) &&
            (mBuildModRange.first() || buildModsCount())    &&
            !curFile.isEmpty();
    QIcon RotateIcon;
    if (buildModEnabled)
        RotateIcon.addFile(":/resources/rotatebuildmod.png");
    else
        RotateIcon.addFile(":/resources/rotaterotstep.png");

    enableBuildModAct->setChecked(buildModEnabled);
    enableRotstepRotateAct->setChecked(!buildModEnabled);
    gMainWindow->mActions[LC_EDIT_ACTION_ROTATE]->setIcon(RotateIcon);
    gMainWindow->mActions[LC_EDIT_ACTION_ROTATESTEP]->setEnabled(!buildModEnabled);
    gApplication->mPreferences.mBuildModificationEnabled = lcGetProfileInt(LC_PROFILE_BUILD_MODIFICATION);
}

void Gui::enableBuildModActions()
{
    if (!createBuildModAct->isEnabled() || !Preferences::modeGUI || exporting())
        return;

    Rc buildModStep = BuildModNoActionRc;

    int hasMod = buildModsCount();

    if (currentStep)
        buildModStep = Rc(getBuildModStep(currentStep->topOfStep()));

    bool oneMod = hasMod == 1;

    bool appliedMod = false, sourceMod = false, removedMod = false;

    bool dlgTitle = applyBuildModAct->text().endsWith("...");

    switch (buildModStep)
    {
        case BuildModSourceRc:
            sourceMod = true;
            break;
        case BuildModApplyRc:
            appliedMod = true;
            break;
        case BuildModRemoveRc:
            removedMod = true;
            break;
        case BuildModNoActionRc:
        default:
            break;
    }

    if (oneMod || !dlgTitle) {
        QList<QAction*> modActions; modActions
                << applyBuildModAct
                << removeBuildModAct
                << loadBuildModAct
                << deleteBuildModAct;
        Q_FOREACH(QAction* action, modActions) {
            QString text = action->text();
            if (oneMod && dlgTitle)
                text.chop(3);
            if (!oneMod && !dlgTitle)
                text.append("...");
            action->setText(text);
        }
    }

    applyBuildModAct->setEnabled(hasMod && (!appliedMod || removedMod) && !sourceMod);
    removeBuildModAct->setEnabled(hasMod && (appliedMod || !removedMod) && !sourceMod);
    updateBuildModAct->setEnabled(hasMod && sourceMod);
    loadBuildModAct->setEnabled(hasMod && !(sourceMod && oneMod));
    deleteBuildModAct->setEnabled(hasMod);
}

void Gui::enableBuildModMenuAndActions()
{
    if (!curFile.isEmpty() && Preferences::buildModEnabled)
        createBuildModAct->setEnabled(mBuildModRange.first() || buildModsCount());
    enableBuildModification();
}

void Gui::showDefaultCameraProperties()
{
  gApplication->mPreferences.mDefaultCameraProperties = defaultCameraPropertiesAct->isChecked();
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
  if (Preferences::modeGUI)
      connect(gMainWindow->mLCStatusBar, SIGNAL(messageChanged(QString)), this, SLOT(showLCStatusMessage()));
}

void Gui::readNativeSettings()
{
    if (!gMainWindow)
        return;

    QSettings Settings;
    Settings.beginGroup(MAINWINDOW);
    gMainWindow->PartSelectionWidgetLoadState(Settings);
    Settings.endGroup();
}

void Gui::writeNativeSettings()
{
    if (!gMainWindow)
        return;

    QSettings Settings;
    Settings.beginGroup(MAINWINDOW);
    gMainWindow->PartSelectionWidgetSaveState(Settings);
    Settings.endGroup();

    gApplication->SaveTabLayout();
}

void Gui::SetActiveModel(const QString &modelName, bool setActive)
{
    if (modelName == VIEWER_MODEL_DEFAULT)
        return;
    if (lcGetActiveProject()->GetImageType() != Options::CSI)
        return;
    if (getCurrentStep() && modelName == getCurrentStep()->topOfStep().modelName)
        return;

    bool displayModelFile = false;
    if (setActive) {
        if (isSubmodel(modelName)) {
            const QString stepKey = getViewerStepKeyWhere(Where(getSubmodelIndex(modelName), 0));
            if (!stepKey.isEmpty()) {
                setCurrentStep(stepKey);
                displayModelFile = true;
            }
        } else if (getCurrentStep()) {
            displayModelFile = true;
        } else {
            emit messageSig(LOG_ERROR, QString("Active model '%1' not found").arg(modelName));
        }
        if (displayModelFile)
            displayFile(&ldrawFile, getCurrentStep()->topOfStep());
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
    lcView* ActiveView   = gMainWindow->GetActiveView();
    lcModel* ActiveModel = ActiveView->GetActiveModel();

    if (ActiveModel){
        // Create a copy of the current camera and add it to cameras
        lcCamera* Camera = ActiveView->GetCamera();
        Camera->CreateName(ActiveModel->GetCameras());
        Camera->SetSelected(true);
        ActiveModel->AddCamera(ActiveView->GetCamera());

        // Get the created camera name
        const QString cameraName = QString("Camera %1").arg(ActiveModel->GetCameras().GetSize());

        // Set the created camera
        ActiveView->SetCamera(cameraName);

        // Save the current model
        if (!lcGetActiveProject()->Save(modelFile))
            emit messageSig(LOG_ERROR, QString("Failed to save current model to file [%1]").arg(modelFile));

        // Reset the camera
        bool RemovedCamera = false;
        for (int CameraIdx = 0; CameraIdx < ActiveModel->GetCameras().GetSize(); )
        {
            QString Name = ActiveModel->GetCameras()[CameraIdx]->GetName();
            if (Name == cameraName)
            {
                RemovedCamera = true;
                ActiveModel->RemoveCameraIndex(CameraIdx);
            }
            else
                CameraIdx++;
        }

        ActiveView->SetCamera(Camera, true);
        ActiveView->GetCamera()->SetName(QString());

        if (RemovedCamera)
            gMainWindow->UpdateCameraMenu();

        Camera = nullptr;
    }
}

/*********************************************
 *
 * Default camera settings
 *
 *********************************************/

float Gui::getDefaultCameraFoV() const
{
    return (Preferences::preferredRenderer == RENDERER_NATIVE ?
            gApplication ? lcGetPreferences().mCFoV : CAMERA_FOV_NATIVE_DEFAULT :
            Preferences::preferredRenderer == RENDERER_LDVIEW && Preferences::perspectiveProjection ?
            CAMERA_FOV_LDVIEW_P_DEFAULT :
            CAMERA_FOV_DEFAULT);
}

float Gui::getDefaultFOVMinRange() const
{
    return (Preferences::preferredRenderer == RENDERER_NATIVE ?
            CAMERA_FOV_NATIVE_MIN_DEFAULT :
            CAMERA_FOV_MIN_DEFAULT);
}

float Gui::getDefaultFOVMaxRange() const
{
    return (Preferences::preferredRenderer == RENDERER_NATIVE ?
            CAMERA_FOV_NATIVE_MAX_DEFAULT :
            Preferences::preferredRenderer == RENDERER_LDVIEW && Preferences::perspectiveProjection ?
            CAMERA_FOV_LDVIEW_P_MAX_DEFAULT :
            CAMERA_FOV_MAX_DEFAULT);
}

float Gui::getDefaultNativeCameraZNear() const
{
    if (gApplication)
        return lcGetPreferences().mCNear;
    return CAMERA_ZNEAR_NATIVE_DEFAULT;
}

float Gui::getDefaultNativeCameraZFar() const
{
    if (gApplication)
        return lcGetPreferences().mCFar;
    return CAMERA_ZFAR_NATIVE_DEFAULT;
}

/*********************************************
 *
 * RotStep Meta
 *
 ********************************************/

void Gui::SetRotStepMeta()
{
    mStepRotation[0] = mRotStepAngleX;
    mStepRotation[1] = mRotStepAngleY;
    mStepRotation[2] = mRotStepAngleZ;

    if (getCurFile() != "") {
        ShowStepRotationStatus();
        Step *currentStep = gui->getCurrentStep();

        if (currentStep){
            bool newCommand = currentStep->rotStepMeta.here() == Where();
            int it = lcGetActiveProject()->GetImageType();
            Where top = currentStep->topOfStep();

            RotStepData rotStepData = currentStep->rotStepMeta.value();
            rotStepData.type    = mRotStepTransform;
            rotStepData.rots[0] = double(mStepRotation[0]);
            rotStepData.rots[1] = double(mStepRotation[1]);
            rotStepData.rots[2] = double(mStepRotation[2]);
            currentStep->rotStepMeta.setValue(rotStepData);
            QString metaString = currentStep->rotStepMeta.format(false/*no LOCAL tag*/,false);

            if (newCommand){
                if (top.modelName == gui->topLevelFile())
                    currentStep->mi(it)->scanPastLPubMeta(top);

                QString line = gui->readLine(top);
                Rc rc = page.meta.parse(line,top);
                if (rc == RotStepRc || rc == StepRc){
                   currentStep->mi(it)->replaceMeta(top, metaString);
                } else {
                   currentStep->mi(it)->insertMeta(top, metaString);
                }
            } else {
                currentStep->mi(it)->replaceMeta(top, metaString);
            }
        }
    }
}

void Gui::ShowStepRotationStatus()
{
    QString rotLabel = QString("%1 X: %2 Y: %3 Z: %4 Transform: %5")
                               .arg(lcGetProfileInt(LC_PROFILE_BUILD_MODIFICATION) ? "BUILD MOD ROTATE" : "ROTSTEP")
                               .arg(QString::number(double(mRotStepAngleX), 'f', 2))
                               .arg(QString::number(double(mRotStepAngleY), 'f', 2))
                               .arg(QString::number(double(mRotStepAngleZ), 'f', 2))
                               .arg(mRotStepTransform == "REL" ? "RELATIVE" :
                                    mRotStepTransform == "ABS" ? "ABSOLUTE" : "ADD");
    statusBarMsg(rotLabel);
}

void Gui::loadTheme(){
  Application::instance()->setTheme();
  emit setTextEditHighlighterSig();
  reloadViewer();
}

void Gui::reloadViewer(){
  if (getCurrentStep()){
    Render::LoadViewer(getCurrentStep()->viewerOptions);
  } else {
    Project* NewProject = new Project();
    gApplication->SetProject(NewProject);
    UpdateAllViews();
  }
}

/*********************************************
 *
 * Native viewer convenience calls
 *
 ********************************************/

 lcPiecesLibrary* Gui::GetPiecesLibrary()
 {
     return lcGetPiecesLibrary();
 }

 bool Gui::OpenProject(const QString& FileName)
 {
     bool ProjectLoaded = false;

     if (gMainWindow)
         ProjectLoaded = gMainWindow->OpenProject(FileName);
     else
     {
         Project* LoadedProject = new Project();

         if (LoadedProject->Load(FileName))
         {
             gApplication->SetProject(LoadedProject);
             ProjectLoaded = true;
         }
         else
         {
             delete LoadedProject;
         }
     }

     return ProjectLoaded;
 }

 lcView* Gui::GetActiveView()
 {
     if (gMainWindow)
         return gMainWindow->GetActiveView();
     else
         return new lcView(lcViewType::View, gApplication->mProject->GetMainModel());
 }

 lcModel* Gui::GetActiveModel()
 {
     return gApplication->mProject->GetMainModel();
 }

 lcPartSelectionWidget* Gui::GetPartSelectionWidget()
 {
     if (gMainWindow)
         return gMainWindow->GetPartSelectionWidget();
     return nullptr;
 }

 lcPreferences& Gui::GetPreferences()
 {
     return lcGetPreferences();
 }

 bool Gui::GetViewPieceIcons()
 {
     return GetPreferences().mViewPieceIcons;
 }

 bool Gui::GetSubmodelIconsLoaded()
 {
     if (gMainWindow)
         return gMainWindow->mSubmodelIconsLoaded;
     return false;
 }

 void Gui::SetSubmodelIconsLoaded(bool value)
 {
     if (gMainWindow)
         gMainWindow->mSubmodelIconsLoaded = value;
 }

 int Gui::GetLPubStepPieces()
 {
     if (GetActiveModel())
         return GetActiveModel()->GetPieces().GetSize();
     return 0;
 }

 int Gui::GetStudStyle()
 {
     if (gApplication)
         return static_cast<int>(lcGetPiecesLibrary()->GetStudStyle());
     else
         return lcGetProfileInt(LC_PROFILE_STUD_STYLE);
 }

 bool Gui::GetAutomateEdgeColor()
 {
     if (gApplication)
         return GetPreferences().mAutomateEdgeColor;
     else
         return lcGetProfileInt(LC_PROFILE_AUTOMATE_EDGE_COLOR);
 }

 quint32 Gui::GetStudCylinderColor()
 {
     if (gApplication)
         return GetPreferences().mStudCylinderColor;
     else
         return lcGetProfileInt(LC_PROFILE_STUD_CYLINDER_COLOR);
 }

 quint32 Gui::GetPartEdgeColor()
 {
     if (gApplication)
         return GetPreferences().mPartEdgeColor;
     else
         return lcGetProfileInt(LC_PROFILE_PART_EDGE_COLOR);
 }

 quint32 Gui::GetBlackEdgeColor()
 {
     if (gApplication)
         return GetPreferences().mBlackEdgeColor;
     else
         return lcGetProfileInt(LC_PROFILE_BLACK_EDGE_COLOR);
 }

 quint32 Gui::GetDarkEdgeColor()
 {
     if (gApplication)
         return GetPreferences().mDarkEdgeColor;
     else
         return lcGetProfileInt(LC_PROFILE_DARK_EDGE_COLOR);
 }

 float Gui::GetPartEdgeContrast()
 {
     if (gApplication)
         return GetPreferences().mPartEdgeContrast;
     else
         return lcGetProfileFloat(LC_PROFILE_PART_EDGE_CONTRAST);
 }

 float Gui::GetPartColorLightDarkIndex()
 {
     if (gApplication)
         return GetPreferences().mPartColorValueLDIndex;
     else
         return lcGetProfileFloat(LC_PROFILE_PART_COLOR_VALUE_LD_INDEX);
 }

 void Gui::SetStudStyle(const NativeOptions* Options, bool value)
 {
     gApplication->mPreferences.mPartColorValueLDIndex = Options->LightDarkIndex;
     gApplication->mPreferences.mStudCylinderColor = Options->StudCylinderColor;
     gApplication->mPreferences.mPartEdgeColor = Options->PartEdgeColor;
     gApplication->mPreferences.mBlackEdgeColor = Options->BlackEdgeColor;
     gApplication->mPreferences.mDarkEdgeColor = Options->DarkEdgeColor;

     lcGetPiecesLibrary()->SetStudStyle(static_cast<lcStudStyle>(Options->StudStyle), value);
 }

 void Gui::SetAutomateEdgeColor(const NativeOptions* Options)
 {
     gApplication->mPreferences.mAutomateEdgeColor = Options->AutoEdgeColor;
     gApplication->mPreferences.mPartEdgeContrast = Options->EdgeContrast;
     gApplication->mPreferences.mPartColorValueLDIndex = Options->EdgeSaturation;

     lcGetPiecesLibrary()->LoadColors();
 }

 void Gui::UpdateAllViews()
 {
     lcView::UpdateAllViews();
 }

 void Gui::UnloadOfficialPiecesLibrary()
 {
     lcGetPiecesLibrary()->UnloadOfficialLib();
 }

 void Gui::UnloadUnofficialPiecesLibrary()
 {
     lcGetPiecesLibrary()->UnloadUnofficialLib();
 }

 bool Gui::ReloadUnofficialPiecesLibrary()
 {
     return lcGetPiecesLibrary()->ReloadUnoffLib();
 }

 void Gui::LoadColors()
 {
     lcGetPiecesLibrary()->LoadColors();
 }

 int Gui::Process3DViewerCommandLine()
 {
     return gApplication->Process3DViewerCommandLine();
 }

 void Gui::LoadDefaults()
 {
     gApplication->mPreferences.LoadDefaults();
 }

 QToolBar* Gui::GetToolsToolBar()
 {
     if (gMainWindow)
         return gMainWindow->GetToolsToolBar();
     return nullptr;
 }

 QDockWidget* Gui::GetTimelineToolBar()
 {
     if (gMainWindow)
         return gMainWindow->GetTimelineToolBar();
     return nullptr;
 }

 QDockWidget* Gui::GetPropertiesToolBar()
 {
     if (gMainWindow)
         return gMainWindow->GetPropertiesToolBar();
     return nullptr;
 }

 QDockWidget* Gui::GetPartsToolBar()
 {
     if (gMainWindow)
         return gMainWindow->GetPartsToolBar();
     return nullptr;
 }

 QDockWidget* Gui::GetColorsToolBar()
 {
     if (gMainWindow)
         return gMainWindow->GetColorsToolBar();
     return nullptr;
 }

 QMenu* Gui::GetCameraMenu()
 {
     if (gMainWindow)
         return gMainWindow->GetCameraMenu();
     return nullptr;
 }

 QMenu* Gui::GetToolsMenu()
 {
     if (gMainWindow)
         return gMainWindow->GetToolsMenu();
     return nullptr;
 }

 QMenu* Gui::GetViewpointMenu()
 {
     if (gMainWindow)
         return gMainWindow->GetViewpointMenu();
     return nullptr;
 }

 QMenu* Gui::GetProjectionMenu()
 {
     if (gMainWindow)
         return gMainWindow->GetProjectionMenu();
     return nullptr;
 }

 QMenu* Gui::GetShadingMenu()
 {
     if (gMainWindow)
         return gMainWindow->GetShadingMenu();
     return nullptr;
 }

/*********************************************
 *
 * build modificaitons
 *
 ********************************************/

void Gui::createBuildModification()
{
    if (!currentStep || !Preferences::buildModEnabled)
        return;

    if (lcGetActiveProject()->GetImageType() != Options::CSI)
        return;

    lcView* ActiveView = GetActiveView();
    lcModel* ActiveModel = ActiveView->GetActiveModel();

    if (ActiveModel) {
        QString BuildModKey = buildModChangeKey;
        bool edit = ! BuildModKey.isEmpty();
        QString statusLabel = edit ? "Updating" : "Creating";

        emit progressBarPermInitSig();
        emit progressPermRangeSig(0, 0);   // Busy indicator
        emit progressPermMessageSig(QString("%1 Build Modification...").arg(statusLabel));

        if (mBuildModRange.first() || edit){

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
                         LPubModContents, ViewerModContents,
                         ModStepKeys;

            bool FadeStep         = Preferences::enableFadeSteps;
            bool HighlightStep    = Preferences::enableHighlightStep && !suppressColourMeta();

            int AddedPieces       = 0;
            int CurrentStep       = 1;
            lcPiece *Piece        = nullptr;
            lcCamera *Camera      = nullptr;
            lcLight  *Light       = nullptr;
            mViewerProperties     = ActiveModel->GetProperties();
            mViewerFileLines      = ActiveModel->GetFileLines();
            mViewerPieces         = ActiveModel->GetPieces();
            lcPiecesLibrary *Library = lcGetPiecesLibrary();

            QString ModStepKey    = viewerStepKey;
            ModStepKeys           = ModStepKey.split(";");

            // When edit, initialize BuildMod StepPieces, and Begin and End range with the existing values
            int BuildModBegin     = edit ? getBuildModBeginLineNumber(BuildModKey)  : 0;
            int BuildModAction    = edit ? getBuildModActionLineNumber(BuildModKey) : 0;
            int BuildModEnd       = edit ? getBuildModEndLineNumber(BuildModKey)    : 0;

            int ModBeginLineNum   = edit ? BuildModBegin  : mBuildModRange.at(BM_BEGIN_LINE_NUM);
            int ModActionLineNum  = edit ? BuildModAction : mBuildModRange.at(BM_ACTION_LINE_NUM);
            int ModEndLineNum     = edit ? BuildModEnd    : mBuildModRange.at(BM_BEGIN_LINE_NUM);
            int ModStepPieces     = edit ? getBuildModStepPieces(BuildModKey) : 0;    // All pieces in the previous step
            int ModelIndex        = edit ? getSubmodelIndex(getBuildModStepKeyModelName(BuildModKey)) : mBuildModRange.at(BM_MODEL_INDEX);
            int ModStepIndex      = getBuildModStepIndex(currentStep->topOfStep());
            int ModStepLineNum    = ModStepKeys[BM_STEP_LINE_KEY].toInt();
            int ModStepNum        = ModStepKeys[BM_STEP_NUM_KEY].toInt();
            int ModDisplayPageNum = displayPageNum;
            QString ModelName     = getSubmodelName(ModelIndex);
            buildModChangeKey     = QString();

            if (ModStepKeys[BM_STEP_MODEL_KEY].toInt() != ModelIndex)
                emit messageSig(LOG_ERROR, QString("BuildMod model (%1) '%2' and StepKey model (%3) mismatch")
                                                   .arg(ModelIndex).arg(ModelName).arg(ModStepKeys[BM_STEP_MODEL_KEY]));

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
                    lcMesh* Mesh = Info->GetMesh();

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

                        if (mViewerProperties.mFileName != Name)
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
            PieceAdjustment = AddedPieces = edit ? mViewerPieces.GetSize() - ModStepPieces : mViewerPieces.GetSize() - getViewerStepPartCount(ModStepKey);

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
                if (PieceInserted) {
                    LineNumber   += PieceInserted;
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
                                                   .arg(LineIndex < 0 ? QString::number(LineIndex) + "], Added Piece [#"+QString::number(AddedPieces) : QString::number(LineIndex))
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
                BuildModKey = QString("%1 Mod %2").arg(ModelName).arg(buildModsCount() + 1);

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

            // Set ModBeginLineNum to the top of the BuildMod step plus the number of 'AddedPieces' introduced by the BuildMod command
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

                beginMacro("BuildModCreate|" + viewerStepKey);

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

                clearWorkingFiles(getBuildModPathsFromStep(viewerStepKey));

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

            QVector<int> ModAttributes =
            {
                ModBeginLineNum,            // 0 BM_BEGIN_LINE_NUM
                ModActionLineNum,           // 1 BM_ACTION_LINE_NUM
                ModEndLineNum,              // 2 BM_END_LINE_NUM
                ModDisplayPageNum,          // 3 BM_DISPLAY_PAGE_NUM
                ModStepPieces,              // 4 BM_STEP_PIECES
                ModelIndex,                 // 5 BM_MODEL_NAME_INDEX
                ModStepLineNum,             // 6 BM_MODEL_LINE_NUM
                ModStepNum                  // 7 BM_MODEL_STEP_NUM
            };

            insertBuildMod(BuildModKey,
                           ModAttributes,
                           ModStepIndex);   // Unique ID

#ifdef QT_DEBUG_MODE
            emit messageSig(LOG_DEBUG, QString("Create BuildMod StepIndex: %1, "
                                               "Action: Apply(64) - %2, "
                                               "Attributes: %3 %4 %5 %6 %7 %8 %9 %10, "
                                               "ModKey: %11, "
                                               "Level: %12")
                                               .arg(edit ? "Update" : "Create")     // 01
                                               .arg(ModStepIndex)                   // 02
                                               .arg(ModBeginLineNum)                // 03 - 0 BM_BEGIN_LINE_NUM
                                               .arg(ModActionLineNum)               // 04 - 1 BM_ACTION_LINE_NUM
                                               .arg(ModEndLineNum)                  // 05 - 2 BM_END_LINE_NUM
                                               .arg(ModDisplayPageNum)              // 06 - 3 BM_DISPLAY_PAGE_NUM
                                               .arg(ModStepPieces)                  // 08 - 4 BM_STEP_PIECES
                                               .arg(ModelIndex)                     // 07 - 5 BM_MODEL_NAME_INDEX
                                               .arg(ModStepLineNum)                 // 08 - 6 BM_MODEL_LINE_NUM
                                               .arg(ModStepNum)                     // 09 - 7 BM_MODEL_STEP_NUM
                                               .arg(ModStepKey)                     // 10
                                               .arg(BuildModKey));                  // 11
#endif

            // Reset the build mod range
            mBuildModRange = { 0/*BM_BEGIN_LINE_NUM*/, 0/*BM_ACTION_LINE_NUM*/, -1/*BM_MODEL_INDEX*/ };
        } // mBuildModRange || edit

        emit progressPermStatusRemoveSig();
        enableBuildModActions();
    }
}

void Gui::applyBuildModification()
{
    if (!currentStep || exporting())
        return;

    QStringList buildModKeys;
    if (!buildModsCount()) {
        return;
    } else if (buildModsCount() == 1) {
        buildModKeys = getBuildModsList();
    } else {
        BuildModDialogGui *buildModDialogGui = new BuildModDialogGui();
        buildModDialogGui->getBuildMod(buildModKeys, BuildModApplyRc);
    }
    const QString buildModKey = buildModKeys.first();

    emit messageSig(LOG_INFO_STATUS, QString("Processing build modification 'apply' action..."));

    Where topOfStep = currentStep->topOfStep();

    // get the last action for this build mod
    Rc buildModAction = Rc(getBuildModAction(buildModKey, getBuildModStepIndex(topOfStep)));
    // was the last action defined in this step ?
    Rc buildModStep =  Rc(getBuildModStep(topOfStep));
    // set flag to remove the last action command if it is not 'Apply'
    bool removeActionCommand = buildModStep == buildModAction;

    QString model = topOfStep.modelName;
    QString line = QString::number(topOfStep.lineNumber);
    QString step = QString::number(currentStep->stepNumber.number);
    QString text, type, title;
    if (getBuildModStepKeyModelIndex(buildModKey) == getSubmodelIndex(model) && getBuildModStepKeyStepNum(buildModKey) > step.toInt()) {
            text  = "Build modification '" + buildModKey + "' was created after this step (" + step + "), "
                    "model '" + model + "', at line " + line + ".<br>"
                    "Applying a build modification before it is created is not supported.<br><br>No action taken.<br>";
            type  = "apply build modification error message";
            title = "Build Modification";

            Preferences::MsgID msgID(Preferences::BuildModErrors, Where("Apply_Before_" + model,line).nameToString());
            Preferences::showMessage(msgID, text, title, type);

            return;

    } else if (getBuildModStepKey(buildModKey) == currentStep->viewerStepKey) {
        text  = "Build modification '" + buildModKey + "' was created in this step (" + step + "), "
                "model '" + model + "', at line " + line + ".<br>"
                "It was automatically applied to the step it was created in.<br><br>No action taken.<br>";
        type  = "apply build modification error message";
        title = "Build Modification";

        Preferences::MsgID msgID(Preferences::BuildModErrors, Where("Source_" + model,line).nameToString());
        Preferences::showMessage(msgID, text, title, type);

        return;

    } else if (buildModAction == BuildModApplyRc) {
        text  = "Build modification '" + buildModKey + "' was already applied to step (" + step + "), "
                "model '" + model + ".<br><br>No action taken.<br>";
        type  = "apply build modification error message";
        title = "Build Modification";

        Preferences::MsgID msgID(Preferences::BuildModErrors, Where("Already_Applied_" + model,line).nameToString());
        Preferences::showMessage(msgID, text, title, type);

        return;

    }  /* else {
        text  = "This action will apply build modification '" + buildModKey + "' "
                "beginning at step " + step + ", in model '" + model + "'.<br><br>Are you sure ?<br>";
        type  = "apply build modification message";
        title = "Build Modification";

        Preferences::MsgID msgID(Preferences::BuildModErrors, Where(model,line).nameToString());
        switch (Preferences::showMessage(msgID, text, title, type, true / *option* /))
        {
        default:
        case QMessageBox::Cancel:
            return; // cancel request
        case QMessageBox::Ok:
            break;  // apply changes
        }


    } */

    int it = lcGetActiveProject()->GetImageType();
    if (it == Options::CSI) {
        QString metaString;
        bool newCommand = false;
        BuildModData buildModData = currentStep->buildMod.value();

        beginMacro("BuildModApply|" + currentStep->viewerStepKey);

        buildModData.action      = QString("APPLY");
        buildModData.buildModKey = buildModKey;
        currentStep->buildMod.setValue(buildModData);
        metaString = currentStep->buildMod.format(false/*local*/,false/*global*/);
        newCommand = currentStep->buildMod.here() ==  Where();
        currentStep->mi(it)->setMetaAlt(newCommand ? topOfStep : currentStep->buildMod.here(), metaString, newCommand, removeActionCommand);

        if (removeActionCommand)
            clearBuildModAction(buildModKey, getBuildModStepIndex(topOfStep));

        clearWorkingFiles(getBuildModPathsFromStep(currentStep->viewerStepKey));

        endMacro();
    }
}

void Gui::removeBuildModification()
{
    if (!currentStep || exporting())
        return;

    QStringList buildModKeys;
    if (!buildModsCount()) {
        return;
    } else if (buildModsCount() == 1) {
        buildModKeys = getBuildModsList();
    } else {
        BuildModDialogGui *buildModDialogGui =
                new BuildModDialogGui();
        buildModDialogGui->getBuildMod(buildModKeys, BuildModRemoveRc);
    }
    const QString buildModKey = buildModKeys.first();

    emit messageSig(LOG_INFO_STATUS, QString("Processing build modification 'remove' action..."));

    Where topOfStep = currentStep->topOfStep();

    // get the last action for this build mod
    Rc buildModAction = Rc(getBuildModAction(buildModKey, getBuildModStepIndex(topOfStep)));
    // was the last action defined in this step ?
    Rc buildModStep =  Rc(getBuildModStep(topOfStep));
    // set flag to remove the last action command if it is not 'Remove'
    bool removeActionCommand = buildModStep == buildModAction;

    QString model = topOfStep.modelName;
    QString line = QString::number(topOfStep.lineNumber);
    QString step = QString::number(currentStep->stepNumber.number);
    QString text, type, title;
    if (getBuildModStepKeyModelIndex(buildModKey) == getSubmodelIndex(model) && getBuildModStepKeyStepNum(buildModKey) > step.toInt()) {
            text  = "Build modification '" + buildModKey + "' was created after this step (" + step + "), "
                    "model '" + model + "', at line " + line + ".<br>"
                    "Removing a build modification before it is created is not supported.<br><br>No action taken.<br>";
            type  = "remove build modification error message";
            title = "Build Modification";

            return;

            Preferences::MsgID msgID(Preferences::BuildModErrors, Where("Remove_Before_" + model,line).nameToString());
            Preferences::showMessage(msgID, text, title, type);
    } else if (getBuildModStepKey(buildModKey) == viewerStepKey) {
        text  = "Build modification '" + buildModKey + "' was created in this step (" + step + "), "
                "in model '" + model + "' at line " + line + ".<br><br>"
                "It cannot be removed from the step it was created in.<br><br>"
                "Select 'Delete Build Modification' to delete from '" + model + "', "
                "step " + step + " at line " + line;
        type  = "remove build modification error message";
        title = "Build Modification";

        Preferences::MsgID msgID(Preferences::BuildModErrors, Where("Not_Applied_" + model,line).nameToString());
        Preferences::showMessage(msgID, text, title, type);

        return;

    } else if (buildModAction == BuildModRemoveRc) {
        text  = "Build modification '" + buildModKey + "' was already removed from step (" + step + "), "
                "model '" + model + ".<br><br>No action taken.<br>";
        type  = "remove build modification error message";
        title = "Build Modification";

        Preferences::MsgID msgID(Preferences::BuildModErrors, Where("Already_Removed_" + model,line).nameToString());
        Preferences::showMessage(msgID, text, title, type);

        return;

    } /* else {
        text  = "This action will remove build modification '" + buildModKey + "' "
                "beginning at step " + step + " in model '" + model + "'.<br><br>Are you sure ?<br>";
        type  = "remove build modification message";
        title = "Build Modification";

        Preferences::MsgID msgID(Preferences::BuildModErrors, Where(model,line).nameToString());
        switch (Preferences::showMessage(msgID, text, title, type, true / *option* /))
        {
        default:
        case QMessageBox::Cancel:
            return;
        case QMessageBox::Ok:
            break;
        }
    } */

    int it = lcGetActiveProject()->GetImageType();
    if (it == Options::CSI) {
        QString metaString;
        bool newCommand = false;
        BuildModData buildModData = currentStep->buildMod.value();

        beginMacro("BuildModRemove|" + currentStep->viewerStepKey);

        buildModData.action      = QString("REMOVE");
        buildModData.buildModKey = buildModKey;
        currentStep->buildMod.setValue(buildModData);
        metaString = currentStep->buildMod.format(false/*local*/,false/*global*/);
        newCommand = currentStep->buildMod.here() == Where();
        currentStep->mi(it)->setMetaAlt(newCommand ? topOfStep : currentStep->buildMod.here(), metaString, newCommand, removeActionCommand);

        if (removeActionCommand)
            clearBuildModAction(buildModKey, getBuildModStepIndex(topOfStep));

        clearWorkingFiles(getBuildModPathsFromStep(currentStep->viewerStepKey));

        endMacro();
    }

    enableBuildModMenuAndActions();
}

void Gui::loadBuildModification()
{
    if (exporting())
        return;

    QStringList buildModKeys;
    if (!buildModsCount()) {
        return;
    } else if (buildModsCount() == 1) {
        buildModKeys = getBuildModsList();
    } else {
        BuildModDialogGui *buildModDialogGui =
                new BuildModDialogGui();
        buildModDialogGui->getBuildMod(buildModKeys, BM_CHANGE);
    }
    const QString buildModKey = buildModKeys.first();

    emit messageSig(LOG_INFO_STATUS, QString("Processing build modification 'load' action..."));

    /*
    QString model = "undefined", line = "undefined", step = "undefined";
    QStringList keys = getViewerStepKeys(true/ *get Name* /, false/ *pliPart* /, getBuildModStepKey(buildModKey));
    if (keys.size() > 2) { model = keys[0]; line = keys[1]; step = keys[2]; }
    QString text  = "This action will load build modification '" + buildModKey + "' "
                     ", step " + step + ", model '" + model + "' into the 3DViewer "
                    "to allow editing.<br><br>Are you sure ?<br>";
    QString type  = "load build modification";
    QString title = "Build Modification";
    Preferences::MsgID msgID(Preferences::BuildModErrors, Where(model,line).nameToString());
    switch (Preferences::showMessage(msgID, text, title, type, true / *option* /))
    {
    default:
    case QMessageBox::Cancel:
        return;
    case QMessageBox::Ok:
        break;
    }
    */

    int it = lcGetActiveProject()->GetImageType();
    if (it == Options::CSI) {

        buildModChangeKey = "";

        int buildModDisplayPageNum = getBuildModDisplayPageNumber(buildModKey);

        QString buildModStepKey = getBuildModStepKey(buildModKey);

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

            buildModChangeKey = buildModKey;
        }
    }
}

bool Gui::setBuildModChangeKey()
{
    if (!currentStep)
        return false;

    int it = lcGetActiveProject()->GetImageType();
    if (it == Options::CSI) {
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
    if (!buildModsCount()) {
        return;
    } else if (buildModsCount() == 1) {
        buildModKeys = getBuildModsList();
    } else {
        BuildModDialogGui *buildModDialogGui =
                new BuildModDialogGui();
        buildModDialogGui->getBuildMod(buildModKeys, BM_DELETE);
    }
    const QString buildModKey = buildModKeys.first();

    emit messageSig(LOG_INFO_STATUS, QString("Processing build modification 'delete' action..."));

    QString model = "undefined", line = "undefined", step = "undefined";
    QStringList keys = getViewerStepKeys(true/*get Name*/, false/*pliPart*/, getBuildModStepKey(buildModKey));
    if (keys.size() > 2) { model = keys[0]; line = keys[1]; step = keys[2]; }
    /*
    QString text  = "This action will permanently delete build modification '" + buildModKey + "' "
                    "from '" + model + "' at step " + step + "' and cannot be completelly undone using the Undo menu action.<br><br>"
                    "The modified CSI image and 3DViewer entry will be parmanently deleted.<br>"
                    "However, you can use 'Reload' menu action to restore all deleted content.<br><br>"
                    "Do you want to continue ?<br>";
    QString type  = "delete build modification";
    QString title = "Build Modification";

    Preferences::MsgID msgID(Preferences::BuildModErrors, Where(model,line).nameToString());
    switch (Preferences::showMessage(msgID, text, title, type, true / *option* /))
    {
    default:
    case QMessageBox::Cancel:
        return;
    case QMessageBox::Ok:
        break;
    }
    */

    const QString buildModStepKey = getBuildModStepKey(buildModKey);
    bool multiStepPage = isViewerStepMultiStep(buildModStepKey);

    // Delete options
    QPixmap _icon = QPixmap(":/icons/lpub96.png");
    QMessageBoxResizable box;
    box.setWindowIcon(QIcon());
    box.setIconPixmap (_icon);
    box.setWindowFlags (Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);

    QString title = tr("Build Modification Image");
    QString text = tr("Click 'Submodel',%1 or 'Step' to reset the respective image cache.").arg(multiStepPage? " 'Page',": "");

    box.setWindowTitle(QString("%1 %2").arg(VER_PRODUCTNAME_STR).arg(title));
    box.setText (tr("Select your option to reset the image cache."));
    box.setInformativeText(text);

    QPushButton *clearSubmodelButton = box.addButton(tr("Modified"), QMessageBox::AcceptRole);
    clearSubmodelButton->setToolTip(tr("Reset modified submodel images from step %1").arg(step));

    QPushButton *clearPageButton = nullptr;
    if (multiStepPage) {
        int pageNum = getBuildModDisplayPageNumber(buildModKey);
        QPushButton *clearPageButton = box.addButton(tr("Page %1").arg(pageNum), QMessageBox::AcceptRole);
        clearPageButton->setToolTip(tr("Reset page %1 step group images").arg(pageNum));
    }

    QPushButton *clearStepButton = box.addButton(tr("Step %1").arg(step), QMessageBox::AcceptRole);
    clearStepButton->setToolTip(tr("Reset step %1 image only").arg(step));

    QPushButton *cancelButton = box.addButton(QMessageBox::Cancel);
    cancelButton->setDefault(true);

    box.exec();

    bool clearSubmodel = box.clickedButton() == clearSubmodelButton;
    bool clearPage     = multiStepPage ? box.clickedButton() == clearPageButton : false;
    bool clearStep     = box.clickedButton() == clearStepButton;

    if (box.clickedButton() == cancelButton)
        return;

    QString clearOption = clearSubmodel ? "_cm" : clearPage ? "_cp" : clearStep ? "_cs" : QString();

    int it = lcGetActiveProject()->GetImageType();
    if (it == Options::CSI) {
        int modBeginLineNum  = getBuildModBeginLineNumber(buildModKey);
        int modActionLineNum = getBuildModActionLineNumber(buildModKey);
        int modEndLineNum    = getBuildModEndLineNumber(buildModKey);
        QString modelName    = getBuildModStepKeyModelName(buildModKey);

        if (modelName.isEmpty() || !modBeginLineNum || !modActionLineNum || !modEndLineNum) {
            emit messageSig(LOG_ERROR, QString("There was a problem receiving build mod attributes for key [%1]<br>"
                                               "Delete build modification cannot continue.").arg(buildModKey));
            return;
        }

        beginMacro("BuildModDelete|" + buildModStepKey + clearOption);

        // delete existing APPLY/REMOVE (action) commands, starting from the bottom of the step
        Rc rc;
        QString modKey, modLine;
        Where here, topOfStep, bottomOfStep;
        QMap<int, int> actionsMap = getBuildModActions(buildModKey);
        QList<int> stepIndexes = actionsMap.keys();
        std::sort(stepIndexes.begin(), stepIndexes.end(), std::greater<int>()); // sort stepIndexes descending
        for (int &stepIndex : stepIndexes) {
            QMap<int, int>::iterator i = actionsMap.find(stepIndex);
            if (i.key()) {                                                      // skip first step at index 0 - deleted later
                if (getBuildModStepIndexWhere(i.key(), topOfStep)) {            // initialize topOfStep Where
                    if (! getBuildModStepIndexWhere(i.key() + 1, bottomOfStep)) // bottom of step is top of next step
                        // handle last step
                        bottomOfStep = Where(topOfStep.modelName, subFileSize(topOfStep.modelName));
                    for (Where walk = bottomOfStep; walk > topOfStep.lineNumber; --walk) {
                        here = walk;
                        modLine = readLine(here);
                        rc = page.meta.parse(modLine, here);
                        switch (rc) {
                        case BuildModApplyRc:
                        case BuildModRemoveRc:
                            modKey = page.meta.LPub.buildMod.key();
                            if (modKey == buildModKey)
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

        // delete step image to trigger image regen
        if (clearSubmodel) {
            clearWorkingFiles(getBuildModPathsFromStep(buildModStepKey));
        } else if (clearPage) {
            PlacementType relativeType = multiStepPage ? StepGroupType : SingleStepType;
            clearPageCSICache(relativeType, &page);
        } else if (clearStep) {
            QString csiPngName = getViewerStepImagePath(buildModStepKey);
            clearStepCSICache(csiPngName);
            // delete viewer step to trigger viewer update
            if (!deleteViewerStep(buildModStepKey))
                emit messageSig(LOG_ERROR,QString("Failed to delete viewer step entry for key %1.").arg(buildModStepKey));
        }

        // delete BuildMod
        if (!deleteBuildMod(buildModKey))
            emit messageSig(LOG_ERROR,QString("Failed to delete build modification for key %1.").arg(buildModKey));

        endMacro();
    }

    enableBuildModMenuAndActions();
}

/*********************************************
 *
 * save viewer model if modified
 *
 ********************************************/

bool Gui::saveBuildModification()
{
    if (!Preferences::buildModEnabled)
        return true;     // continue

    Project* Project = lcGetActiveProject();
    if (Project->GetImageType() != Options::CSI)
        return true;     // continue

    QString change = undoAct->data().toString();
    if (!BuildModChangeTriggers.contains(change))
        return true;     // continue

    QPixmap _icon = QPixmap(":/icons/lpub96.png");
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
        return false; // cancel request

    case QMessageBox::Yes:
        updateBuildModification();
        break;       // apply changes

    case QMessageBox::No:
        break;       // discard changes and continue
    }

    return true;
}

/*********************************************
 *
 * set viewer step key
 *
 ********************************************/

void Gui::setViewerStepKey(const QString &stepKey, int imageType)
{
    Q_UNUSED(imageType)

    viewerStepKey = stepKey;

    mBuildModRange = { 0, 0, -1 };
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
    } else if (keys.at(2).count("_")) {
        QStringList displayStepKeys = keys.at(2).split("_");
        keys.removeLast();
        keys.append(displayStepKeys);
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
          here = Where(modelName,getSubmodelIndex(modelName),lineNumber);
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
 * current step - called for CSI
 *
 ********************************************/

void Gui::setCurrentStep(Step *step, Where here, int stepNumber, int stepType)
{
    bool stepMatch  = false;
    auto calledOutStep = [this, &here, &stepNumber, &stepType] (Step* step, bool &stepMatch)
    {
        if (! (stepMatch = step->stepNumber.number == stepNumber || step->topOfStep() == here )) {
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
                        stepMatch = (step->stepNumber.number == stepNumber || step->topOfStep() == here);
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
                stepMatch = (step->stepNumber.number == stepNumber || step->topOfStep() == here);
        }
    } else if ((step = gStep)) {
        if (stepType == BM_CALLOUT_STEP)
            calledOutStep(step, stepMatch);
        else if (!(stepMatch = (step->stepNumber.number == stepNumber || step->topOfStep() == here)))
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

    QString stepNumberSpecified;
    if (!stepNumber)
        stepNumberSpecified = QString("not specified");
    else
        stepNumberSpecified = QString::number(stepNumber);

    QString stepKey = key.isEmpty() ? viewerStepKey : !stepNumber ? getViewerStepKeyWhere(here) : key;

    if (isViewerStepCalledOut(stepKey))
        stepType = BM_CALLOUT_STEP;
    else if (isViewerStepMultiStep(stepKey))
        stepType = BM_MULTI_STEP;
    else if (page.relativeType == SingleStepType && page.list.size())
        stepType = BM_SINGLE_STEP;

    if (stepType || gStep)
        setCurrentStep(step, here, stepNumber, stepType);
    else {
        emit messageSig(LOG_ERROR, QString("Could not determine step for '%1' at step number '%2'.")
                                           .arg(here.modelName).arg(stepNumberSpecified));
    }

#ifdef QT_DEBUG_MODE
    if (currentStep)
        emit messageSig(LOG_DEBUG,tr("Step %1 loaded from key: %2")
                        .arg(currentStep->stepNumber.number).arg(key));
#endif

    return currentStep;
}

void Gui::setCurrentStep(Step *step)
{
    viewerStepKey = step->viewerStepKey;
    currentStep = step;
//#ifdef QT_DEBUG_MODE
//    emit messageSig(LOG_DEBUG,tr("Current step %1 loaded")
//                    .arg(currentStep->stepNumber.number));
//#endif
}

/*********************************************
 *
 * set step from specified line
 *
 ********************************************/

void Gui::setStepForLine(const TypeLine &here)
{
    if (!currentStep || !gMainWindow || !gMainWindow->isVisible() || exporting())
        return;

    // limit the scope to the current page
    const QString stepKey = getViewerStepKeyFromRange(Where(here.modelIndex, here.lineIndex), topOfPage(), bottomOfPage());

    if (!stepKey.isEmpty()) {
        if (!currentStep->viewerStepKey.startsWith(&stepKey)) {
            if (!setCurrentStep(stepKey))
                return;

            setLineScopeSig(StepLines(getCurrentStep()->topOfStep().lineNumber, getCurrentStep()->bottomOfStep().lineNumber));
#ifdef QT_DEBUG_MODE
            emit messageSig(LOG_DEBUG,tr("Editor step %1 loaded line scope %2-%3")
                            .arg(currentStep->stepNumber.number)
                            .arg(getCurrentStep()->topOfStep().lineNumber + 1/*adjust for 0-start index*/)
                            .arg(getCurrentStep()->bottomOfStep().lineNumber /*actually top next step so no adjustment*/));
#endif
            enableBuildModActions();
            getCurrentStep()->viewerOptions->ZoomExtents = true;
            getCurrentStep()->loadTheViewer();
        }
    }
}

/*********************************************
 *
 * slelcted Line
 *
 ********************************************/

bool Gui::getSelectedLine(int modelIndex, int lineIndex, int source, int &lineNumber) {

    lineNumber        = EDITOR_LINE   ;             // 0
    bool currentModel = modelIndex == QString(viewerStepKey[0]).toInt();
    bool newLine      = lineIndex  == NEW_PART;     //-1
    bool fromViewer   = source      > EDITOR_LINE;  // 0

//#ifdef QT_DEBUG_MODE
//    emit messageSig(LOG_DEBUG, QString("currentModel: %1 [modelIndex: %2 == ViewerStepKey(index,line,step)[0]: %3]")
//                    .arg(currentModel ? "True" : "False")
//                    .arg(modelIndex)
//                    .arg(viewerStepKey));
//    emit messageSig(LOG_DEBUG, QString("newLine: %1 [lineIndex: %2 == NEW_PART: -1]")
//                    .arg(newLine ? "True" : "False")
//                    .arg(lineIndex));
//    emit messageSig(LOG_DEBUG, QString("fromViewer: %1 [source: %2 > EDITOR_LINE: 0]")
//                    .arg(fromViewer ? "True" : "False")
//                    .arg(source));
//#endif

    if (newLine) {
        emit messageSig(LOG_TRACE, QString("New viewer part modelName [%1]")
                                           .arg(getSubmodelName(modelIndex)));
        return false;

    } else if (currentModel) {

        if (!currentStep)
            return false;

#ifdef QT_DEBUG_MODE
        emit messageSig(LOG_TRACE, QString("LPub Step lineIndex count: %1 item(s)")
                        .arg(currentStep->lineTypeIndexes.size()));
//      for (int i = 0; i < currentStep->lineTypeIndexes.size(); ++i)
//          emit messageSig(LOG_TRACE, QString(" -LPub Part lineNumber [%1] at step line lineIndex [%2] - specified lineIndex [%3]")
//                                             .arg(currentStep->lineTypeIndexes.at(i)).arg(i).arg(lineIndex));
#endif

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
        return false;        // lineNumber = EDITOR_LINE;

//#ifdef QT_DEBUG_MODE
//    emit messageSig(LOG_DEBUG, QString("selected lineNumber: [%1] > NOT_FOUND: [-1]")
//                    .arg(lineNumber));
//#endif

    return lineNumber > NOT_FOUND; // -1
}

/*********************************************
 *
 * Selected part index(es)
 *
 ********************************************/

void Gui::SelectedPartLines(QVector<TypeLine> &indexes, PartSource source){
    if (! exporting()) {
        if (!currentStep || (source == EDITOR_LINE && !indexes.size()))
            return;

        QVector<int> lines;
        bool fromViewer   = source > EDITOR_LINE;
        bool validLine    = false;
        bool modsEnabled  = false;
        int lineNumber    = 0;
        int lineIndex     = NEW_PART;
        int modelIndex    = NEW_MODEL;
        QString modelName = "undefined";

#ifdef QT_DEBUG_MODE
        if (source > VIEWER_NONE) {
            const QString SourceNames[] =
            {
                "VIEWER_NONE", // 0
                "VIEWER_LINE", // 1
                "VIEWER_MOD",  // 2
                "VIEWER_DEL",  // 3
                "VIEWER_SEL",  // 4
                "VIEWER_CLR"   // 5
            };
            QString _Message = tr("Selection Source: %1 (%2)").arg(SourceNames[source], QString::number(source));
            emit gui->messageSig(LOG_DEBUG, _Message);
        }
#endif
        if (indexes.size()) {
            modelName  = getSubmodelName(indexes.at(0).modelIndex);
            modelIndex = indexes.at(0).modelIndex;
        } else if (!viewerStepKey.isEmpty()) {
            modelName  = currentStep->topOfStep().modelName;//getSubmodelName(QString(viewerStepKey[0]).toInt());
            modelIndex = getSubmodelIndex(modelName);
        }

#ifdef QT_DEBUG_MODE
        if (modelIndex != NEW_MODEL && source > VIEWER_LINE)
            emit messageSig(LOG_TRACE, QString("Submodel lineIndex count: %1 item(s)")
                            .arg(ldrawFile.getLineTypeRelativeIndexCount(modelIndex)));
#endif

        for (int i = 0; i < indexes.size() && source < VIEWER_CLR; ++i) {
            lineIndex = indexes.at(i).lineIndex;
            // New part lines are added in createBuildModification() routine
            if (lineIndex != NEW_PART) {
                validLine = getSelectedLine(modelIndex, lineIndex, source, lineNumber);
                if (lineNumber != OUT_OF_BOUNDS)
                    lines.append(lineNumber);
            }

            if (validLine) {
                bool buildModChange = source == VIEWER_MOD || source == VIEWER_DEL;
                if (fromViewer && buildModChange && Preferences::buildModEnabled) {
                    if (mBuildModRange.first()) {
                        if (lineNumber < mBuildModRange.first())
                            mBuildModRange[BM_BEGIN_LINE_NUM] = lineNumber;
                        else if (lineNumber > mBuildModRange.last())
                            mBuildModRange[BM_ACTION_LINE_NUM] = lineNumber;
                        mBuildModRange[BM_MODEL_INDEX] = modelIndex;
                    } else {
                        mBuildModRange = { lineNumber, lineNumber, modelIndex };
                    }
                    if (!modsEnabled) {
                        createBuildModAct->setEnabled(true);
                        enableBuildModification();
                        modsEnabled = true;
                    }
                }
            }

#ifdef QT_DEBUG_MODE
            QString Message;
            if (fromViewer) {
                if (lineIndex == NEW_PART) {
                    Message = tr("New viewer part specified at step %1, modelName: [%2]")
                            .arg(currentStep->stepNumber.number)
                            .arg(modelName);
                } else if (validLine) {
                    Message = tr("Selected part modelName [%1] lineNumber: [%2] at step line index [%3]")
                            .arg(modelName).arg(lineNumber).arg(lineIndex < 0 ? "undefined" : QString::number(lineIndex));
                } else {
                    Message = tr("%1 part lineNumber [%2] for step line index [%3]")
                            .arg(indexes.size() ? "Out of bounds" : "Invalid")
                            .arg(lineNumber).arg(lineIndex < 0 ? "undefined" : QString::number(lineNumber));
                }
            } else if (validLine) { // valid and not from viewer
                Message = tr("Selected part modelName [%1] lineNumber: [%2] at step line index [%3]")
                        .arg(modelName).arg(lineIndex).arg(lineNumber < 0 ? "undefined" : QString::number(lineIndex));
            } else {                // invalid and not from viewer
                Message = tr("%1 part lineNumber [%2] for step line index [%3]") // index and number flipped
                        .arg(indexes.size() ? "Out of bounds" : "Invalid")
                        .arg(lineIndex).arg(lineNumber < 0 ? "undefined" : QString::number(lineNumber));
            }
            emit messageSig(LOG_TRACE, Message);
#endif
        } // indexes present and source is not VIEWER_CLR

        if (fromViewer) {
            if (editWindow->isVisible() && (source == VIEWER_SEL || source == VIEWER_MOD || source == VIEWER_CLR)) {
                emit highlightSelectedLinesSig(lines, source == VIEWER_CLR);
            }
            // delete action with no selected lines
            if (source == VIEWER_DEL && Preferences::buildModEnabled) {
                if (!modsEnabled) {
                    createBuildModAct->setEnabled(true);
                    enableBuildModification();
                }
                emit messageSig(LOG_TRACE, tr("Delete viewer part(s) specified at step %1, modelName: [%2]")
                                              .arg(currentStep->stepNumber.number)
                                              .arg(modelName));
            }
        } else { // indexes from editor
            if (lines.size() && gMainWindow->isVisible())
                emit setSelectedPiecesSig(lines);
        }
    } // not exporting
}

void Gui::importLDD()
{
    if (!maybeSave() || !saveBuildModification())
        return;

    QString LoadFileName = QFileDialog::getOpenFileName(this, tr("Import"), QString(), tr("LEGO Diginal Designer Files (*.lxf);;All Files (*.*)"));
    if (LoadFileName.isEmpty())
        return;

    Project* Importer = new Project();

    if (Importer->ImportLDD(LoadFileName))
    {
        QString FileName = QFileDialog::getSaveFileName(this,tr("Save Diginal Designer File As"),
                                                        tr("%1.%2.mpd").arg(QFileInfo(LoadFileName).completeBaseName()),
                                                        tr("LDraw Files (*.mpd *.ldr *.dat);;All Files (*.*)"));
        if (FileName.isEmpty()) {
            QTemporaryDir tempDir;
            if (tempDir.isValid())
                FileName =  QString("%1/%2.%3.mpd").arg(tempDir.path(),QFileInfo(LoadFileName).completeBaseName());
        }
        saveImport(FileName, Importer);
    }
    else
        delete Importer;
}

void Gui::importInventory()
{
    if (!maybeSave() || !saveBuildModification())
        return;

    lcSetsDatabaseDialog Dialog(this);
    if (Dialog.exec() != QDialog::Accepted)
        return;

    Project* Importer = new Project();

    if (Importer->ImportInventory(Dialog.GetSetInventory(), Dialog.GetSetName(), Dialog.GetSetDescription()))
    {
        QString FileName = QFileDialog::getSaveFileName(this,tr("Save Inventory File As"),
                                                        tr("%1.%2.mpd").arg(Dialog.GetSetName()),
                                                        tr("LDraw Files (*.mpd *.ldr *.dat);;All Files (*.*)"));
        if (FileName.isEmpty()) {
            QTemporaryDir tempDir;
            if (tempDir.isValid())
                FileName =  QString("%1/%2.%3.mpd").arg(tempDir.path(),Dialog.GetSetName());
        }
        saveImport(FileName, Importer);
    }
    else
        delete Importer;
}

bool Gui::saveImport(const QString& FileName, Project *Importer)
{
    QFile File(FileName);

    if (!File.open(QIODevice::WriteOnly))
    {
        emit messageSig(LOG_ERROR, tr("Error writing to file '%1':\n%2").arg(FileName, File.errorString()));
        return false;
    }

    QTextStream Stream(&File);
    bool firstStep = true;
    QLatin1String LineEnding("\r\n");
    QLatin1String Extension(".ldr");
    for (lcModel* Model : Importer->GetModels())
    {
        if (firstStep) {
            firstStep = false;
            Stream << QLatin1String("0 FILE Import-") << Model->GetProperties().mFileName << Extension << LineEnding;
            Stream << QLatin1String("0 ") << Model->GetProperties().mDescription << LineEnding;
            Stream << QLatin1String("0 Name: Import-") << Model->GetProperties().mModelName << LineEnding;
            Stream << QLatin1String("0 Author: ") << Model->GetProperties().mAuthor << LineEnding;
            Stream << QLatin1String("0 !LPUB INSERT TEXT \"This page is intentionally left blank\\nto avoid loading the imported parts at the first step.\" \"Arial,36,-1,255,75,0,0,0,0,0\" \"Black\" OFFSET 0.0219 0.4814\r\n");
            Stream << QLatin1String("0 !LPUB INSERT PAGE\r\n");
            Stream << QLatin1String("0 STEP\r\n");
            Stream << QLatin1String("1 16 0 0 0 1 0 0 0 1 0 0 0 1 ") << Model->GetProperties().mFileName << Extension << LineEnding;
            Stream << QLatin1String("0 NOFILE\r\n");
        }
        Stream << QLatin1String("0 FILE ") << Model->GetProperties().mFileName << Extension << LineEnding;
        Model->SaveLDraw(Stream, false);
        Model->SetSaved();
        Stream << QLatin1String("0 NOFILE\r\n");
    }
    File.close();

    int saveLoadMessageFlag = Preferences::ldrawFilesLoadMsgs;
    Preferences::ldrawFilesLoadMsgs = SHOW_WARNING;
    if (!loadFile(FileName)) {
        Preferences::ldrawFilesLoadMsgs = saveLoadMessageFlag;
        return false;
    }
    Preferences::ldrawFilesLoadMsgs = saveLoadMessageFlag;

    return true;
}
