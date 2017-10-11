@ECHO off
Title Create archive packaged and windows installer LPub3D distributions
rem --
rem  Trevor SANDY <trevor.sandy@gmail.com>
rem  Last Update: October 10, 2017
rem  Copyright (c) 2015 - 2017 by Trevor Sandy
rem --
SETLOCAL
@break off
@color 0a

ECHO.
ECHO - Create archive packaged and windows installer LPub3D distributions

:: TEST ONLY ***********************************
::SET APPVEYOR=True
:: *********************************************

:: increment alt_ver_xx to show last released version
SET ALT_VER_01=2.0.20
SET ALT_VER_02=unknown
SET ALT_VER_03=unknown
:: available versions (by platform) shows last released version last 3 versions from previous major release
SET AVAILABLE_VERS_EXE=%ALT_VER_01%,1.3.5,1.2.3,1.0.0
SET AVAILABLE_VERS_DMG=%ALT_VER_01%
SET AVAILABLE_VERS_DEB=%ALT_VER_01%
SET AVAILABLE_VERS_RPM=%ALT_VER_01%
SET AVAILABLE_VERS_PKG=%ALT_VER_01%
:: all versions include available versions plus current version to be released
:: e.g. 2.0.21,2.0.20,1.3.5,1.2.3,1.0.0

:: pwd = lpub3d\
CD /D "builds\windows"
:: pwd = lpub3d\buidls\windows\
SET WIN_PKG_DIR=%cd%

:: DEBUG ONLY **********************************
:: ECHO Working Directory INIT  = %WIN_PKG_DIR%
:: *********************************************

SET RUN_NSIS=1
SET SIGN_APP=0
SET CREATE_PORTABLE=1

SET VERSION_INPUT="0 0 0 00 000"
SET DATETIME_INPUT="0000 00 00 00:00:00.00"

SET AUTO=0
SET UNIVERSAL_BUILD=1
IF "%APPVEYOR%" EQU "True" (
  SET AUTO=1
)

IF %AUTO% NEQ 1 (
  ECHO.
  SET /p RUN_NSIS= - Run NSIS: Type 1 to run, 0 to ignore or 'Enter' to accept default [%RUN_NSIS%]:
  IF %RUN_NSIS% == 0 (
     SET SIGN_APP=0
     SET CREATE_PORTABLE=0
  )
  IF %RUN_NSIS% == 1 (
    ECHO.
    SET /p SIGN_APP= - Code Signing: Type 1 to run, 0 to ignore or 'Enter' to accept default [%SIGN_APP%]:
  )
)

ECHO.
ECHO - Selected build options:
ECHO.
IF %RUN_NSIS% == 0 ECHO - This configuration will allow you to test your NSIS scripts.
IF %APPVEYOR%   == "True" ECHO   APPVEYOR BUILD  [Yes]
IF %UNIVERSAL_BUILD% == 1 ECHO   UNIVERSAL BUILD [Yes]
IF %AUTO%            == 1 ECHO   AUTOMATIC BUILD [Yes]
IF %RUN_NSIS%        == 1 ECHO   RUN_NSIS        [Yes]
IF %SIGN_APP%        == 1 ECHO   SIGN_APP        [Yes]
IF %RUN_NSIS%        == 0 ECHO   RUN_NSIS        [No]
IF %SIGN_APP%        == 0 ECHO   SIGN_APP        [No]
IF %UNIVERSAL_BUILD% == 0 ECHO   UNIVERSAL BUILD [No]
IF %AUTO%            == 0 ECHO   AUTOMATIC BUILD [No]

IF %RUN_NSIS% == 0 ECHO.
IF %RUN_NSIS% == 0 ECHO - Start NSIS test build process...
IF %RUN_NSIS% == 1 ECHO.
IF %RUN_NSIS% == 1 ECHO - Start build process...

SET zipExe=unknown
SET zipWin64=C:\program files\7-zip
SET zipWin32=C:\Program Files (x86)\7-zip

SET NSISExe="C:\Program Files (x86)\NSIS\makensis.exe"
SET SignToolExe="C:\Program Files (x86)\Windows Kits\8.1\bin\x64\signtool.exe"

SET utilitiesPath=..\utilities
SET devRootPath=..\..\mainApp

SET LDRAWINI_BUILD_FILE=LDrawIni161.dll
SET QUAZIP_BUILD_FILE=QuaZIP07.dll

SET LDGLITE_DIR=ldglite-1.3
SET LDVIEW_DIR=ldview-4.3
SET LPUB3D_TRACE_DIR=lpub3d_trace_cui-3.8

SET LDGLITE_EXE=ldglite
SET LDVIEW_EXE=LDView
SET LPUB3D_TRACE_EXE=lpub3d_trace_cui

SET TimeStamp=http://timestamp.digicert.com
SET Sha2=Sha256
SET PKey="%utilitiesPath%\cert\lpub3dPrivateKey.p12"

ECHO.
ECHO - Environment check...

IF %RUN_NSIS% == 0 GOTO MAIN

IF EXIST %NSISExe% (
	ECHO.
  ECHO - NSIS executable found at %NSISExe%
  GOTO SIGN
)

SET %RUN_NSIS%=0
SET %SIGN_APP%=0
SET %CREATE_PORTABLE%=0
ECHO.
ECHO * NSIS exectutable not at path defined. Only NSIS test configuration will be produced.
GOTO MAIN

:SIGN
IF %SIGN_APP% == 0 GOTO CHK_ZIP

IF "%APPVEYOR%" NEQ "True" (
  SET PwD=unknown
  SET CHK_ZIP_GO=0
  IF EXIST %SignToolExe% (
    ECHO.
    ECHO - Signtool executable found at %SignToolExe%
    FOR /f %%i IN (../utilities/cert/credentials.txt) DO SET PwD=%%i
    SET CHK_ZIP_GO=1
  )
  ECHO.
  ECHO - Code signing password is %PWD%.
) ELSE (
  ECHO.
  ECHO * Code signing is not available.
  GOTO CHK_ZIP
)

IF %CHK_ZIP_GO% == 1 GOTO CHK_ZIP

SET %SIGN_APP%=0
ECHO.
ECHO * Signtool exectutable not at path defined. Code signing will be ignored.

:CHK_ZIP
IF EXIST "%zipWin64%" (
	SET zipExe="%zipWin64%\7z.exe"
	ECHO.
  ECHO - Zip exectutable found at "%zipWin64%"
  GOTO MAIN
)

ECHO.

IF EXIST "%zipWin32%" (
	SET zipExe="%zipWin32%\7z.exe"
	ECHO.
  ECHO - Zip exectutable found at "%zipWin32%"
  GOTO MAIN
)

ECHO.

IF %AUTO% NEQ 1 (
  IF NOT EXIST "%zipExe%" (
    ECHO.
    ECHO * Could not find zip executable. Requested manual location entry.
    SET /p zipExe=Could not find any zip executable. You can manually enter a location:
  )

  IF EXIST "%zipExe%" (
    ECHO.
    ECHO - Zip exectutable at (%zipExe%) will be used to archive your portable distributions.
    SET OPTION=1
    SET /p OPTION= Type [1] to exit or Enter to continue:
  )

  IF  %OPTION% == 1  ECHO * Option to exit seleced, the script will terminate.
  IF  %OPTION% == 1  EXIT
)

IF NOT EXIST "%zipExe%" (
	SET CREATE_PORTABLE=0
	ECHO.
	ECHO * Could not find zip executable. Portable distributions will be ignored.
  GOTO MAIN
)

:MAIN
SET PRODUCT=unknown
SET LP3D_VERSION=unknown
SET VER_MAJOR=unknown
SET VER_MINOR=unknown
SET VER_BUILD=unknown
SET VER_PATCH=unknown
SET VER_REVISION=unknown
SET VER_SHA_HASH=unknown

SET LP3D_ARCH=unknown
SET LP3D_ARCH_EXT=unknown

