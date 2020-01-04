@ECHO off
Title Create windows installer and portable package archive LPub3D distributions
rem --
rem  Trevor SANDY <trevor.sandy@gmail.com>
rem  Last Update: Aug 06, 2019
rem  Copyright (c) 2015 - 2020 by Trevor SANDY
rem --
SETLOCAL
@break off
@color 0a

SET start=%time%

ECHO.
ECHO -Start %~nx0
ECHO.
ECHO - Create windows installer and portable package archive LPub3D distributions

REM Set the current working directory to the source directory root - e.g. lpub3d\
FOR %%* IN (.) DO SET SCRIPT_DIR=%%~nx*
IF "%SCRIPT_DIR%" EQU "windows" (
  CD /D ../../
)
FOR %%* IN (%CD%) DO SET LPUB3D_DIR=%%~nx*
IF "%SCRIPT_DIR%" NEQ "%LPUB3D_DIR%" (
  IF "%SCRIPT_DIR%" NEQ "windows" (
    ECHO.
    ECHO You must run %~nx0 from either %LPUB3D_DIR% or %LPUB3D_DIR%\builds\windows source folder.
    ECHO Example console command: CD %LPUB3D_DIR% &%~nx0
    ECHO %~nx0 will terminate!
    GOTO :END
  )
)

SET _PRO_FILE_PWD_=%CD%\mainApp
CD /D "builds\windows"
SET WIN_PKG_DIR=%CD%

SET RUN_NSIS=1
SET SIGN_APP=0
SET CREATE_PORTABLE=1
SET TEST_APPVEYOR=0

SET AUTO=0
SET UNIVERSAL_BUILD=1

IF /I "%INSTALL_CHECK%" EQU "True" (
  ECHO.
  ECHO - Install check detected.
  SET AUTO=1
)
IF /I "%APPVEYOR%" EQU "True" (
  SET AUTO=1
)

IF %AUTO% NEQ 1 (
  ECHO.
  SET /p RUN_NSIS= - Run NSIS? Type 1 to run, 0 to ignore or 'Enter' to accept default [%RUN_NSIS%]:
  IF %RUN_NSIS% EQU 1 (
    ECHO.
    SET /p SIGN_APP= - Code Sign? Type 1 to sign exes', 0 to ignore or 'Enter' to accept default [%SIGN_APP%]:
  )
  ECHO.
  SET /p TEST_APPVEYOR= - Test APPVEYOR Build? Type 1 to run test build, 0 to ignore or 'Enter' to accept default [%TEST_APPVEYOR%]:
)
IF %RUN_NSIS% EQU 0 (
  SET SIGN_APP=0
  SET CREATE_PORTABLE=0
)
IF %TEST_APPVEYOR% EQU 1 (
  SET APPVEYOR=True
  SET build=release
  SET APPVEYOR_BUILD_FOLDER=%USERPROFILE%
  SET APPVEYOR_REPO_BRANCH=master
  FOR /F "usebackq delims==" %%G IN (`git rev-parse HEAD`) DO SET APPVEYOR_REPO_COMMIT=%%G
)

ECHO.
ECHO - Selected build options:
ECHO.
IF %RUN_NSIS%        == 0 ECHO - This configuration will allow you to test your NSIS scripts.
IF /I "%APPVEYOR%"   == "True" ECHO   APPVEYOR BUILD.................[Yes]
IF %UNIVERSAL_BUILD% == 1 ECHO   MULTI-ARCH BUILD...............[Yes]
IF %AUTO%            == 1 ECHO   AUTOMATIC BUILD................[Yes]
IF %RUN_NSIS%        == 1 ECHO   RUN_NSIS.......................[Yes]
IF %SIGN_APP%        == 1 ECHO   SIGN_APP.......................[Yes]
IF %RUN_NSIS%        == 0 ECHO   RUN_NSIS.......................[No]
IF %SIGN_APP%        == 0 ECHO   SIGN_APP.......................[No]
IF %UNIVERSAL_BUILD% == 0 ECHO   MULTI-ARCH BUILD...............[No]
IF %AUTO%            == 0 ECHO   AUTOMATIC BUILD................[No]

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

SET VER_LDGLITE=ldglite-1.3
SET VER_LDVIEW=ldview-4.3
SET VER_LPUB3D_TRACE=lpub3d_trace_cui-3.8

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

IF /I "%APPVEYOR%" NEQ "True" (
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
SET LP3D_ARCH=unknown
SET LP3D_ARCH_EXT=unknown

SET LP3D_PRODUCT=unknown
SET LP3D_COMPANY=unknown
SET LP3D_COMMENTS=unknown
SET LP3D_PUBLISHER=unknown
SET LP3D_COMPANY_URL=unknown
SET LP3D_BUILD_DATE_ALT=unknown
SET LP3D_REVISION_FILE=unknown
SET LP3D_SUPPORT=unknown
SET LPUB3D_BUILD_FILE=unknown

SET LP3D_ALTERNATE_VERSIONS_exe=unknown
SET LP3D_ALTERNATE_VERSIONS_dmg=unknown
SET LP3D_ALTERNATE_VERSIONS_deb=unknown
SET LP3D_ALTERNATE_VERSIONS_rpm=unknown
SET LP3D_ALTERNATE_VERSIONS_pkg=unknown
SET LP3D_ALTERNATE_VERSIONS_api=unknown

SET LP3D_AVAILABLE_VERSIONS_exe=unknown
SET LP3D_AVAILABLE_VERSIONS_dmg=unknown
SET LP3D_AVAILABLE_VERSIONS_deb=unknown
SET LP3D_AVAILABLE_VERSIONS_rpm=unknown
SET LP3D_AVAILABLE_VERSIONS_pkg=unknown
SET LP3D_AVAILABLE_VERSIONS_api=unknown

SET LP3D_GITHUB_BASE=https://github.com/trevorsandy/%LPUB3D_DIR%
SET LP3D_SOURCEFORGE_OPEN_BASE=https://sourceforge.net
SET LP3D_SOURCEFORGE_UPDATE_BASE=https://lpub3d.sourceforge.io

ECHO.
ECHO - Setting up release build parameters...

CD /D "%utilitiesPath%"

SET LP3D_ICON_DIR=%CD%\icons

CALL update-config-files.bat %_PRO_FILE_PWD_%

IF /I "%APPVEYOR%" EQU "True" (

  CALL :CREATE_LP3D_PS_VARS_FILE
  CALL :CREATE_LP3D_BASH_VARS_FILE

)

REM Token assignments
REM tokens=*  : all tokens in %%i                               - e.g. %%i
REM tokens=2* : second token in %%i, rest in next token         - e.g. %%j
REM tokens=1,2: first token in %%i, second in %%j               - e.g. %%i, %%j
REM tokens=1-3: first token in %%i, second in %%j, third in %%k - e.g. %%i,%%j,%%k

REM available versions by platform, set tokens to select specific version or versions as appropriate
FOR /F "tokens=*   delims=," %%i IN ("%LP3D_AVAILABLE_VERSIONS%") DO SET LP3D_AVAILABLE_VERSIONS_exe=%%i
FOR /F "tokens=1-3 delims=," %%i IN ("%LP3D_AVAILABLE_VERSIONS%") DO SET LP3D_AVAILABLE_VERSIONS_dmg=%%i,%%j,%%k
FOR /F "tokens=1-3 delims=," %%i IN ("%LP3D_AVAILABLE_VERSIONS%") DO SET LP3D_AVAILABLE_VERSIONS_deb=%%i,%%j,%%k
FOR /F "tokens=1-3 delims=," %%i IN ("%LP3D_AVAILABLE_VERSIONS%") DO SET LP3D_AVAILABLE_VERSIONS_rpm=%%i,%%j,%%k
FOR /F "tokens=1-3 delims=," %%i IN ("%LP3D_AVAILABLE_VERSIONS%") DO SET LP3D_AVAILABLE_VERSIONS_pkg=%%i,%%j,%%k
FOR /F "tokens=1,2 delims=," %%i IN ("%LP3D_AVAILABLE_VERSIONS%") DO SET LP3D_AVAILABLE_VERSIONS_api=%%i,%%j

FOR /F "tokens=2*  delims=," %%i IN ("%LP3D_AVAILABLE_VERSIONS_exe%") DO SET LP3D_ALTERNATE_VERSIONS_exe=%%i,%%j
FOR /F "tokens=2,3 delims=," %%i IN ("%LP3D_AVAILABLE_VERSIONS_dmg%") DO SET LP3D_ALTERNATE_VERSIONS_dmg=%%i,%%j
FOR /F "tokens=2,3 delims=," %%i IN ("%LP3D_AVAILABLE_VERSIONS_deb%") DO SET LP3D_ALTERNATE_VERSIONS_deb=%%i,%%j
FOR /F "tokens=2,3 delims=," %%i IN ("%LP3D_AVAILABLE_VERSIONS_rpm%") DO SET LP3D_ALTERNATE_VERSIONS_rpm=%%i,%%j
FOR /F "tokens=2,3 delims=," %%i IN ("%LP3D_AVAILABLE_VERSIONS_pkg%") DO SET LP3D_ALTERNATE_VERSIONS_pkg=%%i,%%j
FOR /F "tokens=2   delims=," %%i IN ("%LP3D_AVAILABLE_VERSIONS_api%") DO SET LP3D_ALTERNATE_VERSIONS_api=%%i

CD /D "%WIN_PKG_DIR%"
CD /D "%devRootPath%"

FOR /F "tokens=3*" %%i IN ('FINDSTR /c:"#define VER_COMPANYNAME_BLD_STR" version.h') DO SET LP3D_COMPANY=%%i %%j
FOR /F "tokens=3*" %%i IN ('FINDSTR /c:"#define VER_PRODUCTNAME_STR" version.h') DO SET LP3D_PRODUCT=%%i
FOR /F "tokens=3*" %%i IN ('FINDSTR /c:"#define VER_PUBLISHER_STR" version.h') DO SET LP3D_PUBLISHER=%%i %%j
FOR /F "tokens=3*" %%i IN ('FINDSTR /c:"#define VER_COMPANYDOMAIN_STR" version.h') DO SET LP3D_COMPANY_URL=%%i
FOR /F "tokens=3*" %%i IN ('FINDSTR /c:"#define VER_FILEDESCRIPTION_STR" version.h') DO SET LP3D_COMMENTS=%%i %%j
FOR /F "tokens=3*" %%i IN ('FINDSTR /c:"#define VER_PUBLISHER_SUPPORT_STR" version.h') DO SET LP3D_SUPPORT=%%i

SET LP3D_PRODUCT=%LP3D_PRODUCT:"=%
SET LP3D_DATE_TIME=%LP3D_YEAR% %LP3D_MONTH% %LP3D_DAY% %LP3D_TIME%
SET LP3D_PRODUCT_DIR=%LP3D_PRODUCT%-Any-%LP3D_APP_VERSION_LONG%
SET LP3D_DOWNLOAD_PRODUCT=%LP3D_PRODUCT%-%LP3D_APP_VERSION_LONG%
rem SET LP3D_BUILD_FILE=%LP3D_PRODUCT%%LP3D_VER_MAJOR%%LP3D_VER_MINOR%.exe
rem SET LPUB3D_BUILD_FILE=%LP3D_BUILD_FILE%
SET LP3D_BUILD_FILE=%LP3D_PRODUCT%.exe
SET LPUB3D_BUILD_FILE=%LP3D_BUILD_FILE%

IF /I "%LP3D_BUILD_TYPE%" EQU "Continuous" (
  SET LP3D_VER_TAG_NAME=continuous
) ELSE (
  SET LP3D_VER_TAG_NAME=v%LP3D_VERSION%
)

CD /D "%WIN_PKG_DIR%"

IF NOT EXIST "release\%LP3D_PRODUCT_DIR%" (
  ECHO.
  ECHO * Did not find product directory. Expected %LP3D_PRODUCT_DIR% at "%cd%\release\"
  ECHO * %~nx0 will terminate!
  GOTO :END
)

REM pwd = windows/release/LP3D_PRODUCT_DIR [holds _PKG_DIST_DIR, PKG_UPDATE_DIR, PKG_DOWNLOAD_DIR]
CD /D "release\%LP3D_PRODUCT_DIR%"

ECHO.
ECHO - Create folders and delete old content...

IF NOT EXIST "%LP3D_PRODUCT%_Download\" (
  MKDIR "%LP3D_PRODUCT%_Download\"
)
SET PKG_DOWNLOAD_DIR=%LP3D_PRODUCT%_Download

