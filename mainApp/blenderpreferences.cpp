/****************************************************************************
**
** Copyright (C) 2023 Trevor SANDY. All rights reserved.
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
 * This file describes a dialog that users can use to configure Blender
 * options and settings.
 *
 * Please see lpub.h for an overall description of how the files in LPub
 * make up the LPub program.
 *
 ***************************************************************************/

#include <QFormLayout>
#include <QGridLayout>
#include <QLabel>
#include <QGroupBox>
#include <QLineEdit>
#include <QCheckBox>
#include <QPushButton>
#include <QComboBox>
#include <QProgressBar>
#include <QProcess>
#include <QTimer>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

#include <JlCompress.h>

#include "blenderpreferences.h"
#include "lpub_preferences.h"
#include "commonmenus.h"
#include "lpub.h"
#include "version.h"

#include "lc_colors.h"
#include "lc_qcolorpicker.h"

qreal   BlenderPreferences::mRenderPercentage= 0.0;
bool    BlenderPreferences::mDialogCancelled = false;
bool    BlenderPreferences::mDocumentRender  = false;
QString BlenderPreferences::blenderVersion;
QString BlenderPreferences::blenderAddonVersion;

BlenderPreferences::BlenderPaths  BlenderPreferences::blenderPaths [NUM_BLENDER_PATHS];
BlenderPreferences::BlenderPaths  BlenderPreferences::defaultPaths [] = {
    /*                                 Key:                  MM Key:               Value:             Label:                                   Tooltip (Description):*/
    /* 0   LBL_BLENDER_PATH        */ {"blenderpath",        "blenderpath",        "",    QObject::tr("Blender Path"),             QObject::tr("Full file path to Blender application executable")},
    /* 1   LBL_BLENDFILE_PATH      */ {"blendfile",          "blendfile",          "",    QObject::tr("Blendfile Path"),           QObject::tr("Full file path to a supplement .blend file - specify to append additional settings")},
    /* 2.  LBL_ENVIRONMENT_PATH    */ {"environmentfile",    "environmentfile",    "",    QObject::tr("Environment Texture Path"), QObject::tr("Full file path to .exr environment texture file - specify if not using default bundled in addon")},
    /* 3   LBL_LDCONFIG_PATH       */ {"customldconfigfile", "customldconfigfile", "",    QObject::tr("Custom LDConfig Path"),     QObject::tr("Full file path to custom LDConfig file - specify if not %1 alternate LDConfig file").arg(VER_PRODUCTNAME_STR)},
    /* 4   LBL_LDRAW_PATH          */ {"ldrawdirectory",     "ldrawpath",          "",    QObject::tr("LDraw Directory"),          QObject::tr("Full directory path to the LDraw parts library (download from https://library.ldraw.org)")},
    /* 5   LBL_LSYNTH_PATH         */ {"lsynthdirectory",    "lsynthdirectory",    "",    QObject::tr("LSynth Directory"),         QObject::tr("Full directory path to LSynth primitives - specify if not using default bundled in addon")},
    /* 6   LBL_STUD_LOGO_PATH      */ {"studlogodirectory",  "studlogodirectory",  "",    QObject::tr("Stud Logo Directory"),      QObject::tr("Full directory path to stud logo primitives - if stud logo enabled, specify if unofficial parts not used or not using default bundled in addon")},
    /* 7   LBL_STUDIO_LDRAW_PATH   */ {"studlogodirectory",  "studioldrawpath",    "",    QObject::tr("Stud.io LDraw Path"),       QObject::tr("Full filepath to the Stud.io LDraw Parts Library (download from https://www.bricklink.com/v3/studio/download.page)")}
};

BlenderPreferences::BlenderSettings  BlenderPreferences::blenderSettings [NUM_SETTINGS];
BlenderPreferences::BlenderSettings  BlenderPreferences::defaultSettings [] = {
    /*                                   Key:                              Value:                  Label                                  Tooltip (Description)*/
    /* 0   LBL_ADD_ENVIRONMENT       */ {"addenvironment",                 "1",        QObject::tr("Add Environment"),        QObject::tr("Adds a ground plane and environment texture (affects 'Photo-realistic' look only)")},
    /* 1   LBL_ADD_GAPS              */ {"gaps",                           "0",        QObject::tr("Add Part Gap"),           QObject::tr("Add a small space between each part")},
    /* 2   LBL_BEVEL_EDGES           */ {"beveledges",                     "1",        QObject::tr("Bevel Edges"),            QObject::tr("Adds a Bevel modifier for rounding off sharp edges")},
    /* 3   LBL_BLENDFILE_TRUSTED     */ {"blendfiletrusted",               "0",        QObject::tr("Trusted Blend File"),     QObject::tr("Specify whether to treat the .blend file as being loaded from a trusted source")},
    /* 4   LBL_CROP_IMAGE            */ {"cropimage",                      "0",        QObject::tr("Crop Image"),             QObject::tr("Crop the image border at opaque content. Requires transparent background set to True")},
    /* 5   LBL_CURVED_WALLS          */ {"curvedwalls",                    "1",        QObject::tr("Curved Walls"),           QObject::tr("Makes surfaces look slightly concave")},
    /* 6   LBL_FLATTEN_HIERARCHY     */ {"flattenhierarchy",               "0",        QObject::tr("Flatten Hierarchy"),      QObject::tr("In Scene Outline, all parts are placed directly below the root - there's no tree of submodels")},
    /* 7   LBL_IMPORT_CAMERAS        */ {"importcameras",                  "1",        QObject::tr("Import Cameras"),         QObject::tr("%1 can specify camera definitions within the ldraw data. Choose to load them or ignore them.").arg(VER_PRODUCTNAME_STR)},
    /* 8   LBL_IMPORT_LIGHTS         */ {"importlights",                   "1",        QObject::tr("Import Lights"),          QObject::tr("%1 can specify point and sunlight definitions within the ldraw data. Choose to load them or ignore them.").arg(VER_PRODUCTNAME_STR)},
    /* 9   LBL_INSTANCE_STUDS        */ {"instancestuds",                  "0",        QObject::tr("Instance Studs"),         QObject::tr("Creates a Blender Object for each and every stud (WARNING: can be slow to import and edit in Blender if there are lots of studs)")},
    /*10   LBL_KEEP_ASPECT_RATIO     */ {"keepaspectratio",                "1",        QObject::tr("Keep Aspect Ratio"),      QObject::tr("Maintain the aspect ratio when resizing the output image - this attribute is not passed to Blender")},
    /*11   LBL_LINK_PARTS            */ {"linkparts",                      "1",        QObject::tr("Link Like Parts"),        QObject::tr("Identical parts (of the same type and colour) share the same mesh")},
    /*12   LBL_NUMBER_NODES          */ {"numbernodes",                    "1",        QObject::tr("Number Objects"),         QObject::tr("Each object has a five digit prefix eg. 00001_car. This keeps the list in it's proper order")},
    /*13   LBL_OVERWRITE_IMAGE       */ {"overwriteimage",                 "1",        QObject::tr("Overwrite Image"),        QObject::tr("Specify whether to overwrite an existing rendered image file")},
    /*14   LBL_OVERWRITE_MATERIALS   */ {"overwriteexistingmaterials",     "0",        QObject::tr("Use Existing Material"),  QObject::tr("Overwrite existing material with the same name")},
    /*15   LBL_OVERWRITE_MESHES      */ {"overwriteexistingmeshes",        "0",        QObject::tr("Use Existing Mesh"),      QObject::tr("Overwrite existing mesh with the same name")},
    /*16   LBL_POSITION_CAMERA       */ {"positioncamera",                 "1",        QObject::tr("Position Camera"),        QObject::tr("Position the camera to show the whole model")},
    /*17   LBL_REMOVE_DOUBLES        */ {"removedoubles",                  "1",        QObject::tr("No Duplicate Vertices"),  QObject::tr("Remove duplicate vertices (recommended)")},
    /*18   LBL_RENDER_WINDOW         */ {"renderwindow",                   "1",        QObject::tr("Display Render Window"),  QObject::tr("Specify whether to display the render window during Blender user interface image file render")},
    /*19   LBL_USE_ARCHIVE_LIBS      */ {"usearchivelibrary",              "0",        QObject::tr("Use Archive Libraries"),  QObject::tr("Add any archive (zip) libraries in the LDraw file path to the library search list")},
    /*10   LBL_SEARCH_ADDL_PATHS     */ {"searchadditionalpaths",          "0",        QObject::tr("Search Additional Paths"),QObject::tr("Specify whether to search additional LDraw paths")},
    /*21   LBL_SMOOTH_SHADING        */ {"smoothshading",                  "1",        QObject::tr("Smooth Shading"),         QObject::tr("Smooth faces and add an edge-split modifier (recommended)")},
    /*22   LBL_TRANSPARENT_BACKGROUND*/ {"transparentbackground",          "0",        QObject::tr("Transparent Background"), QObject::tr("Specify whether to render a background (affects 'Photo-realistic look only)")},
    /*23   LBL_UNOFFICIAL_PARTS      */ {"useunofficialparts",             "1",        QObject::tr("Use Unofficial Parts"),   QObject::tr("Specify whether to use parts from the LDraw unofficial parts library path")},
    /*24   LBL_USE_LOGO_STUDS        */ {"uselogostuds",                   "0",        QObject::tr("Use Logo Studs"),         QObject::tr("Shows the LEGO logo on each stud (at the expense of some extra geometry and import time)")},
    /*25   LBL_VERBOSE               */ {"verbose",                        "1",        QObject::tr("Verbose output"),         QObject::tr("Output all messages while working, else only show warnings and errors")},

    /*26/0 LBL_BEVEL_WIDTH           */ {"bevelwidth",                     "0.5",      QObject::tr("Bevel Width"),            QObject::tr("Width of the bevelled edges")},
    /*27/1 LBL_CAMERA_BORDER_PERCENT */ {"cameraborderpercentage",         "5.0",      QObject::tr("Camera Border Percent"),  QObject::tr("When positioning the camera, include a (percentage) border around the model in the render")},
    /*28/2 LBL_DEFAULT_COLOUR        */ {"defaultcolour",                  "4",        QObject::tr("Default Colour"),         QObject::tr("Sets the default part colour")},
    /*29/3 LBL_GAPS_SIZE             */ {"gapwidth",                       "0.01",     QObject::tr("Gap Width"),              QObject::tr("Amount of gap space between each part")},
    /*20/4 LBL_IMAGE_WIDTH           */ {"resolutionwidth",                "800",      QObject::tr("Image Width"),            QObject::tr("Sets the rendered image width in pixels.")},
    /*31/5 LBL_IMAGE_HEIGHT          */ {"resolutionheight",               "600",      QObject::tr("Image Height"),           QObject::tr("Sets the rendered image height in pixels.")},
    /*32/6 LBL_IMAGE_SCALE           */ {"scale",                          "0.02",     QObject::tr("Image Scale"),            QObject::tr("Sets the imported model scale (between .01 and 1.0). Scale is 1.0 and is huge and unwieldy in the viewport")},
    /*33/6 LBL_RENDER_PERCENTAGE     */ {"renderpercentage",               "100",      QObject::tr("Render Percentage"),      QObject::tr("Sets the rendered image percentage scale for its pixel resolution (default is 100)")},


    /*34/0 LBL_COLOUR_SCHEME         */ {"usecolourscheme",                "lgeo",     QObject::tr("Colour Scheme"),          QObject::tr("Colour scheme options - Realistic (lgeo), Original (LDConfig), Alternate (LDCfgalt), Custom (User Defined)")},
    /*35/1 LBL_FLEX_PARTS_SOURCE     */ {"uselsynthparts",                 "1",        QObject::tr("Flex Parts Source"),      QObject::tr("Source used to create flexible parts - string, hoses etc. (LDCad, LSynth or both")},
    /*36/2 LBL_LOGO_STUD_VERSION     */ {"logostudversion",                "4",        QObject::tr("Logo Version"),           QObject::tr("Which version of the logo to use ('3' (flat), '4' (rounded) or '5' (subtle rounded))")},
    /*27/3 LBL_LOOK                  */ {"uselook",                        "normal",   QObject::tr("Look"),                   QObject::tr("Photo-realistic or Schematic 'Instruction' look")},
    /*38/4 LBL_POSITION_OBJECT       */ {"positionobjectongroundatorigin", "1",        QObject::tr("Position Object"),        QObject::tr("The object is centred at the origin, and on the ground plane")},
    /*39/5 LBL_RESOLUTION            */ {"resolution",                     "Standard", QObject::tr("Resolution"),             QObject::tr("Resolution of part primitives, ie. how much geometry they have")},
    /*40/6 LBL_RESOLVE_NORMALS       */ {"resolvenormals",                 "guess",    QObject::tr("Resolve Normals"),        QObject::tr("Some older LDraw parts have faces with ambiguous normals, this specifies what do do with them")}
} ;

BlenderPreferences::ComboOptItems  BlenderPreferences::comboOptItems [] = {
    /*    FIRST item set as default        Data                       Item:   */
    /* 00 LBL_COLOUR_SCHEME            */ {"lgeo|ldraw|alt|custom",  "Realistic Colours|Original LDraw Colours|Alternate LDraw Colours|Custom Colours"},
    /* 01 LBL_FLEX_PARTS_SOURCE (t/f)  */ {"1|0|1",                  "LSynth|LDCad|LDCad and LSynth"},
    /* 02 LBL_LOGO_STUD_VERSION        */ {"4|3|5",                  "Rounded(4)|Flattened(3)|Subtle Rounded(5)"},
    /* 03 LBL_LOOK                     */ {"normal|instructions",    "Photo Realistic|Lego Instructions"},
    /* 04 LBL_POSITION_OBJECT (t/f)    */ {"1|0",                    "Centered At Origin On Ground|Centered At Origin"},
    /* 05 LBL_RESOLUTION               */ {"Standard|High|Low",      "Standard Primitives|High Resolution Primitives|Low Resolution Primitives"},
    /* 06 LBL_RESOLVE_NORMALS          */ {"guess|double",           "Recalculate Normals|Two Faces Back To Back"}
};

