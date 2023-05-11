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

#ifndef BLENDERPREFERENCES_H
#define BLENDERPREFERENCES_H

#include "qtimer.h"
#include <QWidget>
#include <QDialog>

class QFormLayout;
class QGridLayout;
class QLabel;
class QGroupBox;
class QLineEdit;
class QCheckBox;
class QPushButton;
class QComboBox;
class QScrollArea;
class QProgressBar;
class QProcess;
class QTimer;

class BlenderPreferences;
class BlenderPreferencesDialog : public QDialog
{
    Q_OBJECT
public:

    explicit BlenderPreferencesDialog(int      width,
                                      int      height,
                                      double   renderPercentage,
                                      bool     docRender,
                                      QWidget *parent);

    virtual ~BlenderPreferencesDialog();

    static bool getBlenderPreferences(
        int     &width,
        int     &height,
        double  &renderPercentage,
        bool     docRender,
        QWidget *parent = nullptr);

public slots:
    void showPathsGroup();
    void resetSettings();
    void accept() override;
    void reject() override;

private:
    QPushButton *mPathsButton;
    BlenderPreferences *mPreferences;
};

class BlenderPreferences : public QWidget
{
    Q_OBJECT
public:

    explicit BlenderPreferences(int      width,
                                int      height,
                                double   renderPercentage,
                                bool     docRender,
                                QWidget *parent = nullptr);

    ~BlenderPreferences();

    static void loadSettings();
    static void saveSettings();
    static bool exportParameterFile();
    static bool settingsModified(const QString &module = "");

    void apply(const int response = QDialog::Accepted);

    int     mWidth;
    int     mHeight;
    qreal   mRenderPercentage;

protected:
    static int numPaths(bool = false);
    static int numSettings(bool = false);
    static int numSettingsMM(bool = false);
    static bool getBlenderAddon(const QString &);
    static bool extractBlenderAddon(const QString &);
    static void loadDefaultParameters(QByteArray& Buffer, int Which);
    static bool overwriteFile(const QString &file);
    static bool settingsModified(int &width, int &height, double &renderPercentage, QString const &module = "");

    QString readStdErr(bool &hasError) const;
    void clearGroupBox(QGroupBox *groupBox);
    void initPathsAndSettings();
    void initPathsAndSettingsMM();

public slots:
    void resetSettings();
    void showPathsGroup();
    bool promptCancel();

private slots:
    void browseBlender(bool);
    void configureBlenderAddon(bool = false);
    void updateBlenderAddon();
    void enableImportModule();
    void sizeChanged(const QString &);
    void setDefaultColor(int value) const;
    void colorButtonClicked(bool);
    void setModelSize(bool);
    void validateColourScheme(QString const &);
    bool promptAccept();
    void update();
    void readStdOut();
    void writeStdOut();
    void getStandardOutput();
    void showResult();
    void statusUpdate(bool addon, bool error = true, const QString &message = QString());

private:
    QWidget     *mContent;

    QFormLayout *mForm;
    QGridLayout *mExeGridLayout;
    QGridLayout *mAddonGridLayout;
    QGridLayout *mPathsGridLayout;
    QFormLayout *mSettingsSubform;

    QLineEdit   *mBlenderVersionEdit;
    QLabel      *mBlenderVersionLabel;
    QLineEdit   *mAddonVersionEdit;
    QLabel      *mAddonVersionLabel;
    QGroupBox   *mModulesBox;
    QCheckBox   *mImportActBox;
    QCheckBox   *mImportMMActBox;
    QCheckBox   *mRenderActBox;
    QGroupBox   *mSettingsBox;
    QGroupBox   *mPathsBox;

    QProgressBar *mProgressBar;
    QProcess     *mProcess;
    QTimer        mUpdateTimer;

    QStringList  mStdOutList;
    bool         mBlenderVersionFound;

    enum BlenderRenPathType
    {
        LBL_BLENDER_PATH,                       //  0 QLineEdit/QPushButton
        LBL_BLENDFILE_PATH,                     //  1 QLineEdit/QPushButton
        LBL_ENVIRONMENT_PATH,                   //  2 QLineEdit/QPushButton
        LBL_LDCONFIG_PATH,                      //  3 QLineEdit/QPushButton
        LBL_LDRAW_PATH,                         //  4 QLineEdit/QPushButton
        LBL_LSYNTH_PATH,                        //  5 QLineEdit/QPushButton
        LBL_STUD_LOGO_PATH,                     //  6 QLineEdit/QPushButton
        LBL_STUDIO_LDRAW_PATH,                  //  7 QLineEdit/QPushButton

