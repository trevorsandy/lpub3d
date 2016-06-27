;LPub3D Setup Script
;Copyright (C) 2016 by Trevor Sandy

;--------------------------------
;Include Modern UI

  !include "MUI2.nsh"
  !include "x64.nsh"

;--------------------------------
;Utility functions and macros
  !include "nsisFunctions.nsh"
;--------------------------------
;generated define statements
  
  ; Include app version details.
  !include "AppVersion.nsh"

;--------------------------------
;Variables

  Var /global nsDialogFilePathsPage
  Var /global StartMenuFolder
  Var /global FileName
  Var /global LDrawDirPath
  Var /global PathsGrpBox
  Var /global BrowseLDraw
  Var /global LDrawText
  
  Var /global nsDialogOverwriteConfigPage
  Var /global OverwriteMessagelbl
  Var /global OverwriteConfigGrpBox
  Var /global Overwrite_chkBoxAll
  Var /global Overwrite_chkBoxTitle
  Var /global Overwrite_chkBoxFreeform
  Var /global Overwrite_chkBoxSubstitute
  Var /global Overwrite_chkBoxFadeParts

  Var /global OverwriteAll
  Var /global OverwriteTitleAnnotaitonsFile
  Var /global OverwriteFreeformAnnotationsFile
  Var /global OverwriteSubstitutePartsFile
  Var /global OverwriteFadeStepColourPartsFile
  
  ;new stuff
  Var /global LPub3DViewerLibFile
  Var /global LPub3DViewerLibPath
  Var /global HideDeleteDirectoryDialog
   
  Var /global OverwriteDeleteDirectoryGrpBox
  Var /global Overwrite_chkMoveLibraries
  Var /global Overwrite_chkDeleteDirectory
  
  Var /global OverwriteMoveLibraries
  Var /global OverwriteDeleteDirectory
  
  Var /global nsDialogOverwriteConfigPage_Font1
  Var /global nsDialogOverwriteConfigPage_Font2
  
  Var /global IsVC2015DistInstalled
  
;--------------------------------
;General
 
  ;Installer name
  Name "${ProductName}, Ver ${Version}, Rev ${BuildRevision}"

  ; Changes the caption, default beeing 'Setup'
  Caption "${ProductName} 32,64-bit Setup"
  
  ; Rebrand bottom textrow
  BrandingText "${Company} Installer"

  ; Show install details (show|hide|nevershow)
  ShowInstDetails hide
  
  SetCompressor /SOLID lzma
   
  ;The files to write
  !ifdef UpdateMaster
  OutFile "..\release\${ProductName}-UpdateMaster_${BuildVersion}.exe"
  !else
  OutFile "..\release\${ProductName}-${CompleteVersion}.exe"
  !endif
  
  ;Default installation folder
  InstallDir "$INSTDIR"
  
  ;Check if installation directory registry key exist
  ;Get installation folder from registry if available
  InstallDirRegKey HKCU "Software\${Company}\${ProductName}\Installation" "InstallPath"
  
  Icon "setup.ico"

  !define MUI_ICON "setup.ico"
  !define MUI_UNICON "setup.ico"
  
  ; Execution level 
  RequestExecutionLevel admin
    
;--------------------------------
;Interface Settings

  !define MUI_ABORTWARNING
  
;--------------------------------
;Pages

  !define MUI_WELCOMEFINISHPAGE_BITMAP "welcome.bmp"
  !insertmacro MUI_PAGE_WELCOME
  !insertmacro MUI_PAGE_LICENSE "..\docs\Copying.txt"
  !insertmacro MUI_PAGE_DIRECTORY
  
  ;Custom page, Initialize library settings for smoother install.
  Page custom nsDialogShowCustomPage nsDialogLeaveCustomPage
  
  ;Custom page, Prompt user to overwrite configuration files
  Page custom nsDialogShowOverwriteConfigPage nsDialogLeaveOverwriteConfigPage
  
  ;Start Menu Folder Page Configuration
  !define MUI_STARTMENUPAGE_DEFAULTFOLDER "${ProductName}"
  !define MUI_STARTMENUPAGE_REGISTRY_ROOT "HKCU" 
  !define MUI_STARTMENUPAGE_REGISTRY_KEY "Software\${Company}\${ProductName}\Installation" 
  !define MUI_STARTMENUPAGE_REGISTRY_VALUENAME "StartMenuFolder"
  !insertmacro MUI_PAGE_STARTMENU Application $StartMenuFolder
  
  !insertmacro MUI_PAGE_INSTFILES
  
  ;These indented statements modify settings for MUI_PAGE_FINISH
  !define MUI_FINISHPAGE_NOAUTOCLOSE	
  !define MUI_FINISHPAGE_RUN "$FileName"
  !define MUI_FINISHPAGE_RUN_TEXT "Launch ${ProductName}"
  !define MUI_FINISHPAGE_RUN_NOTCHECKED
  !define MUI_FINISHPAGE_SHOWREADME "${ProductName}"
  !define MUI_FINISHPAGE_SHOWREADME_NOTCHECKED
  !define MUI_FINISHPAGE_SHOWREADME_TEXT "Install Desktop Icon"
  !define MUI_FINISHPAGE_SHOWREADME_FUNCTION desktopIcon
  !define MUI_FINISHPAGE_LINK "${CompanyURL}"
  !define MUI_FINISHPAGE_LINK_LOCATION "${CompanyURL}"
  !insertmacro MUI_PAGE_FINISH
  
  ;Uninstall pages
  !define MUI_UNWELCOMEFINISHPAGE_BITMAP "welcome.bmp"
  !define MUI_FINISHPAGE_LINK "${CompanyURL}"
  !define MUI_FINISHPAGE_LINK_LOCATION "${CompanyURL}"
  !insertmacro MUI_UNPAGE_WELCOME
  !insertmacro MUI_UNPAGE_CONFIRM
  !insertmacro MUI_UNPAGE_INSTFILES
  !insertmacro MUI_UNPAGE_FINISH
  