BlenderPreferences::BlenderSettings  BlenderPreferences::blenderSettingsMM [NUM_SETTINGS_MM];
BlenderPreferences::BlenderSettings  BlenderPreferences::defaultSettingsMM [] = {
    /*                                                Key:                             Value:                    Label                                    Tooltip (Description)*/
    /* 00 LBL_ADD_ENVIRONMENT_MM                  */ {"addenvironment",                "1",          QObject::tr("Add Environment"),          QObject::tr("Adds a ground plane and environment texture")},
    /* 01 LBL_BLEND_FILE_TRUSTED_MM               */ {"blendfiletrusted",              "0",          QObject::tr("Trusted Blend File"),       QObject::tr("Specify whether to treat the .blend file as being loaded from a trusted source")},
    /* 02 LBL_CROP_IMAGE_MM                       */ {"cropimage",                     "0",          QObject::tr("Crop Image"),               QObject::tr("Crop the image border at opaque content. Requires transparent background set to True")},
    /* 03 LBL_DISPLAY_LOGO                        */ {"displaylogo",                   "0",          QObject::tr("Display Logo"),             QObject::tr("Display the logo on the stud")},
    /* 04 LBL_IMPORT_CAMERAS_MM                   */ {"importcameras",                 "1",          QObject::tr("Import Cameras"),           QObject::tr("%1 can specify camera definitions within the ldraw data. Choose to load them or ignore them.").arg(VER_PRODUCTNAME_STR)},
    /* 05 LBL_IMPORT_EDGES                        */ {"importedges",                   "0",          QObject::tr("Import Edges"),             QObject::tr("Import LDraw edges as edges")},
    /* 06 LBL_IMPORT_LIGHTS_MM                    */ {"importlights",                  "0",          QObject::tr("Import Lights"),            QObject::tr("%1 can specify point and sunlight definitions within the ldraw data. Choose to load them or ignore them.").arg(VER_PRODUCTNAME_STR)},
    /* 07 LBL_KEEP_ASPECT_RATIO_MM                */ {"keepaspectratio",               "1",          QObject::tr("Keep Aspect Ratio"),        QObject::tr("Maintain the aspect ratio when resizing the output image - this attribute is not passed to Blender")},
    /* 08 LBL_MAKE_GAPS                           */ {"makegaps",                      "1",          QObject::tr("Make Gaps"),                QObject::tr("Make small gaps between bricks. A small gap is more realistic")},
    /* 09 LBL_META_BFC                            */ {"metabfc",                       "1",          QObject::tr("BFC"),                      QObject::tr("Process LDraw Back Face Culling meta commands")},
    /* 10 LBL_META_CLEAR                          */ {"metaclear",                     "0",          QObject::tr("CLEAR Command"),            QObject::tr("Hides all parts in the timeline up to where this command is encountered")},
    /* 11 LBL_META_GROUP                          */ {"metagroup",                     "1",          QObject::tr("GROUP Command"),            QObject::tr("Process GROUP meta commands")},
    /* 12 LBL_META_PAUSE                          */ {"metapause",                     "0",          QObject::tr("PAUSE Command"),            QObject::tr("Not implemented")},
    /* 13 LBL_META_PRINT_WRITE                    */ {"metaprintwrite",                "0",          QObject::tr("PRINT/WRITE Command"),      QObject::tr("Prints PRINT/WRITE META commands to the system console.")},
    /* 14 LBL_META_SAVE                           */ {"metasave",                      "0",          QObject::tr("SAVE Command"),             QObject::tr("Not implemented")},
    /* 15 LBL_META_STEP                           */ {"metastep",                      "0",          QObject::tr("STEP Command"),             QObject::tr("Adds a keyframe that shows the part at the moment in the timeline")},
    /* 16 LBL_META_STEP_GROUPS                    */ {"metastepgroups",                "0",          QObject::tr("STEP Groups"),              QObject::tr("Create collection for individual steps")},
    /* 17 LBL_NO_STUDS                            */ {"nostuds",                       "0",          QObject::tr("No Studs"),                 QObject::tr("Don't import studs")},
    /* 18 LBL_OVERWRITE_IMAGE_MM                  */ {"overwriteimage",                "1",          QObject::tr("Overwrite Image"),          QObject::tr("Specify whether to overwrite an existing rendered image file")},
    /* 19 LBL_POSITION_CAMERA_MM                  */ {"positioncamera",                "1",          QObject::tr("Position Camera"),          QObject::tr("Position the camera to show the whole model")},
    /* 20 LBL_PARENT_TO_EMPTY                     */ {"parenttoempty",                 "1",          QObject::tr("Parent To Empty"),          QObject::tr("Parent the model to an empty")},
    /* 21 LBL_PREFER_STUDIO                       */ {"preferstudio",                  "0",          QObject::tr("Prefer Stud.io Library"),   QObject::tr("Search for parts in Stud.io library first")},
    /* 22 LBL_PREFER_UNOFFICIAL                   */ {"preferunofficial",              "0",          QObject::tr("Prefer Unofficial Parts"),  QObject::tr("Search for unofficial parts first")},
    /* 23 LBL_PRESERVE_HIERARCHY                  */ {"preservehierarchy",             "0",          QObject::tr("Preserve File Structure"),  QObject::tr("Don't merge the constituent subparts and primitives into the top level part. Some parts may not render properly")},
    /* 24 LBL_PROFILE                             */ {"profile",                       "0",          QObject::tr("Profile"),                  QObject::tr("Profile import performance")},
    /* 25 LBL_RECALCULATE_NORMALS                 */ {"recalculatenormals",            "0",          QObject::tr("Recalculate Normals"),      QObject::tr("Recalculate normals. Not recommended if BFC processing is active")},
    /* 26 LBL_REMOVE_DOUBLES_MM                   */ {"removedoubles",                 "1",          QObject::tr("No Duplicate Vertices"),    QObject::tr("Merge vertices that are within a certain distance.")},
    /* 27 LBL_RENDER_WINDOW_MM                    */ {"renderwindow",                  "1",          QObject::tr("Display Render Window"),    QObject::tr("Specify whether to display the render window during Blender user interface image file render")},
    /* 28 LBL_SEARCH_ADDL_PATHS_MM                */ {"searchadditionalpaths",         "0",          QObject::tr("Search Additional Paths"),  QObject::tr("Specify whether to search additional LDraw paths")},
    /* 29 LBL_SETEND_FRAME                        */ {"setendframe",                   "1",          QObject::tr("Set Step End Frame"),       QObject::tr("Set the end frame to the last step")},
    /* 30 LBL_SET_TIMELINE_MARKERS                */ {"settimelinemarkers",            "0",          QObject::tr("Set Timeline Markers"),     QObject::tr("Set timeline markers for meta commands")},
    /* 31 LBL_SHADE_SMOOTH                        */ {"shadesmooth",                   "1",          QObject::tr("Shade Smooth"),             QObject::tr("Use flat or smooth shading for part faces")},
    /* 32 LBL_TRANSPARENT_BACKGROUND_MM           */ {"transparentbackground",         "0",          QObject::tr("Transparent Background"),   QObject::tr("Specify whether to render a background")},
    /* 33 LBL_TREAT_MODELS_WITH_SUBPARTS_AS_PARTS */ {"treatmodelswithsubpartsasparts","1",          QObject::tr("Treat Subparts As Parts"),  QObject::tr("Treat subpart like a part by merging its constituent parts into one object")},
    /* 34 LBL_TREAT_SHORTCUT_AS_MODEL             */ {"treatshortcutasmodel",          "0",          QObject::tr("Treat Shortcuts As Models"),QObject::tr("Split shortcut parts into their constituent pieces as if they were models")},
    /* 35 LBL_TRIANGULATE                         */ {"triangulate",                   "0",          QObject::tr("Triangulate Faces"),        QObject::tr("Triangulate all faces")},
    /* 36 LBL_USE_ARCHIVE_LIBRARY_MM              */ {"usearchivelibrary",             "0",          QObject::tr("Use Archive Libraries"),    QObject::tr("Add any archive (zip) libraries in the LDraw file path to the library search list")},
    /* 37 LBL_USE_FREESTYLE_EDGES                 */ {"usefreestyleedges",             "0",          QObject::tr("Use Freestyle Edges"),      QObject::tr("Render LDraw edges using freestyle")},
    /* 38 LBL_VERBOSE_MM                          */ {"verbose",                       "1",          QObject::tr("Verbose output"),           QObject::tr("Output all messages while working, else only show warnings and errors")},

    /* 39/00 LBL_CAMERA_BORDER_PERCENT_MM         */ {"cameraborderpercent",           "5",        QObject::tr("Camera Border Percent"),    QObject::tr("When positioning the camera, include a (percentage) border around the model in the render")},
    /* 40/01 LBL_FRAMES_PER_STEP                  */ {"framesperstep",                 "3",          QObject::tr("Frames Per Step"),          QObject::tr("Frames per step")},
    /* 41/02 LBL_GAP_SCALE                        */ {"gapscale",                      "0.997",      QObject::tr("Gap Scale"),                QObject::tr("Scale individual parts by this much to create the gap")},
    /* 42/03 LBL_IMPORT_SCALE                     */ {"importscale",                   "0.02",       QObject::tr("Import Scale"),             QObject::tr("What scale to import at. Full scale is 1.0 and is so huge that it is unwieldy in the viewport")},
    /* 43/04 LBL_MERGE_DISTANCE                   */ {"mergedistance",                 "0.05",       QObject::tr("Merge Distance"),           QObject::tr("Maximum distance between elements to merge")},
    /* 44/05 LBL_RENDER_PERCENTAGE_MM             */ {"renderpercentage",              "100",        QObject::tr("Render Percentage"),        QObject::tr("Sets the rendered image percentage scale for its pixel resolution (default is 100)")},
    /* 45/06 LBL_RESOLUTION_WIDTH                 */ {"resolutionwidth",               "800",        QObject::tr("Image Width"),              QObject::tr("Sets the rendered image width in pixels")},
    /* 46/07 LBL_RESOLUTION_HEIGHT                */ {"resolutionheight",              "600",        QObject::tr("Image Height"),             QObject::tr("Sets the rendered image height in pixels")},
    /* 47/08 LBL_STARTING_STEP_FRAME              */ {"startingstepframe",             "1",          QObject::tr("Starting Step Frame"),      QObject::tr("Frame to add the first STEP meta command")},

    /* 48/00 LBL_CHOSEN_LOGO                      */ {"chosenlogo",                    "logo3",      QObject::tr("Chosen Logo"),              QObject::tr("Which logo to display. logo and logo2 aren't used and are only included for completeness")},
    /* 49/01 LBL_COLOUR_SCHEME_MM                 */ {"usecolourscheme",               "lgeo",       QObject::tr("Colour Scheme"),            QObject::tr("Colour scheme options - Realistic (lgeo), Original (LDConfig), Alternate (LDCfgalt), Custom (User Defined)")},
    /* 50/02 LBL_GAP_SCALE_STRATEGY               */ {"gapscalestrategy",              "constraint", QObject::tr("Gap Strategy"),             QObject::tr("Apply gap to object directly or scale and empty to adjust to gaps between parts")},
    /* 51/03 LBL_GAP_TARGET                       */ {"gaptarget",                     "object",     QObject::tr("Gap Target"),               QObject::tr("Whether to scale the object data or mesh data")},
    /* 52/04 LBL_RESOLUTION_MM                    */ {"resolution",                    "Standard",   QObject::tr("Resolution"),               QObject::tr("Resolution of part primitives, ie. how much geometry they have")},
    /* 53/05 LBL_SMOOTH_TYPE                      */ {"smoothtype",                    "edge_split", QObject::tr("Smooth Type"),              QObject::tr("Use either autosmooth or an edge split modifier to smooth part faces")}
};

BlenderPreferences::ComboOptItems  BlenderPreferences::comboOptItemsMM [] = {
    /*    FIRST item set as default        Data                      Item: */
    /* 00 LBL_CHOSEN_LOGO              */ {"logo|logo2|logo3",       "Logo (Not Used)|Logo2 (Not Used)|Flattened(3)"},
    /* 01 LBL_COLOUR_SCHEME_MM         */ {"lgeo|ldraw|alt|custom",  "Realistic Colours|Original LDraw Colours|Alternate LDraw Colours|Custom Colours"},
    /* 02 LBL_GAP_SCALE_STRATEGY       */ {"object|constraint",      "Gap applied directly to object|Gap scaled to adjust to gaps between parts"},
    /* 03 LBL_GAP_TARGET               */ {"object|mesh",            "Scale object data|Scale mesh data"},
    /* 04 LBL_RESOLUTION_MM            */ {"Low|Standard|High",      "Low Resolution Primitives|Standard Primitives|High Resolution Primitives"},
    /* 05 LBL_SMOOTH_TYPE              */ {"auto_smooth|edge_split", "Auto-smooth part faces|Smooth part faces with edge split modifier"}
};

BlenderPreferences *blenderPreferences;

BlenderPreferences::BlenderPreferences(QWidget *parent)
    : QWidget{parent}
{
    blenderPreferences = this;
}

BlenderPreferences::~BlenderPreferences()
{
    clear();
    blenderPreferences = nullptr;
}

void BlenderPreferences::clear()
{
    pathLineEditList.clear();
    pathBrowseButtonList.clear();
    settingLabelList.clear();
    checkBoxList.clear();
    lineEditList.clear();
    comboBoxList.clear();
}

void BlenderPreferences::getRenderSettings(
    int    &width,
    int    &height,
    double &renderPercentage,
    bool docRender)
{
    dialog = new QDialog(this);

    dialog->setLayout(new QVBoxLayout);

    dialog->setWindowTitle(tr("Blender Render Settings"));

    dialog->setWhatsThis(lpubWT(WT_DIALOG_BLENDER_RENDER_SETTINGS,dialog->windowTitle()));

#ifndef QT_NO_PROCESS
    process = nullptr;
#endif

    mWidth  = width;
    mHeight = height;
    mRenderPercentage = renderPercentage;
    mDocumentRender = docRender;

    mDialogCancelled = false;

    mBlenderConfigured = Preferences::blenderInstalled && !Preferences::blenderLDrawConfigFile.isEmpty();

    clear();

    loadSettings();

    blenderContent = new QWidget(dialog);
    blenderContent->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);

    blenderForm = new QFormLayout(blenderContent);

    QPalette ReadOnlyPalette = QApplication::palette();
    if (Preferences::displayTheme == THEME_DARK)
        ReadOnlyPalette.setColor(QPalette::Base,QColor(Preferences::themeColors[THEME_DARK_PALETTE_MIDLIGHT]));
    else
        ReadOnlyPalette.setColor(QPalette::Base,QColor(Preferences::themeColors[THEME_DEFAULT_PALETTE_LIGHT]));
    ReadOnlyPalette.setColor(QPalette::Text,QColor(LPUB3D_DISABLED_TEXT_COLOUR));

    // Blender Executable
    QGroupBox *blenderExeBox = new QGroupBox(tr("Blender Executable"),blenderContent);
    blenderForm->addRow(blenderExeBox);
    blenderExeGridLayout = new QGridLayout(blenderExeBox);
    blenderExeBox->setLayout(blenderExeGridLayout);
    const int i = LBL_BLENDER_PATH;
    QLabel *pathLabel = new QLabel(blenderPaths[i].label, blenderContent);
    pathLabel->setToolTip(blenderPaths[i].tooltip);
    blenderExeGridLayout->addWidget(pathLabel,1,0);

    QLineEdit *pathLineEdit = new QLineEdit(blenderContent);
    pathLineEdit->setText(blenderPaths[i].value);
    pathLineEdit->setObjectName(blenderPaths[i].key);
    pathLineEdit->setToolTip(blenderPaths[i].tooltip);
    pathLineEditList << pathLineEdit;
    blenderExeGridLayout->addWidget(pathLineEdit,1,1);

    QPushButton *pathBrowseButton = new QPushButton(tr("Browse..."), blenderContent);
    pathBrowseButton->setObjectName(blenderPaths[i].key);
    pathBrowseButtonList << pathBrowseButton;
    blenderExeGridLayout->addWidget(pathBrowseButton,1,2);

    QObject::connect(pathLineEdit, SIGNAL(editingFinished()),
                     this,         SLOT(configureBlender()));
    QObject::connect(pathBrowseButton, SIGNAL(clicked(bool)),
                     this,             SLOT(browseBlender(bool)));

    // Blender Version
    blenderVersionLabel = new QLabel(blenderContent);
    blenderExeGridLayout->addWidget(blenderVersionLabel,0,0);

    blenderVersionEdit = new QLineEdit(blenderContent);
    blenderVersionEdit->setPalette(ReadOnlyPalette);
    blenderVersionEdit->setReadOnly(true);
    blenderExeGridLayout->addWidget(blenderVersionEdit,0,1,1,2);

    // LDraw Addon
    QGroupBox *blenderAddonVersionBox = new QGroupBox(tr("%1 Blender LDraw Addon").arg(VER_PRODUCTNAME_STR),blenderContent);
    blenderForm->addRow(blenderAddonVersionBox);

    blenderVersionGridLayout = new QGridLayout(blenderAddonVersionBox);
    blenderAddonVersionBox->setLayout(blenderVersionGridLayout);

    blenderAddonVersionLabel = new QLabel(tr("Addon Version"), blenderContent);
    blenderVersionGridLayout->addWidget(blenderAddonVersionLabel,0,0);

    blenderAddonVersionEdit = new QLineEdit(blenderContent);
    blenderAddonVersionEdit->setToolTip(tr("%1 Blender LDraw import and image renderer addon").arg(VER_PRODUCTNAME_STR));
    blenderAddonVersionEdit->setPalette(ReadOnlyPalette);
    blenderAddonVersionEdit->setReadOnly(true);
    blenderVersionGridLayout->addWidget(blenderAddonVersionEdit,0,1);
    blenderVersionGridLayout->setColumnStretch(1,1/*1 is greater than 0 (default)*/);

    blenderAddonModulesBox = new QGroupBox(tr("Enabled Addon Modules"),blenderContent);
    QHBoxLayout *blenderAddonModulesHLayout = new QHBoxLayout(blenderAddonModulesBox);
    blenderAddonModulesBox->setLayout(blenderAddonModulesHLayout);
    blenderVersionGridLayout->addWidget(blenderAddonModulesBox,1,0,1,4);

    blenderImportActBox = new QCheckBox(tr("LDraw Import TN"),blenderContent);
    blenderImportActBox->setToolTip(tr("Enable addon import module (adapted from LDraw Import by Toby Nelson) in Blender"));
    blenderAddonModulesHLayout->addWidget(blenderImportActBox);
    QObject::connect(blenderImportActBox, SIGNAL(clicked(bool)),
                     this,                SLOT(enableImportModule()));

    blenderImportMMActBox = new QCheckBox(tr("LDraw Import MM"),blenderContent);
    blenderImportMMActBox->setToolTip(tr("Enable addon import module (adapted from LDraw Import by Matthew Morrison) in Blender"));
    QObject::connect(blenderImportMMActBox, SIGNAL(clicked(bool)),
                     this,                  SLOT(enableImportModule()));
    blenderAddonModulesHLayout->addWidget(blenderImportMMActBox);

    blenderRenderActBox = new QCheckBox(tr("%1 Image Render").arg(VER_PRODUCTNAME_STR), blenderContent);
    blenderRenderActBox->setToolTip(tr("Addon image render module in Blender"));
    blenderRenderActBox->setEnabled(false);
    blenderAddonModulesHLayout->addWidget(blenderRenderActBox);

    const QString importModule = Preferences::blenderImportModule == QLatin1String("TN")
                                     ? tr("Import TN")
                                     : Preferences::blenderImportModule == QLatin1String("MM")
                                           ? tr("Import MM")
                                           : "";

    if (mBlenderConfigured) {
        blenderVersionLabel->setText(tr("Blender Version"));
        blenderVersionEdit->setText(blenderVersion);
        if (!blenderAddonVersion.isEmpty()) {
            blenderAddonModulesBox->setEnabled(true);
            blenderAddonVersionEdit->setText(blenderAddonVersion);
            blenderRenderActBox->setChecked(true);
            blenderImportActBox->setChecked(importModule.endsWith(" TN"));
            blenderImportMMActBox->setChecked(importModule.endsWith(" MM"));
        }
    } else {
        const QString colour = Preferences::displayTheme == THEME_DARK
                                   ? Preferences::themeColors[THEME_DARK_DECORATE_LPUB3D_QUOTED_TEXT]
                                   : QLatin1String("blue");
        blenderVersionLabel->setStyleSheet(QString("QLabel { color : %1; }").arg(colour));
        blenderVersionLabel->setText(tr("Blender not configured"));
        blenderVersionEdit->setVisible(mBlenderConfigured);
        // set default LDraw import module
        blenderImportActBox->setChecked(true);
    }

    blenderAddonUpdateButton = new QPushButton(tr("Update..."), blenderContent);
    blenderAddonUpdateButton->setToolTip(tr("Update %1 Blender LDraw addon").arg(VER_PRODUCTNAME_STR));
    blenderAddonUpdateButton->setEnabled(!pathLineEditList[LBL_BLENDER_PATH]->text().isEmpty());
    blenderVersionGridLayout->addWidget(blenderAddonUpdateButton,0,2);
    QObject::connect(blenderAddonUpdateButton, SIGNAL(clicked(bool)),
                     this,                     SLOT(updateLDrawAddon()));

    blenderAddonStdOutButton = new QPushButton(tr("Output..."), blenderContent);
    blenderAddonStdOutButton->setToolTip(tr("Open the standrd output log"));
    blenderAddonStdOutButton->setEnabled(false);
    blenderVersionGridLayout->addWidget(blenderAddonStdOutButton,0,3);
    QObject::connect(blenderAddonStdOutButton, SIGNAL(clicked(bool)),
                     this,                     SLOT(getStandardOutput()));

    mBlenderAddonUpdate = false;

    // Paths
    blenderPathsBox = new QGroupBox(tr("LDraw %2 Addon Paths").arg(importModule),blenderContent);
    blenderForm->addRow(blenderPathsBox);

    // Settings
    blenderSettingsBox = new QGroupBox(tr("LDraw %1 Addon Settings").arg(importModule),blenderContent);
    blenderForm->addRow(blenderSettingsBox);

    if (blenderImportMMActBox->isChecked())
        initLDrawImportMM();
    else
        initLDrawImport();

    // Scroll area
    QScrollArea *scrollArea = new QScrollArea;
    scrollArea->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
    scrollArea->setWidget(blenderContent);
    dialog->layout()->addWidget(scrollArea);

    // Button box
    QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
                               Qt::Horizontal, dialog);
    dialog->layout()->addWidget(&buttonBox);
    QObject::connect(&buttonBox, SIGNAL(accepted()), dialog, SLOT(accept()));
    QObject::connect(&buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    QObject::connect(dialog,     SIGNAL(rejected()), this, SLOT(reject()));

    pathsGroupButton = new QPushButton(tr("Show Paths"));
    buttonBox.addButton(pathsGroupButton,QDialogButtonBox::ActionRole);
    QObject::connect(pathsGroupButton,SIGNAL(clicked()), this,SLOT(showPathsGroup()));

    QPushButton *resetButton = new QPushButton(tr("Reset"));
    buttonBox.addButton(resetButton,QDialogButtonBox::ActionRole);
    QObject::connect(resetButton,SIGNAL(clicked()), this,SLOT(resetSettings()));

    dialog->setModal(true);

    dialog->setMinimumSize(200,400);

    if (dialog->exec() == QDialog::Accepted) {
        if (settingsModified(width, height, renderPercentage))
            saveSettings();
    } else if (!mDialogCancelled) {
        if (settingsModified(width, height, renderPercentage))
            if (promptAccept())
                saveSettings();
    }
}

