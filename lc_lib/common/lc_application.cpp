#include "lc_global.h"
#include <stdio.h>
#include "lc_application.h"
#include "lc_library.h"
#include "lc_profile.h"
#include "project.h"
#include "lc_mainwindow.h"
#include "lc_qpreferencesdialog.h"
#include "lc_partselectionwidget.h"
#include "lc_shortcuts.h"
#include "view.h"
/*** LPub3D Mod - includes ***/
#include "application.h"
#include "name.h"
#include "threadworkers.h"
/*** LPub3D Mod end ***/

lcApplication* gApplication;

void lcPreferences::LoadDefaults()
{
/*** LPub3D Mod - Splash message viewer defaults ***/
    emit Application::instance()->splashMsgSig("25% - Viewer defaults loading...");
/*** LPub3D Mod end ***/
    mFixedAxes = lcGetProfileInt(LC_PROFILE_FIXED_AXES);
    mMouseSensitivity = lcGetProfileInt(LC_PROFILE_MOUSE_SENSITIVITY);
    mShadingMode = (lcShadingMode)lcGetProfileInt(LC_PROFILE_SHADING_MODE);
    mDrawAxes = lcGetProfileInt(LC_PROFILE_DRAW_AXES);
    mDrawEdgeLines = lcGetProfileInt(LC_PROFILE_DRAW_EDGE_LINES);
    mLineWidth = lcGetProfileFloat(LC_PROFILE_LINE_WIDTH);
    mDrawGridStuds = lcGetProfileInt(LC_PROFILE_GRID_STUDS);
    mGridStudColor = lcGetProfileInt(LC_PROFILE_GRID_STUD_COLOR);
    mDrawGridLines = lcGetProfileInt(LC_PROFILE_GRID_LINES);
    mGridLineSpacing = lcGetProfileInt(LC_PROFILE_GRID_LINE_SPACING);
    mGridLineColor = lcGetProfileInt(LC_PROFILE_GRID_LINE_COLOR);
}

void lcPreferences::SaveDefaults()
{
    lcSetProfileInt(LC_PROFILE_FIXED_AXES, mFixedAxes);
    lcSetProfileInt(LC_PROFILE_MOUSE_SENSITIVITY, mMouseSensitivity);
    lcSetProfileInt(LC_PROFILE_SHADING_MODE, mShadingMode);
    lcSetProfileInt(LC_PROFILE_DRAW_AXES, mDrawAxes);
    lcSetProfileInt(LC_PROFILE_DRAW_EDGE_LINES, mDrawEdgeLines);
    lcSetProfileFloat(LC_PROFILE_LINE_WIDTH, mLineWidth);
    lcSetProfileInt(LC_PROFILE_GRID_STUDS, mDrawGridStuds);
    lcSetProfileInt(LC_PROFILE_GRID_STUD_COLOR, mGridStudColor);
    lcSetProfileInt(LC_PROFILE_GRID_LINES, mDrawGridLines);
    lcSetProfileInt(LC_PROFILE_GRID_LINE_SPACING, mGridLineSpacing);
    lcSetProfileInt(LC_PROFILE_GRID_LINE_COLOR, mGridLineColor);
}

lcApplication::lcApplication()
{

/*** LPub3D Mod - disable leoCAD application vars ***/
//#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
//    setApplicationDisplayName("LeoCAD");
//#endif
//  setOrganizationDomain("leocad.org");
//  setOrganizationName("LeoCAD Software");
//  setApplicationName("LeoCAD");
//  setApplicationVersion(LC_VERSION_TEXT);
/*** LPub3D Mod end ***/

    gApplication = this;
    mProject = nullptr;
    mLibrary = nullptr;
/*** LPub3D Mod - initialize loadFile param ***/
    mLoadFile = nullptr;
/*** LPub3D Mod end ***/

    mPreferences.LoadDefaults();
}

lcApplication::~lcApplication()
{
    delete mProject;
    delete mLibrary;
    gApplication = nullptr;
}

void lcApplication::SaveTabLayout() const
{
    if (!mProject || mProject->GetFileName().isEmpty())
        return;

    QSettings Settings;
    QByteArray TabLayout = gMainWindow->GetTabLayout();

    Settings.setValue(GetTabLayoutKey(), TabLayout);
}

