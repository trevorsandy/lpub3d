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
  
  ;sow custom page 
  Var /global nsDialogFilePathsPage
  Var /global nsDialogFilePathsPage_Font1
  Var /global StartMenuFolder
  Var /global FileName
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
  
;--------------------------------
;General
 
  ;Installer name
  Name "${ProductName}, Ver ${Version}, Rev ${BuildRevision} (MinGW)"

  ; Changes the caption, default beeing 'Setup'
  ; Caption "${ProductName} MinGW 32, 64-bit Setup"
  Caption "${ProductName} 32-bit (MinGW) Setup"
  
  ; Rebrand bottom textrow
  BrandingText "${Company} Installer"

  ; Show install details (show|hide|nevershow)
  ShowInstDetails hide
  
  SetCompressor /SOLID lzma
   
  ;The files to write
  !ifdef UpdateMaster
  OutFile "..\release\${ProductName}-UpdateMaster_${Version}_MinGW_x32.exe"
  !else
  OutFile "..\release\${ProductName}-UpdateMaster_${Version}_MinGW_x32.exe"
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
    
  ;Start Menu Folder Page Configuration
  !define MUI_STARTMENUPAGE_DEFAULTFOLDER "${ProductName}"
  !define MUI_STARTMENUPAGE_REGISTRY_ROOT "HKCU" 
  !define MUI_STARTMENUPAGE_REGISTRY_KEY "Software\${Company}\${ProductName}\Installation" 
  !define MUI_STARTMENUPAGE_REGISTRY_VALUENAME "StartMenuFolder"
  !insertmacro MUI_PAGE_STARTMENU Application $StartMenuFolder
  
  !insertmacro MUI_PAGE_INSTFILES
  
  ;These indented statements modify settings for MUI_PAGE_FINISH
  !define MUI_FINISHPAGE_NOAUTOCLOSE	
  !define MUI_FINISHPAGE_RUN 
  !define MUI_FINISHPAGE_RUN_NOTCHECKED
  !define MUI_FINISHPAGE_RUN_TEXT "Launch ${ProductName}"
  !define MUI_FINISHPAGE_RUN_FUNCTION "RunFunction"
  
  !define MUI_FINISHPAGE_SHOWREADME "${ProductName}"
  !define MUI_FINISHPAGE_SHOWREADME_NOTCHECKED
  !define MUI_FINISHPAGE_SHOWREADME_TEXT "Install Desktop Icon"
  !define MUI_FINISHPAGE_SHOWREADME_FUNCTION "desktopIcon"
  !define MUI_FINISHPAGE_LINK "${CompanyURL}"
  !define MUI_FINISHPAGE_LINK_LOCATION "${CompanyURL}"
  
  !define MUI_PAGE_CUSTOMFUNCTION_SHOW FinishFunction
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
  LangString CUST_PAGE_SUBTITLE ${LANG_ENGLISH} "Enter path for your LDraw directory and select user data options"
  
  LangString CUST_PAGE_OVERWRITE_TITLE ${LANG_ENGLISH} "Overwrite Configuration Files"
  LangString CUST_PAGE_OVERWRITE_SUBTITLE ${LANG_ENGLISH} "Check the box next to the configuration file you would like to overwrite."
 
;--------------------------------
;Initialize install directory 
 
Function .onInit 

  ;Hack to prevent startup crash - has something to do with Qt 5.6 - investigatig...
  ; DeleteRegKey HKCU "Software\${Company}\${ProductName}\MainWindow"
  
  ;Initialize user data vars
  Call fnInitializeUserDataVars
  
  ;Get Ldraw library folder and archive file paths from registry if available
   ReadRegStr $LDrawDirPath HKCU "Software\${Company}\${ProductName}\Settings" "LDrawDir"
   ReadRegStr $LPub3DViewerLibFile HKCU "Software\${Company}\${ProductName}\Settings" "PartsLibrary"
   ReadRegStr $ParameterFile HKCU "Software\${Company}\${ProductName}\Settings" "TitleAnnotationFile"
   
  ;Verify old library directory exist - and is not the same as new library directory
   Push $LPub3DViewerLibFile
   Call fnGetParent
   Pop $R0
   StrCpy $LPub3DViewerLibPath $R0   
  ${If} ${DirExists} $LPub3DViewerLibPath
	Call fnVerifyDeleteDirectory
  ${EndIf}
  
  ;Verify if library files are installed - just check one
  IfFileExists $LPub3DViewerLibFile 0 next
  StrCpy $LibrariesExist 1
  
  next:
  ;Verify if paramter files are installed - just check one
  IfFileExists $ParameterFile 0 continue
  StrCpy $ParameterFilesExist 1  

  continue:
  ;Identify installation folder
