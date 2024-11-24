@ECHO OFF &SETLOCAL

Title LPub3D Windows build check script

rem  Trevor SANDY <trevor.sandy@gmail.com>
rem  Last Update: November 24, 2023
rem  Copyright (C) 2018 - 2024 by Trevor SANDY
rem --
rem This script is distributed in the hope that it will be useful,
rem but WITHOUT ANY WARRANTY; without even the implied warranty of
rem MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

rem Construct the staged files path

IF "%LP3D_CONDA_TEST%" NEQ "True" (
  SETLOCAL ENABLEDELAYEDEXPANSION
  SET PKG_DISTRO_DIR=%PACKAGE%_%PKG_PLATFORM%
  SET PKG_PRODUCT_DIR=%PACKAGE%-Any-%LP3D_APP_VERSION_LONG%
  SET PKG_CONFIG_DIR=%ABS_WD%\builds\windows\%CONFIGURATION%
  SET PKG_TARGET_DIR=!PKG_CONFIG_DIR!\!PKG_PRODUCT_DIR!\!PKG_DISTRO_DIR!
  SET PKG_RUNLOG_DIR=!PKG_CONFIG_DIR!\!PKG_PRODUCT_DIR!\%PACKAGE%_Logs
  SETLOCAL DISABLEDELAYEDEXPANSION
) ELSE (
  SET PKG_PLATFORM=x86_64
  SET PKG_DISTRO_DIR=%PACKAGE%_x86_64
  SET PKG_TARGET_DIR=%LIBRARY_PREFIX%\bin
  SET PKG_RUNLOG_DIR=%SRC_DIR%\%PACKAGE%_Logs
)
SET PKG_CHECK_DIR=%ABS_WD%\builds\check
SET PKG_CHECK_FILE=%PKG_CHECK_DIR%\build_checks.mpd
SET PKG_RUNLOG_FILE=%PKG_TARGET_DIR%\logs\%PACKAGE%Log.txt
SET PKG_POV_CONF_DIR=%PKG_TARGET_DIR%\3rdParty\lpub3d_trace_cui-3.8\config
SET PKG_TARGET_FILE=%PKG_TARGET_DIR%\%PACKAGE%.exe
SET PKG_DUMP_FILE=%PKG_TARGET_DIR%\%PACKAGE%.dmp
IF /I "%PKG_PLATFORM%"=="x86" (
  SET PKG_PDB_FILE=%ABS_WD%\mainApp\32bit_%CONFIGURATION%\%PACKAGE%.pdb
) ELSE (
  IF /I "%PKG_PLATFORM%"=="x86_64" (
    SET PKG_PDB_FILE=%ABS_WD%\mainApp\64bit_%CONFIGURATION%\%PACKAGE%.pdb
  )
)
IF NOT EXIST "%PKG_PDB_FILE%" (
  SET PKG_PDB_FILE=%PKG_TARGET_DIR%\%PACKAGE%.pdb
)
IF "%CONFIGURATION%"=="release" (
  SET PKG_CONFIG_FILE=%PKG_TARGET_DIR%\config\LPub3D Software\LPub3D.ini
) ELSE (
  SET PKG_CONFIG_FILE=%PKG_TARGET_DIR%\config\LPub3D Software Maint\LPub3Dd.ini
)
IF "%LP3D_CONDA_TEST%" NEQ "True" (
  IF NOT "%PKG_INSTALL%" EQU "1" SET PKG_RUNLOG_DIR=%LP3D_DOWNLOADS_PATH%
  IF "%BUILD_OPT%" EQU "verify" SET PKG_RUNLOG_DIR=%LP3D_DOWNLOADS_PATH%
)
IF "%LP3D_VALID_7ZIP%" =="" SET LP3D_VALID_7ZIP=0