QString lcApplication::GetTabLayoutKey() const
{
    if (mProject)
    {
        QString FileName = mProject->GetFileName();
        if (!FileName.isEmpty())
        {
            FileName.replace('\\', '?');
            FileName.replace('/', '?');
            return QString("TabLayouts/%1").arg(FileName);
        }
    }

    return QString();
}

void lcApplication::SetProject(Project* Project)
{
    SaveTabLayout();

    delete mProject;
    mProject = Project;

    gMainWindow->RemoveAllModelTabs();

    Project->SetActiveModel(0);
    lcGetPiecesLibrary()->RemoveTemporaryPieces();

    if (mProject && !mProject->GetFileName().isEmpty())
    {
        QSettings Settings;
        QByteArray TabLayout = Settings.value(GetTabLayoutKey()).toByteArray();

        gMainWindow->RestoreTabLayout(TabLayout);
    }
}

void lcApplication::SetClipboard(const QByteArray& Clipboard)
{
    mClipboard = Clipboard;
    gMainWindow->UpdatePaste(!mClipboard.isEmpty());
}

void lcApplication::ExportClipboard(const QByteArray& Clipboard)
{
    QMimeData* MimeData = new QMimeData();

    MimeData->setData("application/vnd.leocad-clipboard", Clipboard);
    QApplication::clipboard()->setMimeData(MimeData);

    SetClipboard(Clipboard);
}

bool lcApplication::LoadPartsLibrary(const QList<QPair<QString, bool>>& LibraryPaths, bool OnlyUsePaths, bool ShowProgress)
{
/*** LPub3D Mod - run search directories ***/
    // load search directories
    PartWorker partWorkerLDSearchDirs;
    partWorkerLDSearchDirs.ldsearchDirPreferences();
    // process search directories to update library archive
    partWorkerLDSearchDirs.processLDSearchDirParts();

    emit Application::instance()->splashMsgSig("80% - Archive libraries loading...");
/*** LPub3D Mod end ***/

    if (mLibrary == nullptr)
        mLibrary = new lcPiecesLibrary();

    if (!OnlyUsePaths)
    {
/*** LPub3D Mod - disable LEOCAD_LIB env var  ***/
/***
        char* EnvPath = getenv("LEOCAD_LIB");

        if (EnvPath && EnvPath[0])
            return mLibrary->Load(EnvPath, ShowProgress);
***/
/*** LPub3D Mod end ***/

        QString CustomPath = lcGetProfileString(LC_PROFILE_PARTS_LIBRARY);

        if (!CustomPath.isEmpty())
            return mLibrary->Load(CustomPath, ShowProgress);
    }

/*** LPub3D Mod - disable LibraryPaths load  ***/
/***
    for (const QPair<QString, bool>& LibraryPathEntry : LibraryPaths)
    {
        if (mLibrary->Load(LibraryPathEntry.first, ShowProgress))
        {
            if (LibraryPathEntry.second)
                mLibrary->SetOfficialPieces();

            return true;
        }
    }
***/
/*** LPub3D Mod end ***/
    return false;
}

