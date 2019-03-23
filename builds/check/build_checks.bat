@ECHO OFF

Title LPub3D Windows build check script

rem  Trevor SANDY <trevor.sandy@gmail.com>
rem  Last Update: March 23, 2019
rem  Copyright (c) 2018 - 2019 by Trevor SANDY
rem --
rem This script is distributed in the hope that it will be useful,
rem but WITHOUT ANY WARRANTY; without even the implied warranty of
rem MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

rem Construct the staged files path
SET PKG_DISTRO_DIR=%PACKAGE%_%PKG_PLATFORM%
SET PKG_PRODUCT_DIR=%PACKAGE%-Any-%LP3D_APP_VERSION_LONG%
SET PKG_TARGET_DIR=builds\windows\%CONFIGURATION%\%PKG_PRODUCT_DIR%\%PKG_DISTRO_DIR%
SET PKG_TARGET=%PKG_TARGET_DIR%\%PACKAGE%.exe

rem Check 1 of 7
SET PKG_CHECK_FILE=%ABS_WD%\builds\check\build_checks.mpd
SET PKG_CHECK_OPTIONS=--process-file --liblego --preferred-renderer native
SET PKG_CHECK_NATIVE_COMMAND=%PKG_TARGET% %PKG_CHECK_OPTIONS% %PKG_CHECK_FILE%
rem Check 2 of 7
SET PKG_CHECK_OPTIONS=--process-file --clear-cache --liblego --preferred-renderer ldview
SET PKG_CHECK_LDVIEW_COMMAND=%PKG_TARGET% %PKG_CHECK_OPTIONS% %PKG_CHECK_FILE%
rem Check 3 of 7
SET PKG_CHECK_OPTIONS=--process-file --clear-cache --liblego --preferred-renderer ldview-sc
SET PKG_CHECK_LDVIEW_SINGLE_CALL_COMMAND=%PKG_TARGET% %PKG_CHECK_OPTIONS% %PKG_CHECK_FILE%
rem Check 4 of 7
SET PKG_CHECK_OPTIONS=--process-export --range 1-3 --clear-cache --liblego --preferred-renderer ldglite
SET PKG_CHECK_RANGE_COMMAND=%PKG_TARGET% %PKG_CHECK_OPTIONS% %PKG_CHECK_FILE%
rem Check 5 of 7
IF "%APPVEYOR%" EQU "True" (
  SET PKG_CHECK_OPTIONS=--process-file --clear-cache --liblego --preferred-renderer povray-ldv
) ELSE (
  SET PKG_CHECK_OPTIONS=--process-file --clear-cache --liblego --preferred-renderer povray
)
SET PKG_CHECK_POV_COMMAND=%PKG_TARGET% %PKG_CHECK_OPTIONS% %PKG_CHECK_FILE%
rem Check 6 of 7
SET PKG_CHECK_FILE=%ABS_WD%\builds\check\TENTE\astromovil.ldr
SET PKG_CHECK_OPTIONS=--process-file --clear-cache --libtente --preferred-renderer ldview
SET PKG_CHECK_TENTE_COMMAND=%PKG_TARGET% %PKG_CHECK_OPTIONS% %PKG_CHECK_FILE%
rem Check 7 of 7
SET PKG_CHECK_FILE=%ABS_WD%\builds\check\VEXIQ\spider.mpd
SET PKG_CHECK_OPTIONS=--process-file --clear-cache --libvexiq --preferred-renderer ldview-scsl
SET PKG_CHECK_VEXIQ_COMMAND=%PKG_TARGET% %PKG_CHECK_OPTIONS% %PKG_CHECK_FILE%
SET PKG_LOG_FILE=Check.out
SET /A PKG_CHECK_PASS=0
SET /A PKG_CHECK_FAIL=0
SET "PKG_CHECKS_PASS="
SET "PKG_CHECKS_FAIL="

ECHO.
ECHO   PKG_CHECK_OPTIONS.........[%PKG_CHECK_OPTIONS%]
ECHO   PKG_DISTRO_DIR............[%PKG_DISTRO_DIR%]
ECHO   PKG_PRODUCT_DIR...........[%PKG_PRODUCT_DIR%]
ECHO   PKG_TARGET_DIR............[%PKG_TARGET_DIR%]
ECHO   PKG_TARGET................[%PKG_TARGET%]
ECHO   LDRAW_LIB_STORE...........[%LDRAW_LIBS%]

CALL :SET_LDRAW_LIBS