        NUM_BLENDER_PATHS
    };

    enum BlenderRenLabelType
    {
        LBL_ADD_ENVIRONMENT,                    //  0   QCheckBox
        LBL_ADD_GAPS,                           //  1   QCheckBox
        LBL_BEVEL_EDGES,                        //  2   QCheckBox
        LBL_BLENDFILE_TRUSTED,                  //  3   QCheckBox
        LBL_CROP_IMAGE,                         //  4   QCheckBox
        LBL_CURVED_WALLS,                       //  5   QCheckBox
        LBL_FLATTEN_HIERARCHY,                  //  6   QCheckBox
        NUM_COMBO_ITEMS,                        //  7
        LBL_IMPORT_CAMERAS = NUM_COMBO_ITEMS,   //  7   QCheckBox
        LBL_IMPORT_LIGHTS,                      //  8   QCheckBox
        LBL_INSTANCE_STUDS,                     //  9   QCheckBox
        LBL_KEEP_ASPECT_RATIO,                  // 10   QCheckBox
        LBL_LINK_PARTS,                         // 11   QCheckBox
        LBL_NUMBER_NODES,                       // 12   QCheckBox
        LBL_OVERWRITE_IMAGE,                    // 13   QCheckBox
        LBL_OVERWRITE_MATERIALS,                // 14   QCheckBox
        LBL_OVERWRITE_MESHES,                   // 15   QCheckBox
        LBL_POSITION_CAMERA,                    // 16   QCheckBox
        LBL_REMOVE_DOUBLES,                     // 17   QCheckBox
        LBL_RENDER_WINDOW,                      // 18   QCheckBox
        LBL_USE_ARCHIVE_LIBS,                   // 19   QChekcBox
        LBL_SEARCH_ADDL_PATHS,                  // 20   QCheckBox
        LBL_SMOOTH_SHADING,                     // 21   QCheckBox
        LBL_TRANSPARENT_BACKGROUND,             // 22   QCheckBox
        LBL_UNOFFICIAL_PARTS,                   // 23   QCheckBox
        LBL_USE_LOGO_STUDS,                     // 24   QCheckBox
        LBL_VERBOSE,                            // 25   QCheckBox

        LBL_BEVEL_WIDTH,                        // 26/0 QLineEdit
        LBL_CAMERA_BORDER_PERCENT,              // 27/1 QLineEdit
        LBL_DEFAULT_COLOUR,                     // 28/2 QLineEdit
        LBL_GAPS_SIZE,                          // 29/3 QLineEdit
        LBL_IMAGE_WIDTH,                        // 30/4 QLineEdit
        LBL_IMAGE_HEIGHT,                       // 31/5 QLineEdit
        LBL_IMAGE_SCALE,                        // 32/6 QLineEdit
        LBL_RENDER_PERCENTAGE,                  // 33/7 QLineEdit

        LBL_COLOUR_SCHEME,                      // 33/0 QComboBox
        LBL_FLEX_PARTS_SOURCE,                  // 34/1 QComboBox
        LBL_LOGO_STUD_VERSION,                  // 35/2 QComboBox
        LBL_LOOK,                               // 26/3 QComboBox
        LBL_POSITION_OBJECT,                    // 37/4 QComboBox
        LBL_RESOLUTION,                         // 38/5 QComboBox
        LBL_RESOLVE_NORMALS,                    // 39/6 QComboBox

        NUM_SETTINGS
    };

