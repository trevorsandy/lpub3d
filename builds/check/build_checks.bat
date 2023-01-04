@ECHO OFF &SETLOCAL

Title LPub3D Windows build check script

rem  Trevor SANDY <trevor.sandy@gmail.com>
rem  Last Update: October 26, 2022
rem  Copyright (C) 2018 - 2023 by Trevor SANDY
rem --
rem This script is distributed in the hope that it will be useful,
rem but WITHOUT ANY WARRANTY; without even the implied warranty of
rem MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

rem Construct the staged files path

SET PKG_CHECK_PATH=%ABS_WD%\builds\check
SET PKG_DISTRO_DIR=%PACKAGE%_%PKG_PLATFORM%
SET PKG_PRODUCT_DIR=%PACKAGE%-Any-%LP3D_APP_VERSION_LONG%
SET PKG_CONFIG_DIR=builds\windows\%CONFIGURATION%
SET PKG_RUNLOG_DIR=%PKG_CONFIG_DIR%\%PKG_PRODUCT_DIR%\%PACKAGE%_Logs
SET PKG_TARGET_DIR=%PKG_CONFIG_DIR%\%PKG_PRODUCT_DIR%\%PKG_DISTRO_DIR%
SET PKG_TARGET=%PKG_TARGET_DIR%\%PACKAGE%.exe
SET PKG_RUNLOG=%PKG_TARGET_DIR%\logs\%PACKAGE%Log.txt
SET PKG_DUMP_FILE=%TEMP%\%PACKAGE%.dmp
IF /I "%PKG_PLATFORM%"=="x86" (
  SET PKG_TARGET_PDB=mainApp\32bit_%CONFIGURATION%\%PACKAGE%.pdb
) ELSE (
  IF /I "%PKG_PLATFORM%"=="x86_64" (
    SET PKG_TARGET_PDB=mainApp\64bit_%CONFIGURATION%\%PACKAGE%.pdb
  )
)
IF NOT "%INSTALL%" EQU "1" SET PKG_RUNLOG_DIR=%LP3D_DOWNLOADS_PATH%
IF "%BUILD_OPT%" EQU "verify" SET PKG_RUNLOG_DIR=%LP3D_DOWNLOADS_PATH%

SET PKG_CHECKS=7
rem Check 1 of 7
SET PKG_CHECK_FILE=%PKG_CHECK_PATH%\build_checks.mpd
SET PKG_CHECK_OPTIONS=--no-console-redirect --process-file --liblego --preferred-renderer native
SET PKG_CHECK_NATIVE_COMMAND=%PKG_TARGET% %PKG_CHECK_OPTIONS% %PKG_CHECK_FILE%
rem Check 2 of 7
SET PKG_CHECK_OPTIONS=--no-console-redirect --process-file --clear-cache --liblego --preferred-renderer ldview
SET PKG_CHECK_LDVIEW_COMMAND=%PKG_TARGET% %PKG_CHECK_OPTIONS% %PKG_CHECK_FILE%
rem Check 3 of 7
SET PKG_CHECK_OPTIONS=--no-console-redirect --process-file --clear-cache --liblego --preferred-renderer ldview-sc
SET PKG_CHECK_LDVIEW_SINGLE_CALL_COMMAND=%PKG_TARGET% %PKG_CHECK_OPTIONS% %PKG_CHECK_FILE%
rem Check 4 of 7
SET PKG_CHECK_OPTIONS=--no-console-redirect --process-export --range 1-3 --clear-cache --liblego --preferred-renderer ldglite
SET PKG_CHECK_RANGE_COMMAND=%PKG_TARGET% %PKG_CHECK_OPTIONS% %PKG_CHECK_FILE%
rem Check 5 of 7
SET PKG_CHECK_OPTIONS=--no-console-redirect --process-file --clear-cache --liblego --preferred-renderer povray-ldv
IF "%GITHUB%" NEQ "True" (
  IF "%APPVEYOR%" NEQ "True" (
    SET PKG_CHECK_OPTIONS=--no-console-redirect --process-file --clear-cache --liblego --preferred-renderer povray
  )
)
SET PKG_CHECK_POV_COMMAND=%PKG_TARGET% %PKG_CHECK_OPTIONS% %PKG_CHECK_FILE%
rem Check 6 of 7
SET PKG_CHECK_FILE=%ABS_WD%\builds\check\TENTE\astromovil.ldr
SET PKG_CHECK_OPTIONS=--no-console-redirect --process-file --clear-cache --libtente --preferred-renderer ldview
SET PKG_CHECK_TENTE_COMMAND=%PKG_TARGET% %PKG_CHECK_OPTIONS% %PKG_CHECK_FILE%
rem Check 7 of 7
SET PKG_CHECK_FILE=%ABS_WD%\builds\check\VEXIQ\spider.mpd
SET PKG_CHECK_OPTIONS=--no-console-redirect --process-file --clear-cache --libvexiq --preferred-renderer ldview-scsl
SET PKG_CHECK_VEXIQ_COMMAND=%PKG_TARGET% %PKG_CHECK_OPTIONS% %PKG_CHECK_FILE%

