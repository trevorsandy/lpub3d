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
#include <QVBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QLineEdit>
#include <QCheckBox>
#include <QPushButton>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QLabel>
#include <QProgressBar>
#include <QProcess>
#include <QTimer>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

#include <JlCompress.h>

#include "blenderpreferences.h"
#include "lpub_preferences.h"
#include "lpub.h"
#include "version.h"
#include "commonmenus.h"

#include "lc_colors.h"
#include "lc_colorpicker.h"

#include "parmswindow.h"

BlenderPreferences::BlenderPaths  BlenderPreferences::mBlenderPaths [NUM_PATHS];
BlenderPreferences::BlenderPaths  BlenderPreferences::mDefaultPaths [NUM_PATHS] =
{
    /*                                 Key:                  MM Key:               Value:             Label:                                   Tooltip (Description):*/
    /* 0   PATH_BLENDER        */ {"blenderpath",        "blenderpath",        "",    QObject::tr("Blender Path"),             QObject::tr("Full file path to Blender application executable")},
    /* 1   PATH_BLENDFILE      */ {"blendfile",          "blendfile",          "",    QObject::tr("Blendfile Path"),           QObject::tr("Full file path to a supplement .blend file - specify to append additional settings")},
    /* 2.  PATH_ENVIRONMENT    */ {"environmentfile",    "environmentfile",    "",    QObject::tr("Environment Texture Path"), QObject::tr("Full file path to .exr environment texture file - specify if not using default bundled in addon")},
    /* 3   PATH_LDCONFIG       */ {"customldconfigfile", "customldconfigfile", "",    QObject::tr("Custom LDConfig Path"),     QObject::tr("Full file path to custom LDConfig file - specify if not %1 alternate LDConfig file").arg(VER_PRODUCTNAME_STR)},
    /* 4   PATH_LDRAW          */ {"ldrawdirectory",     "ldrawpath",          "",    QObject::tr("LDraw Directory"),          QObject::tr("Full directory path to the LDraw parts library (download from https://library.ldraw.org)")},
    /* 5   PATH_LSYNTH         */ {"lsynthdirectory",    "",                   "",    QObject::tr("LSynth Directory"),         QObject::tr("Full directory path to LSynth primitives - specify if not using default bundled in addon")},
    /* 6   PATH_STUD_LOGO      */ {"studlogodirectory",  "",                   "",    QObject::tr("Stud Logo Directory"),      QObject::tr("Full directory path to stud logo primitives - if stud logo enabled, specify if unofficial parts not used or not using default bundled in addon")},
    /* 7   PATH_STUDIO_LDRAW   */ {"",                   "studioldrawpath",    "",    QObject::tr("Stud.io LDraw Path"),       QObject::tr("Full filepath to the Stud.io LDraw Parts Library (download from https://www.bricklink.com/v3/studio/download.page)")}
};

BlenderPreferences::BlenderSettings  BlenderPreferences::mBlenderSettings [NUM_SETTINGS];
BlenderPreferences::BlenderSettings  BlenderPreferences::mDefaultSettings [NUM_SETTINGS] =
{
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
    /*28/2 LBL_DEFAULT_COLOUR        */ {"defaultcolour",                  "16",       QObject::tr("Default Colour"),         QObject::tr("Sets the default part colour using LDraw colour code")},
    /*29/3 LBL_GAPS_SIZE             */ {"gapwidth",                       "0.01",     QObject::tr("Gap Width"),              QObject::tr("Amount of gap space between each part")},
    /*20/4 LBL_IMAGE_WIDTH           */ {"resolutionwidth",                "800",      QObject::tr("Image Width"),            QObject::tr("Sets the rendered image width in pixels - from current step image, label shows config setting.")},
    /*31/5 LBL_IMAGE_HEIGHT          */ {"resolutionheight",               "600",      QObject::tr("Image Height"),           QObject::tr("Sets the rendered image height in pixels - from current step image, label shows config setting.")},
    /*32/6 LBL_IMAGE_SCALE           */ {"scale",                          "0.02",     QObject::tr("Image Scale"),            QObject::tr("Sets the imported model scale (between .01 and 1.0). Scale is 1.0 and is huge and unwieldy in the viewport")},
    /*33/6 LBL_RENDER_PERCENTAGE     */ {"renderpercentage",               "100",      QObject::tr("Render Percentage"),      QObject::tr("Sets the rendered image percentage scale for its pixel resolution - updated from current step, label shows config setting.")},


    /*34/0 LBL_COLOUR_SCHEME         */ {"usecolourscheme",                "lgeo",     QObject::tr("Colour Scheme"),          QObject::tr("Colour scheme options - Realistic (lgeo), Original (LDConfig), Alternate (LDCfgalt), Custom (User Defined)")},
    /*35/1 LBL_FLEX_PARTS_SOURCE     */ {"uselsynthparts",                 "1",        QObject::tr("Flex Parts Source"),      QObject::tr("Source used to create flexible parts - string, hoses etc. (LDCad, LSynth or both")},
    /*36/2 LBL_LOGO_STUD_VERSION     */ {"logostudversion",                "4",        QObject::tr("Logo Version"),           QObject::tr("Which version of the logo to use ('3' (flat), '4' (rounded) or '5' (subtle rounded))")},
    /*27/3 LBL_LOOK                  */ {"uselook",                        "normal",   QObject::tr("Look"),                   QObject::tr("Photo-realistic or Schematic 'Instruction' look")},
    /*38/4 LBL_POSITION_OBJECT       */ {"positionobjectongroundatorigin", "1",        QObject::tr("Position Object"),        QObject::tr("The object is centred at the origin, and on the ground plane")},
    /*39/5 LBL_RESOLUTION            */ {"resolution",                     "Standard", QObject::tr("Resolution"),             QObject::tr("Resolution of part primitives, ie. how much geometry they have")},
    /*40/6 LBL_RESOLVE_NORMALS       */ {"resolvenormals",                 "guess",    QObject::tr("Resolve Normals"),        QObject::tr("Some older LDraw parts have faces with ambiguous normals, this specifies what do do with them")}
};

BlenderPreferences::ComboItems  BlenderPreferences::mComboItems [NUM_COMBO_ITEMS] =
{
    /*    FIRST item set as default        Data                                  Item:   */
    /* 00 LBL_COLOUR_SCHEME            */ {"lgeo|ldraw|alt|custom",  QObject::tr("Realistic Colours|Original LDraw Colours|Alternate LDraw Colours|Custom Colours")},
    /* 01 LBL_FLEX_PARTS_SOURCE (t/f)  */ {"1|0|1",                  QObject::tr("LSynth|LDCad|LDCad and LSynth")},
    /* 02 LBL_LOGO_STUD_VERSION        */ {"4|3|5",                  QObject::tr("Rounded(4)|Flattened(3)|Subtle Rounded(5)")},
    /* 03 LBL_LOOK                     */ {"normal|instructions",    QObject::tr("Photo Realistic|Lego Instructions")},
    /* 04 LBL_POSITION_OBJECT (t/f)    */ {"1|0",                    QObject::tr("Centered At Origin On Ground|Centered At Origin")},
    /* 05 LBL_RESOLUTION               */ {"Standard|High|Low",      QObject::tr("Standard Primitives|High Resolution Primitives|Low Resolution Primitives")},
    /* 06 LBL_RESOLVE_NORMALS          */ {"guess|double",           QObject::tr("Recalculate Normals|Two Faces Back To Back")}
};

BlenderPreferences::BlenderSettings  BlenderPreferences::mBlenderSettingsMM [NUM_SETTINGS_MM];
BlenderPreferences::BlenderSettings  BlenderPreferences::mDefaultSettingsMM [NUM_SETTINGS_MM] =
{
    /*                                                Key:                             Value:                    Label                                    Tooltip (Description)*/
    /* 00 LBL_ADD_ENVIRONMENT_MM                  */ {"addenvironment",                "1",          QObject::tr("Add Environment"),          QObject::tr("Adds a ground plane and environment texture")},
    /* 01 LBL_BEVEL_EDGES_MM                      */ {"beveledges",                    "0",          QObject::tr("Bevel Edgest"),             QObject::tr("Bevel edges. Can cause some parts to render incorrectly")},
    /* 02 LBL_BLEND_FILE_TRUSTED_MM               */ {"blendfiletrusted",              "0",          QObject::tr("Trusted Blend File"),       QObject::tr("Specify whether to treat the .blend file as being loaded from a trusted source")},
    /* 03 LBL_CROP_IMAGE_MM                       */ {"cropimage",                     "0",          QObject::tr("Crop Image"),               QObject::tr("Crop the image border at opaque content. Requires transparent background set to True")},
    /* 04 LBL_DISPLAY_LOGO                        */ {"displaylogo",                   "0",          QObject::tr("Display Logo"),             QObject::tr("Display the logo on the stud")},
    /* 05 LBL_IMPORT_CAMERAS_MM                   */ {"importcameras",                 "1",          QObject::tr("Import Cameras"),           QObject::tr("%1 can specify camera definitions within the ldraw data. Choose to load them or ignore them.").arg(VER_PRODUCTNAME_STR)},
    /* 06 LBL_IMPORT_EDGES                        */ {"importedges",                   "0",          QObject::tr("Import Edges"),             QObject::tr("Import LDraw edges as edges")},
    /* 07 LBL_IMPORT_LIGHTS_MM                    */ {"importlights",                  "0",          QObject::tr("Import Lights"),            QObject::tr("%1 can specify point and sunlight definitions within the ldraw data. Choose to load them or ignore them.").arg(VER_PRODUCTNAME_STR)},
    /* 08 LBL_KEEP_ASPECT_RATIO_MM                */ {"keepaspectratio",               "1",          QObject::tr("Keep Aspect Ratio"),        QObject::tr("Maintain the aspect ratio when resizing the output image - this attribute is not passed to Blender")},
    /* 09 LBL_MAKE_GAPS                           */ {"makegaps",                      "1",          QObject::tr("Make Gaps"),                QObject::tr("Make small gaps between bricks. A small gap is more realistic")},
    /* 10 LBL_META_BFC                            */ {"metabfc",                       "1",          QObject::tr("BFC"),                      QObject::tr("Process LDraw Back Face Culling meta commands")},
    /* 11 LBL_META_CLEAR                          */ {"metaclear",                     "0",          QObject::tr("CLEAR Command"),            QObject::tr("Hides all parts in the timeline up to where this command is encountered")},
    /* 12 LBL_META_GROUP                          */ {"metagroup",                     "1",          QObject::tr("GROUP Command"),            QObject::tr("Process GROUP meta commands")},
    /* 13 LBL_META_PAUSE                          */ {"metapause",                     "0",          QObject::tr("PAUSE Command"),            QObject::tr("Not implemented")},
    /* 14 LBL_META_PRINT_WRITE                    */ {"metaprintwrite",                "0",          QObject::tr("PRINT/WRITE Command"),      QObject::tr("Prints PRINT/WRITE META commands to the system console.")},
    /* 15 LBL_META_SAVE                           */ {"metasave",                      "0",          QObject::tr("SAVE Command"),             QObject::tr("Not implemented")},
    /* 16 LBL_META_STEP                           */ {"metastep",                      "0",          QObject::tr("STEP Command"),             QObject::tr("Adds a keyframe that shows the part at the moment in the timeline")},
    /* 17 LBL_META_STEP_GROUPS                    */ {"metastepgroups",                "0",          QObject::tr("STEP Groups"),              QObject::tr("Create collection for individual steps")},
    /* 18 LBL_META_TEXMAP                         */ {"metatexmap",                    "1",          QObject::tr("TEXMAP and DATA Command"),  QObject::tr("Process TEXMAP and DATA meta commands")},
    /* 19 LBL_NO_STUDS                            */ {"nostuds",                       "0",          QObject::tr("No Studs"),                 QObject::tr("Don't import studs")},
    /* 20 LBL_OVERWRITE_IMAGE_MM                  */ {"overwriteimage",                "1",          QObject::tr("Overwrite Image"),          QObject::tr("Specify whether to overwrite an existing rendered image file")},
    /* 21 LBL_POSITION_CAMERA_MM                  */ {"positioncamera",                "1",          QObject::tr("Position Camera"),          QObject::tr("Position the camera to show the whole model")},
    /* 22 LBL_PARENT_TO_EMPTY                     */ {"parenttoempty",                 "1",          QObject::tr("Parent To Empty"),          QObject::tr("Parent the model to an empty")},
    /* 23 LBL_PREFER_STUDIO                       */ {"preferstudio",                  "0",          QObject::tr("Prefer Stud.io Library"),   QObject::tr("Search for parts in Stud.io library first")},
    /* 24 LBL_PREFER_UNOFFICIAL                   */ {"preferunofficial",              "0",          QObject::tr("Prefer Unofficial Parts"),  QObject::tr("Search for unofficial parts first")},
    /* 25 LBL_PRESERVE_HIERARCHY                  */ {"preservehierarchy",             "0",          QObject::tr("Preserve File Structure"),  QObject::tr("Don't merge the constituent subparts and primitives into the top level part. Some parts may not render properly")},
    /* 26 LBL_PROFILE                             */ {"profile",                       "0",          QObject::tr("Profile"),                  QObject::tr("Profile import performance")},
    /* 27 LBL_RECALCULATE_NORMALS                 */ {"recalculatenormals",            "0",          QObject::tr("Recalculate Normals"),      QObject::tr("Recalculate normals. Not recommended if BFC processing is active")},
    /* 28 LBL_REMOVE_DOUBLES_MM                   */ {"removedoubles",                 "1",          QObject::tr("No Duplicate Vertices"),    QObject::tr("Merge vertices that are within a certain distance.")},
    /* 29 LBL_RENDER_WINDOW_MM                    */ {"renderwindow",                  "1",          QObject::tr("Display Render Window"),    QObject::tr("Specify whether to display the render window during Blender user interface image file render")},
    /* 30 LBL_SEARCH_ADDL_PATHS_MM                */ {"searchadditionalpaths",         "0",          QObject::tr("Search Additional Paths"),  QObject::tr("Specify whether to search additional LDraw paths")},
    /* 31 LBL_SETEND_FRAME                        */ {"setendframe",                   "1",          QObject::tr("Set Step End Frame"),       QObject::tr("Set the end frame to the last step")},
    /* 32 LBL_SET_TIMELINE_MARKERS                */ {"settimelinemarkers",            "0",          QObject::tr("Set Timeline Markers"),     QObject::tr("Set timeline markers for meta commands")},
    /* 33 LBL_SHADE_SMOOTH                        */ {"shadesmooth",                   "1",          QObject::tr("Shade Smooth"),             QObject::tr("Use flat or smooth shading for part faces")},
    /* 34 LBL_TRANSPARENT_BACKGROUND_MM           */ {"transparentbackground",         "0",          QObject::tr("Transparent Background"),   QObject::tr("Specify whether to render a background")},
    /* 35 LBL_TREAT_MODELS_WITH_SUBPARTS_AS_PARTS */ {"treatmodelswithsubpartsasparts","1",          QObject::tr("Treat Subparts As Parts"),  QObject::tr("Treat subpart like a part by merging its constituent parts into one object")},
    /* 36 LBL_TREAT_SHORTCUT_AS_MODEL             */ {"treatshortcutasmodel",          "0",          QObject::tr("Treat Shortcuts As Models"),QObject::tr("Split shortcut parts into their constituent pieces as if they were models")},
    /* 37 LBL_TRIANGULATE                         */ {"triangulate",                   "0",          QObject::tr("Triangulate Faces"),        QObject::tr("Triangulate all faces")},
    /* 38 LBL_USE_ARCHIVE_LIBRARY_MM              */ {"usearchivelibrary",             "0",          QObject::tr("Use Archive Libraries"),    QObject::tr("Add any archive (zip) libraries in the LDraw file path to the library search list")},
    /* 39 LBL_USE_FREESTYLE_EDGES                 */ {"usefreestyleedges",             "0",          QObject::tr("Use Freestyle Edges"),      QObject::tr("Render LDraw edges using freestyle")},
    /* 40 LBL_VERBOSE_MM                          */ {"verbose",                       "1",          QObject::tr("Verbose output"),           QObject::tr("Output all messages while working, else only show warnings and errors")},

    /* 41/00 LBL_BEVEL_SEGMENTS                   */ {"bevelsegments",                 "4",          QObject::tr("Bevel Segments"),           QObject::tr("Bevel segments")},
    /* 42/01 LBL_BEVEL_WEIGHT                     */ {"bevelweight",                   "0.3",        QObject::tr("Bevel Weight"),             QObject::tr("Bevel weight")},
    /* 43/02 LBL_BEVEL_WIDTH_MM                   */ {"bevelwidth",                    "0.3",        QObject::tr("Bevel Width"),              QObject::tr("Width of the bevelled edges")},
    /* 44/03 LBL_CAMERA_BORDER_PERCENT_MM         */ {"cameraborderpercent",           "5",          QObject::tr("Camera Border Percent"),    QObject::tr("When positioning the camera, include a (percentage) border around the model in the render")},
    /* 45/04 LBL_FRAMES_PER_STEP                  */ {"framesperstep",                 "3",          QObject::tr("Frames Per Step"),          QObject::tr("Frames per step")},
    /* 46/05 LBL_GAP_SCALE                        */ {"gapscale",                      "0.997",      QObject::tr("Gap Scale"),                QObject::tr("Scale individual parts by this much to create the gap")},
    /* 47/06 LBL_IMPORT_SCALE                     */ {"importscale",                   "0.02",       QObject::tr("Import Scale"),             QObject::tr("What scale to import at. Full scale is 1.0 and is so huge that it is unwieldy in the viewport")},
    /* 48/07 LBL_MERGE_DISTANCE                   */ {"mergedistance",                 "0.05",       QObject::tr("Merge Distance"),           QObject::tr("Maximum distance between elements to merge")},
    /* 49/08 LBL_RENDER_PERCENTAGE_MM             */ {"renderpercentage",              "100",        QObject::tr("Render Percentage"),        QObject::tr("Sets the rendered image percentage scale for its pixel resolution - updated from current step, label shows config setting.")},
    /* 50/09 LBL_RESOLUTION_WIDTH                 */ {"resolutionwidth",               "800",        QObject::tr("Image Width"),              QObject::tr("Sets the rendered image width in pixels - from current step image, label shows config setting.")},
    /* 51/10 LBL_RESOLUTION_HEIGHT                */ {"resolutionheight",              "600",        QObject::tr("Image Height"),             QObject::tr("Sets the rendered image height in pixels - from current step image, label shows config setting.")},
    /* 52/11 LBL_STARTING_STEP_FRAME              */ {"startingstepframe",             "1",          QObject::tr("Starting Step Frame"),      QObject::tr("Frame to add the first STEP meta command")},

    /* 53/00 LBL_CHOSEN_LOGO                      */ {"chosenlogo",                    "logo3",      QObject::tr("Chosen Logo"),              QObject::tr("Which logo to display. logo and logo2 aren't used and are only included for completeness")},
    /* 54/01 LBL_COLOUR_SCHEME_MM                 */ {"usecolourscheme",               "lgeo",       QObject::tr("Colour Scheme"),            QObject::tr("Colour scheme options - Realistic (lgeo), Original (LDConfig), Alternate (LDCfgalt), Custom (User Defined)")},
    /* 55/02 LBL_GAP_SCALE_STRATEGY               */ {"gapscalestrategy",              "constraint", QObject::tr("Gap Strategy"),             QObject::tr("Apply gap to object directly or scale and empty to adjust to gaps between parts")},
    /* 56/03 LBL_GAP_TARGET                       */ {"gaptarget",                     "object",     QObject::tr("Gap Target"),               QObject::tr("Whether to scale the object data or mesh data")},
    /* 57/04 LBL_RESOLUTION_MM                    */ {"resolution",                    "Standard",   QObject::tr("Resolution"),               QObject::tr("Resolution of part primitives, ie. how much geometry they have")},
    /* 58/05 LBL_SMOOTH_TYPE                      */ {"smoothtype",                    "edge_split", QObject::tr("Smooth Type"),              QObject::tr("Use either autosmooth or an edge split modifier to smooth part faces")}
};