SET PKG_CHECKS=7
rem Check 1 of 7
SET PKG_CHECK_OPTIONS=--no-console-redirect --process-file --liblego --preferred-renderer native
SET PKG_CHECK_NATIVE_COMMAND=%PKG_CHECK_OPTIONS% %PKG_CHECK_FILE%
rem Check 2 of 7
SET PKG_CHECK_OPTIONS=--no-console-redirect --process-file --clear-cache --liblego --preferred-renderer ldview
SET PKG_CHECK_LDVIEW_COMMAND=%PKG_CHECK_OPTIONS% %PKG_CHECK_FILE%
rem Check 3 of 7
SET PKG_CHECK_OPTIONS=--no-console-redirect --process-file --clear-cache --liblego --preferred-renderer ldview-sc
SET PKG_CHECK_LDVIEW_SINGLE_CALL_COMMAND=%PKG_CHECK_OPTIONS% %PKG_CHECK_FILE%
rem Check 4 of 7
SET PKG_CHECK_OPTIONS=--no-console-redirect --process-export --range 1-3 --clear-cache --liblego --preferred-renderer ldglite
SET PKG_CHECK_RANGE_COMMAND=%PKG_CHECK_OPTIONS% %PKG_CHECK_FILE%
rem Check 5 of 7
SET PKG_CHECK_OPTIONS=--no-console-redirect --process-file --clear-cache --liblego --preferred-renderer povray-ldv
IF "%GITHUB%" NEQ "True" (
  IF "%APPVEYOR%" NEQ "True" (
    SET PKG_CHECK_OPTIONS=--no-console-redirect --process-file --clear-cache --liblego --preferred-renderer povray
  )
)
SET PKG_CHECK_POV_COMMAND=%PKG_CHECK_OPTIONS% %PKG_CHECK_FILE%
rem Check 6 of 7
SET PKG_CHECK_FILE=%ABS_WD%\builds\check\TENTE\astromovil.ldr
SET PKG_CHECK_OPTIONS=--no-console-redirect --process-file --clear-cache --libtente --preferred-renderer ldview
SET PKG_CHECK_TENTE_COMMAND=%PKG_CHECK_OPTIONS% %PKG_CHECK_FILE%
rem Check 7 of 7
SET PKG_CHECK_FILE=%ABS_WD%\builds\check\VEXIQ\spider.mpd
SET PKG_CHECK_OPTIONS=--no-console-redirect --process-file --clear-cache --libvexiq --preferred-renderer ldview-scsl
SET PKG_CHECK_VEXIQ_COMMAND=%PKG_CHECK_OPTIONS% %PKG_CHECK_FILE%

rem Setup logging and check status
IF NOT EXIST "%TEMP%\$" (
  MD "%TEMP%\$"
)
SET PKG_LOG_FILE=%TEMP%\$\CheckLog.out
SET PKG_CHECK_OUT=Check.out
SET PKG_CHECK_RESULT=%TEMP%\$\%PKG_CHECK_OUT% ECHO
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
ECHO   CONFIGURATION.............[%CONFIGURATION%]
IF "%LP3D_CONDA_TEST%" NEQ "True" (
  ECHO   BUILD_OPT.................[%BUILD_OPT%]
)
IF "%LP3D_CONDA_TEST%" EQU "True" (
  ECHO   LP3D_CONDA_TEST...........[YES]
)
IF NOT [%ARCHIVE_ASSETS%]==[] (
  ECHO   ARCHIVE_CHECK_ASSETS......[YES]
)
ECHO   PKG_PLATFORM..............[%PKG_PLATFORM%]
IF "%LP3D_CONDA_TEST%" NEQ "True" (
  ECHO   PKG_PRODUCT_DIR...........[%PKG_PRODUCT_DIR%]
)
ECHO   PKG_DISTRO_DIR............[%PKG_DISTRO_DIR%]
ECHO   PKG_TARGET_DIR............[%PKG_TARGET_DIR%]
ECHO   PKG_TARGET_FILE...........[%PKG_TARGET_FILE%]
ECHO   PKG_PDB_FILE..............[%PKG_PDB_FILE%]
ECHO   PKG_DUMP_FILE.............[%PKG_DUMP_FILE%]
ECHO   PKG_RUNLOG_DIR............[%PKG_RUNLOG_DIR%]
ECHO   PKG_ASSETS_FOLDER.........[%PKG_CHECK_DIR%]
IF "%LP3D_CONDA_TEST%" NEQ "True" (
  ECHO   LDRAW_LIB_STORE...........[%LDRAW_LIBS%]
)