IF NOT EXIST "%LP3D_PRODUCT%_Update\" (
  MKDIR "%LP3D_PRODUCT%_Update\"
)
SET PKG_UPDATE_DIR=%LP3D_PRODUCT%_Update

DEL /Q "%PKG_DOWNLOAD_DIR%\*.*"

DEL /Q "%PKG_UPDATE_DIR%\*.*"

ECHO.
ECHO - Building distribution package...
ECHO.
ECHO   LP3D_VER_MAJOR.................[%LP3D_VER_MAJOR%]
ECHO   LP3D_VER_MINOR.................[%LP3D_VER_MINOR%]
ECHO   LP3D_VER_PATCH.................[%LP3D_VER_PATCH%]
ECHO   LP3D_VER_REVISION..............[%LP3D_VER_REVISION%]
ECHO   LP3D_VER_BUILD.................[%LP3D_VER_BUILD%]
ECHO   LP3D_VER_SHA_HASH..............[%LP3D_VER_SHA_HASH%]
IF "%LP3D_VER_TAG_NAME%" NEQ "v%LP3D_VERSION%" (
  ECHO   LP3D_VER_TAG_NAME..............[%LP3D_VER_TAG_NAME%]
)
ECHO.
ECHO   LP3D_YEAR......................[%LP3D_YEAR%]
ECHO   LP3D_MONTH.....................[%LP3D_MONTH%]
ECHO   LP3D_DAY.......................[%LP3D_DAY%]
ECHO   LP3D_TIME......................[%LP3D_TIME%]
ECHO.

ECHO   LP3D_VERSION...................[%LP3D_VERSION%]
ECHO   LP3D_APP_VERSION_LONG..........[%LP3D_APP_VERSION_LONG%]
ECHO   LP3D_AVAILABLE_VERSIONS........[%LP3D_AVAILABLE_VERSIONS%]
REM
REM Vars above this line are sourced from update-config-files.bat and below are local
ECHO.
ECHO   LP3D_COMPANY...................[%LP3D_COMPANY%]
ECHO   LP3D_PRODUCT...................[%LP3D_PRODUCT%]
ECHO   LP3D_PUBLISHER.................[%LP3D_PUBLISHER%]
ECHO   LP3D_COMPANY_URL...............[%LP3D_COMPANY_URL%]
ECHO   LP3D_SUPPORT...................[%LP3D_SUPPORT%]
ECHO   LP3D_COMMENTS..................[%LP3D_COMMENTS%]
ECHO.
ECHO   LP3D_DATE_TIME.................[%LP3D_DATE_TIME%]
ECHO   LP3D_DOWNLOAD_PRODUCT..........[%LP3D_DOWNLOAD_PRODUCT%]
ECHO   LP3D_PRODUCT_DIR...............[%LP3D_PRODUCT_DIR%]
ECHO   LP3D_BUILD_FILE................[%LP3D_BUILD_FILE%]
ECHO.
ECHO   LP3D_AVAILABLE_VERSIONS_exe....[%LP3D_AVAILABLE_VERSIONS_exe%]
ECHO   LP3D_AVAILABLE_VERSIONS_dmg....[%LP3D_AVAILABLE_VERSIONS_dmg%]
ECHO   LP3D_AVAILABLE_VERSIONS_deb....[%LP3D_AVAILABLE_VERSIONS_deb%]
ECHO   LP3D_AVAILABLE_VERSIONS_rpm....[%LP3D_AVAILABLE_VERSIONS_rpm%]
ECHO   LP3D_AVAILABLE_VERSIONS_pkg....[%LP3D_AVAILABLE_VERSIONS_pkg%]
ECHO   LP3D_AVAILABLE_VERSIONS_api....[%LP3D_AVAILABLE_VERSIONS_api%]
ECHO.
ECHO   LP3D_ALTERNATE_VERSIONS_exe....[%LP3D_ALTERNATE_VERSIONS_exe%]
ECHO   LP3D_ALTERNATE_VERSIONS_dmg....[%LP3D_ALTERNATE_VERSIONS_dmg%]
ECHO   LP3D_ALTERNATE_VERSIONS_deb....[%LP3D_ALTERNATE_VERSIONS_deb%]
ECHO   LP3D_ALTERNATE_VERSIONS_rpm....[%LP3D_ALTERNATE_VERSIONS_rpm%]
ECHO   LP3D_ALTERNATE_VERSIONS_PKG....[%LP3D_ALTERNATE_VERSIONS_PKG%]
ECHO   LP3D_ALTERNATE_VERSIONS_api....[%LP3D_ALTERNATE_VERSIONS_api%]