SET LP3D_YEAR=unknown
SET LP3D_MONTH=unknown
SET LP3D_DAY=unknown
SET LP3D_TIME=unknown

SET COMPANY=unknown
SET COMMENTS=unknown
SET PUBLISHER=unknown
SET COMPANY_URL=unknown
SET BUILD_DATE=unknown
SET REVISION_FILE=unknown
SET SUPPORT_EMAIL=unknown
SET LPUB3D_BUILD_FILE=unknown

SET LAST_VER_EXE=unknown
SET LAST_VER_DMG=unknown
SET LAST_VER_DEB=unknown
SET LAST_VER_RPM=unknown
SET LAST_VER_PKG=unknown

SET ALT_VER_EXE=unknown
SET ALT_VER_DMG=unknown
SET ALT_VER_DEB=unknown
SET ALT_VER_RPM=unknown
SET ALT_VER_PKG=unknown

ECHO.
ECHO - Setting up release build parameters...

CD /D "%utilitiesPath%"

SET VERSION_INFO_FILE=version.info
FOR /f "tokens=1" %%i IN (%VERSION_INFO_FILE%) DO SET VER_MAJOR=%%i
FOR /f "tokens=2" %%i IN (%VERSION_INFO_FILE%) DO SET VER_MINOR=%%i
FOR /f "tokens=3" %%i IN (%VERSION_INFO_FILE%) DO SET VER_PATCH=%%i
FOR /f "tokens=4" %%i IN (%VERSION_INFO_FILE%) DO SET VER_REVISION=%%i
FOR /f "tokens=5" %%i IN (%VERSION_INFO_FILE%) DO SET VER_BUILD=%%i
FOR /f "tokens=6" %%i IN (%VERSION_INFO_FILE%) DO SET VER_SHA_HASH=%%i

FOR /f "tokens=7" %%i IN (%VERSION_INFO_FILE%) DO SET LP3D_DAY=%%i
FOR /f "tokens=8" %%i IN (%VERSION_INFO_FILE%) DO SET LP3D_MONTH=%%i
FOR /f "tokens=9" %%i IN (%VERSION_INFO_FILE%) DO SET LP3D_YEAR=%%i
FOR /f "tokens=10" %%i IN (%VERSION_INFO_FILE%) DO SET LP3D_TIME=%%i

CD /D "%WIN_PKG_DIR%"
CD /D "%devRootPath%"

FOR /F "tokens=3*" %%i IN ('FINDSTR /c:"#define VER_COMPANYNAME_BLD_STR" version.h') DO SET COMPANY=%%i %%j
FOR /F "tokens=3*" %%i IN ('FINDSTR /c:"#define VER_PRODUCTNAME_STR" version.h') DO SET PRODUCT=%%i
FOR /F "tokens=3*" %%i IN ('FINDSTR /c:"#define VER_PUBLISHER_STR" version.h') DO SET PUBLISHER=%%i %%j
FOR /F "tokens=3*" %%i IN ('FINDSTR /c:"#define VER_COMPANYDOMAIN_STR" version.h') DO SET COMPANY_URL=%%i
FOR /F "tokens=3*" %%i IN ('FINDSTR /c:"#define VER_FILEDESCRIPTION_STR" version.h') DO SET COMMENTS=%%i %%j
FOR /F "tokens=3*" %%i IN ('FINDSTR /c:"#define VER_PUBLISHER_SUPPORT_EMAIL_STR" version.h') DO SET SUPPORT_EMAIL=%%i

FOR /F "tokens=1 delims=," %%i IN ("%AVAILABLE_VERS_EXE%") DO SET LAST_VER_EXE=%%i
FOR /F "tokens=1 delims=," %%i IN ("%AVAILABLE_VERS_DMG%") DO SET LAST_VER_DMG=%%i
FOR /F "tokens=1 delims=," %%i IN ("%AVAILABLE_VERS_DEB%") DO SET LAST_VER_DEB=%%i
FOR /F "tokens=1 delims=," %%i IN ("%AVAILABLE_VERS_RPM%") DO SET LAST_VER_RPM=%%i
FOR /F "tokens=1 delims=," %%i IN ("%AVAILABLE_VERS_PKG%") DO SET LAST_VER_PKG=%%i

FOR /F "tokens=2 delims=," %%i IN ("%AVAILABLE_VERS_EXE%") DO SET ALT_VER_EXE=%%i
FOR /F "tokens=1 delims=," %%i IN ("%AVAILABLE_VERS_DMG%") DO SET ALT_VER_DMG=%%i
FOR /F "tokens=1 delims=," %%i IN ("%AVAILABLE_VERS_DEB%") DO SET ALT_VER_DEB=%%i
FOR /F "tokens=1 delims=," %%i IN ("%AVAILABLE_VERS_RPM%") DO SET ALT_VER_RPM=%%i
FOR /F "tokens=1 delims=," %%i IN ("%AVAILABLE_VERS_PKG%") DO SET ALT_VER_PKG=%%i

SET VER_BUILD=%VER_BUILD:"=%
SET PRODUCT=%PRODUCT:"=%

SET LP3D_DATE_TIME=%LP3D_YEAR% %LP3D_MONTH% %LP3D_DAY% %LP3D_TIME%
SET LP3D_VERSION=%VER_MAJOR%.%VER_MINOR%.%VER_PATCH%
SET BUILD_VERSION=%LP3D_VERSION%.%VER_REVISION%.%VER_BUILD%
SET FULL_VERSION=%LP3D_VERSION%.%VER_REVISION%.%VER_BUILD%_%LP3D_YEAR%%LP3D_MONTH%%LP3D_DAY%
SET DOWNLOAD_PRODUCT=%PRODUCT%-%FULL_VERSION%
SET SUPPORT_EMAIL=%SUPPORT_EMAIL% %BUILD_VERSION%
SET PRODUCT_DIR=%PRODUCT%-Any-%FULL_VERSION%
SET LPUB3D_BUILD_FILE=%PRODUCT%%VER_MAJOR%%VER_MINOR%.exe

CD /D "%WIN_PKG_DIR%"

IF NOT EXIST "release\%PRODUCT_DIR%" (
  ECHO.
  ECHO * Did not find product directory. Expected %PRODUCT_DIR% at "%cd%\release\".
  ECHO Build %PRODUCT%_%FULL_VERSION% will terminate.
  EXIT /b 5
)

:: pwd = windows/release/PRODUCT_DIR [holds _PKG_DIST_DIR, PKG_UPDATE_DIR, PKG_DOWNLOAD_DIR]
CD /D "release\%PRODUCT_DIR%"

ECHO.
ECHO - Create folders and delete onld content...

IF NOT EXIST "%PRODUCT%_Download\" (
  MKDIR "%PRODUCT%_Download\"
)
SET PKG_DOWNLOAD_DIR=%PRODUCT%_Download

IF NOT EXIST "%PRODUCT%_Update\" (
  MKDIR "%PRODUCT%_Update\"
)
SET PKG_UPDATE_DIR=%PRODUCT%_Update

DEL /Q "%PKG_DOWNLOAD_DIR%\*.*"

DEL /Q "%PKG_UPDATE_DIR%\*.*"

