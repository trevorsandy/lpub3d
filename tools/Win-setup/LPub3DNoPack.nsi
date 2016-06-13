;LPub3D Setup Script
;Copyright (C) 2016 by Trevor Sandy

;--------------------------------
;Include Modern UI

  !include "MUI2.nsh"
  !include "x64.nsh"

;--------------------------------
;File & Directory Exist Macros

;FileExists is already part of LogicLib, but returns true for directories as well as files
!macro _FileExists2 _a _b _t _f
	!insertmacro _LOGICLIB_TEMP
	StrCpy $_LOGICLIB_TEMP "0"
	StrCmp `${_b}` `` +4 0 ;if path is not blank, continue to next check
	IfFileExists `${_b}` `0` +3 ;if path exists, continue to next check (IfFileExists returns true if this is a directory)
	IfFileExists `${_b}\*.*` +2 0 ;if path is not a directory, continue to confirm exists
	StrCpy $_LOGICLIB_TEMP "1" ;file exists
	;now we have a definitive value - the file exists or it does not
	StrCmp $_LOGICLIB_TEMP "1" `${_t}` `${_f}`
!macroend
!undef FileExists
!define FileExists `"" FileExists2`
!macro _DirExists _a _b _t _f
	!insertmacro _LOGICLIB_TEMP
	StrCpy $_LOGICLIB_TEMP "0"	
	StrCmp `${_b}` `` +3 0 ;if path is not blank, continue to next check
	IfFileExists `${_b}\*.*` 0 +2 ;if directory exists, continue to confirm exists
	StrCpy $_LOGICLIB_TEMP "1"
	StrCmp $_LOGICLIB_TEMP "1" `${_t}` `${_f}`
!macroend
!define DirExists `"" DirExists`

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
  OutFile "..\release\${ProductName}-UpdateMaster.exe"
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

  ;Get Ldraw library folder and archive file paths from registry if available
   ReadRegStr $LDrawDirPath HKCU "Software\${Company}\${ProductName}\Settings" "LDrawDir"

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

  ;install directory
  SetOutPath "$INSTDIR"
  
  ;executable and readme
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
  ${EndIf}
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
  SetOutPath "${INSTDIR_AppData}\libraries"
  File "..\release\libraries\complete.zip"
  File "..\release\libraries\lpub3dldrawunf.zip"
  
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
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${ProductName}" "EstimatedSize" 50000
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
  ${Else}
    MessageBox MB_ICONSTOP "You must enter the LDraw Directory to continue!" 
    Abort
  ${EndIf}

FunctionEnd

Function nsDialogShowOverwriteConfigPage

  ;--------------------------------
  ;Prompt user to overwrite configuration files

  ; === nsDialogOverwriteConfigPage (type: nsDialogFilePathsPage) ===
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
  
  ; === OverwriteMessagelbl (type: Label) ===
  ${NSD_CreateLabel} 64.51u 10.46u 170.48u 14.15u ""
  Pop $OverwriteMessagelbl
  SetCtlColors $OverwriteMessagelbl 0xFF0000 0xF0F0F0  
  
  ; === OverwriteConfigGrpBox (type: GroupBox) ===
  ${NSD_CreateGroupBox} 64.51u 31.38u 170.48u 94.77u "Configuration Files"
  Pop $OverwriteConfigGrpBox
  
  ; === chkBoxAll (type: Checkbox) ===
  ${NSD_CreateCheckbox} 74.38u 43.08u 116.51u 14.77u "All Configuration Files" 
  Pop $Overwrite_chkBoxAll
  SendMessage $Overwrite_chkBoxAll ${WM_SETFONT} $nsDialogOverwriteConfigPage_Font1 0
  
  ; === chkBoxTitle (type: Checkbox) ===
  ${NSD_CreateCheckbox} 74.38u 56.62u 116.51u 14.77u "Title Annotations"
  Pop $Overwrite_chkBoxTitle
  
  ; === chkBoxFreeform (type: Checkbox) ===
  ${NSD_CreateCheckbox} 74.38u 70.15u 116.51u 14.77u "Freeform Annotations"
  Pop $Overwrite_chkBoxFreeform
  
  ; === chkBoxSubstitute (type: Checkbox) ===
  ${NSD_CreateCheckbox} 74.38u 83.69u 116.51u 17.23u "Substitute Parts"
  Pop $Overwrite_chkBoxSubstitute
  
  ; === chkBoxFadeParts (type: Checkbox) ===
  ${NSD_CreateCheckbox} 74.38u 99.69u 116.51u 14.77u "Fade Step Colour Parts"
  Pop $Overwrite_chkBoxFadeParts
  
  ${NSD_OnClick} $Overwrite_chkBoxAll fnSetOverwriteAll
  ${NSD_OnClick} $Overwrite_chkBoxTitle fnOverwriteTitle
  ${NSD_OnClick} $Overwrite_chkBoxFreeform fnOverwriteFreeform
  ${NSD_OnClick} $Overwrite_chkBoxSubstitute fnOverwriteSubstitute
  ${NSD_OnClick} $Overwrite_chkBoxFadeParts fnOverwriteFadeParts
  
  nsDialogs::Show
  
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

Function fnWarning
    ${NSD_SetText} $OverwriteMessagelbl "WARNING! You will overwrite your custom settings."
	
FunctionEnd

Function fnClear
    ${If} $OverwriteTitleAnnotaitonsFile <> 1
	${AndIf} $OverwriteFreeformAnnotationsFile <> 1
    ${AndIf} $OverwriteFadeStepColourPartsFile <> 1
	${AndIf} $OverwriteSubstitutePartsFile <> 1
		${NSD_SetText} $OverwriteMessagelbl ""
	${EndIf}
	
FunctionEnd

Function nsDialogLeaveOverwriteConfigPage  
 ${NSD_GetState} $Overwrite_chkBoxTitle $OverwriteTitleAnnotaitonsFile
 ${NSD_GetState} $Overwrite_chkBoxFreeform $OverwriteFreeformAnnotationsFile
 ${NSD_GetState} $Overwrite_chkBoxSubstitute $OverwriteSubstitutePartsFile
 ${NSD_GetState} $Overwrite_chkBoxFadeParts $OverwriteFadeStepColourPartsFile
 
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
  Delete "$INSTDIR\Uninstall.exe"
  Delete "$INSTDIR\README.txt"
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
  RMDir "$INSTDIR\3rdParty\ldglite1.3.1_2g2x_Win"
  RMDir "$INSTDIR\3rdParty\l3p1.4WinB"
  RMDir "$INSTDIR\3rdParty"
  RMDir "$INSTDIR\docs"
  RMDir "$INSTDIR"
  
; Remove registry keys
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${ProductName}"
  DeleteRegKey HKCU "Software\${Company}\${ProductName}\Installation\StartMenuFolder"
  DeleteRegKey HKCU "Software\${Company}\${ProductName}\Settings\LDrawDir"
  DeleteRegKey /ifempty HKCU "Software\${Company}\${ProductName}\Installation"
  DeleteRegKey /ifempty HKCU "Software\${Company}\${ProductName}\Settings"
  DeleteRegKey /ifempty HKCU "Software\${Company}\${ProductName}"

  IfFileExists "$INSTDIR" 0 NoErrorMsg
    MessageBox MB_ICONEXCLAMATION "Note: $INSTDIR could not be removed!" IDOK 0 ; skipped if file doesn't exist
  NoErrorMsg: 
  
SectionEnd