CALL :SET_LDRAW_LIBS
CALL :SET_VALID_7ZIP

SET overall_check_start=%time%

SET LPUB3D_AUTO_RESTART_ENABLED=0
SET LPUB3D_DISABLE_UPDATE_CHECK=1

ECHO.
ECHO ------------Build Checks Start--------------
ECHO.
IF NOT EXIST "%PKG_TARGET_FILE%" (
  ECHO -ERROR - %PKG_TARGET_FILE% does not exist. The build check cannot continue.
  EXIT /b
)
ECHO -%PKG_TARGET_FILE% found.

SET PKG_CHECK=1
SET /A PKG_TIMEOUT=30
SET PKG_DESCRIPTION=PKG_CHECK_NATIVE
ECHO.
ECHO -CHECK %PKG_CHECK% OF %PKG_CHECKS%. PKG_CHECK_NATIVE_COMMAND...[%PKG_CHECK_NATIVE_COMMAND%]
SET QT_DEBUG_PLUGINS=1
CALL :PKG_ELAPSED_TIME Start
CALL :PKG_RUN_COMMAND %PKG_TARGET_FILE% %PKG_CHECK_NATIVE_COMMAND%

SET PKG_CHECK=2
SET /A PKG_TIMEOUT=60
SET PKG_DESCRIPTION=PKG_CHECK_LDVIEW
ECHO.
ECHO -CHECK %PKG_CHECK% OF %PKG_CHECKS%. PKG_CHECK_LDVIEW_COMMAND...[%PKG_CHECK_LDVIEW_COMMAND%]
CALL :PKG_ELAPSED_TIME Start
CALL :PKG_RUN_COMMAND %PKG_TARGET_FILE% %PKG_CHECK_LDVIEW_COMMAND%

SET PKG_CHECK=3
SET /A PKG_TIMEOUT=50
SET PKG_DESCRIPTION=PKG_CHECK_LDVIEW_SINGLE_CALL
ECHO.
ECHO -CHECK %PKG_CHECK% OF %PKG_CHECKS%. PKG_CHECK_LDVIEW_SINGLE_CALL_COMMAND...[%PKG_CHECK_LDVIEW_COMMAND%]
CALL :PKG_ELAPSED_TIME Start
CALL :PKG_RUN_COMMAND %PKG_TARGET_FILE% %PKG_CHECK_LDVIEW_SINGLE_CALL_COMMAND%

SET PKG_CHECK=4
SET /A PKG_TIMEOUT=65
SET PKG_DESCRIPTION=PKG_CHECK_RANGE
ECHO.
ECHO -CHECK %PKG_CHECK% OF %PKG_CHECKS%. PKG_CHECK_RANGE_COMMAND....[%PKG_CHECK_RANGE_COMMAND%]
CALL :PKG_ELAPSED_TIME Start
CALL :PKG_RUN_COMMAND %PKG_TARGET_FILE% %PKG_CHECK_RANGE_COMMAND%

SET PKG_CHECK=5
SET /A PKG_TIMEOUT=180
SET PKG_DESCRIPTION=PKG_CHECK_POV
ECHO.
ECHO -CHECK %PKG_CHECK% OF %PKG_CHECKS%. PKG_CHECK_POV_COMMAND......[%PKG_CHECK_POV_COMMAND%]
CALL :PKG_ELAPSED_TIME Start
CALL :PKG_RUN_COMMAND %PKG_TARGET_FILE% %PKG_CHECK_POV_COMMAND%

SET PKG_CHECK=6
SET /A PKG_TIMEOUT=115
SET PKG_DESCRIPTION=PKG_CHECK_TENTE
ECHO.
ECHO -CHECK %PKG_CHECK% OF %PKG_CHECKS%. PKG_CHECK_TENTE_COMMAND......[%PKG_CHECK_TENTE_COMMAND%]
CALL :PKG_ELAPSED_TIME Start
CALL :PKG_RUN_COMMAND %PKG_TARGET_FILE% %PKG_CHECK_TENTE_COMMAND%