ECHO.
ECHO - Building distribution package...
ECHO.
ECHO   VERSION_INFO_FILE...[%VERSION_INFO_FILE%]
ECHO.
ECHO   VER_MAJOR...........[%VER_MAJOR%]
ECHO   VER_MINOR...........[%VER_MINOR%]
ECHO   VER_PATCH...........[%VER_PATCH%]
ECHO   VER_REVISION........[%VER_REVISION%]
ECHO   VER_BUILD...........[%VER_BUILD%]
ECHO   VER_SHA_HASH........[%VER_SHA_HASH%]
ECHO.
ECHO   LP3D_YEAR...........[%LP3D_YEAR%]
ECHO   LP3D_MONTH..........[%LP3D_MONTH%]
ECHO   LP3D_DAY............[%LP3D_DAY%]
ECHO   LP3D_TIME...........[%LP3D_TIME%]
ECHO   LP3D_DATE_TIME......[%LP3D_DATE_TIME%]
ECHO.
ECHO   COMPANY.............[%COMPANY%]
ECHO   PRODUCT.............[%PRODUCT%]
ECHO   PUBLISHER...........[%PUBLISHER%]
ECHO   COMPANY_URL.........[%COMPANY_URL%]
ECHO   SUPPORT_EMAIL.......[%SUPPORT_EMAIL%"]
ECHO   COMMENTS............[%COMMENTS%]
ECHO.
ECHO   ALT_VER_EXE.........[%ALT_VER_EXE%]
ECHO   ALT_VER_DMG.........[%ALT_VER_DMG%]
ECHO   ALT_VER_DEB.........[%ALT_VER_DEB%]
ECHO   ALT_VER_RPM.........[%ALT_VER_RPM%]
ECHO   ALT_VER_PKG.........[%ALT_VER_PKG%]
ECHO   LAST_VER_EXE........[%LAST_VER_EXE%]
ECHO   LAST_VER_DMG........[%LAST_VER_DMG%]
ECHO   LAST_VER_DEB........[%LAST_VER_DEB%]
ECHO   LAST_VER_RPM........[%LAST_VER_RPM%]
ECHO   LAST_VER_PKG........[%LAST_VER_PKG%]
ECHO   AVAILABLE_VERS_EXE..[%AVAILABLE_VERS_EXE%]
ECHO   AVAILABLE_VERS_DMG..[%AVAILABLE_VERS_DMG%]
ECHO   AVAILABLE_VERS_DEB..[%AVAILABLE_VERS_DEB%]
ECHO   AVAILABLE_VERS_RPM..[%AVAILABLE_VERS_RPM%]
ECHO   AVAILABLE_VERS_PKG..[%AVAILABLE_VERS_PKG%]

IF %UNIVERSAL_BUILD% NEQ 1 (
  IF "%APPVEYOR%" EQU "True" (
    SET LP3D_ARCH=%build%
  ) ELSE (
    IF NOT [%1] == [] (
      SET LP3D_ARCH=%1
    ) ELSE (
      IF %AUTO% EQU 1 (
        ECHO.
        ECHO * No valid architecture detected. The script will terminate.
        GOTO END
      ) ELSE (
        SET LP3D_ARCH=x86_64
        SET /p LP3D_ARCH=  Enter build architecture [x86^|x86_64] or press enter to accept default [%LP3D_ARCH%]:
        IF "%LP3D_ARCH%" NEQ "x86_64" (
          IF "%LP3D_ARCH%" NEQ "x86" (
            ECHO.
            ECHO * No valid architecture detected. The script will terminate.
            GOTO END
          )
        )
      )
    )
  )
  ECHO   LP3D_ARCH...........[%LP3D_ARCH%]
  CALL :BUILDSETUP
  IF %RUN_NSIS% == 1 CALL :DOWNLOADLDRAWLIBS
  IF %RUN_NSIS% == 1 CALL :GENERATENSISINPUT
  IF %RUN_NSIS% == 1 CALL :NSISBUILD
  IF %SIGN_APP% == 1 CALL :SIGNAPP
  IF %CREATE_PORTABLE% == 1 CALL :CREATEPORTABLEDISTRO
) ELSE (
  :: Universal build
  FOR %%A IN ( x86_64, x86 ) DO (
    SET LP3D_ARCH=%%A
    SETLOCAL ENABLEDELAYEDEXPANSION
    ECHO   LP3D_ARCH...........[!LP3D_ARCH!]
    ENDLOCAL)
    CALL :BUILDSETUP
  )
  IF %RUN_NSIS% == 1 CALL :DOWNLOADLDRAWLIBS
  IF %RUN_NSIS% == 1 CALL :GENERATENSISINPUT
  IF %RUN_NSIS% == 1 CALL :NSISBUILD
  IF %SIGN_APP% == 1 CALL :SIGNAPP
  FOR %%A IN ( x86_64, x86 ) DO (
    SET PKG_DISTRO_PORTABLE_DIR=%PRODUCT%_%%A-%FULL_VERSION%
    IF %CREATE_PORTABLE% == 1 CALL :CREATEPORTABLEDISTRO
  )
)
CALL :GENERATE_JSON ELSE EXIT /b %ERRORLEVEL%
GOTO POSTPROCESS

:BUILDSETUP
:: Product Full Version Format:
:: Product _ Version . Revision . Build _ Date (YYYYMMDD)
:: LPub3D  _ 2.0.20  . 106      . 752   _ 20170929

:: Directory Structure Format:
:: Windows                                              - %cd%/windows
::   |_ Release                                         - windows/release
::        |_PRODUCT_DIR          = PRODUCT FULL_VERSION - LPub3D_2.0.20.106.752_20170929
::            |_PKG_DIST_DIR     = PRODUCT LP3D_ARCH    - LPub3D_x86_64
::            |_PKG_DOWNLOAD_DIR = PRODUCT_Download     - LPub3D_Download
::            |_PKG_UPDATE_DIR   = PRODUCT_Update       - LPub3D_Update
SET PKG_DISTRO_DIR=%PRODUCT%_%LP3D_ARCH%

ECHO   LP3D_VERSION........[%LP3D_VERSION%]
ECHO   BUILD_VERSION.......[%BUILD_VERSION%]
ECHO   FULL_VERSION........[%FULL_VERSION%]
ECHO   DOWNLOAD_PRODUCT....[%DOWNLOAD_PRODUCT%]
ECHO   PRODUCT_DIR.........[%PRODUCT_DIR%]
ECHO   PKG_DISTRO_DIR......[%PKG_DISTRO_DIR%]

ECHO.
ECHO - Configuring %PRODUCT% %LP3D_ARCH% build...

IF NOT EXIST "%PKG_DISTRO_DIR%" (
  ECHO.
  ECHO * Did not find distribution package directory.
  ECHO * Expected "%PKG_DISTRO_DIR%" at "%cd%".
  ECHO * Build "%PRODUCT%_%FULL_VERSION%" will terminate.
  EXIT /b 5
)

ECHO.
ECHO - Copying change_log_%LP3D_VERSION% to media folder...

:: pwd = windows/release/PRODUCT_DIR
COPY /V /Y %PKG_DISTRO_DIR%\docs\README.txt %PKG_UPDATE_DIR%\change_log.txt /A
COPY /V /Y %PKG_DISTRO_DIR%\docs\README.txt %PKG_UPDATE_DIR%\change_log_%LP3D_VERSION%.txt /A
COPY /V /Y %PKG_DISTRO_DIR%\docs\README.txt %PKG_DOWNLOAD_DIR%\ /A
ECHO.
EXIT /b

:: AppVersion.nsh pwd = builds\utilities\nsis-scripts
:GENERATENSISINPUT
ECHO.
ECHO - Generating AppVersion.nsh build input script...

SET versionFile=..\..\..\utilities\nsis-scripts\AppVersion.nsh
SET genVersion=%versionFile% ECHO