rem Setup logging and check status
IF NOT EXIST "%TEMP%\$" (
  MD "%TEMP%\$"
)
SET PKG_LOG_FILE=%TEMP%\$\Check.out
SET PKG_CHECK_PASS_IN=CheckPass.in
SET PKG_UPDATE_CHECK_PASS=%TEMP%\$\%PKG_CHECK_PASS_IN% ECHO
SET PKG_CHECKS_PASS_IN=ChecksPass.in
SET PKG_UPDATE_CHECKS_PASS=%TEMP%\$\%PKG_CHECKS_PASS_IN% ECHO
SET PKG_CHECK_FAIL_IN=CheckFail.in
SET PKG_UPDATE_CHECK_FAIL=%TEMP%\$\%PKG_CHECK_FAIL_IN% ECHO
SET PKG_CHECKS_FAIL_IN=ChecksFail.in
SET PKG_UPDATE_CHECKS_FAIL=%TEMP%\$\%PKG_CHECKS_FAIL_IN% ECHO
>%PKG_UPDATE_CHECK_PASS% 0
>%PKG_UPDATE_CHECKS_PASS%.
>%PKG_UPDATE_CHECK_FAIL% 0
>%PKG_UPDATE_CHECKS_FAIL%.

ECHO.
ECHO   PACKAGE...................[%PACKAGE%]
ECHO   PKG_PLATFORM..............[%PKG_PLATFORM%]
ECHO   CONFIGURATION.............[%CONFIGURATION%]
ECHO   BUILD_OPT.................[%BUILD_OPT%]
ECHO   PKG_DISTRO_DIR............[%PKG_DISTRO_DIR%]
ECHO   PKG_PRODUCT_DIR...........[%PKG_PRODUCT_DIR%]
ECHO   PKG_TARGET_DIR............[%PKG_TARGET_DIR%]
ECHO   PKG_TARGET................[%PKG_TARGET%]
ECHO   PKG_TARGET_PDB............[%PKG_TARGET_PDB%]
ECHO   PKG_DUMP_FILE.............[%PKG_DUMP_FILE%]
ECHO   LDRAW_LIB_STORE...........[%LDRAW_LIBS%]

CALL :SET_LDRAW_LIBS
CALL :SET_PKG_ZIP_UTILITY

SET overall_check_start=%time%

