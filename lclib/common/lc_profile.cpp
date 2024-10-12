#include "lc_global.h"
#include "lc_profile.h"
#include "lc_context.h"
#include "image.h"
#include "lc_model.h"
#include "project.h"
#include "lc_viewsphere.h"

lcProfileEntry::lcProfileEntry(const char* Section, const char* Key, int DefaultValue)
{
	mType = LC_PROFILE_ENTRY_INT;
	mSection = Section;
	mKey = Key;
	mDefault.IntValue = DefaultValue;
}

lcProfileEntry::lcProfileEntry(const char* Section, const char* Key, uint DefaultValue)
{
	mType = LC_PROFILE_ENTRY_INT;
	mSection = Section;
	mKey = Key;
	mDefault.UIntValue = DefaultValue;
}

lcProfileEntry::lcProfileEntry(const char* Section, const char* Key, float DefaultValue)
{
	mType = LC_PROFILE_ENTRY_FLOAT;
	mSection = Section;
	mKey = Key;
	mDefault.FloatValue = DefaultValue;
}

lcProfileEntry::lcProfileEntry(const char* Section, const char* Key, const char* DefaultValue)
{
	mType = LC_PROFILE_ENTRY_STRING;
	mSection = Section;
	mKey = Key;
	mDefault.StringValue = DefaultValue;
}

lcProfileEntry::lcProfileEntry(const char* Section, const char* Key, const QStringList& /*StringList*/)
{
	mType = LC_PROFILE_ENTRY_STRINGLIST;
	mSection = Section;
	mKey = Key;
	mDefault.IntValue = 0;
}

lcProfileEntry::lcProfileEntry(const char* Section, const char* Key)
{
	mType = LC_PROFILE_ENTRY_BUFFER;
	mSection = Section;
	mKey = Key;
	mDefault.IntValue = 0;
}