;--------------------------------
;Languages
 
  !insertmacro MUI_LANGUAGE "English"

;--------------------------------
;Descriptions

  ;Language strings
  LangString CUST_PAGE_TITLE ${LANG_ENGLISH} "LDraw Library"
  LangString CUST_PAGE_SUBTITLE ${LANG_ENGLISH} "Enter path for your LDraw directory."
  
  LangString CUST_PAGE_OVERWRITE_TITLE ${LANG_ENGLISH} "Overwrite Configuration Files"
  LangString CUST_PAGE_OVERWRITE_SUBTITLE ${LANG_ENGLISH} "Check the box next to the configuration file you would like to overwrite."
 
;--------------------------------
;Initialize install directory 
 
Function .onInit 

  ;Hack to prevent startup crash - has something to do with Qt 5.6 - investigatig...
   DeleteRegValue HKCU "Software\${Company}\${ProductName}\MainWindow" "pos"
  
  ;Get Ldraw library folder and archive file paths from registry if available
   ReadRegStr $LDrawDirPath HKCU "Software\${Company}\${ProductName}\Settings" "LDrawDir"
   ReadRegStr $LPub3DViewerLibFile HKCU "Software\${Company}\${ProductName}\Settings" "PartsLibrary" 
  ;Verify old library directory exist - and is not the same as new library directory
   Push $LPub3DViewerLibFile
   Call fnGetParent
   Pop $R0
   StrCpy $LPub3DViewerLibPath $R0
   
  ${If} ${DirExists} $LPub3DViewerLibPath
	Call fnVerifyDeleteDirectory
  ${Else}
	StrCpy $HideDeleteDirectoryDialog 1
  ${EndIf}

  ;Identify installation folder
  ${If} ${RunningX64}
	StrCpy $FileName "${ProductName}_x64.exe"
	StrCpy $INSTDIR "$PROGRAMFILES64\${ProductName}"
  ${Else}
	StrCpy $FileName "${ProductName}_x32.exe"
	StrCpy $INSTDIR "$PROGRAMFILES32\${ProductName}"
  ${EndIf}
    
FunctionEnd
 
;--------------------------------
;Installer Sections