ECHO.
ECHO ------------Build Checks Start--------------
ECHO.
IF NOT EXIST "%PKG_TARGET%" (
  ECHO -ERROR - %PKG_TARGET% does not exist. The build check cannot continue.
  EXIT /b
)
ECHO -%PKG_TARGET% found.
IF EXIST "%PKG_LOG_FILE%" DEL /Q "%PKG_LOG_FILE%"
SET PKG_CHECK=1
ECHO.
ECHO -CHECK %PKG_CHECK% OF %PKG_CHECKS%. PKG_CHECK_NATIVE_COMMAND...[%PKG_CHECK_NATIVE_COMMAND%]
SET QT_DEBUG_PLUGINS=1
CALL :ELAPSED_CHECK_TIME Start
CALL %PKG_CHECK_NATIVE_COMMAND% > %PKG_LOG_FILE% 2>&1
FOR %%R IN (%PKG_LOG_FILE%) DO (
  IF NOT %%~zR LSS 1 (
    CALL :GET_PKG_CHECK_RESULT PKG_CHECK_NATIVE %PKG_CHECK%
    ECHO.
  ) ELSE (
    ECHO. -ERROR - PKG_CHECK_NATIVE NO OUTPUT
  )
)
SET PKG_CHECK=2
ECHO.
ECHO -CHECK %PKG_CHECK% OF %PKG_CHECKS%. PKG_CHECK_LDVIEW_COMMAND...[%PKG_CHECK_LDVIEW_COMMAND%]
CALL :ELAPSED_CHECK_TIME Start
CALL %PKG_CHECK_LDVIEW_COMMAND% > %PKG_LOG_FILE% 2>&1
FOR %%R IN (%PKG_LOG_FILE%) DO (
  IF NOT %%~zR LSS 1 (
    CALL :GET_PKG_CHECK_RESULT PKG_CHECK_LDVIEW %PKG_CHECK%
    ECHO.
  ) ELSE (
    ECHO. -ERROR - PKG_CHECK_LDVIEW NO OUTPUT
  )
)
SET PKG_CHECK=3
ECHO.
ECHO -CHECK %PKG_CHECK% OF %PKG_CHECKS%. PKG_CHECK_LDVIEW_SINGLE_CALL_COMMAND...[%PKG_CHECK_LDVIEW_COMMAND%]
CALL :ELAPSED_CHECK_TIME Start
CALL %PKG_CHECK_LDVIEW_SINGLE_CALL_COMMAND% > %PKG_LOG_FILE% 2>&1
FOR %%R IN (%PKG_LOG_FILE%) DO (
  IF NOT %%~zR LSS 1 (
    CALL :GET_PKG_CHECK_RESULT PKG_CHECK_LDVIEW_SINGLE_CALL %PKG_CHECK%
    ECHO.
  ) ELSE (
    ECHO. -ERROR - PKG_CHECK_LDVIEW_SINGLE_CALL NO OUTPUT
  )
)
SET PKG_CHECK=4
ECHO.
ECHO -CHECK %PKG_CHECK% OF %PKG_CHECKS%. PKG_CHECK_RANGE_COMMAND....[%PKG_CHECK_RANGE_COMMAND%]
CALL :ELAPSED_CHECK_TIME Start
CALL %PKG_CHECK_RANGE_COMMAND% > %PKG_LOG_FILE% 2>&1
FOR %%R IN (%PKG_LOG_FILE%) DO (
  IF NOT %%~zR LSS 1 (
    CALL :GET_PKG_CHECK_RESULT PKG_CHECK_RANGE %PKG_CHECK%
    ECHO.
  ) ELSE (
    ECHO. -ERROR - PKG_CHECK_RANGE NO OUTPUT
  )
)
SET PKG_CHECK=5
ECHO.
ECHO -CHECK %PKG_CHECK% OF %PKG_CHECKS%. PKG_CHECK_POV_COMMAND......[%PKG_CHECK_POV_COMMAND%]
CALL :ELAPSED_CHECK_TIME Start
CALL %PKG_CHECK_POV_COMMAND% > %PKG_LOG_FILE% 2>&1
FOR %%R IN (%PKG_LOG_FILE%) DO (
  IF NOT %%~zR LSS 1 (
    CALL :GET_PKG_CHECK_RESULT PKG_CHECK_POV %PKG_CHECK%
    ECHO.
  ) ELSE (
    ECHO. -ERROR - PKG_CHECK_POV NO OUTPUT
  )
)
SET PKG_CHECK=6
ECHO.
ECHO -CHECK %PKG_CHECK% OF %PKG_CHECKS%. PKG_CHECK_TENTE_COMMAND......[%PKG_CHECK_TENTE_COMMAND%]
CALL :ELAPSED_CHECK_TIME Start
CALL %PKG_CHECK_TENTE_COMMAND% > %PKG_LOG_FILE% 2>&1
FOR %%R IN (%PKG_LOG_FILE%) DO (
  IF NOT %%~zR LSS 1 (
    CALL :GET_PKG_CHECK_RESULT PKG_CHECK_TENTE %PKG_CHECK%
    ECHO.
  ) ELSE (
    ECHO. -ERROR - PKG_CHECK_TENTE NO OUTPUT
  )
)
SET PKG_CHECK=7
ECHO.
ECHO -CHECK %PKG_CHECK% OF %PKG_CHECKS%. PKG_CHECK_VEXIQ_COMMAND......[%PKG_CHECK_VEXIQ_COMMAND%]
CALL :ELAPSED_CHECK_TIME Start
CALL %PKG_CHECK_VEXIQ_COMMAND% > %PKG_LOG_FILE% 2>&1
FOR %%R IN (%PKG_LOG_FILE%) DO (
  IF NOT %%~zR LSS 1 (
    CALL :GET_PKG_CHECK_RESULT PKG_CHECK_VEXIQ %PKG_CHECK%
    ECHO.
  ) ELSE (
    ECHO. -ERROR - PKG_CHECK_VEXIQ NO OUTPUT
  )
)