void BlenderPreferences::initLDrawImport()
{
    if (!numSettings())
        loadSettings();

    // Paths
    blenderPathsLayout = new QGridLayout(blenderPathsBox);
    blenderPathsBox->setLayout(blenderPathsLayout);

    for(int i = 1/*skip blender executable*/; i < numPaths(); ++i) {
        int j = i - 1; // adjust for skipping first item - blender executable
        bool isVisible = i != LBL_STUDIO_LDRAW_PATH;
        QLabel *pathLabel = new QLabel(blenderPaths[i].label, blenderPathsBox);
        pathLabel->setToolTip(blenderPaths[i].tooltip);
        blenderPathsLayout->addWidget(pathLabel,j,0);
        pathLabel->setVisible(isVisible);

        QLineEdit *pathLineEdit = new QLineEdit(blenderPathsBox);
        pathLineEdit->setText(blenderPaths[i].value);
        pathLineEdit->setToolTip(blenderPaths[i].tooltip);
        pathLineEditList << pathLineEdit;
        blenderPathsLayout->addWidget(pathLineEdit,j,1);
        pathLineEdit->setVisible(isVisible);

        QPushButton *pathBrowseButton = new QPushButton(tr("Browse..."), blenderPathsBox);
        pathBrowseButtonList << pathBrowseButton;
        blenderPathsLayout->addWidget(pathBrowseButton,j,2);
        pathBrowseButton->setVisible(isVisible);

        if (isVisible)
            QObject::connect(pathBrowseButton, SIGNAL(clicked(bool)),
                             this,             SLOT(browseBlender(bool)));
    }

    blenderPathsBox->setEnabled(mBlenderConfigured);
    blenderPathsBox->hide();

    // Settings
    settingLabelList.clear();
    checkBoxList.clear();
    lineEditList.clear();
    comboBoxList.clear();

    settingsSubform = new QFormLayout(blenderSettingsBox);

    int comboBoxItemsIndex = 0;

    for(int i = 0; i < numSettings(); i++) {
        QLabel *label = new QLabel(blenderSettingsBox);
        if (i == LBL_CROP_IMAGE)
            label->setText(QString("%1 (%2 x %3)")
                               .arg(blenderSettings[i].label)
                               .arg(gui->GetImageWidth()).arg(gui->GetImageHeight()));
        else
            label->setText(blenderSettings[i].label);
        label->setToolTip(blenderSettings[i].tooltip);
        settingLabelList << label;

        if (i < LBL_BEVEL_WIDTH){           // QCheckBoxes
            QCheckBox *checkBox = new QCheckBox(blenderSettingsBox);
            checkBox->setChecked(blenderSettings[i].value.toInt());
            checkBox->setToolTip(blenderSettings[i].tooltip);
            if (mDocumentRender &&
                (i == LBL_ADD_ENVIRONMENT ||
                 i == LBL_CROP_IMAGE ||
                 i == LBL_TRANSPARENT_BACKGROUND)) {
                checkBox->setEnabled(false);
            }
            if (!mDocumentRender &&
                i == LBL_CROP_IMAGE)
                QObject::connect(checkBox,SIGNAL(toggled(bool)),
                                 this,SLOT  (setModelSize(bool)));
            checkBoxList << checkBox;
            settingsSubform->addRow(label,checkBox);
        } else if (i < LBL_COLOUR_SCHEME) { // QLineEdits
            QLineEdit *lineEdit = new QLineEdit(blenderSettingsBox);
            if (i == LBL_IMAGE_WIDTH || i == LBL_IMAGE_HEIGHT){
                connect(lineEdit,SIGNAL(textChanged(const QString &)),
                        this,    SLOT  (sizeChanged(const QString &)));
                lineEdit->setValidator(new QIntValidator(16, RENDER_IMAGE_MAX_SIZE));
            } else if(i == LBL_DEFAULT_COLOUR) {
                lineEdit->setReadOnly(true);
                lineEdit->setStyleSheet("Text-align:left");
                QImage img(16, 16, QImage::Format_ARGB32);
                img.fill(0);
                defaultColourEditAction = lineEdit->addAction(QIcon(QPixmap::fromImage(img)), QLineEdit::TrailingPosition);
                QObject::connect(defaultColourEditAction, SIGNAL(triggered(bool)),
                                 this,                    SLOT  (colorButtonClicked(bool)));
            } else {
                lineEdit->setText(blenderSettings[i].value);
                if (i == LBL_IMAGE_SCALE)
                    lineEdit->setValidator(new QDoubleValidator(0.01,10.0,2));
                else if (i == LBL_RENDER_PERCENTAGE || i == LBL_CAMERA_BORDER_PERCENT)
                    lineEdit->setValidator(new QIntValidator(1,1000));
                else
                    lineEdit->setValidator(new QDoubleValidator(0.01,100.0,2));
            }
            lineEdit->setToolTip(blenderSettings[i].tooltip);
            lineEditList << lineEdit;
            settingsSubform->addRow(label,lineEdit);
        } else {                            // QComboBoxes
            QComboBox *comboBox = new QComboBox(blenderSettingsBox);
            QString value        = blenderSettings[i].value;
            QStringList itemList = comboOptItems[comboBoxItemsIndex].items.split("|");
            QStringList dataList = comboOptItems[comboBoxItemsIndex].data.split("|");
            comboBox->addItems(itemList);
            for (int j = 0; j < comboBox->count(); j++)
                comboBox->setItemData(j, dataList.at(j));
            comboBox->setToolTip(blenderSettings[i].tooltip);
            int currentIndex = int(comboBox->findData(QVariant::fromValue(value)));
            comboBox->setCurrentIndex(currentIndex);
            if (i == LBL_COLOUR_SCHEME)
                QObject::connect(comboBox,SIGNAL(currentIndexChanged(QString const &)),
                                 this,    SLOT  (validateColourScheme(QString const &)));
            comboBoxList << comboBox;
            comboBoxItemsIndex++;
            settingsSubform->addRow(label,comboBox);
        }
    }

    setDefaultColor(blenderSettings[LBL_DEFAULT_COLOUR].value.toInt());
    setModelSize(blenderSettings[LBL_CROP_IMAGE].value.toInt());

    if (!settingsSubform->rowCount())
        settingsSubform = nullptr;

    blenderSettingsBox->setEnabled(mBlenderConfigured);
}

void BlenderPreferences::initLDrawImportMM()
{
    if (!numSettingsMM())
        loadSettings();

    // Paths
    blenderPathsLayout = new QGridLayout(blenderPathsBox);
    blenderPathsBox->setLayout(blenderPathsLayout);

    for(int i = 1/*skip blender executable*/; i < numPaths(); ++i) {
        int j = i - 1; // adjust for skipping first item - blender executable
        bool isVisible = i != LBL_LSYNTH_PATH && i != LBL_STUD_LOGO_PATH;
        QLabel *pathLabel = new QLabel(blenderPaths[i].label, blenderPathsBox);
        pathLabel->setToolTip(blenderPaths[i].tooltip);
        blenderPathsLayout->addWidget(pathLabel,j,0);
        pathLabel->setVisible(isVisible);

        QLineEdit *pathLineEdit = new QLineEdit(blenderPathsBox);
        pathLineEdit->setText(blenderPaths[i].value);
        pathLineEdit->setToolTip(blenderPaths[i].tooltip);
        pathLineEditList << pathLineEdit;
        blenderPathsLayout->addWidget(pathLineEdit,j,1);
        pathLineEdit->setVisible(isVisible);

        QPushButton *pathBrowseButton = new QPushButton(tr("Browse..."), blenderPathsBox);
        pathBrowseButtonList << pathBrowseButton;
        blenderPathsLayout->addWidget(pathBrowseButton,j,2);
        pathBrowseButton->setVisible(isVisible);

        if (isVisible)
            QObject::connect(pathBrowseButton, SIGNAL(clicked(bool)),
                             this,             SLOT(browseBlender(bool)));
    }

    blenderPathsBox->setEnabled(mBlenderConfigured);
    blenderPathsBox->hide();

    // Settings
    settingLabelList.clear();
    checkBoxList.clear();
    lineEditList.clear();
    comboBoxList.clear();

    settingsSubform = new QFormLayout(blenderSettingsBox);

    int comboBoxItemsIndex = 0;

    for(int i = 0; i < numSettingsMM(); i++) {
        QLabel *label = new QLabel(blenderSettingsBox);
        if (i == LBL_CROP_IMAGE_MM)
            label->setText(QString("%1 (%2 x %3)")
                               .arg(blenderSettingsMM[i].label)
                               .arg(gui->GetImageWidth()).arg(gui->GetImageHeight()));
        else
            label->setText(blenderSettingsMM[i].label);
        label->setToolTip(blenderSettingsMM[i].tooltip);
        settingLabelList << label;

        if (i < LBL_CAMERA_BORDER_PERCENT_MM){ // QCheckBoxes
            QCheckBox *checkBox = new QCheckBox(blenderSettingsBox);
            checkBox->setChecked(blenderSettingsMM[i].value.toInt());
            checkBox->setToolTip(blenderSettingsMM[i].tooltip);
            if (mDocumentRender &&
                (i == LBL_ADD_ENVIRONMENT_MM ||
                 i == LBL_CROP_IMAGE_MM ||
                 i == LBL_TRANSPARENT_BACKGROUND_MM)) {
                checkBox->setEnabled(false);
            }
            if (!mDocumentRender &&
                i == LBL_CROP_IMAGE_MM)
                QObject::connect(checkBox,SIGNAL(toggled(bool)),
                                 this,SLOT  (setModelSize(bool)));
            checkBoxList << checkBox;
            settingsSubform->addRow(label,checkBox);
        } else if (i < LBL_CHOSEN_LOGO) { // QLineEdits
            QLineEdit *lineEdit = new QLineEdit(blenderSettingsBox);
            if (i == LBL_RESOLUTION_WIDTH || i == LBL_RESOLUTION_HEIGHT){
                connect(lineEdit,SIGNAL(textChanged(const QString &)),
                        this,    SLOT  (sizeChanged(const QString &)));
                lineEdit->setValidator(new QIntValidator(16, RENDER_IMAGE_MAX_SIZE));
            } else {
                lineEdit->setText(blenderSettingsMM[i].value);
                if (i == LBL_IMPORT_SCALE)
                    lineEdit->setValidator(new QDoubleValidator(0.01,10.0,2));
                else if (i == LBL_RENDER_PERCENTAGE_MM || i == LBL_CAMERA_BORDER_PERCENT_MM)
                    lineEdit->setValidator(new QIntValidator(1,1000));
                else if (i == LBL_GAP_SCALE || i == LBL_MERGE_DISTANCE)
                    lineEdit->setValidator(new QDoubleValidator(0.001,100.0,3));
                else
                    lineEdit->setValidator(new QIntValidator(1, RENDER_IMAGE_MAX_SIZE));
            }
            lineEdit->setToolTip(blenderSettingsMM[i].tooltip);
            lineEditList << lineEdit;
            settingsSubform->addRow(label,lineEdit);
        } else {                            // QComboBoxes
            QComboBox *comboBox = new QComboBox(blenderSettingsBox);
            QString value        = blenderSettingsMM[i].value;
            QStringList itemList = comboOptItemsMM[comboBoxItemsIndex].items.split("|");
            QStringList dataList = comboOptItemsMM[comboBoxItemsIndex].data.split("|");
            comboBox->addItems(itemList);
            for (int j = 0; j < comboBox->count(); j++)
                comboBox->setItemData(j, dataList.at(j));
            comboBox->setToolTip(blenderSettingsMM[i].tooltip);
            int currentIndex = int(comboBox->findData(QVariant::fromValue(value)));
            comboBox->setCurrentIndex(currentIndex);
            if (i == LBL_COLOUR_SCHEME_MM)
                QObject::connect(comboBox,SIGNAL(currentIndexChanged(QString const &)),
                                 this,    SLOT  (validateColourScheme(QString const &)));
            comboBoxList << comboBox;
            comboBoxItemsIndex++;
            settingsSubform->addRow(label,comboBox);
        }
    }

    setModelSize(blenderSettingsMM[LBL_CROP_IMAGE_MM].value.toInt());

    if (!settingsSubform->rowCount())
        settingsSubform = nullptr;

    blenderSettingsBox->setEnabled(mBlenderConfigured);
}