Section "${ProductName} (required)" SecMain${ProductName}

  ;Confirm VC++ 2015 Redistribution is installed
  File "..\release\vcredist\vc_redist.x86.exe"
  File "..\release\vcredist\vc_redist.x64.exe"
  Call fnConfirmVC2015Redist
  
  ;install directory
  SetOutPath "$INSTDIR"
  
  ;executable requireds and readme
  ${If} ${RunningX64}
	File "${Win64BuildDir}\${ProductName}_x64.exe"
	File "${Win64BuildDir}\quazip.dll"
	File "${Win64BuildDir}\ldrawini.dll"
	File "${Win64BuildDir}\Qt5Core.dll"
	File "${Win64BuildDir}\Qt5Network.dll"
	File "${Win64BuildDir}\Qt5Gui.dll"
	File "${Win64BuildDir}\Qt5Widgets.dll"
	File "${Win64BuildDir}\Qt5PrintSupport.dll"
	File "${Win64BuildDir}\Qt5OpenGL.dll"
  ;New Stuff - Qt Libraries	
	File "${Win64BuildDir}\libGLESV2.dll"
	File "${Win64BuildDir}\libEGL.dll"
	File "${Win64BuildDir}\opengl32sw.dll"
	File "${Win64BuildDir}\d3dcompiler_47.dll"
  ;New Stuff - Qt Plugins
    CreateDirectory "$INSTDIR\bearer"
    SetOutPath "$INSTDIR\bearer"
	File "${Win64BuildDir}\bearer\qgenericbearer.dll"
	File "${Win64BuildDir}\bearer\qnativewifibearer.dll"
    CreateDirectory "$INSTDIR\iconengines"
    SetOutPath "$INSTDIR\iconengines"
	File "${Win64BuildDir}\iconengines\qsvgicon.dll"
    CreateDirectory "$INSTDIR\imageformats"
    SetOutPath "$INSTDIR\imageformats"
	File "${Win64BuildDir}\imageformats\qdds.dll"
	File "${Win64BuildDir}\imageformats\qgif.dll"
	File "${Win64BuildDir}\imageformats\qicns.dll"
	File "${Win64BuildDir}\imageformats\qico.dll"
	File "${Win64BuildDir}\imageformats\qjpeg.dll"
	File "${Win64BuildDir}\imageformats\qsvg.dll"
	File "${Win64BuildDir}\imageformats\qtga.dll"
	File "${Win64BuildDir}\imageformats\qtiff.dll"
	File "${Win64BuildDir}\imageformats\qwbmp.dll"
	File "${Win64BuildDir}\imageformats\qwebp.dll"
    CreateDirectory "$INSTDIR\platforms"
    SetOutPath "$INSTDIR\platforms"
	File "${Win64BuildDir}\platforms\qwindows.dll"
    CreateDirectory "$INSTDIR\printsupport"
    SetOutPath "$INSTDIR\printsupport"
	File "${Win64BuildDir}\printsupport\windowsprintersupport.dll"
  ${Else}
	File "${Win32BuildDir}\${ProductName}_x32.exe"
	File "${Win32BuildDir}\quazip.dll"
	File "${Win32BuildDir}\ldrawini.dll"
	File "${Win32BuildDir}\Qt5Core.dll"
	File "${Win32BuildDir}\Qt5Network.dll"
	File "${Win32BuildDir}\Qt5Gui.dll"
	File "${Win32BuildDir}\Qt5Widgets.dll"
	File "${Win32BuildDir}\Qt5PrintSupport.dll"
	File "${Win32BuildDir}\Qt5OpenGL.dll"
  ;New Stuff - Qt Libraries
	File "${Win32BuildDir}\libGLESV2.dll"  
	File "${Win32BuildDir}\libEGL.dll"
	File "${Win32BuildDir}\opengl32sw.dll"	
	File "${Win32BuildDir}\d3dcompiler_47.dll"
  ;New Stuff - Qt Plugins
    CreateDirectory "$INSTDIR\bearer"
    SetOutPath "$INSTDIR\bearer"
	File "${Win32BuildDir}\bearer\qgenericbearer.dll"
	File "${Win32BuildDir}\bearer\qnativewifibearer.dll"
    CreateDirectory "$INSTDIR\iconengines"
    SetOutPath "$INSTDIR\iconengines"
	File "${Win32BuildDir}\iconengines\qsvgicon.dll"
    CreateDirectory "$INSTDIR\imageformats"
    SetOutPath "$INSTDIR\imageformats"
	File "${Win32BuildDir}\imageformats\qdds.dll"
	File "${Win32BuildDir}\imageformats\qgif.dll"
	File "${Win32BuildDir}\imageformats\qicns.dll"
	File "${Win32BuildDir}\imageformats\qico.dll"
	File "${Win32BuildDir}\imageformats\qjpeg.dll"
	File "${Win32BuildDir}\imageformats\qsvg.dll"
	File "${Win32BuildDir}\imageformats\qtga.dll"
	File "${Win32BuildDir}\imageformats\qtiff.dll"
	File "${Win32BuildDir}\imageformats\qwbmp.dll"
	File "${Win32BuildDir}\imageformats\qwebp.dll"
    CreateDirectory "$INSTDIR\platforms"
    SetOutPath "$INSTDIR\platforms"
	File "${Win32BuildDir}\platforms\qwindows.dll"
    CreateDirectory "$INSTDIR\printsupport"
    SetOutPath "$INSTDIR\printsupport"
	File "${Win32BuildDir}\printsupport\windowsprintersupport.dll"	
  ${EndIf}
  
  SetOutPath "$INSTDIR"
  File "..\docs\README.txt"
  
  ;3rd party renderer utility - LdgLite
  CreateDirectory "$INSTDIR\3rdParty\ldglite1.3.1_2g2x_Win"
  SetOutPath "$INSTDIR\3rdParty\ldglite1.3.1_2g2x_Win"
  File "..\release\3rdParty\ldglite1.3.1_2g2x_Win\ldglite.exe"
  File "..\release\3rdParty\ldglite1.3.1_2g2x_Win\LICENCE"
  File "..\release\3rdParty\ldglite1.3.1_2g2x_Win\README.TXT" 
  ;3rd party renderer utility - L3P
  CreateDirectory "$INSTDIR\3rdParty\l3p1.4WinB"
  SetOutPath "$INSTDIR\3rdParty\l3p1.4WinB" 
  File "..\release\3rdParty\l3p1.4WinB\L3P.EXE"
  
  ;AppData setup
  SetShellVarContext all
  !define INSTDIR_AppData "$LOCALAPPDATA\${Company}\${ProductName}"
  
  ;ldraw libraries
  CreateDirectory "${INSTDIR_AppData}\libraries"
  
  ${If} $OverwriteDeleteDirectory == 1
	${If} $OverwriteMoveLibraries == 1
		Call fnCopyLibraries
	${Else}
		Call fnInstallLibraries
	${EndIf}
	${If} ${DirExists} $LPub3DViewerLibPath
		RMDir /r $LPub3DViewerLibPath
	${EndIf}
  ${Else}
	Call fnInstallLibraries
  ${EndIf}
  
  ;extras contents
  CreateDirectory "${INSTDIR_AppData}\extras"
  SetOutPath "${INSTDIR_AppData}\extras"
  File "..\..\mainApp\extras\PDFPrint.jpg"
  File "..\..\mainApp\extras\pli.mpd"
  
 ${If} $OverwriteTitleAnnotaitonsFile == 0
  SetOverwrite off
  File "..\..\mainApp\extras\titleAnnotations.lst"
 ${Else}
  SetOverwrite on
  File "..\..\mainApp\extras\titleAnnotations.lst"
 ${EndIf}
 ${If} $OverwriteFreeformAnnotationsFile == 0
  SetOverwrite off
  File "..\..\mainApp\extras\freeformAnnotations.lst"
 ${Else}
  SetOverwrite on
  File "..\..\mainApp\extras\freeformAnnotations.lst"
 ${EndIf}
 ${If} $OverwriteFadeStepColourPartsFile == 0
  SetOverwrite off
  File "..\..\mainApp\extras\fadeStepColorParts.lst"
 ${Else} 
  SetOverwrite on
  File "..\..\mainApp\extras\fadeStepColorParts.lst"
 ${EndIf}
 ${If} $OverwriteSubstitutePartsFile == 0
  SetOverwrite off
  File "..\..\mainApp\extras\pliSubstituteParts.lst"
 ${Else}
  SetOverwrite on
  File "..\..\mainApp\extras\pliSubstituteParts.lst"
 ${EndIf}
  
  ;documents  
  CreateDirectory "$INSTDIR\docs"
  SetOutPath "$INSTDIR\docs"
  File "..\docs\COPYING.txt"
  File "..\docs\CREDITS.txt"
  SetOutPath "$INSTDIR"
  
  ;Store installation folder
  WriteRegStr HKCU "Software\${Company}\${ProductName}\Installation" "InstallPath" $INSTDIR
  
  ;Store/Update library folder
  WriteRegStr HKCU "Software\${Company}\${ProductName}\Settings" "PartsLibrary" "${INSTDIR_AppData}\libraries\complete.zip"
  
  ;Create uninstaller
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${ProductName}" "DisplayIcon" '"$INSTDIR\$FileName"'  
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${ProductName}" "DisplayName" "${ProductName}"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${ProductName}" "DisplayVersion" "${Version}"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${ProductName}" "Publisher" "${Publisher}"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${ProductName}" "URLInfoAbout" "${CompanyURL}"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${ProductName}" "URLUpdateInfo" "${CompanyURL}"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${ProductName}" "HelpLink" "${SupportEmail}"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${ProductName}" "Comments" "${Comments}"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${ProductName}" "UninstallString" '"$INSTDIR\Uninstall.exe"'
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${ProductName}" "EstimatedSize" 106000
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${ProductName}" "NoModify" 1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${ProductName}" "NoRepair" 1
  WriteUninstaller "$INSTDIR\Uninstall.exe"

  !insertmacro MUI_STARTMENU_WRITE_BEGIN Application
  
    ;Create shortcuts
	SetShellVarContext all
    CreateDirectory "$SMPROGRAMS\$StartMenuFolder"
	CreateShortCut "$SMPROGRAMS\$StartMenuFolder\${ProductName}.lnk" "$INSTDIR\$FileName"
    CreateShortCut "$SMPROGRAMS\$StartMenuFolder\Uninstall ${ProductName}.lnk" "$INSTDIR\Uninstall.exe"
	
  !insertmacro MUI_STARTMENU_WRITE_END
  
