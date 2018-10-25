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
SET PKG_CHECK_OPTIONS=--process-file --clear-cache --liblego --preferred-renderer povray
SET PKG_CHECK_POV_COMMAND=%PKG_TARGET% %PKG_CHECK_OPTIONS% %PKG_CHECK_FILE%
rem Check 6 of 7
SET PKG_CHECK_FILE=%ABS_WD%\builds\check\TENTE\astromovil.ldr
SET PKG_CHECK_OPTIONS=--process-file --clear-cache --libtente --preferred-renderer ldview
SET PKG_CHECK_TENTE_COMMAND=%PKG_TARGET% %PKG_CHECK_OPTIONS% %PKG_CHECK_FILE%
rem Check 7 of 7
SET PKG_CHECK_FILE=%ABS_WD%\builds\check\VEXIQ\spider.mpd
SET PKG_CHECK_OPTIONS=--process-file --clear-cache --libvexiq --preferred-renderer ldview-scsl
SET PKG_CHECK_VEXIQ_COMMAND=%PKG_TARGET% %PKG_CHECK_OPTIONS% %PKG_CHECK_FILE%

ECHO.
ECHO   PKG_CHECK_OPTIONS.........[%PKG_CHECK_OPTIONS%]
ECHO   PKG_DISTRO_DIR............[%PKG_DISTRO_DIR%]
ECHO   PKG_PRODUCT_DIR...........[%PKG_PRODUCT_DIR%]
ECHO   PKG_TARGET_DIR............[%PKG_TARGET_DIR%]
ECHO   PKG_TARGET................[%PKG_TARGET%]
ECHO   LDRAW_LIB_STORE...........[%LDRAW_LIBS%]

CALL :SET_LDRAW_LIBS

ECHO.
ECHO   Build checks start...
ECHO ------------------------------------------------
ECHO.
IF NOT EXIST "%PKG_TARGET%" (
  ECHO -ERROR - %PKG_TARGET% does not exist. The build check cannot continue.
  EXIT /b
) ELSE (
  ECHO -%PKG_TARGET% found.
  IF EXIST "Check.out" DEL /Q "Check.out"
  ECHO.
  ECHO   1 OF 7. PKG_CHECK_NATIVE_COMMAND...[%PKG_CHECK_NATIVE_COMMAND%]
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
  ECHO   2 OF 7. PKG_CHECK_LDVIEW_COMMAND...[%PKG_CHECK_LDVIEW_COMMAND%]
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
  ECHO   3 OF 7. PKG_CHECK_LDVIEW_SINGLE_CALL_COMMAND...[%PKG_CHECK_LDVIEW_COMMAND%]
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
  ECHO   4 OF 7. PKG_CHECK_RANGE_COMMAND....[%PKG_CHECK_RANGE_COMMAND%]
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
  ECHO   5 OF 7. PKG_CHECK_POV_COMMAND......[%PKG_CHECK_POV_COMMAND%]
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
  ECHO   6 OF 7. PKG_CHECK_TENTE_COMMAND......[%PKG_CHECK_TENTE_COMMAND%]
  CALL %PKG_CHECK_TENTE_COMMAND% > Check.out 2>&1
  FOR %%R IN (Check.out) DO (
    IF NOT %%~zR LSS 1 (
      ECHO -BUILD_CHECK_TENTE Output...
      TYPE "Check.out"
      ECHO.
      DEL /Q "Check.out"
      ECHO.
    ) ELSE (
      ECHO. -ERROR - BUILD_CHECK_TENTE failed.
    )
  )
  ECHO.
  ECHO   7 OF 7. PKG_CHECK_VEXIQ_COMMAND......[%PKG_CHECK_VEXIQ_COMMAND%]
  CALL %PKG_CHECK_VEXIQ_COMMAND% > Check.out 2>&1
  FOR %%R IN (Check.out) DO (
    IF NOT %%~zR LSS 1 (
      ECHO -BUILD_CHECK_VEXIQ Output...
      TYPE "Check.out"
      ECHO.
      DEL /Q "Check.out"
      ECHO.
    ) ELSE (
      ECHO. -ERROR - BUILD_CHECK_VEXIQ failed.
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