:GENERATE AppVersion.nsh NSIS build input file
>%genVersion% !define Company %COMPANY%
>>%genVersion% ; ${Company}
>>%genVersion%.
>>%genVersion% !define ProductName "%PRODUCT%"
>>%genVersion% ; ${ProductName}
>>%genVersion%.
>>%genVersion% !define Version "%LP3D_VERSION%"
>>%genVersion% ; ${Version}
>>%genVersion%.
>>%genVersion% !define CompleteVersion "%LP3D_VERSION%.%VER_REVISION%.%VER_BUILD%_%LP3D_YEAR%%LP3D_MONTH%%LP3D_DAY%"
>>%genVersion% ; ${CompleteVersion}
>>%genVersion%.
>>%genVersion% !define UniversalBuild %UNIVERSAL_BUILD%
>>%genVersion% ; ${UniversalBuild}
>>%genVersion%.
>>%genVersion% !define Architecture %LP3D_ARCH%
>>%genVersion% ; ${Architecture}
>>%genVersion%.
>>%genVersion% !define ArchExt %LP3D_ARCH_EXT%
>>%genVersion% ; ${ArchExt}
>>%genVersion%.
>>%genVersion% !define ProductDir "..\..\windows\release\%PRODUCT_DIR%"
>>%genVersion% ; ${ProductDir}
>>%genVersion%.
>>%genVersion% ; OutFile Dir
>>%genVersion% !define OutFileDir "..\..\windows\release\%PRODUCT_DIR%"
>>%genVersion% ; ${OutFileDir}
>>%genVersion%.
>>%genVersion% ; InFile Dir:
IF %UNIVERSAL_BUILD% EQU 1 (
>>%genVersion%.
>>%genVersion% !define WinBuildDir "..\..\windows\release\%PRODUCT_DIR%\%PRODUCT%_x86_64"
>>%genVersion% ; ${WinBuildDir} - non-binary file location - using x86_64 loc for each arche
>>%genVersion%.
>>%genVersion% !define Win64BuildDir "..\..\windows\release\%PRODUCT_DIR%\%PRODUCT%_x86_64"
>>%genVersion% ; ${Win64BuildDir}
>>%genVersion%.
>>%genVersion% !define Win32BuildDir "..\..\windows\release\%PRODUCT_DIR%\%PRODUCT%_x86"
>>%genVersion% ; ${Win32BuildDir}
) ELSE (
>>%genVersion%.
>>%genVersion% !define WinBuildDir "..\..\windows\release\%PRODUCT_DIR%\%PKG_DISTRO_DIR%"
>>%genVersion% ; ${WinBuildDir}
)
>>%genVersion%.
>>%genVersion% !define LPub3DBuildFile "%LPUB3D_BUILD_FILE%"
>>%genVersion% ; ${LPub3DBuildFile}
>>%genVersion%.
>>%genVersion% !define QuaZipBuildFile "%QUAZIP_BUILD_FILE%"
>>%genVersion% ; ${QuaZipBuildFile}
>>%genVersion%.
>>%genVersion% !define LDrawIniBuildFile "%LDRAWINI_BUILD_FILE%"
>>%genVersion% ; ${LDrawIniBuildFile}
>>%genVersion%.
>>%genVersion% !define LDGliteDir "%LDGLITE_DIR%"
>>%genVersion% ; ${LDGliteDir}
>>%genVersion%.
>>%genVersion% !define LDViewDir "%LDVIEW_DIR%"
>>%genVersion% ; ${LDViewDir}
>>%genVersion%.
>>%genVersion% !define LPub3D_TraceDir "%LPUB3D_TRACE_DIR%"
>>%genVersion% ; ${LPub3D_TraceDir}
>>%genVersion%.
>>%genVersion% !define LDGliteExe "%LDGLITE_EXE%"
>>%genVersion% ; ${LDGliteExe}
>>%genVersion%.
>>%genVersion% !define LDViewExe "%LDVIEW_EXE%"
>>%genVersion% ; ${LDViewExe}
>>%genVersion%.
>>%genVersion% !define LPub3D_TraceExe "%LPUB3D_TRACE_EXE%"
>>%genVersion% ; ${LPub3D_TraceExe}
>>%genVersion%.
>>%genVersion% !define BuildRevision "%VER_REVISION%"
>>%genVersion% ; ${BuildRevision}
>>%genVersion%.
>>%genVersion% !define BuildNumber "%VER_BUILD%"
>>%genVersion% ; ${BuildNumber}
>>%genVersion%.
>>%genVersion% !define BuildDate "%LP3D_DATE_TIME%"
>>%genVersion% ; ${BuildDate}
>>%genVersion%.
>>%genVersion% !define Publisher %PUBLISHER%
>>%genVersion% ; ${Publisher}
>>%genVersion%.
>>%genVersion% !define CompanyURL %COMPANY_URL%
>>%genVersion% ; ${CompanyURL}
>>%genVersion%.
>>%genVersion% !define Comments %COMMENTS%
>>%genVersion% ; ${Comments}
>>%genVersion%.
>>%genVersion% !define SupportEmail %SUPPORT_EMAIL% %LP3D_VERSION%.%VER_REVISION%.%VER_BUILD%_%LP3D_YEAR%%LP3D_MONTH%%LP3D_DAY%"
>>%genVersion% ; ${SupportEmail}
>>%genVersion%.
EXIT /b

:: pwd = windows/release/PRODUCT_DIR
:NSISBUILD
IF %RUN_NSIS% == 1 ECHO.
IF %RUN_NSIS% == 1 ECHO - Start NSIS Master Update Build...

IF %RUN_NSIS% == 0 ECHO.
IF %RUN_NSIS% == 0 ECHO - Ignore NSIS Master Update Build...

IF %RUN_NSIS% == 1 %NSISExe% /DUpdateMaster ..\..\..\utilities\nsis-scripts\LPub3DNoPack.nsi | findstr.exe /i /r /c:"^Processing\>" /c:"^Output\>"

IF %RUN_NSIS% == 1 ECHO.
IF %RUN_NSIS% == 1 ECHO - Finished NSIS Master Update  Build...

IF %RUN_NSIS% == 1 ECHO.
IF %RUN_NSIS% == 1 ECHO - Start NSIS Manual Install Build...

IF %RUN_NSIS% == 0 ECHO.
IF %RUN_NSIS% == 0 ECHO - Ignore NSIS Manual Install Build...

IF %RUN_NSIS% == 1 COPY /V /Y %PRODUCT%-UpdateMaster_%LP3D_VERSION%.exe %PKG_DOWNLOAD_DIR%\%DOWNLOAD_PRODUCT%.exe
IF %RUN_NSIS% == 1 COPY /V /Y %PRODUCT%-UpdateMaster_%LP3D_VERSION%.exe %PKG_UPDATE_DIR%\%PRODUCT%-UpdateMaster.exe
IF %RUN_NSIS% == 1 MOVE /Y    %PRODUCT%-UpdateMaster_%LP3D_VERSION%.exe %PKG_UPDATE_DIR%\

IF %RUN_NSIS% == 1 ECHO.
IF %RUN_NSIS% == 1 ECHO - Finished NSIS Manual Install Build...
EXIT /b

:SIGNAPP
IF %SIGN_APP% == 1 ECHO.
IF %SIGN_APP% == 1 ECHO - Start Application Code Signing Build...

IF %SIGN_APP% == 0 ECHO.
IF %SIGN_APP% == 0 ECHO - Ignore Application Code Signing Build...

IF %SIGN_APP% == 1 %SignToolExe% sign /tr %TimeStamp% /td %Sha2% /fd %Sha2% /f %PKey% /p %PwD% %PKG_DOWNLOAD_DIR%\%DOWNLOAD_PRODUCT%.exe
IF %SIGN_APP% == 1 %SignToolExe% sign /tr %TimeStamp% /td %Sha2% /fd %Sha2% /f %PKey% /p %PwD% %PKG_UPDATE_DIR%\%PRODUCT%-UpdateMaster_%LP3D_VERSION%.exe
IF %SIGN_APP% == 1 %SignToolExe% sign /tr %TimeStamp% /td %Sha2% /fd %Sha2% /f %PKey% /p %PwD% %PKG_UPDATE_DIR%\%PRODUCT%-UpdateMaster.exe
IF %UNIVERSAL_BUILD% EQU 1 (
  IF %SIGN_APP% == 1 %SignToolExe% sign /tr %TimeStamp% /td %Sha2% /fd %Sha2% /f %PKey% /p %PwD% %PRODUCT%_x86\%LPUB3D_BUILD_FILE%
  IF %SIGN_APP% == 1 %SignToolExe% sign /tr %TimeStamp% /td %Sha2% /fd %Sha2% /f %PKey% /p %PwD% %PRODUCT%_x86_64\%LPUB3D_BUILD_FILE%
) ELSE (
  IF %SIGN_APP% == 1 %SignToolExe% sign /tr %TimeStamp% /td %Sha2% /fd %Sha2% /f %PKey% /p %PwD% %PKG_DISTRO_DIR%\%LPUB3D_BUILD_FILE%
)