SectionEnd
  
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

    ; === grpBoxPaths (type: GroupBox) ===
    ${NSD_CreateGroupBox} -0.66u 24u 297.52u 88u "Define LDraw Library Path"
	Pop $PathsGrpBox
	
	; === HLine1 (type: HLine) ===
    ${NSD_CreateHLine} 7.9u 89.85u 281.72u 1.23u "HLine"

    ; === lblLDrawText (type: Label) ===
    ${NSD_CreateLabel} 7.9u 46.15u 228.41u 11.69u "Select LDraw Directory"	
	
    ; === LDrawText (type: Text) ===
    ${NSD_CreateText} 7.9u 59.69u 228.41u 12.31u "$LDrawDirPath"	
	Pop $LDrawText

    ; === BrowseLDraw (type: Button) ===
    ${NSD_CreateButton} 240.25u 59.69u 49.37u 14.15u "Browse"
	Pop $BrowseLDraw

	${NSD_OnClick} $BrowseLDraw fnBrowseLDraw
	
 nsDialogs::Show

FunctionEnd

Function fnBrowseLDraw

  nsDialogs::SelectFolderDialog "Select LDraw Directory" $LDrawDirPath
  Pop $LDrawDirPath
  ${NSD_SetText} $LDrawText $LDrawDirPath

FunctionEnd

Function nsDialogLeaveCustomPage
   
  ;Validate the LDraw Directory path
  ${If} ${DirExists} $LDrawDirPath
    ; Update the registry wiht the LDraw Directory path.
	WriteRegStr HKCU "Software\${Company}\${ProductName}\Settings" "LDrawDir" $LDrawDirPath
	Goto Continue
  ${Else}
    MessageBox MB_ICONEXCLAMATION|MB_YESNO "You did not enter a valid LDraw Directory. Do you want to continue?" IDNO Terminate
	Goto Continue
  ${EndIf}
  Terminate:
  Abort
  Continue:
FunctionEnd

