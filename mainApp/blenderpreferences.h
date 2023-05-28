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
class QComboBox;
class QPushButton;
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
    void enableButton(bool);
    void accept() override;
    void reject() override;

private:
    QPushButton *mApplyButton;
    QPushButton *mResetButton;
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
    static bool settingsModified(bool update = true, QString const &module = QString());

    void apply(const int response = QDialog::Accepted);

    int     mImageWidth;
    int     mImageHeight;
    qreal   mRenderPercentage;

protected:
    static int numPaths(bool defaultSettings = false);
    static int numSettings(bool defaultSettings = false);
    static int numSettingsMM(bool defaultSettings = false);
    static bool getBlenderAddon(const QString &blenderDir);
    static bool extractBlenderAddon(const QString &blenderDir);
    static void loadDefaultParameters(QByteArray& buffer, int which);
    static bool overwriteFile(const QString &file);
    static int showMessage(QString const &title,
                           QString const &header,
                           QString const &body,
                           QString const &detail = QString(),
                           const int buttons = MBB_OK,
                           const int icon = 0);

    QString readStdErr(bool &hasError) const;
    void clearGroupBox(QGroupBox *groupBox);
    void initPathsAndSettings();
    void initPathsAndSettingsMM();

signals:
    void settingChangedSig(bool);

public slots:
    void resetSettings();
    void showPathsGroup();
    bool promptCancel();

