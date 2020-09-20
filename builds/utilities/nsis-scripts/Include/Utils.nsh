!include LogicLib.nsh
!include x64.nsh

!define ERROR_ALREADY_EXISTS 0x000000b7
!define ERROR_ACCESS_DENIED 0x5

!macro CheckPlatform PLATFORM
	${if} ${RunningX64}
		!if ${PLATFORM} == "Win32"
			MessageBox MB_ICONSTOP "Please, run the 64-bit installer of ${PRODUCT_NAME} on this version of Windows." /SD IDOK
			Quit ; will SetErrorLevel 2 - Installation aborted by script
		!endif
	${else}
		!if ${PLATFORM} == "Win64"
			MessageBox MB_ICONSTOP "Please, run the 32-bit installer of ${PRODUCT_NAME} on this version of Windows." /SD IDOK
			Quit ; will SetErrorLevel 2 - Installation aborted by script
		!endif
	${endif}		
!macroend

!macro CheckMinWinVer MIN_WIN_VER
	${ifnot} ${AtLeastWin${MIN_WIN_VER}}	
		MessageBox MB_ICONSTOP "This program requires at least Windows ${MIN_WIN_VER}." /SD IDOK
		Quit ; will SetErrorLevel 2 - Installation aborted by script
	${endif}	
!macroend

!macro CheckSingleInstanceFunc UNINSTALLER_PREFIX
	; parameters:
	; $0 - TYPE - "Setup" or "Application"
	; $1 - SCOPE - "Global" or "Local" (default if empty)
	; $2 - MUTEX_NAME - unique mutex name
	Function ${UNINSTALLER_PREFIX}CheckSingleInstance
		Push $3
		Push $4
		Push $5

		${if} $1 == ""
			StrCpy $1 "Local"
		${endif}

		${if} $0 == "Setup"
			StrCpy $5 "The setup of ${PRODUCT_NAME}"
		${else}
			StrCpy $5 "${PRODUCT_NAME}"
		${endif}

		try:
		System::Call 'kernel32::CreateMutex(i 0, i 0, t "$1\$2") i .r3 ?e'
		Pop $4 ; the stack contains the result of GetLastError

		${if} $0 == "Application"
		    ${andif} $3 <> 0
			System::Call 'kernel32::CloseHandle(i $3)' ; close the Application mutex
		${endif}

		${if} $4 = ${ERROR_ALREADY_EXISTS}
			${orif} $4 = ${ERROR_ACCESS_DENIED}	; ERROR_ACCESS_DENIED means the mutex was created by another user and we don't have access to open it, so application is running
			; will display NSIS taskbar button, no way to hide it before GUIInit, $HWNDPARENT is 0
			MessageBox MB_RETRYCANCEL|MB_ICONEXCLAMATION "$5 is already running.$\r$\n\
				Please, close all instances of it and click Retry to continue, or Cancel to exit." /SD IDCANCEL IDCANCEL cancel
			System::Call 'kernel32::CloseHandle(i $3)' ; for next CreateMutex call to succeed
			Goto try

			cancel:
			Quit ; will SetErrorLevel 2 - Installation aborted by script
		${endif}

		Pop $5
		Pop $4
		Pop $3
	FunctionEnd
!macroend

!macro DeleteRetryAbortFunc UNINSTALLER_PREFIX
	; parameters:
	; $0 - FILE_NAME - file to delete
	Function ${UNINSTALLER_PREFIX}DeleteRetryAbort
		; unlike the File instruction, Delete doesn't abort (un)installation on error - it just sets the error flag and skips the file as if nothing happened
		try:
		ClearErrors
		Delete $0
		${if} ${errors}
			MessageBox MB_RETRYCANCEL|MB_ICONEXCLAMATION "Error deleting file:$\r$\n$\r$\n$0$\r$\n$\r$\nClick Retry to try again, or$\r$\nCancel to stop the uninstall." /SD IDCANCEL IDRETRY try
			Abort "Error deleting file $0" ; when called from section, will SetErrorLevel 2 - Installation aborted by script
		${endif}
	FunctionEnd
!macroend