BlenderPreferences::ComboItems  BlenderPreferences::mComboItemsMM [NUM_COMBO_ITEMS_MM] =
{
    /*    FIRST item set as default        Data                                  Item: */
    /* 00 LBL_CHOSEN_LOGO              */ {"logo|logo2|logo3",                   QObject::tr("Logo (Not Used)|Logo2 (Not Used)|Flattened(3)")},
    /* 01 LBL_COLOUR_SCHEME_MM         */ {"lgeo|ldraw|alt|custom",              QObject::tr("Realistic Colours|Original LDraw Colours|Alternate LDraw Colours|Custom Colours")},
    /* 02 LBL_GAP_SCALE_STRATEGY       */ {"object|constraint",                  QObject::tr("Gap applied directly to object|Gap scaled to adjust to gaps between parts")},
    /* 03 LBL_GAP_TARGET               */ {"object|mesh",                        QObject::tr("Scale object data|Scale mesh data")},
    /* 04 LBL_RESOLUTION_MM            */ {"Low|Standard|High",                  QObject::tr("Low Resolution Primitives|Standard Primitives|High Resolution Primitives")},
    /* 05 LBL_SMOOTH_TYPE              */ {"edge_split|auto_smooth|bmesh_split", QObject::tr("Smooth part faces with edge split modifier|Auto-smooth part faces|Split during initial mesh processing")}
};

BlenderPreferences *gAddonPreferences;

BlenderPreferencesDialog::BlenderPreferencesDialog(
    int      width,
    int      height,
    double   renderPercentage,
    bool     docRender,
    QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Blender LDraw Addon Settings"));

    QVBoxLayout *layout = new QVBoxLayout(this);
    setLayout(layout);

    QGroupBox *box = new QGroupBox(this);
    layout->addWidget(box);

    mPreferences = new BlenderPreferences(width,height,renderPercentage,docRender,box);

    QDialogButtonBox *buttonBox;
    buttonBox = new QDialogButtonBox(this);
    mApplyButton = new QPushButton(tr("Apply"), buttonBox);
    mApplyButton->setToolTip(tr("Apply addon paths and settings preferences"));
    mApplyButton->setEnabled(false);
    buttonBox->addButton(mApplyButton, QDialogButtonBox::ActionRole);
    connect(mApplyButton,SIGNAL(clicked()),this,SLOT(accept()));

    mPathsButton = new QPushButton(tr("Hide Paths"), buttonBox);
    mPathsButton->setToolTip(tr("Hide addon path preferences dialog"));
    buttonBox->addButton(mPathsButton,QDialogButtonBox::ActionRole);
    connect(mPathsButton,SIGNAL(clicked()),this,SLOT(showPathsGroup()));

    mResetButton = new QPushButton(tr("Reset"), buttonBox);
    mResetButton->setEnabled(false);
    mResetButton->setToolTip(tr("Reset addon paths and settings preferences"));
    buttonBox->addButton(mResetButton,QDialogButtonBox::ActionRole);
    connect(mResetButton,SIGNAL(clicked()),this,SLOT(resetSettings()));

    buttonBox->addButton(QDialogButtonBox::Cancel);
    connect(buttonBox,SIGNAL(rejected()),this,SLOT(reject()));

    if (!QFileInfo(Preferences::blenderLDrawConfigFile).isReadable() &&
        !Preferences::blenderImportModule.isEmpty())
        mApplyButton->setEnabled(true);

    connect(mPreferences,SIGNAL(settingChangedSig(bool)),this,SLOT(enableButton(bool)));

    layout->addWidget(buttonBox);

    setMinimumSize(box->sizeHint().width() + 50, 500);

    setSizeGripEnabled(true);

    setModal(true);
}

BlenderPreferencesDialog::~BlenderPreferencesDialog()
{
}

bool BlenderPreferencesDialog::getBlenderPreferences(
    int     &width,
    int     &height,
    double  &renderPercentage,
    bool     docRender,
    QWidget *parent)
{
    BlenderPreferencesDialog *dialog = new BlenderPreferencesDialog(width,height,renderPercentage,docRender,parent);

    bool ok = dialog->exec() == QDialog::Accepted;

    if (ok) {
        dialog->mPreferences->apply(ok);
        width            = dialog->mPreferences->mImageWidth;
        height           = dialog->mPreferences->mImageHeight;
        renderPercentage = dialog->mPreferences->mRenderPercentage;
    }

    return ok;
}

void BlenderPreferencesDialog::showPathsGroup()
{
    QString const display = mPathsButton->text().startsWith("Hide") ? tr("Show") : tr("Hide");
    mPathsButton->setText(tr("%1 Paths").arg(display));
    mPathsButton->setToolTip(tr("%1 addon path preferences dialog").arg(display));
    mPreferences->showPathsGroup();
}

void BlenderPreferencesDialog::enableButton(bool change)
{
    mApplyButton->setEnabled(change);
    mResetButton->setEnabled(change);
    mPathsButton->setText(tr("Hide Paths"));
    mPathsButton->setToolTip(tr("Hide addon path preferences dialog"));
}

void BlenderPreferencesDialog::resetSettings()
{
    mPreferences->resetSettings();
    mApplyButton->setEnabled(false);
}

void BlenderPreferencesDialog::accept()
{
    if (mPreferences->settingsModified()) {
        mApplyButton->setEnabled(false);
        mPreferences->saveSettings();
        QDialog::accept();
    } else
        QDialog::reject();
}

void BlenderPreferencesDialog::reject()
{
    if (mPreferences->promptCancel())
        QDialog::reject();
}

BlenderPreferences::BlenderPreferences(
    int      width,
    int      height,
    double   renderPercentage,
    bool     docRender,
    QWidget *parent)
    : QWidget(parent)
{
    gAddonPreferences = this;

#ifndef QT_NO_PROCESS
    mProcess = nullptr;
#endif

    mImageWidth  = width;
    mImageHeight = height;
    mRenderPercentage = renderPercentage;
    mDocumentRender = docRender;

    mDialogCancelled = false;

    QVBoxLayout *layout = new QVBoxLayout(parent);

    QString windowTitle = tr("Blender LDraw Addon Settings");
    if (parent) {
        if (!parent->windowTitle().isEmpty())
            windowTitle = parent->windowTitle();
        parent->setLayout(layout);
        parent->setWhatsThis(lpubWT(WT_DIALOG_BLENDER_RENDER_SETTINGS, windowTitle));
    } else {
        setLayout(layout);
        setWhatsThis(lpubWT(WT_DIALOG_BLENDER_RENDER_SETTINGS, windowTitle));
    }

    mContent = new QWidget();

    mForm = new QFormLayout(mContent);

    mContent->setLayout(mForm);

    QPalette ReadOnlyPalette = QApplication::palette();
    if (Preferences::displayTheme == THEME_DARK)
      ReadOnlyPalette.setColor(QPalette::Base,QColor(Preferences::themeColors[THEME_DARK_PALETTE_MIDLIGHT]));
    else
      ReadOnlyPalette.setColor(QPalette::Base,QColor(Preferences::themeColors[THEME_DEFAULT_PALETTE_LIGHT]));
    ReadOnlyPalette.setColor(QPalette::Text,QColor(LPUB3D_DISABLED_TEXT_COLOUR));

    // Blender Executable
    QGroupBox *blenderExeBox = new QGroupBox(tr("Blender Executable"),mContent);
    mForm->addRow(blenderExeBox);

    mExeGridLayout = new QGridLayout(blenderExeBox);
    blenderExeBox->setLayout(mExeGridLayout);

    mBlenderVersionLabel = new QLabel(blenderExeBox);
    mExeGridLayout->addWidget(mBlenderVersionLabel,0,0);

    mBlenderVersionEdit = new QLineEdit(blenderExeBox);
    mBlenderVersionEdit->setPalette(ReadOnlyPalette);
    mBlenderVersionEdit->setReadOnly(true);
    mExeGridLayout->addWidget(mBlenderVersionEdit,0,1,1,2);

    QLabel *pathLabel = new QLabel(blenderExeBox);
    mExeGridLayout->addWidget(pathLabel,1,0);

    QLineEdit *pathLineEdit = new QLineEdit(blenderExeBox);
    mExeGridLayout->addWidget(pathLineEdit,1,1);
    mPathLineEditList << pathLineEdit;
    connect(pathLineEdit, SIGNAL(editingFinished()),
            this,         SLOT(configureBlenderAddon()));

    QPushButton *pathBrowseButton = new QPushButton(tr("Browse..."), blenderExeBox);
    mExeGridLayout->addWidget(pathBrowseButton,1,2);
    mPathBrowseButtonList << pathBrowseButton;
    connect(pathBrowseButton, SIGNAL(clicked(bool)),
            this,             SLOT(browseBlender(bool)));

    // LDraw Addon
    QGroupBox *blenderAddonVersionBox = new QGroupBox(tr("%1 Blender LDraw Addon").arg(VER_PRODUCTNAME_STR),mContent);
    mForm->addRow(blenderAddonVersionBox);

    mAddonGridLayout = new QGridLayout(blenderAddonVersionBox);
    blenderAddonVersionBox->setLayout(mAddonGridLayout);

    mAddonVersionLabel = new QLabel(blenderAddonVersionBox);
    mAddonGridLayout->addWidget(mAddonVersionLabel,0,0);

    mAddonVersionEdit = new QLineEdit(blenderAddonVersionBox);
    mAddonVersionEdit->setToolTip(tr("%1 Blender LDraw import and image renderer addon").arg(VER_PRODUCTNAME_STR));
    mAddonVersionEdit->setPalette(ReadOnlyPalette);
    mAddonVersionEdit->setReadOnly(true);
    mAddonGridLayout->addWidget(mAddonVersionEdit,0,1);
    mAddonGridLayout->setColumnStretch(1,1/*1 is greater than 0 (default)*/);

    mAddonUpdateButton = new QPushButton(tr("Update"), blenderAddonVersionBox);
    mAddonUpdateButton->setToolTip(tr("Update %1 Blender LDraw addon").arg(VER_PRODUCTNAME_STR));
    mAddonGridLayout->addWidget(mAddonUpdateButton,0,2);
    connect(mAddonUpdateButton, SIGNAL(clicked(bool)),
            this,               SLOT(updateBlenderAddon()));

    mAddonStdOutButton = new QPushButton(tr("Output..."), blenderAddonVersionBox);
    mAddonStdOutButton->setToolTip(tr("Open the standrd output log"));
    mAddonStdOutButton->setEnabled(false);
    mAddonGridLayout->addWidget(mAddonStdOutButton,0,3);
    connect(mAddonStdOutButton, SIGNAL(clicked(bool)),
            this,               SLOT(getStandardOutput()));

    // LDraw Addon Enabled
    mModulesBox = new QGroupBox(tr("Enabled Addon Modules"),mContent);
    QHBoxLayout *modulesHLayout = new QHBoxLayout(mModulesBox);
    mModulesBox->setLayout(modulesHLayout);
    mAddonGridLayout->addWidget(mModulesBox,1,0,1,4);

    mImportActBox = new QCheckBox(tr("LDraw Import TN"),mModulesBox);
    mImportActBox->setToolTip(tr("Enable addon import module (adapted from LDraw Import by Toby Nelson) in Blender"));
    modulesHLayout->addWidget(mImportActBox);
    connect(mImportActBox, SIGNAL(clicked(bool)),
            this,          SLOT(enableImportModule()));

    mImportMMActBox = new QCheckBox(tr("LDraw Import MM"),mModulesBox);
    mImportMMActBox->setToolTip(tr("Enable addon import module (adapted from LDraw Import by Matthew Morrison) in Blender"));
    modulesHLayout->addWidget(mImportMMActBox);
    connect(mImportMMActBox, SIGNAL(clicked(bool)),
            this,            SLOT(enableImportModule()));

    mRenderActBox = new QCheckBox(tr("%1 Image Render").arg(VER_PRODUCTNAME_STR), mModulesBox);
    mRenderActBox->setToolTip(tr("Addon image render module in Blender"));
    mRenderActBox->setEnabled(false);
    modulesHLayout->addWidget(mRenderActBox);

    loadSettings();

    mConfigured = !Preferences::blenderImportModule.isEmpty();

    const int i = PATH_BLENDER;
    pathLabel->setText(mBlenderPaths[i].label);
    pathLabel->setToolTip(mBlenderPaths[i].tooltip);

    pathLineEdit->setText(mBlenderPaths[i].value);
    pathLineEdit->setToolTip(mBlenderPaths[i].tooltip);

    if (mAddonVersion.isEmpty()) {
        mModulesBox->setEnabled(false);
        mAddonUpdateButton->setEnabled(false);
        mImportActBox->setChecked(true); // default addon module
    } else {
        mAddonVersionEdit->setText(mAddonVersion);
        mRenderActBox->setChecked(true);
        mImportActBox->setChecked(  Preferences::blenderImportModule == QLatin1String("TN"));
        mImportMMActBox->setChecked(Preferences::blenderImportModule == QLatin1String("MM"));
    }

    QString versionTextColour = QApplication::palette().text().color().name(),addonTextColour;
    QString versionText = tr("Blender"), addonText = tr("Blender Addon");
    if (mConfigured) {
        addonTextColour = versionTextColour;
        mBlenderVersionEdit->setText(mBlenderVersion);
        mBlenderVersionEdit->setVisible(true);
    } else {
        QString const textColour = Preferences::displayTheme == THEME_DARK
                                       ? Preferences::themeColors[THEME_DARK_DECORATE_LPUB3D_QUOTED_TEXT]
                                       : QLatin1String("blue");

        if (Preferences::blenderExe.isEmpty()) {
            versionText = tr("Blender not configured.");
            versionTextColour = textColour;
        } else
            mBlenderVersionEdit->setText(mBlenderVersion);

        if (QFileInfo(QString("%1/Blender/%2").arg(Preferences::lpub3d3rdPartyConfigDir).arg(VER_BLENDER_ADDON_FILE)).isReadable()) {
            mModulesBox->setEnabled(false);
            mImportActBox->setChecked(true);
            mAddonUpdateButton->setEnabled(true);
            addonText = Preferences::blenderExe.isEmpty()
                            ? tr("Addon not configured.")
                            : tr("Addon not configured - Update.");
            addonTextColour = textColour;
        }
    }

    mAddonVersionEdit->setVisible(!mAddonVersion.isEmpty());
    mBlenderVersionLabel->setStyleSheet(QString("QLabel { color : %1; }").arg(versionTextColour));
    mAddonVersionLabel->setStyleSheet(QString("QLabel { color : %1; }").arg(addonTextColour));
    mBlenderVersionLabel->setText(versionText);
    mAddonVersionLabel->setText(addonText);

    if (mImportMMActBox->isChecked())
        initPathsAndSettingsMM();
    else
        initPathsAndSettings();

    // Scroll area
    QScrollArea *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setWidget(mContent);
    layout->addWidget(scrollArea);
}