SET PKG_CHECK=7
SET /A PKG_TIMEOUT=400
SET PKG_DESCRIPTION=PKG_CHECK_VEXIQ
ECHO.
ECHO -CHECK %PKG_CHECK% OF %PKG_CHECKS%. PKG_CHECK_VEXIQ_COMMAND......[%PKG_CHECK_VEXIQ_COMMAND%]
CALL :PKG_ELAPSED_TIME Start
CALL :PKG_RUN_COMMAND %PKG_TARGET_FILE% %PKG_CHECK_VEXIQ_COMMAND%

CALL :CLEANUP_CHECK_FOLDERS

SET /P PKG_CHECK_PASS=<%TEMP%\$\%PKG_CHECK_PASS_IN%
SET /P PKG_CHECKS_PASS=<%TEMP%\$\%PKG_CHECKS_PASS_IN%
SET /P PKG_CHECK_FAIL=<%TEMP%\$\%PKG_CHECK_FAIL_IN%
SET /P PKG_CHECKS_FAIL=<%TEMP%\$\%PKG_CHECKS_FAIL_IN%

SETLOCAL ENABLEDELAYEDEXPANSION
IF !PKG_CHECK_PASS! GTR 0 (
  SET "PKG_CHECKS_PASS=!PKG_CHECKS_PASS:~1!"
)
IF !PKG_CHECK_FAIL! GTR 0 (
  SET "PKG_CHECKS_FAIL=!PKG_CHECKS_FAIL:~1!"
)
CALL :PKG_ELAPSED_TIME %overall_check_start%
ECHO.
IF !PKG_CHECK_PASS! GTR 0 (
  IF !PKG_CHECK_FAIL! GTR 0 (
    SET "PKG_CHECK_STATUS=PASS ^(!PKG_CHECK_PASS!^)[!PKG_CHECKS_PASS!], FAIL ^(!PKG_CHECK_FAIL!^)[!PKG_CHECKS_FAIL!]"
  ) ELSE (
    SET "PKG_CHECK_STATUS=PASS ^(!PKG_CHECK_PASS!^)[!PKG_CHECKS_PASS!]"
  )
) ELSE (
  IF !PKG_CHECK_FAIL! GTR 0 (
    SET "PKG_CHECK_STATUS=FAIL ^(!PKG_CHECK_FAIL!^)[!PKG_CHECKS_FAIL!]"
  )
)
IF [!PKG_CHECK_STATUS!] NEQ [] (
  SET "PKG_CHECK_STATUS=Build Checks Completed: !PKG_CHECK_STATUS!"
) ELSE (
  SET "PKG_CHECK_STATUS=Build Checks Completed."
)
SET "PKG_CHECK_SUMMARY=!PKG_CHECK_STATUS!, ELAPSED TIME !LP3D_PKG_ELAPSED_TIME!"
>%PKG_CHECK_RESULT% !PKG_CHECK_SUMMARY!
ECHO ---- !PKG_CHECK_SUMMARY! ----
ECHO.
SETLOCAL DISABLEDELAYEDEXPANSION
EXIT /b

:PKG_RUN_COMMAND
SET PKG_COMMAND=%*
IF NOT EXIST "%TEMP%\$" MD "%TEMP%\$"
SET bcc=Command.bat
SET t=%TEMP%\$\%bcc% ECHO
IF EXIST "%TEMP%\$\%bcc%" DEL /Q "%TEMP%\$\%bcc%"
IF EXIST "%PKG_LOG_FILE%" DEL /Q "%PKG_LOG_FILE%"