ECHO.
ECHO ------------Build checks start--------------
ECHO.
IF NOT EXIST "%PKG_TARGET%" (
  ECHO -ERROR - %PKG_TARGET% does not exist. The build check cannot continue.
  EXIT /b
) ELSE (
  ECHO -%PKG_TARGET% found.
  IF EXIST "%PKG_LOG_FILE%" DEL /Q "%PKG_LOG_FILE%"
  ECHO.
  ECHO   1 OF 7. PKG_CHECK_NATIVE_COMMAND...[%PKG_CHECK_NATIVE_COMMAND%]
  CALL %PKG_CHECK_NATIVE_COMMAND% > %PKG_LOG_FILE% 2>&1
  FOR %%R IN (%PKG_LOG_FILE%) DO (
    IF NOT %%~zR LSS 1 (
      CALL :GET_CHECK_RESULT PKG_CHECK_NATIVE 1
      ECHO.
    ) ELSE (
      ECHO. -ERROR - PKG_CHECK_NATIVE NO OUTPUT
    )
  )
  ECHO.
  ECHO   2 OF 7. PKG_CHECK_LDVIEW_COMMAND...[%PKG_CHECK_LDVIEW_COMMAND%]
  CALL %PKG_CHECK_LDVIEW_COMMAND% > %PKG_LOG_FILE% 2>&1
  FOR %%R IN (%PKG_LOG_FILE%) DO (
    IF NOT %%~zR LSS 1 (
      CALL :GET_CHECK_RESULT PKG_CHECK_LDVIEW 2
      ECHO.
    ) ELSE (
      ECHO. -ERROR - PKG_CHECK_LDVIEW NO OUTPUT
    )
  )
  ECHO.
  ECHO   3 OF 7. PKG_CHECK_LDVIEW_SINGLE_CALL_COMMAND...[%PKG_CHECK_LDVIEW_COMMAND%]
  CALL %PKG_CHECK_LDVIEW_SINGLE_CALL_COMMAND% > %PKG_LOG_FILE% 2>&1
  FOR %%R IN (%PKG_LOG_FILE%) DO (
    IF NOT %%~zR LSS 1 (
      CALL :GET_CHECK_RESULT PKG_CHECK_LDVIEW_SINGLE_CALL 3
      ECHO.
    ) ELSE (
      ECHO. -ERROR - PKG_CHECK_LDVIEW_SINGLE_CALL NO OUTPUT
    )
  )
  ECHO.
  ECHO   4 OF 7. PKG_CHECK_RANGE_COMMAND....[%PKG_CHECK_RANGE_COMMAND%]
  CALL %PKG_CHECK_RANGE_COMMAND% > %PKG_LOG_FILE% 2>&1
  FOR %%R IN (%PKG_LOG_FILE%) DO (
    IF NOT %%~zR LSS 1 (
      CALL :GET_CHECK_RESULT PKG_CHECK_RANGE 4
      ECHO.
    ) ELSE (
      ECHO. -ERROR - PKG_CHECK_RANGE NO OUTPUT
    )
  )
  ECHO.
  ECHO   5 OF 7. PKG_CHECK_POV_COMMAND......[%PKG_CHECK_POV_COMMAND%]
  CALL %PKG_CHECK_POV_COMMAND% > %PKG_LOG_FILE% 2>&1
  FOR %%R IN (%PKG_LOG_FILE%) DO (
    IF NOT %%~zR LSS 1 (
      CALL :GET_CHECK_RESULT PKG_CHECK_POV 5
      ECHO.
    ) ELSE (
      ECHO. -ERROR - PKG_CHECK_POV NO OUTPUT
    )
  )
  ECHO.
  ECHO   6 OF 7. PKG_CHECK_TENTE_COMMAND......[%PKG_CHECK_TENTE_COMMAND%]
  CALL %PKG_CHECK_TENTE_COMMAND% > %PKG_LOG_FILE% 2>&1
  FOR %%R IN (%PKG_LOG_FILE%) DO (
    IF NOT %%~zR LSS 1 (
      CALL :GET_CHECK_RESULT PKG_CHECK_TENTE 6
      ECHO.
    ) ELSE (
      ECHO. -ERROR - PKG_CHECK_TENTE NO OUTPUT
    )
  )
  ECHO.
  ECHO   7 OF 7. PKG_CHECK_VEXIQ_COMMAND......[%PKG_CHECK_VEXIQ_COMMAND%]
  CALL %PKG_CHECK_VEXIQ_COMMAND% > %PKG_LOG_FILE% 2>&1
  FOR %%R IN (%PKG_LOG_FILE%) DO (
    IF NOT %%~zR LSS 1 (
      CALL :GET_CHECK_RESULT PKG_CHECK_VEXIQ 7
      ECHO.
    ) ELSE (
      ECHO. -ERROR - PKG_CHECK_VEXIQ NO OUTPUT
    )
  )
  ECHO.
  ECHO ----Build Check Status: PASS ^(%PKG_CHECK_PASS%^)[%PKG_CHECKS_PASS%], FAIL ^(%PKG_CHECK_FAIL%^)[%PKG_CHECKS_FAIL%]----
  ECHO.
  ECHO   Build checks cleanup...
  RMDIR /S /Q %PKG_TARGET_DIR%\cache
  RMDIR /S /Q %PKG_TARGET_DIR%\logs
  RMDIR /S /Q %PKG_TARGET_DIR%\config
  RMDIR /S /Q %PKG_TARGET_DIR%\libraries
  RMDIR /S /Q %PKG_TARGET_DIR%\VEXIQParts
  RMDIR /S /Q %PKG_TARGET_DIR%\TENTEParts
  ECHO.
  ECHO ------------Build checks finished--------------
  ECHO.
)
EXIT /b