/*   ${If} ${RunningX64}
	StrCpy $FileName "${ProductName}_x64.exe"
	StrCpy $INSTDIR "$PROGRAMFILES64\${ProductName}"
  ${Else}
	StrCpy $FileName "${ProductName}_x32.exe"
	StrCpy $INSTDIR "$PROGRAMFILES32\${ProductName}"
  ${EndIf} */
  
  	StrCpy $FileName "${ProductName}_MinGW_x32.exe"
	StrCpy $INSTDIR "$PROGRAMFILES32\${ProductName}"
    
FunctionEnd
 
;--------------------------------
;Installer Sections

Section "${ProductName} (required)" SecMain${ProductName}
  
  ;install directory
  SetOutPath "$INSTDIR"
  
  ;executable requireds and readme
/*   ${If} ${RunningX64}
	File "${Win64BuildDir}\${ProductName}_MinGW_x64.exe"
	File "${Win64MinGWBuildDir}\quazip.dll"
	File "${Win64MinGWBuildDir}\ldrawini.dll"
	File "${Win64MinGWBuildDir}\Qt5Core.dll"
	File "${Win64MinGWBuildDir}\Qt5Network.dll"
	File "${Win64MinGWBuildDir}\Qt5Gui.dll"
	File "${Win64MinGWBuildDir}\Qt5Widgets.dll"
	File "${Win64MinGWBuildDir}\Qt5PrintSupport.dll"
	File "${Win64MinGWBuildDir}\Qt5OpenGL.dll"
  ;New Stuff - Qt Libraries	
	File "${Win64MinGWBuildDir}\libgcc_s_dw2-1.dll"
	File "${Win64MinGWBuildDir}\libstdc++-6.dll"
	File "${Win64MinGWBuildDir}\libwinpthread-1.dll"
  ;New Stuff - Qt Plugins
   CreateDirectory "$INSTDIR\bearer"
   SetOutPath "$INSTDIR\bearer"
	File "${Win64MinGWBuildDir}\bearer\qgenericbearer.dll"
	File "${Win64MinGWBuildDir}\bearer\qnativewifibearer.dll"
   CreateDirectory "$INSTDIR\iconengines"
   SetOutPath "$INSTDIR\iconengines"
	File "${Win64MinGWBuildDir}\iconengines\qsvgicon.dll"
   CreateDirectory "$INSTDIR\imageformats"
   SetOutPath "$INSTDIR\imageformats"
	File "${Win64MinGWBuildDir}\imageformats\qdds.dll"
	File "${Win64MinGWBuildDir}\imageformats\qgif.dll"
	File "${Win64MinGWBuildDir}\imageformats\qicns.dll"
	File "${Win64MinGWBuildDir}\imageformats\qico.dll"
	File "${Win64MinGWBuildDir}\imageformats\qjpeg.dll"
	File "${Win64MinGWBuildDir}\imageformats\qsvg.dll"
	File "${Win64MinGWBuildDir}\imageformats\qtga.dll"
	File "${Win64MinGWBuildDir}\imageformats\qtiff.dll"
	File "${Win64MinGWBuildDir}\imageformats\qwbmp.dll"
	File "${Win64MinGWBuildDir}\imageformats\qwebp.dll"
   CreateDirectory "$INSTDIR\printsupport"
   SetOutPath "$INSTDIR\printsupport"
	File "${Win64MinGWBuildDir}\printsupport\windowsprintersupport.dll"
   CreateDirectory "$INSTDIR\platforms"
   SetOutPath "$INSTDIR\platforms"
	File "${Win64MinGWBuildDir}\platforms\qwindows.dll"		
  ${Else} */
	File "${Win32MinGWBuildDir}\${ProductName}_MinGW_x32.exe"
	File "${Win32MinGWBuildDir}\quazip.dll"
	File "${Win32MinGWBuildDir}\ldrawini.dll"
	File "${Win32MinGWBuildDir}\Qt5Core.dll"
	File "${Win32MinGWBuildDir}\Qt5Network.dll"
	File "${Win32MinGWBuildDir}\Qt5Gui.dll"
	File "${Win32MinGWBuildDir}\Qt5Widgets.dll"
	File "${Win32MinGWBuildDir}\Qt5PrintSupport.dll"
	File "${Win32MinGWBuildDir}\Qt5OpenGL.dll"
  ;New Stuff - Qt Libraries
	File "${Win32MinGWBuildDir}\libgcc_s_dw2-1.dll"  
	File "${Win32MinGWBuildDir}\libstdc++-6.dll"
	File "${Win32MinGWBuildDir}\libwinpthread-1.dll"
  ;New Stuff - Qt Plugins
   CreateDirectory "$INSTDIR\bearer"
   SetOutPath "$INSTDIR\bearer"
	File "${Win32MinGWBuildDir}\bearer\qgenericbearer.dll"
	File "${Win32MinGWBuildDir}\bearer\qnativewifibearer.dll"
   CreateDirectory "$INSTDIR\iconengines"
   SetOutPath "$INSTDIR\iconengines"
	File "${Win32MinGWBuildDir}\iconengines\qsvgicon.dll"
   CreateDirectory "$INSTDIR\imageformats"
   SetOutPath "$INSTDIR\imageformats"
	File "${Win32MinGWBuildDir}\imageformats\qdds.dll"
	File "${Win32MinGWBuildDir}\imageformats\qgif.dll"
	File "${Win32MinGWBuildDir}\imageformats\qicns.dll"
	File "${Win32MinGWBuildDir}\imageformats\qico.dll"
	File "${Win32MinGWBuildDir}\imageformats\qjpeg.dll"
	File "${Win32MinGWBuildDir}\imageformats\qsvg.dll"
	File "${Win32MinGWBuildDir}\imageformats\qtga.dll"
	File "${Win32MinGWBuildDir}\imageformats\qtiff.dll"
	File "${Win32MinGWBuildDir}\imageformats\qwbmp.dll"
	File "${Win32MinGWBuildDir}\imageformats\qwebp.dll"
   CreateDirectory "$INSTDIR\printsupport"
   SetOutPath "$INSTDIR\printsupport"
	File "${Win32MinGWBuildDir}\printsupport\windowsprintersupport.dll"
   CreateDirectory "$INSTDIR\platforms"
   SetOutPath "$INSTDIR\platforms"
	File "${Win32MinGWBuildDir}\platforms\qwindows.dll" 
  ; ${EndIf}
  
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
  
  ;data depository folder
  CreateDirectory "$INSTDIR\data"
  SetOutPath "$INSTDIR\data"
  File "..\..\mainApp\extras\LDConfig.ldr"
  File "..\..\mainApp\extras\PDFPrint.jpg"
  File "..\..\mainApp\extras\pli.mpd"
  File "..\..\mainApp\extras\titleAnnotations.lst"
  File "..\..\mainApp\extras\freeformAnnotations.lst"
  File "..\..\mainApp\extras\fadeStepColorParts.lst"
  File "..\..\mainApp\extras\pliSubstituteParts.lst"
  File "..\release\libraries\complete.zip"
  File "..\release\libraries\lpub3dldrawunf.zip"

  ;documents  
  CreateDirectory "$INSTDIR\docs"
  SetOutPath "$INSTDIR\docs"
  File "..\docs\COPYING.txt"
  File "..\docs\CREDITS.txt"
  SetOutPath "$INSTDIR"  

  ;Store installation folder
  WriteRegStr HKCU "Software\${Company}\${ProductName}\Installation" "InstallPath" $INSTDIR
  
  ;Use data setup
  ${If} $InstallUserData == 1

	  SetShellVarContext current
	  !define INSTDIR_AppData "$LOCALAPPDATA\${Company}\${ProductName}"
	  
	  ;ldraw libraries
	  CreateDirectory "${INSTDIR_AppData}\libraries"
	  
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
	  CreateDirectory "${INSTDIR_AppData}\extras"
	  SetOutPath "${INSTDIR_AppData}\extras"
	  File "..\..\mainApp\extras\PDFPrint.jpg"
	  File "..\..\mainApp\extras\pli.mpd"
	  
	 ${If} $OverwriteUserDataParamFiles == 0
	  SetOverwrite off
	  File "..\..\mainApp\extras\LDConfig.ldr"
	  File "..\..\mainApp\extras\titleAnnotations.lst"
	  File "..\..\mainApp\extras\freeformAnnotations.lst"
	  File "..\..\mainApp\extras\fadeStepColorParts.lst"
	  File "..\..\mainApp\extras\pliSubstituteParts.lst"
	 ${Else}
	  SetOverwrite on
	  File "..\..\mainApp\extras\LDConfig.ldr"
	  File "..\..\mainApp\extras\titleAnnotations.lst"
	  File "..\..\mainApp\extras\freeformAnnotations.lst"
	  File "..\..\mainApp\extras\fadeStepColorParts.lst"
	  File "..\..\mainApp\extras\pliSubstituteParts.lst"
	 ${EndIf}
	  
	  ;Store/Update library folder
	  WriteRegStr HKCU "Software\${Company}\${ProductName}\Settings" "PartsLibrary" "${INSTDIR_AppData}\libraries\complete.zip" 
  ${EndIf}
  
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

	; custom font definition
	CreateFont $nsDialogFilePathsPage_Font1 "Microsoft Sans Serif" "7.25" "400"
	
    ; === UserDataLbl (type: Label) ===
    ${NSD_CreateLabel} 7.9u 0.62u 280.41u 14.15u ""
    Pop $UserDataLbl
	SendMessage $UserDataLbl ${WM_SETFONT} $nsDialogFilePathsPage_Font1 0
    SetCtlColors $UserDataLbl 0xFF0000 0xF0F0F0
	
    ; === grpBoxPaths (type: GroupBox) ===
    ${NSD_CreateGroupBox} 7.9u 21.54u 281.72u 35.08u "Define LDraw Library Path"
	
    ; === LDrawText (type: Text) ===
    ${NSD_CreateText} 17.11u 35.08u 213.92u 12.31u "$LDrawDirPath"	
	Pop $LDrawText

    ; === BrowseLDraw (type: Button) ===
    ${NSD_CreateButton} 234.99u 33.85u 49.37u 14.15u "Browse"
	Pop $BrowseLDraw

	; === UserDataInstallChkBox (type: Checkbox) ===
	${NSD_CreateCheckbox} 7.9u 62.15u 281.72u 14.77u "Check to install user data now or uncheck to install at first application launch"
	Pop $UserDataInstallChkBox
	
	; === DeleteOldUserDataDirectoryChkBox (type: Checkbox) ===
    ${NSD_CreateCheckbox} 7.9u 118.15u 143.49u 14.77u "Delete old archive library directory"
    Pop $DeleteOldUserDataDirectoryChkBox
  
    ; === OverwriteUserDataParamFilesChkBox (type: Checkbox) ===
    ${NSD_CreateCheckbox} 7.9u 80.62u 143.49u 14.77u "Overwrite existing parameter files"
    Pop $OverwriteUserDataParamFilesChkBox

    ; === CopyExistingUserDataLibrariesChkBox (type: Checkbox) ===
    ${NSD_CreateCheckbox} 7.9u 99.08u 143.49u 14.77u "Use existing LDraw archive libraries"
    Pop $CopyExistingUserDataLibrariesChkBox	
  
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
    	  ${NSD_SetText} $UserDataLbl "NOTICE! Data created under Administrator user AppData path. Standard users will not have access."
	