IF %SIGN_APP% == 1 ECHO.
IF %SIGN_APP% == 1 ECHO - Generating hash checksum listing...

IF %SIGN_APP% == 1 CertUtil -hashfile %PKG_DOWNLOAD_DIR%\%DOWNLOAD_PRODUCT%.exe SHA256							       >  %PKG_DOWNLOAD_DIR%\LPub3D.%LP3D_VERSION%.Checksums.txt
IF %SIGN_APP% == 1 CertUtil -hashfile %PKG_UPDATE_DIR%\%PRODUCT%-UpdateMaster_%LP3D_VERSION%.exe SHA256  	>>  %PKG_DOWNLOAD_DIR%\LPub3D.%LP3D_VERSION%.Checksums.txt
IF %SIGN_APP% == 1 CertUtil -hashfile %PKG_UPDATE_DIR%\%PRODUCT%-UpdateMaster.exe SHA256  						    >>  %PKG_DOWNLOAD_DIR%\LPub3D.%LP3D_VERSION%.Checksums.txt
IF %UNIVERSAL_BUILD% EQU 1 (
  IF %SIGN_APP% == 1 CertUtil -hashfile %PRODUCT%_x86\%LPUB3D_BUILD_FILE%                                 >>  %PKG_DOWNLOAD_DIR%\LPub3D.%LP3D_VERSION%.Checksums.txt
  IF %SIGN_APP% == 1 CertUtil -hashfile %PRODUCT%_x86_64\%LPUB3D_BUILD_FILE%                              >>  %PKG_DOWNLOAD_DIR%\LPub3D.%LP3D_VERSION%.Checksums.txt
) ELSE (
  IF %SIGN_APP% == 1 CertUtil -hashfile %PKG_DISTRO_DIR%\%LPUB3D_BUILD_FILE% SHA256		                    >>  %PKG_DOWNLOAD_DIR%\LPub3D.%LP3D_VERSION%.Checksums.txt
)

IF %SIGN_APP% == 1 ECHO.
IF %SIGN_APP% == 1 ECHO - Finished Application Code Signing Build...
EXIT /b

:CREATEPORTABLEDISTRO
IF %CREATE_PORTABLE% == 1 ECHO.
IF %CREATE_PORTABLE% == 1 ECHO - Create portable media zip files...

IF %CREATE_PORTABLE% == 0 ECHO.
IF %CREATE_PORTABLE% == 0 ECHO - Ignore create portable media zip files...

IF %CREATE_PORTABLE% == 1 %zipExe% a -tzip %PKG_DOWNLOAD_DIR%\%PKG_DISTRO_PORTABLE_DIR%.zip %PKG_DISTRO_DIR%\ | findstr.exe /i /r /c:"^Creating\>" /c:"^Everything\>"
EXIT /b

:GENERATE_JSON
ECHO.
ECHO - Generating package distribution json components...

ECHO.
ECHO - Generating lastVersionInsert_Exe.txt input file...

SET lastVersionInsert_ExeFile=%PKG_UPDATE_DIR%\lastVersionInsert_Exe.txt
SET genlastVersionInsert_Exe=%lastVersionInsert_ExeFile% ECHO

:GENERATE lastVersionInsert_Exe.txt file
>%genlastVersionInsert_Exe% "alternate-version-%LAST_VER_EXE%-exe": {
>>%genlastVersionInsert_Exe%   "open-url": "https://sourceforge.net/projects/lpub3d/files/%LAST_VER_EXE%/",
>>%genlastVersionInsert_Exe%   "latest-version": "%LAST_VER_EXE%",
>>%genlastVersionInsert_Exe%   "download-url": "http://lpub3d.sourceforge.net/LPub3D-UpdateMaster_%LAST_VER_EXE%.exe",
>>%genlastVersionInsert_Exe%   "changelog-url": "http://lpub3d.sourceforge.net/change_log_%LAST_VER_EXE%.txt"
>>%genlastVersionInsert_Exe% },

ECHO.
ECHO - Generating lastVersionInsert_Dmg.txt input file...

SET lastVersionInsert_DmgFile=%PKG_UPDATE_DIR%\lastVersionInsert_Dmg.txt
SET genlastVersionInsert_Dmg=%lastVersionInsert_DmgFile% ECHO

:GENERATE lastVersionInsert_Dmg.txt file
>%genlastVersionInsert_Dmg% "alternate-version-%LAST_VER_DMG%-dmg": {
>>%genlastVersionInsert_Dmg%   "open-url": "https://sourceforge.net/projects/lpub3d/files/%LAST_VER_DMG%/",
>>%genlastVersionInsert_Dmg%   "latest-version": "%LAST_VER_DMG%",
>>%genlastVersionInsert_Dmg%   "download-url": "http://lpub3d.sourceforge.net/LPub3D-UpdateMaster_%LAST_VER_DMG%_osx.dmg",
>>%genlastVersionInsert_Dmg%   "changelog-url": "http://lpub3d.sourceforge.net/change_log_%LAST_VER_DMG%.txt"
>>%genlastVersionInsert_Dmg% },

ECHO.
ECHO - Generating lastVersionInsert_Deb.txt input file...

SET lastVersionInsert_DebFile=%PKG_UPDATE_DIR%\lastVersionInsert_Deb.txt
SET genlastVersionInsert_Deb=%lastVersionInsert_DebFile% ECHO

:GENERATE lastVersionInsert_Deb.txt file
>%genlastVersionInsert_Deb% "alternate-version-%LAST_VER_DEB%-deb": {
>>%genlastVersionInsert_Deb%   "open-url": "https://sourceforge.net/projects/lpub3d/files/%LAST_VER_DEB%/",
>>%genlastVersionInsert_Deb%   "latest-version": "%LAST_VER_DEB%",
>>%genlastVersionInsert_Deb%   "download-url": "http://lpub3d.sourceforge.net/LPub3D-UpdateMaster_%LAST_VER_DEB%_0ubuntu1_amd64.deb",
>>%genlastVersionInsert_Deb%   "changelog-url": "http://lpub3d.sourceforge.net/change_log_%LAST_VER_DEB%.txt"
>>%genlastVersionInsert_Deb% },

ECHO.
ECHO - Generating lastVersionInsert_Rpm.txt input file...

SET lastVersionInsert_RpmFile=%PKG_UPDATE_DIR%\lastVersionInsert_Rpm.txt
SET genlastVersionInsert_Rpm=%lastVersionInsert_RpmFile% ECHO

:GENERATE lastVersionInsert_Rpm.txt file
>%genlastVersionInsert_Rpm% "alternate-version-%LAST_VER_RPM%-rpm": {
>>%genlastVersionInsert_Rpm%   "open-url": "https://sourceforge.net/projects/lpub3d/files/%LAST_VER_RPM%/",
>>%genlastVersionInsert_Rpm%   "latest-version": "%LAST_VER_RPM%",
>>%genlastVersionInsert_Rpm%   "download-url": "http://lpub3d.sourceforge.net/LPub3D-UpdateMaster_%LAST_VER_RPM%_fc.%LP3D_ARCH%.rpm",
>>%genlastVersionInsert_Rpm%   "changelog-url": "http://lpub3d.sourceforge.net/change_log_%LAST_VER_RPM%.txt"
>>%genlastVersionInsert_Rpm% },

ECHO.
ECHO - Generating lastVersionInsert_Pkg.txt input file...

