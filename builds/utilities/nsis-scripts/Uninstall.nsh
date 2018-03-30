!insertmacro DeleteRetryAbortFunc "un."

Var SemiSilentMode ; installer started uninstaller in semi-silent mode using /SS parameter
Var RunningFromInstaller ; installer started uninstaller using /uninstall parameter

; Installer Attributes
ShowUninstDetails hide

; Pages
!define MUI_UNWELCOMEFINISHPAGE_BITMAP "..\icons\welcome.bmp"
!define MUI_UNABORTWARNING ; Show a confirmation when cancelling the installation

!insertmacro MUI_UNPAGE_WELCOME

!define MULTIUSER_INSTALLMODE_CHANGE_MODE_FUNCTION un.PageInstallModeChangeMode
!insertmacro MULTIUSER_UNPAGE_INSTALLMODE

!define MUI_PAGE_CUSTOMFUNCTION_PRE un.PageComponentsPre
!define MUI_PAGE_CUSTOMFUNCTION_SHOW un.PageComponentsShow
!insertmacro MUI_UNPAGE_COMPONENTS

!insertmacro MUI_UNPAGE_INSTFILES

Section "un.Program Files" SectionUninstallProgram
	SectionIn RO

	; Try to delete the EXE as the first step - if it's in use, don't remove anything else
	!insertmacro un.DeleteRetryAbort "$INSTDIR\${PROGEXE}"

	!ifdef LICENSE_FILE
		!insertmacro un.DeleteRetryAbort "$INSTDIR\${LICENSE_FILE}"
	!endif

	; Files to be uninstalled.
	!include "LPub3DUninstallFiles.nsh"

	; Clean up "Documentation"
	!insertmacro un.DeleteRetryAbort "$INSTDIR\${README_FILE}"

	; Delete non-configuration User data
	RMDir /r "${INSTDIR_AppDataProduct}\dump"
	RMDir /r "${INSTDIR_AppDataProduct}\logs"
	RMDir /r "${INSTDIR_AppDataProduct}\cache"

  ; Clean up "Program Group" - we check that we created Start menu folder, if $StartMenuFolder is empty, the whole $SMPROGRAMS directory will be removed!
	${if} "$StartMenuFolder" != ""
		RMDir /r "$SMPROGRAMS\$StartMenuFolder"
	${endif}

	; Clean up "Dektop Icon"
	!insertmacro MULTIUSER_GetCurrentUserString $0
	!insertmacro un.DeleteRetryAbort "$DESKTOP\${PRODUCT_NAME}$0.lnk"

	; Clean up "Start Menu Icon"
	!insertmacro MULTIUSER_GetCurrentUserString $0
	!insertmacro un.DeleteRetryAbort "$STARTMENU\${PRODUCT_NAME}$0.lnk"

  ; Clean up "Quick Launch Icon"
	!insertmacro un.DeleteRetryAbort "$QUICKLAUNCH\${PRODUCT_NAME}.lnk"
SectionEnd

Section /o "un.User Data" SectionRemoveUserData
	; Delete all User data - this section is executed only explicitly and shouldn't be placed in SectionUninstallProgram
	RMDir /r "${INSTDIR_LocalAppData}\${COMPANY_NAME}"
SectionEnd

Section /o "un.User Registry Settings" SectionRemoveUserRegistryKeys
  ; Delete all settings regisry hive keys - this section is executed only explicitly and shouldn't be placed in SectionUninstallProgram
	DeleteRegKey HKCU "Software\${COMPANY_NAME}"
SectionEnd

Section "-Uninstall" ; hidden section, must always be the last one!
	; Remove the uninstaller from registry as the very last step - if sth. goes wrong, let the user run it again
	!insertmacro MULTIUSER_RegistryRemoveInstallInfo ; Remove registry keys

  Delete "$INSTDIR\${UNINSTALL_FILENAME}"
  ; remove the directory only if it is empty - the user might have saved some files in it
	RMDir "$INSTDIR"
SectionEnd

; Modern install component descriptions
!insertmacro MUI_UNFUNCTION_DESCRIPTION_BEGIN
	!insertmacro MUI_DESCRIPTION_TEXT ${SectionUninstallProgram} "Uninstall ${PRODUCT_NAME} files."
	!insertmacro MUI_DESCRIPTION_TEXT ${SectionRemoveUserData} "Remove ${PRODUCT_NAME} User Data. Select only if this user does not plan to use the program in the future."
	!insertmacro MUI_DESCRIPTION_TEXT ${SectionRemoveUserRegistryKeys} "Remove User ${PRODUCT_NAME} Registry Settings. Select only if you don't expect this user to reinstall the program in the future."
!insertmacro MUI_UNFUNCTION_DESCRIPTION_END

; Callbacks
Function un.onInit
	${GetParameters} $R0

	${GetOptions} $R0 "/uninstall" $R1
	${ifnot} ${errors}
		StrCpy $RunningFromInstaller 1
	${else}
		StrCpy $RunningFromInstaller 0
	${endif}

	${GetOptions} $R0 "/SS" $R1
	${ifnot} ${errors}
		StrCpy $SemiSilentMode 1
		SetAutoClose true ; auto close (if no errors) if we are called from the installer; if there are errors, will be automatically set to false
	${else}
		StrCpy $SemiSilentMode 0
	${endif}

	${ifnot} ${UAC_IsInnerInstance}
		${andif} $RunningFromInstaller$SemiSilentMode == "00"
		!insertmacro CheckSingleInstance "${SINGLE_INSTANCE_ID}"
	${endif}

	;Set context to 'CurrentUser', capture $AppDataBaseDir then revert context to 'AllUsers' if previously set as such
	StrCpy $R0 "$SMPROGRAMS"
	SetShellVarContext current
	StrCpy $AppDataBaseDir "$LOCALAPPDATA"
	StrCmp $R0 "$SMPROGRAMS" 0 SetAll
	Goto Done
	SetAll:
	SetShellVarContext all
	Done:

	!insertmacro MULTIUSER_UNINIT
FunctionEnd

Function un.PageInstallModeChangeMode
	!insertmacro MUI_STARTMENU_GETFOLDER "" $StartMenuFolder
FunctionEnd

Function un.PageComponentsPre
	${if} $SemiSilentMode == 1
		Abort ; if user is installing, no use to remove program settings anyway (should be compatible with all versions)
	${endif}
FunctionEnd

Function un.PageComponentsShow
	; Show/hide the Back button
	GetDlgItem $0 $HWNDPARENT 3
	ShowWindow $0 $UninstallShowBackButton
FunctionEnd

Function un.onUninstFailed
	${if} $SemiSilentMode == 0
		MessageBox MB_ICONSTOP "${PRODUCT_NAME} ${VERSION} could not be fully uninstalled.$\r$\nPlease, restart Windows and run the uninstaller again." /SD IDOK
	${else}
		MessageBox MB_ICONSTOP "${PRODUCT_NAME} could not be fully installed.$\r$\nPlease, restart Windows and run the setup program again." /SD IDOK
	${endif}
FunctionEnd