    enum BlenderRenControlType
    {                                                    // Index
        BLENDER_PATH_EDIT,                               // 00
        ADD_ENVIRONMENT_BOX     = BLENDER_PATH_EDIT,     // 00
        BEVEL_WIDTH_EDIT        = BLENDER_PATH_EDIT,     // 00
        COLOUR_SCHEME_COMBO     = BLENDER_PATH_EDIT,     // 00
        ADD_GAPS_BOX,                                    // 01
        CAMERA_BORDER_PERCENT_EDIT = ADD_GAPS_BOX,       // 01
        FLEX_PARTS_SOURCE_COMBO = ADD_GAPS_BOX,          // 01
        DEFAULT_SETTINGS        = ADD_GAPS_BOX,          // 01
        BEVEL_EDGES_BOX,                                 // 02
        DEFAULT_COLOUR_EDIT     = BEVEL_EDGES_BOX,       // 02
        LOGO_STUD_VERSION_COMBO = BEVEL_EDGES_BOX,       // 02
        BLENDFILE_TRUSTED_BOX,                           // 03
        GAPS_SIZE_EDIT          = BLENDFILE_TRUSTED_BOX, // 03
        LOOK_COMBO              = BLENDFILE_TRUSTED_BOX, // 03
        CROP_IMAGE_BOX,                                  // 04
        IMAGE_WIDTH_EDIT        = CROP_IMAGE_BOX,        // 04
        POSITION_OBJECT_COMBO   = CROP_IMAGE_BOX,        // 04
        CURVED_WALLS_BOX,                                // 05
        IMAGE_HEIGHT_EDIT       = CURVED_WALLS_BOX,      // 05
        RESOLUTION_COMBO        = CURVED_WALLS_BOX,      // 05
        FLATTEN_HIERARCHY_BOX,                           // 06
        IMAGE_SCALE_EDIT        = FLATTEN_HIERARCHY_BOX, // 06
        RESOLVE_NORMALS_COMBO   = FLATTEN_HIERARCHY_BOX, // 06
        COMBO_BOX_ITEMS         = FLATTEN_HIERARCHY_BOX, // 06
        IMPORT_CAMERAS_BOX,                              // 07
        RENDER_PERCENTAGE_EDIT  = IMPORT_CAMERAS_BOX,    // 07
        LINE_EDIT_ITEMS         = IMPORT_CAMERAS_BOX,    // 07
        IMPORT_LIGHTS_BOX,                               // 08
        INSTANCE_STUDS_BOX,                              // 09
        KEEP_ASPECT_RATIO_BOX,                           // 10
        LINK_PARTS_BOX,                                  // 11
        NUMBER_NODES_BOX,                                // 12
        OVERWRITE_IMAGE_BOX,                             // 13
        OVERWRITE_MATERIALS_BOX,                         // 14
        OVERWRITE_MESHES_BOX,                            // 15
        POSITION_CAMERA_BOX,                             // 16
        REMOVE_DOUBLES_BOX,                              // 17
        RENDER_WINDOW_BOX,                               // 18
        USE_ARCHIVE_LIBS_BOX,                            // 19
        SEARCH_ADDL_PATHS_BOX,                           // 20
        SMOOTH_SHADING_BOX,                              // 21
        TRANSPARENT_BACKGROUND_BOX,                      // 22
        UNOFFICIAL_PARTS_BOX,                            // 23
        USE_LOGO_STUDS_BOX,                              // 24
        VERBOSE_BOX                                      // 25
    };

    enum BlenderRenLabelTypeMM
    {
        LBL_ADD_ENVIRONMENT_MM,                    // 00 QCheckBox
        LBL_BLEND_FILE_TRUSTED_MM,                 // 01 QCheckBox
        LBL_CROP_IMAGE_MM,                         // 02 QCheckBox
        LBL_DISPLAY_LOGO,                          // 03 QCheckBox
        LBL_IMPORT_CAMERAS_MM,                     // 04 QCheckBox
        LBL_IMPORT_EDGES,                          // 05 QCheckBox
        NUM_COMBO_ITEMS_MM,                        // 06
        LBL_IMPORT_LIGHTS_MM = NUM_COMBO_ITEMS_MM, // 06 QCheckBox
        LBL_KEEP_ASPECT_RATIO_MM,                  // 07 QCheckBox
        LBL_MAKE_GAPS,                             // 08 QCheckBox
        LBL_META_BFC,                              // 09 QCheckBox
        LBL_META_CLEAR,                            // 10 QCheckBox
        LBL_META_GROUP,                            // 11 QCheckBox
        LBL_META_PAUSE,                            // 12 QCheckBox
        LBL_META_PRINT_WRITE,                      // 13 QCheckBox
        LBL_META_SAVE,                             // 14 QCheckBox
        LBL_META_STEP,                             // 15 QCheckBox
        LBL_META_STEP_GROUPS,                      // 16 QCheckBox
        LBL_NO_STUDS,                              // 17 QCheckBox
        LBL_OVERWRITE_IMAGE_MM,                    // 18 QCheckBox
        LBL_POSITION_CAMERA_MM,                    // 19 QCheckBox
        LBL_PARENT_TO_EMPTY,                       // 20 QCheckBox
        LBL_PREFER_STUDIO,                         // 21 QCheckBox
        LBL_PREFER_UNOFFICIAL,                     // 22 QCheckBox
        LBL_PRESERVE_HIERARCHY,                    // 23 QCheckBox
        LBL_PROFILE,                               // 24 QCheckBox
        LBL_RECALCULATE_NORMALS,                   // 25 QCheckBox
        LBL_REMOVE_DOUBLES_MM,                     // 26 QCheckBox
        LBL_RENDER_WINDOW_MM,                      // 27 QCheckBox
        LBL_SEARCH_ADDL_PATHS_MM,                  // 28 QCheckBox
        LBL_SETEND_FRAME,                          // 29 QCheckBox
        LBL_SET_TIMELINE_MARKERS,                  // 30 QCheckBox
        LBL_SHADE_SMOOTH,                          // 31 QCheckBox
        LBL_TRANSPARENT_BACKGROUND_MM,             // 32 QCheckBox
        LBL_TREAT_MODELS_WITH_SUBPARTS_AS_PARTS,   // 33 QCheckBox
        LBL_TREAT_SHORTCUT_AS_MODEL,               // 34 QCheckBox
        LBL_TRIANGULATE,                           // 35 QCheckBox
        LBL_USE_ARCHIVE_LIBRARY_MM,                // 36 QCheckBox
        LBL_USE_FREESTYLE_EDGES,                   // 37 QCheckBox
        LBL_VERBOSE_MM,                            // 38 QLineEdit

