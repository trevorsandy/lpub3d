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
    static int showMessage(QString const &header,
                           QString const &title = QString(),
                           QString const &body = QString(),
                           QString const &detail = QString(),
                           const int buttons = MBB_OK,
                           const int icon = 3 /*QMessageBox::Critical*/);

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
    void configureBlenderAddon(bool = false, bool = false, bool = false);
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
    enum BlenderPathType
    {
        PATH_BLENDER,             //  0 QLineEdit/QPushButton
        PATH_BLENDFILE,           //  1 QLineEdit/QPushButton
        PATH_ENVIRONMENT,         //  2 QLineEdit/QPushButton
        PATH_LDCONFIG,            //  3 QLineEdit/QPushButton
        PATH_LDRAW,               //  4 QLineEdit/QPushButton
        PATH_LSYNTH,              //  5 QLineEdit/QPushButton
        PATH_STUD_LOGO,           //  6 QLineEdit/QPushButton
        PATH_STUDIO_LDRAW,        //  7 QLineEdit/QPushButton
        PATH_STUDIO_CUSTOM_PARTS, //  8 QLineEdit/QPushButton

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
        LBL_MINIFIG_HIERARCHY,                  // 12   QCheckBox
        LBL_NUMBER_NODES,                       // 13   QCheckBox
        LBL_OVERWRITE_IMAGE,                    // 14   QCheckBox
        LBL_OVERWRITE_MATERIALS,                // 15   QCheckBox
        LBL_OVERWRITE_MESHES,                   // 16   QCheckBox
        LBL_POSITION_CAMERA,                    // 17   QCheckBox
        LBL_REMOVE_DOUBLES,                     // 18   QCheckBox
        LBL_RENDER_WINDOW,                      // 19   QCheckBox
        LBL_USE_ARCHIVE_LIBS,                   // 20   QChekcBox
        LBL_SEARCH_ADDL_PATHS,                  // 21   QCheckBox
        LBL_SMOOTH_SHADING,                     // 22   QCheckBox
        LBL_TRANSPARENT_BACKGROUND,             // 23   QCheckBox
        LBL_UNOFFICIAL_PARTS,                   // 24   QCheckBox
        LBL_USE_LOGO_STUDS,                     // 25   QCheckBox
        LBL_VERBOSE,                            // 26   QCheckBox

        LBL_BEVEL_WIDTH,                        // 27/0 QLineEdit
        LBL_CAMERA_BORDER_PERCENT,              // 28/1 QLineEdit
        LBL_DEFAULT_COLOUR,                     // 29/2 QLineEdit
        LBL_GAPS_SIZE,                          // 30/3 QLineEdit
        LBL_IMAGE_WIDTH,                        // 31/4 QLineEdit
        LBL_IMAGE_HEIGHT,                       // 32/5 QLineEdit
        LBL_IMAGE_SCALE,                        // 33/6 QLineEdit
        LBL_RENDER_PERCENTAGE,                  // 34/7 QLineEdit

        LBL_COLOUR_SCHEME,                      // 35/0 QComboBox
        LBL_FLEX_PARTS_SOURCE,                  // 36/1 QComboBox
        LBL_LOGO_STUD_VERSION,                  // 27/2 QComboBox
        LBL_LOOK,                               // 38/3 QComboBox
        LBL_POSITION_OBJECT,                    // 39/4 QComboBox
        LBL_RESOLUTION,                         // 40/5 QComboBox
        LBL_RESOLVE_NORMALS,                    // 41/6 QComboBox

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
        CTL_MINIFIG_HIERARCHY_BOX,                               // 12
        CTL_NUMBER_NODES_BOX,                                    // 13
        CTL_OVERWRITE_IMAGE_BOX,                                 // 14
        CTL_OVERWRITE_MATERIALS_BOX,                             // 15
        CTL_OVERWRITE_MESHES_BOX,                                // 16
        CTL_POSITION_CAMERA_BOX,                                 // 17
        CTL_REMOVE_DOUBLES_BOX,                                  // 18
        CTL_RENDER_WINDOW_BOX,                                   // 19
        CTL_USE_ARCHIVE_LIBS_BOX,                                // 20
        CTL_SEARCH_ADDL_PATHS_BOX,                               // 21
        CTL_SMOOTH_SHADING_BOX,                                  // 22
        CTL_TRANSPARENT_BACKGROUND_BOX,                          // 23
        CTL_UNOFFICIAL_PARTS_BOX,                                // 24
        CTL_USE_LOGO_STUDS_BOX,                                  // 25
        CTL_VERBOSE_BOX,                                         // 26
    };

    enum BlenderLabelTypeMM
    {
        LBL_ADD_ENVIRONMENT_MM,                    //  0 QCheckBox
        LBL_BEVEL_EDGES_MM,                        //  1 QCheckBox
        LBL_BLEND_FILE_TRUSTED_MM,                 //  2 QCheckBox
        LBL_CASE_SENSITIVE_FILESYSTEM,             //  3 QCheckBox
        LBL_CROP_IMAGE_MM,                         //  4 QCheckBox
        NUM_COMBO_ITEMS_MM,                        //  5
        LBL_DISPLAY_LOGO = NUM_COMBO_ITEMS_MM,     //  5 QCheckBox
        LBL_IMPORT_CAMERAS_MM,                     //  6 QCheckBox
        LBL_IMPORT_EDGES,                          //  7 QCheckBox
        LBL_IMPORT_LIGHTS_MM,                      //  8 QCheckBox
        LBL_KEEP_ASPECT_RATIO_MM,                  //  9 QCheckBox
        LBL_MAKE_GAPS,                             // 10 QCheckBox
        LBL_META_BFC,                              // 11 QCheckBox
        LBL_META_CLEAR,                            // 12 QCheckBox
        LBL_META_GROUP,                            // 13 QCheckBox
        LBL_META_PAUSE,                            // 14 QCheckBox
        LBL_META_PRINT_WRITE,                      // 15 QCheckBox
        LBL_META_SAVE,                             // 16 QCheckBox
        LBL_META_STEP,                             // 17 QCheckBox
        LBL_META_STEP_GROUPS,                      // 18 QCheckBox
        LBL_META_TEXMAP,                           // 19 QCheckBox
        LBL_NO_STUDS,                              // 20 QCheckBox
        LBL_OVERWRITE_IMAGE_MM,                    // 21 QCheckBox
        LBL_POSITION_CAMERA_MM,                    // 22 QCheckBox
        LBL_PARENT_TO_EMPTY,                       // 23 QCheckBox
        LBL_PREFER_STUDIO,                         // 24 QCheckBox
        LBL_PREFER_UNOFFICIAL,                     // 25 QCheckBox
        LBL_PROFILE,                               // 26 QCheckBox
        LBL_RECALCULATE_NORMALS,                   // 27 QCheckBox
        LBL_REMOVE_DOUBLES_MM,                     // 28 QCheckBox
        LBL_RENDER_WINDOW_MM,                      // 29 QCheckBox
        LBL_SEARCH_ADDL_PATHS_MM,                  // 30 QCheckBox
        LBL_SETEND_FRAME,                          // 31 QCheckBox
        LBL_SET_TIMELINE_MARKERS,                  // 32 QCheckBox
        LBL_SHADE_SMOOTH,                          // 33 QCheckBox
        LBL_TRANSPARENT_BACKGROUND_MM,             // 34 QCheckBox
        LBL_TREAT_SHORTCUT_AS_MODEL,               // 35 QCheckBox
        LBL_TRIANGULATE,                           // 36 QCheckBox
        LBL_USE_ARCHIVE_LIBRARY_MM,                // 37 QCheckBox
        LBL_USE_FREESTYLE_EDGES,                   // 38 QCheckBox
        LBL_VERBOSE_MM,                            // 39 QCheckBox

        LBL_BEVEL_SEGMENTS,                        // 40/ 0 QLineEdit
        LBL_BEVEL_WEIGHT,                          // 41/ 1 QLineEdit
        LBL_BEVEL_WIDTH_MM,                        // 42/ 2 QLineEdit
        LBL_CAMERA_BORDER_PERCENT_MM,              // 43/ 3 QLineEdit
        LBL_FRAMES_PER_STEP,                       // 44/ 4 QLineEdit
        LBL_GAP_SCALE,                             // 45/ 5 QLineEdit
        LBL_IMPORT_SCALE,                          // 46/ 6 QLineEdit
        LBL_MERGE_DISTANCE,                        // 47/ 7 QLineEdit
        LBL_RENDER_PERCENTAGE_MM,                  // 48/ 8 QLineEdit
        LBL_RESOLUTION_WIDTH,                      // 49/ 9 QLineEdit
        LBL_RESOLUTION_HEIGHT,                     // 50/10 QLineEdit
        LBL_STARTING_STEP_FRAME,                   // 51/11 QLineEdit

        LBL_CHOSEN_LOGO,                           // 52/ 0 QComboBox
        LBL_COLOUR_SCHEME_MM,                      // 53/ 1 QComboBox
        LBL_RESOLUTION_MM,                         // 54/ 2 QComboBox
        LBL_SCALE_STRATEGY,                        // 55/ 3 QComboBox
        LBL_SMOOTH_TYPE,                           // 56/ 4 QComboBox

        NUM_SETTINGS_MM
    };

    enum BlenderControlTypeMM
    {
        CTL_BLENDER_PATH_EDIT_MM,                                         //  0
        CTL_ADD_ENVIRONMENT_BOX_MM        = CTL_BLENDER_PATH_EDIT_MM,     //  0
        CTL_BEVEL_SEGMENTS_EDIT           = CTL_BLENDER_PATH_EDIT_MM,     //  0
        CTL_CHOSEN_LOGO_COMBO             = CTL_BLENDER_PATH_EDIT_MM,     //  0
        CTL_BEVEL_EDGES_BOX_MM,                                           //  1
        CTL_BEVEL_WEIGHT_EDIT             = CTL_BEVEL_EDGES_BOX_MM,       //  1
        CTL_COLOUR_SCHEME_COMBO_MM        = CTL_BEVEL_EDGES_BOX_MM,       //  1
        CTL_BLEND_FILE_TRUSTED_BOX_MM,                                    //  2
        CTL_BEVEL_WIDTH_EDIT_MM           = CTL_BLEND_FILE_TRUSTED_BOX_MM,//  2
        CTL_CASE_SENSITIVE_FILESYSTEM_BOX,                                //  3
        CTL_CAMERA_BORDER_PERCENT_EDIT_MM = CTL_CASE_SENSITIVE_FILESYSTEM_BOX,// 3
        CTL_RESOLUTION_COMBO_MM           = CTL_CASE_SENSITIVE_FILESYSTEM_BOX,// 3
        CTL_SCALE_STRATEGY_COMBO          = CTL_CASE_SENSITIVE_FILESYSTEM_BOX,// 3
        CTL_CROP_IMAGE_BOX_MM,                                            //  4
        CTL_FRAMES_PER_STEP_EDIT          = CTL_CROP_IMAGE_BOX_MM,        //  4
        CTL_SMOOTH_TYPE_COMBO             = CTL_CROP_IMAGE_BOX_MM,        //  4
        CTL_DISPLAY_LOGO_BOX,                                             //  5
        CTL_GAP_SCALE_EDIT                = CTL_DISPLAY_LOGO_BOX,         //  5
        CTL_IMPORT_CAMERAS_BOX_MM,                                        //  6
        CTL_IMPORT_SCALE_EDIT             = CTL_IMPORT_CAMERAS_BOX_MM,    //  6
        CTL_IMPORT_EDGES_BOX,                                             //  7
        CTL_MERGE_DISTANCE_EDIT           = CTL_IMPORT_EDGES_BOX,         //  7
        CTL_IMPORT_LIGHTS_BOX_MM,                                         //  8
        CTL_RENDER_PERCENTAGE_EDIT_MM     = CTL_IMPORT_LIGHTS_BOX_MM,     //  8
        CTL_KEEP_ASPECT_RATIO_BOX_MM,                                     //  9
        CTL_RESOLUTION_WIDTH_EDIT         = CTL_KEEP_ASPECT_RATIO_BOX_MM, //  9
        CTL_MAKE_GAPS_BOX,                                                // 10
        CTL_RESOLUTION_HEIGHT_EDIT        = CTL_MAKE_GAPS_BOX,            // 10
        CTL_META_BFC_BOX,                                                 // 11
        CTL_STARTING_STEP_FRAME_EDIT      = CTL_META_BFC_BOX,             // 11
        CTL_META_CLEAR_BOX,                                               // 12
        CTL_META_GROUP_BOX,                                               // 13
        CTL_META_PAUSE_BOX,                                               // 14
        CTL_META_PRINT_WRITE_BOX,                                         // 15
        CTL_META_SAVE_BOX,                                                // 16
        CTL_META_STEP_BOX,                                                // 17
        CTL_META_STEP_GROUPS_BOX,                                         // 18
        CTL_META_TEXMAP_BOX,                                              // 19
        CTL_NO_STUDS_BOX,                                                 // 20
        CTL_POSITION_CAMERA_BOX_MM,                                       // 21
        CTL_OVERWRITE_IMAGE_BOX_MM,                                       // 22
        CTL_PARENT_TO_EMPTY_BOX,                                          // 23
        CTL_PREFER_STUDIO_BOX,                                            // 24
        CTL_PREFER_UNOFFICIAL_BOX,                                        // 25
        CTL_PROFILE_BOX,                                                  // 26
        CTL_RECALCULATE_NORMALS_BOX,                                      // 27
        CTL_REMOVE_DOUBLES_BOX_MM,                                        // 28
        CTL_RENDER_WINDOW_BOX_MM,                                         // 29
        CTL_SEARCH_ADDL_PATHS_MM_BOX,                                     // 20
        CTL_SETEND_FRAME_BOX,                                             // 31
        CTL_SET_TIMELINE_MARKERS_BOX,                                     // 32
        CTL_SHADE_SMOOTH_BOX,                                             // 33
        CTL_TRANSPARENT_BACKGROUND_BOX_MM,                                // 34
        CTL_TREAT_SHORTCUT_AS_MODEL_BOX,                                  // 35
        CTL_TRIANGULATE_BOX,                                              // 36
        CTL_USE_ARCHIVE_LIBRARY_BOX_MM,                                   // 37
        CTL_USE_FREESTYLE_EDGES_BOX,                                      // 38
        CTL_VERBOSE_BOX_MM                                                // 39
    };

    enum BlenderBrickType
    {
        PARAMS_CUSTOM_COLOURS,
        PARAMS_SLOPED_BRICKS,
        PARAMS_LIGHTED_BRICKS
    };

    enum MessageBoxButtonsType
    {
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

    QList<QLineEdit   *> mPathLineEditList;
    QList<QPushButton *> mPathBrowseButtonList;
    QList<QLabel      *> mSettingLabelList;
    QList<QCheckBox   *> mCheckBoxList;
    QList<QLineEdit   *> mLineEditList;
    QList<QComboBox   *> mComboBoxList;

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
    QPushButton *mAddonUpdateButton;
    QPushButton *mAddonStdOutButton;
    QPushButton *mPathsGroupButton;

    QAction      *mDefaultColourEditAction;
    QProgressBar *mProgressBar;
    QProcess     *mProcess;

    QTimer        mUpdateTimer;

    QStringList mStdOutList;

    QString mBlenderVersion;
    QString mAddonVersion;
    bool mConfigured;
    bool mDocumentRender;
    bool mDialogCancelled;
    bool mBlenderVersionFound;
};

extern class BlenderPreferences *gAddonPreferences;

#endif // BLENDERPREFERENCES_H