:PKG_START_COMMAND_AND_TIMER
>%t% @ECHO OFF
>>%t% CALL %PKG_COMMAND% ^> %PKG_LOG_FILE% 2^>^&1
IF NOT EXIST "%TEMP%\$\%bcc%" (
  ECHO.
  ECHO  -ERROR - Command script %TEMP%\$\%bcc% was not found
  EXIT /b
)
START /b "%PKG_DESCRIPTION%" CMD /c %TEMP%\$\%bcc%
WAITFOR TwoSecondDelay /T 2 >NUL 2>&1
CALL :PKG_CHECK_TIMER
SETLOCAL ENABLEDELAYEDEXPANSION
FOR %%R IN (%PKG_LOG_FILE%) DO (
  IF %%~zR LSS 1 SET EMPTY_LOG=True
  IF [!EMPTY_LOG!] NEQ [] ECHO  -ERROR - %PKG_DESCRIPTION% output log is empty
  CALL :GET_PKG_CHECK_RESULT !EMPTY_LOG!
  ECHO.
)
SETLOCAL DISABLEDELAYEDEXPANSION
EXIT /b

:PKG_CHECK_TIMER
SET /A PKG_TIME=0
SET /A PKG_TIME_INTERVAL=5
SET PKG_EXE=%PACKAGE%.exe
SETLOCAL ENABLEEXTENSIONS

:PKG_CHECK_TIMER_LOOP
SET /A "PKG_TIME=PKG_TIME+%PKG_TIME_INTERVAL%">NUL
WAITFOR RunningBuildCheck /T %PKG_TIME_INTERVAL% >NUL 2>&1
IF %PKG_TIME% GEQ %PKG_TIMEOUT% (
  SETLOCAL ENABLEDELAYEDEXPANSION
  CALL :PKG_FORMAT_TIME %PKG_TIME%
  ECHO  -WARNING  - Check %PKG_CHECK% %PKG_DESCRIPTION% timed out after elapsed time !LP3D_PKG_ELAPSED_TIME! - timeout seconds %PKG_TIME%
  SET PKG_TK=%TEMP%\$\TaskKill.out
  TASKKILL /FI "IMAGENAME EQ %PKG_EXE%" /T /F > !PKG_TK! 2>&1
  <!PKG_TK! ( SET /p PKG_TKO= )
  ECHO  -TASKKILL - Check %PKG_CHECK% !PKG_TKO! && IF EXIST "!PKG_TKO!" DEL /Q "!PKG_TKO!"
  SETLOCAL DISABLEDELAYEDEXPANSION
  EXIT /b
)
FOR /F %%x IN ('TASKLIST /NH /FI "IMAGENAME EQ %PKG_EXE%"') DO IF %%x == %PKG_EXE% (
  GOTO :PKG_CHECK_TIMER_LOOP
)
EXIT /b

:GET_PKG_CHECK_RESULT
SET "PKG_LOG_EMPTY=%1"
SET "PKG_CHECK_RESULT=undefined"
SET "PKG_CHECK_SUCCESS=Application terminated with return code 0."
IF "%PKG_LOG_EMPTY%" EQU "True" GOTO :PKG_CHECK_STATUS
FOR /F "tokens=3*" %%i IN ('findstr /c:"%PKG_CHECK_SUCCESS%" %PKG_LOG_FILE%') DO SET "PKG_CHECK_RESULT=%%i %%j"

