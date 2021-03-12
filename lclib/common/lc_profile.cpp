#include "lc_global.h"
#include "lc_profile.h"
#include "lc_context.h"
#include "image.h"
#include "lc_model.h"
#include "project.h"
#include "lc_viewsphere.h"
/*** LPub3D Mod - includes ***/
#include "lpub_preferences.h"
/*** LPub3D Mod end ***/

lcProfileEntry::lcProfileEntry(const char* Section, const char* Key, int DefaultValue)
{
	mType = LC_PROFILE_ENTRY_INT;
	mSection = Section;
	mKey = Key;
	mDefault.IntValue = DefaultValue;
}

lcProfileEntry::lcProfileEntry(const char* Section, const char* Key, unsigned int DefaultValue)
{
	mType = LC_PROFILE_ENTRY_INT;
	mSection = Section;
	mKey = Key;
	mDefault.IntValue = DefaultValue;
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
	lcProfileEntry("Settings", "FixedAxes", false),                                            // LC_PROFILE_FIXED_AXES
	lcProfileEntry("Settings", "LineWidth", 1.0f),                                             // LC_PROFILE_LINE_WIDTH
	lcProfileEntry("Settings", "AllowLOD", true),                                              // LC_PROFILE_ALLOW_LOD
	lcProfileEntry("Settings", "LODDistance", 750.0f),                                         // LC_PROFILE_LOD_DISTANCE
	lcProfileEntry("Settings", "FadeSteps", false),                                            // LC_PROFILE_FADE_STEPS
	lcProfileEntry("Settings", "FadeStepsColor", LC_RGBA(128, 128, 128, 128)),                 // LC_PROFILE_FADE_STEPS_COLOR
	lcProfileEntry("Settings", "HighlightNewParts", 0),                                        // LC_PROFILE_HIGHLIGHT_NEW_PARTS
	lcProfileEntry("Settings", "HighlightNewPartsColor", LC_RGBA(255, 242, 0, 192)),           // LC_PROFILE_HIGHLIGHT_NEW_PARTS_COLOR
	lcProfileEntry("Settings", "ShadingMode", static_cast<int>(lcShadingMode::DefaultLights)), // LC_PROFILE_SHADING_MODE
	lcProfileEntry("Settings", "BackgroundGradient", false),                                   // LC_PROFILE_BACKGROUND_GRADIENT
	lcProfileEntry("Settings", "BackgroundColor", LC_RGB(255, 255, 255)),                      // LC_PROFILE_BACKGROUND_COLOR                           /*** LPub3D Mod - set background colour ***/
	lcProfileEntry("Settings", "GradientColorTop", LC_RGB(54, 72, 95)),                        // LC_PROFILE_GRADIENT_COLOR_TOP
	lcProfileEntry("Settings", "GradientColorBottom", LC_RGB(49, 52, 55)),                     // LC_PROFILE_GRADIENT_COLOR_BOTTOM
	lcProfileEntry("Settings", "DrawAxes", 0),                                                 // LC_PROFILE_DRAW_AXES
	lcProfileEntry("Settings", "AxesColor", LC_RGBA(0, 0, 0, 255)),                            // LC_PROFILE_AXES_COLOR
	lcProfileEntry("Settings", "OverlayColor", LC_RGBA(0, 0, 0, 255)),                         // LC_PROFILE_OVERLAY_COLOR
	lcProfileEntry("Settings", "ActiveViewColor", LC_RGBA(255, 0, 0, 255)),                    // LC_PROFILE_ACTIVE_VIEW_COLOR                          /*** LPub3D Mod - Set default colour ***/
	lcProfileEntry("Settings", "InactiveViewColor", LC_RGBA(69, 69, 69, 255)),                 // LC_PROFILE_INACTIVE_VIEW_COLOR
	lcProfileEntry("Settings", "DrawEdgeLines", 1),                                            // LC_PROFILE_DRAW_EDGE_LINES
	lcProfileEntry("Settings", "GridStuds", 1),                                                // LC_PROFILE_GRID_STUDS
	lcProfileEntry("Settings", "GridStudColor", LC_RGBA(64, 64, 64, 192)),                     // LC_PROFILE_GRID_STUD_COLOR                            /*** LPub3D Mod - Set default colour ***/
	lcProfileEntry("Settings", "GridLines", 1),                                                // LC_PROFILE_GRID_LINES
	lcProfileEntry("Settings", "GridLineSpacing", 5),                                          // LC_PROFILE_GRID_LINE_SPACING
	lcProfileEntry("Settings", "GridLineColor", LC_RGBA(0, 0, 0, 255)),                        // LC_PROFILE_GRID_LINE_COLOR
	lcProfileEntry("Settings", "AASamples", 4),                                                // LC_PROFILE_ANTIALIASING_SAMPLES                       /*** LPub3D Mod - Set default antialiasing ***/
	lcProfileEntry("Settings", "ViewSphereEnabled", 1),                                        // LC_PROFILE_VIEW_SPHERE_ENABLED
	lcProfileEntry("Settings", "ViewSphereLocation", (int)lcViewSphereLocation::TopRight),     // LC_PROFILE_VIEW_SPHERE_LOCATION
	lcProfileEntry("Settings", "ViewSphereSize", 100),                                         // LC_PROFILE_VIEW_SPHERE_SIZE
	lcProfileEntry("Settings", "ViewSphereColor", LC_RGBA(255, 255, 255, 255)),                // LC_PROFILE_VIEW_SPHERE_COLOR                          /*** LPub3D Mod - Set default colour ***/
	lcProfileEntry("Settings", "ViewSphereTextColor", LC_RGBA(100, 100, 100, 255)),            // LC_PROFILE_VIEW_SPHERE_TEXT_COLOR                     /*** LPub3D Mod - Set default colour ***/
	lcProfileEntry("Settings", "ViewSphereHighlightColor", LC_RGBA(255, 0, 0, 255)),           // LC_PROFILE_VIEW_SPHERE_HIGHLIGHT_COLOR                /*** LPub3D Mod - Set default colour ***/

	lcProfileEntry("Settings", "Language", ""),                                                // LC_PROFILE_LANGUAGE
	lcProfileEntry("Settings", "ColorTheme", static_cast<int>(lcColorTheme::System)),          // LC_PROFILE_COLOR_THEME                                /*** LPub3D Mod - Set default colour theme to System ***/
	lcProfileEntry("Settings", "CheckUpdates", 1),                                             // LC_PROFILE_CHECK_UPDATES
	lcProfileEntry("Settings", "ProjectsPath", ""),                                            // LC_PROFILE_PROJECTS_PATH
	lcProfileEntry("Settings", "PartsLibrary", ""),                                            // LC_PROFILE_PARTS_LIBRARY
	lcProfileEntry("Settings", "PartPalettes"),                                                // LC_PROFILE_PART_PALETTES
	lcProfileEntry("Settings", "MinifigSettings", ""),                                         // LC_PROFILE_MINIFIG_SETTINGS
	lcProfileEntry("Settings", "ColorConfig", ""),                                             // LC_PROFILE_COLOR_CONFIG
	lcProfileEntry("Settings", "Shortcuts"),                                                   // LC_PROFILE_KEYBOARD_SHORTCUTS
	lcProfileEntry("Settings", "MouseShortcuts", QStringList()),                               // LC_PROFILE_MOUSE_SHORTCUTS
	lcProfileEntry("Settings", "Categories"),                                                  // LC_PROFILE_CATEGORIES
	lcProfileEntry("Settings", "RecentFile1", ""),                                             // LC_PROFILE_RECENT_FILE1
	lcProfileEntry("Settings", "RecentFile2", ""),                                             // LC_PROFILE_RECENT_FILE2
	lcProfileEntry("Settings", "RecentFile3", ""),                                             // LC_PROFILE_RECENT_FILE3
	lcProfileEntry("Settings", "RecentFile4", ""),                                             // LC_PROFILE_RECENT_FILE4
	lcProfileEntry("Settings", "AutoLoadMostRecent", false),                                   // LC_PROFILE_AUTOLOAD_MOSTRECENT
	lcProfileEntry("Settings", "RestoreTabLayout", true),                                      // LC_PROFILE_RESTORE_TAB_LAYOUT
	lcProfileEntry("Settings", "AutosaveInterval", 10),                                        // LC_PROFILE_AUTOSAVE_INTERVAL
	lcProfileEntry("Settings", "MouseSensitivity", 11),                                        // LC_PROFILE_MOUSE_SENSITIVITY
	lcProfileEntry("Settings", "ImageWidth", 1280),                                            // LC_PROFILE_IMAGE_WIDTH
	lcProfileEntry("Settings", "ImageHeight", 720),                                            // LC_PROFILE_IMAGE_HEIGHT
	lcProfileEntry("Settings", "ImageExtension", ".png"),                                      // LC_PROFILE_IMAGE_EXTENSION
	lcProfileEntry("Settings", "PartsListIcons", 64),                                          // LC_PROFILE_PARTS_LIST_ICONS
	lcProfileEntry("Settings", "PartsListNames", 0),                                           // LC_PROFILE_PARTS_LIST_NAMES
	lcProfileEntry("Settings", "PartsListFixedColor", -1),                                     // LC_PROFILE_PARTS_LIST_FIXED_COLOR
	lcProfileEntry("Settings", "PartsListDecorated", 1),                                       // LC_PROFILE_PARTS_LIST_DECORATED
	lcProfileEntry("Settings", "PartsListAliases", 1),                                         // LC_PROFILE_PARTS_LIST_ALIASES
	lcProfileEntry("Settings", "PartsListListMode", 0),                                        // LC_PROFILE_PARTS_LIST_LISTMODE
	lcProfileEntry("Settings", "StudLogo", 0),                                                 // LC_PROFILE_STUD_LOGO

	lcProfileEntry("Defaults", "Author", ""),                                                  // LC_PROFILE_DEFAULT_AUTHOR_NAME
	lcProfileEntry("Defaults", "AmbientColor", LC_RGB(75, 75, 75)),                            // LC_PROFILE_DEFAULT_AMBIENT_COLOR

	lcProfileEntry("HTML", "Options", LC_HTML_SINGLEPAGE),                                     // LC_PROFILE_HTML_OPTIONS
	lcProfileEntry("HTML", "ImageOptions", LC_IMAGE_TRANSPARENT),                              // LC_PROFILE_HTML_IMAGE_OPTIONS
	lcProfileEntry("HTML", "ImageWidth", 640),                                                 // LC_PROFILE_HTML_IMAGE_WIDTH
	lcProfileEntry("HTML", "ImageHeight", 480),                                                // LC_PROFILE_HTML_IMAGE_HEIGHT

/*** LPub3D Mod - modified POVRay path ***/
	lcProfileEntry("POVRay", "POVRayPath", ""),                                                // LC_PROFILE_POVRAY_PATH                                /*** LPub3D Mod - POV-ray remove default path ***/
/*** LPub3D Mod end ***/

	lcProfileEntry("POVRay", "LGEOPath", ""),                                                  // LC_PROFILE_POVRAY_LGEO_PATH
	lcProfileEntry("POVRay", "Width", 1280),                                                   // LC_PROFILE_POVRAY_WIDTH
	lcProfileEntry("POVRay", "Height", 720),                                                   // LC_PROFILE_POVRAY_HEIGHT

	lcProfileEntry("Settgins", "PreviewViewSphereEnabled", 1),                                    // LC_PROFILE_PREVIEW_VIEW_SPHERE_ENABLED
	lcProfileEntry("Settings", "PreviewViewSphereSize", 75),                                      // LC_PROFILE_PREVIEW_VIEW_SPHERE_SIZE
	lcProfileEntry("Settings", "PreviewViewSphereLocation", (int)lcViewSphereLocation::TopRight), // LC_PROFILE_PREVIEW_VIEW_SPHERE_LOCATION
	lcProfileEntry("Settings", "PreviewEnabled", 1),                                              // LC_PROFILE_PREVIEW_ENABLED
	lcProfileEntry("Settings", "PreviewSize", 300),                                               // LC_PROFILE_PREVIEW_SIZE
	lcProfileEntry("Settings", "PreviewLocation", (int)lcPreviewLocation::BottomRight),           // LC_PROFILE_PREVIEW_LOCATION
	lcProfileEntry("Settings", "PreviewPosition", (int)lcPreviewPosition::Dockable),              // LC_PROFILE_PREVIEW_POSITION
	lcProfileEntry("Settings", "DrawPreviewAxis", 0),                                             // LC_PROFILE_PREVIEW_DRAW_AXES
/*** LPub3D Mod - preview widget for LPub3D ***/	
	lcProfileEntry("Settings", "PreviewLoadPath", ""),                                            // LC_PROFILE_PREVIEW_LOAD_PATH                       /*** LPub3D Mod - preview widget for LPub3D ***/
/*** LPub3D Mod end ***/

/*** LPub3D Mod - Build Modification ***/
	lcProfileEntry("Settings", "BuildModification", 0),                                        // LC_PROFILE_BUILD_MODIFICATION                         /*** LPub3D Mod - Build Modification ***/
/*** LPub3D Mod end ***/

/*** LPub3D Mod - Camera Globe Target Position ***/
	lcProfileEntry("Settings", "UseImageSize", 1),                                             // LC_PROFILE_USE_IMAGE_SIZE                             /*** LPub3D Mod - Camera Globe Target Position ***/
	lcProfileEntry("Settings", "AutoCenterSelection", 1),                                      // LC_PROFILE_AUTO_CENTER_SELECTION                      /*** LPub3D Mod - Camera Globe Target Position ***/
/*** LPub3D Mod end ***/

/*** LPub3D Mod - Update Default Camera ***/
	lcProfileEntry("Settings", "DefaultCameraProperties", 1),                                  //LC_PROFILE_DEFAULT_CAMERA_PROPERTIES                   /*** LPub3D Mod - Update Default Camera ***/
	lcProfileEntry("Settings", "DefaultDistanceFactor", 10.0f),                                //LC_PROFILE_DEFAULT_DISTANCE_FACTOR                     /*** LPub3D Mod - Update Default Camera ***/
	lcProfileEntry("Settings", "DefaultCameraPosition", 1250.0f),                              //LC_PROFILE_CAMERA_DEFAULT_POSITION                     /*** LPub3D Mod - Update Default Camera ***/
	lcProfileEntry("Settings", "DefaultCameraFieldOfView", 30.0f),                             //LC_PROFILE_CAMERA_FOV                                  /*** LPub3D Mod - Update Default Camera ***/
	lcProfileEntry("Settings", "DefaultCameraNearPlane", 25.0f),                               //LC_PROFILE_CAMERA_NEAR_PLANE                           /*** LPub3D Mod - Update Default Camera ***/
	lcProfileEntry("Settings", "DefaultCameraFarPlane", 50000.0f),                             //LC_PROFILE_CAMERA_FAR_PLANE                            /*** LPub3D Mod - Update Default Camera ***/
/*** LPub3D Mod end ***/

/*** LPub3D Mod - Native projection options ***/
	lcProfileEntry("Settings", "NativeViewpoint",  7),                                         // LC_PROFILE_NATIVE_VIEWPOINT  [0 = LC_VIEWPOINT_FRONT] /*** LPub3D Mod - Native Renderer settings ***/
	lcProfileEntry("Settings", "NativeProjection", 0),                                         // LC_PROFILE_NATIVE_PROJECTION [0 = PERSPECTIVE, 1 = ORTHOGRAPHIC]  /*** LPub3D Mod - Native Renderer settings ***/
/*** LPub3D Mod end ***/

/*** LPub3D Mod - piece icons ***/
	lcProfileEntry("Settings", "ViewPieceIcons", 0),                                           // LC_PROFILE_VIEW_PIECE_ICONS                          /*** LPub3D Mod - View piece icons ***/
/*** LPub3D Mod end ***/

/*** LPub3D Mod - Viewpoint zoom extent ***/
	lcProfileEntry("Settings", "ViewpointZoomExtent", 1),                                      // LC_PROFILE_VIEWPOINT_ZOOM_EXTENT                     /*** LPub3D Mod - View point zoom extent ***/
/*** LPub3D Mod end ***/

/*** LPub3D Mod - true fade ***/
	lcProfileEntry("Settings", "LPubTrueFade", 1),                                             // LC_PROFILE_LPUB_TRUE_FADE                            /*** LPub3D Mod - true fade ***/
	lcProfileEntry("Settings", "ConditionalLines", 0),                                         // LC_PROFILE_CONDITIONAL_LINES                         /*** LPub3D Mod - true fade ***/
/*** LPub3D Mod end ***/

};

void lcRemoveProfileKey(LC_PROFILE_KEY Key)
{
	lcProfileEntry& Entry = gProfileEntries[Key];
	QSettings Settings;

	Settings.remove(QString("%1/%2").arg(Entry.mSection, Entry.mKey));
}

int lcGetDefaultProfileInt(LC_PROFILE_KEY Key)
{
	return gProfileEntries[Key].mDefault.IntValue;
}

float lcGetDefaultProfileFloat(LC_PROFILE_KEY Key)
{
	return gProfileEntries[Key].mDefault.FloatValue;
}

QString lcGetDefaultProfileString(LC_PROFILE_KEY Key)
{
	return QString::fromLatin1(gProfileEntries[Key].mDefault.StringValue);
}

int lcGetProfileInt(LC_PROFILE_KEY Key)
{
	lcProfileEntry& Entry = gProfileEntries[Key];
	QSettings Settings;

	return Settings.value(QString("%1/%2").arg(Entry.mSection, Entry.mKey), Entry.mDefault.IntValue).toInt();
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