CALL :CLEANUP_CHECK_FOLDERS

SET /P PKG_CHECK_PASS=<%TEMP%\$\%PKG_CHECK_PASS_IN%
SET /P PKG_CHECKS_PASS=<%TEMP%\$\%PKG_CHECKS_PASS_IN%
SET /P PKG_CHECK_FAIL=<%TEMP%\$\%PKG_CHECK_FAIL_IN%
SET /P PKG_CHECKS_FAIL=<%TEMP%\$\%PKG_CHECKS_FAIL_IN%

SETLOCAL ENABLEDELAYEDEXPANSION
IF !PKG_CHECK_PASS! GTR 0 (
  SET PKG_CHECKS_PASS=!PKG_CHECKS_PASS:~1!
)
IF !PKG_CHECK_FAIL! GTR 0 (
  SET PKG_CHECKS_FAIL=!PKG_CHECKS_FAIL:~1!
)
CALL :ELAPSED_CHECK_TIME %overall_check_start%
ECHO.
ECHO ----Build Checks Completed: PASS ^(!PKG_CHECK_PASS!^)[!PKG_CHECKS_PASS!], FAIL ^(!PKG_CHECK_FAIL!^)[!PKG_CHECKS_FAIL!], ELAPSED TIME !LP3D_ELAPSED_CHECK_TIME! ----
ECHO.
SETLOCAL DISABLEDELAYEDEXPANSION
ENDLOCAL
EXIT /b