static lcProfileEntry gProfileEntries[LC_NUM_PROFILE_KEYS] =
{
	lcProfileEntry("Settings", "FixedAxes", false),                                                        // LC_PROFILE_FIXED_AXES
	lcProfileEntry("Settings", "LineWidth", 1.0f),                                                         // LC_PROFILE_LINE_WIDTH
	lcProfileEntry("Settings", "AllowLOD", true),                                                          // LC_PROFILE_ALLOW_LOD
	lcProfileEntry("Settings", "LODDistance", 750.0f),                                                     // LC_PROFILE_LOD_DISTANCE
	lcProfileEntry("Settings", "ViewerFadeSteps", false),                                                  // LC_PROFILE_FADE_STEPS                                 /*** LPub3D Mod - designate viewer setting ***/
	lcProfileEntry("Settings", "ViewerFadeStepsColor", LC_RGBA(128, 128, 128, 128)),                       // LC_PROFILE_FADE_STEPS_COLOR                           /*** LPub3D Mod - designate viewer colour ***/
	lcProfileEntry("Settings", "ViewerHighlightNewParts", false),                                              // LC_PROFILE_HIGHLIGHT_NEW_PARTS                        /*** LPub3D Mod - designate viewer setting ***/
	lcProfileEntry("Settings", "ViewerHighlightNewPartsColor", LC_RGBA(255, 242, 0, 192)),                 // LC_PROFILE_HIGHLIGHT_NEW_PARTS_COLOR                  /*** LPub3D Mod - designate viewer colour ***/
	lcProfileEntry("Settings", "ShadingMode", static_cast<int>(lcShadingMode::DefaultLights)),             // LC_PROFILE_SHADING_MODE
	lcProfileEntry("Settings", "BackgroundGradient", false),                                               // LC_PROFILE_BACKGROUND_GRADIENT
	lcProfileEntry("Settings", "BackgroundColor", LC_RGB(255, 255, 255)),                                  // LC_PROFILE_BACKGROUND_COLOR                           /*** LPub3D Mod - set background colour ***/
	lcProfileEntry("Settings", "GradientColorTop", LC_RGB(54, 72, 95)),                                    // LC_PROFILE_GRADIENT_COLOR_TOP
	lcProfileEntry("Settings", "GradientColorBottom", LC_RGB(49, 52, 55)),                                 // LC_PROFILE_GRADIENT_COLOR_BOTTOM
	lcProfileEntry("Settings", "DrawAxes", 0),                                                             // LC_PROFILE_DRAW_AXES
	lcProfileEntry("Settings", "DrawAxesLocation", static_cast<int>(lcAxisIconLocation::BottomLeft)),      // LC_PROFILE_DRAW_AXES_LOCATION
	lcProfileEntry("Settings", "AxesColor", LC_RGBA(0, 0, 0, 255)),                                        // LC_PROFILE_AXES_COLOR
	lcProfileEntry("Settings", "TextColor", LC_RGBA(0, 0, 0, 255)),                                        // LC_PROFILE_TEXT_COLOR
	lcProfileEntry("Settings", "MarqueeBorderColor", LC_RGBA(64, 64, 255, 255)),                           // LC_PROFILE_MARQUEE_BORDER_COLOR
	lcProfileEntry("Settings", "MarqueeFillColor", LC_RGBA(64, 64, 255, 64)),                              // LC_PROFILE_MARQUEE_FILL_COLOR
	lcProfileEntry("Settings", "OverlayColor", LC_RGBA(0, 0, 0, 255)),                                     // LC_PROFILE_OVERLAY_COLOR
	lcProfileEntry("Settings", "ActiveViewColor", LC_RGBA(255, 0, 0, 255)),                                // LC_PROFILE_ACTIVE_VIEW_COLOR                          /*** LPub3D Mod - Set default colour ***/
	lcProfileEntry("Settings", "InactiveViewColor", LC_RGBA(69, 69, 69, 255)),                             // LC_PROFILE_INACTIVE_VIEW_COLOR
	lcProfileEntry("Settings", "DrawEdgeLines", 1),                                                        // LC_PROFILE_DRAW_EDGE_LINES
	lcProfileEntry("Settings", "DrawConditionalLines", 1),                                                 // LC_PROFILE_DRAW_CONDITIONAL_LINES
	lcProfileEntry("Settings", "GridStuds", 0),                                                            // LC_PROFILE_GRID_STUDS                                 /*** LPub3D Mod - Default settings ***/
	lcProfileEntry("Settings", "GridStudColor", LC_RGBA(64, 64, 64, 192)),                                 // LC_PROFILE_GRID_STUD_COLOR                            /*** LPub3D Mod - Set default colour ***/
	lcProfileEntry("Settings", "GridLines", 0),                                                            // LC_PROFILE_GRID_LINES                                 /*** LPub3D Mod - Default settings ***/
	lcProfileEntry("Settings", "GridLineSpacing", 5),                                                      // LC_PROFILE_GRID_LINE_SPACING
	lcProfileEntry("Settings", "GridLineColor", LC_RGBA(0, 0, 0, 255)),                                    // LC_PROFILE_GRID_LINE_COLOR
	lcProfileEntry("Settings", "GridOrigin", 0),                                                           // LC_PROFILE_GRID_ORIGIN
	lcProfileEntry("Settings", "AASamples", 4),                                                            // LC_PROFILE_ANTIALIASING_SAMPLES                       /*** LPub3D Mod - Set default antialiasing ***/
	lcProfileEntry("Settings", "ViewSphereEnabled", 1),                                                    // LC_PROFILE_VIEW_SPHERE_ENABLED
	lcProfileEntry("Settings", "ViewSphereLocation", (int)lcViewSphereLocation::TopRight),                 // LC_PROFILE_VIEW_SPHERE_LOCATION
	lcProfileEntry("Settings", "ViewSphereSize", 100),                                                     // LC_PROFILE_VIEW_SPHERE_SIZE
	lcProfileEntry("Settings", "ViewSphereColor", LC_RGBA(255, 255, 255, 255)),                            // LC_PROFILE_VIEW_SPHERE_COLOR                          /*** LPub3D Mod - Set default colour ***/
	lcProfileEntry("Settings", "ViewSphereTextColor", LC_RGBA(100, 100, 100, 255)),                        // LC_PROFILE_VIEW_SPHERE_TEXT_COLOR                     /*** LPub3D Mod - Set default colour ***/
	lcProfileEntry("Settings", "ViewSphereHighlightColor", LC_RGBA(255, 0, 0, 255)),                       // LC_PROFILE_VIEW_SPHERE_HIGHLIGHT_COLOR                /*** LPub3D Mod - Set default colour ***/
	lcProfileEntry("Settings", "ObjectSelectedColor", static_cast<uint>(LC_RGBA(229, 76, 102, 255))),      // LC_PROFILE_OBJECT_SELECTED_COLOR
	lcProfileEntry("Settings", "ObjectFocusedColor", static_cast<uint>(LC_RGBA(102, 76, 229, 255))),       // LC_PROFILE_OBJECT_FOCUSED_COLOR
	lcProfileEntry("Settings", "CameraColor", static_cast<uint>(LC_RGBA(128, 204, 128, 255))),             // LC_PROFILE_CAMERA_COLOR
	lcProfileEntry("Settings", "LightColor", static_cast<uint>(LC_RGBA(128, 204, 128, 255))),              // LC_PROFILE_LIGHT_COLOR
	lcProfileEntry("Settings", "ControlPointColor", static_cast<uint>(LC_RGBA(128, 204, 128, 128))),       // LC_PROFILE_CONTROL_POINT_COLOR
	lcProfileEntry("Settings", "ControlPointFocusedColor", static_cast<uint>(LC_RGBA(102, 76, 229, 128))), // LC_PROFILE_CONTROL_POINT_FOCUSED_COLOR

	lcProfileEntry("Settings", "Language", ""),                                                            // LC_PROFILE_LANGUAGE
	lcProfileEntry("Settings", "ColorTheme", static_cast<int>(lcColorTheme::System)),                      // LC_PROFILE_COLOR_THEME                                /*** LPub3D Mod - Set default colour theme to System ***/
	lcProfileEntry("Settings", "CheckUpdates", 1),                                                         // LC_PROFILE_CHECK_UPDATES
	lcProfileEntry("Settings", "ProjectsPath", ""),                                                        // LC_PROFILE_PROJECTS_PATH
	lcProfileEntry("Settings", "PartsLibrary", ""),                                                        // LC_PROFILE_PARTS_LIBRARY
	lcProfileEntry("Settings", "PartPalettes"),                                                            // LC_PROFILE_PART_PALETTES
	lcProfileEntry("Settings", "MinifigSettings", ""),                                                     // LC_PROFILE_MINIFIG_SETTINGS
	lcProfileEntry("Settings", "ColorConfig", ""),                                                         // LC_PROFILE_COLOR_CONFIG
	lcProfileEntry("Settings", "Shortcuts"),                                                               // LC_PROFILE_KEYBOARD_SHORTCUTS
	lcProfileEntry("Settings", "MouseShortcuts", QStringList()),                                           // LC_PROFILE_MOUSE_SHORTCUTS
	lcProfileEntry("Settings", "Categories"),                                                              // LC_PROFILE_CATEGORIES
	lcProfileEntry("Settings", "RecentFile1", ""),                                                         // LC_PROFILE_RECENT_FILE1
	lcProfileEntry("Settings", "RecentFile2", ""),                                                         // LC_PROFILE_RECENT_FILE2
	lcProfileEntry("Settings", "RecentFile3", ""),                                                         // LC_PROFILE_RECENT_FILE3
	lcProfileEntry("Settings", "RecentFile4", ""),                                                         // LC_PROFILE_RECENT_FILE4
	lcProfileEntry("Settings", "AutoLoadMostRecent", false),                                               // LC_PROFILE_AUTOLOAD_MOSTRECENT
	lcProfileEntry("Settings", "RestoreTabLayout", false),                                                 // LC_PROFILE_RESTORE_TAB_LAYOUT                         /*** LPub3D Mod - preview widget for LPub3D ***/
	lcProfileEntry("Settings", "AutosaveInterval", 10),                                                    // LC_PROFILE_AUTOSAVE_INTERVAL
	lcProfileEntry("Settings", "MouseSensitivity", 11),                                                    // LC_PROFILE_MOUSE_SENSITIVITY
	lcProfileEntry("Settings", "ImageWidth", 1280),                                                        // LC_PROFILE_IMAGE_WIDTH
	lcProfileEntry("Settings", "ImageHeight", 720),                                                        // LC_PROFILE_IMAGE_HEIGHT
	lcProfileEntry("Settings", "ImageExtension", ".png"),                                                  // LC_PROFILE_IMAGE_EXTENSION
	lcProfileEntry("Settings", "PartsListIcons", 64),                                                      // LC_PROFILE_PARTS_LIST_ICONS
	lcProfileEntry("Settings", "PartsListNames", 0),                                                       // LC_PROFILE_PARTS_LIST_NAMES
	lcProfileEntry("Settings", "PartsListFixedColor", -1),                                                 // LC_PROFILE_PARTS_LIST_FIXED_COLOR
	lcProfileEntry("Settings", "PartsListDecorated", 1),                                                   // LC_PROFILE_PARTS_LIST_DECORATED
	lcProfileEntry("Settings", "PartsListAliases", 1),                                                     // LC_PROFILE_PARTS_LIST_ALIASES
	lcProfileEntry("Settings", "PartsListListMode", 0),                                                    // LC_PROFILE_PARTS_LIST_LISTMODE
	lcProfileEntry("Settings", "StudStyle", 0),                                                            // LC_PROFILE_STUD_STYLE

	lcProfileEntry("Defaults", "Author", ""),                                                              // LC_PROFILE_DEFAULT_AUTHOR_NAME
	lcProfileEntry("Defaults", "AmbientColor", LC_RGB(75, 75, 75)),                                        // LC_PROFILE_DEFAULT_AMBIENT_COLOR

	lcProfileEntry("HTML", "Options", LC_HTML_SINGLEPAGE),                                                 // LC_PROFILE_HTML_OPTIONS
	lcProfileEntry("HTML", "ImageOptions", LC_IMAGE_TRANSPARENT),                                          // LC_PROFILE_HTML_IMAGE_OPTIONS
	lcProfileEntry("HTML", "ImageWidth", 640),                                                             // LC_PROFILE_HTML_IMAGE_WIDTH
	lcProfileEntry("HTML", "ImageHeight", 480),                                                            // LC_PROFILE_HTML_IMAGE_HEIGHT

	lcProfileEntry("POVRay", "POVRayPath", ""),                                                            // LC_PROFILE_POVRAY_PATH                                /*** LPub3D Mod - POV-ray remove default path ***/
	lcProfileEntry("POVRay", "LGEOPath", ""),                                                              // LC_PROFILE_POVRAY_LGEO_PATH
	
	lcProfileEntry("Render", "Width", 1280),                                                               // LC_PROFILE_RENDER_WIDTH
	lcProfileEntry("Render", "Height", 720),                                                               // LC_PROFILE_RENDER_HEIGHT

	lcProfileEntry("Blender", "Path", ""),                                                                 // LC_PROFILE_BLENDER_PATH
	lcProfileEntry("Blender", "LDrawConfigPath", ""),                                                      // LC_PROFILE_BLENDER_LDRAW_CONFIG_PATH
	lcProfileEntry("Blender", "Version", ""),                                                              // LC_PROFILE_BLENDER_VERSION
	lcProfileEntry("Blender", "AddonVersion", ""),                                                         // LC_PROFILE_BLENDER_ADDON_VERSION
	lcProfileEntry("Blender", "AddonVersionCheck", 1),                                                     // LC_PROFILE_BLENDER_ADDON_VERSION_CHECK
	lcProfileEntry("Blender", "ImportModule", ""),                                                         // LC_PROFILE_BLENDER_IMPORT_MODULE

	lcProfileEntry("Settgins", "PreviewViewSphereEnabled", 1),                                             // LC_PROFILE_PREVIEW_VIEW_SPHERE_ENABLED                /*** LPub3D Mod - preview widget for LPub3D ***/
	lcProfileEntry("Settings", "PreviewViewSphereSize", 75),                                               // LC_PROFILE_PREVIEW_VIEW_SPHERE_SIZE
	lcProfileEntry("Settings", "PreviewViewSphereLocation", (int)lcViewSphereLocation::TopRight),          // LC_PROFILE_PREVIEW_VIEW_SPHERE_LOCATION
	lcProfileEntry("Settings", "DrawPreviewAxis", 0),                                                      // LC_PROFILE_PREVIEW_DRAW_AXES

	lcProfileEntry("Settings", "StudCylinderColorEnabled", true),                                          // LC_PROFILE_STUD_CYLINDER_COLOR_ENABLED
	lcProfileEntry("Settings", "StudCylinderColor", LC_RGBA(27, 42, 52, 255)),                             // LC_PROFILE_STUD_CYLINDER_COLOR
	lcProfileEntry("Settings", "PartEdgeColorEnabled", true),                                              // LC_PROFILE_PART_EDGE_COLOR_ENABLED
	lcProfileEntry("Settings", "PartEdgeColor", LC_RGBA(0, 0, 0, 255)),                                    // LC_PROFILE_PART_EDGE_COLOR
	lcProfileEntry("Settings", "BlackEdgeColorEnabled", true),                                             // LC_PROFILE_BLACK_EDGE_COLOR_ENABLED
	lcProfileEntry("Settings", "BlackEdgeColor", LC_RGBA(255, 255, 255, 255)),                             // LC_PROFILE_BLACK_EDGE_COLOR
	lcProfileEntry("Settings", "DarkEdgeColorEnabled", true),                                              // LC_PROFILE_DARK_EDGE_COLOR_ENABLED
	lcProfileEntry("Settings", "DarkEdgeColor", LC_RGBA(27, 42, 52, 255)),                                 // LC_PROFILE_DARK_EDGE_COLOR
	lcProfileEntry("Settings", "PartEdgeContrast", 0.5f),                                                  // LC_PROFILE_PART_EDGE_CONTRAST
	lcProfileEntry("Settings", "PartColorValueLDIndex", 0.5f),                                             // LC_PROFILE_PART_COLOR_VALUE_LD_INDEX
	lcProfileEntry("Settings", "AutomateEdgeColor", 0),                                                    // LC_PROFILE_AUTOMATE_EDGE_COLOR

/*** LPub3D Mod - Zoom extents ***/
	lcProfileEntry("Settings", "ZoomExtents", 0),                                                          // LC_PROFILE_ZOOM_EXTENTS                               /*** LPub3D Mod - Zoom extents ***/
/*** LPub3D Mod - ***/

/*** LPub3D Mod - preview widget for LPub3D ***/
	lcProfileEntry("Settings", "PreviewEnabled", 1),                                                       // LC_PROFILE_PREVIEW_ENABLED                            /*** LPub3D Mod - preview widget for LPub3D ***/
	lcProfileEntry("Settings", "PreviewSize", 300),                                                        // LC_PROFILE_PREVIEW_SIZE                               /*** LPub3D Mod - preview widget for LPub3D ***/
	lcProfileEntry("Settings", "PreviewLocation", (int)lcPreviewLocation::BottomRight),                    // LC_PROFILE_PREVIEW_LOCATION                           /*** LPub3D Mod - preview widget for LPub3D ***/
	lcProfileEntry("Settings", "PreviewPosition", (int)lcPreviewPosition::Dockable),                       // LC_PROFILE_PREVIEW_POSITION	                        /*** LPub3D Mod - preview widget for LPub3D ***/
	lcProfileEntry("Settings", "PreviewLoadPath", ""),                                                     // LC_PROFILE_PREVIEW_LOAD_PATH                          /*** LPub3D Mod - preview widget for LPub3D ***/
/*** LPub3D Mod end ***/

/*** LPub3D Mod - Build mod object selected colour ***/
	lcProfileEntry("Settings", "BMObjectSelectedColor", static_cast<uint>(LC_RGBA(121, 216, 121, 255))),   // LC_PROFILE_BM_OBJECT_SELECTED_COLOR                   /*** LPub3D Mod - Build mod object selected colour ***/
/*** LPub3D Mod - ***/

/*** LPub3D Mod - Camera Globe Target Position ***/
	lcProfileEntry("Settings", "UseImageSize", 1),                                                         // LC_PROFILE_USE_IMAGE_SIZE                             /*** LPub3D Mod - Camera Globe Target Position ***/
	lcProfileEntry("Settings", "AutoCenterSelection", 1),                                                  // LC_PROFILE_AUTO_CENTER_SELECTION                      /*** LPub3D Mod - Camera Globe Target Position ***/
/*** LPub3D Mod end ***/

/*** LPub3D Mod - Update Default Camera ***/
	lcProfileEntry("Settings", "DefaultCameraProperties", 1),                                              //LC_PROFILE_DEFAULT_CAMERA_PROPERTIES                   /*** LPub3D Mod - Update Default Camera ***/
	lcProfileEntry("Settings", "DefaultDistanceFactor", 8.0f),                                             //LC_PROFILE_DEFAULT_DISTANCE_FACTOR                     /*** LPub3D Mod - Update Default Camera ***/
	lcProfileEntry("Settings", "DefaultCameraPosition", 1250.0f),                                          //LC_PROFILE_CAMERA_DEFAULT_POSITION                     /*** LPub3D Mod - Update Default Camera ***/
	lcProfileEntry("Settings", "DefaultCameraFieldOfView", 30.0f),                                         //LC_PROFILE_CAMERA_FOV                                  /*** LPub3D Mod - Update Default Camera ***/
	lcProfileEntry("Settings", "DefaultCameraNearPlane", 25.0f),                                           //LC_PROFILE_CAMERA_NEAR_PLANE                           /*** LPub3D Mod - Update Default Camera ***/
	lcProfileEntry("Settings", "DefaultCameraFarPlane", 50000.0f),                                         //LC_PROFILE_CAMERA_FAR_PLANE                            /*** LPub3D Mod - Update Default Camera ***/
/*** LPub3D Mod end ***/

/*** LPub3D Mod - Native projection options ***/
	lcProfileEntry("Settings", "NativeViewpoint",  8),                                                     // LC_PROFILE_NATIVE_VIEWPOINT  [0 = LC_VIEWPOINT_FRONT] /*** LPub3D Mod - Native Renderer settings ***/
	lcProfileEntry("Settings", "NativeProjection", 0),                                                     // LC_PROFILE_NATIVE_PROJECTION [0 = PERSPECTIVE, 1 = ORTHOGRAPHIC]  /*** LPub3D Mod - Native Renderer settings ***/
/*** LPub3D Mod end ***/

/*** LPub3D Mod - piece icons ***/
	lcProfileEntry("Settings", "ViewPieceIcons", 0),                                                       // LC_PROFILE_VIEW_PIECE_ICONS                          /*** LPub3D Mod - View piece icons ***/
/*** LPub3D Mod end ***/

/*** LPub3D Mod - Viewpoint zoom extent ***/
	lcProfileEntry("Settings", "ViewpointZoomExtent", 1),                                                  // LC_PROFILE_VIEWPOINT_ZOOM_EXTENT                     /*** LPub3D Mod - View point zoom extent ***/
/*** LPub3D Mod end ***/

/*** LPub3D Mod - lpub fade highlight ***/
	lcProfileEntry("Settings", "LPubFadeHighlight", 0),                                                    // LC_PROFILE_LPUB_FADE_HIGHLIGHT                       /*** LPub3D Mod - lpub fade highlight ***/
/*** LPub3D Mod end ***/

/*** LPub3D Mod - parts load order ***/
	lcProfileEntry("Settings", "PreferOfficialParts", 1),                                                  // LC_PROFILE_PREFER_OFFICIAL_PARTS                     /*** LPub3D Mod - parts load order ***/
	lcProfileEntry("Settings", "UpdateCacheIndex", 0),                                                     // LC_PROFILE_UPDATE_CACHE_INDEX                        /*** LPub3D Mod - parts load order ***/
/*** LPub3D Mod - ***/
};