IF %UNIVERSAL_BUILD% NEQ 1 (
  IF /I "%APPVEYOR%" EQU "True" (
    SET LP3D_ARCH=%build%
  ) ELSE (
    IF NOT [%1] == [] (
      SET LP3D_ARCH=%1
    ) ELSE (
      IF %AUTO% EQU 1 (
        ECHO.
        ECHO * No valid architecture detected. %~nx0 will terminate!
        GOTO END
      ) ELSE (
        SET LP3D_ARCH=x86_64
        SET /p LP3D_ARCH=  Enter build architecture [x86^|x86_64] or press enter to accept default [%LP3D_ARCH%]:
        IF "%LP3D_ARCH%" NEQ "x86_64" (
          IF "%LP3D_ARCH%" NEQ "x86" (
            ECHO.
            ECHO * No valid architecture detected. %~nx0 will terminate!
            GOTO END
          )
        )
      )
    )
  )
  SET PKG_DISTRO_DIR=%LP3D_PRODUCT%_%LP3D_ARCH%
  SET PKG_DISTRO_PORTABLE_DIR=%LP3D_PRODUCT%_%LP3D_ARCH%-%LP3D_APP_VERSION_LONG%
  ECHO.
  ECHO - Configuring %LP3D_PRODUCT% %LP3D_ARCH% build...
  ECHO.
  ECHO   LP3D_ARCH......................[%LP3D_ARCH%]
  ECHO   PKG_DISTRO_DIR.................[%PKG_DISTRO_DIR%]
  CALL :COPYFILES
  IF %RUN_NSIS% == 1 CALL :DOWNLOADLDRAWLIBS
  IF %RUN_NSIS% == 1 CALL :GENERATENSISPARAMS
  IF %RUN_NSIS% == 1 CALL :NSISBUILD
  IF %SIGN_APP% == 1 CALL :SIGNAPP
  IF %CREATE_PORTABLE% == 1 CALL :CREATEPORTABLEDISTRO
) ELSE (
  REM UNIVERSAL BUILD
  ECHO.
  ECHO - Executing universal package build...
  FOR %%A IN ( x86_64, x86 ) DO (
    SET LP3D_ARCH=%%A
    SET PKG_DISTRO_DIR=%LP3D_PRODUCT%_%%A
    ECHO.
    ECHO - Configuring %LP3D_PRODUCT% %%A build...
    ECHO.
    ECHO   LP3D_ARCH......................[%%A]
    ECHO   PKG_DISTRO_DIR.................[%LP3D_PRODUCT%_%%A]
    CALL :DOWNLOADMSVCREDIST %%A
    CALL :COPYFILES
  )
  IF %RUN_NSIS% == 1 CALL :DOWNLOADLDRAWLIBS
  IF %RUN_NSIS% == 1 CALL :GENERATENSISPARAMS
  IF %RUN_NSIS% == 1 CALL :NSISBUILD
  IF %SIGN_APP% == 1 CALL :SIGNAPP
  FOR %%A IN ( x86_64, x86 ) DO (
    SET LP3D_ARCH=%%A
    SET PKG_DISTRO_DIR=%LP3D_PRODUCT%_%%A
    SET PKG_DISTRO_PORTABLE_DIR=%LP3D_PRODUCT%_%%A-%LP3D_APP_VERSION_LONG%
    IF %CREATE_PORTABLE% == 1 CALL :CREATEPORTABLEDISTRO
  )
)

CALL :GENERATE_JSON

IF %AUTO% NEQ 1 (
  CALL :POSTPROCESS
)

GOTO :END

:COPYFILES
REM Product Full Version Format:
REM Product _ Version . Revision . Build _ Date YYYYMMDD
REM LPub3D  _ 2.0.20  . 106      . 752   _ 20170929

REM Directory Structure Format:
REM Windows                                                            - builds/windows
REM   |_ Release                                                       - windows/release
REM        |_PRODUCT_DIR          = LP3D_PRODUCT LP3D_APP_VERSION_LONG - LPub3D_2.0.20.106.752_20170929
REM            |_PKG_DIST_DIR     = LP3D_PRODUCT LP3D_ARCH             - LPub3D_x86_64
REM            |_PKG_DOWNLOAD_DIR = PRODUCT_Download                   - LPub3D_Download
REM            |_PKG_UPDATE_DIR   = PRODUCT_Update                     - LPub3D_Update
IF NOT EXIST "%PKG_DISTRO_DIR%" (
  ECHO.
  ECHO * Did not find distribution package directory.
  ECHO * Expected "%PKG_DISTRO_DIR%" at "%cd%".
  ECHO * Build "%LP3D_PRODUCT%_%LP3D_APP_VERSION_LONG%" will terminate!
  EXIT /b 5
)

ECHO.
ECHO - Copying change_log_%LP3D_VERSION% to media folder...

REM pwd = windows/release/LP3D_PRODUCT_DIR
COPY /V /Y %PKG_DISTRO_DIR%\docs\README.txt %PKG_UPDATE_DIR%\change_log.txt /A | findstr /i /v /r /c:"copied\>"
COPY /V /Y %PKG_DISTRO_DIR%\docs\README.txt %PKG_UPDATE_DIR%\change_log_%LP3D_VERSION%.txt /A | findstr /i /v /r /c:"copied\>"
COPY /V /Y %PKG_DISTRO_DIR%\docs\README.txt %PKG_DOWNLOAD_DIR%\ /A | findstr /i /v /r /c:"copied\>"

ECHO.
ECHO - Copying release_notes_%LP3D_VERSION% to media folder...

COPY /V /Y %PKG_DISTRO_DIR%\docs\RELEASE_NOTES.html %PKG_UPDATE_DIR%\release_notes.html /A | findstr /i /v /r /c:"copied\>"
COPY /V /Y %PKG_DISTRO_DIR%\docs\RELEASE_NOTES.html %PKG_UPDATE_DIR%\release_notes_%LP3D_VERSION%.%LP3D_VER_REVISION%.html /A | findstr /i /v /r /c:"copied\>"
COPY /V /Y %PKG_DISTRO_DIR%\docs\RELEASE_NOTES.html %PKG_DOWNLOAD_DIR%\release_notes.html /A | findstr /i /v /r /c:"copied\>"

IF EXIST %PKG_DISTRO_DIR%\docs\COPYING_BRIEF (
  ECHO.
  ECHO - Set license file name...

  MOVE /Y %PKG_DISTRO_DIR%\docs\COPYING_BRIEF %PKG_DISTRO_DIR%\docs\COPYING.txt | findstr /i /v /r /c:"moved\>"
)
EXIT /b

REM AppVersion.nsh pwd = builds\utilities\nsis-scripts
:GENERATENSISPARAMS
ECHO.
ECHO - Generating AppVersion.nsh build parameters script...

SET versionFile=..\..\..\utilities\nsis-scripts\AppVersion.nsh
SET genVersion=%versionFile% ECHO

:GENERATE AppVersion.nsh NSIS build parameters file
>%genVersion% !define IconDir "%LP3D_ICON_DIR%"
>>%genVersion% ; ${IconDir}
>>%genVersion%.
>>%genVersion% !define Company %LP3D_COMPANY%
>>%genVersion% ; ${Company}
>>%genVersion%.
>>%genVersion% !define ProductName "%LP3D_PRODUCT%"
>>%genVersion% ; ${ProductName}
>>%genVersion%.
>>%genVersion% !define Version "%LP3D_VERSION%"
>>%genVersion% ; ${Version}
>>%genVersion%.
>>%genVersion% !define ReleaseNotes release_notes_%LP3D_VERSION%.%LP3D_VER_REVISION%.html
>>%genVersion% ; ${ReleaseNotes}
>>%genVersion%.
>>%genVersion% !define VersionMajor "%LP3D_VER_MAJOR%"
>>%genVersion% ; ${VersionMajor}
>>%genVersion%.
>>%genVersion% !define VersionMinor "%LP3D_VER_MINOR%"
>>%genVersion% ; ${VersionMinor}
>>%genVersion%.
>>%genVersion% !define CompleteVersion "%LP3D_VERSION%.%LP3D_VER_REVISION%.%LP3D_VER_BUILD%_%LP3D_YEAR%%LP3D_MONTH%%LP3D_DAY%"
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
>>%genVersion% !define ProductDir "..\..\windows\release\%LP3D_PRODUCT_DIR%"
>>%genVersion% ; ${ProductDir}
>>%genVersion%.
>>%genVersion% ; OutFile Dir
>>%genVersion% !define OutFileDir "..\..\windows\release\%LP3D_PRODUCT_DIR%"
>>%genVersion% ; ${OutFileDir}
>>%genVersion%.
>>%genVersion% ; InFile Dir:
IF %UNIVERSAL_BUILD% EQU 1 (
>>%genVersion%.
>>%genVersion% !define WinBuildDir "..\..\windows\release\%LP3D_PRODUCT_DIR%\%LP3D_PRODUCT%_x86_64"
>>%genVersion% ; ${WinBuildDir} - non-binary file location - using x86_64 loc for each arch
>>%genVersion%.
>>%genVersion% !define Win64BuildDir "..\..\windows\release\%LP3D_PRODUCT_DIR%\%LP3D_PRODUCT%_x86_64"
>>%genVersion% ; ${Win64BuildDir}
>>%genVersion%.
>>%genVersion% !define Win32BuildDir "..\..\windows\release\%LP3D_PRODUCT_DIR%\%LP3D_PRODUCT%_x86"
>>%genVersion% ; ${Win32BuildDir}
) ELSE (
>>%genVersion%.
>>%genVersion% !define WinBuildDir "..\..\windows\release\%LP3D_PRODUCT_DIR%\%PKG_DISTRO_DIR%"
>>%genVersion% ; ${WinBuildDir}
)
>>%genVersion%.
>>%genVersion% !define LPub3DBuildFile "%LPUB3D_BUILD_FILE%"
>>%genVersion% ; ${LPub3DBuildFile}
>>%genVersion%.
>>%genVersion% !define LDGliteDir "%VER_LDGLITE%"
>>%genVersion% ; ${LDGliteDir}
>>%genVersion%.
>>%genVersion% !define LDViewDir "%VER_LDVIEW%"
>>%genVersion% ; ${LDViewDir}
>>%genVersion%.
>>%genVersion% !define LPub3D_TraceDir "%VER_LPUB3D_TRACE%"
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
>>%genVersion% !define BuildRevision "%LP3D_VER_REVISION%"
>>%genVersion% ; ${BuildRevision}
>>%genVersion%.
>>%genVersion% !define BuildNumber "%LP3D_VER_BUILD%"
>>%genVersion% ; ${BuildNumber}
>>%genVersion%.
>>%genVersion% !define BuildDate "%LP3D_DATE_TIME%"
>>%genVersion% ; ${BuildDate}
>>%genVersion%.
>>%genVersion% !define Publisher %LP3D_PUBLISHER%
>>%genVersion% ; ${Publisher}
>>%genVersion%.
>>%genVersion% !define CompanyURL %LP3D_COMPANY_URL%
>>%genVersion% ; ${CompanyURL}
>>%genVersion%.
>>%genVersion% !define Comments %LP3D_COMMENTS%
>>%genVersion% ; ${Comments}
>>%genVersion%.
>>%genVersion% !define SupportURL %LP3D_SUPPORT%
>>%genVersion% ; ${SupportURL}
>>%genVersion%.
EXIT /b

