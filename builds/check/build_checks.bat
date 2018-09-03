@ECHO OFF

Title LPub3D Windows build check script

rem  Trevor SANDY <trevor.sandy@gmail.com>
rem  Last Update: June 28, 2018
rem  Copyright (c) 2018 by Trevor SANDY
rem --
rem This script is distributed in the hope that it will be useful,
rem but WITHOUT ANY WARRANTY; without even the implied warranty of
rem MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

rem Construct the staged files path
SET PKG_DISTRO_DIR=%PACKAGE%_%PKG_PLATFORM%
SET PKG_PRODUCT_DIR=%PACKAGE%-Any-%LP3D_APP_VERSION_LONG%
SET PKG_TARGET_DIR=builds\windows\%CONFIGURATION%\%PKG_PRODUCT_DIR%\%PKG_DISTRO_DIR%
SET PKG_CHECK_FILE=%ABS_WD%\builds\check\build_checks.mpd
SET PKG_TARGET=%PKG_TARGET_DIR%\%PACKAGE%.exe

rem Checks
SET PKG_CHECK_OPTIONS=--ignore-console-redirect --process-file --preferred-renderer native
SET PKG_CHECK_NATIVE_COMMAND=%PKG_TARGET% %PKG_CHECK_OPTIONS% %PKG_CHECK_FILE%

SET PKG_CHECK_OPTIONS=--ignore-console-redirect --process-file --clear-cache --preferred-renderer ldview
SET PKG_CHECK_LDVIEW_COMMAND=%PKG_TARGET% %PKG_CHECK_OPTIONS% %PKG_CHECK_FILE%

SET PKG_CHECK_OPTIONS=--ignore-console-redirect --process-export --range 1-3 --clear-cache --preferred-renderer ldglite
SET PKG_CHECK_RANGE_COMMAND=%PKG_TARGET% %PKG_CHECK_OPTIONS% %PKG_CHECK_FILE%

SET PKG_CHECK_OPTIONS=--ignore-console-redirect --process-file --clear-cache --preferred-renderer povray
SET PKG_CHECK_POV_COMMAND=%PKG_TARGET% %PKG_CHECK_OPTIONS% %PKG_CHECK_FILE%

CALL :CHECK_LDRAW_DIR
CALL :SET_LDRAW_LIBS

ECHO.
ECHO   PKG_CHECK_OPTIONS......[%PKG_CHECK_OPTIONS%]
ECHO   PKG_DISTRO_DIR.........[%PKG_DISTRO_DIR%]
ECHO   PKG_PRODUCT_DIR........[%PKG_PRODUCT_DIR%]
ECHO   PKG_TARGET_DIR.........[%PKG_TARGET_DIR%]
ECHO   PKG_TARGET.............[%PKG_TARGET%]
ECHO.
ECHO -Check for executable file...
ECHO.
IF NOT EXIST "%PKG_TARGET%" (
  ECHO -ERROR - %PKG_TARGET% does not exist, build check will exit.
  EXIT /b
) ELSE (
  ECHO -%PKG_TARGET% found.
  IF EXIST "Check.out" DEL /Q "Check.out"
  ECHO.
  ECHO   PKG_CHECK_NATIVE_COMMAND...[%PKG_CHECK_NATIVE_COMMAND%]
  CALL %PKG_CHECK_NATIVE_COMMAND% > Check.out 2>&1
  FOR %%R IN (Check.out) DO (
    IF NOT %%~zR LSS 1 (
      ECHO -BUILD_CHECK_NATIVE Output...
      TYPE "Check.out"
      ECHO.
      DEL /Q "Check.out"
      ECHO.
    ) ELSE (
      ECHO. -ERROR - BUILD_CHECK_NATIVE failed.
    )
  )
  ECHO.
  ECHO   PKG_CHECK_LDVIEW_COMMAND...[%PKG_CHECK_LDVIEW_COMMAND%]
  CALL %PKG_CHECK_LDVIEW_COMMAND% > Check.out 2>&1
  FOR %%R IN (Check.out) DO (
    IF NOT %%~zR LSS 1 (
      ECHO -BUILD_CHECK_LDVIEW Output...
      TYPE "Check.out"
      ECHO.
      DEL /Q "Check.out"
      ECHO.
    ) ELSE (
      ECHO. -ERROR - BUILD_CHECK_LDVIEW failed.
    )
  )
  ECHO.
  ECHO   PKG_CHECK_RANGE_COMMAND....[%PKG_CHECK_RANGE_COMMAND%]
  CALL %PKG_CHECK_RANGE_COMMAND% > Check.out 2>&1
  FOR %%R IN (Check.out) DO (
    IF NOT %%~zR LSS 1 (
      ECHO -BUILD_CHECK_RANGE Output...
      TYPE "Check.out"
      ECHO.
      DEL /Q "Check.out"
      ECHO.
    ) ELSE (
      ECHO. -ERROR - BUILD_CHECK_RANGE failed.
    )
  )
  ECHO.
  ECHO   PKG_CHECK_POV_COMMAND......[%PKG_CHECK_POV_COMMAND%]
  CALL %PKG_CHECK_POV_COMMAND% > Check.out 2>&1
  FOR %%R IN (Check.out) DO (
    IF NOT %%~zR LSS 1 (
      ECHO -BUILD_CHECK_POV Output...
      TYPE "Check.out"
      ECHO.
      DEL /Q "Check.out"
      ECHO.
    ) ELSE (
      ECHO. -ERROR - BUILD_CHECK_POV failed.
    )
  )
)

