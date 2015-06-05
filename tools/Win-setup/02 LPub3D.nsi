;LPub Setup Script
;Updated by Trevor Sandy

;--------------------------------
;Include Modern UI

  !include "MUI2.nsh"
  !include "x64.nsh"

;--------------------------------
;Variables

  var /global Dialog
  var /global StartMenuFolder
  var /global FileName
  var /global LDrawDirPath
  var /global LeoCADLibFile
  var /global BrowseLDraw
  var /global BrowseLeoCAD
  var /global LDrawText
  var /global LeoCADText
  
;--------------------------------
;General
  
  ; Include app version details.
  !include "AppVersion.nsh"
  
  ;Installer name
  Name "${ProductName}, Ver ${Version}, Rev ${BuildRevision}"

  ; Changes the caption, default beeing 'Setup'
  Caption "${ProductName} x32/x64"
  
  ; Rebrand bottom textrow
  BrandingText "${Company} Installer"

  ; Show install details
  ShowInstDetails show
  
  SetCompressor /SOLID lzma
   
  ;The file to write
  OutFile "..\release\${ProductName}-${Version}-${BuildRevision}.exe"
  
  ;Default installation folder
  InstallDir "$InstDir"
  
  ;Get installation folder from registry if available
  InstallDirRegKey HKCU "Software\${Company}\${ProductName}" "InstallPath"

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
  
  ;Start Menu Folder Page Configuration
  !define MUI_STARTMENUPAGE_DEFAULTFOLDER "${ProductName}"
  !define MUI_STARTMENUPAGE_REGISTRY_ROOT "HKCU" 
  !define MUI_STARTMENUPAGE_REGISTRY_KEY "Software\${Company}\${ProductName}\Settings" 
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
  !insertmacro MUI_PAGE_FINISH
  
  ;Uninstall pages
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
  LangString CUST_PAGE_TITLE ${LANG_ENGLISH} "Library Paths"
  LangString CUST_PAGE_SUBTITLE ${LANG_ENGLISH} "Enter paths for your LDraw Directory and LeoCAD Library File"
 
;--------------------------------
;Initialize install directory 
 
Function .onInit 
  ;Identify installation folder
  
  ${If} ${RunningX64}
	StrCpy $FileName "${ProductName}_x64.exe"
	StrCpy $InstDir "$PROGRAMFILES64\${ProductName}"
  ${Else}
	StrCpy $FileName "${ProductName}_x32.exe"
	StrCpy $InstDir "$PROGRAMFILES32\${ProductName}"
  ${EndIf}
  
FunctionEnd
 
;--------------------------------
;Installer Sections

Section "${ProductName} (required)" SecMain${ProductName}

  ;install directory
  SetOutPath "$INSTDIR"
  
  ;Executable and readme
  ${If} ${RunningX64}
	File "..\release\${ProductName}_x64.exe"
  ${Else}
	File "..\release\${ProductName}_x32.exe"
  ${EndIf}
  File "..\docs\ReadMe.txt"
  
  ;extras contents
  SetShellVarContext all
  !define InstDir_AppData "$LOCALAPPDATA\${Company}\${ProductName}"
  
  CreateDirectory "${InstDir_AppData}\extras"
  SetOutPath "${InstDir_AppData}\extras"
  File "..\..\mainApp\extras\fadeStepColorParts.lst"
  File "..\..\mainApp\extras\freeformAnnotations.lst"
  File "..\..\mainApp\extras\titleAnnotations.lst"
  File "..\..\mainApp\extras\PDFPrint.jpg"
  File "..\..\mainApp\extras\pli.mpd"
  
  ;documents  
  CreateDirectory "$INSTDIR\docs"
  SetOutPath "$INSTDIR\docs"
  File "..\docs\Credits.txt"
  File "..\docs\Copying.txt"
  SetOutPath "$INSTDIR"
  
  ;Store installation folder
  WriteRegStr HKCU "Software\${Company}\${ProductName}" "InstallPath" $INSTDIR
  
  ;Create uninstaller
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${ProductName}" "DisplayIcon" '"$INSTDIR\$FileName"'  
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${ProductName}" "DisplayName" "${ProductName}"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${ProductName}" "DisplayVersion" ${Version}
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${ProductName}" "Publisher" "${Publisher}"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${ProductName}" "UninstallString" '"$INSTDIR\Uninstall.exe"'
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${ProductName}" "EstimatedSize" 11000
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${ProductName}" "NoModify" 1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${ProductName}" "NoRepair" 1
  WriteUninstaller "$INSTDIR\Uninstall.exe"

  
  !insertmacro MUI_STARTMENU_WRITE_BEGIN Application
  
    ;Create shortcuts
	SetShellVarContext all
    CreateDirectory "$SMPROGRAMS\$StartMenuFolder"
	CreateShortCut "$SMPROGRAMS\$StartMenuFolder\${ProductName}.lnk" "$INSTDIR\$FileName"
    CreateShortCut "$SMPROGRAMS\$StartMenuFolder\Uninstall.lnk" "$INSTDIR\Uninstall.exe"
	
  !insertmacro MUI_STARTMENU_WRITE_END
  