REM pwd = windows/release/LP3D_PRODUCT_DIR
:NSISBUILD
IF %RUN_NSIS% == 1 ECHO.
IF %RUN_NSIS% == 1 ECHO - Start NSIS Master Installer Build...

IF %RUN_NSIS% == 0 ECHO.
IF %RUN_NSIS% == 0 ECHO - Ignore NSIS Master Installer Build

IF %RUN_NSIS% == 1 %NSISExe% /DDownloadMaster ..\..\..\utilities\nsis-scripts\LPub3DNoPack.nsi | findstr /i /r /c:"^Processing\>" /c:"^Output\>"

IF %RUN_NSIS% == 1 MOVE /Y    %LP3D_DOWNLOAD_PRODUCT%.exe %PKG_DOWNLOAD_DIR%\ | findstr /i /v /r /c:"moved\>"

IF %RUN_NSIS% == 1 ECHO   Finished NSIS Master Installer Build
EXIT /b

:SIGNAPP
IF %SIGN_APP% == 1 ECHO.
IF %SIGN_APP% == 1 ECHO - Start Application Code Signing...

IF %SIGN_APP% == 0 ECHO.
IF %SIGN_APP% == 0 ECHO - Ignore Application Code Signing

IF %SIGN_APP% == 1 %SignToolExe% sign /tr %TimeStamp% /td %Sha2% /fd %Sha2% /f %PKey% /p %PwD% %PKG_DOWNLOAD_DIR%\%LP3D_DOWNLOAD_PRODUCT%.exe
IF %UNIVERSAL_BUILD% EQU 1 (
  IF %SIGN_APP% == 1 %SignToolExe% sign /tr %TimeStamp% /td %Sha2% /fd %Sha2% /f %PKey% /p %PwD% %LP3D_PRODUCT%_x86\%LPUB3D_BUILD_FILE%
  IF %SIGN_APP% == 1 %SignToolExe% sign /tr %TimeStamp% /td %Sha2% /fd %Sha2% /f %PKey% /p %PwD% %LP3D_PRODUCT%_x86_64\%LPUB3D_BUILD_FILE%
) ELSE (
  IF %SIGN_APP% == 1 %SignToolExe% sign /tr %TimeStamp% /td %Sha2% /fd %Sha2% /f %PKey% /p %PwD% %PKG_DISTRO_DIR%\%LPUB3D_BUILD_FILE%
)

IF %SIGN_APP% == 1 ECHO.
IF %SIGN_APP% == 1 ECHO - Generating Code Signing Hash Checksum listing...

IF %SIGN_APP% == 1 CertUtil -hashfile %PKG_DOWNLOAD_DIR%\%LP3D_DOWNLOAD_PRODUCT%.exe SHA256                     >  %PKG_DOWNLOAD_DIR%\LPub3D.%LP3D_VERSION%.Checksums.txt
IF %UNIVERSAL_BUILD% EQU 1 (
  IF %SIGN_APP% == 1 CertUtil -hashfile %LP3D_PRODUCT%_x86\%LPUB3D_BUILD_FILE%                                 >>  %PKG_DOWNLOAD_DIR%\LPub3D.%LP3D_VERSION%.Checksums.txt
  IF %SIGN_APP% == 1 CertUtil -hashfile %LP3D_PRODUCT%_x86_64\%LPUB3D_BUILD_FILE%                              >>  %PKG_DOWNLOAD_DIR%\LPub3D.%LP3D_VERSION%.Checksums.txt
) ELSE (
  IF %SIGN_APP% == 1 CertUtil -hashfile %PKG_DISTRO_DIR%\%LPUB3D_BUILD_FILE% SHA256                            >>  %PKG_DOWNLOAD_DIR%\LPub3D.%LP3D_VERSION%.Checksums.txt
)

IF %SIGN_APP% == 1 ECHO   Finished Application Code Signing
EXIT /b

:CREATEPORTABLEDISTRO
IF %CREATE_PORTABLE% == 1 ECHO.
IF %CREATE_PORTABLE% == 1 ECHO - Create %LP3D_PRODUCT% %LP3D_ARCH% portable install archive package file...

IF %CREATE_PORTABLE% == 0 ECHO.
IF %CREATE_PORTABLE% == 0 ECHO - Ignore creating %LP3D_PRODUCT% portable install archive package file

IF %CREATE_PORTABLE% == 1 %zipExe% a -tzip %PKG_DOWNLOAD_DIR%\%PKG_DISTRO_PORTABLE_DIR%.zip %PKG_DISTRO_DIR%\ | findstr /i /r /c:"^Creating\>" /c:"^Everything\>"
EXIT /b

:GENERATE_JSON
ECHO.
ECHO - Generating update package alternate version json inserts...
SET LP3D_ARCH=x86_64
SET LP3D_AMDARCH=amd64
SET LP3D_DIST_EXTENSIONS=exe, dmg, deb, rpm, pkg, api
FOR %%e IN ( %LP3D_DIST_EXTENSIONS% ) DO (
 CALL :GENERATE_ALT_VERSION_INSERTS %%e
)

ECHO.
ECHO - Generating update package lpub3dupdates.json template file...

SET updatesFile=%PKG_UPDATE_DIR%\lpub3dupdates.json
SET genLPub3DUpdates=%updatesFile% ECHO

