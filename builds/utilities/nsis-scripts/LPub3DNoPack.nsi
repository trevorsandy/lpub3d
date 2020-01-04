;LPub3D Setup Script
;Last Update: Aug 03, 2019
;Copyright (C) 2016 - 2020 by Trevor SANDY

; Install LPub3D and pre-packaged renderers.

!addplugindir /x86-ansi ".\Plugins\x86-ansi"
!addplugindir /x86-unicode ".\Plugins\x86-unicode"
!addincludedir ".\Include"

!include NsisMultiUser.nsh
!include MUI2.nsh
!include LogicLib.nsh
!include Registry.nsh
!include StdUtils.nsh
; !include Utils.nsh

;--------------------------------
;generated define statements
!include AppVersion.nsh

; Global variables
Var /global CPUArch
Var /global X64Flag
Var /global StartMenuFolder
Var /global CaptionMessage
Var /global AppDataBaseDir

; Custom page variables
Var /global nsDialogFilePathsPage
Var /global nsDialogFilePathsPage_Font1

Var /global LDrawDirPath
Var /global BrowseLDraw
Var /global LDrawText

Var /global ParameterFile
Var /global InstallUserData

Var /global UserDataLbl
Var /global UserDataInstallChkBox

Var /global DeleteOldUserDataDirectoryChkBox
Var /global CopyExistingUserDataLibrariesChkBox
Var /global OverwriteUserDataParamFilesChkBox

Var /global DeleteOldUserDataDirectory
Var /global CopyExistingUserDataLibraries
Var /global OverwriteUserDataParamFiles

Var /global LibrariesExist
Var /global ParameterFilesExist
Var /global OldLibraryDirectoryExist

Var /global LPub3DViewerLibFile
Var /global LPub3DViewerLibPath

; Installer defines
!define PRODUCT_NAME "${ProductName}" ; name of the application as displayed to the user
!define PROGEXE "${LPub3DBuildFile}" ; main application filename
!define COMPANY_NAME "${Company}" ; company, used for registry tree hierarchy
!define LICENSE_FILE "COPYING.txt" ; license file, optional
!define README_FILE "README.txt" ; readme file, optional
!define RELEASE_NOTES_FILE "RELEASE_NOTES.html" ; local release notes file, optional
!define RELEASE_NOTES_URL "https://lpub3d.sourceforge.io/${ReleaseNotes}" ; hosted release notes file, optional
!define PUBLISHER_NAME "${Publisher}"
!define COMPANY_URL "${CompanyURL}"
!define SUPPORT "${SupportURL}"
!define VERSION_MAJOR "${VersionMajor}"
!define VERSION_MINOR "${VersionMinor}"
!define MIN_WIN_VER "XP"
!define PLATFORM "$CPUArch"
!define SETUP_MUTEX "${COMPANY_NAME} ${PRODUCT_NAME} Setup Mutex" ; do not change this between program versions!
!define APP_MUTEX "${COMPANY_NAME} ${PRODUCT_NAME} App Mutex" ; do not change this between program versions!
!define SETTINGS_REG_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}"

; NsisMultiUser optional defines
!define MULTIUSER_INSTALLMODE_ALLOW_BOTH_INSTALLATIONS 0
!define MULTIUSER_INSTALLMODE_ALLOW_ELEVATION 1
!define MULTIUSER_INSTALLMODE_ALLOW_ELEVATION_IF_SILENT 0
!define MULTIUSER_INSTALLMODE_DEFAULT_ALLUSERS 1
!define MULTIUSER_INSTALLMODE_64_BIT $X64Flag
!define MULTIUSER_INSTALLMODE_DISPLAYNAME "${PRODUCT_NAME} ${VERSION} ${PLATFORM}"

; Set LPub3D user data path
!define INSTDIR_LocalAppData $AppDataBaseDir
!define INSTDIR_AppDataProduct "$AppDataBaseDir\${COMPANY_NAME}\${PRODUCT_NAME}"

; Installer Attributes
Name "${PRODUCT_NAME} ${VERSION} Rev ${BuildRevision} Build ${BuildNumber} ${PLATFORM}"
Caption $CaptionMessage
Icon "..\icons\setup.ico"
UninstallIcon "..\icons\setup.ico"
BrandingText "©2020 ${COMPANY_NAME}"
!ifdef UpdateMaster
  OutFile "${OutFileDir}\${PRODUCT_NAME}-UpdateMaster_${VERSION}.exe"
!else
  OutFile "${OutFileDir}\${PRODUCT_NAME}-${CompleteVersion}.exe"
!endif

AllowSkipFiles off
SetOverwrite on ; (default setting) set to on except for where it is manually switched off
ShowInstDetails hide ; Show install details (show|hide|nevershow)
SetCompressor /SOLID lzma