:PKG_CHECK_STATUS
SETLOCAL ENABLEDELAYEDEXPANSION
IF "%PKG_CHECK_RESULT%" EQU "%PKG_CHECK_SUCCESS%" (
  CALL :PKG_ELAPSED_TIME
  ECHO -%PKG_DESCRIPTION% PASSED, ELAPSED TIME !LP3D_PKG_ELAPSED_TIME!
  SET /P PKG_CHECK_PASS=<%TEMP%\$\%PKG_CHECK_PASS_IN%
  SET /A PKG_CHECK_PASS=!PKG_CHECK_PASS!+1
>%PKG_UPDATE_CHECK_PASS% !PKG_CHECK_PASS!
  FOR /f "tokens=* delims=" %%i IN (%TEMP%\$\%PKG_CHECKS_PASS_IN%) DO SET "PKG_CHECKS_PASS=!PKG_CHECKS_PASS!%%i"
  SET "PKG_CHECKS_PASS=!PKG_CHECKS_PASS!,%PKG_CHECK%"
>%PKG_UPDATE_CHECKS_PASS% !PKG_CHECKS_PASS!
  IF NOT [%ARCHIVE_ASSETS%]==[] (
    CALL :ARCHIVE_CHECK_ASSETS %PKG_CHECK%
  )
) ELSE (
  CALL :PKG_ELAPSED_TIME
  ECHO -%PKG_DESCRIPTION% FAILED, ELAPSED TIME !LP3D_PKG_ELAPSED_TIME!
  SET /P PKG_CHECK_FAIL=<%TEMP%\$\%PKG_CHECK_FAIL_IN%
  SET /A PKG_CHECK_FAIL=!PKG_CHECK_FAIL!+1
>%PKG_UPDATE_CHECK_FAIL% !PKG_CHECK_FAIL!
  FOR /f "tokens=* delims=" %%i IN (%TEMP%\$\%PKG_CHECKS_FAIL_IN%) DO SET "PKG_CHECKS_FAIL=!PKG_CHECKS_FAIL!%%i"
  SET "PKG_CHECKS_FAIL=!PKG_CHECKS_FAIL!,%PKG_CHECK%"
>%PKG_UPDATE_CHECKS_FAIL% !PKG_CHECKS_FAIL!
  IF EXIST "%PKG_TARGET_FILE%" (
    ECHO  -Copying CHECK %PKG_CHECK% %PKG_TARGET_FILE% to run asset...
    COPY /V /Y "%PKG_TARGET_FILE%" "%PKG_CHECK_DIR%" /A | findstr /i /v /r /c:"copied\>"
  ) ELSE (
    ECHO  -WARNING - %PKG_TARGET_FILE% was not found.
  )
  IF EXIST "%PKG_PDB_FILE%" (
    ECHO  -Copying CHECK %PKG_CHECK% %PKG_PDB_FILE% to run asset...
    COPY /V /Y "%PKG_PDB_FILE%" "%PKG_CHECK_DIR%" /A | findstr /i /v /r /c:"copied\>"
  ) ELSE (
    ECHO  -WARNING - %PKG_PDB_FILE% was not found.
  )
  IF EXIST "%PKG_DUMP_FILE%" (
    ECHO  -Copying CHECK %PKG_CHECK% %PKG_DUMP_FILE% to run asset...
    COPY /V /Y "%PKG_DUMP_FILE%" "%PKG_CHECK_DIR%" /A | findstr /i /v /r /c:"copied\>"
  ) ELSE (
    IF EXIST "%LOCALAPPDATA%\Temp\%PACKAGE%.dmp" (
	  ECHO  -Copying CHECK %PKG_CHECK% %LOCALAPPDATA%\Temp\%PACKAGE%.dmp to run asset...
	  COPY /V /Y "%LOCALAPPDATA%\Temp\%PACKAGE%.dmp" "%PKG_CHECK_DIR%" /A | findstr /i /v /r /c:"copied\>"
	) ELSE (
      ECHO  -WARNING - %PKG_DUMP_FILE% was not found.
	)
  )
  IF EXIST "%PKG_LOG_FILE%" (
    ECHO  -Copying CHECK %PKG_CHECK% %PKG_LOG_FILE% to run asset...
    COPY /V /Y "%PKG_LOG_FILE%" "%PKG_CHECK_DIR%" /A | findstr /i /v /r /c:"copied\>"
  ) ELSE (
    ECHO  -WARNING - %PKG_LOG_FILE% was not found.
  )
  IF %PKG_CHECK% EQU 5 (
    IF EXIST "%PKG_POV_CONF_DIR%\povray.conf" (
      ECHO  -Copying CHECK %PKG_CHECK% %PKG_POV_CONF_DIR%\povray.conf to run asset...
      COPY /V /Y "%PKG_POV_CONF_DIR%\povray.conf" "%PKG_CHECK_DIR%" /A | findstr /i /v /r /c:"copied\>"
    ) ELSE (
      ECHO  -WARNING - %PKG_POV_CONF_DIR%\povray.conf was not found.
    )
    IF EXIST "%PKG_POV_CONF_DIR%\povray.ini" (
      ECHO  -Copying CHECK %PKG_CHECK% %PKG_POV_CONF_DIR%\povray.ini to run asset...
      COPY /V /Y "%PKG_POV_CONF_DIR%\povray.ini" "%PKG_CHECK_DIR%" /A | findstr /i /v /r /c:"copied\>"
    ) ELSE (
      ECHO  -WARNING - %PKG_POV_CONF_DIR%\povray.ini was not found.
    )
  )
  CALL :ARCHIVE_CHECK_ASSETS %PKG_CHECK%
  IF EXIST "%PKG_LOG_FILE%" TYPE "%PKG_LOG_FILE%"
)
SETLOCAL DISABLEDELAYEDEXPANSION
CALL :CLEANUP_CHECK_OUTPUT
EXIT /b