SectionEnd
  
;--------------------------------
;Custom Dialog to Capture Libraries

Function nsDialogShowCustomPage

  ;Display the InstallOptions dialog
  !insertmacro MUI_HEADER_TEXT $(CUST_PAGE_TITLE) $(CUST_PAGE_SUBTITLE)
  
  #Create Dialog and quit if error
	nsDialogs::Create 1018
	Pop $Dialog

	${If} $Dialog == error
		Abort
	${EndIf}	
	
	${NSD_CreateLabel} 0 10 100% 12u "Select LDraw Directory"
	${NSD_CreateText} 0 30 70% 12u "$LDrawDirPath"
	Pop $LDrawText

	${NSD_CreateBrowseButton} 320 30 30% 12u "Browse"
	Pop $BrowseLDraw

	${NSD_CreateLabel} 0 60 100% 12u "Select LeoCAD Library File (Complete.zip)"
	${NSD_CreateText} 0 80 70% 12u "$LeoCADLibFile"
	Pop $LeoCADText

	${NSD_CreateBrowseButton} 320 80 30% 12u "Browse"
	Pop $BrowseLeoCAD

	${NSD_OnClick} $BrowseLDraw fnBrowseLDraw
	${NSD_OnClick} $BrowseLeoCAD fnBrowseLeoCAD

 nsDialogs::Show

FunctionEnd

Function fnBrowseLDraw

  nsDialogs::SelectFolderDialog "Select LDraw Directory" $LDrawDirPath
  Pop $LDrawDirPath
  ${NSD_SetText} $LDrawText $LDrawDirPath

FunctionEnd

Function fnBrowseLeoCAD

  nsDialogs::SelectFileDialog "open" $LDrawDirPath "LeoCAD Library|*.zip|All files|*.*"
  Pop $LeoCADLibFile
  ${NSD_SetText} $LeoCADText $LeoCADLibFile

FunctionEnd

Function nsDialogLeaveCustomPage
   
  ;Validate the LDraw Directory path
  ${If} ${FileExists} '$LDrawDirPath\*.*'
    ; Update the registry wiht the LeoCad Library path.
	WriteRegStr HKCU "Software\${Company}\${ProductName}\Settings" "LDrawDir" $LDrawDirPath
  ${Else}
    MessageBox mb_iconstop "You must select the LDraw Directory to continue!" 
    Abort
  ${EndIf}

  ;Validate the LeoCAD Library path
  ${If} ${FileExists} '$LeoCADLibFile'
    ; Update the registry wiht the LeoCad Library path.
    WriteRegStr HKCU "Software\${Company}\${ProductName}\Settings" "PartsLibrary" $LeoCADLibFile
  ${Else}
    MessageBox mb_iconstop "You must select the LeoCAD Library file to continue!"
    Abort
  ${EndIf}

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
  Delete "$INSTDIR\Uninstall.exe"
  Delete "$INSTDIR\readme.txt"
  Delete "$INSTDIR\docs\Credits.txt"
  Delete "$INSTDIR\docs\Copying.txt"
  Delete "$INSTDIR\docs\License.txt"
  Delete "$INSTDIR\extras\fadeStepColorParts.lst"
  Delete "$INSTDIR\extras\freeformAnnotations.lst"
  Delete "$INSTDIR\extras\titleAnnotations.lst"
  Delete "$INSTDIR\extras\pli.mpd"  
  
  !insertmacro MUI_STARTMENU_GETFOLDER Application $StartMenuFolder
    
; Remove shortcuts
  SetShellVarContext current
  Delete "$DESKTOP\${ProductName}.lnk"
  SetShellVarContext all
  Delete "$SMPROGRAMS\$StartMenuFolder\${ProductName}.lnk"
  Delete "$SMPROGRAMS\$StartMenuFolder\Uninstall.lnk"

; Remove directories used
  RMDir "$SMPROGRAMS\$StartMenuFolder"
  RMDir "$INSTDIR\extras"
  RMDir "$INSTDIR\docs"
  RMDir "$INSTDIR"
  
; Remove registry keys
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${ProductName}"
  DeleteRegKey HKCU "Software\${Company}\${ProductName}\Settings\StartMenuFolder"
  DeleteRegKey HKCU "Software\${Company}\${ProductName}\Settings\PartsLibrary"
  DeleteRegKey HKCU "Software\${Company}\${ProductName}\Settings\LDrawDir"
  DeleteRegKey /ifempty HKCU "Software\${Company}\${ProductName}\Settings"
  DeleteRegKey /ifempty HKCU "Software\${Company}\${ProductName}"

  IfFileExists "$INSTDIR" 0 NoErrorMsg
    MessageBox MB_OK "Note: $INSTDIR could not be removed!" IDOK 0 ; skipped if file doesn't exist
  NoErrorMsg: 
  
SectionEnd