Function nsDialogShowOverwriteConfigPage

  ;--------------------------------
  ;Prompt user to overwrite configuration files

  ; === nsDialogOverwriteConfigPage ===
  nsDialogs::Create 1018
  Pop $nsDialogOverwriteConfigPage
  
  ${If} $nsDialogOverwriteConfigPage == error
    Abort
  ${EndIf} 
  
  ; === check if directory already exist ===
  ${If} ${DirExists} "${INSTDIR_AppData}\extras"
	Goto DoShowOverwritePage
  ${Else}
    Abort
  ${EndIf}
  
  DoShowOverwritePage:
  
  !insertmacro MUI_HEADER_TEXT $(CUST_PAGE_OVERWRITE_TITLE) $(CUST_PAGE_OVERWRITE_SUBTITLE)
  
  ; custom font definitions
  CreateFont $nsDialogOverwriteConfigPage_Font1 "Microsoft Sans Serif" "7.25" "400"
  CreateFont $nsDialogOverwriteConfigPage_Font2 "Microsoft Sans Serif" "8.25" "700"
  
  ; === OverwriteMessagelbl (type: Label) ===
  ${NSD_CreateLabel} 32.91u 0.01u 226.43u 14.15u ""
  Pop $OverwriteMessagelbl
  SendMessage $OverwriteMessagelbl ${WM_SETFONT} $nsDialogOverwriteConfigPage_Font1 0
  SetCtlColors $OverwriteMessagelbl 0xFF0000 0xF0F0F0 
  
  ; === OverwriteConfigGrpBox (type: GroupBox) ===
  ${NSD_CreateGroupBox} 32.91u 18.46u 226.43u 87.38u "Overwrite Configuration Files"
  Pop $OverwriteConfigGrpBox
    
  ; === chkBoxAll (type: Checkbox) ===
  ${NSD_CreateCheckbox} 42.78u 30.15u 116.51u 14.77u "All Configuration Files"
  Pop $Overwrite_chkBoxAll
  SendMessage $Overwrite_chkBoxAll ${WM_SETFONT} $nsDialogOverwriteConfigPage_Font2 0
  
  ; === chkBoxTitle (type: Checkbox) ===
  ${NSD_CreateCheckbox} 42.78u 43.69u 116.51u 14.77u "Title Annotations"
  Pop $Overwrite_chkBoxTitle
  
  ; === chkBoxFreeform (type: Checkbox) ===
  ${NSD_CreateCheckbox} 42.78u 57.23u 116.51u 14.77u "Freeform Annotations"
  Pop $Overwrite_chkBoxFreeform
  
  ; === chkBoxSubstitute (type: Checkbox) ===
  ${NSD_CreateCheckbox} 42.78u 70.77u 116.51u 17.23u "Substitute Parts"
  Pop $Overwrite_chkBoxSubstitute
  
  ; === chkBoxFadeParts (type: Checkbox) ===
  ${NSD_CreateCheckbox} 42.78u 86.77u 116.51u 14.77u "Fade Step Colour Parts"
  Pop $Overwrite_chkBoxFadeParts
  
  ; === OverwriteDeleteDirectoryGrpBox (type: GroupBox) ===
  ${NSD_CreateGroupBox} 32.91u 107.69u 226.43u 28.92u "Remove Previous LDraw Library Archive Directory"
  Pop $OverwriteDeleteDirectoryGrpBox
  
  ; === chkMoveLibraries (type: Checkbox) ===
  ${NSD_CreateCheckbox} 125.72u 119.38u 123.09u 14.77u "Move libraries to new location?"
  Pop $Overwrite_chkMoveLibraries
  
  ; === chkDeleteDirectory (type: Checkbox) ===
  ${NSD_CreateCheckbox} 36.2u 119.38u 116.51u 14.77u "Delete old directory?"
  Pop $Overwrite_chkDeleteDirectory
  
  ${NSD_OnClick} $Overwrite_chkBoxAll fnSetOverwriteAll
  ${NSD_OnClick} $Overwrite_chkBoxTitle fnOverwriteTitle
  ${NSD_OnClick} $Overwrite_chkBoxFreeform fnOverwriteFreeform
  ${NSD_OnClick} $Overwrite_chkBoxSubstitute fnOverwriteSubstitute
  ${NSD_OnClick} $Overwrite_chkBoxFadeParts fnOverwriteFadeParts
  
  ${NSD_OnClick} $Overwrite_chkMoveLibraries fnMoveLibraries
  ${NSD_OnClick} $Overwrite_chkDeleteDirectory fnDeleteDirectory
  
  Call fnShowDeleteDirectoryDialog
  
  nsDialogs::Show
  
FunctionEnd

Function fnShowDeleteDirectoryDialog
  ${If} $HideDeleteDirectoryDialog == 1	
	ShowWindow $Overwrite_chkMoveLibraries ${SW_HIDE}
	ShowWindow $Overwrite_chkDeleteDirectory ${SW_HIDE}
	ShowWindow $OverwriteDeleteDirectoryGrpBox ${SW_HIDE}
	${NSD_Uncheck} $Overwrite_chkMoveLibraries
	${NSD_Uncheck} $Overwrite_chkDeleteDirectory
	StrCpy $OverwriteDeleteDirectory 0
  ${Else}
    ShowWindow $Overwrite_chkMoveLibraries ${SW_SHOW}
	ShowWindow $Overwrite_chkDeleteDirectory ${SW_SHOW}
	ShowWindow $OverwriteDeleteDirectoryGrpBox ${SW_SHOW}
	${NSD_Check} $Overwrite_chkMoveLibraries
	${NSD_Check} $Overwrite_chkDeleteDirectory
	Call fnMoveLibrariesInfo
  ${EndIf}
  ;MessageBox MB_ICONEXCLAMATION "fnShowDeleteDirectoryDialog HideDeleteDirectoryDialog = $HideDeleteDirectoryDialog" IDOK 0
  