:GENERATE lpub3dupdates.json template file
>%genLPub3DUpdates% {
>>%genLPub3DUpdates%   "_comment": "LPub3D lpub3dupdates.json generated on %LP3D_DATE_TIME%",
>>%genLPub3DUpdates%   "updates": {
>>%genLPub3DUpdates%     "windows-exe": {
>>%genLPub3DUpdates%       "open-url": "%LP3D_GITHUB_BASE%/releases/tag/%LP3D_VER_TAG_NAME%/",
>>%genLPub3DUpdates%       "latest-version": "%LP3D_VERSION%",
>>%genLPub3DUpdates%       "latest-revision": "%LP3D_VER_REVISION%",
>>%genLPub3DUpdates%       "download-url": "%LP3D_GITHUB_BASE%/releases/download/%LP3D_VER_TAG_NAME%/LPub3D-%LP3D_APP_VERSION_LONG%.exe",
>>%genLPub3DUpdates%       "x86-win-portable-download-url": "%LP3D_GITHUB_BASE%/releases/download/%LP3D_VER_TAG_NAME%/LPub3D_x86-%LP3D_APP_VERSION_LONG%.zip",
>>%genLPub3DUpdates%       "x86_64-win-portable-download-url": "%LP3D_GITHUB_BASE%/releases/download/%LP3D_VER_TAG_NAME%/LPub3D_x86_64-%LP3D_APP_VERSION_LONG%.zip",
>>%genLPub3DUpdates%       "changelog-url": "%LP3D_SOURCEFORGE_UPDATE_BASE%/release_notes_%LP3D_VERSION%.%LP3D_VER_REVISION%.html",
>>%genLPub3DUpdates%       "available-versions": "%LP3D_AVAILABLE_VERSIONS_exe%",
>>%genLPub3DUpdates%       "alt-version-gen-placeholder-windows-exe": {}
>>%genLPub3DUpdates%     },
>>%genLPub3DUpdates%     "windows": {
>>%genLPub3DUpdates%       "open-url": "%LP3D_GITHUB_BASE%/releases/tag/%LP3D_VER_TAG_NAME%/",
>>%genLPub3DUpdates%       "latest-version": "%LP3D_VERSION%",
>>%genLPub3DUpdates%       "download-url": "%LP3D_GITHUB_BASE%/releases/download/%LP3D_VER_TAG_NAME%/LPub3D-%LP3D_APP_VERSION_LONG%.exe",
>>%genLPub3DUpdates%       "changelog-url": "%LP3D_GITHUB_BASE%/releases/download/%LP3D_VER_TAG_NAME%/release_notes.html",
>>%genLPub3DUpdates%       "download-url-": "%LP3D_GITHUB_BASE%/releases/download/%LP3D_VER_TAG_NAME%/LPub3D-%LP3D_APP_VERSION_LONG%.exe",
>>%genLPub3DUpdates%       "changelog-url-": "%LP3D_SOURCEFORGE_UPDATE_BASE%/release_notes_%LP3D_VERSION%.%LP3D_VER_REVISION%.html",
>>%genLPub3DUpdates%       "available-versions": "%LP3D_AVAILABLE_VERSIONS_exe%",
>>%genLPub3DUpdates%       "alt-version-gen-placeholder-windows": {}
>>%genLPub3DUpdates%     },
>>%genLPub3DUpdates%     "macos-dmg": {
>>%genLPub3DUpdates%       "open-url": "%LP3D_GITHUB_BASE%/releases/tag/%LP3D_VER_TAG_NAME%/",
>>%genLPub3DUpdates%       "latest-version": "%LP3D_VERSION%",
>>%genLPub3DUpdates%       "latest-revision": "%LP3D_VER_REVISION%",
>>%genLPub3DUpdates%       "download-url": "%LP3D_GITHUB_BASE%/releases/download/%LP3D_VER_TAG_NAME%/LPub3D-%LP3D_APP_VERSION_LONG%-macos.dmg",
>>%genLPub3DUpdates%       "changelog-url": "%LP3D_SOURCEFORGE_UPDATE_BASE%/release_notes_%LP3D_VERSION%.%LP3D_VER_REVISION%.html",
>>%genLPub3DUpdates%       "available-versions": "%LP3D_AVAILABLE_VERSIONS_dmg%",
>>%genLPub3DUpdates%       "alt-version-gen-placeholder-macos-dmg": {}
>>%genLPub3DUpdates%     },
>>%genLPub3DUpdates%     "linux-deb": {
>>%genLPub3DUpdates%       "open-url": "%LP3D_GITHUB_BASE%/releases/tag/%LP3D_VER_TAG_NAME%/",
>>%genLPub3DUpdates%       "latest-version": "%LP3D_VERSION%",
>>%genLPub3DUpdates%       "latest-revision": "%LP3D_VER_REVISION%",
>>%genLPub3DUpdates%       "download-url": "%LP3D_GITHUB_BASE%/releases/download/%LP3D_VER_TAG_NAME%/LPub3D-%LP3D_APP_VERSION_LONG%-bionic-%LP3D_AMDARCH%.deb",
>>%genLPub3DUpdates%       "changelog-url": "%LP3D_SOURCEFORGE_UPDATE_BASE%/release_notes_%LP3D_VERSION%.%LP3D_VER_REVISION%.html",
>>%genLPub3DUpdates%       "available-versions": "%LP3D_AVAILABLE_VERSIONS_deb%",
>>%genLPub3DUpdates%       "alt-version-gen-placeholder-linux-deb": {}
>>%genLPub3DUpdates%     },
>>%genLPub3DUpdates%     "linux-rpm": {
>>%genLPub3DUpdates%       "open-url": "%LP3D_GITHUB_BASE%/releases/tag/%LP3D_VER_TAG_NAME%/",
>>%genLPub3DUpdates%       "latest-version": "%LP3D_VERSION%",
>>%genLPub3DUpdates%       "latest-revision": "%LP3D_VER_REVISION%",
>>%genLPub3DUpdates%       "download-url": "%LP3D_GITHUB_BASE%/releases/download/%LP3D_VER_TAG_NAME%/LPub3D-%LP3D_APP_VERSION_LONG%-1.fc26.%LP3D_ARCH%.rpm",
>>%genLPub3DUpdates%       "changelog-url": "%LP3D_SOURCEFORGE_UPDATE_BASE%/release_notes_%LP3D_VERSION%.%LP3D_VER_REVISION%.html",
>>%genLPub3DUpdates%       "available-versions": "%LP3D_AVAILABLE_VERSIONS_deb%",
>>%genLPub3DUpdates%       "alt-version-gen-placeholder-linux-rpm": {}
>>%genLPub3DUpdates%     },
>>%genLPub3DUpdates%     "linux-pkg": {
>>%genLPub3DUpdates%       "open-url": "%LP3D_GITHUB_BASE%/releases/tag/%LP3D_VER_TAG_NAME%/",
>>%genLPub3DUpdates%       "latest-version": "%LP3D_VERSION%",
>>%genLPub3DUpdates%       "latest-revision": "%LP3D_VER_REVISION%",
>>%genLPub3DUpdates%       "download-url": "%LP3D_GITHUB_BASE%/releases/download/%LP3D_VER_TAG_NAME%/LPub3D-%LP3D_APP_VERSION_LONG%-%LP3D_ARCH%.pkg.tar.xz",
>>%genLPub3DUpdates%       "changelog-url": "%LP3D_SOURCEFORGE_UPDATE_BASE%/release_notes_%LP3D_VERSION%.%LP3D_VER_REVISION%.html",
>>%genLPub3DUpdates%       "available-versions": "%LP3D_AVAILABLE_VERSIONS_deb%",
>>%genLPub3DUpdates%       "alt-version-gen-placeholder-linux-pkg": {}
>>%genLPub3DUpdates%     },
>>%genLPub3DUpdates%     "linux-api": {
>>%genLPub3DUpdates%       "open-url": "%LP3D_GITHUB_BASE%/releases/tag/%LP3D_VER_TAG_NAME%/",
>>%genLPub3DUpdates%       "latest-version": "%LP3D_VERSION%",
>>%genLPub3DUpdates%       "latest-revision": "%LP3D_VER_REVISION%",
>>%genLPub3DUpdates%       "download-url": "%LP3D_GITHUB_BASE%/releases/download/%LP3D_VER_TAG_NAME%/LPub3D-%LP3D_APP_VERSION_LONG%-%LP3D_ARCH%.AppImage",
>>%genLPub3DUpdates%       "changelog-url": "%LP3D_SOURCEFORGE_UPDATE_BASE%/release_notes_%LP3D_VERSION%.%LP3D_VER_REVISION%.html",
>>%genLPub3DUpdates%       "available-versions": "%LP3D_AVAILABLE_VERSIONS_api%",
>>%genLPub3DUpdates%       "alt-version-gen-placeholder-linux-api": {}
>>%genLPub3DUpdates%     }
>>%genLPub3DUpdates%   }
>>%genLPub3DUpdates% }
>>%genLPub3DUpdates%.

ECHO.
ECHO - Merging update package version inserts into lpub3dupdates.json...

(
  FOR /F "tokens=*" %%i IN (%PKG_UPDATE_DIR%\lpub3dupdates.json) DO (
    IF "%%i" EQU ""alt-version-gen-placeholder-windows": {}" (
      TYPE %PKG_UPDATE_DIR%\versionInsert_exe.txt
    )
    IF "%%i" EQU ""alt-version-gen-placeholder-windows-exe": {}" (
      TYPE %PKG_UPDATE_DIR%\versionInsert_exe.txt
    )
    IF "%%i" EQU ""alt-version-gen-placeholder-macos-dmg": {}" (
      TYPE %PKG_UPDATE_DIR%\versionInsert_dmg.txt
    )
    IF "%%i" EQU ""alt-version-gen-placeholder-linux-deb": {}" (
      TYPE %PKG_UPDATE_DIR%\versionInsert_deb.txt
    )
    IF "%%i" EQU ""alt-version-gen-placeholder-linux-rpm": {}" (
      TYPE %PKG_UPDATE_DIR%\versionInsert_rpm.txt
    )
    IF "%%i" EQU ""alt-version-gen-placeholder-linux-pkg": {}" (
      TYPE %PKG_UPDATE_DIR%\versionInsert_pkg.txt
    )
    IF "%%i" EQU ""alt-version-gen-placeholder-linux-api": {}" (
      TYPE %PKG_UPDATE_DIR%\versionInsert_api.txt
    )
  ECHO %%i
  )
) >temp.txt
MOVE /y temp.txt %PKG_UPDATE_DIR%\lpub3dupdates.json | findstr /i /v /r /c:"moved\>"
FOR %%e IN ( %LP3D_DIST_EXTENSIONS% ) DO (
  DEL /Q %PKG_UPDATE_DIR%\versionInsert_%%e.txt
)