private slots:
    void browseBlender(bool);
    void configureBlenderAddon(bool = false, bool = false);
    void updateBlenderAddon();
    void enableImportModule();
    void sizeChanged(const QString &);
    void setDefaultColor(int);
    void colorButtonClicked(bool);
    void setModelSize(bool = false);
    void validateColourScheme(int);
    void settingChanged(int = -1);
    void settingChanged(const QString &);
    void pathChanged();
    bool promptAccept();
    void update();
    void readStdOut();
    void readStdOut(const QString &, QString &);
    void writeStdOut();
    void getStandardOutput();
    void showResult();
    void statusUpdate(bool, bool = true, const QString & = QString());

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

    enum BlenderPathType
    {
        PATH_BLENDER,       //  0 QLineEdit/QPushButton
        PATH_BLENDFILE,     //  1 QLineEdit/QPushButton
        PATH_ENVIRONMENT,   //  2 QLineEdit/QPushButton
        PATH_LDCONFIG,      //  3 QLineEdit/QPushButton
        PATH_LDRAW,         //  4 QLineEdit/QPushButton
        PATH_LSYNTH,        //  5 QLineEdit/QPushButton
        PATH_STUD_LOGO,     //  6 QLineEdit/QPushButton
        PATH_STUDIO_LDRAW,  //  7 QLineEdit/QPushButton

        NUM_PATHS
    };

    enum BlenderLabelType
    {
        LBL_ADD_ENVIRONMENT,                    //  0   QCheckBox
        DEFAULT_SETTINGS,                       //  1
        LBL_ADD_GAPS = DEFAULT_SETTINGS,        //  1   QCheckBox
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

        LBL_COLOUR_SCHEME,                      // 34/0 QComboBox
        LBL_FLEX_PARTS_SOURCE,                  // 35/1 QComboBox
        LBL_LOGO_STUD_VERSION,                  // 36/2 QComboBox
        LBL_LOOK,                               // 27/3 QComboBox
        LBL_POSITION_OBJECT,                    // 38/4 QComboBox
        LBL_RESOLUTION,                         // 39/5 QComboBox
        LBL_RESOLVE_NORMALS,                    // 40/6 QComboBox

        NUM_SETTINGS
    };

    enum BlenderControlType
    {                                                            // Index
        CTL_BLENDER_PATH_EDIT,                                   // 00
        CTL_ADD_ENVIRONMENT_BOX     = CTL_BLENDER_PATH_EDIT,     // 00
        CTL_BEVEL_WIDTH_EDIT        = CTL_BLENDER_PATH_EDIT,     // 00
        CTL_COLOUR_SCHEME_COMBO     = CTL_BLENDER_PATH_EDIT,     // 00
        CTL_ADD_GAPS_BOX,                                        // 01
        CTL_CAMERA_BORDER_PERCENT_EDIT = CTL_ADD_GAPS_BOX,       // 01
        CTL_FLEX_PARTS_SOURCE_COMBO = CTL_ADD_GAPS_BOX,          // 01
        CTL_BEVEL_EDGES_BOX,                                     // 02
        CTL_DEFAULT_COLOUR_EDIT     = CTL_BEVEL_EDGES_BOX,       // 02
        CTL_LOGO_STUD_VERSION_COMBO = CTL_BEVEL_EDGES_BOX,       // 02
        CTL_BLENDFILE_TRUSTED_BOX,                               // 03
        CTL_GAPS_SIZE_EDIT          = CTL_BLENDFILE_TRUSTED_BOX, // 03
        CTL_LOOK_COMBO              = CTL_BLENDFILE_TRUSTED_BOX, // 03
        CTL_CROP_IMAGE_BOX,                                      // 04
        CTL_IMAGE_WIDTH_EDIT        = CTL_CROP_IMAGE_BOX,        // 04
        CTL_POSITION_OBJECT_COMBO   = CTL_CROP_IMAGE_BOX,        // 04
        CTL_CURVED_WALLS_BOX,                                    // 05
        CTL_IMAGE_HEIGHT_EDIT       = CTL_CURVED_WALLS_BOX,      // 05
        CTL_RESOLUTION_COMBO        = CTL_CURVED_WALLS_BOX,      // 05
        CTL_FLATTEN_HIERARCHY_BOX,                               // 06
        CTL_IMAGE_SCALE_EDIT        = CTL_FLATTEN_HIERARCHY_BOX, // 06
        CTL_RESOLVE_NORMALS_COMBO   = CTL_FLATTEN_HIERARCHY_BOX, // 06
        CTL_IMPORT_CAMERAS_BOX,                                  // 07
        CTL_RENDER_PERCENTAGE_EDIT  = CTL_IMPORT_CAMERAS_BOX,    // 07
        CTL_LINE_EDIT_ITEMS         = CTL_IMPORT_CAMERAS_BOX,    // 07
        CTL_IMPORT_LIGHTS_BOX,                                   // 08
        CTL_INSTANCE_STUDS_BOX,                                  // 09
        CTL_KEEP_ASPECT_RATIO_BOX,                               // 10
        CTL_LINK_PARTS_BOX,                                      // 11
        CTL_NUMBER_NODES_BOX,                                    // 12
        CTL_OVERWRITE_IMAGE_BOX,                                 // 13
        CTL_OVERWRITE_MATERIALS_BOX,                             // 14
        CTL_OVERWRITE_MESHES_BOX,                                // 15
        CTL_POSITION_CAMERA_BOX,                                 // 16
        CTL_REMOVE_DOUBLES_BOX,                                  // 17
        CTL_RENDER_WINDOW_BOX,                                   // 18
        CTL_USE_ARCHIVE_LIBS_BOX,                                // 19
        CTL_SEARCH_ADDL_PATHS_BOX,                               // 20
        CTL_SMOOTH_SHADING_BOX,                                  // 21
        CTL_TRANSPARENT_BACKGROUND_BOX,                          // 22
        CTL_UNOFFICIAL_PARTS_BOX,                                // 23
        CTL_USE_LOGO_STUDS_BOX,                                  // 24
        CTL_VERBOSE_BOX,                                         // 25
    };

    enum BlenderLabelTypeMM
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

    enum BlenderControlTypeMM
    {                                                                     // Index
        CTL_BLENDER_PATH_EDIT_MM,                                         // 00
        CTL_ADD_ENVIRONMENT_BOX_MM        = CTL_BLENDER_PATH_EDIT_MM,     // 00
        CTL_CAMERA_BORDER_PERCENT_EDIT_MM = CTL_BLENDER_PATH_EDIT_MM,     // 00
        CTL_CHOSEN_LOGO_COMBO             = CTL_BLENDER_PATH_EDIT_MM,     // 00
        CTL_BLEND_FILE_TRUSTED_BOX_MM,                                    // 01
        CTL_FRAMES_PER_STEP_EDIT          = CTL_BLEND_FILE_TRUSTED_BOX_MM,// 01
        CTL_COLOUR_SCHEME_COMBO_MM        = CTL_BLEND_FILE_TRUSTED_BOX_MM,// 01
        CTL_CROP_IMAGE_BOX_MM,                                            // 02
        CTL_GAP_SCALE_EDIT                = CTL_CROP_IMAGE_BOX_MM,        // 02
        CTL_GAP_SCALE_STRATEGY_COMBO      = CTL_CROP_IMAGE_BOX_MM,        // 02
        CTL_DISPLAY_LOGO_BOX,                                             // 03
        CTL_IMPORT_SCALE_EDIT             = CTL_DISPLAY_LOGO_BOX,         // 03
        CTL_GAP_TARGET_COMBO              = CTL_DISPLAY_LOGO_BOX,         // 03
        CTL_IMPORT_CAMERAS_BOX_MM,                                        // 04
        CTL_MERGE_DISTANCE_EDIT           = CTL_IMPORT_CAMERAS_BOX_MM,    // 04
        CTL_RESOLUTION_COMBO_MM           = CTL_IMPORT_CAMERAS_BOX_MM,    // 04
        CTL_IMPORT_EDGES_BOX,                                             // 05
        CTL_RENDER_PERCENTAGE_EDIT_MM     = CTL_IMPORT_EDGES_BOX,         // 05
        CTL_SMOOTH_TYPE_COMBO             = CTL_IMPORT_EDGES_BOX,         // 05
        CTL_IMPORT_LIGHTS_BOX_MM,                                         // 06
        CTL_RESOLUTION_WIDTH_EDIT         = CTL_IMPORT_LIGHTS_BOX_MM,     // 06
        CTL_KEEP_ASPECT_RATIO_BOX_MM,                                     // 07
        CTL_RESOLUTION_HEIGHT_EDIT        = CTL_KEEP_ASPECT_RATIO_BOX_MM, // 07
        CTL_MAKE_GAPS_BOX,                                                // 08
        CTL_STARTING_STEP_FRAME_EDIT      = CTL_MAKE_GAPS_BOX,            // 08
        CTL_LINE_EDIT_ITEMS_MM            = CTL_MAKE_GAPS_BOX,            // 08
        CTL_META_BFC_BOX,                                                 // 09
        CTL_META_CLEAR_BOX,                                               // 10
        CTL_META_GROUP_BOX,                                               // 11
        CTL_META_PAUSE_BOX,                                               // 12
        CTL_META_PRINT_WRITE_BOX,                                         // 13
        CTL_META_SAVE_BOX,                                                // 14
        CTL_META_STEP_BOX,                                                // 15
        CTL_META_STEP_GROUPS_BOX,                                         // 16
        CTL_NO_STUDS_BOX,                                                 // 17
        CTL_OVERWRITE_IMAGE_BOX_MM,                                       // 18
        CTL_PARENT_TO_EMPTY_BOX,                                          // 19
        CTL_PREFER_STUDIO_BOX,                                            // 20
        CTL_PREFER_UNOFFICIAL_BOX,                                        // 21
        CTL_PRESERVE_HIERARCHY_BOX,                                       // 22
        CTL_PROFILE_BOX,                                                  // 23
        CTL_RECALCULATE_NORMALS_BOX,                                      // 24
        CTL_REMOVE_DOUBLES_BOX_MM,                                        // 25
        CTL_RENDER_WINDOW_BOX_MM,                                         // 26
        CTL_SEARCH_ADDL_PATHS_MM_BOX,                                     // 27
        CTL_SETEND_FRAME_BOX,                                             // 28
        CTL_SET_TIMELINE_MARKERS_BOX,                                     // 29
        CTL_SHADE_SMOOTH_BOX,                                             // 30
        CTL_TRANSPARENT_BACKGROUND_BOX_MM,                                // 31
        CTL_TREAT_MODELS_WITH_SUBPARTS_AS_PARTS_BOX,                      // 32
        CTL_TREAT_SHORTCUT_AS_MODEL_BOX,                                  // 33
        CTL_TRIANGULATE_BOX,                                              // 34
        CTL_USE_ARCHIVE_LIBRARY_BOX_MM,                                   // 35
        CTL_USE_FREESTYLE_EDGES_BOX,                                      // 36
        CTL_VERBOSE_BOX_MM                                                // 37
    };

    enum BlenderBrickType
    {
        PARAMS_CUSTOM_COLOURS,
        PARAMS_SLOPED_BRICKS,
        PARAMS_LIGHTED_BRICKS
    };

    enum MessageBoxButtonsType {
        MBB_OK,
        MBB_YES,
        MBB_YES_NO
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
    bool mDocumentRender;
    bool mDialogCancelled;
};

extern class BlenderPreferences *gAddonPreferences;

#endif // BLENDERPREFERENCES_H
