;LPub3D Setup Script
;Copyright (C) 2015 by Trevor Sandy

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
;Additional plugins
  !addplugindir /x86-ansi "C:\Users\Trevor\Downloads\NSIS\Inetc\Plugins"

;--------------------------------
;LDraw Libraries

!define LDRAW_OFFICIAL_LIB	"http://www.ldraw.org/library/updates/complete.zip"
!define LDRAW_UNOFFICIAL_LIB "http://www.ldraw.org/library/unofficial/ldrawunf.zip"

;--------------------------------
;Variables

  var /global Dialog
  var /global StartMenuFolder
  var /global FileName
  var /global LDrawDirPath
  var /global LeoCADLibFile
  var /global LDrawUnoffLibFile
  var /global PathsGrpBox
  var /global BrowseLDraw
  var /global BrowseLeoCAD
  var /global LDrawText
  var /global LeoCADText
  var /global DownloadLDrawLibrary
  
;--------------------------------
;General
  
  ; Include app version details.
  !include "AppVersion.nsh"
  
  ;Installer name
  Name "${ProductName}, Ver ${Version}, Rev ${BuildRevision}"

  ; Changes the caption, default beeing 'Setup'
  Caption "${ProductName} x86 32/64-bit Setup"
  
  ; Rebrand bottom textrow
  BrandingText "${Company} Installer"

  ; Show install details
  ShowInstDetails show
  
  SetCompressor /SOLID lzma
   
  ;The file to write
  !ifdef UpdateMaster
  OutFile "..\release\${ProductName}-UpdateMaster.exe"
  !else
  OutFile "..\release\${ProductName}-${CompleteVersion}.exe"
  !endif
  
  ;Default installation folder
  InstallDir "$INSTDIR"
  
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
  LangString CUST_PAGE_SUBTITLE ${LANG_ENGLISH} "Enter paths for your LDraw directory and LeoCAD library file \
											     $\r$\nIf you do not have a LeoCAD library archive file, select Download."
 
;--------------------------------
;Initialize install directory 
 
Function .onInit 
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
  
  ;executable and readme ©
  ${If} ${RunningX64}
	File "..\release\${ProductName}_x64.exe"
  ${Else}
	File "..\release\${ProductName}_x32.exe"
  ${EndIf}
  File "..\docs\ReadMe.txt"
  
  ;3rd party renderer (LdgLite, L3P)
  CreateDirectory "$INSTDIR\3rdParty\ldglite1.2.6Win"
  SetOutPath "$INSTDIR\3rdParty\ldglite1.2.6Win"
  File "..\release\3rdParty\ldglite1.2.6Win\ldglite.exe"
  File "..\release\3rdParty\ldglite1.2.6Win\LICENCE"
  File "..\release\3rdParty\ldglite1.2.6Win\README.TXT" 
  CreateDirectory "$INSTDIR\3rdParty\ldglite1.2.6Win\plugins"
  SetOutPath "$INSTDIR\3rdParty\ldglite1.2.6Win\plugins" 
  File "..\release\3rdParty\ldglite1.2.6Win\plugins\pluginldlist.dll"
  CreateDirectory "$INSTDIR\3rdParty\l3p1.4WinB"
  SetOutPath "$INSTDIR\3rdParty\l3p1.4WinB" 
  File "..\release\3rdParty\l3p1.4WinB\L3P.EXE"
  
  ;extras contents
  SetShellVarContext all
  !define INSTDIR_AppData "$LOCALAPPDATA\${Company}\${ProductName}"
  
  CreateDirectory "${INSTDIR_AppData}\extras"
  SetOutPath "${INSTDIR_AppData}\extras"
  File "..\..\mainApp\extras\fadeStepColorParts.lst"
  File "..\..\mainApp\extras\freeformAnnotations.lst"
  File "..\..\mainApp\extras\titleAnnotations.lst"
  File "..\..\mainApp\extras\PDFPrint.jpg"
  File "..\..\mainApp\extras\pli.mpd"
  
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
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${ProductName}" "EstimatedSize" 14000
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

	${NSD_CreateGroupBox} 1u 12.31u 297.52u 120.62u "Define Library Paths"
	Pop $PathsGrpBox
	
	${NSD_CreateHLine} 7.9u 68.31u 281.72u 1.23u "HLine"
	
	${NSD_CreateLabel} 7.9u 31.38u 228.41u 11.69u "Select LDraw Directory"
	${NSD_CreateText} 7.9u 44.92u 228.41u 12.31u "$LDrawDirPath"
	Pop $LDrawText

	${NSD_CreateButton} 240.25u 44.92u 49.37u 14.15u "Browse"
	Pop $BrowseLDraw

	${NSD_CreateLabel} 7.9u 78.15u 228.41u 11.69u "Select LeoCAD Library Archive File (Complete.zip)"
	${NSD_CreateText} 7.9u 92.31u 228.41u 12.31u "$LeoCADLibFile"
	Pop $LeoCADText

	${NSD_CreateButton} 240.25u 92.31u 49.37u 14.15u "Browse"
	Pop $BrowseLeoCAD
	
	${NSD_CreateLabel} 7.9u 110.15u 228.41u 11.69u "Optional - Download Library (Complete.zip, Ldrawunf.zip)"
	
	${NSD_CreateButton} 240.25u 110.15u 49.37u 14.15u "Download"
	Pop $DownloadLDrawLibrary

	${NSD_OnClick} $BrowseLDraw fnBrowseLDraw
	${NSD_OnClick} $BrowseLeoCAD fnBrowseLeoCAD
	${NSD_OnClick} $DownloadLDrawLibrary fnDownloadLDrawLibrary
	
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