ECHO.
ECHO - Copying additional json FILES to media folder...

REM pwd = windows\release\LP3D_PRODUCT_DIR
COPY /V /Y ..\..\..\utilities\json\complete.json %PKG_UPDATE_DIR%\ /A | findstr /i /v /r /c:"copied\>"
COPY /V /Y ..\..\..\utilities\json\lpub3dldrawunf.json %PKG_UPDATE_DIR%\ /A | findstr /i /v /r /c:"copied\>"

ECHO.
ECHO - Generating latest.txt version input file (for backward compatability)...

SET latestFile=%PKG_UPDATE_DIR%\latest.txt
SET genLatest=%latestFile% ECHO

:GENERATE latest.txt file
>%genLatest% %LP3D_VERSION%
EXIT /b

:GENERATE_ALT_VERSION_INSERTS
SET "LP3D_EXT=%1"
SET "exe=.%LP3D_EXT%"
SET "dmg=_osx.%LP3D_EXT%"
SET "deb=_0ubuntu1_%LP3D_AMDARCH%.%LP3D_EXT%"
SET "rpm=_1fedora.%LP3D_ARCH%.%LP3D_EXT%"
SET "pkg=.645_%LP3D_ARCH%.%LP3D_EXT%.tar.xz"
SET "api=-%LP3D_ARCH%.AppImage"
SET "LP3D_ALT_VERS=LP3D_ALTERNATE_VERSIONS_%LP3D_EXT%"
REM LP3D_DIST_SUFFIX expands to the LP3D_EXTension variable
CALL SET "LP3D_DIST_SUFFIX=%%%LP3D_EXT%%%"
REM LP3D_ALT_VERS expands to the alternate version for a given LP3D_EXTension
CALL SET "LP3D_ALTERNATE_VERSIONS=%%%LP3D_ALT_VERS%%%"
IF "%1" EQU "api" (
  SET "LP3D_DIST_PREFIX=LPub3D-"
) ELSE (
  SET "LP3D_DIST_PREFIX=LPub3D-UpdateMaster_"
)
SET versionInsert=%PKG_UPDATE_DIR%\versionInsert_%LP3D_EXT%.txt
SET genVersionInsert=%versionInsert% ECHO
SETLOCAL ENABLEDELAYEDEXPANSION
FOR %%V IN ( %LP3D_ALTERNATE_VERSIONS% ) DO (
  IF "%%V" LEQ "2.0.20" (
    >>%genVersionInsert% "alternate-version-%%V-%LP3D_EXT%": {
    >>%genVersionInsert%   "open-url": "%LP3D_SOURCEFORGE_OPEN_BASE%/projects/lpub3d/files/%%V/",
    >>%genVersionInsert%   "latest-version": "%%V",
    >>%genVersionInsert%   "download-url": "%LP3D_SOURCEFORGE_UPDATE_BASE%/%LP3D_DIST_PREFIX%%%V%LP3D_DIST_SUFFIX%",
    IF "%%V" EQU "2.0.20" (
      IF "%1" EQU "exe" (
        SET LP3D_ALT_VERSION_LONG=2.0.20.0.645_20170208
        >>%genVersionInsert%   "x86-win-portable-download-url": "%LP3D_GITHUB_BASE%/releases/download/v%%V/LPub3D_x86-!LP3D_ALT_VERSION_LONG!.zip",
        >>%genVersionInsert%   "x86_64-win-portable-download-url": "%LP3D_GITHUB_BASE%/releases/download/v%%V/LPub3D_x86_64-!LP3D_ALT_VERSION_LONG!.zip",
      )
    )
    >>%genVersionInsert%   "changelog-url": "%LP3D_SOURCEFORGE_UPDATE_BASE%/change_log_%%V.txt"
    >>%genVersionInsert% },
  ) ELSE (
    SET LP3D_ALT_VERSION_LONG=2.3.6.0.1101_20181218
    IF "%1" EQU "exe" (
      SET LP3D_DIST_SUFFIX=.exe
    )
    IF "%1" EQU "dmg" (
      SET LP3D_DIST_SUFFIX=-macos.dmg
    )
    IF "%1" EQU "deb" (
      SET LP3D_DIST_SUFFIX=-bionic-%LP3D_AMDARCH%.deb
    )
    IF "%1" EQU "rpm" (
      SET LP3D_DIST_SUFFIX=-1.fc26.%LP3D_ARCH%.rpm
    )
    IF "%1" EQU "pkg" (
      SET LP3D_DIST_SUFFIX=-%LP3D_ARCH%.pkg.tar.xz
    )
    IF "%1" EQU "api" (
      SET LP3D_DIST_SUFFIX=-%LP3D_ARCH%.AppImage
    )
    >>%genVersionInsert% "alternate-version-%%V-%LP3D_EXT%": {
    >>%genVersionInsert%   "open-url": "%LP3D_GITHUB_BASE%/releases/tag/v%%V/",
    >>%genVersionInsert%   "latest-version": "%%V",
    >>%genVersionInsert%   "download-url": "%LP3D_GITHUB_BASE%/releases/download/v%%V/LPub3D-!LP3D_ALT_VERSION_LONG!!LP3D_DIST_SUFFIX!",
    IF "%1" EQU "exe" (
      >>%genVersionInsert%   "x86-win-portable-download-url": "%LP3D_GITHUB_BASE%/releases/download/v%%V/LPub3D_x86-!LP3D_ALT_VERSION_LONG!.zip",
      >>%genVersionInsert%   "x86_64-win-portable-download-url": "%LP3D_GITHUB_BASE%/releases/download/v%%V/LPub3D_x86_64-!LP3D_ALT_VERSION_LONG!.zip",
    )
    >>%genVersionInsert%   "changelog-url": "%LP3D_SOURCEFORGE_UPDATE_BASE%/release_notes_%%V.html"
    >>%genVersionInsert% },
  )
)
SETLOCAL DISABLEDELAYEDEXPANSION
ECHO   Generated %1 json version insert
EXIT /b

:DOWNLOADMSVCREDIST
ECHO.
ECHO - Download Microsoft Visual C++ 2015 %1 Redistributable Update 3...

IF /I "%APPVEYOR%" EQU "True" (
  SET DIST_DIR=%LP3D_DIST_DIR_PATH%
) ELSE (
  CALL :DIST_DIR_REL_TO_ABS ..\..\..\..\..\lpub3d_windows_3rdparty
)

IF NOT EXIST "%DIST_DIR%\" (
  ECHO.
  ECHO - ERROR - Could not locate distribution path [%DIST_DIR%]
  EXIT /b
)

SET MSVC_REDIST_DIR=%DIST_DIR%\vcredist

IF NOT EXIST "%MSVC_REDIST_DIR%\" (
  ECHO.
  ECHO - Create MSVC 2015 %1 Redistributable store %MSVC_REDIST_DIR%
  MKDIR "%MSVC_REDIST_DIR%\"
)

SET OutputPATH=%MSVC_REDIST_DIR%
SET RedistCONTENT=vcredist_%1.exe

CALL :CREATEWEBCONTENTDOWNLOADVBS

ECHO.
ECHO - MSVC 2015 %1 Redistributable download path: [%OutputPATH%]

IF NOT EXIST "%OutputPATH%\%RedistCONTENT%" (
  CALL :GET_MSVC_REDIST
)  ELSE (
  ECHO.
  ECHO - MSVC 2015 %1 Redistributable %RedistCONTENT% exist. Nothing to do.
)

CALL :SET_MSVC_REDIST %1
EXIT /b

:GET_MSVC_REDIST
SET WebCONTENT="%OutputPATH%\%RedistCONTENT%"
SET WebNAME=https://github.com/trevorsandy/lpub3d_libs/releases/download/v1.0.1/%RedistCONTENT%

ECHO.
ECHO - MSVC 2015 %1 Redistributable: %WebCONTENT%...
ECHO.
cscript //Nologo %TEMP%\$\%vbs% %WebNAME% %WebCONTENT% && @ECHO off
IF EXIST "%OutputPATH%\%RedistCONTENT%" (
  ECHO.
  ECHO - MSVC 2015 %1 Redistributable %RedistCONTENT% is availble
)
EXIT /b

:SET_MSVC_REDIST
SET PKG_TARGET_DIR=%WIN_PKG_DIR%\release\%LP3D_PRODUCT_DIR%\%LP3D_PRODUCT%_%1\vcredist
IF NOT EXIST "%PKG_TARGET_DIR%\" (
  ECHO.
  ECHO - Create MSVC 2015 %1 Redistributable package path %MSVC_REDIST_DIR%...
  MKDIR "%PKG_TARGET_DIR%\"
)