BlenderPreferences::~BlenderPreferences()
{
    gAddonPreferences = nullptr;
}

void BlenderPreferences::clearGroupBox(QGroupBox *groupBox)
{
    int row = -1;
    QFormLayout::ItemRole itemRole = QFormLayout::SpanningRole;

    mForm->getWidgetPosition(groupBox, &row, &itemRole);

    if(row == -1 || itemRole != QFormLayout::SpanningRole) return;

    QLayoutItem* groupBoxIitem = mForm->itemAt ( row, itemRole );

    mForm->removeItem(groupBoxIitem);

    QWidget* widgetItem = groupBoxIitem->widget();
    if(widgetItem) {
        if (groupBox == mPathsBox) {
            delete mPathsBox;
            mPathsBox = nullptr;
        } else if (groupBox == mSettingsBox) {
            delete mSettingsBox;
            mSettingsBox = nullptr;
        }
    }

    delete groupBoxIitem;
}

void BlenderPreferences::initPathsAndSettings()
{
    if (!numSettings())
        loadSettings();

    // Paths
    clearGroupBox(mPathsBox);
    mPathsBox = new QGroupBox(mContent);
    mPathsBox->setTitle(tr("LDraw Import TN Addon Paths"));
    mPathsGridLayout = new QGridLayout(mPathsBox);
    mPathsBox->setLayout(mPathsGridLayout);
    mForm->addRow(mPathsBox);

    for(int i = 1/*skip blender executable*/; i < numPaths(); ++i) {
        int j = i - 1; // adjust for skipping first item - blender executable
        bool isVisible = i != PATH_STUDIO_LDRAW;
        QLabel *pathLabel = new QLabel(mBlenderPaths[i].label, mPathsBox);
        pathLabel->setToolTip(mBlenderPaths[i].tooltip);
        mPathsGridLayout->addWidget(pathLabel,j,0);
        pathLabel->setVisible(isVisible);

        QLineEdit *pathLineEdit = new QLineEdit(mPathsBox);
        pathLineEdit->setProperty("ControlID",QVariant(i));
        pathLineEdit->setText(mBlenderPaths[i].value);
        pathLineEdit->setToolTip(mBlenderPaths[i].tooltip);
        if (mPathLineEditList.size() > i)
            mPathLineEditList.replace(i, pathLineEdit);
        else
            mPathLineEditList << pathLineEdit;
        mPathsGridLayout->addWidget(pathLineEdit,j,1);
        pathLineEdit->setVisible(isVisible);

        QPushButton *pathBrowseButton = new QPushButton(tr("Browse..."), mPathsBox);
        if (mPathBrowseButtonList.size() > i)
            mPathBrowseButtonList.replace(i, pathBrowseButton);
        else
            mPathBrowseButtonList << pathBrowseButton;
        mPathsGridLayout->addWidget(pathBrowseButton,j,2);
        pathBrowseButton->setVisible(isVisible);

        if (isVisible) {
            connect(pathBrowseButton, SIGNAL(clicked(bool)),
                    this,             SLOT  (browseBlender(bool)));
            connect(pathLineEdit,     SIGNAL(editingFinished()),
                    this,             SLOT  (pathChanged()));
        }
    }

    mPathsBox->setEnabled(mConfigured);

    // Settings
    clearGroupBox(mSettingsBox);
    mSettingsBox = new QGroupBox(mContent);
    mSettingsSubform = new QFormLayout(mSettingsBox);
    mSettingsBox->setLayout(mSettingsSubform);
    mForm->addRow(mSettingsBox);

    mSettingsBox->setTitle(tr("LDraw Import TN Addon Settings"));
    mSettingLabelList.clear();
    mCheckBoxList.clear();
    mLineEditList.clear();
    mComboBoxList.clear();

    int comboBoxItemsIndex = 0;

    for(int i = 0; i < numSettings(); i++) {
        QLabel *label = new QLabel(mSettingsBox);
        label->setText(mBlenderSettings[i].label);
        label->setToolTip(mBlenderSettings[i].tooltip);
        mSettingLabelList << label;

        if (i < LBL_BEVEL_WIDTH){           // QCheckBoxes
            QCheckBox *checkBox = new QCheckBox(mSettingsBox);
            checkBox->setProperty("ControlID",QVariant(i));
            checkBox->setChecked(mBlenderSettings[i].value.toInt());
            checkBox->setToolTip(mBlenderSettings[i].tooltip);
            if (mDocumentRender &&
                (i == LBL_ADD_ENVIRONMENT ||
                 i == LBL_CROP_IMAGE ||
                 i == LBL_TRANSPARENT_BACKGROUND)) {
                checkBox->setEnabled(false);
            }
            if (!mDocumentRender &&
                i == LBL_CROP_IMAGE) {
                connect(checkBox,SIGNAL(toggled(bool)),
                        this,    SLOT  (setModelSize(bool)));
            } else {
                connect(checkBox,SIGNAL(clicked()),
                        this,    SLOT  (settingChanged()));
            }
            mCheckBoxList << checkBox;
            mSettingsSubform->addRow(label,checkBox);
        } else if (i < LBL_COLOUR_SCHEME) { // QLineEdits
            QLineEdit *lineEdit = new QLineEdit(mSettingsBox);
            lineEdit->setProperty("ControlID",QVariant(i));
            if (i == LBL_IMAGE_WIDTH || i == LBL_IMAGE_HEIGHT) {
                connect(lineEdit,SIGNAL(textChanged(const QString &)),
                        this,    SLOT  (sizeChanged(const QString &)));
                lineEdit->setValidator(new QIntValidator(16, RENDER_IMAGE_MAX_SIZE));
            } else if(i == LBL_DEFAULT_COLOUR) {
                lineEdit->setReadOnly(true);
                lineEdit->setStyleSheet("Text-align:left");
                mDefaultColourEditAction = lineEdit->addAction(QIcon(), QLineEdit::TrailingPosition);
                connect(mDefaultColourEditAction, SIGNAL(triggered(bool)),
                        this,                     SLOT  (colorButtonClicked(bool)));
            } else {
                lineEdit->setText(mBlenderSettings[i].value);
                if (i == LBL_IMAGE_SCALE)
                    lineEdit->setValidator(new QDoubleValidator(0.01,10.0,2));
                else if (i == LBL_RENDER_PERCENTAGE || i == LBL_CAMERA_BORDER_PERCENT)
                    lineEdit->setValidator(new QIntValidator(1,1000));
                else
                    lineEdit->setValidator(new QDoubleValidator(0.01,100.0,2));
                connect(lineEdit, SIGNAL(textEdited(    const QString &)),
                        this,     SLOT  (settingChanged(const QString &)));
            }
            lineEdit->setToolTip(mBlenderSettings[i].tooltip);
            mLineEditList << lineEdit;
            if (i == LBL_DEFAULT_COLOUR)
                setDefaultColor(lcGetColorIndex(mBlenderSettings[LBL_DEFAULT_COLOUR].value.toInt()));
            mSettingsSubform->addRow(label,lineEdit);
        } else {                            // QComboBoxes
            QComboBox *comboBox = new QComboBox(mSettingsBox);
            comboBox->setProperty("ControlID",QVariant(i));
            QString const value = mBlenderSettings[i].value;
            QStringList const dataList = mComboItems[comboBoxItemsIndex].dataList.split("|");
            QStringList const itemList = mComboItems[comboBoxItemsIndex].itemList.split("|");
            comboBox->addItems(itemList);
            for (int j = 0; j < comboBox->count(); j++)
                comboBox->setItemData(j, dataList.at(j));
            comboBox->setToolTip(mBlenderSettings[i].tooltip);
            int currentIndex = int(comboBox->findData(QVariant::fromValue(value)));
            comboBox->setCurrentIndex(currentIndex);
            if (i == LBL_COLOUR_SCHEME)
                connect(comboBox,SIGNAL(currentIndexChanged(int)),
                        this,    SLOT  (validateColourScheme(int)));
            else
                connect(comboBox,SIGNAL(currentIndexChanged(int)),
                        this,    SLOT  (settingChanged(int)));
            mComboBoxList << comboBox;
            comboBoxItemsIndex++;
            mSettingsSubform->addRow(label,comboBox);
        }
    }

    setModelSize();

    if (!mSettingsSubform->rowCount())
        mSettingsSubform = nullptr;

    mSettingsBox->setEnabled(mConfigured);
}

void BlenderPreferences::initPathsAndSettingsMM()
{
    if (!numSettingsMM())
        loadSettings();

    // Paths
    clearGroupBox(mPathsBox);
    mPathsBox = new QGroupBox(mContent);
    mPathsBox->setTitle(tr("LDraw Import MM Addon Paths"));
    mPathsGridLayout = new QGridLayout(mPathsBox);
    mPathsBox->setLayout(mPathsGridLayout);
    mForm->addRow(mPathsBox);

    for(int i = 1/*skip blender executable*/; i < numPaths(); ++i) {
        int j = i - 1; // adjust for skipping first item - blender executable
        bool isVisible = i != PATH_LSYNTH && i != PATH_STUD_LOGO;
        QLabel *pathLabel = new QLabel(mBlenderPaths[i].label, mPathsBox);
        pathLabel->setToolTip(mBlenderPaths[i].tooltip);
        mPathsGridLayout->addWidget(pathLabel,j,0);
        pathLabel->setVisible(isVisible);

        QLineEdit *pathLineEdit = new QLineEdit(mPathsBox);
        pathLineEdit->setProperty("ControlID",QVariant(i));
        pathLineEdit->setText(mBlenderPaths[i].value);
        pathLineEdit->setToolTip(mBlenderPaths[i].tooltip);
        if (mPathLineEditList.size() > i)
            mPathLineEditList.replace(i, pathLineEdit);
        else
            mPathLineEditList << pathLineEdit;
        mPathsGridLayout->addWidget(pathLineEdit,j,1);
        pathLineEdit->setVisible(isVisible);

        QPushButton *pathBrowseButton = new QPushButton(tr("Browse..."), mPathsBox);
        if (mPathBrowseButtonList.size() > i)
            mPathBrowseButtonList.replace(i, pathBrowseButton);
        else
            mPathBrowseButtonList << pathBrowseButton;
        mPathsGridLayout->addWidget(pathBrowseButton,j,2);
        pathBrowseButton->setVisible(isVisible);

        if (isVisible) {
            connect(pathBrowseButton, SIGNAL(clicked(bool)),
                    this,             SLOT  (browseBlender(bool)));
            connect(pathLineEdit,     SIGNAL(editingFinished()),
                    this,             SLOT  (pathChanged()));
        }
    }

    mPathsBox->setEnabled(mConfigured);

    // Settings
    clearGroupBox(mSettingsBox);
    mSettingsBox = new QGroupBox(mContent);
    mSettingsSubform = new QFormLayout(mSettingsBox);
    mSettingsBox->setLayout(mSettingsSubform);
    mForm->addRow(mSettingsBox);

    mSettingsBox->setTitle(tr("LDraw Import MM Addon Settings"));
    mSettingLabelList.clear();
    mCheckBoxList.clear();
    mLineEditList.clear();
    mComboBoxList.clear();

    int comboBoxItemsIndex = 0;

    for(int i = 0; i < numSettingsMM(); i++) {
        QLabel *label = new QLabel(mSettingsBox);
        label->setText(mBlenderSettingsMM[i].label);
        label->setToolTip(mBlenderSettingsMM[i].tooltip);
        mSettingLabelList << label;

        if (i < LBL_BEVEL_SEGMENTS) { // QCheckBoxes
            QCheckBox *checkBox = new QCheckBox(mSettingsBox);
            checkBox->setProperty("ControlID",QVariant(i));
            checkBox->setChecked(mBlenderSettingsMM[i].value.toInt());
            checkBox->setToolTip(mBlenderSettingsMM[i].tooltip);
            if (mDocumentRender &&
                (i == LBL_ADD_ENVIRONMENT_MM ||
                 i == LBL_CROP_IMAGE_MM ||
                 i == LBL_TRANSPARENT_BACKGROUND_MM)) {
                checkBox->setEnabled(false);
            }
            if (!mDocumentRender &&
                i == LBL_CROP_IMAGE_MM) {
                connect(checkBox,SIGNAL(toggled(bool)),
                        this,    SLOT  (setModelSize(bool)));
            } else {
                connect(checkBox,SIGNAL(clicked()),
                        this,    SLOT  (settingChanged()));
            }
            mCheckBoxList << checkBox;
            mSettingsSubform->addRow(label,checkBox);
        } else if (i < LBL_CHOSEN_LOGO) { // QLineEdits
            QLineEdit *lineEdit = new QLineEdit(mSettingsBox);
            lineEdit->setProperty("ControlID",QVariant(i));
            if (i == LBL_RESOLUTION_WIDTH || i == LBL_RESOLUTION_HEIGHT){
                connect(lineEdit,SIGNAL(textChanged(const QString &)),
                        this,    SLOT  (sizeChanged(const QString &)));
                lineEdit->setValidator(new QIntValidator(16, RENDER_IMAGE_MAX_SIZE));
            } else {
                lineEdit->setText(mBlenderSettingsMM[i].value);
                if (i == LBL_IMPORT_SCALE)
                    lineEdit->setValidator(new QDoubleValidator(0.01,10.0,2));
                else if (i == LBL_RENDER_PERCENTAGE_MM || i == LBL_CAMERA_BORDER_PERCENT_MM)
                    lineEdit->setValidator(new QIntValidator(1,1000));
                else if (i == LBL_GAP_SCALE || i == LBL_MERGE_DISTANCE)
                    lineEdit->setValidator(new QDoubleValidator(0.001,100.0,3));
                else if (i == LBL_BEVEL_WEIGHT || i == LBL_BEVEL_WIDTH_MM)
                    lineEdit->setValidator(new QDoubleValidator(0.0,10.0,1));
                else
                    lineEdit->setValidator(new QIntValidator(1, RENDER_IMAGE_MAX_SIZE));
                connect(lineEdit, SIGNAL(textEdited(    const QString &)),
                        this,     SLOT  (settingChanged(const QString &)));
            }
            lineEdit->setToolTip(mBlenderSettingsMM[i].tooltip);
            mLineEditList << lineEdit;
            mSettingsSubform->addRow(label,lineEdit);
        } else {                            // QComboBoxes
            QComboBox *comboBox = new QComboBox(mSettingsBox);
            comboBox->setProperty("ControlID",QVariant(i));
            QString const value = mBlenderSettingsMM[i].value;
            QStringList const dataList = mComboItemsMM[comboBoxItemsIndex].dataList.split("|");
            QStringList const itemList = mComboItemsMM[comboBoxItemsIndex].itemList.split("|");
            comboBox->addItems(itemList);
            for (int j = 0; j < comboBox->count(); j++)
                comboBox->setItemData(j, dataList.at(j));
            comboBox->setToolTip(mBlenderSettingsMM[i].tooltip);
            int currentIndex = int(comboBox->findData(QVariant::fromValue(value)));
            comboBox->setCurrentIndex(currentIndex);
            if (i == LBL_COLOUR_SCHEME_MM)
                connect(comboBox,SIGNAL(currentIndexChanged(int)),
                        this,    SLOT  (validateColourScheme(int)));
            else
                connect(comboBox,SIGNAL(currentIndexChanged(int)),
                        this,    SLOT  (settingChanged(int)));
            mComboBoxList << comboBox;
            comboBoxItemsIndex++;
            mSettingsSubform->addRow(label,comboBox);
        }
    }

    setModelSize();

    if (!mSettingsSubform->rowCount())
        mSettingsSubform = nullptr;

    mSettingsBox->setEnabled(mConfigured);
}