Function fnDownloadLDrawLibrary

;--------------------------------
;LDraw Archive Library Settings

  ;Validate the LDraw Directory path

  ${If} ${DirExists} $LDrawDirPath 
	Goto DoDownload
  ${Else}
    MessageBox MB_ICONEXCLAMATION "You must select the LDraw Directory to continue!" 
    Abort
  ${EndIf}
	
  DoDownload:	
	; disable browse dialog
	EnableWindow $LeoCADText 0	
	EnableWindow $BrowseLeoCAD 0 
  
	MessageBox MB_OKCANCEL|MB_USERICON "${ProductName} will download \
	the LDraw Official and Unofficial parts library to folder \
	$LDrawDirPath\LeoCAD-Libraries" IDCANCEL Cancel
	
	CreateDirectory "$LDrawDirPath\LeoCAD-Libraries"
	
	StrCpy $LeoCADLibFile "$LDrawDirPath\LeoCAD-Libraries\complete.zip"
	StrCpy $LDrawUnoffLibFile "$LDrawDirPath\LeoCAD-Libraries\ldrawunf.zip"	
	
	INETC::get /caption "Download LDraw Libraries" /popup "" ${LDRAW_OFFICIAL_LIB} $LeoCADLibFile ${LDRAW_UNOFFICIAL_LIB} $LDrawUnoffLibFile  /end
	Pop $R0 ;Get the return value
		StrCmp $R0 "OK" UpdateDialog
		MessageBox MB_ICONSTOP "Download library failed: $R0"
		; restore browse dialog
		EnableWindow $LeoCADText 1	
		EnableWindow $BrowseLeoCAD 1
		Abort
	
	UpdateDialog:	
	${NSD_SetText} $LeoCADText $LeoCADLibFile
	${NSD_SetText} $PathsGrpBox "Library paths defined - click Next to continue."
	Goto Done
	
	Cancel:
	; restore browse dialog
	EnableWindow $LeoCADText 1	
	EnableWindow $BrowseLeoCAD 1 
	
	Done:
FunctionEnd

Function nsDialogLeaveCustomPage
   
  ;Validate the LDraw Directory path
  ${If} ${DirExists} $LDrawDirPath
    ; Update the registry wiht the LDraw Directory path.
	WriteRegStr HKCU "Software\${Company}\${ProductName}\Settings" "LDrawDir" $LDrawDirPath
  ${Else}
    MessageBox MB_ICONSTOP "You must select the LDraw Directory to continue!" 
    Abort
  ${EndIf}

  ;Validate the LeoCAD Library path
  ${If} ${FileExists} $LeoCADLibFile
    ; Update the registry wiht the LeoCad Library path.
    WriteRegStr HKCU "Software\${Company}\${ProductName}\Settings" "PartsLibrary" $LeoCADLibFile
  ${Else}
    MessageBox MB_ICONSTOP "You must select the LeoCAD Library file to continue!"
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
  Delete "$INSTDIR\3rdParty\ldglite1.2.6Win\plugins\pluginldlist.dll"  
  Delete "$INSTDIR\3rdParty\ldglite1.2.6Win\ldglite.exe"
  Delete "$INSTDIR\3rdParty\ldglite1.2.6Win\LICENCE"
  Delete "$INSTDIR\3rdParty\ldglite1.2.6Win\README.TXT" 
  Delete "$INSTDIR\3rdParty\l3p1.4WinB\L3P.EXE"
  Delete "${INSTDIR_AppData}\extras\fadeStepColorParts.lst"
  Delete "${INSTDIR_AppData}\extras\freeformAnnotations.lst"
  Delete "${INSTDIR_AppData}\extras\titleAnnotations.lst"
  Delete "${INSTDIR_AppData}\extras\pli.mpd"    
  
  !insertmacro MUI_STARTMENU_GETFOLDER Application $StartMenuFolder
    
; Remove shortcuts
  SetShellVarContext current
  Delete "$DESKTOP\${ProductName}.lnk"
  SetShellVarContext all
  Delete "$SMPROGRAMS\$StartMenuFolder\${ProductName}.lnk"
  Delete "$SMPROGRAMS\$StartMenuFolder\Uninstall.lnk"

; Remove directories used
  RMDir "$SMPROGRAMS\$StartMenuFolder"
  RMDir "${INSTDIR_AppData}\extras"
  RMDir "$INSTDIR\3rdParty\ldglite1.2.6Win\plugins"
  RMDir "$INSTDIR\3rdParty\ldglite1.2.6Win"
  RMDir "$INSTDIR\3rdParty\l3p1.4WinB"
  RMDir "$INSTDIR\3rdParty"
  RMDir "$INSTDIR\docs"
  RMDir "$INSTDIR"
  
; Remove registry keys
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${ProductName}"
  DeleteRegKey HKCU "Software\${Company}\${ProductName}\Installation\StartMenuFolder"
  DeleteRegKey HKCU "Software\${Company}\${ProductName}\Settings\PartsLibrary"
  DeleteRegKey HKCU "Software\${Company}\${ProductName}\Settings\LDrawDir"
  DeleteRegKey /ifempty HKCU "Software\${Company}\${ProductName}\Installation"
  DeleteRegKey /ifempty HKCU "Software\${Company}\${ProductName}\Settings"
  DeleteRegKey /ifempty HKCU "Software\${Company}\${ProductName}"

  IfFileExists "$INSTDIR" 0 NoErrorMsg
    MessageBox MB_ICONEXCLAMATION "Note: $INSTDIR could not be removed!" IDOK 0 ; skipped if file doesn't exist
  NoErrorMsg: 
  
SectionEnd