:GET_CHECK_RESULT
SET PKG_CHECK_SUCCESS=Application terminated with return code 0.
SETLOCAL ENABLEDELAYEDEXPANSION
FOR /F "tokens=2*" %%i IN ('FINDSTR /c:"%PKG_CHECK_SUCCESS%" %PKG_LOG_FILE%') DO (
  SET PKG_CHECK_TMP=%%i %%j
)
IF "!PKG_CHECK_TMP!" EQU "%PKG_CHECK_SUCCESS%" (
  ECHO -%1 PASS
  SET /A PKG_CHECK_PASS=!PKG_CHECK_PASS!+1
  SET "PKG_CHECKS_PASS=!PKG_CHECKS_PASS!,%2"
  FOR /F %%a IN ('SET PKG_CHECK_PASS^&SET PKG_CHECKS_PASS') DO (IF "!"=="" ENDLOCAL)&SET "%%a"
) ELSE (
  ECHO -%1 FAIL
  SET /A PKG_CHECK_FAIL=!PKG_CHECK_FAIL!+1
  SET "PKG_CHECKS_FAIL=!PKG_CHECKS_FAIL!,%2"
  FOR /F %%a IN ('SET PKG_CHECK_FAIL^&SET PKG_CHECKS_FAIL') DO (IF "!"=="" ENDLOCAL)&SET "%%a"
  TYPE "%PKG_LOG_FILE%"
)
DEL /Q "%PKG_LOG_FILE%"
EXIT /b

:SET_LDRAW_LIBS
ECHO.
ECHO -Copy LDraw archive libraries to extras folder...

IF NOT EXIST "%PKG_TARGET_DIR%\extras\%OfficialCONTENT%" (
  IF EXIST "%LDRAW_LIBS%\%OfficialCONTENT%" (
    COPY /V /Y "%LDRAW_LIBS%\%OfficialCONTENT%" "%PKG_TARGET_DIR%\extras\" /A | findstr /i /v /r /c:"copied\>"
  ) ELSE (
    ECHO.
    ECHO -ERROR - LDraw archive lib %OfficialCONTENT% does not exist in %LDRAW_LIBS%\.
  )
) ELSE (
  ECHO.
  ECHO - Archive library %OfficialCONTENT% exist in extras folder. Nothing to do.
)
IF NOT EXIST "%PKG_TARGET_DIR%\extras\%TenteCONTENT%" (
  IF EXIST "%LDRAW_LIBS%\%TenteCONTENT%" (
    COPY /V /Y "%LDRAW_LIBS%\%TenteCONTENT%" "%PKG_TARGET_DIR%\extras\" /A | findstr /i /v /r /c:"copied\>"
  ) ELSE (
    ECHO.
    ECHO -ERROR - LDraw archive lib %TenteCONTENT% does not exist in %LDRAW_LIBS%\.
  )
) ELSE (
  ECHO.
  ECHO - Archive library %TenteCONTENT% exist in extras folder. Nothing to do.
)
IF NOT EXIST "%PKG_TARGET_DIR%\extras\%VexiqCONTENT%" (
  IF EXIST "%LDRAW_LIBS%\%VexiqCONTENT%" (
    COPY /V /Y "%LDRAW_LIBS%\%VexiqCONTENT%" "%PKG_TARGET_DIR%\extras\" /A | findstr /i /v /r /c:"copied\>"
  ) ELSE (
    ECHO.
    ECHO -ERROR - LDraw archive lib %VexiqCONTENT% does not exist in %LDRAW_LIBS%\.
  )
) ELSE (
  ECHO.
  ECHO - Archive library %VexiqCONTENT% exist in extras folder. Nothing to do.
)
IF NOT EXIST "%PKG_TARGET_DIR%\extras\%LPub3DCONTENT%" (
  IF EXIST "%LDRAW_LIBS%\%LPub3DCONTENT%" (
    COPY /V /Y "%LDRAW_LIBS%\%LPub3DCONTENT%" "%PKG_TARGET_DIR%\extras\" /A | findstr /i /v /r /c:"copied\>"
  ) ELSE (
    ECHO.
    ECHO -ERROR - LDraw archive lib %LPub3DCONTENT% does not exist in %LDRAW_LIBS%\.
  )
) ELSE (
  ECHO.
  ECHO - Archive library %LPub3DCONTENT% exist in extras folder. Nothing to do.
)
EXIT /b