bool lcApplication::Initialize(QList<QPair<QString, bool>>& LibraryPaths, QMainWindow *parent)
{
/*** LPub3D Mod - move ShowWindow from application ***/
    bool ShowWindow = false;
/*** LPub3D Mod end ***/
    bool OnlyUseLibraryPaths = false;
    bool SaveImage = false;
    bool SaveWavefront = false;
    bool Save3DS = false;
    bool SaveCOLLADA = false;
    bool SaveHTML = false;
    bool SetCameraAngles = false;
    bool Orthographic = false;
    bool ImageHighlight = false;
    int ImageWidth = lcGetProfileInt(LC_PROFILE_IMAGE_WIDTH);
    int ImageHeight = lcGetProfileInt(LC_PROFILE_IMAGE_HEIGHT);
    int ImageStart = 0;
    int ImageEnd = 0;
    int PartImagesWidth = -1;
    int PartImagesHeight = -1;
    float CameraLatitude, CameraLongitude;
    QString ImageName;
    QString ModelName;
    QString CameraName;
    QString ViewpointName;
    QString ProjectName;
    QString SaveWavefrontName;
    QString Save3DSName;
    QString SaveCOLLADAName;
    QString SaveHTMLName;

/*** LPub3D Mod - Splash message 3D Viewer ***/
    emit Application::instance()->splashMsgSig("45% - 3D Viewer widgets loading...");
/*** LPub3D Mod end ***/

/*** LPub3D Mod - Get arguments ***/
    QStringList Arguments = Application::instance()->arguments();
/*** LPub3D Mod end ***/
    const int NumArguments = Arguments.size();
    for (int ArgIdx = 1; ArgIdx < NumArguments; ArgIdx++)
    {
        const QString& Param = Arguments[ArgIdx];

        if (Param[0] != '-')
        {
            ProjectName = Param;
/*** LPub3D Mod - capture file loaded from command line ***/
            mLoadFile = Param;
/*** LPub3D Mod end ***/
            continue;
        }

        auto ParseString = [&ArgIdx, &Arguments, NumArguments](QString& Value, bool Required)
        {
            if (ArgIdx < NumArguments - 1 && Arguments[ArgIdx + 1][0] != '-')
            {
                ArgIdx++;
                Value = Arguments[ArgIdx];
            }
            else if (Required)
                printf("Not enough parameters for the '%s' argument.\n", Arguments[ArgIdx].toLatin1().constData());
        };

        auto ParseInteger = [&ArgIdx, &Arguments, NumArguments](int& Value)
        {
            if (ArgIdx < NumArguments - 1 && Arguments[ArgIdx + 1][0] != '-')
            {
                bool Ok = false;
                ArgIdx++;
                int NewValue = Arguments[ArgIdx].toInt(&Ok);

                if (Ok)
                    Value = NewValue;
                else
                    printf("Invalid value specified for the '%s' argument.\n", Arguments[ArgIdx - 1].toLatin1().constData());
            }
            else
                printf("Not enough parameters for the '%s' argument.\n", Arguments[ArgIdx].toLatin1().constData());
        };

        auto ParseVector2 = [&ArgIdx, &Arguments, NumArguments](float& Value1, float& Value2)
        {
            if (ArgIdx < NumArguments - 2 && Arguments[ArgIdx + 1][0] != '-' && Arguments[ArgIdx + 2][0] != '-')
            {
                bool Ok1 = false, Ok2 = false;

                ArgIdx++;
                float NewValue1 = Arguments[ArgIdx].toFloat(&Ok1);
                ArgIdx++;
                float NewValue2 = Arguments[ArgIdx].toFloat(&Ok2);

                if (Ok1 && Ok2)
                {
                    Value1 = NewValue1;
                    Value2 = NewValue2;
                    return true;
                }
                else
                    printf("Invalid value specified for the '%s' argument.\n", Arguments[ArgIdx - 2].toLatin1().constData());
            }
            else
                printf("Not enough parameters for the '%s' argument.\n", Arguments[ArgIdx].toLatin1().constData());

            return false;
        };

        if (Param == QLatin1String("-l") || Param == QLatin1String("--libpath"))
        {
            QString LibPath;
            ParseString(LibPath, true);
            if (!LibPath.isEmpty())
            {
                LibraryPaths.clear();
                LibraryPaths += qMakePair<QString, bool>(LibPath, false);
                OnlyUseLibraryPaths = true;
            }
        }
        else if (Param == QLatin1String("-i") || Param == QLatin1String("--image"))
        {
            SaveImage = true;
            ParseString(ImageName, false);
        }
        else if (Param == QLatin1String("-w") || Param == QLatin1String("--width"))
            ParseInteger(ImageWidth);
        else if (Param == QLatin1String("-h") || Param == QLatin1String("--height"))
            ParseInteger(ImageHeight);
        else if (Param == QLatin1String("-f") || Param == QLatin1String("--from"))
            ParseInteger(ImageStart);
        else if (Param == QLatin1String("-t") || Param == QLatin1String("--to"))
            ParseInteger(ImageEnd);
        else if (Param == QLatin1String("-s") || Param == QLatin1String("--submodel"))
            ParseString(ModelName, true);
        else if (Param == QLatin1String("-c") || Param == QLatin1String("--camera"))
            ParseString(CameraName, true);
        else if (Param == QLatin1String("--viewpoint"))
            ParseString(ViewpointName, true);
        else if (Param == QLatin1String("--camera-angles"))
            SetCameraAngles = ParseVector2(CameraLatitude, CameraLongitude);
        else if (Param == QLatin1String("--orthographic"))
            Orthographic = true;
        else if (Param == QLatin1String("--highlight"))
            ImageHighlight = true;
        else if (Param == QLatin1String("-obj") || Param == QLatin1String("--export-wavefront"))
        {
            SaveWavefront = true;
            ParseString(SaveWavefrontName, false);
        }
        else if (Param == QLatin1String("-3ds") || Param == QLatin1String("--export-3ds"))
        {
            Save3DS = true;
            ParseString(Save3DSName, false);
        }
        else if (Param == QLatin1String("-dae") || Param == QLatin1String("--export-collada"))
        {
            SaveCOLLADA = true;
            ParseString(SaveCOLLADAName, false);
        }
        else if (Param == QLatin1String("-html") || Param == QLatin1String("--export-html"))
        {
            SaveHTML = true;
            ParseString(SaveHTMLName, false);
        }
        else if (Param == QLatin1String("--html-parts-width"))
            ParseInteger(PartImagesWidth);
        else if (Param == QLatin1String("--html-parts-height"))
            ParseInteger(PartImagesHeight);
        else
            printf("Unknown commandline parameter: '%s'\n", Param.toLatin1().constData());
    }

/*** LPub3D Mod - initialize mainwindow with LPub3D parent ***/
    gMainWindow = new lcMainWindow(parent);
/*** LPub3D Mod end ***/

    lcLoadDefaultKeyboardShortcuts();
    lcLoadDefaultMouseShortcuts();

/*** LPub3D Mod - add modeGUI to ShowWindow var ***/
    ShowWindow = Application::instance()->modeGUI() && !SaveImage && !SaveWavefront && !Save3DS && !SaveCOLLADA && !SaveHTML;
/*** LPub3D Mod end ***/

    if (!LoadPartsLibrary(LibraryPaths, OnlyUseLibraryPaths, ShowWindow))
    {
        QString Message;

        if (mLibrary->LoadBuiltinPieces())
        {
/*** LPub3D Mod - modify initial load KO message ***/
            Message = tr("3DViewer could not find a compatible Parts Library so only a small number of parts will be available.\n");
/*** LPub3D Mod end ***/
        }
        else
        {
/*** LPub3D Mod - modify initial load KO message ***/
            Message = tr("3DViewer could not load Parts Library.\n");
/*** LPub3D Mod end ***/
        }
        if (ShowWindow)
            QMessageBox::information(gMainWindow, tr("3DViewer"), Message);
        else
            fprintf(stderr, "%s", Message.toLatin1().constData());
    }

    gMainWindow->CreateWidgets();

    Project* NewProject = new Project();
    SetProject(NewProject);

    if (!ProjectName.isEmpty() && gMainWindow->OpenProject(ProjectName))
    {
        if (!ModelName.isEmpty())
            mProject->SetActiveModel(ModelName);

        View* ActiveView = gMainWindow->GetActiveView();

        if (!CameraName.isEmpty())
        {
            ActiveView->SetCamera(CameraName.toLatin1()); // todo: qstring

            if (!ViewpointName.isEmpty())
                printf("Warning: --viewpoint is ignored when --camera is set.\n");

            if (Orthographic)
                printf("Warning: --orthographic is ignored when --camera is set.\n");

            if (SetCameraAngles)
                printf("Warning: --camera-angles is ignored when --camera is set.\n");
        }
        else
        {
            if (!ViewpointName.isEmpty())
            {
                if (ViewpointName == QLatin1String("front"))
                    ActiveView->SetViewpoint(LC_VIEWPOINT_FRONT);
                else if (ViewpointName == QLatin1String("back"))
                    ActiveView->SetViewpoint(LC_VIEWPOINT_BACK);
                else if (ViewpointName == QLatin1String("top"))
                    ActiveView->SetViewpoint(LC_VIEWPOINT_TOP);
                else if (ViewpointName == QLatin1String("bottom"))
                    ActiveView->SetViewpoint(LC_VIEWPOINT_BOTTOM);
                else if (ViewpointName == QLatin1String("left"))
                    ActiveView->SetViewpoint(LC_VIEWPOINT_LEFT);
                else if (ViewpointName == QLatin1String("right"))
                    ActiveView->SetViewpoint(LC_VIEWPOINT_RIGHT);
                else if (ViewpointName == QLatin1String("home"))
                    ActiveView->SetViewpoint(LC_VIEWPOINT_HOME);
                else
                    printf("Unknown viewpoint: '%s'\n", ViewpointName.toLatin1().constData());

                if (SetCameraAngles)
                    printf("Warning: --camera-angles is ignored when --viewpoint is set.\n");
            }
            else if (SetCameraAngles)
                ActiveView->SetCameraAngles(CameraLatitude, CameraLongitude);

            ActiveView->SetProjection(Orthographic);
        }

        if (SaveImage)
        {
            if (ImageName.isEmpty())
                ImageName = mProject->GetImageFileName();

            if (ImageEnd < ImageStart)
                ImageEnd = ImageStart;
            else if (ImageStart > ImageEnd)
                ImageStart = ImageEnd;

            if ((ImageStart == 0) && (ImageEnd == 0))
                ImageStart = ImageEnd = mProject->GetActiveModel()->GetCurrentStep();
            else if ((ImageStart == 0) && (ImageEnd != 0))
                ImageStart = ImageEnd;
            else if ((ImageStart != 0) && (ImageEnd == 0))
                ImageEnd = ImageStart;

            if (ImageStart > 255)
                ImageStart = 255;

            if (ImageEnd > 255)
                ImageEnd = 255;

            QString Frame;

            if (ImageStart != ImageEnd)
            {
                QString Extension = QFileInfo(ImageName).suffix();
                Frame = ImageName.left(ImageName.length() - Extension.length() - 1) + QLatin1String("%1.") + Extension;
            }
            else
                Frame = ImageName;

            lcGetActiveModel()->SaveStepImages(Frame, ImageStart != ImageEnd, CameraName == nullptr, ImageHighlight, ImageWidth, ImageHeight, ImageStart, ImageEnd);
        }

        if (SaveWavefront)
        {
            QString FileName;

            if (!SaveWavefrontName.isEmpty())
                FileName = SaveWavefrontName;
            else
                FileName = ProjectName;

            QString Extension = QFileInfo(FileName).suffix().toLower();

            if (Extension.isEmpty())
            {
                FileName += ".obj";
            }
            else if (Extension != "obj")
            {
                FileName = FileName.left(FileName.length() - Extension.length() - 1);
                FileName += ".obj";
            }

            mProject->ExportWavefront(FileName);
        }

        if (Save3DS)
        {
            QString FileName;

            if (!Save3DSName.isEmpty())
                FileName = Save3DSName;
            else
                FileName = ProjectName;

            QString Extension = QFileInfo(FileName).suffix().toLower();

            if (Extension.isEmpty())
            {
                FileName += ".3ds";
            }
            else if (Extension != "3ds")
            {
                FileName = FileName.left(FileName.length() - Extension.length() - 1);
                FileName += ".3ds";
            }

            mProject->Export3DStudio(FileName);
        }

        if (SaveCOLLADA)
        {
            QString FileName;

            if (!SaveCOLLADAName.isEmpty())
                FileName = SaveCOLLADAName;
            else
                FileName = ProjectName;

            QString Extension = QFileInfo(FileName).suffix().toLower();

            if (Extension.isEmpty())
            {
                FileName += ".dae";
            }
            else if (Extension != "dae")
            {
                FileName = FileName.left(FileName.length() - Extension.length() - 1);
                FileName += ".dae";
            }

            mProject->ExportCOLLADA(FileName);
        }

        if (SaveHTML)
        {
            lcHTMLExportOptions Options(mProject);

            if (!SaveHTMLName.isEmpty())
                Options.PathName = SaveHTMLName;

            if (PartImagesWidth > 0)
                Options.PartImagesWidth = PartImagesWidth;

            if (PartImagesHeight > 0)
                Options.PartImagesHeight = PartImagesHeight;

            mProject->ExportHTML(Options);
        }
    }

    if (ShowWindow)
    {
        gMainWindow->SetColorIndex(lcGetColorIndex(4));
        gMainWindow->GetPartSelectionWidget()->SetDefaultPart();
/*** LPub3D Mod - suppress recent files and mainwindow show ***/
//        gMainWindow->UpdateRecentFiles();
//        gMainWindow->show();
/*** LPub3D Mod end ***/
    }

    return true;
}