void BlenderPreferences::enableImportModule()
{
    QString preferredImportModule;
    if (sender() == blenderImportActBox && blenderImportActBox->isChecked()) {
        preferredImportModule = QLatin1String("TN");
        blenderImportMMActBox->setChecked(false);
    } else if (sender() == blenderImportMMActBox && blenderImportMMActBox->isChecked()) {
        preferredImportModule = QLatin1String("MM");
        blenderImportActBox->setChecked(false);
    }

    if (preferredImportModule.isEmpty())
        return;

    mDialogCancelled = true;
    dialog->accept();

    Preferences::setBlenderImportModule(preferredImportModule);

    getRenderSettings(mWidth,mHeight,mRenderPercentage,mDocumentRender);
}

bool BlenderPreferences::settingsModified(int &width, int &height, double &renderPercentage)
{
    if  (mDialogCancelled)
        return false;

    bool ok, modified = false;
    qreal _width = 0.0, _height = 0.0, _renderPercentage = 0.0, _value = 0.0, _oldValue = 0.0;
    QString oldValue;

    auto itemChanged = [] (qreal oldValue, qreal newValue) {
        return newValue > oldValue || newValue < oldValue;
    };

    if (blenderImportMMActBox->isChecked()) {
        // settings
        for(int i = 0; i < numSettingsMM(); i++) {
            // checkboxes
            if (i < LBL_CAMERA_BORDER_PERCENT_MM) {
                for(int j = 0; j < checkBoxList.size(); j++) {
                    oldValue = blenderSettingsMM[i].value;
                    blenderSettingsMM[i].value = QString::number(checkBoxList[j]->isChecked());
                    if (!modified)
                        modified = blenderSettingsMM[i].value != oldValue;
                    if (i < LBL_VERBOSE_MM)
                        i++;
                }
            }
            // lineedits
            else if (i < LBL_CHOSEN_LOGO) {
                for(int j = 0; j < lineEditList.size(); j++) {
                    if (j == RESOLUTION_WIDTH_EDIT) {
                        _oldValue = width;
                        _width  = lineEditList[j]->text().toDouble(&ok);
                        if (ok) {
                            width = int(_width);
                            blenderSettingsMM[i].value = QString::number(width);
                            if (!modified)
                                modified = itemChanged(_oldValue, _width);
                        }
                    } else if (j == RESOLUTION_HEIGHT_EDIT) {
                        _oldValue = height;
                        _height = lineEditList[j]->text().toDouble(&ok);
                        if (ok) {
                            height = int(_height);
                            blenderSettingsMM[i].value = QString::number(height);
                            if (!modified)
                                modified = itemChanged(_oldValue, _height);
                        }
                    } else if (j == LBL_RENDER_PERCENTAGE_MM) {
                        _oldValue = renderPercentage;
                        _renderPercentage = lineEditList[j]->text().toInt(&ok);
                        if (ok) {
                            renderPercentage = double(_renderPercentage / 100);
                            blenderSettingsMM[i].value = QString::number(_renderPercentage);
                            if (!modified)
                                modified = itemChanged(_oldValue, renderPercentage);
                        }
                    } else {
                        _oldValue = blenderSettingsMM[i].value.toDouble();
                        _value = lineEditList[j]->text().toDouble(&ok);
                        if (ok) {
                            blenderSettingsMM[i].value = QString::number(_value);
                            if (!modified)
                                modified = itemChanged(_oldValue, _value);
                        }
                    }
                    if (i < LBL_STARTING_STEP_FRAME)
                        i++;
                }
            }
            // comboboxes
            else {
                for(int j = 0; j < comboBoxList.size(); j++) {
                    oldValue = blenderSettingsMM[i].value;
                    QString value = comboBoxList[j]->itemData(comboBoxList[j]->currentIndex()).toString();
                    blenderSettingsMM[i].value = value;
                    if (!modified)
                        modified = value != oldValue;
                    i++;
                }
            }
        }
    } else {
        // settings
        for(int i = 0; i < numSettings(); i++) {
            // checkboxes
            if (i < LBL_BEVEL_WIDTH) {
                for(int j = 0; j < checkBoxList.size(); j++) {
                    oldValue = blenderSettings[i].value;
                    blenderSettings[i].value = QString::number(checkBoxList[j]->isChecked());
                    if (!modified)
                        modified = blenderSettings[i].value != oldValue;
                    if (i < LBL_VERBOSE)
                        i++;
                }
            }
            // lineedits
            else if (i < LBL_COLOUR_SCHEME) {
                for(int j = 0; j < lineEditList.size(); j++) {
                    if (j == IMAGE_WIDTH_EDIT) {
                        _oldValue = width;
                        _width  = lineEditList[j]->text().toDouble(&ok);
                        if (ok) {
                            width = int(_width);
                            blenderSettings[i].value = QString::number(width);
                            if (!modified)
                                modified = itemChanged(_oldValue, _width);
                        }
                    } else if (j == IMAGE_HEIGHT_EDIT) {
                        _oldValue = height;
                        _height = lineEditList[j]->text().toDouble(&ok);
                        if (ok) {
                            height = int(_height);
                            blenderSettings[i].value = QString::number(height);
                            if (!modified)
                                modified = itemChanged(_oldValue, _height);
                        }
                    } else if (j == LBL_RENDER_PERCENTAGE) {
                        _oldValue = renderPercentage;
                        _renderPercentage = lineEditList[j]->text().toInt(&ok);
                        if (ok) {
                            renderPercentage = double(_renderPercentage / 100);
                            blenderSettings[i].value = QString::number(_renderPercentage);
                            if (!modified)
                                modified = itemChanged(_oldValue, renderPercentage);
                        }
                    } else {
                        _oldValue = blenderSettings[i].value.toDouble();
                        _value = j == DEFAULT_COLOUR_EDIT ? lineEditList[j]->property("ColourID").toDouble(&ok)
                                                          : lineEditList[j]->text().toDouble(&ok);
                        if (ok) {
                            blenderSettings[i].value = QString::number(j == DEFAULT_COLOUR_EDIT ? int(_value) : _value);
                            if (!modified)
                                modified = itemChanged(_oldValue, _value);
                        }
                    }
                    if (i < LBL_RENDER_PERCENTAGE)
                        i++;
                }
            }
            // comboboxes
            else {
                for(int j = 0; j < comboBoxList.size(); j++) {
                    oldValue = blenderSettings[i].value;
                    QString value = comboBoxList[j]->itemData(comboBoxList[j]->currentIndex()).toString();
                    blenderSettings[i].value = value;
                    if (!modified)
                        modified = value != oldValue;
                    i++;
                }
            }
        }
    }

    // paths
    for (int i = 0; i < numPaths(); i++) {
        oldValue = blenderPaths[i].value;
        QString value = pathLineEditList[i]->text();
        blenderPaths[i].value = value;
        if (!modified)
            modified = value != oldValue;
    }

    return modified;
}

void BlenderPreferences::resetSettings()
{
    blenderPaths[LBL_BLENDER_PATH].value = Preferences::blenderExe;
    const QStringList versionItems       = Preferences::blenderVersion.split("|");
    blenderVersion                       = versionItems.first();
    blenderAddonVersion                  = versionItems.size() == 2 ? versionItems.last() : QString();

    blenderVersionEdit->setText(blenderVersion);
    blenderAddonVersionEdit->setText(blenderAddonVersion);

    mBlenderConfigured = !Preferences::blenderExe.isEmpty() && !Preferences::blenderLDrawConfigFile.isEmpty();;

    if (blenderImportActBox->isChecked()) {
        disconnect(lineEditList[IMAGE_HEIGHT_EDIT],SIGNAL(textChanged(const QString &)),
                   this,                           SLOT  (sizeChanged(const QString &)));
        disconnect(lineEditList[IMAGE_WIDTH_EDIT],SIGNAL(textChanged(const QString &)),
                   this,                          SLOT  (sizeChanged(const QString &)));

        for(int i = 0; i < numSettings(); i++) {
            if (i < LBL_BEVEL_WIDTH) {
                for(int j = 0; j < checkBoxList.size(); j++) {
                    checkBoxList[j]->setChecked(blenderSettings[i].value.toInt());
                    if (i < LBL_VERBOSE)
                        i++;
                }
            } else if (i < LBL_COLOUR_SCHEME) {
                for(int j = 0; j < lineEditList.size(); j++) {
                    if (j == IMAGE_WIDTH_EDIT)
                        lineEditList[j]->setText(QString::number(mWidth));
                    else if (j == IMAGE_HEIGHT_EDIT)
                        lineEditList[j]->setText(QString::number(mHeight));
                    else if (j == RENDER_PERCENTAGE_EDIT)
                        lineEditList[j]->setText(QString::number(mRenderPercentage * 100));
                    else if (j == DEFAULT_COLOUR_EDIT)
                        setDefaultColor(blenderSettings[LBL_DEFAULT_COLOUR].value.toInt());
                    else
                        lineEditList[j]->setText(blenderSettings[i].value);
                    if (i < LBL_RENDER_PERCENTAGE)
                        i++;
                }
            } else {
                for(int j = 0; j < comboBoxList.size(); j++) {
                    comboBoxList[j]->setCurrentIndex(int(comboBoxList[j]->findText(blenderSettings[i].value)));
                    i++;
                }
            }
        }
        for (int i = 0; i < numPaths(); i++) {
            pathLineEditList[i]->setText(blenderPaths[i].value);
        }

        connect(lineEditList[IMAGE_HEIGHT_EDIT],SIGNAL(textChanged(const QString &)),
                this,SLOT  (sizeChanged(const QString &)));
        connect(lineEditList[IMAGE_WIDTH_EDIT],SIGNAL(textChanged(const QString &)),
                this,SLOT  (sizeChanged(const QString &)));

    } else if (blenderImportMMActBox->isChecked()) {

        disconnect(lineEditList[RESOLUTION_HEIGHT_EDIT],SIGNAL(textChanged(const QString &)),
                   this,                                 SLOT  (sizeChanged(const QString &)));
        disconnect(lineEditList[RESOLUTION_WIDTH_EDIT],SIGNAL(textChanged(const QString &)),
                   this,                               SLOT  (sizeChanged(const QString &)));

        for(int i = 0; i < numSettingsMM(); i++) {
            if (i < LBL_CAMERA_BORDER_PERCENT_MM) {
                for(int j = 0; j < checkBoxList.size(); j++) {
                    checkBoxList[j]->setChecked(blenderSettingsMM[i].value.toInt());
                    if (i < LBL_VERBOSE_MM)
                        i++;
                }
            } else if (i < LBL_CHOSEN_LOGO) {
                for(int j = 0; j < lineEditList.size(); j++) {
                    if (j == RESOLUTION_WIDTH_EDIT)
                        lineEditList[j]->setText(QString::number(mWidth));
                    else if (j == RESOLUTION_HEIGHT_EDIT)
                        lineEditList[j]->setText(QString::number(mHeight));
                    else if (j == RENDER_PERCENTAGE_EDIT_MM)
                        lineEditList[j]->setText(QString::number(mRenderPercentage * 100));
                    else
                        lineEditList[j]->setText(blenderSettingsMM[i].value);
                    if (i < LBL_STARTING_STEP_FRAME)
                        i++;
                }
            } else {
                for(int j = 0; j < comboBoxList.size(); j++) {
                    comboBoxList[j]->setCurrentIndex(int(comboBoxList[j]->findText(blenderSettingsMM[i].value)));
                    i++;
                }
            }
        }
        for (int i = 0; i < numPaths(); i++) {
            pathLineEditList[i]->setText(blenderPaths[i].value);
        }

        connect(lineEditList[RESOLUTION_HEIGHT_EDIT],SIGNAL(textChanged(const QString &)),
                this,SLOT  (sizeChanged(const QString &)));
        connect(lineEditList[RESOLUTION_WIDTH_EDIT],SIGNAL(textChanged(const QString &)),
                this,SLOT  (sizeChanged(const QString &)));
    }
}

void BlenderPreferences::loadSettings() {
    // settings
    if (!numSettings()) {
        for (int i = 0; i < numSettings(DEFAULT_SETTINGS); i++) {
            blenderSettings[i] = {
                defaultSettings[i].key,
                defaultSettings[i].value,
                defaultSettings[i].label,
                defaultSettings[i].tooltip
            };
        }
        blenderSettings[LBL_RENDER_PERCENTAGE].value = QString::number(mRenderPercentage * 100);
        blenderSettings[LBL_DEFAULT_COLOUR].value = QString::number(gDefaultColor);
    }

    if (!numSettingsMM()) {
        for (int i = 0; i < numSettingsMM(DEFAULT_SETTINGS); i++) {
            blenderSettingsMM[i] = {
                defaultSettingsMM[i].key,
                defaultSettingsMM[i].value,
                defaultSettingsMM[i].label,
                defaultSettingsMM[i].tooltip
            };
        }
        blenderSettingsMM[LBL_RENDER_PERCENTAGE_MM].value = QString::number(mRenderPercentage * 100);
    }

    // paths
    if (!numPaths()) {
        QString defaultBlendFile = QString("%1/Blender/config/%2")
                                       .arg(Preferences::lpub3d3rdPartyConfigDir)
                                       .arg(VER_BLENDER_DEFAULT_BLEND_FILE);
        QStringList addonPaths = QStringList()
                                 /* LBL_BLENDER_PATH      */ << Preferences::blenderExe
                                 /* LBL_BLENDFILE_PATH    */ << (Preferences::defaultBlendFile ? defaultBlendFile : QString())
                                 /* LBL_ENVIRONMENT_PATH  */ << QString()
                                 /* LBL_LDCONFIG_PATH     */ << Preferences::altLDConfigPath
                                 /* LBL_LDRAW_PATH        */ << Preferences::Preferences::ldrawLibPath
                                 /* LBL_LSYNTH_PATH       */ << QString()
                                 /* LBL_STUD_LOGO_PATH    */ << QString()
                                 /* LBL_STUDIO_LDRAW_PATH */ << QString();
        for (int i = 0; i < numPaths(DEFAULT_SETTINGS); i++) {
            blenderPaths[i] = {
                defaultPaths[i].key,
                defaultPaths[i].key_mm,
                QDir::toNativeSeparators(addonPaths.at(i)),
                defaultPaths[i].label,
                defaultPaths[i].tooltip
            };
        }
    }

    QFileInfo blenderConfigFileInfo;
    if (mDocumentRender)
        blenderConfigFileInfo.setFile(Preferences::blenderPreferencesFile);
    else
        blenderConfigFileInfo.setFile(Preferences::blenderLDrawConfigFile);

    bool configFileExists = blenderConfigFileInfo.exists();

    if (mDocumentRender) {
        blenderSettings[LBL_ADD_ENVIRONMENT].value        = "0";
        blenderSettings[LBL_TRANSPARENT_BACKGROUND].value = "1";
        blenderSettings[LBL_CROP_IMAGE].value             = "1";

        blenderSettingsMM[LBL_ADD_ENVIRONMENT_MM].value        = "0";
        blenderSettingsMM[LBL_TRANSPARENT_BACKGROUND_MM].value = "1";
        blenderSettingsMM[LBL_CROP_IMAGE_MM].value             = "1";
    }

    if (configFileExists) {
        QSettings Settings(blenderConfigFileInfo.absoluteFilePath(), QSettings::IniFormat);

        for (int i = 0; i < numSettings(); i++) {
            QString key = QString("%1/%2").arg(IMPORTLDRAW, blenderSettings[i].key);
            QString value = Settings.value(key, QString()).toString();
            if (!value.isEmpty())
                blenderSettings[i].value = value == "True" ? "1" : value == "False" ? "0" : value;
        }

        for (int i = 0; i < numSettingsMM(); i++) {
            QString key = QString("%1/%2").arg(IMPORTLDRAWMM, blenderSettingsMM[i].key);
            QString value = Settings.value(key, QString()).toString();
            if (!value.isEmpty())
                blenderSettingsMM[i].value = value == "True" ? "1" : value == "False" ? "0" : value;
        }

        if (Preferences::blenderImportModule == QLatin1String("MM")) {
            for (int i = 1/*skip blender executable*/; i < numPaths(); i++) {
                QString key = QString("%1/%2").arg(IMPORTLDRAWMM, blenderPaths[i].key_mm);
                QString value = Settings.value(key, QString()).toString();
                if (QFileInfo(value).exists())
                    blenderPaths[i].value = QDir::toNativeSeparators(value);
            }
        } else {
            for (int i = 1/*skip blender executable*/; i < numPaths(); i++) {
                QString key = QString("%1/%2").arg(IMPORTLDRAW, blenderPaths[i].key);
                QString value = Settings.value(key, QString()).toString();
                if (QFileInfo(value).exists())
                    blenderPaths[i].value = QDir::toNativeSeparators(value);
            }
        }
    }

    blenderPaths[LBL_BLENDER_PATH].value = Preferences::blenderExe;
    const QStringList versionItems       = Preferences::blenderVersion.split("|");
    blenderVersion                       = versionItems.first();
    blenderAddonVersion                  = versionItems.size() == 2 ? versionItems.last() : QString();
}

