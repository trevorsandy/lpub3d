@ECHO OFF

Title LPub3D Windows build check script

rem  Trevor SANDY <trevor.sandy@gmail.com>
rem  Last Update: October 22, 2018
rem  Copyright (c) 2018 by Trevor SANDY
rem --
rem This script is distributed in the hope that it will be useful,
rem but WITHOUT ANY WARRANTY; without even the implied warranty of
rem MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

rem Construct the staged files path
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

SET PKG_CHECK_OPTIONS=--ignore-console-redirect --process-file --clear-cache --preferred-renderer ldview-sc
SET PKG_CHECK_LDVIEW_SINGLE_CALL_COMMAND=%PKG_TARGET% %PKG_CHECK_OPTIONS% %PKG_CHECK_FILE%

SET PKG_CHECK_OPTIONS=--ignore-console-redirect --process-export --range 1-3 --clear-cache --preferred-renderer ldglite
SET PKG_CHECK_RANGE_COMMAND=%PKG_TARGET% %PKG_CHECK_OPTIONS% %PKG_CHECK_FILE%

SET PKG_CHECK_OPTIONS=--ignore-console-redirect --process-file --clear-cache --preferred-renderer povray
SET PKG_CHECK_POV_COMMAND=%PKG_TARGET% %PKG_CHECK_OPTIONS% %PKG_CHECK_FILE%

CALL :CHECK_LDRAW_DIR
CALL :SET_LDRAW_LIBS

ECHO.
ECHO   PKG_CHECK_OPTIONS.........[%PKG_CHECK_OPTIONS%]
ECHO   PKG_DISTRO_DIR............[%PKG_DISTRO_DIR%]
ECHO   PKG_PRODUCT_DIR...........[%PKG_PRODUCT_DIR%]
ECHO   PKG_TARGET_DIR............[%PKG_TARGET_DIR%]
ECHO   PKG_TARGET................[%PKG_TARGET%]
ECHO.
ECHO   Build checks start...
ECHO ------------------------------------------------
ECHO.
IF NOT EXIST "%PKG_TARGET%" (
  ECHO -ERROR - %PKG_TARGET% does not exist, build check will exit.
  EXIT /b
) ELSE (
  ECHO -%PKG_TARGET% found.
  IF EXIST "Check.out" DEL /Q "Check.out"
  ECHO.
  ECHO   1 OF 5. PKG_CHECK_NATIVE_COMMAND...[%PKG_CHECK_NATIVE_COMMAND%]
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
  ECHO   2 OF 5. PKG_CHECK_LDVIEW_COMMAND...[%PKG_CHECK_LDVIEW_COMMAND%]
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
  ECHO   3 OF 5. PKG_CHECK_LDVIEW_SINGLE_CALL_COMMAND...[%PKG_CHECK_LDVIEW_COMMAND%]
  CALL %PKG_CHECK_LDVIEW_SINGLE_CALL_COMMAND% > Check.out 2>&1
  FOR %%R IN (Check.out) DO (
    IF NOT %%~zR LSS 1 (
      ECHO -BUILD_CHECK_LDVIEW_SINGLE_CALL Output...
      TYPE "Check.out"
      ECHO.
      DEL /Q "Check.out"
      ECHO.
    ) ELSE (
      ECHO. -ERROR - BUILD_CHECK_LDVIEW_SINGLE_CALL failed.
    )
  )
  ECHO.
  ECHO   4 OF 5. PKG_CHECK_RANGE_COMMAND....[%PKG_CHECK_RANGE_COMMAND%]
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
  ECHO   5 OF 5. PKG_CHECK_POV_COMMAND......[%PKG_CHECK_POV_COMMAND%]
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
  ECHO.
  ECHO   Build checks cleanup...
  RMDIR /S /Q %PKG_TARGET_DIR%\cache
  RMDIR /S /Q %PKG_TARGET_DIR%\logs
  RMDIR /S /Q %PKG_TARGET_DIR%\config
  RMDIR /S /Q %PKG_TARGET_DIR%\libraries

  ECHO.
  ECHO   Build checks completed.
  ECHO ------------------------------------------------
  ECHO.
)