!macro DeleteRetryAbort FILE_NAME
	Push $0
	
	StrCpy $0 "${FILE_NAME}"
	!ifndef __UNINSTALL__
		Call DeleteRetryAbort
	!else
	    Call un.DeleteRetryAbort
	!endif
	
	Pop $0
!macroend

!macro CheckSingleInstance TYPE SCOPE MUTEX_NAME
	Push $0
	Push $1
	Push $2
	
	StrCpy $0 "${TYPE}"
	StrCpy $1 "${SCOPE}"
	StrCpy $2 "${MUTEX_NAME}"
	!ifndef __UNINSTALL__
		Call CheckSingleInstance
	!else
		Call un.CheckSingleInstance
	!endif
	
	Pop $2
	Pop $1
	Pop $0
!macroend

!insertmacro DeleteRetryAbortFunc ""
!insertmacro CheckSingleInstanceFunc ""

; LPub3D Content
;--------------------------------

;LPub3D Setup Script - Utility Functions and Macros
;Last Update: March 26, 2019
;Copyright (C) 2016 2020 by Trevor Sandy

!ifndef UTILS_NSH
!define UTILS_NSH

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
; Backup and Restore Files Macros
; Note: To use below macros, included this file 'nsisFunctions' to your .nsi file.
; examples:!include "nsisFunctions.nsh"

; Usage:
; !insertmacro BackupFile "Dir with file in it" "File" "Backup to"
; !insertmacro RestoreFile "Backup dir with file in it" "File" "Restore to"

; Examples:
; Section "Install Files"
; #Backup old copy
; !insertmacro BackupFile "$INSTDIR" "blah1.nsi" "$INSTDIR\nsi-backup"
; #Install new copy
;  File "/oname=$INSTDIR" "blah1.nsi"
; SectionEnd

; Section "Uninstall"
; #Remove installed copy
;  Delete "$INSTDIR\blah1.nsi"
; #Restore old copy
; !insertmacro RestoreFile "$INSTDIR\nsi-backup" "blah1.nsi" "$INSTDIR"
; SectionEnd

!macro BackupFile FILE_DIR FILE BACKUP_TO
    IfFileExists "${BACKUP_TO}\*.*" +2
     CreateDirectory "${BACKUP_TO}"
    IfFileExists "${FILE_DIR}\${FILE}" 0 +2
     Rename "${FILE_DIR}\${FILE}" "${BACKUP_TO}\${FILE}"
!macroend

!macro RestoreFile BUP_DIR FILE RESTORE_TO
   IfFileExists "${BUP_DIR}\${FILE}" 0 +2
    Rename "${BUP_DIR}\${FILE}" "${RESTORE_TO}\${FILE}"
!macroend

;--------------------------------
; Backup and Restore Folder Macros
; Note: To use below macros, included this file 'nsisFunctions' to your .nsi file.
; examples:!include "nsisFunctions.nsh"

; Usage:
; !insertmacro BackupFolder "Abs path of folder to backup" "Abs path of folder to backup to"
; !insertmacro RestoreFolder "Abs path of folder to restore" "Abs path of folder to restore to"

; Examples:
; !insertmacro BackupFolder "${INSTDIR_AppData}" "$LOCALAPPDATA\temp\${ProductName}"
; !insertmacro RestoreFolder "$LOCALAPPDATA\temp\${ProductName}" "${INSTDIR_AppData}"

!macro BackupFolder FOLDER_TO_BACKUP BACKUP_TO_FOLDER
    CreateDirectory "${BACKUP_TO_FOLDER}.install.backup"
    CopyFiles /SILENT "${FOLDER_TO_BACKUP}\*.*" "${BACKUP_TO_FOLDER}.install.backup"
    IfFileExists "${BACKUP_TO_FOLDER}.install.backup\*.*" 0 +2
     DetailPrint '"${FOLDER_TO_BACKUP}" backed up successfully.'
!macroend