void BlenderPreferences::updateBlenderAddon()
{
    mAddonUpdateButton->setEnabled(false);

    disconnect(mPathLineEditList[PATH_BLENDER], SIGNAL(editingFinished()),
               this,                            SLOT  (configureBlenderAddon()));

    configureBlenderAddon(sender() == mPathBrowseButtonList[PATH_BLENDER],
                          sender() == mAddonUpdateButton);

    connect(mPathLineEditList[PATH_BLENDER], SIGNAL(editingFinished()),
            this,                            SLOT  (configureBlenderAddon()));
}

void BlenderPreferences::configureBlenderAddon(bool testBlender, bool addonUpdate, bool moduleChange)
{
    mProgressBar = nullptr;

    // Confirm blender exe exist
    QString const blenderExe = QDir::toNativeSeparators(mPathLineEditList[PATH_BLENDER]->text());

    if (blenderExe.isEmpty()) {
        mBlenderVersion.clear();
        mConfigured = false;
        statusUpdate(false/*addon*/,true/*error*/);
        mAddonUpdateButton->setEnabled(mConfigured);
        mPathsBox->setEnabled(mConfigured);
        mSettingsBox->setEnabled(mConfigured);
        emit gui->messageSig(LOG_INFO, tr("Blender path is empty. Quitting."));
        return;
    }

    if (QFileInfo(blenderExe).isReadable()) {
        // Setup
        enum ProcEnc { PR_OK, PR_FAIL, PR_WAIT, PR_INSTALL, PR_TEST };
        QString const blenderDir         = QDir::toNativeSeparators(QString("%1/Blender").arg(Preferences::lpub3d3rdPartyConfigDir));
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

        if (mConfigured && !addonUpdate && !moduleChange && !newBlenderExe)
            return;

        // Process command
        auto processCommand = [&] (ProcEnc action) {

            mProcess = new QProcess();

            QString processAction = tr("addon install");
            if (action == PR_INSTALL) {
                connect(mProcess, SIGNAL(readyReadStandardOutput()), this, SLOT(readStdOut()));

                QStringList systemEnvironment = QProcess::systemEnvironment();
                systemEnvironment.prepend("LDRAW_DIRECTORY=" + Preferences::ldrawLibPath);
                systemEnvironment.prepend("ADDONS_TO_LOAD=" + addonPathsAndModuleNames);
                mProcess->setEnvironment(systemEnvironment);
            }
            else
            {
                processAction = tr("test");
                disconnect(&mUpdateTimer, SIGNAL(timeout()), this, SLOT(update()));
            }

            mProcess->setWorkingDirectory(blenderDir);

            mProcess->setStandardErrorFile(QString("%1/stderr-blender-addon-install").arg(blenderDir));

            if (action == PR_INSTALL) {
                mProcess->start(blenderExe, arguments);
            } else {
#ifdef Q_OS_WIN
                mProcess->start(shellProgram, QStringList() << "/C" << script.fileName());
#else
                mProcess->start(shellProgram, QStringList() << script.fileName());
#endif
            }

            if (!mProcess->waitForStarted()) {
                message = tr("Cannot start Blender %1 mProcess.\n%2")
                              .arg(processAction)
                              .arg(QString(mProcess->readAllStandardError()));
                delete mProcess; // Close mProcess
                mProcess = nullptr;
                return PR_WAIT;
            } else {
                if (mProcess->exitStatus() != QProcess::NormalExit || mProcess->exitCode() != 0) {
                    message = tr("Failed to execute Blender %1.\n%2")
                                 .arg(processAction)
                                 .arg(QString(mProcess->readAllStandardError()));
                    return PR_FAIL;
                } else {
                    message = tr("Blender %1 mProcess [%2] running...").arg(processAction).arg(mProcess->processId());
                }
            }

            if (action == PR_TEST) {
                while (mProcess && mProcess->state() != QProcess::NotRunning) {
                    QTime waiting = QTime::currentTime().addMSecs(500);
                    while (QTime::currentTime() < waiting)
                        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
                }
                connect(&mUpdateTimer, SIGNAL(timeout()), this, SLOT(update()));
                QString const stdOut = QString(mProcess->readAllStandardOutput());
                if (!stdOut.contains(blenderTestString)) {
                    message =  tr("A simple check to test if the selected file is Blender failed."
                                  "Please create an %1 GitHub ticket if you are sure the file is Blender 2.8 or newer."
                                  "The ticket should contain the full path to the Blender executable.").arg(VER_PRODUCTNAME_STR);
                    return PR_FAIL;
                } else {
                    // Get Blender Version
                    QStringList items = stdOut.split('\n',SkipEmptyParts).last().split(" ");
                    if (items.count() > 6 && items.at(0) == QLatin1String("Blender")) {
                        items.takeLast();
                        mBlenderVersion.clear();
                        for (int i = 1; i < items.size(); i++)
                            mBlenderVersion.append(items.at(i)+" ");
                        mBlenderVersionFound = !mBlenderVersion.isEmpty();
                        if (mBlenderVersionFound) {
                            mBlenderVersion = mBlenderVersion.trimmed().prepend("v").append(")");
                            mBlenderVersionEdit->setText(mBlenderVersion);
                            // set default LDraw import module if not configured
                            if (!mImportActBox->isChecked() && !mImportMMActBox->isChecked())
                                mImportActBox->setChecked(true);
                            message = tr("Blender %1 mProcess completed. Version: %2 validated.").arg(processAction).arg(mBlenderVersion);
                        }
                    }
                }
            }

            return PR_OK;
        };

        connect(&mUpdateTimer, SIGNAL(timeout()), this, SLOT(update()));
        mUpdateTimer.start(500);

        if (!moduleChange) {
            mProgressBar = new QProgressBar(mContent);
            mProgressBar->setMaximum(0);
            mProgressBar->setMinimum(0);
            mProgressBar->setValue(1);

            // Test Blender executable
            testBlender |= sender() == mPathLineEditList[PATH_BLENDER];
            if (testBlender) {
                mExeGridLayout->replaceWidget(mBlenderVersionEdit, mProgressBar);
                mProgressBar->show();

                arguments << QString("--factory-startup");
                arguments << QString("-b");
                arguments << QString("--python-expr");
                arguments << QString("\"import sys;print('%1');sys.stdout.flush();sys.exit()\"").arg(blenderTestString);

                bool error = false;

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
                script.setFileName(QString("%1/%2").arg(QDir::tempPath()).arg(scriptName));
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

                if (error) {
                    emit gui->messageSig(LOG_ERROR, message);
                    statusUpdate(false/*addon*/);
                    return;
                }

                QThread::sleep(1);

#ifdef Q_OS_WIN
                shellProgram = QLatin1String(WINDOWS_SHELL);
#else
                shellProgram = QLatin1String(UNIX_SHELL);
#endif
                result = processCommand(PR_TEST);
                bool testOk = result != PR_FAIL;
                QString const statusLabel = testOk ? "" : tr("Blender test failed.");
                LogType logType = testOk ? LOG_INFO : LOG_ERROR;
                emit gui->messageSig(logType, message);
                statusUpdate(false/*addon*/, testOk, statusLabel);
                if (testOk) {
                    Preferences::setBlenderVersionPreference(mBlenderVersion);
                    Preferences::setBlenderExePathPreference(blenderExe);
                } else {
                    return;
                }
            } // Test Blender

            if (!mBlenderVersion.isEmpty() && !mImportMMActBox->isChecked() && !mImportActBox->isChecked()) {
                QString const &title = tr ("%1 Blender LDraw Addon Modules").arg(VER_PRODUCTNAME_STR);
                QString const &header = tr ("No import module enabled."
                                            "If you continue, the default import module (Import TN) will be used.<br>"
                                            "If you select No, all addon modules will be disabled.");
                QString const &body = tr ("Continue with the default import module ?");
                int exec = showMessage(header, title, body, QString(), MBB_YES_NO, QMessageBox::NoIcon);
                if (exec != QMessageBox::Yes) {
                    mRenderActBox->setChecked(false);
                    if (exec == QMessageBox::Cancel) {
                        return;
                    }
                }

                mImportActBox->setChecked(true);
            }

            if (testBlender) {
                QString const preferredImportModule =
                    mImportActBox->isChecked()
                        ? QString("TN")
                        : mImportMMActBox->isChecked()
                              ? QString("MM")
                              : QString();  // disable all import modules
                Preferences::setBlenderImportModule(preferredImportModule);
            } else {
                mBlenderVersionEdit->setVisible(addonUpdate);
            }

            if (mProgressBar) {
                mProgressBar->setMaximum(0);
                mProgressBar->setMinimum(0);
                mProgressBar->setValue(1);
                mAddonGridLayout->replaceWidget(mAddonVersionEdit, mProgressBar);
                mProgressBar->show();
            }

            // Download and extract blender addon
            if (!extractBlenderAddon(blenderDir)) {
                if (addonUpdate) {
                    mConfigured = true;
                    mBlenderVersionLabel->setText(tr("Blender"));
                    mBlenderVersionLabel->setStyleSheet(QString("QLabel { color : %1; }").arg(QApplication::palette().text().color().name()));
                    mBlenderVersionEdit->setText(mBlenderVersion);
                    mBlenderVersionEdit->setToolTip(tr("Display the Blender and %1 Render addon version").arg(VER_PRODUCTNAME_STR));
                    mBlenderVersionEdit->setVisible(mConfigured);
                    if (!mAddonVersion.isEmpty()) {
                        mModulesBox->setEnabled(true);
                        mAddonVersionEdit->setText(mAddonVersion);
                    }
                    mAddonUpdateButton->setEnabled(mConfigured);
                    if (mProgressBar) {
                        mExeGridLayout->replaceWidget(mProgressBar, mAddonVersionEdit);
                        mProgressBar->close();
                    }
                }
                return;
            }

            // Main installation script
            if (!QFileInfo(blenderInstallFile).exists()) {
                gui->messageSig(LOG_ERROR, tr("Could not find addon install file: %1").arg(blenderInstallFile));
                statusUpdate(true/*addon*/,true/*error*/,tr("Addon file not found."));
                return;
            }

            // Install Blender addon
            statusUpdate(true/*addon*/, false/*error*/, tr("Installing addon..."));

            // Create Blender config directory
            QDir configDir(Preferences::blenderConfigDir);
            if(!QDir(configDir).exists())
                configDir.mkpath(".");
        }

        // Save initial settings
        saveSettings();

        arguments.clear();
        arguments << QString("--background");
        arguments << QString("--python");
        arguments << blenderInstallFile;
        arguments << "--";
        if (!mRenderActBox->isChecked() &&
            !mImportActBox->isChecked() &&
            !mImportMMActBox->isChecked())
            arguments << QString("--disable_ldraw_addons");
        else if (!mImportActBox->isChecked())
            arguments << QString("--disable_ldraw_import");
        else if (!mImportMMActBox->isChecked())
            arguments << QString("--disable_ldraw_import_mm");
        else if (!mRenderActBox->isChecked())
            arguments << QString("--disable_ldraw_render");

        message = tr("Blender Addon Install Arguments: %1 %2").arg(blenderExe).arg(arguments.join(" "));
        emit gui->messageSig(LOG_INFO, message);

        if (!testBlender)
            mBlenderVersionFound = false;

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
            //F. Fill QByteArray with QJsonDocument (json compact format)
            addonPathsAndModuleNames = jsonDoc.toJson(QJsonDocument::Compact);
        }

        result = processCommand(PR_INSTALL);

        emit gui->messageSig(result == PR_OK ? LOG_INFO : LOG_ERROR, message);

        if (result != PR_OK)
            statusUpdate(true/*addon*/, true/*error*/, tr("Addon install failed."));

    } else {
        emit gui->messageSig(LOG_ERROR, tr("Blender executable not found at [%1]").arg(blenderExe), true);
    }
}

bool BlenderPreferences::extractBlenderAddon(const QString &blenderDir)
{
    bool proceed = true;

    QDir dir(blenderDir);
    if (!dir.exists())
        dir.mkdir(blenderDir);

    // Extract Blender addon
    if (getBlenderAddon(blenderDir)) {
        gAddonPreferences->statusUpdate(true/*addon*/, false/*error*/, tr("Extract addon..."));
        QString const blenderAddonFile = QDir::toNativeSeparators(QString("%1/%2").arg(blenderDir).arg(VER_BLENDER_ADDON_FILE));
        QStringList addonList = JlCompress::extractDir(blenderAddonFile, blenderDir);
        if (addonList.isEmpty()){
            emit gui->messageSig(LOG_ERROR, tr("Failed to extract %1 to %2")
                                                .arg(blenderAddonFile).arg(blenderDir));
            proceed = false;
        }
            emit gui->messageSig(LOG_INFO, tr("%1 items archive extracted to %2")
                                               .arg(addonList.size()).arg(blenderDir));
    }

    if (!proceed)
        gAddonPreferences->statusUpdate(true/*addon*/, true/*error*/,tr("Extract addon failed."));

    return proceed;
}