void BlenderPreferences::saveSettings()
{
    if (!numSettings() || !numSettingsMM())
        loadSettings();

    QString value = blenderPaths[LBL_BLENDER_PATH].value;
    if (value.isEmpty())
        value = blenderPreferences->pathLineEditList[LBL_BLENDER_PATH]->text();
    Preferences::setBlenderExePathPreference(value);

    value.clear();
    if (!blenderVersion.isEmpty())
        value = blenderVersion;
    if (!blenderAddonVersion.isEmpty()) {
        blenderPreferences->blenderAddonModulesBox->setEnabled(true);
        blenderPreferences->blenderAddonVersionEdit->setText(blenderAddonVersion);
        value.append(QString("|%1").arg(blenderAddonVersion));
    }
    Preferences::setBlenderVersionPreference(value);

    if (mDocumentRender)
        value = Preferences::blenderPreferencesFile.isEmpty()
                    ? QString("%1/%2").arg(Preferences::blenderConfigDir).arg(VER_BLENDER_DOCUMENT_CONFIG_FILE)
                    : Preferences::blenderPreferencesFile;
    else
        value = Preferences::blenderLDrawConfigFile.isEmpty()
                    ? QString("%1/%2").arg(Preferences::blenderConfigDir).arg(VER_BLENDER_RENDER_CONFIG_FILE)
                    : Preferences::blenderLDrawConfigFile;
    Preferences::setBlenderLDrawConfigPreference(value);

    QString searchDirectoriesKey;
    QString parameterFileKey = QLatin1String("parameterFile");
    QString parameterFile = QString("%1/%2").arg(Preferences::blenderConfigDir).arg(VER_BLENDER_LDRAW_PARAMS_FILE);

    QSettings Settings(value, QSettings::IniFormat);

    auto concludeSettingsGroup = [&] () {
        if (!QFileInfo(parameterFile).exists())
            exportParameterFile();
        QString value = QDir::toNativeSeparators(Preferences::ldSearchDirs.join(","));
        Settings.setValue(searchDirectoriesKey, QVariant(value));
        Settings.endGroup();
    };

    Settings.beginGroup(IMPORTLDRAW);

    for (int i = 1/*skip blender executable*/; i < numPaths(); i++) {
        if (i == LBL_STUDIO_LDRAW_PATH)
            continue;
        value = QDir::toNativeSeparators(blenderPaths[i].value);
        Settings.setValue(QString("%1").arg(blenderPaths[i].key), QVariant(value));
    }

    for (int i = 0; i < numSettings(); i++) {
        if (i > LBL_VERBOSE && i < LBL_COLOUR_SCHEME)
            value = blenderSettings[i].value;
        else if (
            i == LBL_KEEP_ASPECT_RATIO)
            continue;
        else
            value = blenderSettings[i].value == "1"
                        ? "True"
                        : blenderSettings[i].value == "0"
                              ? "False"
                              : blenderSettings[i].value;
        Settings.setValue(QString("%1").arg(blenderSettings[i].key), QVariant(value));
    }

    Settings.setValue(parameterFileKey, QVariant(QDir::toNativeSeparators(parameterFile)));

    searchDirectoriesKey = QLatin1String("additionalSearchDirectories");

    concludeSettingsGroup();

    Settings.beginGroup(IMPORTLDRAWMM);

    for (int i = 1/*skip blender executable*/; i < numPaths(); i++) {
        if (i == LBL_LSYNTH_PATH || i == LBL_STUD_LOGO_PATH)
            continue;
        value = QDir::toNativeSeparators(blenderPaths[i].value);
        Settings.setValue(QString("%1").arg(blenderPaths[i].key_mm), QVariant(value));
    }

    for (int i = 0; i < numSettingsMM(); i++) {
        if (i > LBL_VERBOSE_MM && i < LBL_CHOSEN_LOGO)
            value = blenderSettingsMM[i].value;
        else if (
            i == LBL_KEEP_ASPECT_RATIO_MM)
            continue;
        else
            value = blenderSettingsMM[i].value == "1"
                        ? "True"
                        : blenderSettingsMM[i].value == "0"
                              ? "False"
                              : blenderSettingsMM[i].value;
        Settings.setValue(QString("%1").arg(blenderSettingsMM[i].key), QVariant(value));
    }

    searchDirectoriesKey = QLatin1String("additionalSearchPaths");

    concludeSettingsGroup();

    QString const preferredImportModule =
        blenderPreferences->blenderImportActBox->isChecked()
            ? QString("TN")
            : blenderPreferences->blenderImportMMActBox->isChecked()
                  ? QString("MM") : QString();

    Preferences::setBlenderImportModule(preferredImportModule);
}

void BlenderPreferences::showPathsGroup()
{
    if (blenderPathsBox->isHidden()){
        blenderPathsBox->show();
        pathsGroupButton->setText(tr("Hide Paths"));
        blenderContent->adjustSize();
    }
    else{
        blenderPathsBox->hide();
        pathsGroupButton->setText(tr("Show Paths"));
        blenderContent->adjustSize();
    }
}

int BlenderPreferences::numSettings(bool defaultSettings)
{
    int size = 0;
    if (!blenderSettings[0].key.isEmpty() || defaultSettings)
        size = sizeof(blenderSettings)/sizeof(blenderSettings[0]);
    return size;
}

int BlenderPreferences::numSettingsMM(bool defaultSettings)
{
    int size = 0;
    if (!blenderSettingsMM[0].key.isEmpty() || defaultSettings)
        size = sizeof(blenderSettingsMM)/sizeof(blenderSettingsMM[0]);
    return size;
}

int BlenderPreferences::numPaths(bool defaultSettings)
{
    int size = 0;
    if (!blenderPaths[0].key.isEmpty() || defaultSettings)
        size = sizeof(blenderPaths)/sizeof(blenderPaths[0]);
    return size;
}

void BlenderPreferences::colorButtonClicked(bool)
{
    int ColorIndex = lineEditList[DEFAULT_COLOUR_EDIT]->property("ColourID").toInt();

    QWidget *parent = lineEditList[DEFAULT_COLOUR_EDIT];
    lcQColorPickerPopup *popup = new lcQColorPickerPopup(parent, ColorIndex);
    connect(popup, SIGNAL(selected(int)), SLOT(setDefaultColor(int)));
    popup->setMinimumSize(300, 200);

    const QRect desktop = QApplication::desktop()->geometry();

    QPoint pos = parent->mapToGlobal(parent->rect().bottomLeft());
    if (pos.x() < desktop.left())
        pos.setX(desktop.left());
    if (pos.y() < desktop.top())
        pos.setY(desktop.top());

    if ((pos.x() + popup->width()) > desktop.width())
        pos.setX(desktop.width() - popup->width());
    if ((pos.y() + popup->height()) > desktop.bottom())
        pos.setY(desktop.bottom() - popup->height());
    popup->move(pos);

    popup->setFocus();
    popup->show();
}

void BlenderPreferences::setDefaultColor(int value) const
{
    QImage img(12, 12, QImage::Format_ARGB32);
    img.fill(0);

    lcColor* color = &gColorList[uint(value)];
    QPainter painter(&img);
    painter.setCompositionMode(QPainter::CompositionMode_Source);
    painter.setPen(Qt::darkGray);
    painter.setBrush(QColor::fromRgbF(qreal(color->Value[0]), qreal(color->Value[1]), qreal(color->Value[2])));
    painter.drawRect(0, 0, img.width() - 1, img.height() - 1);
    painter.end();

    lineEditList[DEFAULT_COLOUR_EDIT]->setText(QString("%1 (%2)").arg(color->Name).arg(value));
    lineEditList[DEFAULT_COLOUR_EDIT]->setProperty("ColourID", QVariant::fromValue(value));
    defaultColourEditAction->setIcon(QPixmap::fromImage(img));
    defaultColourEditAction->setToolTip(tr("Select Colour"));
}

void BlenderPreferences::browseBlender(bool unused)
{
    Q_UNUSED(unused)
    for(int i = 0; i < numPaths(); ++i) {
        if (sender() == pathBrowseButtonList.at(i)) {
            QString const blenderPath = QDir::toNativeSeparators(blenderPaths[i].value).toLower();
            QFileDialog dialog(nullptr);
            dialog.setWindowTitle(tr("Locate %1").arg(blenderPaths[i].label));
            if (i < LBL_LDRAW_PATH)
                dialog.setFileMode(QFileDialog::ExistingFile);
            else
                dialog.setFileMode(QFileDialog::Directory);
            if (!blenderPath.isEmpty())
                dialog.setDirectory(QFileInfo(blenderPath).absolutePath());
            if (dialog.exec()) {
                QStringList selectedPathList = dialog.selectedFiles();
                if (selectedPathList.size() == 1) {
                    QFileInfo  pathInfo(selectedPathList.at(0));
                    if (pathInfo.exists()) {
                        QString const selectedPath = QDir::toNativeSeparators(pathInfo.absoluteFilePath()).toLower();
                        pathLineEditList[i]->setText(selectedPathList.at(0));
                        if (i == LBL_BLENDER_PATH && blenderPath != selectedPath) {
                            blenderPaths[i].value = selectedPath;
                            updateLDrawAddon();
                        }
                    }
                }
            }
        }
    }
}

void BlenderPreferences::sizeChanged(const QString &value)
{
    const bool importMM = blenderImportMMActBox->isChecked();
    const int keep_aspect_ratio_index = importMM ? KEEP_ASPECT_RATIO_BOX_MM : KEEP_ASPECT_RATIO_BOX;
    const int width_edit_index = importMM ? RESOLUTION_WIDTH_EDIT : IMAGE_WIDTH_EDIT;
    const int height_edit_index = importMM ? RESOLUTION_HEIGHT_EDIT : IMAGE_HEIGHT_EDIT;

    /* original height x new width / original width = new height */
    int mNewValue = value.toInt();
    if (checkBoxList[keep_aspect_ratio_index]->isChecked())
    {
        if (sender() == lineEditList[width_edit_index])
        {
            disconnect(lineEditList[height_edit_index],SIGNAL(textChanged(const QString &)),
                       this,SLOT  (sizeChanged(const QString &)));
            lineEditList[height_edit_index]->setText(QString::number(qRound(double(mHeight * mNewValue / mWidth))));
            connect(lineEditList[height_edit_index],SIGNAL(textChanged(const QString &)),
                    this,SLOT  (sizeChanged(const QString &)));
        }
        else if (sender() == lineEditList[height_edit_index])
        {
            disconnect(lineEditList[width_edit_index],SIGNAL(textChanged(const QString &)),
                       this, SLOT  (sizeChanged(const QString &)));
            lineEditList[width_edit_index]->setText(QString::number(qRound(double(mNewValue * mWidth / mHeight))));
            connect(lineEditList[width_edit_index],SIGNAL(textChanged(const QString &)),
                    this,SLOT  (sizeChanged(const QString &)));
        }
    }
}