SET lastVersionInsert_PkgFile=%PKG_UPDATE_DIR%\lastVersionInsert_Pkg.txt
SET genlastVersionInsert_Pkg=%lastVersionInsert_PkgFile% ECHO

:GENERATE lastVersionInsert_Pkg.txt file
>%genlastVersionInsert_Pkg% "alternate-version-%LAST_VER_PKG%-pkg": {
>>%genlastVersionInsert_Pkg%   "open-url": "https://sourceforge.net/projects/lpub3d/files/%LAST_VER_PKG%/",
>>%genlastVersionInsert_Pkg%   "latest-version": "%LAST_VER_PKG%",
>>%genlastVersionInsert_Pkg%   "download-url": "http://lpub3d.sourceforge.net/LPub3D-UpdateMaster_%LAST_VER_PKG%_%LP3D_ARCH%.pkg.tar.xz",
>>%genlastVersionInsert_Pkg%   "changelog-url": "http://lpub3d.sourceforge.net/change_log_%LAST_VER_PKG%.txt"
>>%genlastVersionInsert_Pkg% },

ECHO.
ECHO - Generating lpub3dupdates.json template file...

SET updatesFile=%PKG_UPDATE_DIR%\lpub3dupdates.json
SET genLPub3DUpdates=%updatesFile% ECHO