bool BlenderPreferences::getBlenderAddon(const QString &blenderDir)
{
    enum AddonEnc
    {
        ADDON_FAIL = -1,
        ADDON_NOT_FOUND,
        ADDON_DOWNLOAD = ADDON_NOT_FOUND,
        ADDON_ARCHIVE,
        ADDON_EXTRACTED,
        ADDON_RELOAD,
        ADDON_CANCEL
    };

    QString const blenderAddonDir  = QDir::toNativeSeparators(QString("%1/addons").arg(blenderDir));
    QString const blenderAddonFile = QDir::toNativeSeparators(QString("%1/%2").arg(blenderDir).arg(VER_BLENDER_ADDON_FILE));
    QString const addonVersionFile = QDir::toNativeSeparators(QString("%1/%2/__version__.py").arg(blenderAddonDir).arg(BLENDER_RENDER_ADDON_FOLDER));
    bool extractedAddon            = QFileInfo(addonVersionFile).isReadable();
    bool blenderAddonExists        = extractedAddon || QFileInfo(blenderAddonFile).isReadable();
    QString status                 = tr("Installing Blender addon...");
    AddonEnc addonAction           = ADDON_DOWNLOAD;
    QString localVersion, onlineVersion;

    using namespace std;
    auto versionStringCompare = [](string v1, string v2)
    {   // Returns 1 if v2 is smaller, -1 if v1 is smaller, 0 if equal
        int vnum1 = 0, vnum2 = 0;
        for (quint32 i = 0, j = 0; (i < v1.length() || j < v2.length());) {
            while (i < v1.length() && v1[i] != '.') {
                vnum1 = vnum1 * 10 + (v1[i] - '0');
                i++;
            }
            while (j < v2.length() && v2[j] != '.') {
                vnum2 = vnum2 * 10 + (v2[j] - '0');
                j++;
            }
            if (vnum1 > vnum2)
                return 1;
            if (vnum2 > vnum1)
                return -1;
            vnum1 = vnum2 = 0;
            i++;
            j++;
        }
        return 0;
    };

    auto getBlenderAddonVersionMatch = [&] ()
    {
        lpub->downloadFile(VER_BLENDER_ADDON_LATEST_URL, tr("Latest Addon"),false/*promptRedirect*/,false/*showProgress*/);
        QByteArray response_data = lpub->getDownloadedFile();
        if (!response_data.isEmpty()) {
            QJsonDocument json = QJsonDocument::fromJson(response_data);
            onlineVersion = json.object()["tag_name"].toString();
        } else {
            emit gui->messageSig(LOG_WARNING, tr("Check latest addon version failed."));
            return true; // Reload existing archive
        }

        QByteArray ba;
        if (!extractedAddon) {
            QuaZip zip(blenderAddonFile);
            if (!zip.open(QuaZip::mdUnzip)) {
                QString const result = tr("Could not open archive to check content. Return code %1.<br>"
                                          "Archive file %2 may be open in another program.")
                                          .arg(zip.getZipError()).arg(QFileInfo(blenderAddonFile).fileName());
                emit gui->messageSig(LOG_WARNING, result);
                return false; // Download new archive
            }

            QString const addonVersionFile = QString("addons/%1/__version__.py").arg(BLENDER_RENDER_ADDON_FOLDER);
            zip.setCurrentFile(addonVersionFile);
            QuaZipFile file(&zip);
            // check if legacy addon
            if (!file.open(QIODevice::ReadOnly))
                zip.setCurrentFile(QLatin1String("addons/io_scene_lpub3d_renderldraw/__version__.py"));
            if (!file.open(QIODevice::ReadOnly)) {
                emit gui->messageSig(LOG_WARNING, QObject::tr("Cannot read addon archive version file: [%1]<br>%2.")
                                                              .arg(addonVersionFile).arg(file.errorString()));
                return false; // Download new archive
            }
            ba = file.readAll();
            file.close();
            zip.close();
            if (zip.getZipError() != UNZ_OK)
                emit gui->messageSig(LOG_WARNING, tr("Archive close error. Return code %1.").arg(zip.getZipError()));
        } else {
            QFile file(addonVersionFile);
            if (!file.open(QIODevice::ReadOnly)) {
                emit gui->messageSig(LOG_WARNING, QObject::tr("Cannot read addon version file: [%1]<br>%2.")
                                                              .arg(addonVersionFile).arg(file.errorString()));
                return false; // Download new archive
            }
            ba = file.readAll();
            file.close();
        }

        QTextStream content(ba.data());
        while (!content.atEnd())
        {
            QString Token;
            content >> Token;     // version = (1, 3, 7)
            if (Token == QLatin1String("version")) {
                content >> Token; // skip the '=' sign
                localVersion = content.readAll().trimmed().replace("(","v").replace(",",".").replace(" ","").replace(")","");
            }
        }

        if (!localVersion.isEmpty() && !onlineVersion.isEmpty()) {
            // localVersion is smaller than onlineVersion so prompt to download new archive
            if (versionStringCompare(localVersion.toStdString(), onlineVersion.toStdString()) < 0)
                return false; // Download new archive
        }

        // Reload existing archive
        return true;
    };

    if (blenderAddonExists) {
        if (getBlenderAddonVersionMatch()) {
            addonAction = ADDON_RELOAD;
        } else if (Preferences::modeGUI) {
            if (localVersion.isEmpty())
                localVersion = gAddonPreferences->mAddonVersion;
            QString const &title = tr ("%1 Blender LDraw Addon").arg(VER_PRODUCTNAME_STR);
            QString const &header = tr ("Detected %1 Blender LDraw addon %2. A newer version %3 exists.")
                                        .arg(VER_PRODUCTNAME_STR)
                                        .arg(localVersion).arg(onlineVersion);
            QString const &body  = tr ("Do you want to download version %1 ?").arg(onlineVersion);
            int exec = showMessage(header, title, body, QString(), MBB_YES, QMessageBox::NoIcon);
            if (exec == QMessageBox::Cancel) {
                status = tr("Blender addon setup cancelled");
                addonAction = ADDON_CANCEL;
            } else if (exec == QMessageBox::No) {
                addonAction = ADDON_RELOAD;
            }
        }

        if (addonAction == ADDON_DOWNLOAD)
            status = tr("Downloading addon...");

        gAddonPreferences->statusUpdate(true/*addon*/, false/*error*/,status);

        if (addonAction == ADDON_CANCEL) {
            gAddonPreferences->mDialogCancelled = true;
            return false;
        }
    }

    // Remove old extracted addon if exist
    if (QFileInfo(blenderAddonDir).exists()) {
        bool result = true;
        QDir dir(blenderAddonDir);
        Q_FOREACH(QFileInfo const &info, dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot | QDir::NoSymLinks, QDir::DirsFirst)) {
            if (info.isDir())
                result &= QDir(info.absoluteFilePath()).removeRecursively();
            else
                result &= QFile::remove(info.absoluteFilePath());
            if (result)
                emit gui->messageSig(LOG_INFO, tr("Removed addon: [%1]").arg(info.absoluteFilePath()));
            else
                emit gui->messageSig(LOG_NOTICE, tr("Failed to remove addon: %1").arg(info.absoluteFilePath()));
        }
        result &= dir.rmdir(blenderAddonDir);
        if (!result)
            emit gui->messageSig(LOG_NOTICE, tr("Failed to remove Blender addon: %1")
                                                .arg(blenderAddonDir));
    }

    // Download Blender addon
    if (addonAction == ADDON_DOWNLOAD) {
        blenderAddonExists = false;
        lpub->downloadFile(VER_BLENDER_ADDON_URL, tr("Blender Addon"),false/*promptRedirect*/,false/*showProgress*/);
        QByteArray Buffer = lpub->getDownloadedFile();
        if (!Buffer.isEmpty()) {
            if (QFileInfo(blenderAddonFile).exists()) {
                QDir dir(blenderDir);
                if (!dir.remove(blenderAddonFile))
                    emit gui->messageSig(LOG_NOTICE, tr("Failed to remove Blender addon archive:<br>%1")
                                                         .arg(blenderAddonFile));
            }
            QFile file(blenderAddonFile);
            if (file.open(QIODevice::WriteOnly)) {
                file.write(Buffer);
                file.close();
                blenderAddonExists = true;
            } else {
                emit gui->messageSig(LOG_ERROR, tr("Failed to open Blender addon file:<br>%1:<br>%2")
                                                    .arg(blenderAddonFile)
                                                    .arg(file.errorString()));
            }
        } else {
            emit gui->messageSig(LOG_ERROR, tr("Failed to download Blender addon archive:<br>%1")
                                               .arg(blenderAddonFile));
        }
        if (!blenderAddonExists) {
            status = tr("Download addon failed.");
            gAddonPreferences->statusUpdate(true/*addon*/, true/*error*/,status);
        }
    } else if (!blenderAddonExists){
        emit gui->messageSig(LOG_ERROR, tr("Blender addon archive %1 was not found")
                                            .arg(blenderAddonFile));
    }

    return blenderAddonExists;
}

void BlenderPreferences::statusUpdate(bool addon, bool error, const QString &message)
{
    QString label, colour;
    QString const which = addon ? tr("Blender addon") : tr("Blender");
    if (mProgressBar) {
        if (addon) {
            mAddonGridLayout->replaceWidget(mProgressBar, mAddonVersionEdit);
        } else {
            mExeGridLayout->replaceWidget(mProgressBar, mBlenderVersionEdit);
            mProgressBar->hide();
        }
    }
    if (error) {
        if (!addon)
            mPathLineEditList[PATH_BLENDER]->text() = QString();

        Preferences::setBlenderImportModule(QString());

        label  = ! message.isEmpty() ? message : tr("%1 not configured").arg(which);
        colour = message.startsWith("Error:", Qt::CaseInsensitive)
                     ? QLatin1String("red")
                     : Preferences::displayTheme == THEME_DARK
                           ? Preferences::themeColors[THEME_DARK_DECORATE_LPUB3D_QUOTED_TEXT]
                           : QLatin1String("blue");

        mDialogCancelled = true;
    } else {
        label  = !message.isEmpty() ? message : tr("%1 setup...").arg(which);
        colour = QApplication::palette().text().color().name();
    }

    if (addon) {
        bool hasAddonVersion = !mAddonVersion.isEmpty();
        mAddonVersionLabel->setStyleSheet(QString("QLabel { color : %1; }").arg(colour));
        mAddonVersionLabel->setText(label);
        mAddonVersionEdit->setVisible(hasAddonVersion);
    } else {
        mBlenderVersionLabel->setStyleSheet(QString("QLabel { color : %1; }").arg(colour));
        mBlenderVersionLabel->setText(label);
        mBlenderVersionEdit->setVisible(!mBlenderVersion.isEmpty());
    }
}

void BlenderPreferences::showResult()
{
    QString message;
    bool hasError;
    const QString StdErrLog = readStdErr(hasError);

    if (mProgressBar)
        mProgressBar->close();

    writeStdOut();

    if (mProcess->exitStatus() != QProcess::NormalExit || mProcess->exitCode() != 0 || hasError)
    {
        QString const blenderDir = QString("%1/Blender").arg(Preferences::lpub3d3rdPartyConfigDir);
        message = tr("Addon install failed. See %1/stderr-blender-addon-install for details.").arg(blenderDir);
        statusUpdate(true/*addon*/, true/*error*/,tr("%1: Addon install failed.").arg("Error"));
        mConfigured = false;

        // emit gui->messageSig(LOG_BLENDER_ADDON, StdErrLog, true);

        QString const &title = tr ("%1 Blender Addon Install").arg(VER_PRODUCTNAME_STR);
        QString const &header =  "<b>" + tr ("Addon install failed.") + "</b>";
        QString const &body = tr ("LDraw addon install encountered one or more errors. See Show Details...");
        showMessage(header, title, body, StdErrLog, MBB_OK, QMessageBox::Critical);
    } else {
        QString const textColour = QString("QLabel { color : %1; }").arg(QApplication::palette().text().color().name());
        mAddonGridLayout->replaceWidget(mProgressBar, mAddonVersionEdit);
        mConfigured = true;
        mBlenderVersionLabel->setText(tr("Blender"));
        mBlenderVersionLabel->setStyleSheet(textColour);
        mBlenderVersionEdit->setText(mBlenderVersion);
        mBlenderVersionEdit->setToolTip(tr("Display the Blender and %1 Render addon version").arg(VER_PRODUCTNAME_STR));
        mBlenderVersionEdit->setVisible(mConfigured);
        mPathsBox->setEnabled(mConfigured);
        mSettingsBox->setEnabled(mConfigured);
        if (!mAddonVersion.isEmpty()) {
            mAddonVersionLabel->setText(tr("Blender Addon"));
            mAddonVersionLabel->setStyleSheet(textColour);
            mAddonVersionEdit->setText(mAddonVersion);
            mAddonVersionEdit->setVisible(true);
            mModulesBox->setEnabled(true);
            mAddonUpdateButton->setEnabled(true);
            Preferences::setBlenderVersionPreference(
                QString("%1|%2").arg(mBlenderVersion).arg(mAddonVersion));
            setModelSize(true/*update*/);
            saveSettings();
            mDialogCancelled = false;
        }
        message = tr("Blender version %1").arg(mBlenderVersion);
    }

    // Close mProcess
    delete mProcess;
    mProcess = nullptr;

    emit gui->messageSig(hasError ? LOG_ERROR : LOG_INFO, message);
}

void BlenderPreferences::settingChanged(const QString &value)
{
    bool change = false;

    QLineEdit *lineEdit = qobject_cast<QLineEdit *>(sender());
    if (lineEdit) {
        int i = lineEdit->property("ControlID").toInt();
        if (mImportMMActBox->isChecked()) {
            change = mBlenderSettingsMM[i].value != value;
        } else {
            change = mBlenderSettings[i].value != value;
        }

        change |= settingsModified(false/*update*/);
        emit settingChangedSig(change);
    }
}

void BlenderPreferences::settingChanged(int index)
{
    int i = -1;
    bool change = false;
    QString item;

    if (index > -1) {
        QComboBox *comboBox = qobject_cast<QComboBox *>(sender());
        if (comboBox) {
            i = comboBox->property("ControlID").toInt();
            item = comboBox->itemData(index).toString();
        }
    } else {
        QCheckBox *checkBox = qobject_cast<QCheckBox *>(sender());
        if (checkBox) {
            i = checkBox->property("ControlID").toInt();
            item = QString::number(checkBox->isChecked());
        }
    }

    if (i > -1) {
        if (mImportMMActBox->isChecked()) {
            change = mBlenderSettingsMM[i].value != item;
        } else {
            change = mBlenderSettings[i].value != item;
        }
    }

    change |= settingsModified(false/*update*/);
    emit settingChangedSig(change);
}

void BlenderPreferences::pathChanged()
{
    QLineEdit *lineEdit = qobject_cast<QLineEdit *>(sender());
    if (lineEdit) {
        bool change = false;
        const int i = lineEdit->property("ControlID").toInt();
        const QString &path = QDir::toNativeSeparators(lineEdit->text()).toLower();

        if (i != PATH_BLENDER) {
            change = QDir::toNativeSeparators(mBlenderPaths[i].value).toLower() != path;
        }

        change |= settingsModified(false/*update*/);
        emit settingChangedSig(change);
    }
}

void BlenderPreferences::getStandardOutput()
{
    QString const logFile = QString("%1/Blender/stdout-blender-addon-install").arg(Preferences::lpub3d3rdPartyConfigDir);
    QFileInfo fileInfo(logFile);
    if (!fileInfo.exists()) {
        emit gui->messageSig(LOG_ERROR, tr("Blender addon standard output file not found: %1.")
                                            .arg(fileInfo.absoluteFilePath()));
        return;
    }

    if (logFile.isEmpty())
        return;

    if (Preferences::useSystemEditor) {
        QDesktopServices::openUrl(QUrl("file:///"+logFile, QUrl::TolerantMode));
    } else {
        const QString _title = tr("%1 Blender Addon Standard Output").arg(VER_PRODUCTNAME_STR);
        ParmsWindow *parmsWindow = new ParmsWindow(mContent);
        parmsWindow->setWindowTitle(_title);
        parmsWindow->displayParmsFile(logFile);
        parmsWindow->show();
    }
}

void BlenderPreferences::readStdOut(const QString &stdOutput, QString &errors)
{
    QRegExp rxInfo("^INFO: ");
    QRegExp rxData("^DATA: ");
    QRegExp rxError("^(?:\\w)*ERROR: ", Qt::CaseInsensitive);
    QRegExp rxWarning("^(?:\\w)*WARNING: ", Qt::CaseInsensitive);
    QRegExp rxAddonVersion("^ADDON VERSION: ", Qt::CaseInsensitive);

    bool errorEncountered = false;
    QStringList items, errorList;
    QStringList stdOutLines = stdOutput.split(QRegExp("\n|\r\n|\r"));

    QString const saveAddonVersion = mAddonVersion;
    QString const saveVersion = mBlenderVersion;

    int lineCount = 0;
    int editListItems = mPathLineEditList.size();
    for (QString const &stdOutLine : stdOutLines) {
        if (stdOutLine.isEmpty())
            continue;

        if (!mBlenderVersionFound) {
            // Get Blender Version
            items = stdOutLine.split(" ");
            if (items.count() > 6 && items.at(0) == QLatin1String("Blender")) {
                items.takeLast();
                mBlenderVersion.clear();
                for (int i = 1; i < items.size(); i++)
                    mBlenderVersion.append(items.at(i)+" ");
                mBlenderVersionFound = !mBlenderVersion.isEmpty();
                if (mBlenderVersionFound) {
                    mBlenderVersion = mBlenderVersion.trimmed().prepend("v").append(")");
                    mBlenderVersionEdit->setText(mBlenderVersion);
                    // set default LDraw import module if not configured
                    if (!mImportActBox->isChecked() && !mImportMMActBox->isChecked())
                        mImportActBox->setChecked(true);
                    //emit gui->messageSig(LOG_DEBUG, tr("Blender version: %1 validated.").arg(mBlenderVersion));
                }
            }
        }

        if (stdOutLine.contains(rxInfo)) {
            items = stdOutLine.split(": ");
        } else if (stdOutLine.contains(rxData)) {
            items = stdOutLine.split(": ");
            if (items.at(1) == "ENVIRONMENT_FILE") {
                mBlenderPaths[PATH_ENVIRONMENT].value = items.at(2);
                if (editListItems > PATH_ENVIRONMENT)
                    mPathLineEditList[PATH_ENVIRONMENT]->setText(items.at(2));
            } else if (items.at(1) == "LSYNTH_DIRECTORY") {
                mBlenderPaths[PATH_LSYNTH].value = items.at(2);
                if (editListItems > PATH_LSYNTH)
                    mPathLineEditList[PATH_LSYNTH]->setText(items.at(2));
            } else if (items.at(1) == "STUDLOGO_DIRECTORY") {
                mBlenderPaths[PATH_STUD_LOGO].value = items.at(2);
                if (editListItems > PATH_STUD_LOGO)
                    mPathLineEditList[PATH_STUD_LOGO]->setText(items.at(2));
            }
        } else if (stdOutLine.contains(rxError) || stdOutLine.contains(rxWarning)) {
            errorList << stdOutLine.trimmed() +  + "<br>";
            if (!errorEncountered)
                errorEncountered = stdOutLine.contains(rxError);
        } else if (stdOutLine.contains(rxAddonVersion)) {
            // Get Addon version
            items = stdOutLine.split(":");
            mAddonVersion = tr("v%1").arg(items.at(1).trimmed()); // 1 addon version
            mAddonVersionEdit->setText(mAddonVersion);
        }
        lineCount++;
    }

    if (errorList.size()) {
        if (!mBlenderVersionFound) {
            if (mBlenderVersion != saveVersion) {
                mConfigured = false;
                mBlenderVersion = saveVersion;
                mBlenderVersionEdit->setText(mBlenderVersion);
            }
        }
        if (mAddonVersion != saveAddonVersion) {
            mConfigured = false;
            mAddonVersion = saveAddonVersion;
            mAddonVersionEdit->setText(mAddonVersion);
        }
        errors = errorList.join(" ");
    }
}