:SET_VALID_7ZIP
IF "%LP3D_7ZIP_WIN64%" == "" (
  SET "LP3D_7ZIP_WIN64=%ProgramFiles%\7-zip\7z.exe"
)
IF %LP3D_VALID_7ZIP% == 0 (
  "%LP3D_7ZIP_WIN64%" > %TEMP%\output.tmp 2>&1
  FOR /f "usebackq eol= delims=" %%a IN (%TEMP%\output.tmp) DO (
    ECHO.%%a | findstr /C:"7-Zip">NUL && (
      SET LP3D_VALID_7ZIP=1
      ECHO.
      ECHO -7zip x64 executable found at %LP3D_7ZIP_WIN64%
      GOTO :END_7ZIP_LOOP
    ) || (
      GOTO :END_7ZIP64_LOOP
    )
  )
)
:END_7ZIP64_LOOP
IF "%LP3D_7ZIP_WIN32%" == "" (
  SET "LP3D_7ZIP_WIN32=C:\Program Files ^(x86^)\7-zip\7z.exe"
)
IF %LP3D_VALID_7ZIP% == 0 (
  "%LP3D_7ZIP_WIN32%" > %TEMP%\output.tmp 2>&1
  FOR /f "usebackq eol= delims=" %%a IN (%TEMP%\output.tmp) DO (
    ECHO.%%a | findstr /C:"7-Zip">NUL && (
      SET LP3D_VALID_7ZIP=1
      ECHO.
      ECHO -7zip x86 executable found at %LP3D_7ZIP_WIN32%
    ) || (
      ECHO -WARNING - 7zip not found at %LP3D_7ZIP_WIN32% or %LP3D_7ZIP_WIN64%. Cannot archive check assets.
    )
    GOTO :END_7ZIP_LOOP
  )
)
:END_7ZIP_LOOP
EXIT /b

:ARCHIVE_CHECK_ASSETS
SET PKG_CHECK_ASSETS=Check_%1_%PKG_PLATFORM%_Assets.zip
IF "%LP3D_7ZIP_WIN64%" NEQ "" (
  SET "LP3D_7ZIP_WIN=%LP3D_7ZIP_WIN64%"
) ELSE (
  IF "%LP3D_7ZIP_WIN32%" == "" (
    SET "LP3D_7ZIP_WIN=%LP3D_7ZIP_WIN32%"
  )
)
IF "%LP3D_7ZIP_WIN%" NEQ "" (
  ECHO  -Archiving CHECK %1 assets to %PKG_RUNLOG_DIR%\%PKG_CHECK_ASSETS%
  IF NOT EXIST %PKG_RUNLOG_DIR% ( MKDIR %PKG_RUNLOG_DIR% >NUL 2>&1 )
  "%LP3D_7ZIP_WIN%" a -tzip %PKG_RUNLOG_DIR%\%PKG_CHECK_ASSETS% %PKG_CHECK_DIR%\ >NUL 2>&1
  IF EXIST "%PKG_RUNLOG_DIR%\%PKG_CHECK_ASSETS%" ( ECHO  -Archive succeeded ) ELSE ( ECHO  -ERROR - Archive failed )
)
EXIT /b