        LBL_CAMERA_BORDER_PERCENT_MM,              // 39/00 QLineEdit
        LBL_FRAMES_PER_STEP,                       // 40/00 QLineEdit
        LBL_GAP_SCALE,                             // 41/01 QLineEdit
        LBL_IMPORT_SCALE,                          // 42/02 QLineEdit
        LBL_MERGE_DISTANCE,                        // 43/03 QLineEdit
        LBL_RENDER_PERCENTAGE_MM,                  // 44/04 QLineEdit
        LBL_RESOLUTION_WIDTH,                      // 45/05 QLineEdit
        LBL_RESOLUTION_HEIGHT,                     // 46/06 QLineEdit
        LBL_STARTING_STEP_FRAME,                   // 47/07 QLineEdit

        LBL_CHOSEN_LOGO,                           // 48/00 QComboBox
        LBL_COLOUR_SCHEME_MM,                      // 49/01 QComboBox
        LBL_GAP_SCALE_STRATEGY,                    // 50/02 QComboBox
        LBL_GAP_TARGET,                            // 51/03 QComboBox
        LBL_RESOLUTION_MM,                         // 52/04 QComboBox
        LBL_SMOOTH_TYPE,                           // 53/05 QComboBox

        NUM_SETTINGS_MM
    };

    enum BlenderRenControlTypeMM
    {                                                             // Index
        BLENDER_PATH_EDIT_MM,                                     // 00
        ADD_ENVIRONMENT_BOX_MM        = BLENDER_PATH_EDIT_MM,     // 00
        CAMERA_BORDER_PERCENT_EDIT_MM = BLENDER_PATH_EDIT_MM,     // 00
        CHOSEN_LOGO_COMBO             = BLENDER_PATH_EDIT_MM,     // 00
        BLEND_FILE_TRUSTED_BOX_MM,                                // 01
        FRAMES_PER_STEP_EDIT          = BLEND_FILE_TRUSTED_BOX_MM,// 01
        COLOUR_SCHEME_COMBO_MM        = BLEND_FILE_TRUSTED_BOX_MM,// 01
        CROP_IMAGE_BOX_MM,                                        // 02
        GAP_SCALE_EDIT                = CROP_IMAGE_BOX_MM,        // 02
        GAP_SCALE_STRATEGY_COMBO      = CROP_IMAGE_BOX_MM,        // 02
        DISPLAY_LOGO_BOX,                                         // 03
        IMPORT_SCALE_EDIT             = DISPLAY_LOGO_BOX,         // 03
        GAP_TARGET_COMBO              = DISPLAY_LOGO_BOX,         // 03
        IMPORT_CAMERAS_BOX_MM,                                    // 04
        MERGE_DISTANCE_EDIT           = IMPORT_CAMERAS_BOX_MM,    // 04
        RESOLUTION_COMBO_MM           = IMPORT_CAMERAS_BOX_MM,    // 04
        IMPORT_EDGES_BOX,                                         // 05
        RENDER_PERCENTAGE_EDIT_MM     = IMPORT_EDGES_BOX,         // 05
        SMOOTH_TYPE_COMBO             = IMPORT_EDGES_BOX,         // 05
        COMBO_BOX_ITEMS_MM            = IMPORT_EDGES_BOX,         // 05
        IMPORT_LIGHTS_BOX_MM,                                     // 06
        RESOLUTION_WIDTH_EDIT         = IMPORT_LIGHTS_BOX_MM,     // 06
        KEEP_ASPECT_RATIO_BOX_MM,                                 // 07
        RESOLUTION_HEIGHT_EDIT        = KEEP_ASPECT_RATIO_BOX_MM, // 07
        MAKE_GAPS_BOX,                                            // 08
        STARTING_STEP_FRAME_EDIT      = MAKE_GAPS_BOX,            // 08
        LINE_EDIT_ITEMS_MM            = MAKE_GAPS_BOX,            // 08
        META_BFC_BOX,                                             // 09
        META_CLEAR_BOX,                                           // 10
        META_GROUP_BOX,                                           // 11
        META_PAUSE_BOX,                                           // 12
        META_PRINT_WRITE_BOX,                                     // 13
        META_SAVE_BOX,                                            // 14
        META_STEP_BOX,                                            // 15
        META_STEP_GROUPS_BOX,                                     // 16
        NO_STUDS_BOX,                                             // 17
        OVERWRITE_IMAGE_BOX_MM,                                   // 18
        PARENT_TO_EMPTY_BOX,                                      // 19
        PREFER_STUDIO_BOX,                                        // 20
        PREFER_UNOFFICIAL_BOX,                                    // 21
        PRESERVE_HIERARCHY_BOX,                                   // 22
        PROFILE_BOX,                                              // 23
        RECALCULATE_NORMALS_BOX,                                  // 24
        REMOVE_DOUBLES_BOX_MM,                                    // 25
        RENDER_WINDOW_BOX_MM,                                     // 26
        SEARCH_ADDL_PATHS_MM_BOX,                                 // 27
        SETEND_FRAME_BOX,                                         // 28
        SET_TIMELINE_MARKERS_BOX,                                 // 29
        SHADE_SMOOTH_BOX,                                         // 30
        TRANSPARENT_BACKGROUND_BOX_MM,                            // 31
        TREAT_MODELS_WITH_SUBPARTS_AS_PARTS_BOX,                  // 32
        TREAT_SHORTCUT_AS_MODEL_BOX,                              // 33
        TRIANGULATE_BOX,                                          // 34
        USE_ARCHIVE_LIBRARY_BOX_MM,                               // 35
        USE_FREESTYLE_EDGES_BOX,                                  // 36
        VERBOSE_BOX_MM                                            // 37
    };