FunctionEnd

Function fnSetOverwriteAll
 Pop $Overwrite_chkBoxAll 
 
 ${NSD_GetState} $Overwrite_chkBoxAll $OverwriteAll
 ${If} $OverwriteAll == 1 
	${NSD_Check} $Overwrite_chkBoxTitle
	${NSD_Check} $Overwrite_chkBoxFreeform
	${NSD_Check} $Overwrite_chkBoxSubstitute
	${NSD_Check} $Overwrite_chkBoxFadeParts
	Call fnWarning
 ${Else}
	${NSD_Uncheck} $Overwrite_chkBoxTitle
	${NSD_Uncheck} $Overwrite_chkBoxFreeform
	${NSD_Uncheck} $Overwrite_chkBoxSubstitute
	${NSD_Uncheck} $Overwrite_chkBoxFadeParts
	Call fnClear 
 ${EndIf}

FunctionEnd

Function fnOverwriteTitle
	Pop $Overwrite_chkBoxTitle
	${NSD_GetState} $Overwrite_chkBoxTitle $OverwriteTitleAnnotaitonsFile
    ${If} $OverwriteTitleAnnotaitonsFile == 1
		Call fnWarning
	${Else}
		Call fnClear 
	${EndIf}
	
FunctionEnd

Function fnOverwriteFreeform
	Pop $Overwrite_chkBoxFreeform
    ${NSD_GetState} $Overwrite_chkBoxFreeform $OverwriteFreeformAnnotationsFile
    ${If} $OverwriteFreeformAnnotationsFile == 1
		Call fnWarning
	${Else}
		Call fnClear 
	${EndIf}
	
FunctionEnd

Function fnOverwriteSubstitute	
	Pop $Overwrite_chkBoxSubstitute
	${NSD_GetState} $Overwrite_chkBoxSubstitute $OverwriteSubstitutePartsFile
    ${If} $OverwriteSubstitutePartsFile == 1
		Call fnWarning
	${Else}
		Call fnClear 
	${EndIf}

FunctionEnd

Function fnOverwriteFadeParts	
	Pop $Overwrite_chkBoxFadeParts
	${NSD_GetState} $Overwrite_chkBoxFadeParts $OverwriteFadeStepColourPartsFile	
    ${If} $OverwriteFadeStepColourPartsFile == 1
		Call fnWarning
	${Else}
		Call fnClear 
	${EndIf}

FunctionEnd

Function fnDeleteDirectory
	Pop $Overwrite_chkDeleteDirectory
	${NSD_GetState} $Overwrite_chkDeleteDirectory $OverwriteDeleteDirectory
    ${If} $OverwriteDeleteDirectory == 1
		${NSD_GetState} $Overwrite_chkMoveLibraries $OverwriteMoveLibraries
		${If} $OverwriteMoveLibraries <> 1
			Call fnDeleteDirectoryWarning
		${EndIf}
	${Else}
		${NSD_GetState} $Overwrite_chkMoveLibraries $OverwriteMoveLibraries
		${If} $OverwriteMoveLibraries == 1
			Call fnMoveLibrariesInfo
		${Else}
			Call fnClear 
		${EndIf}		
	${EndIf}
	
FunctionEnd

Function fnMoveLibraries	
	Pop $Overwrite_chkMoveLibraries
	${NSD_GetState} $Overwrite_chkMoveLibraries $OverwriteMoveLibraries
    ${If} $OverwriteMoveLibraries == 1
		Call fnMoveLibrariesInfo
	${Else}
		${NSD_GetState} $Overwrite_chkDeleteDirectory $OverwriteDeleteDirectory
		${If} $OverwriteDeleteDirectory == 1
			Call fnDeleteDirectoryWarning
		${Else}
			Call fnClear 
		${EndIf}	
	${EndIf}

FunctionEnd

Function fnWarning
    ${NSD_SetText} $OverwriteMessagelbl "WARNING! You will overwrite your custom settings."
	
FunctionEnd

Function fnMoveLibrariesInfo
    ${NSD_SetText} $OverwriteMessagelbl "INFO: LDraw library archives will be moved to a new directory:$\r$\n'$LOCALAPPDATA\${Company}\${ProductName}\libraries'."
	
FunctionEnd

Function fnDeleteDirectoryWarning
    ${NSD_SetText} $OverwriteMessagelbl "WARNING! Current libraries will be deleted. Check Move to preserve."
	
FunctionEnd

Function fnClear
    ${If} $OverwriteTitleAnnotaitonsFile <> 1
	${AndIf} $OverwriteFreeformAnnotationsFile <> 1
    ${AndIf} $OverwriteFadeStepColourPartsFile <> 1
	${AndIf} $OverwriteSubstitutePartsFile <> 1
	${AndIf} $OverwriteMoveLibraries <> 1
	${AndIf} $OverwriteDeleteDirectory <> 1
		${NSD_SetText} $OverwriteMessagelbl ""
	${EndIf}
	
FunctionEnd