void lcApplication::Shutdown()
{
    delete mLibrary;
    mLibrary = nullptr;
}

void lcApplication::ShowPreferencesDialog()
{
    lcPreferencesDialogOptions Options;
    int CurrentAASamples = lcGetProfileInt(LC_PROFILE_ANTIALIASING_SAMPLES);

    Options.Preferences = mPreferences;

    Options.DefaultAuthor = lcGetProfileString(LC_PROFILE_DEFAULT_AUTHOR_NAME);
    Options.LibraryPath = lcGetProfileString(LC_PROFILE_PARTS_LIBRARY);
    Options.POVRayPath = lcGetProfileString(LC_PROFILE_POVRAY_PATH);
    Options.LGEOPath = lcGetProfileString(LC_PROFILE_POVRAY_LGEO_PATH);
    Options.CheckForUpdates = lcGetProfileInt(LC_PROFILE_CHECK_UPDATES);

    Options.AASamples = CurrentAASamples;

    Options.Categories = gCategories;
    Options.CategoriesModified = false;
    Options.CategoriesDefault = false;

    Options.KeyboardShortcuts = gKeyboardShortcuts;
    Options.KeyboardShortcutsModified = false;
    Options.KeyboardShortcutsDefault = false;
    Options.MouseShortcuts = gMouseShortcuts;
    Options.MouseShortcutsModified = false;
    Options.MouseShortcutsDefault = false;

    lcQPreferencesDialog Dialog(gMainWindow, &Options);
    if (Dialog.exec() != QDialog::Accepted)
        return;

    bool LibraryChanged = Options.LibraryPath != lcGetProfileString(LC_PROFILE_PARTS_LIBRARY);
    bool AAChanged = CurrentAASamples != Options.AASamples;

    mPreferences = Options.Preferences;

    mPreferences.SaveDefaults();

    lcSetProfileString(LC_PROFILE_DEFAULT_AUTHOR_NAME, Options.DefaultAuthor);
    lcSetProfileString(LC_PROFILE_PARTS_LIBRARY, Options.LibraryPath);
    lcSetProfileString(LC_PROFILE_POVRAY_PATH, Options.POVRayPath);
    lcSetProfileString(LC_PROFILE_POVRAY_LGEO_PATH, Options.LGEOPath);
    lcSetProfileInt(LC_PROFILE_CHECK_UPDATES, Options.CheckForUpdates);
    lcSetProfileInt(LC_PROFILE_ANTIALIASING_SAMPLES, Options.AASamples);

    if (LibraryChanged && AAChanged)
        QMessageBox::information(gMainWindow, tr("3DViewer"), tr("Parts library and Anti-aliasing changes will only take effect the next time you start LPub3D."));
    else if (LibraryChanged)
        QMessageBox::information(gMainWindow, tr("3DViewer"), tr("Parts library changes will only take effect the next time you start LPub3D."));
    else if (AAChanged)
        QMessageBox::information(gMainWindow, tr("3DViewer"), tr("Anti-aliasing changes will only take effect the next time you start LPub3D."));

    if (Options.CategoriesModified)
    {
        if (Options.CategoriesDefault)
            lcResetDefaultCategories();
        else
        {
            gCategories = Options.Categories;
            lcSaveDefaultCategories();
        }

        gMainWindow->UpdateCategories();
    }

    if (Options.KeyboardShortcutsModified)
    {
        if (Options.KeyboardShortcutsDefault)
            lcResetDefaultKeyboardShortcuts();
        else
        {
            gKeyboardShortcuts = Options.KeyboardShortcuts;
            lcSaveDefaultKeyboardShortcuts();
        }

        gMainWindow->UpdateShortcuts();
    }

    if (Options.MouseShortcutsModified)
    {
        if (Options.MouseShortcutsDefault)
            lcResetDefaultMouseShortcuts();
        else
        {
            gMouseShortcuts = Options.MouseShortcuts;
            lcSaveDefaultMouseShortcuts();
        }
    }

    // TODO: printing preferences
    /*
    strcpy(opts.strFooter, m_strFooter);
    strcpy(opts.strHeader, m_strHeader);
    */

    gMainWindow->SetShadingMode(Options.Preferences.mShadingMode);
    gMainWindow->UpdateAllViews();
}