:GET_PKG_CHECK_RESULT
SET PKG_CHECK_RESULT=undefined
SET PKG_CHECK_SUCCESS=Application terminated with return code 0.
SETLOCAL ENABLEDELAYEDEXPANSION
FOR /F "tokens=2*" %%i IN ('FINDSTR /c:"%PKG_CHECK_SUCCESS%" %PKG_LOG_FILE%') DO SET PKG_CHECK_RESULT=%%i %%j
IF "!PKG_CHECK_RESULT!" EQU "%PKG_CHECK_SUCCESS%" (
  CALL :ELAPSED_CHECK_TIME
  ECHO -%1 PASSED, ELAPSED TIME !LP3D_ELAPSED_CHECK_TIME!
  SET /P PKG_CHECK_PASS=<%TEMP%\$\%PKG_CHECK_PASS_IN%
  SET /A PKG_CHECK_PASS=!PKG_CHECK_PASS!+1
>%PKG_UPDATE_CHECK_PASS% !PKG_CHECK_PASS!
  FOR /f "tokens=* delims=" %%i IN (%TEMP%\$\%PKG_CHECKS_PASS_IN%) DO SET "PKG_CHECKS_PASS=!PKG_CHECKS_PASS!%%i"
  SET "PKG_CHECKS_PASS=!PKG_CHECKS_PASS!,%2"
>%PKG_UPDATE_CHECKS_PASS% !PKG_CHECKS_PASS!
) ELSE (
  CALL :ELAPSED_CHECK_TIME
  ECHO -%1 FAILED, ELAPSED TIME !LP3D_ELAPSED_CHECK_TIME!
  SET /P PKG_CHECK_FAIL=<%TEMP%\$\%PKG_CHECK_FAIL_IN%
  SET /A PKG_CHECK_FAIL=!PKG_CHECK_FAIL!+1
>%PKG_UPDATE_CHECK_FAIL% !PKG_CHECK_FAIL!
  FOR /f "tokens=* delims=" %%i IN (%TEMP%\$\%PKG_CHECKS_FAIL_IN%) DO SET "PKG_CHECKS_FAIL=!PKG_CHECKS_FAIL!%%i"
  SET "PKG_CHECKS_FAIL=!PKG_CHECKS_FAIL!,%2"
>%PKG_UPDATE_CHECKS_FAIL% !PKG_CHECKS_FAIL!
  IF EXIST "%PKG_TARGET%" (
    ECHO  -Copying CHECK %PKG_CHECK% %PKG_TARGET% to run assets....
    COPY /V /Y "%PKG_TARGET%" "%PKG_CHECK_PATH%" /A | findstr /i /v /r /c:"copied\>"
  ) ELSE (
    ECHO  -WARNING - %PKG_TARGET% was not found.
  )
  IF EXIST "%PKG_TARGET_PDB%" (
    ECHO  -Copying CHECK %PKG_CHECK% %PKG_TARGET_PDB% to run assets....
    COPY /V /Y "%PKG_TARGET_PDB%" "%PKG_CHECK_PATH%" /A | findstr /i /v /r /c:"copied\>"
  ) ELSE (
    ECHO  -WARNING - %PKG_TARGET_PDB% was not found.
  )
  IF EXIST "%PKG_DUMP_FILE%" (
    ECHO  -Copying CHECK %PKG_CHECK% %PKG_DUMP_FILE% to run assets....
    COPY /V /Y "%PKG_DUMP_FILE%" "%PKG_CHECK_PATH%" /A | findstr /i /v /r /c:"copied\>"
  ) ELSE (
    ECHO  -WARNING - %PKG_DUMP_FILE% was not found.
  )
  IF EXIST "%PKG_LOG_FILE%" (
    ECHO  -Copying CHECK %PKG_CHECK% %PKG_LOG_FILE% to run assets....
    COPY /V /Y "%PKG_LOG_FILE%" "%PKG_CHECK_PATH%" /A | findstr /i /v /r /c:"copied\>"
  ) ELSE (
    ECHO  -WARNING - %PKG_LOG_FILE% was not found.
  )
  CALL :ARCHIVE_CHECK_ASSETS %PKG_CHECK%
  TYPE "%PKG_LOG_FILE%"
)
SETLOCAL DISABLEDELAYEDEXPANSION
CALL :CLEANUP_CHECK_OUTPUT
EXIT /b

:SET_PKG_ZIP_UTILITY
SET PKG_ZIP_UTILITY=0
SET PKG_ZIP_UTILITY_64=C:\program files\7-zip
IF EXIST "%PKG_ZIP_UTILITY_64%" (
  SET "PATH=%PKG_ZIP_UTILITY_64%;%PATH%"
  SET PKG_ZIP_UTILITY=1
  ECHO.
  ECHO -Zip x86_64 executable found at "%PKG_ZIP_UTILITY_64%"
  EXIT /b
)
SET PKG_ZIP_UTILITY_32=C:\Program Files (x86)\7-zip
IF EXIST "%PKG_ZIP_UTILITY_32%" (
  SET "PATH=%PKG_ZIP_UTILITY_32%;%PATH%"
  SET PKG_ZIP_UTILITY=1
  ECHO.
  ECHO -Zip x86 executable found at "%PKG_ZIP_UTILITY_32%"
  EXIT /b
)
ECHO -WARNING - Could not find 7-zip. Cannot archive check assets.
EXIT /b

:ARCHIVE_CHECK_ASSETS
SET PKG_CHECK_ASSETS=Check_%1_%PKG_PLATFORM%_Assets.zip
IF %PKG_ZIP_UTILITY% == 1 (
  ECHO  -Archiving CHECK %1 assets to %PKG_RUNLOG_DIR%\%PKG_CHECK_ASSETS%
  IF NOT EXIST %PKG_RUNLOG_DIR% ( MKDIR %PKG_RUNLOG_DIR% >NUL 2>&1 )
  7z.exe a -tzip %PKG_RUNLOG_DIR%\%PKG_CHECK_ASSETS% %PKG_CHECK_PATH%\ | findstr /i /r /c:"^Creating\>" /c:"^Everything\>"
)
EXIT /b