Function nsDialogLeaveOverwriteConfigPage
	${NSD_GetState} $Overwrite_chkBoxTitle $OverwriteTitleAnnotaitonsFile
	${NSD_GetState} $Overwrite_chkBoxFreeform $OverwriteFreeformAnnotationsFile
	${NSD_GetState} $Overwrite_chkBoxSubstitute $OverwriteSubstitutePartsFile
	${NSD_GetState} $Overwrite_chkBoxFadeParts $OverwriteFadeStepColourPartsFile

	${NSD_GetState} $Overwrite_chkMoveLibraries $OverwriteMoveLibraries
	${NSD_GetState} $Overwrite_chkDeleteDirectory $OverwriteDeleteDirectory
	
FunctionEnd

Function fnInstallLibraries
	SetOutPath "${INSTDIR_AppData}\libraries"
	File "..\release\libraries\complete.zip"
	File "..\release\libraries\lpub3dldrawunf.zip"
	
FunctionEnd

Function fnCopyLibraries
	SetOutPath "${INSTDIR_AppData}\libraries"
	IfFileExists "$LPub3DViewerLibPath\complete.zip" 0 install_new_off_Lib
	CopyFiles "$LPub3DViewerLibPath\complete.zip" "${INSTDIR_AppData}\libraries\complete.zip"
	goto Next
	install_new_off_Lib:
	File "..\release\libraries\complete.zip"
	Next:
	IfFileExists "$LPub3DViewerLibPath\ldrawunf.zip" 0 install_new_unoff_Lib
	CopyFiles "$LPub3DViewerLibPath\ldrawunf.zip" "${INSTDIR_AppData}\libraries\lpub3dldrawunf.zip"
	goto Finish
	install_new_unoff_Lib:
	File "..\release\libraries\lpub3dldrawunf.zip"	
	Finish:
	
FunctionEnd

Function fnVerifyDeleteDirectory
  StrCpy $HideDeleteDirectoryDialog 0
  ${StrContains} $0 "$LOCALAPPDATA\${Company}\${ProductName}\libraries" $LPub3DViewerLibPath
    StrCmp $0 "" doNotMatch
    StrCpy $HideDeleteDirectoryDialog 1
    Goto Finish
  doNotMatch:
    StrCpy $HideDeleteDirectoryDialog 0
  Finish:
    ;MessageBox MB_ICONEXCLAMATION "fnVerifyDeleteDirectory HideDeleteDirectoryDialog = $HideDeleteDirectoryDialog$\r$\nCompare this: ($LOCALAPPDATA\${Company}\${ProductName}\libraries)$\r$\nto ($LPub3DViewerLibPath)" IDOK 0 
FunctionEnd

Function fnConfirmVC2015Redist
	; Test if Visual Studio 2015 C++ Redistributables are installed
	IfFileExists "$Windir\System32\vcruntime140.dll" +2 0
	StrCpy $IsVC2015DistInstalled 0
	${If} $IsVC2015DistInstalled == 0
		;MessageBox MB_ICONEXCLAMATION "Note: Visual Studio 2015 VC++ Redistributable not found and will be installed!" IDOK 0
		SetOutPath "$TEMP"
		${If} ${RunningX64}
			File "..\release\vcredist\vc_redist.x64.exe"
			goto InstallVC2015x64Redist
		${Else}
			File "..\release\vcredist\vc_redist.x86.exe"
			goto InstallVC2015x86Redist
		${EndIf}		
	${Else}
		;MessageBox MB_ICONEXCLAMATION "Note: Visual Studio 2015 VC++ Redistributable found!" IDOK 0
		goto Finish
	${EndIf}
	InstallVC2015x86Redist:
	ExecWait '"$TEMP/vc_redist.x86.exe" /q' ; '/q' to install silently
	goto Finish
	InstallVC2015x64Redist:
	ExecWait '"$TEMP/vc_redist.x64.exe" /q' 
    Finish:
	
FunctionEnd

Function desktopIcon

    SetShellVarContext current
    CreateShortCut "$DESKTOP\${ProductName}.lnk" "$INSTDIR\$FileName"
	
FunctionEnd

;--------------------------------
;Uninstaller Section

Section "Uninstall"