void BlenderPreferences::readStdOut()
{
    QString const &StdOut = QString(mProcess->readAllStandardOutput());

    mStdOutList.append(StdOut);

    QRegExp rxInfo("^INFO: ");
    QRegExp rxError("(?:\\w)*ERROR: ", Qt::CaseInsensitive);
    QRegExp rxWarning("(?:\\w)*WARNING: ", Qt::CaseInsensitive);

    bool const hasError = StdOut.contains(rxError);
    bool const hasWarning = StdOut.contains(rxWarning);

    if (StdOut.contains(rxInfo) && !hasError)
        statusUpdate(true/*addon*/, false/*error*/);

    QString errorsAndWarnings;

    readStdOut(StdOut, errorsAndWarnings);

    if (!errorsAndWarnings.isEmpty()) {
        QString const stdOutLog = QDir::toNativeSeparators(QString("<br>- See %1/Blender/stdout-blender-addon-install")
                                                               .arg(Preferences::lpub3d3rdPartyConfigDir));

        QMessageBox::Icon icon = hasError ? QMessageBox::Critical : QMessageBox::Warning;
        QString const &items = hasError ? tr("errors%1").arg(hasWarning ? tr(" and warnings") : "") : hasWarning ? tr("warnings") : "";

        QString const &title = tr ("%1 Blender Addon Install").arg(VER_PRODUCTNAME_STR);
        QString const &header =  "<b>" + tr ("Addon install standard output.") + "</b>";
        QString const &body = tr ("LDraw addon install encountered %1. See Show Details...").arg(items);
        showMessage(header, title, body, errorsAndWarnings.append(stdOutLog), MBB_OK, icon);
    }
}

QString BlenderPreferences::readStdErr(bool &hasError) const
{
    auto cleanLine = [] (const QString &line) {
        return line.trimmed()
               /*.replace("<","&lt;")
                 .replace(">","&gt;")
                 .replace("&","&amp;")*/ + "<br>";
    };
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
        QString const &line = in.readLine(0);
        returnLines << cleanLine(line);
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
        for (const QString& Line : mStdOutList)
            Out << Line << lpub_endl;
        file.close();
        mAddonStdOutButton->setEnabled(true);
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
    if (mProcess) {
        QString const &title = tr ("Cancel %1 Addon Install").arg(VER_PRODUCTNAME_STR);
        QString const &header =  "<b>" + tr("Are you sure you want to cancel the add on install ?") + "</b>";
        int exec = showMessage(header, title, QString(), QString(), MBB_YES_NO, QMessageBox::Question);
        if (exec == QMessageBox::Yes)
        {
            mProcess->kill();

            // Close mProcess
            delete mProcess;
            mProcess = nullptr;

        }
        else
            return false;
    }
#endif
    mDialogCancelled = true;
    return true;
}

void BlenderPreferences::update()
{
#ifndef QT_NO_PROCESS
    if (!mProcess)
        return;

    if (mProcess->state() == QProcess::NotRunning)
    {
        emit gui->messageSig(LOG_INFO, tr("Addon install finished"));
        showResult();
    }
#endif
    QApplication::processEvents();
}

void BlenderPreferences::apply(const int response)
{
    if (response == QDialog::Accepted) {
        if (settingsModified())
            saveSettings();
    } else if (mDialogCancelled) {
        if (settingsModified())
            if (promptAccept())
                saveSettings();
    }
}

bool BlenderPreferences::settingsModified(bool update, const QString &module)
{
    if  (gAddonPreferences->mDialogCancelled)
        return false;

    int &width = gAddonPreferences->mImageWidth;
    int &height = gAddonPreferences->mImageHeight;
    double &renderPercentage = gAddonPreferences->mRenderPercentage;

    bool moduleMM = !module.isEmpty()
                        ? module == QLatin1String("MM")
                        : gAddonPreferences->mImportMMActBox->isChecked();

    bool ok, modified = !QFileInfo(Preferences::blenderLDrawConfigFile).isReadable();
    qreal _width = 0.0, _height = 0.0, _renderPercentage = 0.0, _value = 0.0, _oldValue = 0.0;
    QString oldValue;

    auto itemChanged = [] (qreal oldValue, qreal newValue) {
        return newValue > oldValue || newValue < oldValue;
    };

    if (moduleMM) {
        // settings
        for(int i = 0; i < numSettingsMM(); i++) {
            // checkboxes
            if (i < LBL_BEVEL_SEGMENTS) {
                for(int j = 0; j < gAddonPreferences->mCheckBoxList.size(); j++) {
                    oldValue = mBlenderSettingsMM[i].value;
                    if (update)
                        mBlenderSettingsMM[i].value = QString::number(gAddonPreferences->mCheckBoxList[j]->isChecked());
                    modified |= mBlenderSettingsMM[i].value != oldValue;
                    if (i < LBL_VERBOSE_MM)
                        i++;
                }
            }
            // lineedits
            else if (i < LBL_CHOSEN_LOGO) {
                for(int j = 0; j < gAddonPreferences->mLineEditList.size(); j++) {
                    if (j == CTL_RESOLUTION_WIDTH_EDIT) {
                        _oldValue = width;
                        _width  = gAddonPreferences->mLineEditList[j]->text().toDouble(&ok);
                        if (ok) {
                            if (update) {
                                width = int(_width);
                                mBlenderSettingsMM[i].value = QString::number(width);
                            }
                            modified |= itemChanged(_oldValue, _width);
                        }
                    } else if (j == CTL_RESOLUTION_HEIGHT_EDIT) {
                        _oldValue = height;
                        _height = gAddonPreferences->mLineEditList[j]->text().toDouble(&ok);
                        if (ok) {
                            if (update) {
                                height = int(_height);
                                mBlenderSettingsMM[i].value = QString::number(height);
                            }
                            modified |= itemChanged(_oldValue, _height);
                        }
                    } else if (j == CTL_RENDER_PERCENTAGE_EDIT_MM) {
                        _oldValue = renderPercentage;
                        _renderPercentage = gAddonPreferences->mLineEditList[j]->text().toInt(&ok);
                        if (ok) {
                            if (update) {
                                renderPercentage = double(_renderPercentage / 100);
                                mBlenderSettingsMM[i].value = QString::number(_renderPercentage);
                            }
                            modified |= itemChanged(_oldValue, renderPercentage);
                        }
                    } else {
                        _oldValue = mBlenderSettingsMM[i].value.toDouble();
                        _value = gAddonPreferences->mLineEditList[j]->text().toDouble(&ok);
                        if (ok) {
                            if (update)
                                mBlenderSettingsMM[i].value = QString::number(_value);
                            modified |= itemChanged(_oldValue, _value);
                        }
                    }
                    if (i < LBL_STARTING_STEP_FRAME)
                        i++;
                }
            }
            // comboboxes
            else {
                for(int j = 0; j < gAddonPreferences->mComboBoxList.size(); j++) {
                    oldValue = mBlenderSettingsMM[i].value;
                    QString const value = gAddonPreferences->mComboBoxList[j]->itemData(gAddonPreferences->mComboBoxList[j]->currentIndex()).toString();
                    if (update)
                        mBlenderSettingsMM[i].value = value;
                    modified |= value != oldValue;
                    i++;
                }
            }
        }
    } else {
        // settings
        for(int i = 0; i < numSettings(); i++) {
            // checkboxes
            if (i < LBL_BEVEL_WIDTH) {
                for(int j = 0; j < gAddonPreferences->mCheckBoxList.size(); j++) {
                    oldValue = mBlenderSettings[i].value;
                    if (update)
                        mBlenderSettings[i].value = QString::number(gAddonPreferences->mCheckBoxList[j]->isChecked());
                    modified |= mBlenderSettings[i].value != oldValue;
                    if (i < LBL_VERBOSE)
                        i++;
                }
            }
            // lineedits
            else if (i < LBL_COLOUR_SCHEME) {
                for(int j = 0; j < gAddonPreferences->mLineEditList.size(); j++) {
                    if (j == CTL_IMAGE_WIDTH_EDIT) {
                        _oldValue = width;
                        _width  = gAddonPreferences->mLineEditList[j]->text().toDouble(&ok);
                        if (ok) {
                            if (update) {
                                width = int(_width);
                                mBlenderSettings[i].value = QString::number(width);
                            }
                            modified |= itemChanged(_oldValue, _width);
                        }
                    } else if (j == CTL_IMAGE_HEIGHT_EDIT) {
                        _oldValue = height;
                        _height = gAddonPreferences->mLineEditList[j]->text().toDouble(&ok);
                        if (ok) {
                            if (update) {
                                height = int(_height);
                                mBlenderSettings[i].value = QString::number(height);
                            }
                            modified |= itemChanged(_oldValue, _height);
                        }
                    } else if (j == CTL_RENDER_PERCENTAGE_EDIT) {
                        _oldValue = renderPercentage;
                        _renderPercentage = gAddonPreferences->mLineEditList[j]->text().toInt(&ok);
                        if (ok) {
                            if (update) {
                                renderPercentage = double(_renderPercentage / 100);
                                mBlenderSettings[i].value = QString::number(_renderPercentage);
                            }
                            modified |= itemChanged(_oldValue, renderPercentage);
                        }
                    } else {
                        if (j == CTL_DEFAULT_COLOUR_EDIT) {
                            _oldValue = lcGetColorIndex(mBlenderSettings[i].value.toInt());  // colour code
                            _value = gAddonPreferences->mLineEditList[j]->property("ColorIndex").toInt(&ok);
                        } else {
                            _oldValue = mBlenderSettings[i].value.toDouble();
                            _value = gAddonPreferences->mLineEditList[j]->text().toDouble(&ok);
                        }
                        if (ok) {
                            if (update) {
                                if (j == CTL_DEFAULT_COLOUR_EDIT) {
                                    mBlenderSettings[i].value = QString::number(lcGetColorCode(qint32(_value)));
                                } else {
                                    mBlenderSettings[i].value = QString::number(_value);
                                }
                            }
                            modified |= itemChanged(_oldValue, _value);
                        }
                    }
                    if (i < LBL_RENDER_PERCENTAGE)
                        i++;
                }
            }
            // comboboxes
            else {
                for(int j = 0; j < gAddonPreferences->mComboBoxList.size(); j++) {
                    oldValue = mBlenderSettings[i].value;
                    QString const value = gAddonPreferences->mComboBoxList[j]->itemData(gAddonPreferences->mComboBoxList[j]->currentIndex()).toString();
                    if (update)
                        mBlenderSettings[i].value = value;
                    modified |= value != oldValue;
                    i++;
                }
            }
        }
    }

    // paths
    for (int i = 0; i < numPaths(); i++) {
        oldValue = mBlenderPaths[i].value;
        QString const value = gAddonPreferences->mPathLineEditList[i]->text();
        if (update)
            mBlenderPaths[i].value = value;
        modified |= value != oldValue;
    }

    return modified;
}

void BlenderPreferences::resetSettings()
{
    BlenderPaths const *paths = mBlenderPaths;
    BlenderSettings const *settings = mBlenderSettings;
    BlenderSettings const *settingsMM = mBlenderSettingsMM;

    QDialog *dlg = new QDialog(this);
    dlg->setWindowTitle(tr("Addon Reset"));
    dlg->setWhatsThis(tr("Select how to reset settings. Choice is since last apply or system default."));
    QVBoxLayout *layout = new QVBoxLayout(dlg);
    QGroupBox *dlgGroup = new QGroupBox(dlg);
    QHBoxLayout *dlgLayout = new QHBoxLayout(dlgGroup);
    QRadioButton *lastBtn = new QRadioButton(tr("Last Apply"));
    dlgLayout->addWidget(lastBtn);
    QRadioButton *defaultBtn = new QRadioButton(tr("System Default"));
    dlgLayout->addWidget(defaultBtn);
    dlgGroup->setLayout(dlgLayout);
    layout->addWidget(dlgGroup);
    dlg->setLayout(layout);

    lastBtn->setChecked(true);

    QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
                               Qt::Horizontal, dlg);
    layout->addWidget(&buttonBox);
    connect(&buttonBox, SIGNAL(accepted()), dlg, SLOT(accept()));
    connect(&buttonBox, SIGNAL(rejected()), dlg, SLOT(reject()));

    if (dlg->exec() == QDialog::Accepted) {
        if (defaultBtn->isChecked()) {
            paths = mDefaultPaths;
            settings = mDefaultSettings;
            settingsMM = mDefaultSettingsMM;
        }
    }

    mConfigured = !Preferences::blenderImportModule.isEmpty();

    mBlenderPaths[PATH_BLENDER].value = Preferences::blenderExe;
    mBlenderVersion                   = Preferences::blenderVersion;
    mAddonVersion                     = Preferences::blenderAddonVersion;

    mBlenderVersionEdit->setText(mBlenderVersion);
    mAddonVersionEdit->setText(mAddonVersion);

    if (mImportActBox->isChecked()) {
        disconnect(mLineEditList[CTL_IMAGE_HEIGHT_EDIT],SIGNAL(textChanged(const QString &)),
                   this,                                SLOT  (sizeChanged(const QString &)));
        disconnect(mLineEditList[CTL_IMAGE_WIDTH_EDIT], SIGNAL(textChanged(const QString &)),
                   this,                                SLOT  (sizeChanged(const QString &)));

        for(int i = 0; i < numSettings(); i++) {
            if (i < LBL_BEVEL_WIDTH) {
                for(int j = 0; j < mCheckBoxList.size(); j++) {
                    mCheckBoxList[j]->setChecked(settings[i].value.toInt());
                    if (i < LBL_VERBOSE)
                        i++;
                }
            } else if (i < LBL_COLOUR_SCHEME) {
                for(int j = 0; j < mLineEditList.size(); j++) {
                    if (j == CTL_IMAGE_WIDTH_EDIT)
                        mLineEditList[j]->setText(QString::number(mImageWidth));
                    else if (j == CTL_IMAGE_HEIGHT_EDIT)
                        mLineEditList[j]->setText(QString::number(mImageHeight));
                    else if (j == CTL_RENDER_PERCENTAGE_EDIT)
                        mLineEditList[j]->setText(QString::number(mRenderPercentage * 100));
                    else if (j == CTL_DEFAULT_COLOUR_EDIT)
                        setDefaultColor(lcGetColorIndex(settings[LBL_DEFAULT_COLOUR].value.toInt()));
                    else
                        mLineEditList[j]->setText(settings[i].value);
                    if (i < LBL_RENDER_PERCENTAGE)
                        i++;
                }
            } else {
                for(int j = 0; j < mComboBoxList.size(); j++) {
                    mComboBoxList[j]->setCurrentIndex(int(mComboBoxList[j]->findData(QVariant::fromValue(settings[i].value))));
                    i++;
                }
            }
        }
        for (int i = 0; i < numPaths(); i++) {
            mPathLineEditList[i]->setText(paths[i].value);
        }

        connect(mLineEditList[CTL_IMAGE_HEIGHT_EDIT],SIGNAL(textChanged(const QString &)),
                this,                                SLOT  (sizeChanged(const QString &)));
        connect(mLineEditList[CTL_IMAGE_WIDTH_EDIT], SIGNAL(textChanged(const QString &)),
                this,                                SLOT  (sizeChanged(const QString &)));

    } else if (mImportMMActBox->isChecked()) {

        disconnect(mLineEditList[CTL_RESOLUTION_HEIGHT_EDIT],SIGNAL(textChanged(const QString &)),
                   this,                                     SLOT  (sizeChanged(const QString &)));
        disconnect(mLineEditList[CTL_RESOLUTION_WIDTH_EDIT], SIGNAL(textChanged(const QString &)),
                   this,                                     SLOT  (sizeChanged(const QString &)));

        for(int i = 0; i < numSettingsMM(); i++) {
            if (i < LBL_BEVEL_SEGMENTS) {
                for(int j = 0; j < mCheckBoxList.size(); j++) {
                    mCheckBoxList[j]->setChecked(settingsMM[i].value.toInt());
                    if (i < LBL_VERBOSE_MM)
                        i++;
                }
            } else if (i < LBL_CHOSEN_LOGO) {
                for(int j = 0; j < mLineEditList.size(); j++) {
                    if (j == CTL_RESOLUTION_WIDTH_EDIT)
                        mLineEditList[j]->setText(QString::number(mImageWidth));
                    else if (j == CTL_RESOLUTION_HEIGHT_EDIT)
                        mLineEditList[j]->setText(QString::number(mImageHeight));
                    else if (j == CTL_RENDER_PERCENTAGE_EDIT_MM)
                        mLineEditList[j]->setText(QString::number(mRenderPercentage * 100));
                    else
                        mLineEditList[j]->setText(settingsMM[i].value);
                    if (i < LBL_STARTING_STEP_FRAME)
                        i++;
                }
            } else {
                for(int j = 0; j < mComboBoxList.size(); j++) {
                    mComboBoxList[j]->setCurrentIndex(int(mComboBoxList[j]->findData(QVariant::fromValue(settingsMM[i].value))));
                    i++;
                }
            }
        }
        for (int i = 0; i < numPaths(); i++) {
            mPathLineEditList[i]->setText(paths[i].value);
        }

        connect(mLineEditList[CTL_RESOLUTION_HEIGHT_EDIT],SIGNAL(textChanged(const QString &)),
                this,                                     SLOT  (sizeChanged(const QString &)));
        connect(mLineEditList[CTL_RESOLUTION_WIDTH_EDIT], SIGNAL(textChanged(const QString &)),
                this,                                     SLOT  (sizeChanged(const QString &)));
    }

    emit settingChangedSig(true/*change*/);
}