:GENERATE lpub3dupdates.json template file
>%genLPub3DUpdates% {
>>%genLPub3DUpdates%   "_comment": "LPub3D lpub3dupdates.json generated on %LP3D_DATE_TIME%",
>>%genLPub3DUpdates%   "updates": {
>>%genLPub3DUpdates%     "windows": {
>>%genLPub3DUpdates%       "open-url": "https://sourceforge.net/projects/lpub3d/files/%LP3D_VERSION%/",
>>%genLPub3DUpdates%       "latest-version": "%LP3D_VERSION%",
>>%genLPub3DUpdates%       "download-url": "http://lpub3d.sourceforge.net/LPub3D-UpdateMaster.exe",
>>%genLPub3DUpdates%       "changelog-url": "http://lpub3d.sourceforge.net/change_log.txt",
>>%genLPub3DUpdates%       "available-versions": "%LP3D_VERSION%,%AVAILABLE_VERS_EXE%",
>>%genLPub3DUpdates%       "alternate-version-1.3.5": {
>>%genLPub3DUpdates%         "open-url": "https://sourceforge.net/projects/lpub3d/files/1.3.5/",
>>%genLPub3DUpdates%         "latest-version": "1.3.5",
>>%genLPub3DUpdates%         "download-url": "http://lpub3d.sourceforge.net/LPub3D-UpdateMaster_1.3.5.exe",
>>%genLPub3DUpdates%         "changelog-url": "http://lpub3d.sourceforge.net/change_log_1.3.5.txt"
>>%genLPub3DUpdates%       },
>>%genLPub3DUpdates%       "alternate-version-1.2.3": {
>>%genLPub3DUpdates%         "open-url": "https://sourceforge.net/projects/lpub3d/files/1.2.3/",
>>%genLPub3DUpdates%         "latest-version": "1.2.3",
>>%genLPub3DUpdates%         "download-url": "http://lpub3d.sourceforge.net/LPub3D-UpdateMaster_1.2.3.exe",
>>%genLPub3DUpdates%         "changelog-url": "http://lpub3d.sourceforge.net/change_log_1.2.3.txt"
>>%genLPub3DUpdates%       },
>>%genLPub3DUpdates%       "alternate-version-1.0.0": {
>>%genLPub3DUpdates%         "open-url": "https://sourceforge.net/projects/lpub3d/files/1.0.0/",
>>%genLPub3DUpdates%         "latest-version": "1.0.0",
>>%genLPub3DUpdates%         "download-url": "http://lpub3d.sourceforge.net/LPub3D-UpdateMaster_1.0.0.exe",
>>%genLPub3DUpdates%         "changelog-url": "http://lpub3d.sourceforge.net/change_log_1.0.0.txt"
>>%genLPub3DUpdates%       }
>>%genLPub3DUpdates%     "windows-exe": {
>>%genLPub3DUpdates%       "open-url": "https://sourceforge.net/projects/lpub3d/files/%LP3D_VERSION%/",
>>%genLPub3DUpdates%       "latest-version": "%LP3D_VERSION%",
>>%genLPub3DUpdates%       "download-url": "http://lpub3d.sourceforge.net/LPub3D-UpdateMaster.exe",
>>%genLPub3DUpdates%       "changelog-url": "http://lpub3d.sourceforge.net/change_log.txt",
>>%genLPub3DUpdates%       "download-url-": "http://lpub3d.sourceforge.net/LPub3D-UpdateMaster_%LP3D_VERSION%.exe",
>>%genLPub3DUpdates%       "changelog-url-": "http://lpub3d.sourceforge.net/change_log_%LP3D_VERSION%.txt",
>>%genLPub3DUpdates%       "available-versions": "%LP3D_VERSION%,%AVAILABLE_VERS_EXE%",
>>%genLPub3DUpdates%       "alternate-version-1.3.5-exe": {
>>%genLPub3DUpdates%         "open-url": "https://sourceforge.net/projects/lpub3d/files/1.3.5/",
>>%genLPub3DUpdates%         "latest-version": "1.3.5",
>>%genLPub3DUpdates%         "download-url": "http://lpub3d.sourceforge.net/LPub3D-UpdateMaster_1.3.5.exe",
>>%genLPub3DUpdates%         "changelog-url": "http://lpub3d.sourceforge.net/change_log_1.3.5.txt"
>>%genLPub3DUpdates%       },
>>%genLPub3DUpdates%       "alternate-version-1.2.3-exe": {
>>%genLPub3DUpdates%         "open-url": "https://sourceforge.net/projects/lpub3d/files/1.2.3/",
>>%genLPub3DUpdates%         "latest-version": "1.2.3",
>>%genLPub3DUpdates%         "download-url": "http://lpub3d.sourceforge.net/LPub3D-UpdateMaster_1.2.3.exe",
>>%genLPub3DUpdates%         "changelog-url": "http://lpub3d.sourceforge.net/change_log_1.2.3.txt"
>>%genLPub3DUpdates%       },
>>%genLPub3DUpdates%       "alternate-version-1.0.0-exe": {
>>%genLPub3DUpdates%         "open-url": "https://sourceforge.net/projects/lpub3d/files/1.0.0/",
>>%genLPub3DUpdates%         "latest-version": "1.0.0",
>>%genLPub3DUpdates%         "download-url": "http://lpub3d.sourceforge.net/LPub3D-UpdateMaster_1.0.0.exe",
>>%genLPub3DUpdates%         "changelog-url": "http://lpub3d.sourceforge.net/change_log_1.0.0.txt"
>>%genLPub3DUpdates%       }
>>%genLPub3DUpdates%     },
>>%genLPub3DUpdates%     "osx-dmg": {
>>%genLPub3DUpdates%       "open-url": "https://sourceforge.net/projects/lpub3d/files/%LP3D_VERSION%/",
>>%genLPub3DUpdates%       "latest-version": "%LP3D_VERSION%",
>>%genLPub3DUpdates%       "download-url": "http://lpub3d.sourceforge.net/LPub3D-UpdateMaster_%LP3D_VERSION%_osx.dmg",
>>%genLPub3DUpdates%       "changelog-url": "http://lpub3d.sourceforge.net/change_log_%LP3D_VERSION%.txt",
>>%genLPub3DUpdates%       "available-versions": "%LP3D_VERSION%,%AVAILABLE_VERS_DMG%",
>>%genLPub3DUpdates%       "alternate-version-%ALT_VER_01%-dmg": {
>>%genLPub3DUpdates%         "open-url": "https://sourceforge.net/projects/lpub3d/files/%ALT_VER_01%/",
>>%genLPub3DUpdates%         "latest-version": "%ALT_VER_01%",
>>%genLPub3DUpdates%         "download-url": "http://lpub3d.sourceforge.net/LPub3D-UpdateMaster_%ALT_VER_01%_osx.dmg",
>>%genLPub3DUpdates%         "changelog-url": "http://lpub3d.sourceforge.net/change_log_%ALT_VER_01%.txt"
>>%genLPub3DUpdates%       }
>>%genLPub3DUpdates%     },
>>%genLPub3DUpdates%     "linux-deb": {
>>%genLPub3DUpdates%       "open-url": "https://sourceforge.net/projects/lpub3d/files/%LP3D_VERSION%/",
>>%genLPub3DUpdates%       "latest-version": "%LP3D_VERSION%",
>>%genLPub3DUpdates%       "download-url": "http://lpub3d.sourceforge.net/LPub3D-UpdateMaster_%LP3D_VERSION%_amd64.deb",
>>%genLPub3DUpdates%       "changelog-url": "http://lpub3d.sourceforge.net/change_log_%LP3D_VERSION%.txt",
>>%genLPub3DUpdates%       "available-versions": "%LP3D_VERSION%,%AVAILABLE_VERS_DEB%",
>>%genLPub3DUpdates%       "alternate-version-%ALT_VER_01%-deb": {
>>%genLPub3DUpdates%         "open-url": "https://sourceforge.net/projects/lpub3d/files/%ALT_VER_01%/",
>>%genLPub3DUpdates%         "latest-version": "%ALT_VER_01%",
>>%genLPub3DUpdates%         "download-url": "http://lpub3d.sourceforge.net/LPub3D-UpdateMaster_%ALT_VER_01%_amd64.deb",
>>%genLPub3DUpdates%         "changelog-url": "http://lpub3d.sourceforge.net/change_log_%ALT_VER_01%.txt"
>>%genLPub3DUpdates%       }
>>%genLPub3DUpdates%     },
>>%genLPub3DUpdates%     "linux-rpm": {
>>%genLPub3DUpdates%       "open-url": "https://sourceforge.net/projects/lpub3d/files/%LP3D_VERSION%/",
>>%genLPub3DUpdates%       "latest-version": "%LP3D_VERSION%",
>>%genLPub3DUpdates%       "download-url": "http://lpub3d.sourceforge.net/LPub3D-UpdateMaster_%LP3D_VERSION%_fc.%LP3D_ARCH%.rpm",
>>%genLPub3DUpdates%       "changelog-url": "http://lpub3d.sourceforge.net/change_log_%LP3D_VERSION%.txt",
>>%genLPub3DUpdates%       "available-versions": "%LP3D_VERSION%,%AVAILABLE_VERS_DEB%",
>>%genLPub3DUpdates%       "alternate-version-%ALT_VER_01%-rpm": {
>>%genLPub3DUpdates%         "open-url": "https://sourceforge.net/projects/lpub3d/files/%ALT_VER_01%/",
>>%genLPub3DUpdates%         "latest-version": "%ALT_VER_01%",
>>%genLPub3DUpdates%         "download-url": "http://lpub3d.sourceforge.net/LPub3D-UpdateMaster_%ALT_VER_01%_fc.%LP3D_ARCH%.rpm",
>>%genLPub3DUpdates%         "changelog-url": "http://lpub3d.sourceforge.net/change_log_%ALT_VER_01%.txt"
>>%genLPub3DUpdates%       }
>>%genLPub3DUpdates%     },
>>%genLPub3DUpdates%     "linux-pkg": {
>>%genLPub3DUpdates%       "open-url": "https://sourceforge.net/projects/lpub3d/files/%LP3D_VERSION%/",
>>%genLPub3DUpdates%       "latest-version": "%LP3D_VERSION%",
>>%genLPub3DUpdates%       "download-url": "http://lpub3d.sourceforge.net/LPub3D-UpdateMaster_%LP3D_VERSION%_%LP3D_ARCH%.pkg.tar.xz",
>>%genLPub3DUpdates%       "changelog-url": "http://lpub3d.sourceforge.net/change_log_%LP3D_VERSION%.txt",
>>%genLPub3DUpdates%       "available-versions": "%LP3D_VERSION%,%AVAILABLE_VERS_DEB%",
>>%genLPub3DUpdates%       "alternate-version-%ALT_VER_01%-pkg": {
>>%genLPub3DUpdates%         "open-url": "https://sourceforge.net/projects/lpub3d/files/%ALT_VER_01%/",
>>%genLPub3DUpdates%         "latest-version": "%ALT_VER_01%",
>>%genLPub3DUpdates%         "download-url": "http://lpub3d.sourceforge.net/LPub3D-UpdateMaster_%ALT_VER_01%_%LP3D_ARCH%.pkg.tar.xz",
>>%genLPub3DUpdates%         "changelog-url": "http://lpub3d.sourceforge.net/change_log_%ALT_VER_01%.txt"
>>%genLPub3DUpdates%       }
>>%genLPub3DUpdates%     }
>>%genLPub3DUpdates%   }
>>%genLPub3DUpdates% }
>>%genLPub3DUpdates%.

ECHO.
ECHO - Generating lpub3dupdates.json package distribution file...

(
  FOR /F "tokens=*" %%i IN (%PKG_UPDATE_DIR%\lpub3dupdates.json) DO (
    IF "%%i" EQU ""alternate-version-%ALT_VER_EXE%": {" (
      TYPE %PKG_UPDATE_DIR%\lastVersionInsert_Exe.txt
    )
    IF "%%i" EQU ""alternate-version-%ALT_VER_DMG%": {" (
      TYPE %PKG_UPDATE_DIR%\lastVersionInsert_Dmg.txt
    )
    IF "%%i" EQU ""alternate-version-%ALT_VER_DEB%": {" (
      TYPE %PKG_UPDATE_DIR%\lastVersionInsert_Deb.txt
    )
    IF "%%i" EQU ""alternate-version-%ALT_VER_RPM%": {" (
      TYPE %PKG_UPDATE_DIR%\lastVersionInsert_Rpm.txt
    )
    IF "%%i" EQU ""alternate-version-%ALT_VER_PKG%": {" (
      TYPE %PKG_UPDATE_DIR%\lastVersionInsert_Pkg.txt
    )
  ECHO %%i
  )
) >temp.txt
MOVE /y temp.txt %PKG_UPDATE_DIR%\lpub3dupdates.json
DEL /Q %PKG_UPDATE_DIR%\lastVersionInsert_Exe.txt
DEL /Q %PKG_UPDATE_DIR%\lastVersionInsert_Dmg.txt
DEL /Q %PKG_UPDATE_DIR%\lastVersionInsert_Deb.txt
DEL /Q %PKG_UPDATE_DIR%\lastVersionInsert_Rpm.txt
DEL /Q %PKG_UPDATE_DIR%\lastVersionInsert_Pkg.txt

ECHO.
ECHO - Copying additional json FILES to media folder...

:: pwd = windows\release\PRODUCT_DIR
COPY /V /Y ..\..\..\utilities\json\complete.json %PKG_UPDATE_DIR%\ /A
COPY /V /Y ..\..\..\utilities\json\lpub3dldrawunf.json %PKG_UPDATE_DIR%\ /A

ECHO.
ECHO - Generating latest.txt version input file (backgward compatability)...

SET latestFile=%PKG_UPDATE_DIR%\latest.txt
SET genLatest=%latestFile% ECHO

:GENERATE latest.txt file
>%genLatest% %LP3D_VERSION%
EXIT /b


:DOWNLOADLDRAWLIBS
ECHO.
ECHO - Download LDraw archive libraries...

SET OfficialCONTENT=complete.zip
SET UnofficialCONTENT=ldrawunf.zip
SET Lpub3dCONTENT=lpub3dldrawunf.zip

SET OutputPATH=%WIN_PKG_DIR%\release\%PRODUCT_DIR%

IF "%APPVEYOR%" EQU "True" (
 GOTO APPVEYORDOWNLOAD
)

ECHO.
ECHO - Prepare BATCH to VBS to Web Content Downloader...

IF NOT EXIST "%TEMP%\$" (
  MD "%TEMP%\$"
)

SET vbs=WebContentDownload.vbs
SET t=%TEMP%\$\%vbs% ECHO

IF EXIST %TEMP%\$\%vbs% (
 DEL %TEMP%\$\%vbs%
)

:WEB CONTENT SAVE-AS Download-- VBS
>%t% Option Explicit
>>%t% On Error Resume Next
>>%t%.
>>%t% Dim args, http, fileSystem, adoStream, url, target, status
>>%t%.
>>%t% Set args = Wscript.Arguments
>>%t% Set http = CreateObject("WinHttp.WinHttpRequest.5.1")
>>%t% url = args(0)
>>%t% target = args(1)
>>%t% WScript.Echo "- Getting '" ^& target ^& "' from '" ^& url ^& "'...", vbLF
>>%t%.
>>%t% http.Open "GET", url, False
>>%t% http.Send
>>%t% status = http.Status
>>%t%.
>>%t% If status ^<^> 200 Then
>>%t% WScript.Echo "- FAILED to download: HTTP Status " ^& status, vbLF
>>%t% WScript.Quit 1
>>%t% End If
>>%t%.
>>%t% Set adoStream = CreateObject("ADODB.Stream")
>>%t% adoStream.Open
>>%t% adoStream.Type = 1
>>%t% adoStream.Write http.ResponseBody
>>%t% adoStream.Position = 0
>>%t%.
>>%t% Set fileSystem = CreateObject("Scripting.FileSystemObject")
>>%t% If fileSystem.FileExists(target) Then fileSystem.DeleteFile target
>>%t% If Err.Number ^<^> 0 Then
>>%t%   WScript.Echo "- Error - CANNOT DELETE: '" ^& target ^& "', " ^& Err.Description
>>%t%   WScript.Echo " The file may be in use by another process.", vbLF
>>%t%   adoStream.Close
>>%t%   Err.Clear
>>%t% Else
>>%t%  adoStream.SaveToFile target
>>%t%  adoStream.Close
>>%t%  WScript.Echo "- Download successful!"
>>%t% End If
>>%t%.
>>%t% 'WebContentDownload.vbs
>>%t% 'Title: BATCH to VBS to Web Content Downloader
>>%t% 'CMD ^> cscript //Nologo %TEMP%\$\%vbs% WebNAME WebCONTENT
>>%t% 'VBS Created on %date% at %time%
>>%t%.

ECHO.
ECHO - VBS file "%vbs%" is done compiling
ECHO.
ECHO - LDraw archive library download path: %OutputPATH%

SET LibraryOPTION=Unofficial
SET WebCONTENT="%OutputPATH%\%UnofficialCONTENT%"
SET WebNAME=http://www.ldraw.org/library/unofficial/ldrawunf.zip

ECHO.
ECHO - Download LDraw %LibraryOPTION% library archive...

ECHO.
ECHO - Web URL: "%WebNAME%"
ECHO.
ECHO - Download file: %WebCONTENT%

IF EXIST %WebCONTENT% (
 DEL %WebCONTENT%
)

ECHO.
cscript //Nologo %TEMP%\$\%vbs% %WebNAME% %WebCONTENT% && @ECHO off
IF "%LibraryOPTION%" EQU "Unofficial" (
	ECHO.
	ECHO - Rename archive file %UnofficialCONTENT% to %Lpub3dCONTENT%
	REN %UnofficialCONTENT% %Lpub3dCONTENT%
  ECHO.
  IF %UNIVERSAL_BUILD% EQU 1 (
    ECHO -Copy and move archive file %OfficialCONTENT% to extras directory
    COPY /V /Y ".\%Lpub3dCONTENT%"  "%PRODUCT%_x86_64\extras\"
    MOVE /y ".\%Lpub3dCONTENT%"  "%PRODUCT%_x86\extras\"
  ) ELSE (
    ECHO - Move archive file %OfficialCONTENT% to extras directory
    MOVE /y ".\%Lpub3dCONTENT%"  "%PKG_DISTRO_DIR%\extras\"
  )
)
ECHO.
ECHO - LDraw archive library %UnofficialCONTENT% downloaded

SET LibraryOPTION=Official
SET WebCONTENT="%OutputPATH%\%OfficialCONTENT%"
SET WebNAME=http://www.ldraw.org/library/updates/complete.zip

ECHO.
ECHO - Download LDraw %LibraryOPTION% library archive...

ECHO.
ECHO - Web URL: "%WebNAME%"
ECHO.
ECHO - Download archive file: %WebCONTENT%

IF EXIST %WebCONTENT% (
 DEL %WebCONTENT%
)

ECHO.
cscript //Nologo %TEMP%\$\%vbs% %WebNAME% %WebCONTENT% && @ECHO off
ECHO.
IF %UNIVERSAL_BUILD% EQU 1 (
  ECHO - Copy and move archive file %OfficialCONTENT% to extras directory
  COPY /V /Y ".\%OfficialCONTENT%"  "%PRODUCT%_x86_64\extras\"
  MOVE /y ".\%OfficialCONTENT%"  "%PRODUCT%_x86\extras\"
) ELSE (
  ECHO - Move archive file %OfficialCONTENT% to extras directory
  MOVE /y ".\%OfficialCONTENT%"  "%PKG_DISTRO_DIR%\extras\"
)

ECHO.
ECHO - LDraw archive library %OfficialCONTENT% downloaded
ECHO.
ECHO - LDraw archive libraries download finshed
ENDLOCAL
EXIT /b 0

:: pwd = windows/release/PRODUCT_DIR
:APPVEYORDOWNLOAD
ECHO.
ECHO - Download LDraw Official archive library %OfficialCONTENT%
appveyor Downloadfile "http://www.ldraw.org/library/updates/%OfficialCONTENT%" -FileName "%WIN_PKG_DIR%\release\%PRODUCT_DIR%\%OfficialCONTENT%"
ECHO.
ECHO - Download LDraw Unifficial archive library %UnofficialCONTENT%
appveyor Downloadfile "http://www.ldraw.org/library/unofficial/%UnofficialCONTENT%" -FileName "%WIN_PKG_DIR%\release\%PRODUCT_DIR%\%Lpub3dCONTENT%"
ECHO.
ECHO - Copy and move archive files to extras directory
IF %UNIVERSAL_BUILD% EQU 1 (
  COPY /V /Y ".\%OfficialCONTENT%"  "%PRODUCT%_x86_64\extras\"
  COPY /V /Y ".\%Lpub3dCONTENT%"  "%PRODUCT%_x86_64\extras\"
  MOVE /y ".\%OfficialCONTENT%"  "%PRODUCT%_x86\extras\"
  MOVE /y ".\%Lpub3dCONTENT%"  "%PRODUCT%_x86\extras\"
) ELSE (
  MOVE /y ".\%OfficialCONTENT%"  "%PKG_DISTRO_DIR%\extras\"
  MOVE /y ".\%Lpub3dCONTENT%"  "%PKG_DISTRO_DIR%\extras\"
)
ECHO.
ECHO - LDraw archive libraries download finshed
EXIT /b 0

:POSTPROCESS
IF %AUTO% NEQ 1 (
  ECHO.
  ECHO - Postprocess
  ECHO.
  ECHO If everything went well Press any key to EXIT!
  %SystemRoot%\explorer.exe "C:\Users\Trevor\Downloads\LEGO\LPub\project\LPub\LPub3D\builds\windows\release\%LP3D_VERSION%"
  PAUSE >NUL
)

:END
ENDLOCAL
ECHO.
ECHO - Finished
EXIT /b 0