:CHECK_LDRAW_DIR
ECHO.
ECHO -Check for LDraw library...
IF NOT EXIST "%LDRAW_DIR%\parts" (
  ECHO.
  ECHO -LDraw directory %LDRAW_DIR% does not exist - creating...
  REM SET CHECK=0
  IF NOT EXIST "%LDRAW_LIBS%\%OfficialCONTENT%" (
    ECHO.
    ECHO -LDraw archive library %LDRAW_LIBS%\%OfficialCONTENT% does not exist - Downloading...

    CALL :DOWNLOAD_LDRAW_LIBS

  ) ELSE (
    COPY /V /Y "%LDRAW_LIBS%\%OfficialCONTENT%" "%LDRAW_DOWNLOAD_DIR%\" /A | findstr /i /v /r /c:"copied\>"
  )
  IF EXIST "%LDRAW_DOWNLOAD_DIR%\%OfficialCONTENT%" (
    IF EXIST "%zipWin64%" (
      ECHO.
      ECHO -7zip exectutable found at "%zipWin64%"
      ECHO.
      ECHO -Extracting %OfficialCONTENT%...
      ECHO.
      "%zipWin64%\7z.exe" x -o"%LDRAW_DOWNLOAD_DIR%\" "%LDRAW_DOWNLOAD_DIR%\%OfficialCONTENT%" | findstr /i /r /c:"^Extracting\>" /c:"^Everything\>"
      IF EXIST "%LDRAW_DIR%\parts" (
        ECHO.
        ECHO -LDraw directory %LDRAW_DIR% extracted.
        ECHO.
        ECHO -Cleanup %OfficialCONTENT%...
        DEL /Q "%LDRAW_DOWNLOAD_DIR%\%OfficialCONTENT%"
        ECHO.
        ECHO -Set LDRAWDIR to %LDRAW_DIR%.
        SET LDRAWDIR=%LDRAW_DIR%
      )
    ) ELSE (
      ECHO [WARNING] Could not find 7zip executable.
      SET CHECK=0
    )
  ) ELSE (
    ECHO.
    ECHO -[WARNING] Could not find %LDRAW_DOWNLOAD_DIR%\%OfficialCONTENT%.
    SET CHECK=0
  )
) ELSE (
  ECHO.
  ECHO -LDraw directory exist at [%LDRAW_DIR%].
  ECHO.
  ECHO -Set LDRAWDIR to %LDRAW_DIR%.
  SET LDRAWDIR=%LDRAW_DIR%
)
EXIT /b

:SET_LDRAW_LIBS
IF NOT EXIST "%LDRAW_LIBS%\%OfficialCONTENT%" (
  ECHO.
  ECHO -LDraw archive libs does not exist - Downloading...

  CALL :DOWNLOAD_LDRAW_LIBS for_build_check

  IF NOT EXIST "%LDRAW_LIBS%\" (
    ECHO.
    ECHO -Create LDraw archive libs store %LDRAW_LIBS%
    MKDIR "%LDRAW_LIBS%\"
  )
  IF EXIST "%LDRAW_DOWNLOAD_DIR%\%OfficialCONTENT%" (
    MOVE /Y %LDRAW_DOWNLOAD_DIR%\%OfficialCONTENT% %LDRAW_LIBS%\%OfficialCONTENT% | findstr /i /v /r /c:"moved\>"
  ) ELSE (
    ECHO.
    ECHO -ERROR - LDraw archive libs %LDRAW_DOWNLOAD_DIR%\%OfficialCONTENT% does not exist.
  )
  IF EXIST "%LDRAW_DOWNLOAD_DIR%\%LPub3DCONTENT%" (
    MOVE /Y %LDRAW_DOWNLOAD_DIR%\%LPub3DCONTENT% %LDRAW_LIBS%\%LPub3DCONTENT% | findstr /i /v /r /c:"moved\>"
  ) ELSE (
    ECHO -ERROR - LDraw archive libs %LDRAW_DOWNLOAD_DIR%\%LPub3DCONTENT% does not exist.
  )
)
IF EXIST "%LDRAW_LIBS%\%OfficialCONTENT%" (
  COPY /V /Y "%LDRAW_LIBS%\%OfficialCONTENT%" "%PKG_TARGET_DIR%\extras\" /A | findstr /i /v /r /c:"copied\>"
)
IF EXIST "%LDRAW_LIBS%\%LPub3DCONTENT%" (
  COPY /V /Y "%LDRAW_LIBS%\%LPub3DCONTENT%" "%PKG_TARGET_DIR%\extras\" /A | findstr /i /v /r /c:"copied\>"
)
EXIT /b