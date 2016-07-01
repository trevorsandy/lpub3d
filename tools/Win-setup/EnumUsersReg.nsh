!ifndef ___EnumUsersReg___
!define ___EnumUsersReg___
 
!include "LogicLib.nsh"
 
!define TOKEN_QUERY             0x0008
!define TOKEN_ADJUST_PRIVILEGES 0x0020
 
!define SE_RESTORE_NAME         SeRestorePrivilege
 
!define SE_PRIVILEGE_ENABLED    0x00000002
 
!define HKEY_USERS              0x80000003
 
!macro _EnumUsersReg_AdjustTokens
 
StrCpy $R1 0
 
System::Call "kernel32::GetCurrentProcess() i .R0"
System::Call "advapi32::OpenProcessToken(i R0, i ${TOKEN_QUERY}|${TOKEN_ADJUST_PRIVILEGES}, \
                                         *i R1R1) i .R0"
 
${If} $R0 != 0
  System::Call "advapi32::LookupPrivilegeValue(t n, t '${SE_RESTORE_NAME}', *l .R2) i .R0"
 
  ${If} $R0 != 0
    System::Call "*(i 1, l R2, i ${SE_PRIVILEGE_ENABLED}) i .R0"
    System::Call "advapi32::AdjustTokenPrivileges(i R1, i 0, i R0, i 0, i 0, i 0)"
    System::Free $R0
  ${EndIf}
 
  System::Call "kernel32::CloseHandle(i R1)"
${EndIf}
 
!macroend
 
!macro _EnumUsersReg_InvokeCallback CALLBACK SUBKEY
 
Push $0
Push $1
Push $R0
Push $R1
Push $R2
 
Push "${SUBKEY}"
 
Call "${CALLBACK}"
 
Pop $R2
Pop $R1
Pop $R0
Pop $1
Pop $0
 
!macroend
 
!macro _EnumUsersReg_Load FILE CALLBACK SUBKEY
 
GetFullPathName /SHORT $R2 ${FILE}
System::Call "advapi32::RegLoadKey(i ${HKEY_USERS}, t '${SUBKEY}', t R2) i .R2"
 
${If} $R2 == 0
  !insertmacro _EnumUsersReg_InvokeCallback "${CALLBACK}" "${SUBKEY}"
  System::Call "advapi32::RegUnLoadKey(i ${HKEY_USERS}, t '${SUBKEY}')"
${EndIf}
 
!macroend
 
!macro EnumUsersReg UN CALLBACK SUBKEY
 
Push $0
Push $1
 
GetFunctionAddress $0 "${CALLBACK}"
StrCpy $1 "${SUBKEY}"
Call ${UN}_EnumUsersReg
 
Pop $1
Pop $0
 
!macroend
 
!define EnumUsersReg '!insertmacro EnumUsersReg ""'
!define un.EnumUsersReg '!insertmacro EnumUsersReg "un."'
 
!macro _EnumUsersReg UN
Function ${UN}_EnumUsersReg
 
Push $R0
Push $R1
Push $R2
 
# enumerate logged on users
 
StrCpy $R0 0
${Do}
  EnumRegKey $R1 HKU "" $R0
  ${If} $R1 != ""
    !insertmacro _EnumUsersReg_InvokeCallback $0 $R1
 
    IntOp $R0 $R0 + 1
  ${EndIf}
${LoopUntil} $R1 == ""
 
# enumerate logged off users
 
System::Call "kernel32::GetVersion() i .R0"
IntOp $R0 $R0 & 0x80000000
 
${If} $R0 == 0
  # nt
  !insertmacro _EnumUsersReg_AdjustTokens
 
  StrCpy $R0 0
  ${Do}
    EnumRegKey $R1 HKLM "SOFTWARE\Microsoft\Windows NT\CurrentVersion\ProfileList" $R0
    ${If} $R1 != ""
      ReadRegStr $R1 HKLM "SOFTWARE\Microsoft\Windows NT\CurrentVersion\ProfileList\$R1" \
                 ProfileImagePath
      ExpandEnvStrings $R1 $R1
 
      !insertmacro _EnumUsersReg_Load "$R1\NTUSER.DAT" $0 $1
 
      IntOp $R0 $R0 + 1
    ${EndIf}
  ${LoopUntil} $R1 == ""
${Else}
  # 9x
  ClearErrors
  FindFirst $R1 $R2 "$WINDIR\Profiles\*.*"
  ${Unless} ${Errors}
    ${Do}
      ${If} $R2 != "."
      ${AndIf} $R2 != ".."
        ${If} ${FileExists} "$WINDIR\Profiles\$R2\USER.DAT"
          !insertmacro _EnumUsersReg_Load "$WINDIR\Profiles\$R2\USER.DAT" $0 $1
        ${EndIf}
      ${EndIf}
      ClearErrors
      FindNext $R1 $R2
    ${LoopUntil} ${Errors}
    FindClose $R1
  ${EndUnless}
${Endif}
 
Pop $R2
Pop $R1
Pop $R0
 
FunctionEnd
!macroend
 
!insertmacro _EnumUsersReg ""
!insertmacro _EnumUsersReg "un."
!endif