:CLEANUP_CHECK_FOLDERS
ECHO.
IF [%SKIP_CLEANUP_CHECK_FOLDERS%] NEQ [] (
  ECHO Skipping checks cleanup.
  EXIT /b
)
ECHO   Build checks cleanup...
IF EXIST %PKG_RUNLOG% (
  ECHO.
  ECHO   Copying %PKG_DISTRO_DIR%_Run.log to log assets '%PKG_RUNLOG_DIR%\%PKG_DISTRO_DIR%_Run.log'...
  IF NOT EXIST %PKG_RUNLOG_DIR% ( MKDIR %PKG_RUNLOG_DIR% )
  COPY /V /Y "%PKG_RUNLOG%" "%PKG_RUNLOG_DIR%\%PKG_DISTRO_DIR%_Run.log" /A | findstr /i /v /r /c:"copied\>"
) ELSE (
  ECHO.
  ECHO -[WARNING] Could not find %PKG_RUNLOG%.
)
RMDIR /S /Q %PKG_TARGET_DIR%\cache
RMDIR /S /Q %PKG_TARGET_DIR%\logs
RMDIR /S /Q %PKG_TARGET_DIR%\config
RMDIR /S /Q %PKG_TARGET_DIR%\libraries
RMDIR /S /Q %PKG_TARGET_DIR%\VEXIQParts
RMDIR /S /Q %PKG_TARGET_DIR%\TENTEParts
EXIT /b

:CLEANUP_CHECK_OUTPUT
DEL /Q %PKG_CHECK_PATH%\*.out >NUL 2>&1
DEL /Q %PKG_CHECK_PATH%\*.pdf >NUL 2>&1
DEL /Q /S %PKG_CHECK_PATH%\std* >NUL 2>&1
DEL /Q /S %PKG_CHECK_PATH%\*.dmp >NUL 2>&1
RMDIR /Q /S %PKG_CHECK_PATH%\LPub3D >NUL 2>&1
RMDIR /Q /S %PKG_CHECK_PATH%\TENTE\LPub3D >NUL 2>&1
RMDIR /Q /S %PKG_CHECK_PATH%\VEXIQ\LPub3D >NUL 2>&1
EXIT /b

:ELAPSED_CHECK_TIME
IF [%1] EQU [] (SET start=%check_start%) ELSE (
  IF "%1"=="Start" (
    SET check_start=%time%
    EXIT /b
  ) ELSE (
    SET start=%1
  )
)
SET end=%time%
SET options="tokens=1-4 delims=:.,"
FOR /f %options% %%a IN ("%start%") DO SET start_h=%%a&SET /a start_m=100%%b %% 100&SET /a start_s=100%%c %% 100&SET /a start_ms=100%%d %% 100
FOR /f %options% %%a IN ("%end%") DO SET end_h=%%a&SET /a end_m=100%%b %% 100&SET /a end_s=100%%c %% 100&SET /a end_ms=100%%d %% 100

SET /a hours=%end_h%-%start_h%
SET /a mins=%end_m%-%start_m%
SET /a secs=%end_s%-%start_s%
SET /a ms=%end_ms%-%start_ms%
IF %ms% lss 0 SET /a secs = %secs% - 1 & SET /a ms = 100%ms%
IF %secs% lss 0 SET /a mins = %mins% - 1 & SET /a secs = 60%secs%
IF %mins% lss 0 SET /a hours = %hours% - 1 & SET /a mins = 60%mins%
IF %hours% lss 0 SET /a hours = 24%hours%
IF 1%ms% lss 100 SET ms=0%ms%
SET LP3D_ELAPSED_CHECK_TIME=%hours%:%mins%:%secs%.%ms%
EXIT /b

:SET_LDRAW_LIBS
ECHO.
ECHO - Copy LDraw archive libraries to extras folder...

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