FunctionEnd

Function fnMoveLibrariesInfo
    ${NSD_SetText} $UserDataLbl "INFO: LDraw library archives will be moved to a new directory:$\r$\n'$LOCALAPPDATA\${Company}\${ProductName}\libraries'."
	
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
  ${StrContains} $0 "$LOCALAPPDATA\${Company}\${ProductName}\libraries" $LPub3DViewerLibPath
    StrCmp $0 "" doNotMatch
    StrCpy $OldLibraryDirectoryExist 1
    Goto Finish
  doNotMatch:
    StrCpy $OldLibraryDirectoryExist 0
  Finish:
    ;MessageBox MB_ICONEXCLAMATION "fnVerifyDeleteDirectory LibrariesExist = $LibrariesExist$\r$\nCompare this: ($LOCALAPPDATA\${Company}\${ProductName}\libraries)$\r$\nto ($LPub3DViewerLibPath)" IDOK 0 
FunctionEnd

Function fnInstallLibraries
	SetOutPath "${INSTDIR_AppData}\libraries"
	File "..\release\libraries\complete.zip"
	File "..\release\libraries\lpub3dldrawunf.zip"
	
FunctionEnd

Function fnCopyLibraries
	SetOutPath "${INSTDIR_AppData}\libraries"
	IfFileExists "${INSTDIR_AppData}\libraries\complete.zip" 0 +2
	goto Next
	IfFileExists "$LPub3DViewerLibPath\complete.zip" 0 install_new_off_Lib
	CopyFiles "$LPub3DViewerLibPath\complete.zip" "${INSTDIR_AppData}\libraries\complete.zip"
	goto Next
	install_new_off_Lib:
	File "..\release\libraries\complete.zip"
	Next:
	IfFileExists "${INSTDIR_AppData}\libraries\lpub3dldrawunf.zip" 0 +2
	goto Finish
	IfFileExists "$LPub3DViewerLibPath\ldrawunf.zip" 0 install_new_unoff_Lib
	CopyFiles "$LPub3DViewerLibPath\ldrawunf.zip" "${INSTDIR_AppData}\libraries\lpub3dldrawunf.zip"
	goto Finish
	install_new_unoff_Lib:
	File "..\release\libraries\lpub3dldrawunf.zip"	
	Finish:
	