void BlenderPreferences::setModelSize(bool checked)
{
    const bool importMM = blenderImportMMActBox->isChecked();
    const int add_environment_index = importMM ? ADD_ENVIRONMENT_BOX_MM : ADD_ENVIRONMENT_BOX;
    const int trans_background_index = importMM ? TRANSPARENT_BACKGROUND_BOX_MM : TRANSPARENT_BACKGROUND_BOX;
    const int keep_aspect_ratio_index = importMM ? KEEP_ASPECT_RATIO_BOX_MM : KEEP_ASPECT_RATIO_BOX;
    const int width_edit_index = importMM ? RESOLUTION_WIDTH_EDIT : IMAGE_WIDTH_EDIT;
    const int height_edit_index = importMM ? RESOLUTION_HEIGHT_EDIT : IMAGE_HEIGHT_EDIT;

    if (checked) {
        bool conflict[3];
        if ((conflict[1] = checkBoxList[add_environment_index]->isChecked()))
            checkBoxList[add_environment_index]->setChecked(!checked);
        if ((conflict[2] = !checkBoxList[trans_background_index]->isChecked()))
            checkBoxList[trans_background_index]->setChecked(checked);
        if ((conflict[0] = checkBoxList[keep_aspect_ratio_index]->isChecked()))
            checkBoxList[keep_aspect_ratio_index]->setChecked(!checked);

        if (conflict[0] || conflict[1] || conflict[2]) {
            QMessageBox box;
            box.setTextFormat (Qt::RichText);
            box.setIcon (QMessageBox::Information);
            box.setStandardButtons (QMessageBox::Ok);
            box.setDefaultButton   (QMessageBox::Ok);
            box.setWindowFlags (Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
            box.setWindowTitle(tr ("LDraw Render Settings"));
            QString header = "<b>" + tr ("Crop image configuration settings conflict were resolved.") + "</b>";
            QString body = tr ("%1%2%3")
                               .arg(conflict[0] ? "Keep aspect ratio set to false.<br>" : "")
                               .arg(conflict[1] ? "Add environment (backdrop and base plane) set to false.<br>" : "")
                               .arg(conflict[2] ? "Transparent background set to true.<br>" : "");
            box.setText (header);
            box.setInformativeText (body);
            box.exec();
        }
    }

    disconnect(lineEditList[width_edit_index],SIGNAL(textChanged(const QString &)),
               this,SLOT  (sizeChanged(const QString &)));
    disconnect(lineEditList[height_edit_index],SIGNAL(textChanged(const QString &)),
               this, SLOT  (sizeChanged(const QString &)));

    if (checked)
    {
        lineEditList[width_edit_index]->setText(QString::number(gui->GetImageWidth()));
        lineEditList[height_edit_index]->setText(QString::number(gui->GetImageHeight()));
    }
    else
    {
        lineEditList[width_edit_index]->setText(QString::number(mWidth));
        lineEditList[height_edit_index]->setText(QString::number(mHeight));
    }

    connect(lineEditList[height_edit_index],SIGNAL(textChanged(const QString &)),
            this,SLOT  (sizeChanged(const QString &)));
    connect(lineEditList[width_edit_index],SIGNAL(textChanged(const QString &)),
            this,SLOT  (sizeChanged(const QString &)));
}

void BlenderPreferences::validateColourScheme(QString const &value)
{
    const bool importMM = blenderImportMMActBox->isChecked();

    if (value == "custom" &&
        blenderPaths[LBL_LDCONFIG_PATH].value.isEmpty() &&
        Preferences::altLDConfigPath.isEmpty()) {
        if (importMM)
            blenderSettingsMM[LBL_COLOUR_SCHEME_MM].value = blenderSettingsMM[LBL_COLOUR_SCHEME_MM].value;
        else
            blenderSettings[LBL_COLOUR_SCHEME].value = defaultSettings[LBL_COLOUR_SCHEME].value;
        QMessageBox box;
        box.setTextFormat (Qt::RichText);
        box.setIcon (QMessageBox::Critical);
        box.setStandardButtons (QMessageBox::Ok);
        box.setDefaultButton   (QMessageBox::Ok);
        box.setWindowFlags (Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
        box.setWindowTitle(tr ("Custom LDraw Colours"));
        QString header = "<b>" + tr ("Custom LDConfig file not found.&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"
                                    "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"
                                    "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;") + "</b>";
        QString body = tr ("Colour scheme 'custom' selected but no LDConfig file was specified.<br>"
                          "The default colour scheme '%1' will be used.")
                           .arg(importMM ? blenderSettingsMM[LBL_COLOUR_SCHEME_MM].value : blenderSettings[LBL_COLOUR_SCHEME].value);
        box.setText (header);
        box.setInformativeText (body);
        box.exec();
    }
}

bool BlenderPreferences::promptAccept()
{
    if (QMessageBox::question(nullptr,
                              tr("Render Settings Modified"),
                              tr("Do you want to accept the modified settings before quitting ?"),
                              QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel) == QMessageBox::Yes)
    {
        return true;
    }

    return false;
}

void BlenderPreferences::updateLDrawAddon()
{
    blenderAddonUpdateButton->setEnabled(false);

    QObject::disconnect(pathLineEditList[LBL_BLENDER_PATH], SIGNAL(editingFinished()), this,
                        SLOT(configureBlender()));

    mBlenderAddonUpdate = mBlenderConfigured;            // was true

    configureBlender(sender() == pathBrowseButtonList[LBL_BLENDER_PATH]);

    QObject::connect(pathLineEditList[LBL_BLENDER_PATH], SIGNAL(editingFinished()), this,
                     SLOT(configureBlender()));
}

void BlenderPreferences::configureBlender(bool testBlender)
{
    progressBar = nullptr;

    // Confirm blender exe exist
    QString blenderFile = pathLineEditList[LBL_BLENDER_PATH]->text();
    if (blenderFile.isEmpty()) {
        blenderVersion.clear();
        mBlenderConfigured = false;
        const QString colour = Preferences::displayTheme == THEME_DARK
                                   ? Preferences::themeColors[THEME_DARK_DECORATE_LPUB3D_QUOTED_TEXT]
                                   : QLatin1String("blue");
        blenderVersionLabel->setStyleSheet(QString("QLabel { color : %1; }").arg(colour));
        blenderVersionLabel->setText(tr("Blender not configured"));
        blenderVersionEdit->setVisible(mBlenderConfigured);
        blenderAddonUpdateButton->setEnabled(mBlenderConfigured);
        blenderPathsBox->setEnabled(mBlenderConfigured);
        blenderSettingsBox->setEnabled(mBlenderConfigured);
        emit gui->messageSig(LOG_INFO, tr("Blender path is empty. Quitting."));
        return;
    }

    if (QFileInfo(blenderFile).exists()) {
        // Setup
        enum ProcEnc { PR_OK, PR_FAIL, PR_WAIT, PR_INSTALL, PR_TEST };
        QString const blenderExe = QDir::toNativeSeparators(blenderFile);
        QString const blenderDir = QDir::toNativeSeparators(QString("%1/Blender").arg(Preferences::lpub3d3rdPartyConfigDir));
        QString const blenderAddonDir    = QDir::toNativeSeparators(QString("%1/addons").arg(blenderDir));
        QString const blenderSetupDir    = QDir::toNativeSeparators(QString("%1/setup").arg(blenderDir));
        QString const blenderExeCompare  = QDir::toNativeSeparators(Preferences::blenderExe).toLower();
        QString const blenderInstallFile = QDir::toNativeSeparators(QString("%1/%2").arg(blenderDir).arg(VER_BLENDER_ADDON_INSTALL_FILE));
        QString const blenderTestString  = QLatin1String("###TEST_BLENDER###");
        QString const allowModifyExternalPython = "yes";
        QByteArray addonPathsAndModuleNames;
        QString message, shellProgram;
        QStringList arguments;
        ProcEnc result = PR_OK;
        QFile script;

        bool newBlenderExe = blenderExeCompare != blenderExe.toLower();

        if (mBlenderConfigured && !mBlenderAddonUpdate && !newBlenderExe)
            return;

        // Process command
        auto processCommand = [&] (ProcEnc action) {

            process = new QProcess();

            QString processAction = tr("addon install");
            if (action == PR_INSTALL) {
                connect(process, SIGNAL(readyReadStandardOutput()), this, SLOT(readStdOut()));

                QStringList systemEnvironment = QProcess::systemEnvironment();
                systemEnvironment.prepend("LDRAW_DIRECTORY=" + Preferences::ldrawLibPath);
                systemEnvironment.prepend("ADDONS_TO_LOAD=" + addonPathsAndModuleNames);
                systemEnvironment.prepend("ALLOW_MODIFY_EXTERNAL_PYTHON=" + allowModifyExternalPython);
                process->setEnvironment(systemEnvironment);
                qDebug() << qPrintable(QString("ADDON SYSTEM ENVIRONMENT: %1").arg(systemEnvironment.join(" ")));
            } 
            else
            {
                processAction = tr("test");
                disconnect(&updateTimer, SIGNAL(timeout()), this, SLOT(update()));
            }



            process->setWorkingDirectory(blenderDir);

            process->setStandardErrorFile(QString("%1/stderr-blender-addon-install").arg(blenderDir));

            if (action == PR_INSTALL) {
                process->start(blenderExe, arguments);
            } else {
#ifdef Q_OS_WIN
                process->start(shellProgram, QStringList() << "/C" << script.fileName());
#else
                process->start(shellProgram, QStringList() << script.fileName());
#endif
            }

            if (!process->waitForStarted()) {
                message = tr("Cannot start Blender %1 process.\n%2")
                              .arg(processAction)
                              .arg(QString(process->readAllStandardError()));
                delete process; // Close process
                process = nullptr;
                return PR_WAIT;
            } else {
                if (process->exitStatus() != QProcess::NormalExit || process->exitCode() != 0) {
                    message = tr("Failed to execute Blender %1.\n%2")
                                 .arg(processAction)
                                 .arg(QString(process->readAllStandardError()));
                    return PR_FAIL;
                } else {
                    message = tr("Blender %1 process [%2] running...").arg(processAction).arg(process->processId());
                }
            }

            if (action == PR_TEST) {
                while (process && process->state() != QProcess::NotRunning) {
                    QTime waiting = QTime::currentTime().addMSecs(500);
                    while (QTime::currentTime() < waiting)
                        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
                }
                connect(&updateTimer, SIGNAL(timeout()), this, SLOT(update()));
                QString const stdOut = QString(process->readAllStandardOutput());
                if (!stdOut.contains(blenderTestString)) {
                    message =  tr("A simple check to test if the selected file is Blender failed."
                                  "Please create an LPub3D GitHub ticket if you are sure the file is Blender 2.8 or newer."
                                  "The ticket should contain the full path to the Blender executable.");
                    return PR_FAIL;
                } else {
                    // Get Blender Version
                    QStringList items = stdOut.split('\n',Qt::SkipEmptyParts).last().split(" ");
                    if (items.count() > 6 && items.at(0) == QLatin1String("Blender")) {
                        items.takeLast();
                        blenderVersion.clear();
                        for (int i = 1; i < items.size(); i++)
                            blenderVersion.append(items.at(i)+" ");
                        blenderVersion = blenderVersion.trimmed().append(")");
                        blenderVersionEdit->setText(blenderVersion);
                        versionFound = true;
                        //emit gui->messageSig(LOG_DEBUG, tr("Blender version: %1").arg(blenderVersion));
                    }
                }
            }

            return PR_OK;
        };

        if (!blenderImportMMActBox->isChecked() && !blenderImportActBox->isChecked()) {

            QString const blenderAddonFile = QString("%1/%2").arg(blenderDir).arg(VER_BLENDER_ADDON_FILE);

            bool const addonExists = QFileInfo(blenderAddonFile).exists() && QFileInfo(blenderAddonFile).size();

            QPixmap _icon = QPixmap(":/icons/lpub96.png");
            QMessageBox box;
            box.setWindowIcon(QIcon());
            box.setIconPixmap (_icon);
            box.setTextFormat (Qt::RichText);
            box.setWindowTitle(tr ("%1 Blender Addon Modules").arg(VER_PRODUCTNAME_STR));
            box.setWindowFlags (Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
            QString title = tr ("No import module enabled.<br><br>%1")
                                .arg(addonExists
                                         ? tr("The default import module (LDraw Import TN) will be used.")
                                         : tr("You will not be able to import models."));
            box.setText (title);
            QString text  = tr ("Do you want to continue ?");
            box.setInformativeText (text);
            box.setStandardButtons (QMessageBox::Yes | QMessageBox::Cancel);
            box.setDefaultButton   (QMessageBox::Yes);

            if (box.exec() == QMessageBox::Cancel)
                return;

            if (addonExists)
                blenderImportActBox->setChecked(true);
        }

        connect(&updateTimer, SIGNAL(timeout()), this, SLOT(update()));
        updateTimer.start(500);

        progressBar = new QProgressBar(blenderContent);
        progressBar->setMaximum(0);
        progressBar->setMinimum(0);
        progressBar->setValue(1);

        // Test Blender executable
        testBlender |= sender() == pathLineEditList[LBL_BLENDER_PATH];
        if (testBlender) {
            arguments << QString("--factory-startup");
            arguments << QString("-b");
            arguments << QString("--python-expr");
            arguments << QString("\"import sys;print('%1');sys.stdout.flush();sys.exit()\"").arg(blenderTestString);

            QStringList configPathList = QStandardPaths::standardLocations(QStandardPaths::TempLocation);
            QString const scriptDir = configPathList.first();

            bool error = false;

            if (QFileInfo(scriptDir).exists()) {
                QString scriptName, scriptCommand;

#ifdef Q_OS_WIN
                scriptName =  QLatin1String("blender_test.bat");
#else
                scriptName =  QLatin1String("blender_test.sh");
#endif
                scriptCommand = QString("%1 %2").arg(blenderExe).arg(arguments.join(" "));

                message = tr("Blender Test Command: %1").arg(scriptCommand);
#ifdef QT_DEBUG_MODE
                qDebug() << qPrintable(message);
#else
                emit gui->messageSig(LOG_INFO, message);
#endif
                script.setFileName(QString("%1/%2").arg(scriptDir).arg(scriptName));
                if(script.open(QIODevice::WriteOnly | QIODevice::Text)) {
                    QTextStream stream(&script);
#ifdef Q_OS_WIN
                    stream << QLatin1String("@ECHO OFF &SETLOCAL") << lpub_endl;
#else
                    stream << QLatin1String("#!/bin/bash") << lpub_endl;
#endif
                    stream << scriptCommand << lpub_endl;
                    script.close();
                    message = tr("Blender Test Script: %2").arg(QDir::toNativeSeparators(script.fileName()));
#ifdef QT_DEBUG_MODE
                    qDebug() << qPrintable(message);
#else
                    emit gui->messageSig(LOG_INFO, message);
#endif
                } else {
                    message = tr("Cannot write Blender render script file [%1] %2.")
                                  .arg(script.fileName())
                                  .arg(script.errorString());
                    error = true;
                }
            } else {
                message = tr("Cannot create Blender render script temp path.");
                error = true;
            }

            if (error) {
                emit gui->messageSig(LOG_ERROR, message);
                statusUpdate();
                return;
            }

            QThread::sleep(1);

#ifdef Q_OS_WIN
            shellProgram = QLatin1String("cmd.exe");
#else
            shellProgram = QLatin1String("/bin/sh");
#endif
            if (processCommand(PR_TEST) == PR_FAIL) {
                emit gui->messageSig(LOG_ERROR, message);
                statusUpdate();
                return;
            }
        } // Test Blender

        QString const preferredImportModule =
            blenderImportActBox->isChecked()
                ? QString("TN")
                : blenderImportMMActBox->isChecked()
                      ? QString("MM") : QString();
        Preferences::setBlenderImportModule(preferredImportModule);

        blenderVersionEdit->setVisible(mBlenderAddonUpdate); // was false
        blenderAddonVersionEdit->clear();

        // Download and extract blender addon
        if (!extractBlenderAddon(blenderDir)) {
            if (mBlenderAddonUpdate) {
                blenderExeGridLayout->replaceWidget(progressBar, blenderVersionEdit);
                mBlenderConfigured = true;
                mBlenderAddonUpdate = !mBlenderConfigured;
                blenderVersionLabel->setText(tr("Blender Version"));
                blenderVersionLabel->setStyleSheet(QString("QLabel { color : %1; }").arg(Preferences::displayTheme == THEME_DARK ? QLatin1String("white") : QLatin1String("black")));
                blenderVersionEdit->setText(blenderVersion);
                blenderVersionEdit->setToolTip(tr("Display the Blender and %1 Render addon version").arg(VER_PRODUCTNAME_STR));
                blenderVersionEdit->setVisible(mBlenderConfigured);
                if (!blenderAddonVersion.isEmpty()) {
                    blenderAddonModulesBox->setEnabled(true);
                    blenderAddonVersionEdit->setText(blenderAddonVersion);
                }
                blenderAddonUpdateButton->setEnabled(mBlenderConfigured);
                if (progressBar)
                    progressBar->close();
            } else {
                statusUpdate();
            }
            return;
        }

        // Main installation script
        if (!QFileInfo(blenderInstallFile).exists()) {
            gui->messageSig(LOG_ERROR, tr("Could not find addon install file: %1").arg(blenderInstallFile));
            statusUpdate();
            return;
        }

        // Create Blender config directory
        QDir configDir(Preferences::blenderConfigDir);
        if(!QDir(configDir).exists())
            configDir.mkpath(".");

        // Save Blender settings
        saveSettings();

        // Install Blender addon

        arguments.clear();
        arguments << QString("--background");
        arguments << QString("--python");
        arguments << blenderInstallFile;
        arguments << "--";
        if (!blenderRenderActBox->isChecked() &&
            !blenderImportActBox->isChecked() &&
            !blenderImportMMActBox->isChecked())
            arguments << QString("--disable_ldraw_addons");
        else if (!blenderImportActBox->isChecked())
            arguments << QString("--disable_ldraw_import");
        else if (!blenderImportMMActBox->isChecked())
            arguments << QString("--disable_ldraw_import_mm");
        else if (!blenderRenderActBox->isChecked())
            arguments << QString("--disable_ldraw_render");

        message = tr("Blender Addon Install Arguments: %1 %2").arg(blenderExe).arg(arguments.join(" "));
        emit gui->messageSig(LOG_INFO, message);

        if (!testBlender)
            versionFound = false;

        // Check if there are addon folders in /addons
        if (QDir(blenderAddonDir).entryInfoList(QDir::Dirs|QDir::NoSymLinks).count() > 0) {
            //A. Create a QJsonDocument
            QJsonDocument jsonDoc;
            //B. Create jsonArray
            QJsonArray jsonArray;
            // 1. get list of addons
            QStringList addonDirs = QDir(blenderAddonDir).entryList(QDir::NoDotAndDotDot | QDir::Dirs, QDir::SortByMask);
            // 2. search each addon folder for addon file __init__.py
            Q_FOREACH (QString const &addon, addonDirs) {
                // First, check if there are files in the addon
                QDir dir(QString("%1/%2").arg(blenderAddonDir).arg(addon));
                dir.setFilter(QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks);
                QFileInfoList list = dir.entryInfoList();
                for (int i = 0; i < list.size(); i++) {
                    // Second, read the file to get the module name
                    if (list.at(i).fileName() == QLatin1String("__init__.py")) {
                        QFile file(QFileInfo(list.at(i)).absoluteFilePath());
                        if (!file.open(QFile::ReadOnly | QFile::Text)) {
                            emit gui->messageSig(LOG_ERROR, tr("Cannot read addon file %1<br>%2")
                                                               .arg(list.at(i).fileName())
                                                               .arg(file.errorString()));
                            break;
                        } else {
                            // Third, append the path and module list to addonPathsAndModuleNamesList
                            bool foundModule = false;
                            QTextStream in(&file);
                            while ( ! in.atEnd()) {
                                if (QString(in.readLine(0)).startsWith("bl_info")) {
                                    foundModule = true;
                                    break;
                                }
                            }
                            file.close();
                            if (foundModule) {
                                //C. Create Item of Json Object content (object of object)
                                QJsonObject jsonItemObj;
                                jsonItemObj["load_dir"] = QDir::toNativeSeparators(dir.absolutePath());
                                jsonItemObj["module_name"] = dir.dirName();
                                //D. Add jsonItemObj to jsonArray
                                jsonArray.append(jsonItemObj);
                            }
                        }
                    }
                }
            }
            //E. Add jsonArray to jsonDocument
            jsonDoc.setArray(jsonArray);
            //F. Create a QByteArray and fill it with QJsonDocument (json compact format)
            addonPathsAndModuleNames = jsonDoc.toJson(QJsonDocument::Compact);
        }

        result = processCommand(PR_INSTALL);

        if (result == PR_WAIT) {
            emit gui->messageSig(LOG_ERROR, message);
            statusUpdate();
        } else {
            if (result == PR_FAIL) {
                emit gui->messageSig(LOG_ERROR, message);
            } else {
                emit gui->messageSig(LOG_INFO, message);
                statusUpdate(true, tr("Installing Blender addon..."));
            }
        }

    } else {
        emit gui->messageSig(LOG_ERROR, tr("Blender executable not found at [%1]").arg(blenderFile), true);
    }
}

bool BlenderPreferences::extractBlenderAddon(const QString &blenderDir)
{
    QDir dir(blenderDir);
    if (!dir.exists())
        dir.mkdir(blenderDir);

    // Get Blender addon
    QString const blenderAddonFile = QDir::toNativeSeparators(QString("%1/%2").arg(blenderDir).arg(VER_BLENDER_ADDON_FILE));


    BlenderAddOnUpdate AddOnUpdate = static_cast<BlenderAddOnUpdate>(getBlenderAddon(blenderDir));

    bool const addonExists = QFileInfo(blenderAddonFile).exists() && QFileInfo(blenderAddonFile).size();

    if ((AddOnUpdate == ADD_ON_FAIL && !addonExists) || AddOnUpdate == ADD_ON_CANCEL)
        return false;

    // Extract Blender addon
    if (addonExists) {
        QStringList result = JlCompress::extractDir(blenderAddonFile, blenderDir);
        if (result.isEmpty()){
            emit gui->messageSig(LOG_ERROR, tr("Failed to extract %1 to %2")
                                                .arg(blenderAddonFile).arg(blenderDir));
            return false;
        } else {
            emit gui->messageSig(LOG_INFO, tr("%1 items archive extracted to %2")
                                               .arg(result.size()).arg(blenderDir));
        }
    } else {
        emit gui->messageSig(LOG_ERROR, tr("Blender addon archive %1 was not found")
                                            .arg(blenderAddonFile));
        return false;
    }

    return true;
}

int BlenderPreferences::getBlenderAddon(const QString &blenderDir)
{
    QString const blenderAddonDir    = QDir::toNativeSeparators(QString("%1/addons").arg(blenderDir));
    QString const blenderAddonFile   = QDir::toNativeSeparators(QString("%1/%2").arg(blenderDir).arg(VER_BLENDER_ADDON_FILE));
    BlenderAddOnUpdate AddOnUpdate   = ADD_ON_DOWNLOAD;

    if (QFileInfo(blenderAddonFile).exists()) {
        if (Preferences::modeGUI) {
            QPixmap _icon = QPixmap(":/icons/lpub96.png");
            if (_icon.isNull())
                _icon = QPixmap (":/icons/update.png");
            QMessageBox box;
            box.setWindowIcon(QIcon());
            box.setIconPixmap (_icon);
            box.setTextFormat (Qt::RichText);
            box.setWindowTitle(tr ("%1 Blender LDraw Addon").arg(VER_PRODUCTNAME_STR));
            box.setWindowFlags (Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
            QString title = tr ("An existing %1 Blender LDraw addon archive file was detected.").arg(VER_PRODUCTNAME_STR);
            box.setText (title);
            QString text  = tr ("Do you want to download the latest addon archive file ?");
            box.setInformativeText (text);
            box.setStandardButtons (QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
            box.setDefaultButton   (QMessageBox::Yes);
            int execReturn = box.exec();
            if (execReturn == QMessageBox::Cancel) {
                blenderPreferences->mDialogCancelled = true;
                blenderPreferences->statusUpdate(true, tr("Cancelled"));
                return static_cast<int>(ADD_ON_CANCEL);
            } else if (execReturn == QMessageBox::No) {
                blenderPreferences->statusUpdate(true, tr("Installing Blender addon..."));
                AddOnUpdate = ADD_ON_RELOAD;
            } else {
                blenderPreferences->statusUpdate(true, tr("Downloading Blender addon..."));
            }
        }
    }

    if (blenderPreferences->progressBar) {
        blenderPreferences->blenderExeGridLayout->replaceWidget(blenderPreferences->blenderVersionEdit, blenderPreferences->progressBar);
        blenderPreferences->progressBar->show();
    }

    // Remove old addon archive if exist
    if (QFileInfo(blenderAddonDir).exists()) {
        QDir dir(blenderAddonDir);
        dir.setNameFilters(QStringList() << "*.*");
        dir.setFilter(QDir::Files);

        for (const QString &dirFile : dir.entryList()) {
            dir.remove(dirFile);
            emit gui->messageSig(LOG_INFO, tr("Removed Blender addon item: [%1]").arg(dirFile));
        }

        if (!dir.rmdir("."))
            emit gui->messageSig(LOG_NOTICE, tr("Failed to remove Blender folder: %1")
                                                 .arg(blenderAddonDir));
    }

    // Download Blender addon
    if (AddOnUpdate == ADD_ON_DOWNLOAD) {
        lpub->downloadFile(VER_BLENDER_RENDER_ADDONS_URL, tr("Blender Addon"),false/*promptRedirect*/,false/*showProgress*/);
        QByteArray Buffer = lpub->getDownloadedFile();
        if (!Buffer.isEmpty()) {
            if (QFileInfo(blenderAddonFile).exists()) {
                QDir dir(blenderDir);
                if (!dir.remove(blenderAddonFile))
                    emit gui->messageSig(LOG_NOTICE, tr("Failed to remove Blender archive: %1")
                                                         .arg(blenderAddonFile));
            }
            QFile file(blenderAddonFile);
            if (! file.open(QIODevice::WriteOnly)) {
                emit gui->messageSig(LOG_ERROR, tr("Failed to open Blender file: %1:<br>%2")
                                                    .arg(blenderAddonFile)
                                                    .arg(file.errorString()));
                if (blenderPreferences->progressBar)
                    blenderPreferences->progressBar->close();
                return static_cast<int>(ADD_ON_FAIL);
            }
            file.write(Buffer);
            file.close();
        }
    }

    return static_cast<int>(AddOnUpdate);
}

void BlenderPreferences::getStandardOutput()
{
    QString const logFile = QString("%1/Blender/stdout-blender-addon-install").arg(Preferences::lpub3d3rdPartyConfigDir);
    QFileInfo fileInfo(logFile);
    if (!fileInfo.exists()) {
        emit gui->messageSig(LOG_ERROR, tr("Blender Standard output file not found: %1.")
                                            .arg(fileInfo.absoluteFilePath()));
        return;
    }
    gui->viewStandardOutput(fileInfo.absoluteFilePath());
}

void BlenderPreferences::statusUpdate(bool ok, const QString &message)
{
    QString label, colour;
    if (ok){
        label  = ! message.isEmpty() ? message : tr("Installing Blender addon... ");
        colour = "black";
    } else {
        if (progressBar) {
            blenderExeGridLayout->replaceWidget(progressBar, blenderVersionEdit);
            progressBar->close();
        }
        pathLineEditList[LBL_BLENDER_PATH]->text() = QString();
        Preferences::setBlenderImportModule(QString());
        label  = ! message.isEmpty() ? message : tr("Blender not configured");
        colour = message.startsWith("Error:", Qt::CaseInsensitive)
                     ? QLatin1String("red")
                     : Preferences::displayTheme == THEME_DARK
                           ? Preferences::themeColors[THEME_DARK_DECORATE_LPUB3D_QUOTED_TEXT]
                           : QLatin1String("blue");
        mDialogCancelled = true;
    }
    blenderVersionLabel->setText(label);
    blenderVersionLabel->setStyleSheet(QString("QLabel { color : %1; }").arg(colour));
    blenderVersionEdit->setVisible(mBlenderConfigured);
}

void BlenderPreferences::update()
{
#ifndef QT_NO_PROCESS
    if (!process)
        return;

    if (process->state() == QProcess::NotRunning)
    {
        emit gui->messageSig(LOG_INFO, tr("Addon install finished"));
        showResult();
    }
#endif
    QApplication::processEvents();
}

void BlenderPreferences::readStdOut()
{
    QString StdOut = QString(process->readAllStandardOutput());

    stdOutList.append(StdOut);

    QRegExp rxInfo("^INFO: ");
    QRegExp rxData("^DATA: ");
    QRegExp rxError("^(?:\\w)*ERROR: ", Qt::CaseInsensitive);
    QRegExp rxWarning("^(?:\\w)*WARNING: ", Qt::CaseInsensitive);
    QRegExp rxAddonVersion("^ADDON VERSION: ", Qt::CaseInsensitive);

    bool errorEncountered = false;
    QStringList items,messages;
    QStringList stdOutLines = StdOut.split(QRegExp("\n|\r\n|\r"));

    int lineCount = 0;
    for (QString const &stdOutLine : stdOutLines) {
        if (stdOutLine.isEmpty())
            continue;

        if (!versionFound) {
            // Get Blender Version
            items = stdOutLine.split(" ");
            if (items.count() > 6 && items.at(0) == QLatin1String("Blender")) {
                items.takeLast();
                blenderVersion.clear();
                for (int i = 1; i < items.size(); i++)
                    blenderVersion.append(items.at(i)+" ");
                blenderVersion = blenderVersion.trimmed().append(")");
                blenderVersionEdit->setText(blenderVersion);
                versionFound = true;
                //emit gui->messageSig(LOG_DEBUG, tr("Blender version: %1").arg(blenderVersion));
            }
        }

        if (stdOutLine.contains(rxInfo)) {
            items = stdOutLine.split(": ");
            statusUpdate(true, items.last());
        } else if (stdOutLine.contains(rxData)) {
            items = stdOutLine.split(": ");
            if (items.at(1) == "ENVIRONMENT_FILE") {
                blenderPaths[LBL_ENVIRONMENT_PATH].value = items.at(2);
                pathLineEditList[LBL_ENVIRONMENT_PATH]->setText(items.at(2));
            } else if (items.at(1) == "LSYNTH_DIRECTORY") {
                blenderPaths[LBL_LSYNTH_PATH].value = items.at(2);
                pathLineEditList[LBL_LSYNTH_PATH]->setText(items.at(2));
            } else if (items.at(1) == "STUDLOGO_DIRECTORY") {
                blenderPaths[LBL_STUD_LOGO_PATH].value = items.at(2);
                pathLineEditList[LBL_STUD_LOGO_PATH]->setText(items.at(2));
            }
        } else if (stdOutLine.contains(rxError) || stdOutLine.contains(rxWarning)) {
            auto cleanLine = [&] () {
                return stdOutLine.trimmed()
                           .replace("<","&lt;")
                           .replace(">","&gt;")
                           .replace("&","&amp;") + "\n";
            };
            errorEncountered = stdOutLine.contains(rxError);
            messages << cleanLine();
            int errorCount = lineCount;
            for (;errorCount < stdOutLines.size(); errorCount++) {
                if (stdOutLine.at(0) == "")
                    messages << cleanLine();
                else
                    break;
            }
        } else if (stdOutLine.contains(rxAddonVersion)) {
            // Get Addon version
            items = stdOutLine.split(":");
            blenderAddonVersion = tr("v%1").arg(items.at(1).trimmed()); // 1 addon version
            blenderAddonVersionEdit->setText(blenderAddonVersion);
        }
        lineCount++;
    }
    if (messages.size()) {
        mBlenderConfigured = false;
        blenderVersionEdit->clear();
        QString const stdOutLog = QDir::toNativeSeparators(QString("\n\n- See %1/Blender/stdout-blender-addon-install")
                                                                .arg(Preferences::lpub3d3rdPartyConfigDir));
        emit gui->messageSig(LOG_BLENDER_ADDON, messages.join(" ").append(stdOutLog), errorEncountered);
    }
}

QString BlenderPreferences::readStdErr(bool &hasError) const
{
    hasError = false;
    QStringList returnLines;
    QString const blenderDir = QString("%1/Blender").arg(Preferences::lpub3d3rdPartyConfigDir);
    QFile file(QString("%1/stderr-blender-addon-install").arg(blenderDir));
    if ( ! file.open(QFile::ReadOnly | QFile::Text))
    {
        QString message = tr("Failed to open log file: %1:\n%2")
                              .arg(file.fileName())
                              .arg(file.errorString());
        return message;
    }
    QTextStream in(&file);
    while ( ! in.atEnd())
    {
        QString line = in.readLine(0);
        returnLines << line.trimmed().replace("<","&lt;")
                               .replace(">","&gt;")
                               .replace("&","&amp;") + "<br>";
        if (!hasError)
            hasError = !line.isEmpty();
    }
    return returnLines.join(" ");
}

void BlenderPreferences::writeStdOut()
{
    QString const blenderDir = QString("%1/Blender").arg(Preferences::lpub3d3rdPartyConfigDir);
    QFile file(QString("%1/stdout-blender-addon-install").arg(blenderDir));
    if (file.open(QFile::WriteOnly | QIODevice::Truncate | QFile::Text))
    {
        QTextStream Out(&file);
        for (const QString& Line : stdOutList)
            Out << Line;
        file.close();
    }
    else
    {
        emit gui->messageSig(LOG_NOTICE, tr("Error writing to %1 file '%2':\n%3")
                                             .arg("stdout").arg(file.fileName(), file.errorString()));
    }
}

bool BlenderPreferences::promptCancel()
{
#ifndef QT_NO_PROCESS
    if (process) {
        if (QMessageBox::question(nullptr,
                                  tr("Cancel Addon Install"),
                                  tr("Are you sure you want to cancel the add on install?"),
                                  QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
        {
            process->kill();

            // Close process
            delete process;
            process = nullptr;

        }
        else
            return false;
    }
#endif
    mDialogCancelled = true;
    return true;
}

void BlenderPreferences::reject()
{
    if (promptCancel())
        if (sender() != dialog)
            dialog->reject();
}

void BlenderPreferences::showResult()
{
    QString message;
    bool hasError;
    const QString StdErrLog = readStdErr(hasError);

    progressBar->close();

    writeStdOut();

    if (process->exitStatus() != QProcess::NormalExit || process->exitCode() != 0 || hasError)
    {
        QString const blenderDir = QString("%1/Blender").arg(Preferences::lpub3d3rdPartyConfigDir);
        message = tr("Addon install failed. See %1/stderr-blender-addon-install for details.").arg(blenderDir);
        statusUpdate(false, tr("%1: Addon install failed.").arg("Error"));
        mBlenderConfigured = false;
        blenderVersionEdit->clear();
        emit gui->messageSig(LOG_BLENDER_ADDON, StdErrLog, true);
    } else {
        blenderExeGridLayout->replaceWidget(progressBar, blenderVersionEdit);
        mBlenderConfigured = true;
        mBlenderAddonUpdate = !mBlenderConfigured;
        blenderVersionLabel->setText(tr("Blender Version"));
        blenderVersionLabel->setStyleSheet(QString("QLabel { color : %1; }").arg(Preferences::displayTheme == THEME_DARK ? QLatin1String("white") : QLatin1String("black")));
        blenderVersionEdit->setText(blenderVersion);
        blenderVersionEdit->setToolTip(tr("Display the Blender and %1 Render addon version").arg(VER_PRODUCTNAME_STR));
        blenderVersionEdit->setVisible(mBlenderConfigured);
        if (!blenderAddonVersion.isEmpty()) {
            blenderAddonModulesBox->setEnabled(true);
            blenderAddonVersionEdit->setText(blenderAddonVersion);
            Preferences::setBlenderVersionPreference(
                QString("%1|%2").arg(blenderVersion).arg(blenderAddonVersion));
        }
        blenderAddonUpdateButton->setEnabled(mBlenderConfigured);
        blenderPathsBox->setEnabled(mBlenderConfigured);
        blenderSettingsBox->setEnabled(mBlenderConfigured);
        message = tr("Blender version %1").arg(blenderVersion);
    }

    // Restore update action
    blenderAddonUpdateButton->setEnabled(true);
    blenderAddonStdOutButton->setEnabled(true);

    // Close process
    delete process;
    process = nullptr;

    emit gui->messageSig(hasError ? LOG_NOTICE : LOG_INFO, message);
}

void BlenderPreferences::loadDefaultParameters(QByteArray& Buffer, int Which)
{

    /*
    # File: BlenderLDrawParameters.lst
    #
    # This config file captures parameters for the Blender LDraw Render addon
    # Parameters must be prefixed using one of the following predefined
    # 'Item' labels:
    # - lgeo_colour
    # - sloped_brick
    # - light_brick

    # LGEO CUSTOM COLOURS
    # LGEO is a parts library for rendering LEGO using the POV-Ray
    # rendering software. This is the list of LEGO colours suitable
    # for realistic rendering extracted from the LGEO file 'lg_color.inc'.
    # When the 'Colour Scheme' option is set to 'Realistic', the standard
    # LDraw colours RGB value is overwritten with the values defined here.
    # Note: You can customize these RGB values as you want.

    #   Item-------  ID--    R--   G--   B--
    */
    const char DefaultCustomColours[] = {
        "lgeo_colour,   0,    33,   33,   33\n"
        "lgeo_colour,   1,    13,  105,  171\n"
        "lgeo_colour,   2,    40,  127,   70\n"
        "lgeo_colour,   3,     0,  143,  155\n"
        "lgeo_colour,   4,   196,   40,   27\n"
        "lgeo_colour,   5,   205,   98,  152\n"
        "lgeo_colour,   6,    98,   71,   50\n"
        "lgeo_colour,   7,   161,  165,  162\n"
        "lgeo_colour,   8,   109,  110,  108\n"
        "lgeo_colour,   9,   180,  210,  227\n"
        "lgeo_colour,   10,   75,  151,   74\n"
        "lgeo_colour,   11,   85,  165,  175\n"
        "lgeo_colour,   12,  242,  112,   94\n"
        "lgeo_colour,   13,  252,  151,  172\n"
        "lgeo_colour,   14,  245,  205,   47\n"
        "lgeo_colour,   15,  242,  243,  242\n"
        "lgeo_colour,   17,  194,  218,  184\n"
        "lgeo_colour,   18,  249,  233,  153\n"
        "lgeo_colour,   19,  215,  197,  153\n"
        "lgeo_colour,   20,  193,  202,  222\n"
        "lgeo_colour,   21,  224,  255,  176\n"
        "lgeo_colour,   22,  107,   50,  123\n"
        "lgeo_colour,   23,   35,   71,  139\n"
        "lgeo_colour,   25,  218,  133,   64\n"
        "lgeo_colour,   26,  146,   57,  120\n"
        "lgeo_colour,   27,  164,  189,   70\n"
        "lgeo_colour,   28,  149,  138,  115\n"
        "lgeo_colour,   29,  228,  173,  200\n"
        "lgeo_colour,   30,  172,  120,  186\n"
        "lgeo_colour,   31,  225,  213,  237\n"
        "lgeo_colour,   32,    0,   20,   20\n"
        "lgeo_colour,   33,  123,  182,  232\n"
        "lgeo_colour,   34,  132,  182,  141\n"
        "lgeo_colour,   35,  217,  228,  167\n"
        "lgeo_colour,   36,  205,   84,   75\n"
        "lgeo_colour,   37,  228,  173,  200\n"
        "lgeo_colour,   38,  255,   43,    0\n"
        "lgeo_colour,   40,  166,  145,  130\n"
        "lgeo_colour,   41,  170,  229,  255\n"
        "lgeo_colour,   42,  198,  255,    0\n"
        "lgeo_colour,   43,  193,  223,  240\n"
        "lgeo_colour,   44,  150,  112,  159\n"
        "lgeo_colour,   46,  247,  241,  141\n"
        "lgeo_colour,   47,  252,  252,  252\n"
        "lgeo_colour,   52,  156,  149,  199\n"
        "lgeo_colour,   54,  255,  246,  123\n"
        "lgeo_colour,   57,  226,  176,   96\n"
        "lgeo_colour,   65,  236,  201,   53\n"
        "lgeo_colour,   66,  202,  176,    0\n"
        "lgeo_colour,   67,  255,  255,  255\n"
        "lgeo_colour,   68,  243,  207,  155\n"
        "lgeo_colour,   69,  142,   66,  133\n"
        "lgeo_colour,   70,  105,   64,   39\n"
        "lgeo_colour,   71,  163,  162,  164\n"
        "lgeo_colour,   72,   99,   95,   97\n"
        "lgeo_colour,   73,  110,  153,  201\n"
        "lgeo_colour,   74,  161,  196,  139\n"
        "lgeo_colour,   77,  220,  144,  149\n"
        "lgeo_colour,   78,  246,  215,  179\n"
        "lgeo_colour,   79,  255,  255,  255\n"
        "lgeo_colour,   80,  140,  140,  140\n"
        "lgeo_colour,   82,  219,  172,   52\n"
        "lgeo_colour,   84,  170,  125,   85\n"
        "lgeo_colour,   85,   52,   43,  117\n"
        "lgeo_colour,   86,  124,   92,   69\n"
        "lgeo_colour,   89,  155,  178,  239\n"
        "lgeo_colour,   92,  204,  142,  104\n"
        "lgeo_colour,  100,  238,  196,  182\n"
        "lgeo_colour,  115,  199,  210,   60\n"
        "lgeo_colour,  134,  174,  122,   89\n"
        "lgeo_colour,  135,  171,  173,  172\n"
        "lgeo_colour,  137,  106,  122,  150\n"
        "lgeo_colour,  142,  220,  188,  129\n"
        "lgeo_colour,  148,   62,   60,   57\n"
        "lgeo_colour,  151,   14,   94,   77\n"
        "lgeo_colour,  179,  160,  160,  160\n"
        "lgeo_colour,  183,  242,  243,  242\n"
        "lgeo_colour,  191,  248,  187,   61\n"
        "lgeo_colour,  212,  159,  195,  233\n"
        "lgeo_colour,  216,  143,   76,   42\n"
        "lgeo_colour,  226,  253,  234,  140\n"
        "lgeo_colour,  232,  125,  187,  221\n"
        "lgeo_colour,  256,   33,   33,   33\n"
        "lgeo_colour,  272,   32,   58,   86\n"
        "lgeo_colour,  273,   13,  105,  171\n"
        "lgeo_colour,  288,   39,   70,   44\n"
        "lgeo_colour,  294,  189,  198,  173\n"
        "lgeo_colour,  297,  170,  127,   46\n"
        "lgeo_colour,  308,   53,   33,    0\n"
        "lgeo_colour,  313,  171,  217,  255\n"
        "lgeo_colour,  320,  123,   46,   47\n"
        "lgeo_colour,  321,   70,  155,  195\n"
        "lgeo_colour,  322,  104,  195,  226\n"
        "lgeo_colour,  323,  211,  242,  234\n"
        "lgeo_colour,  324,  196,    0,   38\n"
        "lgeo_colour,  326,  226,  249,  154\n"
        "lgeo_colour,  330,  119,  119,   78\n"
        "lgeo_colour,  334,  187,  165,   61\n"
        "lgeo_colour,  335,  149,  121,  118\n"
        "lgeo_colour,  366,  209,  131,    4\n"
        "lgeo_colour,  373,  135,  124,  144\n"
        "lgeo_colour,  375,  193,  194,  193\n"
        "lgeo_colour,  378,  120,  144,  129\n"
        "lgeo_colour,  379,   94,  116,  140\n"
        "lgeo_colour,  383,  224,  224,  224\n"
        "lgeo_colour,  406,    0,   29,  104\n"
        "lgeo_colour,  449,  129,    0,  123\n"
        "lgeo_colour,  450,  203,  132,   66\n"
        "lgeo_colour,  462,  226,  155,   63\n"
        "lgeo_colour,  484,  160,   95,   52\n"
        "lgeo_colour,  490,  215,  240,    0\n"
        "lgeo_colour,  493,  101,  103,   97\n"
        "lgeo_colour,  494,  208,  208,  208\n"
        "lgeo_colour,  496,  163,  162,  164\n"
        "lgeo_colour,  503,  199,  193,  183\n"
        "lgeo_colour,  504,  137,  135,  136\n"
        "lgeo_colour,  511,  250,  250,  250\n"
    };

    /*
    # SLOPED BRICKS
    # Dictionary with part number (without any extension for decorations), as key,
    # of pieces that have grainy slopes, and, as values, a set containing the angles (in
    # degrees) of the face's normal to the horizontal plane. Use a | delimited tuple to
    # represent a range within which the angle must lie.

    #   Item--------  PartID-  Angle/Angle Range (in degrees)
    */
    const char DefaultSlopedBricks[] = {
        "sloped_brick,     962,  45\n"
        "sloped_brick,    2341, -45\n"
        "sloped_brick,    2449, -16\n"
        "sloped_brick,    2875,  45\n"
        "sloped_brick,    2876,  40|63\n"
        "sloped_brick,    3037,  45\n"
        "sloped_brick,    3038,  45\n"
        "sloped_brick,    3039,  45\n"
        "sloped_brick,    3040,  45\n"
        "sloped_brick,    3041,  45\n"
        "sloped_brick,    3042,  45\n"
        "sloped_brick,    3043,  45\n"
        "sloped_brick,    3044,  45\n"
        "sloped_brick,    3045,  45\n"
        "sloped_brick,    3046,  45\n"
        "sloped_brick,    3048,  45\n"
        "sloped_brick,    3049,  45\n"
        "sloped_brick,    3135,  45\n"
        "sloped_brick,    3297,  63\n"
        "sloped_brick,    3298,  63\n"
        "sloped_brick,    3299,  63\n"
        "sloped_brick,    3300,  63\n"
        "sloped_brick,    3660, -45\n"
        "sloped_brick,    3665, -45\n"
        "sloped_brick,    3675,  63\n"
        "sloped_brick,    3676, -45\n"
        "sloped_brick,   3678b,  24\n"
        "sloped_brick,    3684,  15\n"
        "sloped_brick,    3685,  16\n"
        "sloped_brick,    3688,  15\n"
        "sloped_brick,    3747, -63\n"
        "sloped_brick,    4089, -63\n"
        "sloped_brick,    4161,  63\n"
        "sloped_brick,    4286,  63\n"
        "sloped_brick,    4287, -63\n"
        "sloped_brick,    4445,  45\n"
        "sloped_brick,    4460,  16\n"
        "sloped_brick,    4509,  63\n"
        "sloped_brick,    4854, -45\n"
        "sloped_brick,    4856, -60|-70, -45\n"
        "sloped_brick,    4857,  45\n"
        "sloped_brick,    4858,  72\n"
        "sloped_brick,    4861,  45,      63\n"
        "sloped_brick,    4871, -45\n"
        "sloped_brick,    4885,  72\n"
        "sloped_brick,    6069,  72,      45\n"
        "sloped_brick,    6153,  60|70,   26|4\n"
        "sloped_brick,    6227,  45\n"
        "sloped_brick,    6270,  45\n"
        "sloped_brick,   13269,  40|63\n"
        "sloped_brick,   13548,  45\n"
        "sloped_brick,   15571,  45\n"
        "sloped_brick,   18759, -45\n"
        "sloped_brick,   22390,  40|55\n"
        "sloped_brick,   22391,  40|55\n"
        "sloped_brick,   22889, -45\n"
        "sloped_brick,   28192,  45\n"
        "sloped_brick,   30180,  47\n"
        "sloped_brick,   30182,  45\n"
        "sloped_brick,   30183, -45\n"
        "sloped_brick,   30249,  35\n"
        "sloped_brick,   30283, -45\n"
        "sloped_brick,   30363,  72\n"
        "sloped_brick,   30373, -24\n"
        "sloped_brick,   30382,  11,      45\n"
        "sloped_brick,   30390, -45\n"
        "sloped_brick,   30499,  16\n"
        "sloped_brick,   32083,  45\n"
        "sloped_brick,   43708,  72\n"
        "sloped_brick,   43710,  72,      45\n"
        "sloped_brick,   43711,  72,      45\n"
        "sloped_brick,   47759,  40|63\n"
        "sloped_brick,   52501, -45\n"
        "sloped_brick,   60219, -45\n"
        "sloped_brick,   60477,  72\n"
        "sloped_brick,   60481,  24\n"
        "sloped_brick,   63341,  45\n"
        "sloped_brick,   72454, -45\n"
        "sloped_brick,   92946,  45\n"
        "sloped_brick,   93348,  72\n"
        "sloped_brick,   95188,  65\n"
        "sloped_brick,   99301,  63\n"
        "sloped_brick,  303923,  45\n"
        "sloped_brick,  303926,  45\n"
        "sloped_brick,  304826,  45\n"
        "sloped_brick,  329826,  64\n"
        "sloped_brick,  374726, -64\n"
        "sloped_brick,  428621,  64\n"
        "sloped_brick, 4162628,  17\n"
        "sloped_brick, 4195004,  45\n"
    };

    /*
    # LIGHTED BRICKS
    # Dictionary with part number (with extension), as key,
    # of lighted bricks, light emission colour and intensity, as values.

    #    Item---------  PartID---  Light--------------  Intensity
    */
    const char DefaultLightedBricks[] = {
        "lighted_brick, 62930.dat, 1.000, 0.373, 0.059, 1.0\n"
        "lighted_brick, 54869.dat, 1.000, 0.052, 0.017, 1.0\n"
    };

    Buffer.clear();
    if (Which == PARAMS_CUSTOM_COLOURS)
        Buffer.append(DefaultCustomColours, sizeof(DefaultCustomColours));
    else if (Which == PARAMS_SLOPED_BRICKS)
        Buffer.append(DefaultSlopedBricks, sizeof(DefaultSlopedBricks));
    else if (Which == PARAMS_LIGHTED_BRICKS)
        Buffer.append(DefaultLightedBricks, sizeof(DefaultLightedBricks));
}

bool BlenderPreferences::exportParameterFile(){

    QString const parameterFile = QString("%1/%2").arg(Preferences::blenderConfigDir).arg(VER_BLENDER_LDRAW_PARAMS_FILE);
    QFile file(parameterFile);

    if (!overwriteFile(file.fileName()))
        return true;

    QString message;
    if(file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        int counter = 1;
        QByteArray Buffer;
        QTextStream outstream(&file);
        outstream << "# File: " << QFileInfo(parameterFile).fileName() << lpub_endl;
        outstream << "" << lpub_endl;
        outstream << "# This config file captures parameters for the Blender LDraw Render addon" << lpub_endl;
        outstream << "# Parameters must be prefixed using one of the following predefined" << lpub_endl;
        outstream << "# 'Item' labels:" << lpub_endl;
        outstream << "# - lgeo_colour" << lpub_endl;
        outstream << "# - sloped_brick" << lpub_endl;
        outstream << "# - light_brick" << lpub_endl;
        outstream << "" << lpub_endl;
        outstream << "# All items must use a comma',' delimiter as the primary delimiter." << lpub_endl;
        outstream << "# For sloped_brick items, a pipe'|' delimiter must be used to specify" << lpub_endl;
        outstream << "# a range (min|max) within which the angle must lie when appropriate." << lpub_endl;
        outstream << "# Spaces between item attributes are not required and are used to" << lpub_endl;
        outstream << "# facilitate human readability." << lpub_endl;
        outstream << "" << lpub_endl;
        outstream << "" << lpub_endl;
        outstream << "# LGEO CUSTOM COLOURS" << lpub_endl;
        outstream << "# LGEO is a parts library for rendering LEGO using the POV-Ray" << lpub_endl;
        outstream << "# rendering software. This is the list of LEGO colours suitable" << lpub_endl;
        outstream << "# for realistic rendering extracted from the LGEO file 'lg_color.inc'." << lpub_endl;
        outstream << "# When the 'Colour Scheme' option is set to 'Realistic', the standard" << lpub_endl;
        outstream << "# LDraw colours RGB value is overwritten with the values defined here." << lpub_endl;
        outstream << "# Note: You can customize these RGB values as you want." << lpub_endl;
        outstream << "" << lpub_endl;
        outstream << "# Item-----  ID-    R--   G--   B--" << lpub_endl;

        loadDefaultParameters(Buffer, PARAMS_CUSTOM_COLOURS);
        QTextStream colourstream(Buffer);
        for (QString sLine = colourstream.readLine(); !sLine.isNull(); sLine = colourstream.readLine())
        {
            outstream << sLine << lpub_endl;
            counter++;
        }

        outstream << "" << lpub_endl;
        outstream << "# SLOPED BRICKS" << lpub_endl;
        outstream << "# Dictionary with part number (without any extension for decorations), as key," << lpub_endl;
        outstream << "# of pieces that have grainy slopes, and, as values, a set containing the angles (in" << lpub_endl;
        outstream << "# degrees) of the face's normal to the horizontal plane. Use a | delimited tuple to" << lpub_endl;
        outstream << "# represent a range within which the angle must lie." << lpub_endl;
        outstream << "" << lpub_endl;
        outstream << "# Item------  PartID-  Angle/Angle Range (in degrees)" << lpub_endl;

        loadDefaultParameters(Buffer, PARAMS_SLOPED_BRICKS);
        QTextStream slopedstream(Buffer);
        for (QString sLine = slopedstream.readLine(); !sLine.isNull(); sLine = slopedstream.readLine())
        {
            outstream << sLine << lpub_endl;
            counter++;
        }

        outstream << "" << lpub_endl;
        outstream << "# LIGHTED BRICKS" << lpub_endl;
        outstream << "# Dictionary with part number (with extension), as key," << lpub_endl;
        outstream << "# of lighted bricks, light emission colour and intensity, as values." << lpub_endl;
        outstream << "" << lpub_endl;
        outstream << "# Item-------  PartID---  Light--------------  Intensity" << lpub_endl;

        loadDefaultParameters(Buffer, PARAMS_LIGHTED_BRICKS);
        QTextStream lightedstream(Buffer);
        for (QString sLine = lightedstream.readLine(); !sLine.isNull(); sLine = lightedstream.readLine())
        {
            outstream << sLine << lpub_endl;
            counter++;
        }

        outstream << "" << lpub_endl;
        outstream << "# end of parameters" << lpub_endl;

        file.close();
        message = tr("Finished writing Blender parameter entries. Processed %1 lines in file [%2].")
                      .arg(counter)
                      .arg(file.fileName());
        emit gui->messageSig(LOG_INFO, message);
    }
    else
    {
        message = tr("Failed to open Blender parameter file: %1:<br>%2")
                      .arg(file.fileName())
                      .arg(file.errorString());
        emit gui->messageSig(LOG_ERROR, message);
        return false;
    }
    return true;
}

bool BlenderPreferences::overwriteFile(const QString &file)
{
    QFileInfo fileInfo(file);

    if (!fileInfo.exists())
        return true;

    // Get the application icon as a pixmap
    QPixmap _icon = QPixmap(":/icons/lpub96.png");
    if (_icon.isNull())
        _icon = QPixmap (":/icons/update.png");

    QMessageBox box;
    box.setWindowIcon(QIcon());
    box.setIconPixmap (_icon);
    box.setTextFormat (Qt::RichText);
    box.setWindowFlags (Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
    QString title = "<b>" + QMessageBox::tr ("Export %1").arg(fileInfo.fileName()) + "</b>";
    QString text = QMessageBox::tr ("\"%1\"<br>This file already exists.<br>Replace existing file?").arg(fileInfo.fileName());
    box.setText (title);
    box.setInformativeText (text);
    box.setStandardButtons (QMessageBox::Cancel | QMessageBox::Yes);
    box.setDefaultButton   (QMessageBox::Yes);

    return (box.exec() == QMessageBox::Yes);
}