IF NOT EXIST "%PKG_TARGET_DIR%\%RedistCONTENT%" (
  ECHO.
  ECHO - Copy MSVC 2015 %1 Redistributable to %PKG_TARGET_DIR% folder...
  IF EXIST "%MSVC_REDIST_DIR%\%RedistCONTENT%" (
    COPY /V /Y "%MSVC_REDIST_DIR%\%RedistCONTENT%" "%PKG_TARGET_DIR%\" /A | findstr /i /v /r /c:"copied\>"
  ) ELSE (
    ECHO.
    ECHO -ERROR - MSVC 2015 %1 Redistributable %RedistCONTENT% does not exist in %MSVC_REDIST_DIR%\.
  )
) ELSE (
  ECHO.
  ECHO - MSVC 2015 %1 Redistributable %RedistCONTENT% exist in %PKG_TARGET_DIR%. Nothing to do.
)
EXIT /b

:DOWNLOADLDRAWLIBS
ECHO.
ECHO - Download LDraw archive libraries...

IF /I "%APPVEYOR%" EQU "True" (
  SET LDRAW_LIBS=%APPVEYOR_BUILD_FOLDER%\LDrawLibs
) ELSE (
  SET LDRAW_LIBS=%USERPROFILE%
)

IF NOT EXIST "%LDRAW_LIBS%\" (
  ECHO.
  ECHO - Create LDraw archive libs store %LDRAW_LIBS%
  MKDIR "%LDRAW_LIBS%\"
)

SET OutputPATH=%LDRAW_LIBS%
SET OfficialCONTENT=complete.zip
SET UnOfficialCONTENT=ldrawunf.zip
SET LPub3DCONTENT=lpub3dldrawunf.zip
SET TenteCONTENT=tenteparts.zip
SET VexiqCONTENT=vexiqparts.zip

CALL :CREATEWEBCONTENTDOWNLOADVBS

ECHO.
ECHO - LDraw archive library download path: %OutputPATH%

IF NOT EXIST "%OutputPATH%\%OfficialCONTENT%" (
  CALL :GET_OFFICIAL_LIBRARY
)  ELSE (
  ECHO.
  ECHO - LDraw archive library %OfficialCONTENT% exist. Nothing to do.
)
IF NOT EXIST "%OutputPATH%\%TenteCONTENT%" (
  CALL :GET_TENTE_LIBRARY
) ELSE (
  ECHO.
  ECHO - LDraw archive library %TenteCONTENT% exist. Nothing to do.
)
IF NOT EXIST "%OutputPATH%\%VexiqCONTENT%" (
  CALL :GET_VEXIQ_LIBRARY
) ELSE (
  ECHO.
  ECHO - LDraw archive library %VexiqCONTENT% exist. Nothing to do.
)
IF NOT EXIST "%OutputPATH%\%LPub3DCONTENT%" (
  CALL :GET_UNOFFICIAL_LIBRARY
) ELSE (
  ECHO.
  ECHO - LDraw archive library %UnOfficialCONTENT% exist. Nothing to do.
)

FOR %%A IN ( x86_64, x86 ) DO (
  CALL :SET_LDRAW_LIBRARIES %%A
)
EXIT /b

:GET_OFFICIAL_LIBRARY
SET WebCONTENT="%OutputPATH%\%OfficialCONTENT%"
SET WebNAME=http://www.ldraw.org/library/updates/%OfficialCONTENT%

ECHO.
ECHO - Download archive file: %WebCONTENT%...
ECHO.
cscript //Nologo %TEMP%\$\%vbs% %WebNAME% %WebCONTENT% && @ECHO off
IF EXIST "%OutputPATH%\%OfficialCONTENT%" (
  ECHO.
  ECHO - LDraw archive library %OfficialCONTENT% is availble
)
EXIT /b

:GET_TENTE_LIBRARY
SET WebCONTENT="%OutputPATH%\%TenteCONTENT%"
SET WebNAME=https://github.com/trevorsandy/lpub3d_libs/releases/download/v1.0.1/%TenteCONTENT%

ECHO.
ECHO - Download archive file: %WebCONTENT%...
ECHO.
cscript //Nologo %TEMP%\$\%vbs% %WebNAME% %WebCONTENT% && @ECHO off
IF EXIST "%OutputPATH%\%TenteCONTENT%" (
  ECHO.
  ECHO - LDraw archive library %TenteCONTENT% is availble
)
EXIT /b

:GET_VEXIQ_LIBRARY
SET WebCONTENT="%OutputPATH%\%VexiqCONTENT%"
SET WebNAME=https://github.com/trevorsandy/lpub3d_libs/releases/download/v1.0.1/%VexiqCONTENT%

ECHO.
ECHO - Download archive file: %WebCONTENT%...
ECHO.
cscript //Nologo %TEMP%\$\%vbs% %WebNAME% %WebCONTENT% && @ECHO off
IF EXIST "%OutputPATH%\%VexiqCONTENT%" (
  ECHO.
  ECHO - LDraw archive library %VexiqCONTENT% is availble
)
EXIT /b

:GET_UNOFFICIAL_LIBRARY
SET WebCONTENT="%OutputPATH%\%UnofficialCONTENT%"
SET WebNAME=http://www.ldraw.org/library/unofficial/%UnofficialCONTENT%

ECHO.
ECHO - Download archive file: %WebCONTENT%...
ECHO.
cscript //Nologo %TEMP%\$\%vbs% %WebNAME% %WebCONTENT% && @ECHO off
ECHO.
ECHO - Rename %WebCONTENT% to %LPub3DCONTENT%
REN "%WebCONTENT%" %LPub3DCONTENT%
IF EXIST "%OutputPATH%\%LPub3DCONTENT%" (
  ECHO.
  ECHO - LDraw archive library %LPub3DCONTENT% is availble
)
EXIT /b

:SET_LDRAW_LIBRARIES
SET PKG_TARGET_DIR=%WIN_PKG_DIR%\release\%LP3D_PRODUCT_DIR%\%LP3D_PRODUCT%_%1

ECHO.
ECHO - Copy LDraw archive libraries to %PKG_TARGET_DIR%\extras folder...

IF NOT EXIST "%PKG_TARGET_DIR%\extras\%OfficialCONTENT%" (
  IF EXIST "%LDRAW_LIBS%\%OfficialCONTENT%" (
    COPY /V /Y "%LDRAW_LIBS%\%OfficialCONTENT%" "%PKG_TARGET_DIR%\extras\" /A | findstr /i /v /r /c:"copied\>"
  ) ELSE (
    ECHO.
    ECHO -ERROR - LDraw archive lib %OfficialCONTENT% does not exist in %LDRAW_LIBS%\.
  )
) ELSE (
  ECHO.
  ECHO - Archive library %OfficialCONTENT% exist in %LP3D_PRODUCT%_%1\extras folder. Nothing to do.
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
  ECHO - Archive library %TenteCONTENT% exist in %LP3D_PRODUCT%_%1\extras folder. Nothing to do.
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
  ECHO - Archive library %VexiqCONTENT% exist in %LP3D_PRODUCT%_%1\extras folder. Nothing to do.
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
  ECHO - Archive library %LPub3DCONTENT% exist in %LP3D_PRODUCT%_%1\extras folder. Nothing to do.
)
EXIT /b

:CREATEWEBCONTENTDOWNLOADVBS
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

:WEB CONTENT SAVE-AS Download -- VBS
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
>>%t%   WScript.Echo "  The file may be in use by another process.", vbLF
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
EXIT /b

:DIST_DIR_REL_TO_ABS
IF [%1] EQU [] (EXIT /b) ELSE (SET REL_DIST_DIR=%1)
SET REL_DIST_DIR=%REL_DIST_DIR:/=\%
SET DIST_DIR=
PUSHD %REL_DIST_DIR%
SET DIST_DIR=%CD%
POPD
EXIT /b