void lcRemoveProfileKey(LC_PROFILE_KEY Key)
{
	lcProfileEntry& Entry = gProfileEntries[Key];
	QSettings Settings;

	Settings.remove(QString("%1/%2").arg(Entry.mSection, Entry.mKey));
}

int lcGetProfileInt(LC_PROFILE_KEY Key)
{
	lcProfileEntry& Entry = gProfileEntries[Key];
	QSettings Settings;

	return Settings.value(QString("%1/%2").arg(Entry.mSection, Entry.mKey), Entry.mDefault.IntValue).toInt();
}

uint lcGetProfileUInt(LC_PROFILE_KEY Key)
{
	lcProfileEntry& Entry = gProfileEntries[Key];
	QSettings Settings;

	return Settings.value(QString("%1/%2").arg(Entry.mSection, Entry.mKey), Entry.mDefault.UIntValue).toUInt();
}

float lcGetProfileFloat(LC_PROFILE_KEY Key)
{
	lcProfileEntry& Entry = gProfileEntries[Key];
	QSettings Settings;

	return Settings.value(QString("%1/%2").arg(Entry.mSection, Entry.mKey), Entry.mDefault.FloatValue).toFloat();
}

QString lcGetProfileString(LC_PROFILE_KEY Key)
{
	lcProfileEntry& Entry = gProfileEntries[Key];
	QSettings Settings;

	return Settings.value(QString("%1/%2").arg(Entry.mSection, Entry.mKey), Entry.mDefault.StringValue).toString();
}