:CLEANUP_CHECK_FOLDERS
ECHO.
IF [%SKIP_CLEANUP_PKG_CHECK_FOLDERS%] NEQ [] (
  ECHO -Skipped checks cleanup.
  EXIT /b
)
ECHO   Build checks cleanup...
IF EXIST "%PKG_RUNLOG_FILE%" (
  ECHO.
  ECHO   Copying %PKG_DISTRO_DIR%_Run.log to log assets '%PKG_RUNLOG_DIR%\%PKG_DISTRO_DIR%_Run.log'...
  IF NOT EXIST %PKG_RUNLOG_DIR% ( MKDIR %PKG_RUNLOG_DIR% )
  COPY /V /Y "%PKG_RUNLOG_FILE%" "%PKG_RUNLOG_DIR%\%PKG_DISTRO_DIR%_Run.log" /A | findstr /i /v /r /c:"copied\>"
) ELSE (
  ECHO.
  ECHO -WARNING - Could not find %PKG_RUNLOG_FILE%.
)
IF EXIST "%PKG_CONFIG_FILE%" (
  ECHO.
  ECHO   Copying %PKG_DISTRO_DIR%_Config.ini to log assets '%PKG_RUNLOG_DIR%\%PKG_DISTRO_DIR%_Config.ini'...
  IF NOT EXIST %PKG_RUNLOG_DIR% ( MKDIR %PKG_RUNLOG_DIR% )
  COPY /V /Y "%PKG_CONFIG_FILE%" "%PKG_RUNLOG_DIR%\%PKG_DISTRO_DIR%_Config.ini" /A | findstr /i /v /r /c:"copied\>"
) ELSE (
  ECHO.
  ECHO -WARNING - Could not find %PKG_CONFIG_FILE%.
)
RMDIR /S /Q %PKG_TARGET_DIR%\cache >NUL 2>&1
RMDIR /S /Q %PKG_TARGET_DIR%\logs >NUL 2>&1
RMDIR /S /Q %PKG_TARGET_DIR%\config >NUL 2>&1
RMDIR /S /Q %PKG_TARGET_DIR%\libraries >NUL 2>&1
RMDIR /S /Q %PKG_TARGET_DIR%\VEXIQParts >NUL 2>&1
RMDIR /S /Q %PKG_TARGET_DIR%\TENTEParts >NUL 2>&1
EXIT /b

:CLEANUP_CHECK_OUTPUT
DEL /Q %PKG_CHECK_DIR%\*.out >NUL 2>&1
DEL /Q %PKG_CHECK_DIR%\*.pdf >NUL 2>&1
DEL /Q /S %PKG_CHECK_DIR%\std* >NUL 2>&1
DEL /Q /S %PKG_CHECK_DIR%\*.dmp >NUL 2>&1
DEL /Q /S %PKG_CHECK_DIR%\*.exe >NUL 2>&1
DEL /Q /S %PKG_CHECK_DIR%\*.pdb >NUL 2>&1
DEL /Q /S %PKG_CHECK_DIR%\*.ini >NUL 2>&1
DEL /Q /S %PKG_CHECK_DIR%\*.conf >NUL 2>&1
RMDIR /Q /S %PKG_CHECK_DIR%\LPub3D >NUL 2>&1
RMDIR /Q /S %PKG_CHECK_DIR%\TENTE\LPub3D >NUL 2>&1
RMDIR /Q /S %PKG_CHECK_DIR%\VEXIQ\LPub3D >NUL 2>&1
EXIT /b

:PKG_ELAPSED_TIME
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
SET LP3D_PKG_ELAPSED_TIME=%hours%:%mins%:%secs%.%ms%
EXIT /b

:PKG_FORMAT_TIME
rem Format the elapsed time provided in seconds
SET seconds=%1
SET /a days = %seconds% / 86400
SET /a hours = (%seconds% / 3600) - (%days% * 24)
SET /a mins = (%seconds% / 60) - (%days% * 1440) - (%hours% * 60)
SET /a secs = %seconds% %% 60
SET LP3D_PKG_ELAPSED_TIME=%hours%:%mins%:%secs%
EXIT /b

:SET_LDRAW_LIBS
IF "%LP3D_CONDA_TEST%" EQU "True" EXIT /b
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