void BlenderPreferences::loadSettings()
{
    if (QFileInfo(Preferences::blenderExe).isReadable())
    {
        if (Preferences::blenderImportModule.isEmpty())
            Preferences::setBlenderImportModule(QLatin1String(DEFAULT_BLENDER_IMPORT_MODULE));
    } else {
        Preferences::setBlenderExePathPreference(QString());
        Preferences::setBlenderImportModule(QString());
    }

    QString const blenderDir = QString("%1/Blender").arg(Preferences::lpub3d3rdPartyConfigDir);

    if (!QDir(QString("%1/addons/%2").arg(blenderDir).arg(BLENDER_RENDER_ADDON_FOLDER)).isReadable())
        Preferences::setBlenderImportModule(QString());

    // load default paths if paths not populated
    if (!numPaths()) {
        QString const defaultBlendFile = QString("%1/config/%2").arg(blenderDir).arg(VER_BLENDER_DEFAULT_BLEND_FILE);
        QStringList const addonPaths = QStringList()
        /* PATH_BLENDER      */        << Preferences::blenderExe
        /* PATH_BLENDFILE    */        << (Preferences::defaultBlendFile ? defaultBlendFile : QString())
        /* PATH_ENVIRONMENT  */        << QString()
        /* PATH_LDCONFIG     */        << Preferences::altLDConfigPath
        /* PATH_LDRAW        */        << Preferences::ldrawLibPath
        /* PATH_LSYNTH       */        << QString()
        /* PATH_STUD_LOGO    */        << QString()
        /* PATH_STUDIO_LDRAW */        << QString();
        for (int i = 0; i < numPaths(DEFAULT_SETTINGS); i++) {
            mBlenderPaths[i] = {
                mDefaultPaths[i].key,
                mDefaultPaths[i].key_mm,
                QDir::toNativeSeparators(addonPaths.at(i)),
                mDefaultPaths[i].label,
                mDefaultPaths[i].tooltip
            };
        }
    }

    // load default TN settings if settings not populated
    if (!numSettings()) {
        for (int i = 0; i < numSettings(DEFAULT_SETTINGS); i++) {
            mBlenderSettings[i] = {
                mDefaultSettings[i].key,
                mDefaultSettings[i].value,
                mDefaultSettings[i].label,
                mDefaultSettings[i].tooltip
            };
        }
    }

    // load default MM settings if settings not populated
    if (!numSettingsMM()) {
        for (int i = 0; i < numSettingsMM(DEFAULT_SETTINGS); i++) {
            mBlenderSettingsMM[i] = {
                mDefaultSettingsMM[i].key,
                mDefaultSettingsMM[i].value,
                mDefaultSettingsMM[i].label,
                mDefaultSettingsMM[i].tooltip
            };
        }
    }

    // set config file
    QFileInfo blenderConfigFileInfo;
    if (gAddonPreferences->mDocumentRender)
        blenderConfigFileInfo.setFile(Preferences::blenderPreferencesFile);
    else
        blenderConfigFileInfo.setFile(Preferences::blenderLDrawConfigFile);

    bool configFileExists = blenderConfigFileInfo.exists();

    // set defaults for document render settings
    if (gAddonPreferences->mDocumentRender) {
        mBlenderSettings[LBL_ADD_ENVIRONMENT].value             = "0";
        mBlenderSettings[LBL_TRANSPARENT_BACKGROUND].value      = "1";
        mBlenderSettings[LBL_CROP_IMAGE].value                  = "1";

        mBlenderSettingsMM[LBL_ADD_ENVIRONMENT_MM].value        = "0";
        mBlenderSettingsMM[LBL_TRANSPARENT_BACKGROUND_MM].value = "1";
        mBlenderSettingsMM[LBL_CROP_IMAGE_MM].value             = "1";
    }

    // config file settings
    if (configFileExists) {
        QSettings Settings(blenderConfigFileInfo.absoluteFilePath(), QSettings::IniFormat);

        for (int i = 1/*skip blender executable*/; i < numPaths(); i++) {
            if (i == PATH_STUDIO_LDRAW)
                continue;
            QString const &key = QString("%1/%2").arg(IMPORTLDRAW, mBlenderPaths[i].key);
            QString const &value = Settings.value(key, QString()).toString();
            if (QFileInfo(value).exists()) {
                mBlenderPaths[i].value = QDir::toNativeSeparators(value);
            }
        }

        for (int i = 1/*skip blender executable*/; i < numPaths(); i++) {
            if (i == PATH_LSYNTH || i == PATH_STUD_LOGO)
                continue;
            QString const &key = QString("%1/%2").arg(IMPORTLDRAWMM, mBlenderPaths[i].key_mm);
            QString const &value = Settings.value(key, QString()).toString();
            if (QFileInfo(value).exists()) {
                mBlenderPaths[i].value = QDir::toNativeSeparators(value);
            }
        }

        for (int i = 0; i < numSettings(); i++) {
            QString const &key = QString("%1/%2").arg(IMPORTLDRAW, mBlenderSettings[i].key);
            QString const &value = Settings.value(key, QString()).toString();
            if (!value.isEmpty()) {
                mBlenderSettings[i].value = value == "True" ? "1" : value == "False" ? "0" : value;
            }
            if (i == LBL_IMAGE_WIDTH || i == LBL_IMAGE_HEIGHT || i == LBL_RENDER_PERCENTAGE) {
                QString const &label = mDefaultSettings[i].label;
                mBlenderSettings[i].label = QString("%1 - Setting (%2)").arg(label).arg(value);
            }
        }

        for (int i = 0; i < numSettingsMM(); i++) {
            QString const &key = QString("%1/%2").arg(IMPORTLDRAWMM, mBlenderSettingsMM[i].key);
            QString const &value = Settings.value(key, QString()).toString();
            if (!value.isEmpty()) {
                mBlenderSettingsMM[i].value = value == "True" ? "1" : value == "False" ? "0" : value;
            }
            if (i == LBL_RENDER_PERCENTAGE_MM || i == LBL_RESOLUTION_WIDTH || i == LBL_RESOLUTION_HEIGHT) {
                QString const &label = mDefaultSettingsMM[i].label;
                mBlenderSettingsMM[i].label = QString("%1 - Setting (%2)").arg(label).arg(value);
            }
        }
    } else {
        QString const logFile = QString("%1/Blender/stdout-blender-addon-install").arg(Preferences::lpub3d3rdPartyConfigDir);
        if (QFileInfo(logFile).isReadable()) {
            QFile file(logFile);
            if (file.open(QFile::ReadOnly | QFile::Text))
            {
                QByteArray ba = file.readAll();
                file.close();
                QString errors;
                gAddonPreferences->mProgressBar = nullptr;
                gAddonPreferences->readStdOut(QString(ba), errors);
            } else {
                emit gui->messageSig(LOG_WARNING, tr("Failed to open log file: %1:\n%2")
                                                      .arg(file.fileName())
                                                      .arg(file.errorString()));
            }
        }
    }

    mBlenderSettings[LBL_IMAGE_WIDTH].value            = QString::number(gAddonPreferences->mImageWidth);
    mBlenderSettings[LBL_IMAGE_HEIGHT].value           = QString::number(gAddonPreferences->mImageHeight);
    mBlenderSettings[LBL_RENDER_PERCENTAGE].value      = QString::number(gAddonPreferences->mRenderPercentage * 100);

    mBlenderSettingsMM[LBL_RESOLUTION_WIDTH].value     = QString::number(gAddonPreferences->mImageWidth);
    mBlenderSettingsMM[LBL_RESOLUTION_HEIGHT].value    = QString::number(gAddonPreferences->mImageHeight);
    mBlenderSettingsMM[LBL_RENDER_PERCENTAGE_MM].value = QString::number(gAddonPreferences->mRenderPercentage * 100);

    mBlenderPaths[PATH_BLENDER].value              = Preferences::blenderExe;
    gAddonPreferences->mBlenderVersion                 = Preferences::blenderVersion;
    gAddonPreferences->mAddonVersion                   = Preferences::blenderAddonVersion;
}

void BlenderPreferences::saveSettings()
{
    if (!numSettings() || !numSettingsMM())
        loadSettings();

    QString value = mBlenderPaths[PATH_BLENDER].value;
    if (value.isEmpty())
        value = gAddonPreferences->mPathLineEditList[PATH_BLENDER]->text();
    Preferences::setBlenderExePathPreference(QDir::toNativeSeparators(value));

    value.clear();
    if (!gAddonPreferences->mBlenderVersion.isEmpty())
        value = gAddonPreferences->mBlenderVersion;
    if (!gAddonPreferences->mAddonVersion.isEmpty()) {
        gAddonPreferences->mModulesBox->setEnabled(true);
        gAddonPreferences->mAddonVersionEdit->setText(gAddonPreferences->mAddonVersion);
        value.append(QString("|%1").arg(gAddonPreferences->mAddonVersion));
    }
    Preferences::setBlenderVersionPreference(value);

    if (gAddonPreferences->mDocumentRender)
        value = Preferences::blenderPreferencesFile.isEmpty()
                    ? QString("%1/%2").arg(Preferences::blenderConfigDir).arg(VER_BLENDER_DOCUMENT_CONFIG_FILE)
                    : Preferences::blenderPreferencesFile;
    else
        value = Preferences::blenderLDrawConfigFile.isEmpty()
                    ? QString("%1/%2").arg(Preferences::blenderConfigDir).arg(VER_BLENDER_ADDON_CONFIG_FILE)
                    : Preferences::blenderLDrawConfigFile;
    Preferences::setBlenderLDrawConfigPreference(QDir::toNativeSeparators(value));

    QString searchDirectoriesKey;
    QString parameterFileKey = QLatin1String("parameterFile");
    QString parameterFile = QString("%1/%2").arg(Preferences::blenderConfigDir).arg(VER_BLENDER_LDRAW_PARAMS_FILE);

    QSettings Settings(value, QSettings::IniFormat);

    auto concludeSettingsGroup = [&] () {
        if (!QFileInfo(parameterFile).exists())
            exportParameterFile();
        QString const value = QDir::toNativeSeparators(Preferences::ldSearchDirs.join(","));
        Settings.setValue(searchDirectoriesKey, QVariant(value));
        Settings.endGroup();
    };

    Settings.beginGroup(IMPORTLDRAW);

    for (int i = 1/*skip blender executable*/; i < numPaths(); i++) {
        if (i == PATH_STUDIO_LDRAW)
            continue;
        QString const key = mBlenderPaths[i].key;
        QString const value = QDir::toNativeSeparators(mBlenderPaths[i].value);
        Settings.setValue(key, QVariant(value));
    }

    for (int i = 0; i < numSettings(); i++) {
        if (i == LBL_KEEP_ASPECT_RATIO) {
            continue;
        } else if (i < LBL_BEVEL_WIDTH) {
            value = mBlenderSettings[i].value == "1" ? "True" : "False";
        } else if (i > LBL_VERBOSE) {
            if (i == LBL_FLEX_PARTS_SOURCE || i == LBL_POSITION_OBJECT)
                value = mBlenderSettings[i].value == "1" ? "True" : "False";
            else
                value = mBlenderSettings[i].value;
        }

        QString const key = mBlenderSettings[i].key;
        Settings.setValue(key, QVariant(value));
    }

    Settings.setValue(parameterFileKey, QVariant(QDir::toNativeSeparators(parameterFile)));

    searchDirectoriesKey = QLatin1String("additionalSearchDirectories");

    concludeSettingsGroup();

    Settings.beginGroup(IMPORTLDRAWMM);

    for (int i = 1/*skip blender executable*/; i < numPaths(); i++) {
        if (i == PATH_LSYNTH || i == PATH_STUD_LOGO)
            continue;
        QString const key = mBlenderPaths[i].key_mm;
        QString const value = QDir::toNativeSeparators(mBlenderPaths[i].value);
        Settings.setValue(key, QVariant(value));
    }

    for (int i = 0; i < numSettingsMM(); i++) {
        if (i == LBL_KEEP_ASPECT_RATIO_MM) {
            continue;
        } else if (i < LBL_BEVEL_SEGMENTS) {
            value = mBlenderSettingsMM[i].value == "1" ? "True" : "False";
        } else if (i > LBL_VERBOSE_MM) {
            value = mBlenderSettingsMM[i].value;
        }
        QString const key = mBlenderSettingsMM[i].key;
        Settings.setValue(key, QVariant(value));
    }

    searchDirectoriesKey = QLatin1String("additionalSearchPaths");

    concludeSettingsGroup();

    QString const preferredImportModule =
        gAddonPreferences->mImportActBox->isChecked()
            ? QString("TN")
            : gAddonPreferences->mImportMMActBox->isChecked()
                  ? QString("MM") : QString();
    if (preferredImportModule != Preferences::blenderImportModule)
        Preferences::setBlenderImportModule(preferredImportModule);
}

void BlenderPreferences::enableImportModule()
{
    QString saveImportModule, preferredImportModule;
    if (sender() == mImportActBox && mImportActBox->isChecked()) {
        preferredImportModule = QLatin1String("TN");
        if (mImportMMActBox->isChecked())
            saveImportModule = QLatin1String("MM");
        mImportMMActBox->setChecked(false);
    } else if (sender() == mImportMMActBox && mImportMMActBox->isChecked()) {
        preferredImportModule = QLatin1String("MM");
        if (mImportActBox->isChecked())
            saveImportModule = QLatin1String("TN");
        mImportActBox->setChecked(false);
    }

    if (preferredImportModule.isEmpty())
        return;

    if (settingsModified(true/*update*/, saveImportModule))
        saveSettings();

    Preferences::setBlenderImportModule(preferredImportModule);

    if (mImportMMActBox->isChecked())
        initPathsAndSettingsMM();
    else
        initPathsAndSettings();

    configureBlenderAddon(false/*testBlender*/, false/*addonUpdate*/, true/*moduleChange*/);
}