:CREATE_LP3D_PS_VARS_FILE
ECHO.
ECHO - Create set_ps_vars.ps1 to add update-config-files environment variables to PowerShell...
SET set_ps_vars=%CD%\ci\set_ps_vars.ps1
SET genFile=%set_ps_vars% ECHO
>%genFile% # This script sets the update-config-files environment variables in Powershell
>>%genFile% #
>>%genFile% # From PowerShell scripts, run as follows:
>>%genFile% #
>>%genFile% #   Set-ExecutionPolicy remotesigned -scope process -force
>>%genFile% #   %set_ps_vars%
>>%genFile% #
>>%genFile% # From batch files, run as follows:
>>%genFile% #
>>%genFile% #   powershell -executionpolicy remotesigned -File %set_ps_vars%
>>%genFile% #
>>%genFile% # Both procedures will cause the following envionment variables to be set:
>>%genFile%.
>>%genFile% $env:LP3D_SOURCE_DIR = "%LP3D_SOURCE_DIR%"
>>%genFile% $env:LP3D_CALL_DIR = "%LP3D_CALL_DIR%"
>>%genFile% $env:LP3D_DAY = "%LP3D_DAY%"
>>%genFile% $env:LP3D_MONTH = "%LP3D_MONTH%"
>>%genFile% $env:LP3D_YEAR = "%LP3D_YEAR%"
>>%genFile% $env:LP3D_HOUR = "%LP3D_HOUR%"
>>%genFile% $env:LP3D_MIN = "%LP3D_MIN%"
>>%genFile% $env:LP3D_SEC = "%LP3D_SEC%"
>>%genFile% $env:LP3D_TIME = "%LP3D_TIME%"
>>%genFile% $env:LP3D_WEEK_DAY = "%LP3D_WEEK_DAY%"
>>%genFile% $env:LP3D_MONTH_OF_YEAR = "%LP3D_MONTH_OF_YEAR%"
>>%genFile% $env:LP3D_VER_MAJOR = "%LP3D_VER_MAJOR%"
>>%genFile% $env:LP3D_VER_MINOR = "%LP3D_VER_MINOR%"
>>%genFile% $env:LP3D_VER_PATCH = "%LP3D_VER_PATCH%"
>>%genFile% $env:LP3D_VER_REVISION = "%LP3D_VER_REVISION%"
>>%genFile% $env:LP3D_VER_BUILD = "%LP3D_VER_BUILD%"
>>%genFile% $env:LP3D_VER_SHA_HASH = "%LP3D_VER_SHA_HASH%"
IF [%LP3D_VER_SUFFIX%] NEQ [] (
  >>%genFile% $env:LP3D_VER_SUFFIX = "%LP3D_VER_SUFFIX%"
)
>>%genFile% $env:LP3D_VERSION = "%LP3D_VERSION%"
>>%genFile% $env:LP3D_APP_VERSION = "%LP3D_APP_VERSION%"
>>%genFile% $env:LP3D_APP_VERSION_TAG = "%LP3D_APP_VERSION_TAG%"
>>%genFile% $env:LP3D_APP_VER_SUFFIX = "%LP3D_APP_VER_SUFFIX%"
>>%genFile% $env:LP3D_APP_VERSION_LONG = "%LP3D_APP_VERSION_LONG%"
>>%genFile% $env:LP3D_BUILD_VERSION = "%LP3D_BUILD_VERSION%"
>>%genFile% $env:LP3D_VERSION_INFO = "%LP3D_VERSION_INFO%"
>>%genFile% $env:LP3D_BUILD_DATE_TIME = "%LP3D_BUILD_DATE_TIME%"
>>%genFile% $env:LP3D_CHANGE_DATE_LONG = "%LP3D_CHANGE_DATE_LONG%"
>>%genFile% $env:LP3D_AVAILABLE_VERSIONS = "%LP3D_AVAILABLE_VERSIONS%"
>>%genFile% $env:LP3D_BUILD_PACKAGE = "${env:LP3D_PACKAGE}-Any-${env:LP3D_APP_VERSION_LONG}"
>>%genFile% $env:LP3D_BUILD_TARGET = "${env:LP3D_PACKAGE_PATH}\${env:LP3D_BUILD_PACKAGE}"
>>%genFile% $env:LP3D_DOWNLOAD_ASSETS = "${env:LP3D_BUILD_TARGET}\${env:LP3D_PACKAGE}_Download"
>>%genFile% $env:LP3D_UPDATE_ASSETS = "${env:LP3D_BUILD_TARGET}\${env:LP3D_PACKAGE}_Update"
>>%genFile% write-host "`n- Update-config-files environment variables set in Powershell"
IF EXIST "%set_ps_vars%" (
  ECHO   FILE set_ps_vars.ps1...........[written to %set_ps_vars%]
) ELSE (
  ECHO   FILE set_ps_vars.ps1...........[ERROR - file %set_ps_vars% not found]
)
EXIT /b 0

:CREATE_LP3D_BASH_VARS_FILE
ECHO.
ECHO - Create set_bash_vars.sh to add update-config-files environment variables to Bash...
SET LP3D_PACKAGE_PATH_BASH=%LP3D_PACKAGE_PATH:\=/%
SET set_bash_vars=%CD%\ci\set_bash_vars.sh
SET genFile=%set_bash_vars% ECHO
>%genFile% #!/bin/bash
>>%genFile% #
>>%genFile% # This script sets the update-config-files environment variables in Bash
>>%genFile% #
>>%genFile% #  Trevor SANDY ^<trevor.sandy@gmail.com^>
>>%genFile% #  Last Update: March 06, 2019
>>%genFile% #  Copyright ^(c^) 2020 by Trevor SANDY
>>%genFile%.
>>%genFile% export LP3D_SOURCE_DIR="%LP3D_SOURCE_DIR%"
>>%genFile% export LP3D_CALL_DIR="%LP3D_CALL_DIR%"
>>%genFile% export LP3D_DAY="%LP3D_DAY%"
>>%genFile% export LP3D_MONTH="%LP3D_MONTH%"
>>%genFile% export LP3D_YEAR="%LP3D_YEAR%"
>>%genFile% export LP3D_HOUR="%LP3D_HOUR%"
>>%genFile% export LP3D_MIN="%LP3D_MIN%"
>>%genFile% export LP3D_SEC="%LP3D_SEC%"
>>%genFile% export LP3D_TIME="%LP3D_TIME%"
>>%genFile% export LP3D_WEEK_DAY="%LP3D_WEEK_DAY%"
>>%genFile% export LP3D_MONTH_OF_YEAR="%LP3D_MONTH_OF_YEAR%"
>>%genFile% export LP3D_VER_MAJOR="%LP3D_VER_MAJOR%"
>>%genFile% export LP3D_VER_MINOR="%LP3D_VER_MINOR%"
>>%genFile% export LP3D_VER_PATCH="%LP3D_VER_PATCH%"
>>%genFile% export LP3D_VER_REVISION="%LP3D_VER_REVISION%"
>>%genFile% export LP3D_VER_BUILD="%LP3D_VER_BUILD%"
>>%genFile% export LP3D_VER_SHA_HASH="%LP3D_VER_SHA_HASH%"
IF [%LP3D_VER_SUFFIX%] NEQ [] (
  >>%genFile% export LP3D_VER_SUFFIX="%LP3D_VER_SUFFIX%"
)
>>%genFile% export LP3D_VERSION="%LP3D_VERSION%"
>>%genFile% export LP3D_APP_VERSION="%LP3D_APP_VERSION%"
>>%genFile% export LP3D_APP_VERSION_TAG="%LP3D_APP_VERSION_TAG%"
>>%genFile% export LP3D_APP_VER_SUFFIX="%LP3D_APP_VER_SUFFIX%"
>>%genFile% export LP3D_APP_VERSION_LONG="%LP3D_APP_VERSION_LONG%"
>>%genFile% export LP3D_BUILD_VERSION="%LP3D_BUILD_VERSION%"
>>%genFile% export LP3D_VERSION_INFO="%LP3D_VERSION_INFO%"
>>%genFile% export LP3D_BUILD_DATE_TIME="%LP3D_BUILD_DATE_TIME%"
>>%genFile% export LP3D_CHANGE_DATE_LONG="%LP3D_CHANGE_DATE_LONG%"
>>%genFile% export LP3D_AVAILABLE_VERSIONS="%LP3D_AVAILABLE_VERSIONS%"
>>%genFile% export LP3D_BUILD_PACKAGE="%LP3D_PACKAGE%-Any-%LP3D_APP_VERSION_LONG%"
>>%genFile% export LP3D_BUILD_TARGET="%LP3D_PACKAGE_PATH_BASH%/${LP3D_BUILD_PACKAGE}"
>>%genFile% export LP3D_DOWNLOAD_ASSETS="${LP3D_BUILD_TARGET}/%LP3D_PACKAGE%_Download"
>>%genFile% export LP3D_UPDATE_ASSETS="${LP3D_BUILD_TARGET}/%LP3D_PACKAGE%_Update"
>>%genFile% echo ^&^& echo "- Update-config-files environment variables set in Bash"
>>%genFile% echo ^&^& echo "- LP3D BUILD Environment Variables:" ^&^& env ^| sort ^| grep 'LP3D_BUILD_*'
IF EXIST "%set_bash_vars%" (
  ECHO   FILE set_bash_vars.sh..........[written to %set_bash_vars%]
) ELSE (
  ECHO   FILE set_bash_vars.sh..........[ERROR - file %set_bash_vars% not found]
)
EXIT /b 0

:POSTPROCESS
ECHO.
ECHO - Post process...
ECHO.
ECHO - If everything went well Press any key to EXIT!
%SystemRoot%\explorer.exe "%WIN_PKG_DIR%\release\%LP3D_PRODUCT_DIR%"
PAUSE >NUL
EXIT /b

:END
ECHO.
ECHO - %~nx0 finished.
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
ECHO   Elapsed build time %hours%:%mins%:%secs%.%ms%
ENDLOCAL
EXIT /b