!macro RestoreFolder FOLDER_TO_RESTORE RESTORE_TO_FOLDER
    IfFileExists "${FOLDER_TO_RESTORE}.install.backup\*.*" 0 DirToRestoreNotFound
     CopyFiles /SILENT "${FOLDER_TO_RESTORE}.install.backup\*.*" "${RESTORE_TO_FOLDER}"
     IfFileExists "${RESTORE_TO_FOLDER}\*.*" 0 RestoreFolderFinish
      DetailPrint '"${RESTORE_TO_FOLDER}" restored successfully.'
	  DetailPrint "Cleanup ${FOLDER_TO_RESTORE}.install.backup..."
	  RMDir /r "${FOLDER_TO_RESTORE}.install.backup"
	  GoTo RestoreFolderFinish
    DirToRestoreNotFound:
     MessageBox MB_ICONEXCLAMATION "Could not find backup for ${FOLDER_TO_RESTORE}!" IDOK 0
    RestoreFolderFinish:
!macroend
;--------------------------------
; Date and time stamp
; Examples
; Name "MyApplicationName ${MyTIMESTAMP}"
; OutFile "MyApplicationNameSetup-${MyTIMESTAMP}.exe"

!define /date MyTIMESTAMP "%Y-%m-%d-%H-%M-%S"

;--------------------------------
; StrContains
; This function does a case sensitive searches for an occurrence of a substring in a string.
; It returns the substring if it is found.
; Otherwise it returns null("").
; Written by kenglish_hi
; Adapted from StrReplace written by dandaman32

Var STR_HAYSTACK
Var STR_NEEDLE
Var STR_CONTAINS_VAR_1
Var STR_CONTAINS_VAR_2
Var STR_CONTAINS_VAR_3
Var STR_CONTAINS_VAR_4
Var STR_RETURN_VAR

Function StrContains
  Exch $STR_NEEDLE
  Exch 1
  Exch $STR_HAYSTACK
  ; Uncomment to debug
  ;MessageBox MB_OK 'STR_NEEDLE = $STR_NEEDLE STR_HAYSTACK = $STR_HAYSTACK '
    StrCpy $STR_RETURN_VAR ""
    StrCpy $STR_CONTAINS_VAR_1 -1
    StrLen $STR_CONTAINS_VAR_2 $STR_NEEDLE
    StrLen $STR_CONTAINS_VAR_4 $STR_HAYSTACK
    loop:
      IntOp $STR_CONTAINS_VAR_1 $STR_CONTAINS_VAR_1 + 1
      StrCpy $STR_CONTAINS_VAR_3 $STR_HAYSTACK $STR_CONTAINS_VAR_2 $STR_CONTAINS_VAR_1
      StrCmp $STR_CONTAINS_VAR_3 $STR_NEEDLE found
      StrCmp $STR_CONTAINS_VAR_1 $STR_CONTAINS_VAR_4 done
      Goto loop
    found:
      StrCpy $STR_RETURN_VAR $STR_NEEDLE
      Goto done
    done:
   Pop $STR_NEEDLE ;Prevent "invalid opcode" errors and keep the
   Exch $STR_RETURN_VAR
FunctionEnd

!macro _StrContainsConstructor OUT NEEDLE HAYSTACK
  Push `${HAYSTACK}`
  Push `${NEEDLE}`
  Call StrContains
  Pop `${OUT}`
!macroend

!define StrContains '!insertmacro "_StrContainsConstructor"'

;--------------------------------
; GetParent
; input, top of stack  (e.g. C:\Program Files\Poop)
; output, top of stack (replaces, with e.g. C:\Program Files)
; modifies no other variables.
;
; Usage:
;   Push "C:\Program Files\Directory\Whatever"
;   Call GetParent
;   Pop $R0
;   ; at this point $R0 will equal "C:\Program Files\Directory"

Function fnGetParent

    Exch $R0
    Push $R1
    Push $R2
    Push $R3

    StrCpy $R1 0
    StrLen $R2 $R0

    loop:
        IntOp $R1 $R1 + 1
        IntCmp $R1 $R2 get 0 get
        StrCpy $R3 $R0 1 -$R1
        StrCmp $R3 "\" get
    Goto loop

    get:
        StrCpy $R0 $R0 -$R1

        Pop $R3
        Pop $R2
        Pop $R1
        Exch $R0

FunctionEnd
!endif