int BlenderPreferences::numSettings(bool defaultSettings)
{
    int size = 0;
    if (!mBlenderSettings[0].key.isEmpty() || defaultSettings)
        size = sizeof(mBlenderSettings)/sizeof(mBlenderSettings[0]);
    return size;
}

int BlenderPreferences::numSettingsMM(bool defaultSettings)
{
    int size = 0;
    if (!mBlenderSettingsMM[0].key.isEmpty() || defaultSettings)
        size = sizeof(mBlenderSettingsMM)/sizeof(mBlenderSettingsMM[0]);
    return size;
}

int BlenderPreferences::numPaths(bool defaultSettings)
{
    int size = 0;
    if (!mBlenderPaths[0].key.isEmpty() || defaultSettings)
        size = sizeof(mBlenderPaths)/sizeof(mBlenderPaths[0]);
    return size;
}

void BlenderPreferences::showPathsGroup()
{
    if (mPathsBox->isHidden())
        mPathsBox->show();
    else
        mPathsBox->hide();
    mContent->adjustSize();
}

void BlenderPreferences::colorButtonClicked(bool)
{
    int ColorIndex = mLineEditList[CTL_DEFAULT_COLOUR_EDIT]->property("ColorIndex").toInt();

    QWidget *parent = mLineEditList[CTL_DEFAULT_COLOUR_EDIT];
    lcColorPickerPopup *popup = new lcColorPickerPopup(parent, ColorIndex);
    connect(popup, SIGNAL(Selected(int)), SLOT(setDefaultColor(int)));
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

void BlenderPreferences::setDefaultColor(int colorIndex)
{
    QImage img(12, 12, QImage::Format_ARGB32);
    img.fill(0);

    lcColor* color = &gColorList[colorIndex];
    QPainter painter(&img);
    painter.setCompositionMode(QPainter::CompositionMode_Source);
    painter.setPen(Qt::darkGray);
    painter.setBrush(QColor::fromRgbF(qreal(color->Value[0]), qreal(color->Value[1]), qreal(color->Value[2])));
    painter.drawRect(0, 0, img.width() - 1, img.height() - 1);
    painter.end();

    int const colorCode = lcGetColorCode(colorIndex);
    mLineEditList[CTL_DEFAULT_COLOUR_EDIT]->setText(QString("%1 (%2)").arg(color->Name).arg(colorCode));
    mLineEditList[CTL_DEFAULT_COLOUR_EDIT]->setProperty("ColorIndex", QVariant::fromValue(colorIndex));
    mDefaultColourEditAction->setIcon(QPixmap::fromImage(img));
    mDefaultColourEditAction->setToolTip(tr("Select Colour"));

    bool change = mBlenderSettings[LBL_DEFAULT_COLOUR].value != QString::number(colorCode);
    change |= settingsModified(false/*update*/);
    emit settingChangedSig(change);
}

void BlenderPreferences::browseBlender(bool)
{
    for(int i = 0; i < numPaths(); ++i) {
        if (sender() == mPathBrowseButtonList.at(i)) {
            QString const blenderPath = QDir::toNativeSeparators(mBlenderPaths[i].value).toLower();
            QFileDialog fileDialog(nullptr);
            fileDialog.setWindowTitle(tr("Locate %1").arg(mBlenderPaths[i].label));
            if (i < PATH_LDRAW)
                fileDialog.setFileMode(QFileDialog::ExistingFile);
            else
                fileDialog.setFileMode(QFileDialog::Directory);
            if (!blenderPath.isEmpty())
                fileDialog.setDirectory(QFileInfo(blenderPath).absolutePath());
            if (fileDialog.exec()) {
                QStringList selectedPathList = fileDialog.selectedFiles();
                if (selectedPathList.size() == 1) {
                    QFileInfo  pathInfo(selectedPathList.at(0));
                    if (pathInfo.exists()) {
                        QString const selectedPath = QDir::toNativeSeparators(pathInfo.absoluteFilePath()).toLower();
                        mPathLineEditList[i]->setText(selectedPathList.at(0));
                        if (i != PATH_BLENDER) {
                            bool change = false;
                            if (mImportMMActBox->isChecked()) {
                                change = QDir::toNativeSeparators(mBlenderSettingsMM[i].value).toLower() != selectedPath;
                            } else {
                                change = QDir::toNativeSeparators(mBlenderSettings[i].value).toLower() != selectedPath;
                            }
                            change |= settingsModified(false/*update*/);
                            emit settingChangedSig(change);
                        }
                        if (i == PATH_BLENDER && blenderPath != selectedPath) {
                            mBlenderPaths[i].value = selectedPath;
                            updateBlenderAddon();
                        }
                    }
                }
            }
        }
    }
}

void BlenderPreferences::sizeChanged(const QString &value)
{
    const bool importMM = mImportMMActBox->isChecked();
    const int keep_aspect_ratio = importMM ? int(CTL_KEEP_ASPECT_RATIO_BOX_MM) : int(CTL_KEEP_ASPECT_RATIO_BOX);
    const int width_edit = importMM ? int(CTL_RESOLUTION_WIDTH_EDIT) : int(CTL_IMAGE_WIDTH_EDIT);
    const int height_edit = importMM ? int(CTL_RESOLUTION_HEIGHT_EDIT) : int(CTL_IMAGE_HEIGHT_EDIT);
    BlenderSettings const *settings = importMM ? mBlenderSettingsMM : mBlenderSettings;

    /* original height x new width / original width = new height */
    bool change = false;
    int mNewValue = value.toInt();
    if (mCheckBoxList[keep_aspect_ratio]->isChecked())
    {
        if (sender() == mLineEditList[width_edit])
        {
            disconnect(mLineEditList[height_edit],SIGNAL(textChanged(const QString &)),
                       this,                      SLOT  (sizeChanged(const QString &)));

            QString const height = QString::number(qRound(double(mImageHeight * mNewValue / mImageWidth)));
            mLineEditList[height_edit]->setText(height);

            change = settings[height_edit].value != height;

            connect(mLineEditList[height_edit],SIGNAL(textChanged(const QString &)),
                    this,                      SLOT  (sizeChanged(const QString &)));
        }
        else if (sender() == mLineEditList[height_edit])
        {
            disconnect(mLineEditList[width_edit],SIGNAL(textChanged(const QString &)),
                       this,                     SLOT  (sizeChanged(const QString &)));

            QString const width = QString::number(qRound(double(mNewValue * mImageWidth / mImageHeight)));
            mLineEditList[width_edit]->setText(width);

            change = settings[height_edit].value != width;

            connect(mLineEditList[width_edit],SIGNAL(textChanged(const QString &)),
                    this,                     SLOT  (sizeChanged(const QString &)));
        }

        // Change is provided here for consistency only as ImageWidth,
        // ImageHeight, and RenderPercentage are passed at the render command
        change |= settingsModified(false/*update*/);
        emit settingChangedSig(change);
    }
}

void BlenderPreferences::setModelSize(bool update)
{
    const bool importMM = mImportMMActBox->isChecked();
    const int crop_image = importMM ? int(CTL_CROP_IMAGE_BOX_MM) : int(CTL_CROP_IMAGE_BOX);
    const int add_environment = importMM ? int(CTL_ADD_ENVIRONMENT_BOX_MM) : int(CTL_ADD_ENVIRONMENT_BOX);
    const int trans_background = importMM ? int(CTL_TRANSPARENT_BACKGROUND_BOX_MM) : int(CTL_TRANSPARENT_BACKGROUND_BOX);
    const int keep_aspect_ratio = importMM ? int(CTL_KEEP_ASPECT_RATIO_BOX_MM) : int(CTL_KEEP_ASPECT_RATIO_BOX);
    const int width_edit = importMM ? int(CTL_RESOLUTION_WIDTH_EDIT) : int(CTL_IMAGE_WIDTH_EDIT);
    const int height_edit = importMM ? int(CTL_RESOLUTION_HEIGHT_EDIT) : int(CTL_IMAGE_HEIGHT_EDIT);
    const int crop_image_label = importMM ? int(LBL_CROP_IMAGE_MM) : int(LBL_CROP_IMAGE);

    const int imageWidth = gui->GetImageWidth();
    const int imageHeight = gui->GetImageHeight();

    const bool cropImage = mCheckBoxList[crop_image]->isChecked();

    QString const cropImageLabel = mSettingLabelList[crop_image_label]->text();
    mSettingLabelList[crop_image_label]->setText(QString("%1 (%2 x %3)")
                                                     .arg(cropImageLabel)
                                                     .arg(imageWidth)
                                                     .arg(imageHeight));

    if (cropImage) {
        bool conflict[3];

        if ((conflict[1] = mCheckBoxList[add_environment]->isChecked()))
            mCheckBoxList[add_environment]->setChecked(!cropImage);
        if ((conflict[2] = !mCheckBoxList[trans_background]->isChecked()))
            mCheckBoxList[trans_background]->setChecked(cropImage);
        if ((conflict[0] = mCheckBoxList[keep_aspect_ratio]->isChecked()))
            mCheckBoxList[keep_aspect_ratio]->setChecked(!cropImage);

        if (conflict[0] || conflict[1] || conflict[2]) {
            QString const &title = tr ("LDraw Render Settings Conflict");
            QString const &header = "<b>" + tr ("Crop image configuration settings conflict were resolved.") + "</b>";
            QString const &body = QString("%1%2%3")
                                      .arg(conflict[0] ? tr("Keep aspect ratio set to false.<br>") : "")
                                      .arg(conflict[1] ? tr("Add environment (backdrop and base plane) set to false.<br>") : "")
                                      .arg(conflict[2] ? tr("Transparent background set to true.<br>") : "");
            showMessage(header, title, body, QString(), MBB_OK, QMessageBox::Information);
        }
    }

    disconnect(mLineEditList[width_edit],SIGNAL(textChanged(const QString &)),
               this,                     SLOT  (sizeChanged(const QString &)));
    disconnect(mLineEditList[height_edit],SIGNAL(textChanged(const QString &)),
               this,                      SLOT  (sizeChanged(const QString &)));

    QString const width = QString::number(cropImage ? imageWidth : mImageWidth);
    QString const height = QString::number(cropImage ? imageHeight : mImageHeight);

    mLineEditList[width_edit]->setText(width);
    mLineEditList[height_edit]->setText(height);

    if (update)
        settingsModified(true/*update*/);

    connect(mLineEditList[height_edit],SIGNAL(textChanged(const QString &)),
            this,                      SLOT  (sizeChanged(const QString &)));
    connect(mLineEditList[width_edit],SIGNAL(textChanged(const QString &)),
            this,                     SLOT  (sizeChanged(const QString &)));
}

void BlenderPreferences::validateColourScheme(int index)
{
    QComboBox *combo = qobject_cast<QComboBox *>(sender());
    if (!combo)
        return;

    const bool importMM = mImportMMActBox->isChecked();
    const int color_scheme = importMM ? int(LBL_COLOUR_SCHEME_MM) : int(LBL_COLOUR_SCHEME);
    BlenderSettings *settings = importMM ? mBlenderSettingsMM : mBlenderSettings;

    if (combo->itemText(index) == "custom" &&
        mBlenderPaths[PATH_LDCONFIG].value.isEmpty() &&
        Preferences::altLDConfigPath.isEmpty()) {
        BlenderSettings const *defaultSettings = importMM ? mDefaultSettingsMM : mDefaultSettings;
        settings[color_scheme].value = defaultSettings[color_scheme].value;

        QString const &title = tr ("Custom LDraw Colours");
        QString const &header = "<b>" + tr ("Colour scheme 'custom' cannot be enabled. Custom LDConfig file not found.") + "</b>";
        QString const &body = tr ("Colour scheme 'custom' selected but no LDConfig file was specified.<br>"
                                  "The default colour scheme '%1' will be used.<br>")
                                  .arg(settings[color_scheme].value);
        showMessage(header, title, body, QString(), MBB_OK, QMessageBox::Warning);
    } else {
        bool change = settings[color_scheme].value != combo->itemText(index);
        change |= settingsModified(false/*update*/);
        emit settingChangedSig(change);
    }
}

bool BlenderPreferences::promptAccept()
{
    QString const &title = tr ("Render Settings Modified");
    QString const &header =  "<b>" + tr("Do you want to accept the modified settings before quitting ?") + "</b>";
    int exec = showMessage(header, title, QString(), QString(), MBB_YES_NO, QMessageBox::Question);
    if (exec == QMessageBox::Yes)
    {
        return true;
    }

    return false;
}

void BlenderPreferences::loadDefaultParameters(QByteArray& buffer, int which)
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
        "sloped_brick,   43708,  64|72\n"
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

    buffer.clear();
    if (which == PARAMS_CUSTOM_COLOURS)
        buffer.append(DefaultCustomColours, sizeof(DefaultCustomColours));
    else if (which == PARAMS_SLOPED_BRICKS)
        buffer.append(DefaultSlopedBricks, sizeof(DefaultSlopedBricks));
    else if (which == PARAMS_LIGHTED_BRICKS)
        buffer.append(DefaultLightedBricks, sizeof(DefaultLightedBricks));
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

    QString const &title = tr ("Replace Existing File");
    QString const header = "<b>" + QMessageBox::tr ("Existing file %1 detected.").arg(fileInfo.fileName()) + "</b>";
    QString const body = QMessageBox::tr ("\"%1\"<br>This file already exists.<br>Replace existing file?").arg(fileInfo.fileName());
    int exec = showMessage(header, title, body, QString(), MBB_YES, QMessageBox::NoIcon);

    return (exec == QMessageBox::Yes);
}

int BlenderPreferences::showMessage(
    QString const &header,
    QString const &title,
    QString const &body,
    const QString &detail,
    int const buttons,
    int const icon)
{
    if (Preferences::loggingEnabled) {
        if (icon == QMessageBox::Critical) {
            logError() << qPrintable(QString(header).replace("<br>","\n"));
        } else if (icon == QMessageBox::Warning) {
            logWarning() << qPrintable(QString(header).replace("<br>","\n"));
        }
    }

    if (!Preferences::modeGUI)
        return QMessageBox::Ok;

    QMessageBox box;
    box.setWindowIcon(QIcon());
    if (!icon) {
        QPixmap _icon = QPixmap(":/icons/lpub96.png");
        box.setIconPixmap (_icon);
    } else {
        box.setIcon (static_cast<QMessageBox::Icon>(icon));
    }
    box.setTextFormat (Qt::RichText);
    box.setWindowFlags (Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
    if (!title.isEmpty()) {
        box.setWindowTitle(title);
    } else {
        box.setWindowTitle(tr("%1 Blender LDraw Addon").arg(VER_PRODUCTNAME_STR));
    }

    box.setText (header);
    if (!body.isEmpty()) {
        box.setInformativeText (body);
    }
    if (!detail.isEmpty()) {
        box.setDetailedText(detail);
        if (icon == QMessageBox::Critical) {
            logError() << qPrintable(QString(detail).replace("<br>","\n"));
        } else if (icon == QMessageBox::Warning) {
            logWarning() << qPrintable(QString(detail).replace("<br>","\n"));
        }
    }
    switch (buttons) {
    case MBB_YES:
        box.setStandardButtons (QMessageBox::Yes | QMessageBox::Cancel);
        box.setDefaultButton   (QMessageBox::Yes);
        break;
    case MBB_YES_NO:
        box.setStandardButtons (QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
        box.setDefaultButton   (QMessageBox::Yes);
        break;
    default:
        box.setStandardButtons (QMessageBox::Ok);
        break;
    }

    int minimumWidth = 400;
    int fontWidth = QFontMetrics(box.font()).averageCharWidth();
    int fixedTextLength = (minimumWidth / fontWidth);
    if (header.length() < body.length() && header.length() < fixedTextLength)
    {
        QGridLayout* boxLayout = (QGridLayout*)box.layout();
        QLayoutItem* boxLayoutItem = boxLayout->itemAtPosition(0, 2);
        QWidget* textWidget = boxLayoutItem->widget();
        if (textWidget)
        {
            int fixedWidth = body.length() * fontWidth;
            if (fixedWidth == minimumWidth)
            {
                int index = (minimumWidth / fontWidth) - 1;
                if (!body.midRef(index, 1).isEmpty())
                    fixedWidth = body.indexOf(" ", index);
            }
            else if (fixedWidth < minimumWidth)
                fixedWidth = minimumWidth;
            textWidget->setFixedWidth(fixedWidth);
        }
    }

    return box.exec();
}