QStringList lcGetProfileStringList(LC_PROFILE_KEY Key)
{
	lcProfileEntry& Entry = gProfileEntries[Key];
	QSettings Settings;

	return Settings.value(QString("%1/%2").arg(Entry.mSection, Entry.mKey), QStringList()).toStringList();
}

QByteArray lcGetProfileBuffer(LC_PROFILE_KEY Key)
{
	lcProfileEntry& Entry = gProfileEntries[Key];
	QSettings Settings;

	return Settings.value(QString("%1/%2").arg(Entry.mSection, Entry.mKey)).toByteArray();
}

void lcSetProfileInt(LC_PROFILE_KEY Key, int Value)
{
	lcProfileEntry& Entry = gProfileEntries[Key];
	QSettings Settings;

	Settings.setValue(QString("%1/%2").arg(Entry.mSection, Entry.mKey), Value);
}

void lcSetProfileUInt(LC_PROFILE_KEY Key, uint Value)
{
	lcProfileEntry& Entry = gProfileEntries[Key];
	QSettings Settings;

	Settings.setValue(QString("%1/%2").arg(Entry.mSection, Entry.mKey), Value);
}

void lcSetProfileFloat(LC_PROFILE_KEY Key, float Value)
{
	lcProfileEntry& Entry = gProfileEntries[Key];
	QSettings Settings;

	Settings.setValue(QString("%1/%2").arg(Entry.mSection, Entry.mKey), Value);
}

void lcSetProfileString(LC_PROFILE_KEY Key, const QString& Value)
{
	lcProfileEntry& Entry = gProfileEntries[Key];
	QSettings Settings;

	Settings.setValue(QString("%1/%2").arg(Entry.mSection, Entry.mKey), Value);
}

void lcSetProfileStringList(LC_PROFILE_KEY Key, const QStringList& Value)
{
	lcProfileEntry& Entry = gProfileEntries[Key];
	QSettings Settings;

	Settings.setValue(QString("%1/%2").arg(Entry.mSection, Entry.mKey), Value);
}

void lcSetProfileBuffer(LC_PROFILE_KEY Key, const QByteArray& Buffer)
{
	lcProfileEntry& Entry = gProfileEntries[Key];
	QSettings Settings;

	Settings.setValue(QString("%1/%2").arg(Entry.mSection, Entry.mKey), Buffer);
}