FunctionEnd

Function desktopIcon

    SetShellVarContext current
    CreateShortCut "$DESKTOP\${ProductName}.lnk" "$INSTDIR\$FileName"
	
FunctionEnd

Function FinishFunction
  
  ShowWindow $mui.Finishpage.Run ${SW_HIDE}
/*   ${If} $ParameterFilesExist == 1
  ${OrIf} InstallUserData == 1
    ShowWindow $mui.Finishpage.Run ${SW_SHOW}
  ${Else}
	ShowWindow $mui.Finishpage.Run ${SW_HIDE}
  ${EndIf} */
  
FunctionEnd

Function RunFunction
  ; Insert application to run here!
  Exec '"$INSTDIR\$FileName"'

FunctionEnd

;--------------------------------
;Uninstaller Section

!include "un.EnumUsersReg.nsh"

Section "Uninstall"

; Remove files
/*   ${If} ${RunningX64}
	Delete "$INSTDIR\${ProductName}_x64.exe"
  ${Else}
	Delete "$INSTDIR\${ProductName}_x32.exe"
  ${EndIf} */
  Delete "$INSTDIR\${ProductName}_MinGW_x32.exe"
 
  Delete "$INSTDIR\docs\Credits.txt"
  Delete "$INSTDIR\docs\Copying.txt"
  Delete "$INSTDIR\docs\License.txt" 
  Delete "$INSTDIR\3rdParty\ldglite1.3.1_2g2x_Win\ldglite.exe"
  Delete "$INSTDIR\3rdParty\ldglite1.3.1_2g2x_Win\LICENCE"
  Delete "$INSTDIR\3rdParty\ldglite1.3.1_2g2x_Win\README.TXT" 
  Delete "$INSTDIR\3rdParty\l3p1.4WinB\L3P.EXE"
  Delete "$INSTDIR\data\PDFPrint.jpg"
  Delete "$INSTDIR\data\pli.mpd"
  Delete "$INSTDIR\data\titleAnnotations.lst"
  Delete "$INSTDIR\data\freeformAnnotations.lst"
  Delete "$INSTDIR\data\fadeStepColorParts.lst"
  Delete "$INSTDIR\data\pliSubstituteParts.lst"
  Delete "$INSTDIR\data\complete.zip"
  Delete "$INSTDIR\data\lpub3dldrawunf.zip"
  
  Delete "$INSTDIR\quazip.dll"
  Delete "$INSTDIR\ldrawini.dll"
  Delete "$INSTDIR\Qt5Core.dll"
  Delete "$INSTDIR\Qt5Network.dll"
  Delete "$INSTDIR\Qt5Gui.dll"
  Delete "$INSTDIR\Qt5Widgets.dll"
  Delete "$INSTDIR\Qt5PrintSupport.dll"
  Delete "$INSTDIR\Qt5OpenGL.dll"
  Delete "$INSTDIR\libgcc_s_dw2-1.dll"  
  Delete "$INSTDIR\libstdc++-6.dll"
  Delete "$INSTDIR\libwinpthread-1.dll"
  Delete "$INSTDIR\platforms\qwindows.dll"

  Delete "$INSTDIR\README.txt"
  Delete "$INSTDIR\Uninstall.exe"

  !insertmacro MUI_STARTMENU_GETFOLDER Application $StartMenuFolder
    