!include Utils.nsh ; this must come after encoding (Error: Can't change target charset after data already got compressed or header already changed!)

; Pages
!define MUI_ICON "..\icons\setup.ico"
!define MUI_UNICON "..\icons\setup.ico"
!define MUI_WELCOMEFINISHPAGE_BITMAP "..\icons\welcome.bmp"

!define MUI_ABORTWARNING ; Show a confirmation when cancelling the installation

!define MUI_PAGE_CUSTOMFUNCTION_PRE PageWelcomeLicensePre
!insertmacro MUI_PAGE_WELCOME

!ifdef LICENSE_FILE
  !define MUI_PAGE_CUSTOMFUNCTION_PRE PageWelcomeLicensePre
  !insertmacro MUI_PAGE_LICENSE "${WinBuildDir}\docs\${LICENSE_FILE}"
!endif

!define MUI_PAGE_CUSTOMFUNCTION_PRE PageReadmePre
!define MUI_PAGE_CUSTOMFUNCTION_SHOW PageReadmeShow
!define MUI_PAGE_HEADER_TEXT "Readme File"
!define MUI_PAGE_HEADER_SUBTEXT "Please review the readme file before installing ${PRODUCT_NAME} ${VERSION} ${PLATFORM}."
!define MUI_LICENSEPAGE_TEXT_TOP "Press Page Down to see the rest of the readme file."
!define MUI_LICENSEPAGE_TEXT_BOTTOM "When you have finished reading, click on Next to continue the installation."
!insertmacro MUI_PAGE_LICENSE Empty.txt

!define MULTIUSER_INSTALLMODE_CHANGE_MODE_FUNCTION PageInstallModeChangeMode
!insertmacro MULTIUSER_PAGE_INSTALLMODE

!define MUI_COMPONENTSPAGE_SMALLDESC
!define MUI_PAGE_CUSTOMFUNCTION_PRE PageComponentsPre
!insertmacro MUI_PAGE_COMPONENTS

!define MUI_PAGE_CUSTOMFUNCTION_PRE PageDirectoryPre
!define MUI_PAGE_CUSTOMFUNCTION_SHOW PageDirectoryShow
!insertmacro MUI_PAGE_DIRECTORY

;Custom page, Initialize library settings for smoother install.
Page custom nsDialogShowCustomPage nsDialogLeaveCustomPage

!define MUI_STARTMENUPAGE_NODISABLE ; Do not display the checkbox to disable the creation of Start Menu shortcuts
!define MUI_STARTMENUPAGE_DEFAULTFOLDER "${PRODUCT_NAME}"
!define MUI_STARTMENUPAGE_REGISTRY_ROOT "SHCTX" ; writing to $StartMenuFolder happens in MUI_STARTMENU_WRITE_END, so it's safe to use "SHCTX" here
!define MUI_STARTMENUPAGE_REGISTRY_KEY "${SETTINGS_REG_KEY}"
!define MUI_STARTMENUPAGE_REGISTRY_VALUENAME "StartMenuFolder"
!define MUI_PAGE_CUSTOMFUNCTION_PRE PageStartMenuPre
!insertmacro MUI_PAGE_STARTMENU "" "$StartMenuFolder"
!define MUI_STARTMENUPAGE_DEFAULTFOLDER "${PRODUCT_NAME}" ; the MUI_PAGE_STARTMENU macro undefines MUI_STARTMENUPAGE_DEFAULTFOLDER, but we need it

!define MUI_PAGE_CUSTOMFUNCTION_SHOW PageInstFilesPre
!insertmacro MUI_PAGE_INSTFILES

!define MUI_FINISHPAGE_NOAUTOCLOSE
!define MUI_FINISHPAGE_RUN
!define MUI_FINISHPAGE_RUN_FUNCTION PageFinishRun
!define MUI_FINISHPAGE_LINK "${CompanyURL}"
!define MUI_FINISHPAGE_LINK_LOCATION "${CompanyURL}"
!insertmacro MUI_PAGE_FINISH

; remove next line if you're using signing after the uninstaller is extracted from the initially compiled setup
!include Uninstall.nsh

!insertmacro MUI_LANGUAGE "English" ; Set languages (first is default language) - must be inserted after all pages

;Language strings
LangString CUST_PAGE_TITLE ${LANG_ENGLISH} "LDraw Library"
LangString CUST_PAGE_SUBTITLE ${LANG_ENGLISH} "Enter path for your LDraw directory and select user data options"
LangString CUST_PAGE_OVERWRITE_TITLE ${LANG_ENGLISH} "Overwrite Configuration Files"
LangString CUST_PAGE_OVERWRITE_SUBTITLE ${LANG_ENGLISH} "Check the box next to the configuration file you would like to overwrite."

;backup legacy registry key
!insertmacro COPY_REGISTRY_KEY

InstType "Typical"
InstType "Minimal"
InstType "Full"

Section "Core Files (required)" SectionCoreFiles
  SectionIn 1 2 3 RO

  ; if there's an installed version, uninstall it first (I chose not to start the uninstaller silently, so that user sees what failed)
  ; if both per-user and per-machine versions are installed, unistall the one that matches $MultiUser.InstallMode
  StrCpy $0 ""
  ${if} $HasCurrentModeInstallation == 1
    StrCpy $0 "$MultiUser.InstallMode"
  ${else}
    !if ${MULTIUSER_INSTALLMODE_ALLOW_BOTH_INSTALLATIONS} == 0
      ${if} $HasPerMachineInstallation == 1
        StrCpy $0 "AllUsers"    ; if there's no per-user installation, but there's per-machine installation, uninstall it
      ${elseif} $HasPerUserInstallation == 1
        StrCpy $0 "CurrentUser" ; if there's no per-machine installation, but there's per-user installation, uninstall it
      ${endif}
    !endif
  ${endif}

  ${if} "$0" != ""

    ;Set InstallString, InstallationFolder
    ${if} $0 == "AllUsers"
      StrCpy $1 "$PerMachineUninstallString"
      StrCpy $3 "$PerMachineInstallationFolder"
    ${else}
      StrCpy $1 "$PerUserUninstallString"
      StrCpy $3 "$PerUserInstallationFolder"
    ${endif}

    ${if} ${silent}
      StrCpy $2 "/S"
    ${else}
      StrCpy $2 ""
    ${endif}

    ;Remove previous installation
    DetailPrint 'UninstallString $1'
    DetailPrint 'InstallationFolder $3'

    ; Legacy uninstall will delete everything so backup user data and registry keys
    ${if} $HasLegacyPerMachineInstallation == 1
      DetailPrint "Removing legacy install - using mode ($0)..."
      StrCpy $5 "${INSTDIR_LocalAppData}\${COMPANY_NAME}\${PRODUCT_NAME}" ; $5 = LEGACY_INSTDIR_AppDataProduct
      StrCpy $6 "${INSTDIR_LocalAppData}\Temp\${PRODUCT_NAME}"            ; $6 = LEGACY_INSTDIR_AppDataProduct_Backup
      DetailPrint "Backup files '$5' to '$6'..."
      !insertmacro BackupFolder "$5" "$6" ; Backup Legacy Current User Data
      ${COPY_REGISTRY_KEY} HKCU "Software\${COMPANY_NAME}" HKCU "Software\LPUB3D_BACKUP\${COMPANY_NAME}" ; backup Legacy Current User Hive Key
      ; manually copy uninstaller to temp dir so it can be deleted in the install folder
      InitPluginsDir
      DetailPrint "Copy file '$PerMachineInstallationFolder\Uninstall.exe' to '$pluginsdir\uninst\'..."
      CreateDirectory "$pluginsdir\uninst"
      CopyFiles /SILENT /FILESONLY "$PerMachineInstallationFolder\Uninstall.exe" "$pluginsdir\uninst\"
      StrCpy $1 "$pluginsdir\uninst\Uninstall.exe"
    ${endif}

    HideWindow
    ClearErrors
    StrCpy $0 0
    DetailPrint "Uninstall Command: ExecWait '$1 /SS $2 _?=$3'"
    ExecWait '$1 /SS $2 _?=$3' $0 ; $1 is quoted in registry; the _? param stops the uninstaller from copying itself to the temporary directory, which is the only way for ExecWait to work

    ${if} ${errors} ; stay in installer
      SetErrorLevel 2 ; Installation aborted by script
      BringToFront
	  Call fnAskForceUninstall
    ${else}
      ${Switch} $0
        ${Case} 0 ; uninstaller completed successfully - continue with installation
          BringToFront
          ${Break}
        ${Case} 1 ; Installation aborted by user (cancel button)
        ${Case} 2 ; Installation aborted by script
          SetErrorLevel $0
          Quit ; uninstaller was started, but completed with errors - Quit installer
        ${Default} ; all other error codes - uninstaller could not start, elevate, etc. - Abort installer
          SetErrorLevel $0
          BringToFront
          ;Abort "Error executing uninstaller."
		  Call fnAskForceUninstall
      ${EndSwitch}
    ${endif}

    ; Restore legacy user data and registry keys
    ${if} $HasLegacyPerMachineInstallation == 1
        DetailPrint "Restore files '$6' to '$5'..."
        !insertmacro RestoreFolder "$6" "$5" ; Restore LEGACY_INSTDIR_AppDataProduct_Backup
        ; Restore Reg from Current User Hive Key to Context Hive Key
        ${COPY_REGISTRY_KEY} HKCU "Software\LPUB3D_BACKUP\${COMPANY_NAME}" HKCU "Software\${COMPANY_NAME}"
        ; Cleanup legacy user data backup
        DeleteRegKey HKCU "Software\LPUB3D_BACKUP"; Delete Current User Hive Key
        ; Cleanup unused registry values
        DeleteRegKey HKCU "Software\${COMPANY_NAME}\${PRODUCT_NAME}\Installation"
        DeleteRegValue HKCU "Software\${COMPANY_NAME}\${PRODUCT_NAME}\POVRay" "L3P"
        DeleteRegValue HKCU "Software\${COMPANY_NAME}\${PRODUCT_NAME}\POVRay" "POVRayFileGenerator"
        DeleteRegValue HKCU "Software\${COMPANY_NAME}\${PRODUCT_NAME}\Settings" "LDGLite"
        DeleteRegValue HKCU "Software\${COMPANY_NAME}\${PRODUCT_NAME}\Settings" "LDView"
        DeleteRegValue HKCU "Software\${COMPANY_NAME}\${PRODUCT_NAME}\Settings" "POVRay"
        ; Cleanup erroneously deposited legacy files
        Delete "$PerMachineInstallationFolder\fadeStepColorParts.lst"
    ${endif}

    Delete "$2\${UNINSTALL_FILENAME}" ; the uninstaller doesn't delete itself when not copied to the temp directory
    RMDir "$2"
  ${endif}

  ; Set the install directory path and create it (recursively if necessary), if it does not exist.
  SetOutPath $INSTDIR

  ; Write uninstaller and registry uninstall info as the first step,
  ; so that the user has the option to run the uninstaller if sth. goes wrong
  WriteUninstaller "${UNINSTALL_FILENAME}"
  ; or this if you're using signing:
  ; File "${UNINSTALL_FILENAME}"
  !insertmacro MULTIUSER_RegistryAddInstallInfo ; add registry keys

  ; Core files to be installed.
  !include "LPub3DInstallFiles.nsh"

  ;Install MSVC 2015 Redistributable
  ${If} ${RunningX64}
     ExecWait '"vcredist_x86_64.exe"  /quiet /norestart'
  ${Else}
     ExecWait '"vcredist_x86.exe"  /quiet /norestart'
  ${EndIf}

  !ifdef LICENSE_FILE
    File "${WinBuildDir}\docs\${LICENSE_FILE}"
  !endif

  ;LPub3D User data setup
  ${If} $InstallUserData == 1       # install user data

    ;ldraw libraries - user data location
    CreateDirectory "${INSTDIR_AppDataProduct}\libraries"

    ${If} $CopyExistingUserDataLibraries == 1
      Call fnCopyLibraries
    ${Else}
      Call fnInstallLibraries
    ${EndIf}

    ${If} $DeleteOldUserDataDirectory == 1
      ${AndIf} ${DirExists} $LPub3DViewerLibPath
          RMDir /r $LPub3DViewerLibPath
    ${EndIf}

    ;extras contents
    CreateDirectory "${INSTDIR_AppDataProduct}\extras"
    SetOutPath "${INSTDIR_AppDataProduct}\extras"
    File "${WinBuildDir}\extras\PDFPrint.jpg"
    File "${WinBuildDir}\extras\pli.mpd"

    ${If} $OverwriteUserDataParamFiles == 0
      ; Always overwrite fadeStepColorParts.lst but backup first
      IfFileExists "${INSTDIR_AppDataProduct}\extras\fadeStepColorParts.lst" 0 +2
      !insertmacro BackupFile "${INSTDIR_AppDataProduct}\extras" "fadeStepColorParts.lst" "${INSTDIR_AppDataProduct}\extras\${PRODUCT_NAME}.${MyTIMESTAMP}.bak"
      SetOverwrite on
      File "${WinBuildDir}\extras\fadeStepColorParts.lst"
      SetOverwrite off
      File "${WinBuildDir}\extras\excludedParts.lst"
      File "${WinBuildDir}\extras\freeformAnnotations.lst"
      File "${WinBuildDir}\extras\LDConfig.ldr"
      File "${WinBuildDir}\extras\pli.mpd"
      File "${WinBuildDir}\extras\pliSubstituteParts.lst"
      File "${WinBuildDir}\extras\titleAnnotations.lst"
    ${Else}
      SetOverwrite on
      File "${WinBuildDir}\extras\excludedParts.lst"
      File "${WinBuildDir}\extras\fadeStepColorParts.lst"
      File "${WinBuildDir}\extras\freeformAnnotations.lst"
      File "${WinBuildDir}\extras\LDConfig.ldr"
      File "${WinBuildDir}\extras\pli.mpd"
      File "${WinBuildDir}\extras\pliSubstituteParts.lst"
      File "${WinBuildDir}\extras\titleAnnotations.lst"
    ${EndIf}

    ;Store/Update library folder
    DetailPrint "LPub3D PartsLibrary Path ${INSTDIR_AppDataProduct}\libraries\complete.zip"
    WriteRegStr HKCU "Software\${COMPANY_NAME}\${PRODUCT_NAME}\Settings" "PartsLibrary" "${INSTDIR_AppDataProduct}\libraries\complete.zip"

  ${Else}               # do not install user data (backup and write new version of fadeStepColorParts.lst if already exist)
    IfFileExists "${INSTDIR_AppDataProduct}\extras\fadeStepColorParts.lst" 0 GoToEnd
    !insertmacro BackupFile "${INSTDIR_AppDataProduct}\extras" "fadeStepColorParts.lst" "${INSTDIR_AppDataProduct}\extras\${PRODUCT_NAME}.${MyTIMESTAMP}.bak"
    SetOutPath "${INSTDIR_AppDataProduct}\extras"
    SetOverwrite on
    File "${WinBuildDir}\extras\fadeStepColorParts.lst"
    GoToEnd:
  ${EndIf}
SectionEnd

Section "Documentation" SectionDocumentation
  SectionIn 1 3

  SetOutPath $INSTDIR
  File "${WinBuildDir}\docs\${README_FILE}"
  File "${WinBuildDir}\docs\${RELEASE_NOTES_FILE}"

SectionEnd

SectionGroup /e "Integration" SectionGroupIntegration

Section "Program Group" SectionProgramGroup
  SectionIn 1 3

  !insertmacro MUI_STARTMENU_WRITE_BEGIN ""

    CreateDirectory "$SMPROGRAMS\$StartMenuFolder"
    CreateShortCut "$SMPROGRAMS\$StartMenuFolder\${PRODUCT_NAME}.lnk" "$INSTDIR\${PROGEXE}"

    !ifdef LICENSE_FILE
      CreateShortCut "$SMPROGRAMS\$StartMenuFolder\License Agreement.lnk" "$INSTDIR\${LICENSE_FILE}"
    !endif
    ${if} $MultiUser.InstallMode == "AllUsers"
      CreateShortCut "$SMPROGRAMS\$StartMenuFolder\Uninstall.lnk" "$INSTDIR\${UNINSTALL_FILENAME}" "/allusers"
    ${else}
      CreateShortCut "$SMPROGRAMS\$StartMenuFolder\Uninstall (current user).lnk" "$INSTDIR\${UNINSTALL_FILENAME}" "/currentuser"
    ${endif}

  !insertmacro MUI_STARTMENU_WRITE_END
SectionEnd

Section "Desktop Icon" SectionDesktopIcon
  SectionIn 1 3

  !insertmacro MULTIUSER_GetCurrentUserString $0
  CreateShortCut "$DESKTOP\${PRODUCT_NAME}.lnk" "$INSTDIR\${PROGEXE}"
SectionEnd

Section /o "Start Menu Icon" SectionStartMenuIcon
  SectionIn 3
  
  !insertmacro MULTIUSER_GetCurrentUserString $0
  CreateShortCut "$STARTMENU\${PRODUCT_NAME}.lnk" "$INSTDIR\${PROGEXE}"
SectionEnd

Section /o "Quick Launch Icon" SectionQuickLaunchIcon
    SectionIn 3

    ${if} ${AtLeastWin7}
        ${StdUtils.InvokeShellVerb} $0 "$INSTDIR" "${PROGEXE}" ${StdUtils.Const.ShellVerb.PinToTaskbar}
    ${else}
        ; The $QUICKLAUNCH folder is always only for the current user
        CreateShortCut "$QUICKLAUNCH\${PRODUCT_NAME}.lnk" "$INSTDIR\${PROGEXE}"
    ${endif}
SectionEnd
SectionGroupEnd ; end 'Integrtion' group

Section "-Write Install Size" ; hidden section, write install size as the final step
  ;Write installed size
  !insertmacro MULTIUSER_RegistryAddInstallSizeInfo
SectionEnd

; Modern install component descriptions
!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
  !insertmacro MUI_DESCRIPTION_TEXT ${SectionCoreFiles} "Core files requred to run ${PRODUCT_NAME}."
  !insertmacro MUI_DESCRIPTION_TEXT ${SectionDocumentation} "README file for ${PRODUCT_NAME}."

  !insertmacro MUI_DESCRIPTION_TEXT ${SectionGroupIntegration} "Select how to integrate the program in Windows."
  !insertmacro MUI_DESCRIPTION_TEXT ${SectionProgramGroup} "Create a ${PRODUCT_NAME} program group under Start Menu > Programs."
  !insertmacro MUI_DESCRIPTION_TEXT ${SectionDesktopIcon} "Create ${PRODUCT_NAME} icon on the Desktop."
  !insertmacro MUI_DESCRIPTION_TEXT ${SectionStartMenuIcon} "Create ${PRODUCT_NAME} icon in the Start Menu."
  !insertmacro MUI_DESCRIPTION_TEXT ${SectionQuickLaunchIcon} "Create ${PRODUCT_NAME} icon in Quick Launch."
!insertmacro MUI_FUNCTION_DESCRIPTION_END

; Callbacks
Function .onInit
   ; Set the MULTIUSER_INSTALLMODE_64_BIT flag
   ${If} ${RunningX64}
     StrCpy $X64Flag 1
     StrCpy $CPUArch "Win64"
   ${Else}
     StrCpy $X64Flag 0
     StrCpy $CPUArch "Win32"
   ${EndIf}

  !insertmacro CheckPlatform ${PLATFORM}
  !insertmacro CheckMinWinVer ${MIN_WIN_VER}

  ${ifnot} ${UAC_IsInnerInstance}
    !insertmacro CheckSingleInstance "Setup" "Global" "${SETUP_MUTEX}"
    !insertmacro CheckSingleInstance "Application" "Local" "${APP_MUTEX}"
  ${endif}

  ;LPub3D Mod, Set context to 'CurrentUser', capture $AppDataBaseDir then revert context to 'AllUsers' if previously set as such
  StrCpy $R0 "$SMPROGRAMS"
  SetShellVarContext current
  StrCpy $AppDataBaseDir "$LOCALAPPDATA"
  StrCmp $R0 "$SMPROGRAMS" 0 SetAll
  Goto Done
  SetAll:
  SetShellVarContext all
  Done:

  !insertmacro MULTIUSER_INIT

  ; LPub3D Directives
  ; -------------------------------------
  
  ;Get Ldraw library folder, archive file paths, and uninstall string from registry if available
  ReadRegStr $LDrawDirPath HKCU "Software\${COMPANY_NAME}\${PRODUCT_NAME}\Settings" "LDrawDir"
  ReadRegStr $LPub3DViewerLibFile HKCU "Software\${COMPANY_NAME}\${PRODUCT_NAME}\Settings" "PartsLibrary"
  ReadRegStr $ParameterFile HKCU "Software\${COMPANY_NAME}\${PRODUCT_NAME}\Settings" "TitleAnnotationFile"

  ; Set caption according to architecture
  StrCmp ${UniversalBuild} "1" 0 SignleArchBuild
  StrCpy $CaptionMessage "${PRODUCT_NAME} 32,64bit Setup"
  GoTo InitDataVars

  SignleArchBuild:
  StrCmp ${ArchExt} "x64" 0 +2
  StrCpy $CaptionMessage "${PRODUCT_NAME} 64bit Setup"
  StrCpy $CaptionMessage "${PRODUCT_NAME} 32bit Setup"

  InitDataVars:
  ;Initialize user data vars
  Call fnInitializeUserDataVars

  ;Verify old library directory exist - and is not the same as new library directory
  Push $LPub3DViewerLibFile
  Call fnGetParent
  Pop $R0
  StrCpy $LPub3DViewerLibPath $R0
  ${If} ${DirExists} $LPub3DViewerLibPath
    Call fnVerifyDeleteDirectory
  ${EndIf}

  ;Verify if library files are installed - just check one
  IfFileExists $LPub3DViewerLibFile 0 checkParamFiles
  StrCpy $LibrariesExist 1

  checkParamFiles:
  ;Verify if parameter files are installed - just check one
  IfFileExists $ParameterFile 0 finished
  StrCpy $ParameterFilesExist 1

  finished:
FunctionEnd

Function PageWelcomeLicensePre
  ${if} $InstallShowPagesBeforeComponents == 0
    Abort ; don't display the Welcome and License pages for the inner instance
  ${endif}
FunctionEnd

Function PageReadmePre
  ${if} $InstallShowPagesBeforeComponents == 0
    Abort ; don't display the Readme page for the inner instance
  ${endif}
  GetDlgItem $0 $HWNDPARENT 1                          ;Agree/Next/Install button           
  SendMessage $0 ${WM_SETTEXT} 0 "STR:$(^NextBtn)"     ;Change 'I Agree' to 'Next'
FunctionEnd

Function PageReadmeShow
  FindWindow $R0 `#32770` `` $HWNDPARENT               ;Finds the inner dialog
  GetDlgItem $R0 $R0 1000
  EmbedHTML::Load /replace $R0 `${RELEASE_NOTES_URL}`  ;Set HTML release notes
FunctionEnd

Function PageInstallModeChangeMode
  !insertmacro MUI_STARTMENU_GETFOLDER "" $StartMenuFolder
FunctionEnd

Function PageComponentsPre
    GetDlgItem $0 $HWNDPARENT 1
    SendMessage $0 ${BCM_SETSHIELD} 0 0 ; hide SHIELD (Windows Vista and above)

    ${if} $MultiUser.InstallMode == "AllUsers"
        ${if} ${AtLeastWin7} ; add "(current user only)" text to section "Start Menu Icon"
            SectionGetText ${SectionStartMenuIcon} $0
            SectionSetText ${SectionStartMenuIcon} "$0 (current user only)"
        ${endif}

        ; add "(current user only)" text to section "Quick Launch Icon"
        SectionGetText ${SectionQuickLaunchIcon} $0
        SectionSetText ${SectionQuickLaunchIcon} "$0 (current user only)"
    ${endif}
FunctionEnd

Function PageDirectoryPre
  GetDlgItem $0 $HWNDPARENT 1
  ${if} ${SectionIsSelected} ${SectionProgramGroup}
    SendMessage $0 ${WM_SETTEXT} 0 "STR:$(^NextBtn)" ; this is not the last page before installing
  ${else}
    SendMessage $0 ${WM_SETTEXT} 0 "STR:$(^InstallBtn)" ; this is the last page before installing
  ${endif}
FunctionEnd

Function PageDirectoryShow
  ${if} $CmdLineDir != ""
    FindWindow $R1 "#32770" "" $HWNDPARENT

    GetDlgItem $0 $R1 1019 ; Directory edit
    SendMessage $0 ${EM_SETREADONLY} 1 0 ; read-only is better than disabled, as user can copy contents

    GetDlgItem $0 $R1 1001 ; Browse button
    EnableWindow $0 0
  ${endif}
FunctionEnd

Function PageStartMenuPre
    ${ifnot} ${SectionIsSelected} ${SectionProgramGroup}
        Abort ; don't display this dialog if SectionProgramGroup is not selected
    ${else}
        GetDlgItem $1 $HWNDPARENT 1
        Call MultiUser.CheckPageElevationRequired
        ${if} $0 = 2
            SendMessage $1 ${BCM_SETSHIELD} 0 1 ; display SHIELD (Windows Vista and above)
        ${endif}
    ${endif}
FunctionEnd

Function PageInstFilesPre
    GetDlgItem $0 $HWNDPARENT 1
    SendMessage $0 ${BCM_SETSHIELD} 0 0 ; hide SHIELD (Windows Vista and above)
FunctionEnd

Function PageFinishRun
  ; the installer might exit too soon before the application starts and it loses the right to be the foreground window and starts in the background
  ; however, if there's no active window when the application starts, it will become the active window, so we hide the installer
  HideWindow
  ; the installer will show itself again quickly before closing (w/o Taskbar button), we move it offscreen
  !define SWP_NOSIZE 0x0001
  !define SWP_NOZORDER 0x0004
  System::Call "User32::SetWindowPos(i, i, i, i, i, i, i) b ($HWNDPARENT, 0, -1000, -1000, 0, 0, ${SWP_NOZORDER}|${SWP_NOSIZE})"

  !insertmacro UAC_AsUser_ExecShell "open" "$INSTDIR\${PROGEXE}" "" "$INSTDIR" ""
FunctionEnd

Function .onInstFailed
  MessageBox MB_ICONSTOP "${PRODUCT_NAME} ${VERSION} could not be fully installed.$\r$\nPlease, restart Windows and run the setup program again." /SD IDOK
FunctionEnd

; LPub3D Custom Functions
;--------------------------------

;Custom nsDialogFilePathsPage to Capture Libraries
Function nsDialogShowCustomPage
  ;Display the InstallOptions nsDialogFilePathsPage
  !insertmacro MUI_HEADER_TEXT $(CUST_PAGE_TITLE) $(CUST_PAGE_SUBTITLE)

  #Create nsDialogFilePathsPage and quit if error
  nsDialogs::Create 1018
  Pop $nsDialogFilePathsPage

  ${If} $nsDialogFilePathsPage == error
      Abort
  ${EndIf}

  ; custom font definition
  CreateFont $nsDialogFilePathsPage_Font1 "Microsoft Sans Serif" "7.25" "400"

  ; === UserDataLbl (type: Label) ===
  ${NSD_CreateLabel} 7.9u 0.62u 280.41u 14.15u ""
  Pop $UserDataLbl
  SendMessage $UserDataLbl ${WM_SETFONT} $nsDialogFilePathsPage_Font1 0
  SetCtlColors $UserDataLbl 0xFF0000 0xF0F0F0

  ; === grpBoxPaths (type: GroupBox) ===
  ${NSD_CreateGroupBox} 7.9u 16.62u 281.72u 35.08u "Define LDraw Library Path"

  ; === LDrawText (type: Text) ===
  ${NSD_CreateText} 17.11u 30.15u 213.92u 12.31u "$LDrawDirPath"
  Pop $LDrawText

  ; === BrowseLDraw (type: Button) ===
  ${NSD_CreateButton} 234.99u 28.92u 49.37u 14.15u "Browse"
  Pop $BrowseLDraw

  ; === UserDataInstallChkBox (type: Checkbox) ===
  ${NSD_CreateCheckbox} 17.11u 67.08u 267.24u 14.77u "Check to install user data now or uncheck to install at first application launch"
  Pop $UserDataInstallChkBox

  ; === DeleteOldUserDataDirectoryChkBox (type: Checkbox) ===
  ${NSD_CreateCheckbox} 17.11u 108.31u 134.28u 14.77u "Delete old archive library directory"
  Pop $DeleteOldUserDataDirectoryChkBox

  ; === OverwriteUserDataParamFilesChkBox (type: Checkbox) ===
  ${NSD_CreateCheckbox} 17.11u 80.62u 134.28u 14.77u "Overwrite existing parameter files"
  Pop $OverwriteUserDataParamFilesChkBox

  ; === CopyExistingUserDataLibrariesChkBox (type: Checkbox) ===
  ${NSD_CreateCheckbox} 17.11u 94.15u 134.28u 14.77u "Use existing LDraw archive libraries"
  Pop $CopyExistingUserDataLibrariesChkBox

  ; === grpBoxUserDataOptions (type: GroupBox) ===
  ${NSD_CreateGroupBox} 7.9u 55.38u 281.72u 77.54u "User Data Options"

  ${NSD_OnClick} $BrowseLDraw fnBrowseLDraw
  ${NSD_OnClick} $UserDataInstallChkBox fnInstallUserData
  ${NSD_OnClick} $DeleteOldUserDataDirectoryChkBox fnDeleteOldUserDataDirectory
  ${NSD_OnClick} $OverwriteUserDataParamFilesChkBox fnOverwriteUserDataParamFiles
  ${NSD_OnClick} $CopyExistingUserDataLibrariesChkBox fnCopyExistingUserDataLibraries

  Call fnShowUserDataLibraryDelete
  Call fnShowUserDataParamFilesManagement
  Call fnShowUserDataLibraryManagement

  nsDialogs::Show
FunctionEnd

Function fnBrowseLDraw
  nsDialogs::SelectFolderDialog "Select LDraw Directory" $LDrawDirPath
  Pop $LDrawDirPath
  ${NSD_SetText} $LDrawText $LDrawDirPath
FunctionEnd

Function fnInstallUserData
  Pop $UserDataInstallChkBox
  ${NSD_GetState} $UserDataInstallChkBox $InstallUserData
  ${If} $InstallUserData == 1
    Call fnUserDataInfo
  ${Else}
    ${NSD_SetText} $UserDataLbl ""
  ${EndIf}
FunctionEnd

Function nsDialogLeaveCustomPage
  ;Validate InstallUserData
  ${NSD_GetState} $UserDataInstallChkBox $InstallUserData
  ${NSD_GetState} $DeleteOldUserDataDirectoryChkBox $DeleteOldUserDataDirectory
  ${NSD_GetState} $OverwriteUserDataParamFilesChkBox $OverwriteUserDataParamFiles
  ${NSD_GetState} $CopyExistingUserDataLibrariesChkBox $CopyExistingUserDataLibraries
  ;MessageBox MB_ICONEXCLAMATION "InstallUserData (nsDialogLeaveCustomPage) = $InstallUserData" IDOK 0

  ;Validate the LDraw Directory path
  ${If} ${DirExists} $LDrawDirPath
    ; Update the registry with the LDraw Directory path.
    WriteRegStr HKCU "Software\${COMPANY_NAME}\${PRODUCT_NAME}\Settings" "LDrawDir" $LDrawDirPath
    Goto Continue
  ${Else}
    MessageBox MB_ICONEXCLAMATION|MB_YESNO "A valid LDraw Directory was not detected.$\r$\nDo you want to continue?" IDYES Continue
    Abort
  ${EndIf}

  Continue:
FunctionEnd

Function fnDeleteOldUserDataDirectory
  Pop $DeleteOldUserDataDirectoryChkBox
  ${NSD_GetState} $DeleteOldUserDataDirectoryChkBox $DeleteOldUserDataDirectory
  ${If} $DeleteOldUserDataDirectory == 1
    ${NSD_GetState} $CopyExistingUserDataLibrariesChkBox $CopyExistingUserDataLibraries
    ${If} $CopyExistingUserDataLibraries <> 1
      Call fnDeleteDirectoryWarning
    ${EndIf}
  ${Else}
    ${NSD_GetState} $CopyExistingUserDataLibrariesChkBox $CopyExistingUserDataLibraries
    ${If} $CopyExistingUserDataLibraries == 1
      Call fnMoveLibrariesInfo
    ${Else}
      ${NSD_SetText} $UserDataLbl ""
    ${EndIf}
  ${EndIf}
FunctionEnd

Function fnCopyExistingUserDataLibraries
  Pop $CopyExistingUserDataLibrariesChkBox
  ${NSD_GetState} $CopyExistingUserDataLibrariesChkBox $CopyExistingUserDataLibraries
  ${If} $CopyExistingUserDataLibraries == 1
    Call fnMoveLibrariesInfo
  ${Else}
    ${NSD_GetState} $DeleteOldUserDataDirectoryChkBox $DeleteOldUserDataDirectory
    ${If} $DeleteOldUserDataDirectory == 1
      Call fnDeleteDirectoryWarning
    ${Else}
      ${NSD_SetText} $UserDataLbl ""
    ${EndIf}
  ${EndIf}
FunctionEnd

Function fnOverwriteUserDataParamFiles
  Pop $OverwriteUserDataParamFilesChkBox
  ${NSD_GetState} $OverwriteUserDataParamFilesChkBox $OverwriteUserDataParamFiles
  ${If} $OverwriteUserDataParamFiles == 1
    Call fnWarning
  ${Else}
    ${NSD_SetText} $UserDataLbl ""
  ${EndIf}
FunctionEnd

Function fnShowUserDataParamFilesManagement
  ${If} $ParameterFilesExist == 1
    ShowWindow $UserDataInstallChkBox ${SW_HIDE}
    ShowWindow $OverwriteUserDataParamFilesChkBox ${SW_SHOW}
  ${Else}
    ShowWindow $UserDataInstallChkBox ${SW_SHOW}
    ShowWindow $OverwriteUserDataParamFilesChkBox ${SW_HIDE}
  ${EndIf}
FunctionEnd

Function fnShowUserDataLibraryManagement
  ${If} $LibrariesExist == 1
    ShowWindow $CopyExistingUserDataLibrariesChkBox ${SW_SHOW}
    ${NSD_Check} $CopyExistingUserDataLibrariesChkBox
    ${If} $OldLibraryDirectoryExist == 1
      Call fnMoveLibrariesInfo
    ${EndIf}
  ${Else}
    ShowWindow $CopyExistingUserDataLibrariesChkBox ${SW_HIDE}
    ${NSD_Uncheck} $CopyExistingUserDataLibrariesChkBox
  ${EndIf}
FunctionEnd

Function fnShowUserDataLibraryDelete
  ${If} $OldLibraryDirectoryExist == 1
    ShowWindow $DeleteOldUserDataDirectoryChkBox ${SW_SHOW}
    ${NSD_Check} $DeleteOldUserDataDirectoryChkBox
  ${Else}
    ShowWindow $DeleteOldUserDataDirectoryChkBox ${SW_HIDE}
    ${NSD_Uncheck} $DeleteOldUserDataDirectoryChkBox
  ${EndIf}
FunctionEnd

Function fnWarning
    ${NSD_SetText} $UserDataLbl "WARNING! You will overwrite your custom settings."
FunctionEnd

Function fnUserDataInfo
    ${NSD_SetText} $UserDataLbl "NOTICE! Data created 'per user'. Only the installing user will have access."
FunctionEnd

Function fnMoveLibrariesInfo
    ${NSD_SetText} $UserDataLbl "INFO: LDraw library archives will be moved to a new directory:$\r$\n'${INSTDIR_LocalAppData}\${COMPANY_NAME}\${PRODUCT_NAME}\libraries'."
FunctionEnd

Function fnDeleteDirectoryWarning
    ${NSD_SetText} $UserDataLbl "WARNING! Current libraries will be deleted. Check Use existing libraries to preserve."
FunctionEnd

Function fnInitializeUserDataVars
  StrCpy $InstallUserData 0

  StrCpy $DeleteOldUserDataDirectory 0
  StrCpy $CopyExistingUserDataLibraries 0
  StrCpy $OverwriteUserDataParamFiles 0

  StrCpy $LibrariesExist 0
  StrCpy $ParameterFilesExist 0
FunctionEnd

Function fnVerifyDeleteDirectory
  StrCpy $OldLibraryDirectoryExist 0
  ${StrContains} $0 "${INSTDIR_LocalAppData}\${COMPANY_NAME}\${PRODUCT_NAME}\libraries" $LPub3DViewerLibPath
    StrCmp $0 "" doNotMatch
    StrCpy $OldLibraryDirectoryExist 1
    Goto Finish
  doNotMatch:
    StrCpy $OldLibraryDirectoryExist 0
  Finish:
    ;MessageBox MB_ICONEXCLAMATION "fnVerifyDeleteDirectory LibrariesExist = $LibrariesExist$\r$\nCompare this: ($LOCALAPPDATA\${Company}\${ProductName}\libraries)$\r$\nto ($LPub3DViewerLibPath)" IDOK 0
FunctionEnd

Function fnInstallLibraries
  SetOutPath "${INSTDIR_AppDataProduct}\libraries"
  File "${WinBuildDir}\extras\complete.zip"
  File "${WinBuildDir}\extras\lpub3dldrawunf.zip"
  File "${WinBuildDir}\extras\tenteparts.zip"
  File "${WinBuildDir}\extras\vexiqparts.zip"
FunctionEnd

Function fnCopyLibraries
    SetOutPath "${INSTDIR_AppDataProduct}\libraries"
    IfFileExists "${INSTDIR_AppDataProduct}\libraries\complete.zip" 0 +2
    goto Next
    IfFileExists "$LPub3DViewerLibPath\complete.zip" 0 Install_new_off_Lib
    ${If} $OldLibraryDirectoryExist == 1
        CopyFiles "$LPub3DViewerLibPath\complete.zip" "${INSTDIR_AppDataProduct}\libraries\complete.zip"
    ${EndIf}
    goto Next
    Install_new_off_Lib:
  File "${WinBuildDir}\extras\complete.zip"
    Next:
    IfFileExists "${INSTDIR_AppDataProduct}\libraries\lpub3dldrawunf.zip" 0 +2
    goto CopyTente_lib
    IfFileExists "$LPub3DViewerLibPath\ldrawunf.zip" 0 Install_new_unoff_Lib
    ${If} $OldLibraryDirectoryExist == 1
        CopyFiles "$LPub3DViewerLibPath\ldrawunf.zip" "${INSTDIR_AppDataProduct}\libraries\lpub3dldrawunf.zip"
    ${EndIf}
    goto CopyTente_lib
    Install_new_unoff_Lib:
  File "${WinBuildDir}\extras\lpub3dldrawunf.zip"
    CopyTente_lib:
    IfFileExists "${INSTDIR_AppDataProduct}\libraries\tenteparts.zip" 0 +2
    goto CopyVEXIQ_lib
    IfFileExists "$LPub3DViewerLibPath\tenteparts.zip" 0 Install_new_tente_Lib
    goto CopyVEXIQ_lib
    Install_new_tente_Lib:
  File "${WinBuildDir}\extras\tenteparts.zip"
    CopyVEXIQ_lib:
    IfFileExists "${INSTDIR_AppDataProduct}\libraries\vexiqparts.zip" 0 +2
    goto Finish
    IfFileExists "$LPub3DViewerLibPath\vexiqparts.zip" 0 Install_new_vexiq_Lib
    goto Finish
    Install_new_vexiq_Lib:
  File "${WinBuildDir}\extras\vexiqparts.zip"
    Finish:
FunctionEnd

Function fnAskForceUninstall
  DetailPrint "Error executing uninstaller."
  MessageBox MB_ICONEXCLAMATION|MB_YESNO "Error executing uninstaller!.$\r$\n\
                                          Do you want to completely remove ${PRODUCT_NAME}?$\r$\n\
                                          Warning: Custom user data and registry settings will be deleted." IDYES Proceed
  Abort "Error executing uninstaller."

  Proceed:
  DetailPrint "Removing all previously installed components of ${PRODUCT_NAME}..."
  
  ; Clean up "Quick Launch Icon"
  ${if} ${AtLeastWin7}
    ${StdUtils.InvokeShellVerb} $1 "$INSTDIR" "${PROGEXE}" ${StdUtils.Const.ShellVerb.UnpinFromStart}
  ${else}
    !insertmacro DeleteRetryAbort "$STARTMENU\${PRODUCT_NAME}$0.lnk"
  ${endif}
  
  ; Clean up "Program Group" - we check that we created Start menu folder, if $StartMenuFolder is empty, the whole $SMPROGRAMS directory will be removed!
  ${if} "$StartMenuFolder" != ""
      RMDir /r "$SMPROGRAMS\$StartMenuFolder"
  ${endif}
    
  ; Clean up "Desktop Icon"
  !insertmacro DeleteRetryAbort "$DESKTOP\${PRODUCT_NAME}$0.lnk"
  
  ; Delete user data folder and all its contents
  ${if} ${DirExists} "${INSTDIR_AppDataProduct}\${COMPANY_NAME}"
      RMDir /r "${INSTDIR_LocalAppData}\${COMPANY_NAME}"
  ${EndIf}
  
  ; Delete registry hive settings
  DeleteRegKey HKCU "Software\${COMPANY_NAME}"
    
  ; Delete install folder and all its contents - this function is executed only when the normal uninstall fails and the user elects to force remove the application
  ${if} ${DirExists} "$INSTDIR"
      RMDir /r "$INSTDIR"
  ${EndIf}
  
  ; Remove the uninstaller keys from registry
  !insertmacro MULTIUSER_RegistryRemoveInstallInfo
FunctionEnd