    enum BlenderRenBrickType
    {
        PARAMS_CUSTOM_COLOURS,
        PARAMS_SLOPED_BRICKS,
        PARAMS_LIGHTED_BRICKS
    };

    struct BlenderSettings
    {
        QString key;
        QString value;
        QString label;
        QString tooltip;
    };

    struct BlenderPaths
    {
        QString key;
        QString key_mm;
        QString value;
        QString label;
        QString tooltip;
    };

    struct ComboItems
    {
        QString dataList;
        QString itemList;
    };

    static BlenderPaths    mBlenderPaths[];
    static BlenderPaths    mDefaultPaths[];
    static BlenderSettings mBlenderSettings[];
    static BlenderSettings mBlenderSettingsMM[];
    static BlenderSettings mDefaultSettings[];
    static BlenderSettings mDefaultSettingsMM[];
    static ComboItems      mComboItems[];
    static ComboItems      mComboItemsMM[];



    QAction     *mDefaultColourEditAction;
    QPushButton *mAddonUpdateButton;
    QPushButton *mAddonStdOutButton;
    QPushButton *mPathsGroupButton;

    QList<QLineEdit   *> mPathLineEditList;
    QList<QPushButton *> mPathBrowseButtonList;
    QList<QLabel      *> mSettingLabelList;
    QList<QCheckBox   *> mCheckBoxList;
    QList<QLineEdit   *> mLineEditList;
    QList<QComboBox   *> mComboBoxList;

    QString mBlenderVersion;
    QString mAddonVersion;
    bool mConfigured;
    bool mAddonUpdate;
    bool mDocumentRender;
    bool mDialogCancelled;
};

extern class BlenderPreferences *gAddonPreferences;

#endif // BLENDERPREFERENCES_H