; Remove shortcuts
  SetShellVarContext current
  Delete "$DESKTOP\${ProductName}.lnk"
  SetShellVarContext all
  Delete "$SMPROGRAMS\$StartMenuFolder\${ProductName}.lnk"
  Delete "$SMPROGRAMS\$StartMenuFolder\Uninstall ${ProductName}.lnk"
	
; Remove directories used
  RMDir "$SMPROGRAMS\$StartMenuFolder"

  RMDir "$INSTDIR\platforms"
  RMDir "$INSTDIR\3rdParty\ldglite1.3.1_2g2x_Win"
  RMDir "$INSTDIR\3rdParty\l3p1.4WinB"
  RMDir "$INSTDIR\3rdParty"
  RMDir "$INSTDIR\docs"
  RMDir "$INSTDIR\data"
  RMDir /r "$INSTDIR"
  
    ;Use data uninstall
  ${If} $InstallUserData == 1
	Delete "${INSTDIR_AppData}\extras\fadeStepColorParts.lst"
	Delete "${INSTDIR_AppData}\extras\freeformAnnotations.lst"
	Delete "${INSTDIR_AppData}\extras\titleAnnotations.lst"
	Delete "${INSTDIR_AppData}\extras\pliSubstituteParts.lst"
	Delete "${INSTDIR_AppData}\extras\pli.mpd"
	Delete "${INSTDIR_AppData}\extras\LDConfig.ldr"
	Delete "${INSTDIR_AppData}\extras\PDFPrint.jpg"
	Delete "${INSTDIR_AppData}\dump\minidump.dmp"
	Delete "${INSTDIR_AppData}\libraries\complete.zip"
	Delete "${INSTDIR_AppData}\libraries\lpub3dldrawunf.zip"  
	
	RMDir "${INSTDIR_AppData}\libraries"  
	RMDir "${INSTDIR_AppData}\extras"
	RMDir "${INSTDIR_AppData}\dump"
	RMDir /r "${INSTDIR_AppData}\cache"
	RMDir /r "${INSTDIR_AppData}\logs"
	RMDir "${INSTDIR_AppData}"
  ${EndIf}
  
  ; Uninstall Users Data
  ${un.EnumUsersReg} un.EraseAppDataCB temp.key
  