; Remove files
  ${If} ${RunningX64}
	Delete "$INSTDIR\${ProductName}_x64.exe"
  ${Else}
	Delete "$INSTDIR\${ProductName}_x32.exe"
  ${EndIf}
  
  Delete "$INSTDIR\quazip.dll"
  Delete "$INSTDIR\ldrawini.dll"
  Delete "$INSTDIR\Qt5Core.dll"
  Delete "$INSTDIR\Qt5Network.dll"
  Delete "$INSTDIR\Qt5Gui.dll"
  Delete "$INSTDIR\Qt5Widgets.dll"
  Delete "$INSTDIR\Qt5PrintSupport.dll"
  Delete "$INSTDIR\Qt5OpenGL.dll"
  Delete "$INSTDIR\libGLESV2.dll"  
  Delete "$INSTDIR\libEGL.dll"
  Delete "$INSTDIR\opengl32sw.dll"	
  Delete "$INSTDIR\d3dcompiler_47.dll"
  Delete "$INSTDIR\Uninstall.exe"
  Delete "$INSTDIR\README.txt"
  Delete "$INSTDIR\bearer\qgenericbearer.dll"
  Delete "$INSTDIR\bearer\qnativewifibearer.dll"
  Delete "$INSTDIR\iconengines\qsvgicon.dll"
  Delete "$INSTDIR\imageformats\qdds.dll"
  Delete "$INSTDIR\imageformats\qgif.dll"
  Delete "$INSTDIR\imageformats\qicns.dll"
  Delete "$INSTDIR\imageformats\qico.dll"
  Delete "$INSTDIR\imageformats\qjpeg.dll"
  Delete "$INSTDIR\imageformats\qsvg.dll"
  Delete "$INSTDIR\imageformats\qtga.dll"
  Delete "$INSTDIR\imageformats\qtiff.dll"
  Delete "$INSTDIR\imageformats\qwbmp.dll"
  Delete "$INSTDIR\imageformats\qwebp.dll"
  Delete "$INSTDIR\platforms\qwindows.dll"
  Delete "$INSTDIR\printsupport\windowsprintersupport.dll"  
  Delete "$INSTDIR\docs\Credits.txt"
  Delete "$INSTDIR\docs\Copying.txt"
  Delete "$INSTDIR\docs\License.txt" 
  Delete "$INSTDIR\3rdParty\ldglite1.3.1_2g2x_Win\ldglite.exe"
  Delete "$INSTDIR\3rdParty\ldglite1.3.1_2g2x_Win\LICENCE"
  Delete "$INSTDIR\3rdParty\ldglite1.3.1_2g2x_Win\README.TXT" 
  Delete "$INSTDIR\3rdParty\l3p1.4WinB\L3P.EXE"
  Delete "${INSTDIR_AppData}\extras\fadeStepColorParts.lst"
  Delete "${INSTDIR_AppData}\extras\freeformAnnotations.lst"
  Delete "${INSTDIR_AppData}\extras\titleAnnotations.lst"
  Delete "${INSTDIR_AppData}\extras\pliSubstituteParts.lst"
  Delete "${INSTDIR_AppData}\extras\pli.mpd"
  Delete "${INSTDIR_AppData}\dump\minidump.dmp"
  Delete "${INSTDIR_AppData}\libraries\complete.zip"
  Delete "${INSTDIR_AppData}\libraries\lpub3dldrawunf.zip"
  
  !insertmacro MUI_STARTMENU_GETFOLDER Application $StartMenuFolder
    
; Remove shortcuts
  SetShellVarContext current
  Delete "$DESKTOP\${ProductName}.lnk"
  SetShellVarContext all
  Delete "$SMPROGRAMS\$StartMenuFolder\${ProductName}.lnk"
  Delete "$SMPROGRAMS\$StartMenuFolder\Uninstall ${ProductName}.lnk"

; Remove directories used
  RMDir "$SMPROGRAMS\$StartMenuFolder"
  RMDir "${INSTDIR_AppData}\libraries"  
  RMDir "${INSTDIR_AppData}\extras"
  RMDir "${INSTDIR_AppData}\dump"
  RMDir /r "${INSTDIR_AppData}\cache"
  RMDir /r "${INSTDIR_AppData}\logs"
  RMDir "${INSTDIR_AppData}"
  RMDir "$INSTDIR\bearer"
  RMDir "$INSTDIR\iconengines"
  RMDir "$INSTDIR\imageformats"
  RMDir "$INSTDIR\platforms"
  RMDir "$INSTDIR\printsupport"
  RMDir "$INSTDIR\3rdParty\ldglite1.3.1_2g2x_Win"
  RMDir "$INSTDIR\3rdParty\l3p1.4WinB"
  RMDir "$INSTDIR\3rdParty"
  RMDir "$INSTDIR\docs"
  RMDir "$INSTDIR"
  
; Remove registry keys
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${ProductName}"
  DeleteRegKey HKCU "Software\${Company}\${ProductName}\Installation\StartMenuFolder"
  DeleteRegKey HKCU "Software\${Company}\${ProductName}\Settings\LDrawDir"
  DeleteRegKey HKCU "Software\${Company}\${ProductName}\Settings\LDSearchDirs"  
  DeleteRegKey HKCU "Software\${Company}\${ProductName}\Settings\PartsLibrary"
  DeleteRegKey HKCU "Software\${Company}\${ProductName}\Settings\FadeStepColorPartsFile"
  DeleteRegKey HKCU "Software\${Company}\${ProductName}\Settings\PliSubstitutePartsFile"
  DeleteRegKey HKCU "Software\${Company}\${ProductName}\Settings\TitleAnnotationFile"
  DeleteRegKey HKCU "Software\${Company}\${ProductName}\Settings\FreeFormAnnotationsFile"
  DeleteRegKey HKCU "Software\${Company}\${ProductName}\MainWindow"
  DeleteRegKey HKCU "Software\${Company}\${ProductName}\Defaults"
  DeleteRegKey HKCU "Software\${Company}\${ProductName}\Updates"
  DeleteRegKey HKCU "Software\${Company}\${ProductName}\POVRay"  
  DeleteRegKey /ifempty HKCU "Software\${Company}\${ProductName}\Installation"
  DeleteRegKey /ifempty HKCU "Software\${Company}\${ProductName}\Settings"
  DeleteRegKey /ifempty HKCU "Software\${Company}\${ProductName}"

  IfFileExists "$INSTDIR" 0 NoErrorMsg
    MessageBox MB_ICONEXCLAMATION "Note: $INSTDIR could not be removed!" IDOK 0 ; skipped if file doesn't exist
  NoErrorMsg: 
  
SectionEnd