; Remove registry keys
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${ProductName}"
  DeleteRegKey HKCU "Software\${Company}\${ProductName}\Defaults" 
  DeleteRegKey HKCU "Software\${Company}\${ProductName}\Installation"
  DeleteRegKey HKCU "Software\${Company}\${ProductName}\MainWindow"
  DeleteRegKey HKCU "Software\${Company}\${ProductName}\ParmsWindow"
  DeleteRegKey HKCU "Software\${Company}\${ProductName}\POVRay" 
  DeleteRegKey HKCU "Software\${Company}\${ProductName}\Settings"  
  DeleteRegKey HKCU "Software\${Company}\${ProductName}\Updates"
  DeleteRegKey HKCU "Software\${Company}\${ProductName}"
  DeleteRegKey HKCU "Software\${Company}"

  IfFileExists "$INSTDIR" 0 NoErrorMsg
    MessageBox MB_ICONEXCLAMATION "Note: $INSTDIR could not be removed!" IDOK 0 ; skipped if file doesn't exist
  NoErrorMsg: 
  
SectionEnd

Function "un.EraseAppDataCB"
 Pop $0
 ReadRegStr $0 HKU "$0\Software\Microsoft\Windows\CurrentVersion\Explorer\Shell Folders" "AppData"
 ;RMDir /r /REBOOTOK "$0\${Company}"
  RMDir /r "$0\${Company}"
  
FunctionEnd
