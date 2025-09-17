@ECHO OFF
Title Create windows installer and portable package archive LPub3D distributions
rem --
rem  Trevor SANDY <trevor.sandy@gmail.com>
rem  Last Update: September 12, 2025
rem  Copyright (C) 2015 - 2025 by Trevor SANDY
rem --
rem This script is distributed in the hope that it will be useful,
rem but WITHOUT ANY WARRANTY; without even the implied warranty of
rem MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

SETLOCAL
@break off
@color 0a

CALL :ELAPSED_BUILD_TIME Start

ECHO.
ECHO ------------------------------------------------------
ECHO   -Start %~nx0...
ECHO ======================================================
ECHO.
ECHO - Create Windows LPub3D distributions

REM Set the current working directory to the source directory root - e.g. lpub3d\
FOR %%* IN (.) DO SET SCRIPT_RUN_DIR=%%~nx*
IF "%SCRIPT_RUN_DIR%" EQU "windows" (
  CALL :WD_ABS_PATH ..\..\
) ELSE (
  CALL :WD_ABS_PATH .
)

IF NOT EXIST "%ABS_WD%\mainApp" (
  ECHO -ERROR - mainApp folder not found. Working directory %ABS_WD% is not valid.
  GOTO :ERROR_END
)

SET _PRO_FILE_PWD_=%CD%\mainApp
CD /D "builds\windows"
SET WIN_PKG_DIR=%CD%

IF "%LP3D_VSVERSION%" == "" SET LP3D_VSVERSION=2022
rem determine openssl version - GEQ 2019 EQU v1.1
IF "%LP3D_QT32VCVERSION%" == "" SET LP3D_QT32VCVERSION=2019
IF "%LP3D_QT64VCVERSION%" == "" SET LP3D_QT64VCVERSION=2022
rem determine if using openssl or Qt plugin - GEQ 6 EQU Qt plugin
IF "%LP3D_QT32VERSION%" == "" SET LP3D_QT32VERSION=5.5
IF "%LP3D_QT64VERSION%" == "" SET LP3D_QT64VERSION=6.0
rem determine MSVC redistributable version - 1900 EQU 2015, 1910-16 EQU 2017, 1920-29 EQU 2019, 1930-44 EQU 2022
IF "%LP3D_MSC32_VER%" == "" SET LP3D_MSC32_VER=1929
IF "%LP3D_MSC64_VER%" == "" SET LP3D_MSC64_VER=1944
IF "%LP3D_MSCARM64_VER%" == "" SET LP3D_MSCARM64_VER=1944

SET RUN_NSIS=1
SET SIGN_APP=0
SET WIN32_SSL=1
SET WIN64_SSL=0
SET CREATE_PORTABLE=1
SET CONFIGURATION=release
SET TEST_CI_BUILD=0
SET LP3D_AMD64_ARM64_CROSS=0
SET LP3D_QT_MAJ_VER=5

rem Set ARM64 cross compilation
IF /I "%LP3D_BUILD_ARCH%" EQU "arm64" (
  SET LP3D_BUILD_ARCH=ARM64
  IF /I "%PROCESSOR_ARCHITECTURE%" == "AMD64" (
    SET LP3D_AMD64_ARM64_CROSS=1
  )
)

rem Set Windows built-in Tar
IF "%LP3D_VALID_TAR%" == "" SET LP3D_VALID_TAR=0
IF "%LP3D_SYS_DIR%" == "" (
  SET "LP3D_SYS_DIR=%WINDIR%\System32"
)
IF "%LP3D_WIN_TAR%" == "" (
  SET "LP3D_WIN_TAR=%LP3D_SYS_DIR%\Tar.exe"
)
IF NOT EXIST "%LP3D_WIN_TAR%" (
  SET LP3D_WIN_TAR=
  SET LP3D_WIN_TAR_MSG=Not Found
) ELSE (
  SET LP3D_VALID_TAR=1
  SET LP3D_VALID_7ZIP=0
  SET LP3D_WIN_TAR_MSG=%LP3D_WIN_TAR%
)

SET AUTO=0
IF "%LP3D_AMD_UNIVERSAL_BUILD%"=="" SET LP3D_AMD_UNIVERSAL_BUILD=1
IF "%LP3D_BUILD_ARCH%" EQU "x86" SET LP3D_AMD_UNIVERSAL_BUILD=0
IF "%LP3D_BUILD_ARCH%" EQU "x86_64" SET LP3D_AMD_UNIVERSAL_BUILD=0
IF "%LP3D_BUILD_ARCH%" EQU "ARM64" SET LP3D_AMD_UNIVERSAL_BUILD=0

IF /I "%INSTALL_CHECK_CI%" EQU "True" (
  ECHO.
  ECHO - Install check for CI detected.
  SET CI_PLATFORM=GITHUB
  SET TEST_CI_BUILD=1
)

IF /I "%INSTALL_CHECK%" EQU "True" (
  ECHO.
  ECHO - Install check detected.
  SET AUTO=1
)

IF /I "%GITHUB%" EQU "True" (
  SET AUTO=1
  SET SIGN_APP=1
  SET GITHUB_RUNNER_IMAGE=Visual Studio %LP3D_VSVERSION%
)

IF /I "%APPVEYOR%" EQU "True" (
  SET AUTO=1
  SET SIGN_APP=1
  SET APPVEYOR_BUILD_WORKER_IMAGE=Visual Studio %LP3D_VSVERSION%
)

IF %AUTO% NEQ 1 (
  ECHO.
  SET /p RUN_NSIS= - Run NSIS? Type 1 to run, 0 to ignore or 'Enter' to accept default [%RUN_NSIS%]:
  ECHO.
  SET /p CREATE_PORTABLE= - Create Portable Distributions? Type 1 to create, 0 to ignore or 'Enter' to accept default [%CREATE_PORTABLE%]:
  ECHO.
  SET /p TEST_CI_BUILD= - Test CI Build? Type 1 to run test build, 0 to ignore or 'Enter' to accept default [%TEST_CI_BUILD%]:
  SET SIGN_APP=0
)

IF %TEST_CI_BUILD% EQU 1 (
  SET SIGN_APP=0
  FOR /F "usebackq delims==" %%G IN (`git rev-parse HEAD`) DO (
    IF /I "%GITHUB%" EQU "True" (SET GITHUB_SHA=%%G)
    IF /I "%APPVEYOR%" EQU "True" (SET APPVEYOR_REPO_COMMIT=%%G)
  )
)

ECHO.
ECHO - Selected build options:
ECHO.
IF %TEST_CI_BUILD%   == 1 ECHO - This configuration will allow you to test your NSIS scripts.
IF /I "%GITHUB%"     == "True" ECHO   GITHUB BUILD.........................[Yes]
IF /I "%APPVEYOR%"   == "True" ECHO   APPVEYOR BUILD.......................[Yes]
IF %LP3D_AMD_UNIVERSAL_BUILD% == 1 ECHO   MULTI-ARCH BUILD.....................[Yes]
IF %AUTO%            == 1 ECHO   AUTOMATIC BUILD......................[Yes]
IF %RUN_NSIS%        == 1 ECHO   RUN NSIS BUILD UTILITY...............[Yes]
IF %SIGN_APP%        == 1 ECHO   SIGN INSTALL PACKAGES................[Yes]
IF %RUN_NSIS%        == 0 ECHO   RUN NSIS BUILD UTILITY...............[No]
IF %SIGN_APP%        == 0 ECHO   SIGN INSTALL PACKAGES................[No]
IF %LP3D_AMD_UNIVERSAL_BUILD% == 0 ECHO   MULTI-ARCH BUILD.....................[No]
IF %AUTO%            == 0 ECHO   AUTOMATIC BUILD......................[No]

IF %RUN_NSIS% == 0 ECHO.
IF %RUN_NSIS% == 0 ECHO - Start NSIS test build process...
IF %RUN_NSIS% == 1 ECHO.
IF %RUN_NSIS% == 1 ECHO - Start build process...

IF %LP3D_VALID_TAR% == 1 (
  SET "ZIP_UTILITY=%LP3D_WIN_TAR%"
) ELSE (
  SET ZIP_UTILITY=unknown
  SET "ZIP_UTILITY_64=%ProgramFiles%\7-zip"
  SET "ZIP_UTILITY_32=%ProgramFiles(x86)%\7-zip"
)

SET "NSIS_UTILITY=%ProgramFiles(x86)%\NSIS\makensis.exe"
SET "SIGN_UTILITY=%ProgramFiles(x86)%\Windows Kits\8.1\bin\x64\signtool.exe"

SET "UTILITIES_DIR=..\utilities"
SET "MAIN_APP_DIR=..\..\mainApp"
SET "LP3D_REPO_DIR=lpub3d"

SET VER_LDGLITE=ldglite-1.3
SET VER_LDVIEW=ldview-4.6
SET VER_LPUB3D_TRACE=lpub3d_trace_cui-3.8

SET LDGLITE_EXE=ldglite
SET LDVIEW_EXE=LDView
SET LPUB3D_TRACE_EXE=lpub3d_trace_cui

SET OPENSSL_LIB=OpenSSL
SET OPENSSL_VER=v1.1

SET TimeStamp=http://timestamp.digicert.com
SET Sha2=Sha256
SET PKey="%UTILITIES_DIR%\cert\lpub3dPrivateKey.p12"

ECHO.
ECHO - Environment check...

IF %RUN_NSIS% == 0 GOTO :MAIN

IF EXIST "%NSIS_UTILITY%" (
  ECHO.
  ECHO - NSIS executable found at "%NSIS_UTILITY%"
  GOTO :SIGN
)
ECHO.
ECHO * NSIS exectutable not at path defined. Only NSIS test configuration will be produced.
GOTO :MAIN

:SIGN
IF %SIGN_APP% == 0 GOTO :CHK_ZIP
IF /I "%GITHUB%" NEQ "True" (
  IF /I "%APPVEYOR%" NEQ "True" (
    SET PwD=unknown
    SET CHK_ZIP_GO=0
    IF EXIST "%SIGN_UTILITY%" (
      ECHO.
      ECHO - Signtool executable found at "%SIGN_UTILITY%"
      FOR /f %%i IN (../utilities/cert/credentials.txt) DO SET PwD=%%i
      SET CHK_ZIP_GO=1
      ECHO.
      ECHO - Code signing password is %PwD%.
    ) ELSE (
      ECHO.
      ECHO * Signtool exectutable not at path defined. Code signing will be ignored.
      SET SIGN_APP=0
    )
  )
)

:CHK_ZIP
IF EXIST "%ZIP_UTILITY_64%" (
  SET "ZIP_UTILITY=%ZIP_UTILITY_64%\7z.exe"
  SET "LP3D_WIN_TAR_MSG=%ZIP_UTILITY%"
  SET LP3D_VALID_7ZIP=1
  ECHO.
  ECHO - Zip exectutable found at "%ZIP_UTILITY_64%"
  GOTO :MAIN
) ELSE (
  IF EXIST "%ZIP_UTILITY_32%" (
    SET "ZIP_UTILITY=%ZIP_UTILITY_32%\7z.exe"
    SET "LP3D_WIN_TAR_MSG=%ZIP_UTILITY%"
    SET LP3D_VALID_7ZIP=1
    ECHO.
    ECHO - Zip exectutable found at "%ZIP_UTILITY_32%"
    GOTO :MAIN
  )
)

SET OPTION=0
IF %AUTO% NEQ 1 (
  IF NOT EXIST "%ZIP_UTILITY%" (
    ECHO.
    ECHO * Could not find 7zip executable. Requested manual location entry.
    SET /p ZIP_UTILITY=Could not find any zip executable. You can manually enter a location:
  )

  IF EXIST "%ZIP_UTILITY%" (
    ECHO.
    ECHO - 7Zip exectutable at "%ZIP_UTILITY%" will be used to archive your portable distributions.
    SET LP3D_VALID_7ZIP=1
    SET /p OPTION= Type [1] to exit or Enter to continue:
  )

  IF %OPTION% == 1 (
    ECHO * Option to exit seleced, the script will terminate.
    GOTO :ERROR_END
  )
)

IF NOT EXIST "%ZIP_UTILITY%" (
  SET CREATE_PORTABLE=0
  ECHO.
  ECHO * Could not find zip executable. Portable distributions will be ignored.
  GOTO :MAIN
)

:MAIN
SET LP3D_PRODUCT=unknown
SET LP3D_COMPANY=unknown
SET LP3D_COMMENTS=unknown
SET LP3D_PUBLISHER=unknown
SET LP3D_COMPANY_URL=unknown
SET LP3D_BUILD_DATE_ALT=unknown
SET LP3D_REVISION_FILE=unknown
SET LP3D_SUPPORT=unknown
SET LP3D_DISTRO_ARCH=unknown
SET LPUB3D_BUILD_FILE=unknown

SET "LP3D_GITHUB_URL=https://github.com/trevorsandy"
SET "LP3D_GITHUB_BASE=%LP3D_GITHUB_URL%/%LP3D_REPO_DIR%"
SET "LP3D_SOURCEFORGE_OPEN_BASE=https://sourceforge.net"
SET "LP3D_SOURCEFORGE_UPDATE_BASE=https://lpub3d.sourceforge.io"
SET "LP3D_LIBS_BASE=%LP3D_GITHUB_URL%/lpub3d_libs/releases/download/v1.0.1"

SET LP3D_AARCH64_ARCH=aarch64
SET LP3D_X64_ARCH=x86_64
SET LP3D_AMD_ARCH=amd64
SET LP3D_ARM_ARCH=arm64
SET LP3D_UPDATE_SIZE=0
SET LP3D_DEB=noble
SET LP3D_FCV=fc41

SET LP3D_DIST_EXTENSIONS=amd_exe, arm_exe, amd_con_exe, amd_msys_exe, exe, arm_dmg, amd_dmg, dmg, amd_deb, arm_deb, deb, amd_rpm, rpm, amd_pkg, pkg, amd_api, arm_api, api, amd_snp, arm_snp, snp, amd_flp, arm_flp, flp
FOR %%C IN ( %LP3D_DIST_EXTENSIONS% ) DO (SET /A LP3D_UPDATE_SIZE+=1)
FOR %%e IN ( %LP3D_DIST_EXTENSIONS% ) DO (
  SET LP3D_AVAILABLE_VERSIONS_%%e=unknown
  SET LP3D_ALTERNATE_VERSIONS_%%e=unknown
)

SET "amd_exe=.exe"                              & ::- 01 Windows
SET "arm_exe=.exe"                              & ::- 02 Windows
SET "amd_con_exe=-%LP3D_X64_ARCH%.tar.bz2"      & ::- 03 Windows
SET "amd_msys_exe=-any.pkg.tar.zst"             & ::- 04 Windows
SET "win_exe=.exe"                                  & ::- 05 Windows
SET "arm_dmg=-%LP3D_ARM_ARCH%-macos.dmg"        & ::- 06 Apple macOS
SET "amd_dmg=-%LP3D_X64_ARCH%-macos.dmg"        & ::- 07 Apple macOS
SET "dmg=-macos.dmg"                            & ::- 08 Apple macOS
SET "amd_deb=-%LP3D_DEB%-%LP3D_AMD_ARCH%.deb"   & ::- 09 Debian Linux
SET "arm_deb=-%LP3D_DEB%-%LP3D_ARM_ARCH%.deb"   & ::- 10 Debian Linux
SET "deb=-%LP3D_DEB%-%LP3D_AMD_ARCH%.deb"       & ::- 11 Debian Linux
SET "amd_rpm=-1.%LP3D_FCV%.%LP3D_X64_ARCH%.rpm" & ::- 12 Red Hat Linux
SET "rpm=-1.%LP3D_FCV%.%LP3D_X64_ARCH%.rpm"     & ::- 13 Red Hat Linux
SET "amd_pkg=-%LP3D_X64_ARCH%.pkg.tar.zst"      & ::- 14 Arch Linux
SET "pkg=-%LP3D_X64_ARCH%.pkg.tar.xz"           & ::- 15 Arch Linux
SET "amd_api=-%LP3D_X64_ARCH%.AppImage"         & ::- 16 Linux AppImage
SET "arm_api=-%LP3D_AARCH64_ARCH%.AppImage"     & ::- 17 Linux AppImage
SET "api=-%LP3D_X64_ARCH%.AppImage"             & ::- 18 Linux AppImage
SET "amd_snp=-%LP3D_X64_ARCH%.snap"             & ::- 19 Linux Snap
SET "arm_snp=-%LP3D_ARM_ARCH%.snap"             & ::- 20 Linux Snap
SET "snp=-%LP3D_X64_ARCH%.snap"                 & ::- 21 Linux Snap
SET "amd_flp=-%LP3D_X64_ARCH%.flatpak"          & ::- 22 Linux Flatpak
SET "arm_flp=-%LP3D_ARM_ARCH%.flatpak"          & ::- 23 Linux Flatpak
SET "flp=-%LP3D_X64_ARCH%.flatpak"              & ::- 24 Linux Flatpak

ECHO.
ECHO - Setting up release build parameters...

CD /D "%UTILITIES_DIR%"

SET "LP3D_ICON_DIR=%CD%\icons"

CALL update-config-files.bat %_PRO_FILE_PWD_%
IF ERRORLEVEL 1 (GOTO :ERROR_END)

IF /I "%APPVEYOR%" EQU "True" (
  CALL :CREATE_LP3D_PS_VARS_FILE
)
CALL :CREATE_LP3D_BASH_VARS_FILE

REM Token assignments
REM tokens=*  : all tokens in %%i                               - e.g. %%i
REM tokens=2* : second token in %%i, rest in next %%j token     - e.g. %%i, %%j
REM tokens=1,2: first token in %%i, second in %%j               - e.g. %%i, %%j
REM tokens=1-3: first token in %%i, second in %%j, third in %%k - e.g. %%i,%%j,%%k

REM Available versions imported from update-config-files.bat
REM LP3D_PAST_RELEASES=2.3.6,2.0.20,1.3.5,1.2.3,1.0.0
REM LP3D_AVAILABLE_VERSIONS=%LP3D_VERSION%,%LP3D_PAST_RELEASES%

REM available versions by platform, set tokens to select specific version or versions as appropriate
FOR /F "tokens=*   delims=," %%i IN ("%LP3D_AVAILABLE_VERSIONS%") DO SET LP3D_AVAILABLE_VERSIONS_amd_exe=%%i
FOR /F "tokens=1   delims=," %%i IN ("%LP3D_AVAILABLE_VERSIONS%") DO SET LP3D_AVAILABLE_VERSIONS_arm_exe=%%i
FOR /F "tokens=1   delims=," %%i IN ("%LP3D_AVAILABLE_VERSIONS%") DO SET LP3D_AVAILABLE_VERSIONS_amd_con_exe=%%i
FOR /F "tokens=1   delims=," %%i IN ("%LP3D_AVAILABLE_VERSIONS%") DO SET LP3D_AVAILABLE_VERSIONS_amd_msys_exe=%%i
FOR /F "tokens=*   delims=," %%i IN ("%LP3D_AVAILABLE_VERSIONS%") DO SET LP3D_AVAILABLE_VERSIONS_exe=%%i
FOR /F "tokens=1   delims=," %%i IN ("%LP3D_AVAILABLE_VERSIONS%") DO SET LP3D_AVAILABLE_VERSIONS_arm_dmg=%%i
FOR /F "tokens=1-3 delims=," %%i IN ("%LP3D_AVAILABLE_VERSIONS%") DO SET LP3D_AVAILABLE_VERSIONS_amd_dmg=%%i,%%j,%%k
FOR /F "tokens=1-3 delims=," %%i IN ("%LP3D_AVAILABLE_VERSIONS%") DO SET LP3D_AVAILABLE_VERSIONS_dmg=%%i,%%j,%%k
FOR /F "tokens=1-3 delims=," %%i IN ("%LP3D_AVAILABLE_VERSIONS%") DO SET LP3D_AVAILABLE_VERSIONS_amd_deb=%%i,%%j,%%k
FOR /F "tokens=1   delims=," %%i IN ("%LP3D_AVAILABLE_VERSIONS%") DO SET LP3D_AVAILABLE_VERSIONS_arm_deb=%%i
FOR /F "tokens=1-3 delims=," %%i IN ("%LP3D_AVAILABLE_VERSIONS%") DO SET LP3D_AVAILABLE_VERSIONS_deb=%%i,%%j,%%k
FOR /F "tokens=1-3 delims=," %%i IN ("%LP3D_AVAILABLE_VERSIONS%") DO SET LP3D_AVAILABLE_VERSIONS_amd_rpm=%%i,%%j,%%k
FOR /F "tokens=1-3 delims=," %%i IN ("%LP3D_AVAILABLE_VERSIONS%") DO SET LP3D_AVAILABLE_VERSIONS_rpm=%%i,%%j,%%k
FOR /F "tokens=1-3 delims=," %%i IN ("%LP3D_AVAILABLE_VERSIONS%") DO SET LP3D_AVAILABLE_VERSIONS_amd_pkg=%%i,%%j,%%k
FOR /F "tokens=1-3 delims=," %%i IN ("%LP3D_AVAILABLE_VERSIONS%") DO SET LP3D_AVAILABLE_VERSIONS_pkg=%%i,%%j,%%k
FOR /F "tokens=1-2 delims=," %%i IN ("%LP3D_AVAILABLE_VERSIONS%") DO SET LP3D_AVAILABLE_VERSIONS_amd_api=%%i,%%j
FOR /F "tokens=1   delims=," %%i IN ("%LP3D_AVAILABLE_VERSIONS%") DO SET LP3D_AVAILABLE_VERSIONS_arm_api=%%i
FOR /F "tokens=1-2 delims=," %%i IN ("%LP3D_AVAILABLE_VERSIONS%") DO SET LP3D_AVAILABLE_VERSIONS_api=%%i,%%j
FOR /F "tokens=1   delims=," %%i IN ("%LP3D_AVAILABLE_VERSIONS%") DO SET LP3D_AVAILABLE_VERSIONS_amd_snp=%%i
FOR /F "tokens=1   delims=," %%i IN ("%LP3D_AVAILABLE_VERSIONS%") DO SET LP3D_AVAILABLE_VERSIONS_arm_snp=%%i
FOR /F "tokens=1   delims=," %%i IN ("%LP3D_AVAILABLE_VERSIONS%") DO SET LP3D_AVAILABLE_VERSIONS_snp=%%i
FOR /F "tokens=1   delims=," %%i IN ("%LP3D_AVAILABLE_VERSIONS%") DO SET LP3D_AVAILABLE_VERSIONS_amd_flp=%%i
FOR /F "tokens=1   delims=," %%i IN ("%LP3D_AVAILABLE_VERSIONS%") DO SET LP3D_AVAILABLE_VERSIONS_arm_flp=%%i
FOR /F "tokens=1   delims=," %%i IN ("%LP3D_AVAILABLE_VERSIONS%") DO SET LP3D_AVAILABLE_VERSIONS_flp=%%i

FOR /F "tokens=2*  delims=," %%i IN ("%LP3D_AVAILABLE_VERSIONS_amd_exe%") DO SET LP3D_ALTERNATE_VERSIONS_amd_exe=%%i,%%j
FOR /F "tokens=1   delims=," %%i IN ("%LP3D_AVAILABLE_VERSIONS_arm_exe%") DO SET LP3D_ALTERNATE_VERSIONS_arm_exe=%%i
FOR /F "tokens=1   delims=," %%i IN ("%LP3D_AVAILABLE_VERSIONS_amd_con_exe%") DO SET LP3D_ALTERNATE_VERSIONS_amd_con_exe=%%i
FOR /F "tokens=1   delims=," %%i IN ("%LP3D_AVAILABLE_VERSIONS_amd_msys_exe%") DO SET LP3D_ALTERNATE_VERSIONS_amd_msys_exe=%%i
FOR /F "tokens=2*  delims=," %%i IN ("%LP3D_AVAILABLE_VERSIONS_exe%") DO SET LP3D_ALTERNATE_VERSIONS_exe=%%i,%%j
FOR /F "tokens=1   delims=," %%i IN ("%LP3D_AVAILABLE_VERSIONS_arm_dmg%") DO SET LP3D_ALTERNATE_VERSIONS_arm_dmg=%%i
FOR /F "tokens=2,3 delims=," %%i IN ("%LP3D_AVAILABLE_VERSIONS_amd_dmg%") DO SET LP3D_ALTERNATE_VERSIONS_amd_dmg=%%i,%%j
FOR /F "tokens=2,3 delims=," %%i IN ("%LP3D_AVAILABLE_VERSIONS_dmg%") DO SET LP3D_ALTERNATE_VERSIONS_dmg=%%i,%%j
FOR /F "tokens=2,3 delims=," %%i IN ("%LP3D_AVAILABLE_VERSIONS_amd_deb%") DO SET LP3D_ALTERNATE_VERSIONS_amd_deb=%%i,%%j
FOR /F "tokens=1   delims=," %%i IN ("%LP3D_AVAILABLE_VERSIONS_arm_deb%") DO SET LP3D_ALTERNATE_VERSIONS_arm_deb=%%i
FOR /F "tokens=2,3 delims=," %%i IN ("%LP3D_AVAILABLE_VERSIONS_deb%") DO SET LP3D_ALTERNATE_VERSIONS_deb=%%i,%%j
FOR /F "tokens=2,3 delims=," %%i IN ("%LP3D_AVAILABLE_VERSIONS_amd_rpm%") DO SET LP3D_ALTERNATE_VERSIONS_amd_rpm=%%i,%%j
FOR /F "tokens=2,3 delims=," %%i IN ("%LP3D_AVAILABLE_VERSIONS_rpm%") DO SET LP3D_ALTERNATE_VERSIONS_rpm=%%i,%%j
FOR /F "tokens=2,3 delims=," %%i IN ("%LP3D_AVAILABLE_VERSIONS_amd_pkg%") DO SET LP3D_ALTERNATE_VERSIONS_amd_pkg=%%i,%%j
FOR /F "tokens=2,3 delims=," %%i IN ("%LP3D_AVAILABLE_VERSIONS_pkg%") DO SET LP3D_ALTERNATE_VERSIONS_pkg=%%i,%%j
FOR /F "tokens=2   delims=," %%i IN ("%LP3D_AVAILABLE_VERSIONS_amd_api%") DO SET LP3D_ALTERNATE_VERSIONS_amd_api=%%i
FOR /F "tokens=1   delims=," %%i IN ("%LP3D_AVAILABLE_VERSIONS_arm_api%") DO SET LP3D_ALTERNATE_VERSIONS_arm_api=%%i
FOR /F "tokens=2   delims=," %%i IN ("%LP3D_AVAILABLE_VERSIONS_api%") DO SET LP3D_ALTERNATE_VERSIONS_api=%%i
FOR /F "tokens=1   delims=," %%i IN ("%LP3D_AVAILABLE_VERSIONS_amd_snp%") DO SET LP3D_ALTERNATE_VERSIONS_amd_snp=%%i
FOR /F "tokens=1   delims=," %%i IN ("%LP3D_AVAILABLE_VERSIONS_arm_snp%") DO SET LP3D_ALTERNATE_VERSIONS_arm_snp=%%i
FOR /F "tokens=1   delims=," %%i IN ("%LP3D_AVAILABLE_VERSIONS_snp%") DO SET LP3D_ALTERNATE_VERSIONS_snp=%%i
FOR /F "tokens=1   delims=," %%i IN ("%LP3D_AVAILABLE_VERSIONS_amd_flp%") DO SET LP3D_ALTERNATE_VERSIONS_amd_flp=%%i
FOR /F "tokens=1   delims=," %%i IN ("%LP3D_AVAILABLE_VERSIONS_arm_flp%") DO SET LP3D_ALTERNATE_VERSIONS_arm_flp=%%i
FOR /F "tokens=1   delims=," %%i IN ("%LP3D_AVAILABLE_VERSIONS_flp%") DO SET LP3D_ALTERNATE_VERSIONS_flp=%%i

CD /D "%WIN_PKG_DIR%"
CD /D "%MAIN_APP_DIR%"

FOR /F "tokens=3*" %%i IN ('FINDSTR /c:"#define VER_COMPANYNAME_BLD_STR" version.h') DO SET LP3D_COMPANY=%%i %%j
FOR /F "tokens=3*" %%i IN ('FINDSTR /c:"#define VER_PRODUCTNAME_STR" version.h') DO SET LP3D_PRODUCT=%%i
FOR /F "tokens=3*" %%i IN ('FINDSTR /c:"#define VER_PUBLISHER_STR" version.h') DO SET LP3D_PUBLISHER=%%i %%j
FOR /F "tokens=3*" %%i IN ('FINDSTR /c:"#define VER_COMPANYDOMAIN_STR" version.h') DO SET LP3D_COMPANY_URL=%%i
FOR /F "tokens=3*" %%i IN ('FINDSTR /c:"#define VER_FILEDESCRIPTION_STR" version.h') DO SET LP3D_COMMENTS=%%i %%j
FOR /F "tokens=3*" %%i IN ('FINDSTR /c:"#define VER_PUBLISHER_SUPPORT_STR" version.h') DO SET LP3D_SUPPORT=%%i

SET LP3D_PRODUCT=%LP3D_PRODUCT:"=%
SET LP3D_DATE_TIME=%LP3D_YEAR% %LP3D_MONTH% %LP3D_DAY% %LP3D_TIME%
SET LP3D_PRODUCT_DIR=%LP3D_PRODUCT%-Any-%LP3D_APP_VERSION_LONG%
SET LP3D_BUILD_FILE=%LP3D_PRODUCT%.exe
SET LPUB3D_BUILD_FILE=%LP3D_BUILD_FILE%
IF %LP3D_AMD_UNIVERSAL_BUILD% == 1 (
  SET LP3D_OUTPUT_FILENAME=%LP3D_APP_VERSION_LONG%
) ELSE (
  SET LP3D_OUTPUT_FILENAME=%LP3D_BUILD_ARCH%-%LP3D_APP_VERSION_LONG%
)
SET LP3D_DOWNLOAD_PRODUCT=%LP3D_PRODUCT%-%LP3D_OUTPUT_FILENAME%

IF /I "%LP3D_BUILD_TYPE%" EQU "Continuous" (
  SET LP3D_VER_TAG_NAME=continuous
) ELSE (
  SET LP3D_VER_TAG_NAME=v%LP3D_VERSION%
)

CD /D "%WIN_PKG_DIR%"

IF NOT EXIST "%CONFIGURATION%\%LP3D_PRODUCT_DIR%" (
  ECHO.
  ECHO -ERROR - Did not find %LP3D_PRODUCT% product directory.
  ECHO -Expected %LP3D_PRODUCT_DIR% at "%CD%\%CONFIGURATION%\"
  GOTO :ERROR_END
)

REM pwd = windows/release/LP3D_PRODUCT_DIR [holds _PKG_DIST_DIR, PKG_UPDATE_DIR, PKG_DOWNLOAD_DIR, PKG_LOGS_DIR]
CD /D "%CONFIGURATION%\%LP3D_PRODUCT_DIR%"

ECHO.
ECHO - Create folders and delete old content...

SET PKG_DOWNLOAD_DIR=%LP3D_PRODUCT%_Download
IF NOT EXIST "%PKG_DOWNLOAD_DIR%" (
  MKDIR "%PKG_DOWNLOAD_DIR%"
) ELSE (
  DEL /Q "%PKG_DOWNLOAD_DIR%\*.*"
)

SET PKG_UPDATE_DIR=%LP3D_PRODUCT%_Update
IF NOT EXIST "%PKG_UPDATE_DIR%" (
  MKDIR "%PKG_UPDATE_DIR%"
) ELSE (
  DEL /Q "%PKG_UPDATE_DIR%\*.*"
)

SET PKG_LOGS_DIR=%LP3D_PRODUCT%_Logs
IF NOT EXIST "%PKG_LOGS_DIR%" (
  MKDIR "%PKG_LOGS_DIR%"
) ELSE (
  DEL /Q "%PKG_LOGS_DIR%\*.*"
)

ECHO.
ECHO - Building distribution package...
ECHO.
ECHO   LP3D_VER_MAJOR.......................[%LP3D_VER_MAJOR%]
ECHO   LP3D_VER_MINOR.......................[%LP3D_VER_MINOR%]
ECHO   LP3D_VER_PATCH.......................[%LP3D_VER_PATCH%]
ECHO   LP3D_VER_REVISION....................[%LP3D_VER_REVISION%]
ECHO   LP3D_VER_BUILD.......................[%LP3D_VER_BUILD%]
ECHO   LP3D_VER_SHA_HASH....................[%LP3D_VER_SHA_HASH%]
IF "%LP3D_VER_TAG_NAME%" NEQ "v%LP3D_VERSION%" (
  ECHO   LP3D_VER_TAG_NAME....................[%LP3D_VER_TAG_NAME%]
)
ECHO.
ECHO   LP3D_YEAR............................[%LP3D_YEAR%]
ECHO   LP3D_MONTH...........................[%LP3D_MONTH%]
ECHO   LP3D_DAY.............................[%LP3D_DAY%]
ECHO   LP3D_TIME............................[%LP3D_TIME%]
ECHO.

ECHO   LP3D_VERSION.........................[%LP3D_VERSION%]
ECHO   LP3D_APP_VERSION.....................[%LP3D_APP_VERSION%]
ECHO   LP3D_APP_VERSION_LONG................[%LP3D_APP_VERSION_LONG%]
ECHO   LP3D_AVAILABLE_VERSIONS..............[%LP3D_AVAILABLE_VERSIONS%]
REM
REM Vars above this line are sourced from update-config-files.bat and below are local
ECHO.
ECHO   LP3D_COMPANY.........................[%LP3D_COMPANY%]
ECHO   LP3D_PRODUCT.........................["%LP3D_PRODUCT%"]
ECHO   LP3D_PUBLISHER.......................[%LP3D_PUBLISHER%]
ECHO   LP3D_COMPANY_URL.....................[%LP3D_COMPANY_URL%]
ECHO   LP3D_SUPPORT.........................[%LP3D_SUPPORT%]
ECHO   LP3D_COMMENTS........................[%LP3D_COMMENTS%]
ECHO.
ECHO   LP3D_DATE_TIME.......................[%LP3D_DATE_TIME%]
ECHO   LP3D_DOWNLOAD_PRODUCT................[%LP3D_DOWNLOAD_PRODUCT%]
ECHO   LP3D_PRODUCT_DIR.....................[%LP3D_PRODUCT_DIR%]
ECHO   LP3D_BUILD_FILE......................[%LP3D_BUILD_FILE%]
ECHO   LP3D_WIN_TAR.........................[%LP3D_WIN_TAR_MSG%]
ECHO.
IF /I "%GITHUB%" EQU "True" (
ECHO   BUILD_WORKER_IMAGE...................[%GITHUB_RUNNER_IMAGE%]
)
IF /I "%APPVEYOR%" EQU "True" (
ECHO   BUILD_WORKER_IMAGE...................[%APPVEYOR_BUILD_WORKER_IMAGE%]
)
IF /I "%PLATFORM_ARCH%" == "ARM64" (
ECHO   PROCESSOR_ARCH.......................[%PROCESSOR_ARCHITECTURE%]
IF %LP3D_AMD64_ARM64_CROSS% EQU 1 (
ECHO   COMPILATION..........................[Cross compile ARM64 build on AMD64 host]
)
)

SETLOCAL ENABLEDELAYEDEXPANSION
SET LP3D_DIST_EXTENSIONS_ECHO=amd_exe, arm_exe, amd_con_exe, amd_msys_exe, arm_dmg, amd_dmg, amd_deb, arm_deb, amd_rpm, amd_pkg, amd_api, arm_api, amd_snp, arm_snp, amd_flp, arm_flp
FOR %%e IN ( AVAILABLE, ALTERNATE ) DO (
  ECHO.
  SET /A c=0
  SET "VERSION_KEY=LP3D_%%e_VERSIONS"
  FOR %%f IN ( %LP3D_DIST_EXTENSIONS_ECHO% ) DO (
    SET /A c+=1
    IF !c! EQU 3 (
      SET DOTS=..
    ) ELSE (
      IF !c! EQU 4 (SET DOTS=.) ELSE (SET DOTS=......)
    )
    CALL SET "VERSION_VALUE=%%!VERSION_KEY!_%%f%%"
    ECHO   !VERSION_KEY!_%%f!DOTS![!VERSION_VALUE!]
  )
)
SETLOCAL DISABLEDELAYEDEXPANSION

IF %LP3D_AMD_UNIVERSAL_BUILD% NEQ 1 (
  REM SINGLE ARCH BUILD
  SETLOCAL ENABLEDELAYEDEXPANSION
  SET LP3D_DISTRO_ARCH=%LP3D_BUILD_ARCH%
  IF /I "%GITHUB%" NEQ "True" (
    IF /I "%APPVEYOR%" NEQ "True" (
      IF "%1" NEQ "" (
        SET LP3D_DISTRO_ARCH=%1
      ) ELSE (
        IF %AUTO% NEQ 1 (
          SET /p LP3D_DISTRO_ARCH=  Enter build architecture [x86^|x86_64^|ARM64] or press enter to accept default [%LP3D_DISTRO_ARCH%]:
        )
      )
      IF "!LP3D_DISTRO_ARCH!" NEQ "x86" (
        IF "!LP3D_DISTRO_ARCH!" NEQ "x86_64" (
          IF "!LP3D_DISTRO_ARCH!" NEQ "ARM64" (
            ECHO.
            ECHO * !LP3D_DISTRO_ARCH! is not valid for single architecture build.
            GOTO :ERROR_END
          )
        )
      )
    )
  )
  SET PKG_DISTRO_DIR=%LP3D_PRODUCT%_!LP3D_DISTRO_ARCH!
  SET PKG_DISTRO_PORTABLE_DIR=%LP3D_PRODUCT%_!LP3D_DISTRO_ARCH!-%LP3D_APP_VERSION_LONG%
  ECHO.
  ECHO - Configuring %LP3D_PRODUCT% !LP3D_DISTRO_ARCH! build...
  ECHO.
  ECHO   LP3D_DISTRO_ARCH..................[!LP3D_DISTRO_ARCH!]
  ECHO   PKG_DISTRO_DIR....................[!PKG_DISTRO_DIR!]
  IF !LP3D_DISTRO_ARCH!==x86 (
    ECHO   LP3D_MSC32_VER....................[%LP3D_MSC32_VER%]
    ECHO   LP3D_QT32_VCVERSION...............[%LP3D_QT32VCVERSION%]
    ECHO   LP3D_QT32_VERSION.................[%LP3D_QT32VERSION%]
    FOR /f "tokens=1 delims=." %%A IN ("%LP3D_QT32VERSION%") DO SET "LP3D_QT_MAJ_VER=%%A"
    ECHO   LP3D_QT_MAJOR_VERSION.............[!LP3D_QT_MAJ_VER!]
    CALL :SETSSLVERSION %LP3D_QT32VCVERSION%
    CALL :DOWNLOADOPENSSLLIBS
    CALL :DOWNLOADMSVCREDIST %LP3D_MSC32_VER% !LP3D_DISTRO_ARCH!
  ) ELSE (
    rem !LP3D_DISTRO_ARCH!==x86_64 or !LP3D_DISTRO_ARCH!==ARM64
    ECHO   LP3D_MSC64_VER....................[%LP3D_MSC64_VER%]
    ECHO   LP3D_QT64_VCVERSION...............[%LP3D_QT64VCVERSION%]
    ECHO   LP3D_QT64_VERSION.................[%LP3D_QT64VERSION%]
    FOR /f "tokens=1 delims=." %%A IN ("%LP3D_QT64VERSION%") DO SET "LP3D_QT_MAJ_VER=%%A"
    ECHO   LP3D_QT_MAJOR_VERSION.............[!LP3D_QT_MAJ_VER!]
    IF !LP3D_QT_MAJ_VER! EQU 6 (SET WIN64_SSL=0) ELSE (SET WIN64_SSL=1)
    IF !WIN64_SSL! EQU 1 CALL :SETSSLVERSION %LP3D_QT64VCVERSION%
    IF !WIN64_SSL! EQU 1 CALL :DOWNLOADOPENSSLLIBS
    IF /I !LP3D_DISTRO_ARCH!==x86_64 CALL :DOWNLOADMSVCREDIST %LP3D_MSC64_VER% !LP3D_DISTRO_ARCH!
    IF /I !LP3D_DISTRO_ARCH!==ARM64 CALL :DOWNLOADMSVCREDIST %LP3D_MSCARM64_VER% !LP3D_DISTRO_ARCH!
  )
  SET LP3D_DOWNLOAD_PRODUCT=%LP3D_PRODUCT%-!LP3D_DISTRO_ARCH!-%LP3D_APP_VERSION_LONG%
  CALL :COPYFILES
  CALL :DOWNLOADLDRAWLIBS
  IF %RUN_NSIS% == 1 CALL :GENERATENSISPARAMS
  IF %RUN_NSIS% == 1 CALL :NSISBUILD
  IF %SIGN_APP% == 1 CALL :SIGNAPP
  IF %CREATE_PORTABLE% == 1 CALL :CREATEPORTABLEDISTRO
  ENDLOCAL DISABLEDELAYEDEXPANSION
) ELSE (
  REM AMD UNIVERSAL BUILD
  ECHO.
  ECHO - Executing universal package build...
  FOR %%A IN ( x86_64, x86 ) DO (
    SET LP3D_DISTRO_ARCH=%%A
    SET PKG_DISTRO_DIR=%LP3D_PRODUCT%_%%A
    ECHO.
    ECHO - Configuring %LP3D_PRODUCT% %%A build...
    ECHO.
    ECHO   LP3D_DISTRO_ARCH..................[%%A]
    ECHO   PKG_DISTRO_DIR....................[%LP3D_PRODUCT%_%%A]
    SETLOCAL ENABLEDELAYEDEXPANSION
    IF %%A==x86 (
      ECHO   LP3D_MSC32_VER....................[%LP3D_MSC32_VER%]
      ECHO   LP3D_QT32_VCVERSION...............[%LP3D_QT32VCVERSION%]
      ECHO   LP3D_QT32_VERSION.................[%LP3D_QT32VERSION%]
      FOR /f "tokens=1 delims=." %%B IN ("%LP3D_QT32VERSION%") DO SET "LP3D_QT_MAJ_VER=%%B"
      ECHO   LP3D_QT_MAJOR_VERSION.............[!LP3D_QT_MAJ_VER!]
      CALL :SETSSLVERSION %LP3D_QT32VCVERSION%
      CALL :DOWNLOADOPENSSLLIBS
      CALL :DOWNLOADMSVCREDIST %LP3D_MSC32_VER% %%A
    ) ELSE (
      rem %%A==x86_64
      ECHO   LP3D_MSC64_VER....................[%LP3D_MSC64_VER%]
      ECHO   LP3D_QT64_VCVERSION...............[%LP3D_QT64VCVERSION%]
      ECHO   LP3D_QT64_VERSION.................[%LP3D_QT64VERSION%]
      FOR /f "tokens=1 delims=." %%B IN ("%LP3D_QT64VERSION%") DO SET "LP3D_QT_MAJ_VER=%%B"
      ECHO   LP3D_QT_MAJOR_VERSION.............[!LP3D_QT_MAJ_VER!]
      IF !LP3D_QT_MAJ_VER! EQU 6 (SET WIN64_SSL=0) ELSE (SET WIN64_SSL=1)
      IF !WIN64_SSL! EQU 1 CALL :SETSSLVERSION %LP3D_QT64VCVERSION%
      IF !WIN64_SSL! EQU 1 CALL :DOWNLOADOPENSSLLIBS
      IF %%A==x86_64 CALL :DOWNLOADMSVCREDIST %LP3D_MSC64_VER% %%A
    )
    CALL :COPYFILES
    ENDLOCAL DISABLEDELAYEDEXPANSION
  )
  CALL :DOWNLOADLDRAWLIBS
  IF %RUN_NSIS% == 1 CALL :GENERATENSISPARAMS
  IF %RUN_NSIS% == 1 CALL :NSISBUILD
  IF %SIGN_APP% == 1 CALL :SIGNAPP
  FOR %%A IN ( x86_64, x86 ) DO (
  SET LP3D_DISTRO_ARCH=%%A
    SET PKG_DISTRO_DIR=%LP3D_PRODUCT%_%%A
    SET PKG_DISTRO_PORTABLE_DIR=%LP3D_PRODUCT%_%%A-%LP3D_APP_VERSION_LONG%
    IF %CREATE_PORTABLE% == 1 CALL :CREATEPORTABLEDISTRO
  )
)

CALL :GENERATE_JSON

IF "%GITHUB%" EQU "True" (
  CALL :MOVE_ASSETS_TO_DISTRO_PATH
  REM reset ErrorLevel to 0 from 1 for unsuccessful FINDSTR in MOVE_ASSETS_TO_DISTRO_PATH
  (CALL )
)

IF %AUTO% NEQ 1 (
  CALL :POSTPROCESS
)

GOTO :END

:SETSSLVERSION
IF %1 GEQ 2019 (
  SET OPENSSL_VER=v1.1
) ELSE (
  SET OPENSSL_VER=v1.0
)
ECHO   OPEN_SSL_VERSION..................[%OPENSSL_VER%]
EXIT /b

:COPYFILES
REM Product Full Version Format:
REM Product _ Version . Revision . Build _ Date YYYYMMDD
REM LPub3D  _ 2.0.20  . 106      . 752   _ 20170929

REM Directory Structure Format:
REM Windows                                                            - builds/windows
REM   |_Release                                                        - windows/release
REM        |_PRODUCT_DIR          = LP3D_PRODUCT LP3D_APP_VERSION_LONG - LPub3D_2.0.20.106.752_20170929
REM            |_PKG_DIST_DIR     = LP3D_PRODUCT LP3D_DISTRO_ARCH      - LPub3D_x86_64
REM            |_PKG_DOWNLOAD_DIR = LP3D_PRODUCT_Download              - LPub3D_Download
REM            |_PKG_UPDATE_DIR   = LP3D_PRODUCT_Update                - LPub3D_Update
REM            |_PKG_LOGS_DIR     = LP3D_PRODUCT_Logs                  - LPub3D_Logs
IF NOT EXIST "%PKG_DISTRO_DIR%" (
  ECHO.
  ECHO * Did not find distribution package directory.
  ECHO * Expected "%PKG_DISTRO_DIR%" at "%CD%".
  ECHO * Build "%LP3D_PRODUCT%_%LP3D_APP_VERSION_LONG%" cannot continue.
  GOTO :ERROR_END
)

ECHO.
ECHO - Copying change_log_%LP3D_VERSION% to media folder...

REM pwd = windows/release/LP3D_PRODUCT_DIR
COPY /V /Y %PKG_DISTRO_DIR%\docs\README.txt %PKG_UPDATE_DIR%\change_log.txt /A | FINDSTR /i /v /r /c:"copied\>"
COPY /V /Y %PKG_DISTRO_DIR%\docs\README.txt %PKG_UPDATE_DIR%\change_log_%LP3D_VERSION%.txt /A | FINDSTR /i /v /r /c:"copied\>"
COPY /V /Y %PKG_DISTRO_DIR%\docs\README.txt %PKG_DOWNLOAD_DIR%\ | FINDSTR /i /v /r /c:"copied\>"

ECHO.
ECHO - Copying release_notes_%LP3D_VERSION% to media folder...

COPY /V /Y %PKG_DISTRO_DIR%\docs\RELEASE_NOTES.html %PKG_UPDATE_DIR%\release_notes.html /A | FINDSTR /i /v /r /c:"copied\>"
COPY /V /Y %PKG_DISTRO_DIR%\docs\RELEASE_NOTES.html %PKG_UPDATE_DIR%\release_notes_%LP3D_VERSION%.%LP3D_VER_REVISION%.html /A | FINDSTR /i /v /r /c:"copied\>"
COPY /V /Y %PKG_DISTRO_DIR%\docs\RELEASE_NOTES.html %PKG_DOWNLOAD_DIR%\release_notes.html /A | FINDSTR /i /v /r /c:"copied\>"

IF EXIST %PKG_DISTRO_DIR%\docs\COPYING_BRIEF (
  ECHO.
  ECHO - Set license file name...

  MOVE /Y %PKG_DISTRO_DIR%\docs\COPYING_BRIEF %PKG_DISTRO_DIR%\docs\COPYING.txt | FINDSTR /i /v /r /c:"moved\>"
)
EXIT /b

:GENERATENSISPARAMS
REM pwd = builds\windows\release\LP3D_PRODUCT_DIR
PUSHD "%CONFIGURATION%\%LP3D_PRODUCT_DIR%"
IF %LP3D_AMD_UNIVERSAL_BUILD% EQU 1 (
  SET LP3D_WIN64_3RDPARTY=%LP3D_PRODUCT%_x86_64\3rdParty
) ELSE (
  SET LP3D_WIN64_3RDPARTY=%LP3D_PRODUCT%_%LP3D_DISTRO_ARCH%\3rdParty
)
SET LP3D_WIN64_LDGLITE=%LP3D_WIN64_3RDPARTY%\%VER_LDGLITE%\bin\%LDGLITE_EXE%.exe
SET LP3D_WIN64_LDVIEW=%LP3D_WIN64_3RDPARTY%\%VER_LDVIEW%\bin\%LDVIEW_EXE%64.exe
SET LP3D_WIN64_LPUB3D_TRACE=%LP3D_WIN64_3RDPARTY%\%VER_LPUB3D_TRACE%\bin\%LPUB3D_TRACE_EXE%64.exe

SET LP3D_WIN32_3RDPARTY=%LP3D_PRODUCT%_x86\3rdParty
SET LP3D_WIN32_LDGLITE=%LP3D_WIN32_3RDPARTY%\%VER_LDGLITE%\bin\%LDGLITE_EXE%.exe
SET LP3D_WIN32_LDVIEW=%LP3D_WIN32_3RDPARTY%\%VER_LDVIEW%\bin\%LDVIEW_EXE%.exe
SET LP3D_WIN32_LPUB3D_TRACE=%LP3D_WIN32_3RDPARTY%\%VER_LPUB3D_TRACE%\bin\%LPUB3D_TRACE_EXE%32.exe
REM AppVersion.nsh pwd = builds\utilities\nsis-scripts
SET LP3D_BUILD_DIR=..\..\windows\%CONFIGURATION%\%LP3D_PRODUCT_DIR%\%LP3D_PRODUCT%
POPD

SET LP3D_LDGLITE_STATUS=Installed
SET LP3D_LDVIEW_STATUS=Installed
SET LP3D_LPUB3D_TRACE_STATUS=Installed
ECHO.
ECHO - Generate AppVersion.nsh build parameters script...
ECHO.
SETLOCAL ENABLEDELAYEDEXPANSION
IF %LP3D_AMD_UNIVERSAL_BUILD% EQU 1 (
  IF NOT EXIST "%LP3D_WIN64_LDGLITE%" ( SET LP3D_LDGLITE_STATUS=Not Found )
  IF NOT EXIST "%LP3D_WIN64_LDVIEW%" ( SET LP3D_LDVIEW_STATUS=Not Found )
  IF NOT EXIST "%LP3D_WIN64_LPUB3D_TRACE%" ( SET LP3D_LPUB3D_TRACE_STATUS=Not Found )

  ECHO   LP3D_WIN64_LDGLITE.........^(!LP3D_LDGLITE_STATUS!^)[%LP3D_WIN64_LDGLITE%]
  ECHO   LP3D_WIN64_LDVIEW..........^(!LP3D_LDVIEW_STATUS!^)[%LP3D_WIN64_LDVIEW%]
  ECHO   LP3D_WIN64_LPUB3D_TRACE....^(!LP3D_LPUB3D_TRACE_STATUS!^)[%LP3D_WIN64_LPUB3D_TRACE%]

  IF NOT EXIST "%LP3D_WIN32_LDGLITE%" ( SET LP3D_LDGLITE_STATUS=Not Found )
  IF NOT EXIST "%LP3D_WIN32_LDVIEW%" ( SET LP3D_LDVIEW_STATUS=Not Found )
  IF NOT EXIST "%LP3D_WIN32_LPUB3D_TRACE%" ( SET LP3D_LPUB3D_TRACE_STATUS=Not Found )

  ECHO   LP3D_WIN32_LDGLITE.........^(!LP3D_LDGLITE_STATUS!^)[%LP3D_WIN32_LDGLITE%]
  ECHO   LP3D_WIN32_LDVIEW..........^(!LP3D_LDVIEW_STATUS!^)[%LP3D_WIN32_LDVIEW%]
  ECHO   LP3D_WIN32_LPUB3D_TRACE....^(!LP3D_LPUB3D_TRACE_STATUS!^)[%LP3D_WIN32_LPUB3D_TRACE%]
) ELSE (
  IF "%LP3D_DISTRO_ARCH%" EQU "x86" (
    IF NOT EXIST "%LP3D_WIN32_LDGLITE%" ( SET LP3D_LDGLITE_STATUS=Not Found )
    IF NOT EXIST "%LP3D_WIN32_LDVIEW%" ( SET LP3D_LDVIEW_STATUS=Not Found )
    IF NOT EXIST "%LP3D_WIN32_LPUB3D_TRACE%" ( SET LP3D_LPUB3D_TRACE_STATUS=Not Found )

    ECHO   LP3D_WIN32_LDGLITE.........^(!LP3D_LDGLITE_STATUS!^)[%LP3D_WIN32_LDGLITE%]
    ECHO   LP3D_WIN32_LDVIEW..........^(!LP3D_LDVIEW_STATUS!^)[%LP3D_WIN32_LDVIEW%]
    ECHO   LP3D_WIN32_LPUB3D_TRACE....^(!LP3D_LPUB3D_TRACE_STATUS!^)[%LP3D_WIN32_LPUB3D_TRACE%]
  ) ELSE (
    IF NOT EXIST "%LP3D_WIN64_LDGLITE%" ( SET LP3D_LDGLITE_STATUS=Not Found )
    IF NOT EXIST "%LP3D_WIN64_LDVIEW%" ( SET LP3D_LDVIEW_STATUS=Not Found )
    IF NOT EXIST "%LP3D_WIN64_LPUB3D_TRACE%" ( SET LP3D_LPUB3D_TRACE_STATUS=Not Found )

    ECHO   LP3D_WIN64_LDGLITE.........^(!LP3D_LDGLITE_STATUS!^)[%LP3D_WIN64_LDGLITE%]
    ECHO   LP3D_WIN64_LDVIEW..........^(!LP3D_LDVIEW_STATUS!^)[%LP3D_WIN64_LDVIEW%]
    ECHO   LP3D_WIN64_LPUB3D_TRACE....^(!LP3D_LPUB3D_TRACE_STATUS!^)[%LP3D_WIN64_LPUB3D_TRACE%]
  )
  SET LP3D_OUTPUT_FILENAME=%LP3D_DISTRO_ARCH%-%LP3D_APP_VERSION_LONG%
)
SETLOCAL DISABLEDELAYEDEXPANSION

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
>>%genVersion% !define OutputFilename "%LP3D_OUTPUT_FILENAME%"
>>%genVersion% ; ${OutputFilename}
>>%genVersion%.
>>%genVersion% !define UniversalBuild %LP3D_AMD_UNIVERSAL_BUILD%
>>%genVersion% ; ${UniversalBuild}
>>%genVersion%.
>>%genVersion% !define ProductDir "..\..\windows\%CONFIGURATION%\%LP3D_PRODUCT_DIR%"
>>%genVersion% ; ${ProductDir}
>>%genVersion%.
>>%genVersion% !define OutFileDir "..\..\windows\%CONFIGURATION%\%LP3D_PRODUCT_DIR%"
>>%genVersion% ; ${OutFileDir} - Output path
>>%genVersion%.
>>%genVersion% !define LDGliteDir "%VER_LDGLITE%"
>>%genVersion% ; ${LDGliteDir}
>>%genVersion%.
>>%genVersion% !define LDGliteExe "%LDGLITE_EXE%"
>>%genVersion% ; ${LDGliteExe}
>>%genVersion%.
>>%genVersion% !define LDViewDir "%VER_LDVIEW%"
>>%genVersion% ; ${LDViewDir}
>>%genVersion%.
>>%genVersion% !define LDViewExe "%LDVIEW_EXE%"
>>%genVersion% ; ${LDViewExe}
>>%genVersion%.
>>%genVersion% !define LPub3DTraceDir "%VER_LPUB3D_TRACE%"
>>%genVersion% ; ${LPub3DTraceDir}
>>%genVersion%.
>>%genVersion% !define LPub3DTraceExe "%LPUB3D_TRACE_EXE%"
>>%genVersion% ; ${LPub3DTraceExe}
>>%genVersion%.
>>%genVersion% !define WinBuildDir "%LP3D_BUILD_DIR%_%LP3D_DISTRO_ARCH%"
>>%genVersion% ; ${WinBuildDir} - Input path for documents and extras - using %LP3D_DISTRO_ARCH% location
IF %LP3D_AMD_UNIVERSAL_BUILD% EQU 1 (
  >>%genVersion%.
  >>%genVersion% !define X86_INSTALL
  >>%genVersion% ; X86_INSTALL
  >>%genVersion%.
  >>%genVersion% !define X64_INSTALL
  >>%genVersion% ; X64_INSTALL
  >>%genVersion%.
  >>%genVersion% !define Win64BuildDir "%LP3D_BUILD_DIR%_x86_64"
  >>%genVersion% ; ${Win64BuildDir} - Input path for x86_64 architecture
  >>%genVersion%.
  >>%genVersion% !define Win32BuildDir "%LP3D_BUILD_DIR%_x86"
  >>%genVersion% ; ${Win32BuildDir} - Input path for x86 architecture
) ELSE (
  IF "%LP3D_DISTRO_ARCH%" EQU "x86" (
    >>%genVersion%.
    >>%genVersion% !define X86_INSTALL
    >>%genVersion% ; X86_INSTALL
    >>%genVersion%.
    >>%genVersion% !define Win32BuildDir "%LP3D_BUILD_DIR%_x86"
    >>%genVersion% ; ${Win32BuildDir} - Input path for x86 architecture
  ) ELSE (
    >>%genVersion%.
    >>%genVersion% !define X64_INSTALL
    >>%genVersion% ; X64_INSTALL
    IF "%LP3D_DISTRO_ARCH%" EQU "ARM64" (
      >>%genVersion%.
      >>%genVersion% !define ARM64_INSTALL
      >>%genVersion% ; ARM64_INSTALL
    )
    >>%genVersion%.
    >>%genVersion% !define Win64BuildDir "%LP3D_BUILD_DIR%_%LP3D_DISTRO_ARCH%"
    >>%genVersion% ; ${Win64BuildDir} - Input path for %LP3D_DISTRO_ARCH% architecture
  )
)
IF "%OPENSSL_VER%" EQU "v1.1" (
  IF %LP3D_AMD_UNIVERSAL_BUILD% EQU 1 (
    IF %WIN64_SSL% == 1 (
      >>%genVersion%.
      >>%genVersion% !define WIN64_SSL
      >>%genVersion% ; WIN64_SSL
      >>%genVersion%.
      >>%genVersion% !define OpenSSL64LibCrypto "libcrypto-1_1-x64.dll"
      >>%genVersion% ; ${OpenSSL64LibCrypto}
      >>%genVersion%.
      >>%genVersion% !define OpenSSL64LibSSL "libssl-1_1-x64.dll"
      >>%genVersion% ; ${OpenSSL64LibSSL}
    )
    >>%genVersion%.
    >>%genVersion% !define OpenSSL32LibCrypto "libcrypto-1_1.dll"
    >>%genVersion% ; ${OpenSSL32LibCrypto}
    >>%genVersion%.
    >>%genVersion% !define OpenSSL32LibSSL "libssl-1_1.dll"
    >>%genVersion% ; ${OpenSSL32LibSSL}
  ) ELSE (
    IF "%LP3D_DISTRO_ARCH%" EQU "x86" (
      >>%genVersion%.
      >>%genVersion% !define OpenSSL32LibCrypto "libcrypto-1_1.dll"
      >>%genVersion% ; ${OpenSSL32LibCrypto}
      >>%genVersion%.
      >>%genVersion% !define OpenSSL32LibSSL "libssl-1_1.dll"
      >>%genVersion% ; ${OpenSSL32LibSSL}
    ) ELSE (
      IF %WIN64_SSL% == 1 (
        >>%genVersion%.
        >>%genVersion% !define OpenSSL64LibCrypto "libcrypto-1_1-x64.dll"
        >>%genVersion% ; ${OpenSSL64LibCrypto}
        >>%genVersion%.
        >>%genVersion% !define OpenSSL64LibSSL "libssl-1_1-x64.dll"
        >>%genVersion% ; ${OpenSSL64LibSSL}
      )
    )
  )
)
IF "%OPENSSL_VER%" EQU "v1.0" (
  IF %LP3D_AMD_UNIVERSAL_BUILD% EQU 1 (
    >>%genVersion%.
    >>%genVersion% !define OpenSSL64LibCrypto "libeay32.dll"
    >>%genVersion% ; ${OpenSSL64LibCrypto}
    >>%genVersion%.
    >>%genVersion% !define OpenSSL64LibSSL "ssleay32.dll"
    >>%genVersion% ; ${OpenSSL64LibSSL}
    >>%genVersion%.
    >>%genVersion% !define OpenSSL32LibCrypto "libeay32.dll"
    >>%genVersion% ; ${OpenSSL32LibCrypto}
    >>%genVersion%.
    >>%genVersion% !define OpenSSL32LibSSL "ssleay32.dll"
    >>%genVersion% ; ${OpenSSL32LibSSL}
  ) ELSE (
    IF "%LP3D_DISTRO_ARCH%" EQU "x86" (
      >>%genVersion%.
      >>%genVersion% !define OpenSSL32LibCrypto "libeay32.dll"
      >>%genVersion% ; ${OpenSSL32LibCrypto}
      >>%genVersion%.
      >>%genVersion% !define OpenSSL32LibSSL "ssleay32.dll"
      >>%genVersion% ; ${OpenSSL32LibSSL}
    ) ELSE (
      >>%genVersion%.
      >>%genVersion% !define OpenSSL64LibCrypto "libeay32.dll"
      >>%genVersion% ; ${OpenSSL64LibCrypto}
      >>%genVersion%.
      >>%genVersion% !define OpenSSL64LibSSL "ssleay32.dll"
      >>%genVersion% ; ${OpenSSL64LibSSL}
    )
  )
)
>>%genVersion%.
>>%genVersion% !define LPub3DBuildFile "%LPUB3D_BUILD_FILE%"
>>%genVersion% ; ${LPub3DBuildFile}
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
REM IF EXIST "%versionFile%" (
REM   ECHO - Generated AppVersion.nsh build parameters script:
REM   TYPE "%versionFile%"
REM )
EXIT /b

REM pwd = windows/release/LP3D_PRODUCT_DIR
:NSISBUILD
IF %RUN_NSIS% == 1 ECHO.
IF %RUN_NSIS% == 1 ECHO - Start NSIS Master Installer Build...

IF %RUN_NSIS% == 0 ECHO.
IF %RUN_NSIS% == 0 ECHO - Ignore NSIS Master Installer Build

IF %RUN_NSIS% == 1 "%NSIS_UTILITY%" /DDownloadMaster ..\..\..\utilities\nsis-scripts\LPub3DNoPack.nsi | FINDSTR /i /r /c:"^Processing\>" /c:"^Output\>"

IF %RUN_NSIS% == 1 MOVE /Y    %LP3D_DOWNLOAD_PRODUCT%.exe %PKG_DOWNLOAD_DIR%\ | FINDSTR /i /v /r /c:"moved\>"

IF %RUN_NSIS% == 1 ECHO   Finished NSIS Master Installer Build
EXIT /b

:CREATEPORTABLEDISTRO
IF %CREATE_PORTABLE% == 1 ECHO.
IF %CREATE_PORTABLE% == 1 ECHO - Create %LP3D_PRODUCT% %LP3D_DISTRO_ARCH% portable install archive package file...

IF %CREATE_PORTABLE% == 0 ECHO.
IF %CREATE_PORTABLE% == 0 ECHO - Ignore creating %LP3D_PRODUCT% %LP3D_DISTRO_ARCH% portable install archive package file

IF %CREATE_PORTABLE% == 1 (
  IF %LP3D_VALID_TAR% == 1 (
    PUSHD %PKG_DISTRO_DIR%\..
    %ZIP_UTILITY% -acf %PKG_DOWNLOAD_DIR%\%PKG_DISTRO_PORTABLE_DIR%.zip %LP3D_PRODUCT%_%LP3D_DISTRO_ARCH%
    POPD
  ) ELSE (
    IF %LP3D_VALID_7ZIP% == 1 (
      %ZIP_UTILITY% a -tzip %PKG_DOWNLOAD_DIR%\%PKG_DISTRO_PORTABLE_DIR%.zip %PKG_DISTRO_DIR%\ | FINDSTR /i /r /c:"^Creating\>" /c:"^Everything\>"
    )
  )
)
EXIT /b

:SIGNAPP
IF /I "%GITHUB%" EQU "True" (
  ECHO.
  ECHO * Application sha512 hash signed at end of build.
  EXIT /b
)

IF /I "%APPVEYOR%" EQU "True" (
  ECHO.
  ECHO * Application sha512 hash signed at end of build.
  EXIT /b
)

IF %SIGN_APP% == 1 ECHO.
IF %SIGN_APP% == 1 ECHO - Start Application Code Signing...

IF %SIGN_APP% == 0 ECHO.
IF %SIGN_APP% == 0 ECHO - Ignore Application Code Signing

IF %SIGN_APP% == 1 "%SIGN_UTILITY%" sign /tr %TimeStamp% /td %Sha2% /fd %Sha2% /f %PKey% /p %PwD% %PKG_DOWNLOAD_DIR%\%LP3D_DOWNLOAD_PRODUCT%.exe
IF %LP3D_AMD_UNIVERSAL_BUILD% EQU 1 (
  IF %SIGN_APP% == 1 "%SIGN_UTILITY%" sign /tr %TimeStamp% /td %Sha2% /fd %Sha2% /f %PKey% /p %PwD% %LP3D_PRODUCT%_x86\%LPUB3D_BUILD_FILE%
  IF %SIGN_APP% == 1 "%SIGN_UTILITY%" sign /tr %TimeStamp% /td %Sha2% /fd %Sha2% /f %PKey% /p %PwD% %LP3D_PRODUCT%_x86_64\%LPUB3D_BUILD_FILE%
) ELSE (
  IF %SIGN_APP% == 1 "%SIGN_UTILITY%" sign /tr %TimeStamp% /td %Sha2% /fd %Sha2% /f %PKey% /p %PwD% %PKG_DISTRO_DIR%\%LPUB3D_BUILD_FILE%
)

IF %SIGN_APP% == 1 ECHO.
IF %SIGN_APP% == 1 ECHO - Generate Code Signing Hash Checksum listing...

IF %SIGN_APP% == 1 CertUtil -hashfile %PKG_DOWNLOAD_DIR%\%LP3D_DOWNLOAD_PRODUCT%.exe SHA256  >  %PKG_DOWNLOAD_DIR%\LPub3D.%LP3D_VERSION%.Checksums.txt
IF %LP3D_AMD_UNIVERSAL_BUILD% EQU 1
  IF %SIGN_APP% == 1 CertUtil -hashfile %LP3D_PRODUCT%_x86\%LPUB3D_BUILD_FILE%              >>  %PKG_DOWNLOAD_DIR%\LPub3D.%LP3D_VERSION%.Checksums.txt
  IF %SIGN_APP% == 1 CertUtil -hashfile %LP3D_PRODUCT%_x86_64\%LPUB3D_BUILD_FILE%           >>  %PKG_DOWNLOAD_DIR%\LPub3D.%LP3D_VERSION%.Checksums.txt
) ELSE
  IF %SIGN_APP% == 1 CertUtil -hashfile %PKG_DISTRO_DIR%\%LPUB3D_BUILD_FILE% SHA256         >>  %PKG_DOWNLOAD_DIR%\LPub3D.%LP3D_VERSION%.Checksums.txt
)

IF %SIGN_APP% == 1 ECHO   Finished Application Code Signing
EXIT /b

:GENERATE_JSON
ECHO.
ECHO - Generate update package alternate version json inserts...
SETLOCAL ENABLEDELAYEDEXPANSION
SET /A updateCount=0
FOR %%e IN ( %LP3D_DIST_EXTENSIONS% ) DO (
 SET /A updateCount+=1
 CALL :GENERATE_ALT_VERSION_INSERTS %%e !updateCount!
)
SETLOCAL DISABLEDELAYEDEXPANSION

ECHO.
ECHO - Generate update package lpub3dupdates.json template file...

SET updateName=lpub3dupdates
SET updatesFile=%PKG_UPDATE_DIR%\%updateName%.json
SET genLPub3DUpdates=%updatesFile% ECHO

:GENERATE LPub3D lpub3dupdates.json template file
>%genLPub3DUpdates% {
>>%genLPub3DUpdates%   "_comment": "LPub3D application updates %updateName%.json generated on %LP3D_DATE_TIME%",
>>%genLPub3DUpdates%   "updates": {
SETLOCAL ENABLEDELAYEDEXPANSION
SET /A updateCount=0
FOR %%b IN ( %LP3D_DIST_EXTENSIONS% ) DO (
  SET /A updateCount+=1
  CALL :GENERATE_UPDATES_JSON %updateName% %%b !updateCount!
)
SETLOCAL DISABLEDELAYEDEXPANSION
>>%genLPub3DUpdates%   }
>>%genLPub3DUpdates% }
>>%genLPub3DUpdates%.

ECHO.
ECHO - Merge update package version inserts into %updateName%.json...
(
  SETLOCAL ENABLEDELAYEDEXPANSION
  FOR /F "tokens=*" %%i IN (%PKG_UPDATE_DIR%\%updateName%.json) DO (
    SET /A updateCount=0
    FOR %%e IN ( %LP3D_DIST_EXTENSIONS% ) DO (
      SET /A updateCount+=1
      IF !updateCount! GTR 8 (
        SET DIST_ID=linux
      ) ELSE (
        IF !updateCount! GTR 5 (SET DIST_ID=macos) ELSE (SET DIST_ID=windows)
      )
      IF "%%i" EQU ""alt-version-gen-placeholder-!DIST_ID!-%%e": {}" (
        TYPE %PKG_UPDATE_DIR%\versionInsert_%%e.txt
        SET SKIP_TOKEN=1
      )
    )
    IF NOT "!SKIP_TOKEN!" EQU "1" (ECHO %%i)
    SET SKIP_TOKEN=
  )
  SETLOCAL DISABLEDELAYEDEXPANSION
) >temp.txt
MOVE /y temp.txt %PKG_UPDATE_DIR%\%updateName%.json | FINDSTR /i /v /r /c:"moved\>"
FOR %%e IN ( %LP3D_DIST_EXTENSIONS% ) DO (
  DEL /Q %PKG_UPDATE_DIR%\versionInsert_%%e.txt
)

ECHO.
ECHO - Generate LDraw library json FILES to media folder...

REM pwd = windows\release\LP3D_PRODUCT_DIR
SET updateName=complete
SET updatesFile=%PKG_UPDATE_DIR%\%updateName%.json
SET genLPub3DUpdates=%updatesFile% ECHO

:GENERATE LDRaw official library json file
>%genLPub3DUpdates% {
>>%genLPub3DUpdates%   "_comment": "LPub3D LDraw library %updateName%.json generated on %LP3D_DATE_TIME%",
>>%genLPub3DUpdates%   "updates": {
SETLOCAL ENABLEDELAYEDEXPANSION
SET /A updateCount=0
FOR %%b IN ( %LP3D_DIST_EXTENSIONS% ) DO (
  SET /A updateCount+=1
  CALL :GENERATE_LIBRARY_JSON %updateName% %%b !updateCount!
)
SETLOCAL DISABLEDELAYEDEXPANSION
>>%genLPub3DUpdates%   }
>>%genLPub3DUpdates% }
>>%genLPub3DUpdates%.

SET updateName=lpub3dldrawunf
SET updatesFile=%PKG_UPDATE_DIR%\%updateName%.json
SET genLPub3DUpdates=%updatesFile% ECHO

:GENERATE LDRaw unofficial library json file
>%genLPub3DUpdates% {
>>%genLPub3DUpdates%   "_comment": "LPub3D lpub3dupdates.json generated on %LP3D_DATE_TIME%",
>>%genLPub3DUpdates%   "updates": {
SETLOCAL ENABLEDELAYEDEXPANSION
SET /A updateCount=0
FOR %%b IN ( %LP3D_DIST_EXTENSIONS% ) DO (
  SET /A updateCount+=1
  CALL :GENERATE_LIBRARY_JSON %updateName% %%b !updateCount!
)
SETLOCAL DISABLEDELAYEDEXPANSION
>>%genLPub3DUpdates%   }
>>%genLPub3DUpdates% }
>>%genLPub3DUpdates%.

ECHO.
ECHO - Generate latest.txt version input file (for backward compatability)...

SET latestFile=%PKG_UPDATE_DIR%\latest.txt
SET genLatest=%latestFile% ECHO
:GENERATE latest.txt file
>%genLatest% %LP3D_VERSION%
EXIT /b

:GENERATE_UPDATES_JSON
IF %3 GTR 8 (
  SET DIST_ID=linux
) ELSE (
  IF %3 GTR 5 (SET DIST_ID=macos) ELSE (SET DIST_ID=windows)
)
SET "LP3D_EXT=%2"
SET "LP3D_AVAILABLE_VERSIONS=LP3D_AVAILABLE_VERSIONS_%LP3D_EXT%"
SET LP3D_UPDATE_ARCH=%LP3D_X64_ARCH%
IF %3 EQU  2 (SET LP3D_UPDATE_ARCH=%LP3D_ARM_ARCH%)
IF %3 EQU  6 (SET LP3D_UPDATE_ARCH=%LP3D_ARM_ARCH%)
IF %3 EQU 10 (SET LP3D_UPDATE_ARCH=%LP3D_ARM_ARCH%)
IF %3 EQU 17 (SET LP3D_UPDATE_ARCH=%LP3D_AARCH64_ARCH%)
IF %3 EQU 20 (SET LP3D_UPDATE_ARCH=%LP3D_ARM_ARCH%)
IF %3 EQU 23 (SET LP3D_UPDATE_ARCH=%LP3D_ARM_ARCH%)
REM LP3D_EXT expands to the file extension - if %1 is 'api', %LP3D_EXT% is '-api.AppImage'
CALL SET "LP3D_DIST_SUFFIX=%%%LP3D_EXT%%%"
CALL SET "LP3D_AVAILABLE_VERSIONS=%%%LP3D_AVAILABLE_VERSIONS%%%"

SET updatesFile=%PKG_UPDATE_DIR%\%1.json
SET genLPub3D1Updates=%updatesFile% ECHO

:GENERATE Generate LPub3D application updates json file
IF %3 EQU  5 (>>%genLPub3DUpdates%     "_comment_exe": "legacy windows block",)         & ::windows-exe
IF %3 EQU  8 (>>%genLPub3DUpdates%     "_comment_dmg": "legacy macos block",)           & ::macos-dmg
IF %3 EQU 11 (>>%genLPub3DUpdates%     "_comment_deb": "legacy linux deb block",)       & ::linux-deb
IF %3 EQU 13 (>>%genLPub3DUpdates%     "_comment_rpm": "legacy linux rpm block",)       & ::linux-rpm
IF %3 EQU 15 (>>%genLPub3DUpdates%     "_comment_pkg": "legacy linux package block",)   & ::linux-pkg
IF %3 EQU 18 (>>%genLPub3DUpdates%     "_comment_api": "legacy linux app-image block",) & ::linux-api
IF %3 EQU 21 (>>%genLPub3DUpdates%     "_comment_snp": "legacy linux snap block",)      & ::linux-snp
IF %3 EQU 24 (>>%genLPub3DUpdates%     "_comment_flp": "legacy linux flatpak block",)   & ::linux-flp
>>%genLPub3DUpdates%     "%DIST_ID%-%LP3D_EXT%": {
>>%genLPub3DUpdates%       "open-url": "%LP3D_GITHUB_BASE%/releases/tag/%LP3D_VER_TAG_NAME%/",
>>%genLPub3DUpdates%       "latest-version": "%LP3D_VERSION%",
>>%genLPub3DUpdates%       "latest-revision": "%LP3D_VER_REVISION%",
IF %3 EQU 4 (
  ::windows-amd_msys_exe
  >>%genLPub3DUpdates%       "download-url": "%LP3D_GITHUB_BASE%/releases/download/%LP3D_VER_TAG_NAME%/mingw-w64-%LP3D_X64_ARCH%-lpub3d-%LP3D_APP_VERSION%%LP3D_DIST_SUFFIX%",
) ELSE (
  IF %3 EQU 2 (
    ::windows-arm_exe
    >>%genLPub3DUpdates%       "download-url": "%LP3D_GITHUB_BASE%/releases/download/%LP3D_VER_TAG_NAME%/LPub3D-%LP3D_UPDATE_ARCH%-%LP3D_APP_VERSION_LONG%%LP3D_DIST_SUFFIX%",
  ) ELSE (
    >>%genLPub3DUpdates%       "download-url": "%LP3D_GITHUB_BASE%/releases/download/%LP3D_VER_TAG_NAME%/LPub3D-%LP3D_APP_VERSION_LONG%%LP3D_DIST_SUFFIX%",
  )
)
REM Windows Portable Distros
ECHO.%2 | FINDSTR /I /R /C:"^exe" /C:"amd_exe" /C:"arm_exe" >NUL && (
  ECHO.%2 | FINDSTR /I /R /C:"^exe" /C:"amd_exe" >NUL && (
    >>%genLPub3DUpdates%       "x86-win-portable-download-url": "%LP3D_GITHUB_BASE%/releases/download/%LP3D_VER_TAG_NAME%/LPub3D_x86-%LP3D_APP_VERSION_LONG%.zip",
  )
  >>%genLPub3DUpdates%       "%LP3D_UPDATE_ARCH%-win-portable-download-url": "%LP3D_GITHUB_BASE%/releases/download/%LP3D_VER_TAG_NAME%/LPub3D_%LP3D_UPDATE_ARCH%-%LP3D_APP_VERSION_LONG%.zip",
)
>>%genLPub3DUpdates%       "changelog-url": "%LP3D_SOURCEFORGE_UPDATE_BASE%/release_notes_%LP3D_VERSION%.%LP3D_VER_REVISION%.html",
>>%genLPub3DUpdates%       "available-versions": "%LP3D_AVAILABLE_VERSIONS%",
>>%genLPub3DUpdates%       "alt-version-gen-placeholder-%DIST_ID%-%LP3D_EXT%": {}
IF %3 EQU %LP3D_UPDATE_SIZE% (
  >>%genLPub3DUpdates%     }
) ELSE (
  >>%genLPub3DUpdates%     },
)
EXIT /b

:GENERATE_LIBRARY_JSON
IF "%1" EQU "complete" (
  SET DIR=updates
  SET NAME=%1
) ELSE (
  SET DIR=unofficial
  SET NAME=ldrawunf
)
IF %3 GTR 8 (
  SET DIST_ID=linux
) ELSE (
  IF %3 GTR 5 (SET DIST_ID=macos) ELSE (SET DIST_ID=windows)
)

SET updatesFile=%PKG_UPDATE_DIR%\%1.json
SET genLPub3D1Updates=%updatesFile% ECHO

:GENERATE Generate LDraw library json file
>>%genLPub3DUpdates%     "%DIST_ID%-%2": {
>>%genLPub3DUpdates%       "ownload-name": "%1.zip",
>>%genLPub3DUpdates%       "open-url": "https://library.ldraw.org/",
>>%genLPub3DUpdates%       "download-url": "https://library.ldraw.org/library/%DIR%/%NAME%.zip"
IF %3 EQU %LP3D_UPDATE_SIZE% (
>>%genLPub3DUpdates%     }
) ELSE (
>>%genLPub3DUpdates%     },
)
EXIT /b

:GENERATE_ALT_VERSION_INSERTS
SET "LP3D_EXT=%1"
SET LP3D_MASTER=1
IF "%LP3D_EXT%" EQU "exe" (SET "LP3D_EXT=win_exe") & ::Replace 'exe' to avoid 'exe=.exe' which expands to 'exeexe'
SET "LP3D_ALT_VERS=LP3D_ALTERNATE_VERSIONS_%1"
REM LP3D_ALT_VERS expands to the alternate versions for a given LP3D_EXTension
CALL SET "LP3D_ALTERNATE_VERSIONS=%%%LP3D_ALT_VERS%%%"
SET "LP3D_DIST_PREFIX=LPub3D-"
ECHO.%1 | FINDSTR /I /R /C:"api" /C:"snp" /C:"flp" /C:"con_exe" /C:"msys_exe" /C:"arm_" >NUL && (SET LP3D_MASTER=0)
IF %LP3D_MASTER% EQU 1 (SET "LP3D_DIST_PREFIX=LPub3D-UpdateMaster_")
SET versionInsert=%PKG_UPDATE_DIR%\versionInsert_%1.txt
SET genVersionInsert=%versionInsert% ECHO
SETLOCAL ENABLEDELAYEDEXPANSION
REM Alternate versions <current version>,2.3.6,2.0.20
SET /A LP3D_ALTERNATE_SIZE=0
SET /A LP3D_ALTERNATE_COUNT=0
FOR %%C IN ( %LP3D_ALTERNATE_VERSIONS% ) DO (SET /A LP3D_ALTERNATE_SIZE+=1)
FOR %%V IN ( %LP3D_ALTERNATE_VERSIONS% ) DO (
  SET /A LP3D_ALTERNATE_COUNT+=1
  REM Strip dots from version and add trailing zeros to 4 numbers
  SET "L=0"
  SET "I=%%V"
  SET "I=!I:.=!"
  CALL :VERSIONLENGTH I L
  SET "Z=0000"
  SET "P=!I!!Z!"
  SET "I=!P:~0,4!"
  >>%genVersionInsert% "alternate-version-%%V-%1": {
  IF !I! LEQ 2020 (
    REM Extension format for version 2.0.20 or earlier using SourceForge
    SET "pkg=.645_%LP3D_X64_ARCH%.pkg.tar.xz"
    SET "amd_pkg=!pkg!"
    SET "deb=_0ubuntu1_%LP3D_AMD_ARCH%.deb"
    SET "amd_deb=!deb!"
    SET "rpm=_1fedora.%LP3D_X64_ARCH%.rpm"
    SET "amd_rpm=!rpm!"
    SET "dmg=_osx.dmg"
    SET "amd_dmg=!dmg!"
    REM LP3D_EXT expands to the file extension - if %1 is 'api', %LP3D_EXT% is '-api.AppImage'
    CALL SET "LP3D_DIST_SUFFIX=%%%LP3D_EXT%%%"
    SET LP3D_ALT_VERSION_LONG=%%V
    IF !I! LSS 2020 (
      >>%genVersionInsert%   "open-url": "https://trevorsandy.github.io/lpub3d/",
    ) ELSE (
      >>%genVersionInsert%   "open-url": "%LP3D_GITHUB_BASE%/releases/tag/v%%V/",
    )
    >>%genVersionInsert%   "latest-version": "%%V",
    >>%genVersionInsert%   "download-url": "%LP3D_SOURCEFORGE_UPDATE_BASE%/%LP3D_DIST_PREFIX%!LP3D_ALT_VERSION_LONG!!LP3D_DIST_SUFFIX!",
    IF !I! EQU 2020 (
      ECHO.%1 | FINDSTR /I /R /C:"^exe" /C:"amd_exe" >NUL && (
        SET LP3D_ALT_VERSION_LONG=%%V.0.645_20170208
        >>%genVersionInsert%   "x86-win-portable-download-url": "%LP3D_GITHUB_BASE%/releases/download/v%%V/LPub3D_x86-!LP3D_ALT_VERSION_LONG!.zip",
        >>%genVersionInsert%   "%LP3D_X64_ARCH%-win-portable-download-url": "%LP3D_GITHUB_BASE%/releases/download/v%%V/LPub3D_%LP3D_X64_ARCH%-!LP3D_ALT_VERSION_LONG!.zip",
      )
    )
    >>%genVersionInsert%   "changelog-url": "%LP3D_SOURCEFORGE_UPDATE_BASE%/change_log_%%V.txt"
    IF !LP3D_ALTERNATE_COUNT! EQU %LP3D_ALTERNATE_SIZE% (
      >>%genVersionInsert% }
    ) ELSE (
      >>%genVersionInsert% },
    )
  ) ELSE (
    >>%genVersionInsert%   "open-url": "%LP3D_GITHUB_BASE%/releases/tag/v%%V/",
    >>%genVersionInsert%   "latest-version": "%%V",
    REM Extension format from version 2.1.0 or greater
    IF !I! EQU 2360 (
      SET "deb=-xenial-%LP3D_AMD_ARCH%.deb"
      SET "amd_deb=!deb!
      SET "rpm=-1.fc26.%LP3D_X64_ARCH%.rpm"
      SET "amd_rpm=!rpm!
      SET "amd_dmg=%dmg%"
      SET LP3D_ALT_VERSION_LONG=%%V
    ) ELSE (
      IF "%1" EQU "arm_exe" (
        SET LP3D_ALT_VERSION_LONG=%LP3D_ARM_ARCH%-%LP3D_APP_VERSION_LONG%
      ) ELSE (
        IF "%1" EQU "amd_msys_exe" (
          SET LP3D_ALT_VERSION_LONG=%LP3D_APP_VERSION%
        ) ELSE (
          SET LP3D_ALT_VERSION_LONG=%LP3D_APP_VERSION_LONG%
        )
      )
    )
    IF !I! GEQ 2400 (
      SET "LP3D_DOWNLOAD_BASE=%LP3D_GITHUB_BASE%/releases/download/v%%V"
    ) ELSE (
      SET "LP3D_DOWNLOAD_BASE=%LP3D_SOURCEFORGE_UPDATE_BASE%"
    )
    CALL SET "LP3D_DIST_SUFFIX=%%%LP3D_EXT%%%"
    IF "%1" EQU "amd_msys_exe" (
      >>%genVersionInsert%   "download-url": "!LP3D_DOWNLOAD_BASE!/mingw-w64-%LP3D_X64_ARCH%-lpub3d-!LP3D_ALT_VERSION_LONG!!LP3D_DIST_SUFFIX!",
    ) ELSE (
      >>%genVersionInsert%   "download-url": "!LP3D_DOWNLOAD_BASE!/!LP3D_DIST_PREFIX!!LP3D_ALT_VERSION_LONG!!LP3D_DIST_SUFFIX!",
      IF "%1" EQU "arm_exe" (
        >>%genVersionInsert%   "%LP3D_ARM_ARCH%-win-portable-download-url": "%LP3D_GITHUB_BASE%/releases/download/v%%V/LPub3D_%LP3D_ARM_ARCH%-!LP3D_ALT_VERSION_LONG!.zip",
      ) ELSE (
        ECHO.%1 | FINDSTR /I /R /C:"^exe" /C:"amd_exe" >NUL && (
          >>%genVersionInsert%   "x86-win-portable-download-url": "%LP3D_GITHUB_BASE%/releases/download/v%%V/LPub3D_x86-!LP3D_ALT_VERSION_LONG!.zip",
          >>%genVersionInsert%   "%LP3D_X64_ARCH%-win-portable-download-url": "%LP3D_GITHUB_BASE%/releases/download/v%%V/LPub3D_%LP3D_X64_ARCH%-!LP3D_ALT_VERSION_LONG!.zip",
        )
      )
    )
    >>%genVersionInsert%   "changelog-url": "%LP3D_SOURCEFORGE_UPDATE_BASE%/release_notes_%%V.html"
    IF !LP3D_ALTERNATE_COUNT! EQU %LP3D_ALTERNATE_SIZE% (
      >>%genVersionInsert% }
    ) ELSE (
      >>%genVersionInsert% },
    )
  )
)
SETLOCAL DISABLEDELAYEDEXPANSION
IF EXIST "%versionInsert%" (
  ECHO   Generated %2 %1 json version insert.
) ELSE (
  ECHO   %2 %1 json version insert [%versionInsert%] was not generated.
)
EXIT /b

:VERSIONLENGTH
REM %1 = input var, %2 = output var
SETLOCAL ENABLEDELAYEDEXPANSION
SET "IN=!%~1!"
SET "LEN=0"
FOR %%A IN (!IN!) DO (
  SET /A LEN+=1
)
:ENDLOOP
( ENDLOCAL & SET "%~2=%LEN%" )
EXIT /b

:DOWNLOADOPENSSLLIBS
ECHO.
ECHO - Download Windows %OPENSSL_LIB% libraries for platform [%LP3D_DISTRO_ARCH%]...

IF /I "%GITHUB%" EQU "True" (
  SET DIST_DIR=%LP3D_DIST_DIR_PATH%
)
IF /I "%APPVEYOR%" EQU "True" (
  SET DIST_DIR=%LP3D_DIST_DIR_PATH%
)
IF /I "%GITHUB%" NEQ "True" (
  IF /I "%APPVEYOR%" NEQ "True" (
    CALL :DIST_DIR_REL_TO_ABS ..\..\..\..\..\lpub3d_windows_3rdparty
  )
)

IF NOT EXIST "%DIST_DIR%\" (
  ECHO.
  ECHO - ERROR - Could not locate distribution path [%DIST_DIR%]
  EXIT /b
)

SET OPENSSL_LIB_DIR=%DIST_DIR%\%OPENSSL_LIB%\%OPENSSL_VER%\%LP3D_DISTRO_ARCH%

IF NOT EXIST "%OPENSSL_LIB_DIR%\" (
  ECHO.
  ECHO - Create %OPENSSL_LIB% libraries store %OPENSSL_LIB_DIR%
  MKDIR "%OPENSSL_LIB_DIR%\"
)

SET OpensslCONTENT=unknown
SET OpensslVERIFIED=unknown
SET OutputPATH=%OPENSSL_LIB_DIR%

CALL :CREATEWEBCONTENTDOWNLOADVBS

REM For libEAY
IF "%OPENSSL_VER%" EQU "v1.0" (
  SET OpensslCONTENT=libeay32-x64.dll
  SET OpensslVERIFIED=libeay32.dll
  IF "%LP3D_DISTRO_ARCH%" EQU "x86" (
    SET OpensslCONTENT=libeay32.dll
    SET OpensslVERIFIED=libeay32.dll
  )
)
IF "%OPENSSL_VER%" EQU "v1.1" (
  SET OpensslCONTENT=libcrypto-1_1-x64.dll
  SET OpensslVERIFIED=libcrypto-1_1-x64.dll
  IF "%LP3D_DISTRO_ARCH%" EQU "x86" (
    SET OpensslCONTENT=libcrypto-1_1.dll
    SET OpensslVERIFIED=libcrypto-1_1.dll
  )
)

ECHO.
ECHO - Library %OpensslVERIFIED% download path: [%OutputPATH%\%OpensslCONTENT%]

IF NOT EXIST "%OutputPATH%\%OpensslVERIFIED%" (
  CALL :GET_OPENSSL_LIB
)  ELSE (
  ECHO.
  ECHO - %OPENSSL_LIB% %OPENSSL_VER% %LP3D_DISTRO_ARCH% library %OpensslVERIFIED% exist. Nothing to do.
)

CALL :SET_OPENSSL_LIB

REM For libSSL
IF "%OPENSSL_VER%" EQU "v1.0" (
  SET OpensslCONTENT=ssleay32-x64.dll
  SET OpensslVERIFIED=ssleay32.dll
  IF "%LP3D_DISTRO_ARCH%" EQU "x86" (
    SET OpensslCONTENT=ssleay32.dll
    SET OpensslVERIFIED=ssleay32.dll
  )
)
IF "%OPENSSL_VER%" EQU "v1.1" (
  SET OpensslCONTENT=libssl-1_1-x64.dll
  SET OpensslVERIFIED=libssl-1_1-x64.dll
  IF "%LP3D_DISTRO_ARCH%" EQU "x86" (
    SET OpensslCONTENT=libssl-1_1.dll
    SET OpensslVERIFIED=libssl-1_1.dll
  )
)

ECHO.
ECHO - Library %OpensslVERIFIED% download path: [%OutputPATH%\%OpensslCONTENT%]

IF NOT EXIST "%OutputPATH%\%OpensslVERIFIED%" (
  CALL :GET_OPENSSL_LIB
)  ELSE (
  ECHO.
  ECHO - %OPENSSL_LIB% %OPENSSL_VER% %LP3D_DISTRO_ARCH% library %OpensslVERIFIED% exist. Nothing to do.
)

CALL :SET_OPENSSL_LIB
EXIT /b

:GET_OPENSSL_LIB
SET WebCONTENT="%OutputPATH%\%OpensslCONTENT%"
SET WebNAME=%LP3D_LIBS_BASE%/%OpensslCONTENT%

ECHO.
cscript //Nologo %TEMP%\$\%vbs% %WebNAME% %WebCONTENT% && @ECHO off

IF NOT EXIST "%OutputPATH%\%OpensslCONTENT%" (
  ECHO.
  ECHO - ERROR - %OPENSSL_LIB% library %OpensslCONTENT% %OPENSSL_VER% %LP3D_DISTRO_ARCH% not downloaded
)

IF "%OPENSSL_VER%" EQU "v1.0" (
IF "%LP3D_DISTRO_ARCH%" EQU "x86_64" (
    ECHO.
    ECHO - Renaming library file "%OpensslCONTENT%" to "%OpensslVERIFIED%"
    REN "%WebCONTENT%" %OpensslVERIFIED%
    ECHO.
    IF EXIST "%OutputPATH%\%OpensslVERIFIED%" (
      ECHO - %OPENSSL_LIB% library %OpensslVERIFIED% %OPENSSL_VER% %LP3D_DISTRO_ARCH% is available
    ) ELSE (
      ECHO - ERROR - %OPENSSL_LIB% library %OpensslVERIFIED% %OPENSSL_VER% %LP3D_DISTRO_ARCH% not found
    )
  )
)
EXIT /b

:SET_OPENSSL_LIB
SET PKG_TARGET_DIR=%WIN_PKG_DIR%\%CONFIGURATION%\%LP3D_PRODUCT_DIR%\%LP3D_PRODUCT%_%LP3D_DISTRO_ARCH%

IF NOT EXIST "%PKG_TARGET_DIR%\" (
  ECHO.
  ECHO - ERROR - %PKG_TARGET_DIR% does not exist...
)

IF NOT EXIST "%PKG_TARGET_DIR%\%OpensslVERIFIED%" (
  ECHO.
  ECHO - Copying %OPENSSL_LIB% library %OpensslVERIFIED% %OPENSSL_VER% for Architecture %LP3D_DISTRO_ARCH%...
  ECHO - From: %OPENSSL_LIB_DIR%
  ECHO - To:   %PKG_TARGET_DIR%
  IF EXIST "%OPENSSL_LIB_DIR%\%OpensslVERIFIED%" (
    COPY /V /Y "%OPENSSL_LIB_DIR%\%OpensslVERIFIED%" "%PKG_TARGET_DIR%\"  /A | FINDSTR /i /v /r /c:"copied\>"
    ECHO - %OpensslVERIFIED% copied to "%PKG_TARGET_DIR%\%OpensslVERIFIED%"
  ) ELSE (
    ECHO.
    ECHO - ERROR - %OPENSSL_LIB% library %OpensslVERIFIED% %OPENSSL_VER% %LP3D_DISTRO_ARCH% does not exist in %OPENSSL_LIB_DIR%\.
  )
) ELSE (
  ECHO.
  ECHO - %OPENSSL_LIB% library %OpensslVERIFIED% %OPENSSL_VER% %LP3D_DISTRO_ARCH% exist in %PKG_TARGET_DIR%. Nothing to do.
)
EXIT /b

:DOWNLOADMSVCREDIST
SET LP3D_DISTRO_ARCH=%2
IF /I "%LP3D_DISTRO_ARCH%" EQU "x86" SET _AR=%LP3D_DISTRO_ARCH%
IF /I "%LP3D_DISTRO_ARCH%" EQU "x86_64" SET _AR=x64
IF /I "%LP3D_DISTRO_ARCH%" EQU "ARM64" SET _AR=arm64
ECHO.
ECHO - Download Microsoft Visual C++ MSVC %1 %LP3D_DISTRO_ARCH% Redistributable...

IF /I "%GITHUB%" EQU "True" (
  SET "DIST_DIR=%LP3D_DIST_DIR_PATH%"
)
IF /I "%APPVEYOR%" EQU "True" (
  SET "DIST_DIR=%LP3D_DIST_DIR_PATH%"
)
IF /I "%GITHUB%" NEQ "True" (
  IF /I "%APPVEYOR%" NEQ "True" (
    CALL :DIST_DIR_REL_TO_ABS ..\..\..\..\..\lpub3d_windows_3rdparty
  )
)

IF NOT EXIST "%DIST_DIR%\" (
  ECHO.
  ECHO - ERROR - Could not locate distribution path ["%DIST_DIR%"]
  EXIT /b
)

SET "MSVC_REDIST_DIR=%DIST_DIR%\vcredist"

IF NOT EXIST "%MSVC_REDIST_DIR%\" (
  ECHO.
  ECHO - Create MSVC %1 %LP3D_DISTRO_ARCH% Redistributable store %MSVC_REDIST_DIR%
  MKDIR "%MSVC_REDIST_DIR%\"
)

SET "OutputPATH=%MSVC_REDIST_DIR%"
IF /I "%LP3D_DISTRO_ARCH%" == "ARM64" (
  SET RedistCONTENT=vc_redist_2022_%_AR%.exe
) ELSE (
  SET RedistCONTENT=vc_redist_2015_2022_%_AR%.exe
)

SET Redistributable=vc_redist.%_AR%.exe

ECHO.
ECHO - MSVC %1 %LP3D_DISTRO_ARCH% Redistributable download path: [%OutputPATH%]
IF NOT EXIST "%OutputPATH%\%Redistributable%" (
  CALL :CREATEWEBCONTENTDOWNLOADVBS
  CALL :GET_MSVC_REDIST %1 %_AR%
) ELSE (
  ECHO.
  ECHO - MSVC %1 %LP3D_DISTRO_ARCH% Redistributable %Redistributable% exist. Nothing to do.
)

CALL :SET_MSVC_REDIST %1 %_AR%
EXIT /b

:GET_MSVC_REDIST
SET _AR=%2
SET "WebCONTENT=%OutputPATH%\%RedistCONTENT%"
SET "WebNAME=%LP3D_LIBS_BASE%/%RedistCONTENT%"

ECHO.
ECHO - MSVC %1 %LP3D_DISTRO_ARCH% Redistributable: %WebCONTENT%...
ECHO.
cscript //Nologo %TEMP%\$\%vbs% %WebNAME% %WebCONTENT% && @ECHO off

IF "%LP3D_DISTRO_ARCH%" == "ARM64" (
  SET RedistCONTENT=vc_redist.%_AR%.exe
) ELSE (
  SET RedistCONTENT=vc_redist.%_AR%.exe
)
ECHO.
ECHO - Rename %WebCONTENT% %RedistCONTENT%
REN "%WebCONTENT%" "%RedistCONTENT%"

IF EXIST "%OutputPATH%\%RedistCONTENT%" (
  ECHO.
  ECHO - MSVC %1 %LP3D_DISTRO_ARCH% Redistributable %RedistCONTENT% is available
)
EXIT /b

:SET_MSVC_REDIST
SET _AR=%2
SET "PKG_TARGET_DIR=%WIN_PKG_DIR%\%CONFIGURATION%\%LP3D_PRODUCT_DIR%\%LP3D_PRODUCT%_%LP3D_DISTRO_ARCH%\vcredist"
IF NOT EXIST "%PKG_TARGET_DIR%\" (
  ECHO.
  ECHO - Create MSVC %1 %LP3D_DISTRO_ARCH% Redistributable package path %MSVC_REDIST_DIR%...
  MKDIR "%PKG_TARGET_DIR%\"
)
IF "%LP3D_DISTRO_ARCH%" == "ARM64" (
  SET RedistTARGET=vc_redist.%_AR%.exe
) ELSE (
  SET RedistTARGET=vc_redist.%_AR%.exe
)
IF NOT EXIST "%PKG_TARGET_DIR%\%RedistTARGET%" (
  ECHO.
  ECHO - Copy MSVC %1 %RedistTARGET% Redistributable to %PKG_TARGET_DIR% folder...
  IF EXIST "%MSVC_REDIST_DIR%\%RedistTARGET%" (
    COPY /V /Y "%MSVC_REDIST_DIR%\%RedistTARGET%" "%PKG_TARGET_DIR%\%RedistTARGET%" /B | FINDSTR /i /v /r /c:"copied\>"
  ) ELSE (
    ECHO.
    ECHO - ERROR - MSVC %1 %LP3D_DISTRO_ARCH% Redistributable %RedistTARGET% does not exist in %MSVC_REDIST_DIR%\.
  )
) ELSE (
  ECHO.
  ECHO - MSVC %1 %RedistTARGET% Redistributable %RedistCONTENT% exist in %PKG_TARGET_DIR%. Nothing to do.
)
EXIT /b

:DOWNLOADLDRAWLIBS
ECHO.
ECHO - Download LDraw archive libraries...

IF /I "%GITHUB%" EQU "True" (
  SET LDRAW_LIBS=%LP3D_3RD_PARTY_PATH%
)
IF /I "%APPVEYOR%" EQU "True" (
  SET LDRAW_LIBS=%CI_APPVEYOR_LDRAW_BASE%\LDrawLibs
)
IF /I "%GITHUB%" NEQ "True" (
  IF /I "%APPVEYOR%" NEQ "True" (
    SET LDRAW_LIBS=%USERPROFILE%
  )
)

IF NOT EXIST "%LDRAW_LIBS%\" (
  ECHO.
  ECHO - Create LDraw archive libs store %LDRAW_LIBS%
  MKDIR "%LDRAW_LIBS%\"
)

SET OutputPATH=%LDRAW_LIBS%
SET OfficialCONTENT=complete.zip
SET LPub3DCONTENT=lpub3dldrawunf.zip
SET TenteCONTENT=tenteparts.zip
SET VexiqCONTENT=vexiqparts.zip

(CALL :CREATEWEBCONTENTDOWNLOADVBS )

ECHO.
ECHO - LDraw archive library download path: %OutputPATH%

CALL :GET_OFFICIAL_LIBRARY
CALL :GET_TENTE_LIBRARY
CALL :GET_VEXIQ_LIBRARY
CALL :GET_UNOFFICIAL_LIBRARY

IF %LP3D_AMD_UNIVERSAL_BUILD% EQU 1 (
  CALL :SET_LDRAW_LIBRARIES x86_64
  CALL :SET_LDRAW_LIBRARIES x86
) ELSE (
  CALL :SET_LDRAW_LIBRARIES %LP3D_DISTRO_ARCH%
)
EXIT /b

:GET_OFFICIAL_LIBRARY
IF EXIST "%OutputPATH%\%OfficialCONTENT%" (
  ECHO.
  ECHO - LDraw archive library %OfficialCONTENT% exist. Nothing to do.
  EXIT /b
)

SET WebCONTENT="%OutputPATH%\%OfficialCONTENT%"
SET WebNAME=%LP3D_LIBS_BASE%/%OfficialCONTENT%

ECHO.
ECHO - Download archive file: %WebCONTENT%...
ECHO.
cscript //Nologo %TEMP%\$\%vbs% %WebNAME% %WebCONTENT% && @ECHO off
IF EXIST "%OutputPATH%\%OfficialCONTENT%" (
  ECHO.
  ECHO - LDraw archive library %OfficialCONTENT% is available
)
EXIT /b

:GET_TENTE_LIBRARY
IF EXIST "%OutputPATH%\%TenteCONTENT%" (
  ECHO.
  ECHO - LDraw archive library %TenteCONTENT% exist. Nothing to do.
  EXIT /b
)

SET WebCONTENT="%OutputPATH%\%TenteCONTENT%"
SET WebNAME=%LP3D_LIBS_BASE%/%TenteCONTENT%

ECHO.
ECHO - Download archive file: %WebCONTENT%...
ECHO.
cscript //Nologo %TEMP%\$\%vbs% %WebNAME% %WebCONTENT% && @ECHO off
IF EXIST "%OutputPATH%\%TenteCONTENT%" (
  ECHO.
  ECHO - LDraw archive library %TenteCONTENT% is available
)
EXIT /b

:GET_VEXIQ_LIBRARY
IF EXIST "%OutputPATH%\%VexiqCONTENT%" (
  ECHO.
  ECHO - LDraw archive library %VexiqCONTENT% exist. Nothing to do.
  EXIT /b
)

SET WebCONTENT="%OutputPATH%\%VexiqCONTENT%"
SET WebNAME=%LP3D_LIBS_BASE%/%VexiqCONTENT%

ECHO.
ECHO - Download archive file: %WebCONTENT%...
ECHO.
cscript //Nologo %TEMP%\$\%vbs% %WebNAME% %WebCONTENT% && @ECHO off
IF EXIST "%OutputPATH%\%VexiqCONTENT%" (
  ECHO.
  ECHO - LDraw archive library %VexiqCONTENT% is available
)
EXIT /b

:GET_UNOFFICIAL_LIBRARY
IF EXIST "%OutputPATH%\%LPub3DCONTENT%" (
  ECHO.
  ECHO - LDraw archive library %LPub3DCONTENT% exist. Nothing to do.
  EXIT /b
)

SET WebCONTENT="%OutputPATH%\%LPub3DCONTENT%"
SET WebNAME=%LP3D_LIBS_BASE%/%LPub3DCONTENT%

ECHO.
ECHO - Download archive file: %WebCONTENT%...
ECHO.
cscript //Nologo %TEMP%\$\%vbs% %WebNAME% %WebCONTENT% && @ECHO off
ECHO.
ECHO - Rename %WebCONTENT% to %LPub3DCONTENT%
REN "%WebCONTENT%" %LPub3DCONTENT%
IF EXIST "%OutputPATH%\%LPub3DCONTENT%" (
  ECHO.
  ECHO - LDraw archive library %LPub3DCONTENT% is available
)
EXIT /b

:SET_LDRAW_LIBRARIES
SET "PKG_TARGET_DIR=%WIN_PKG_DIR%\%CONFIGURATION%\%LP3D_PRODUCT_DIR%\%LP3D_PRODUCT%_%1"
ECHO.
ECHO - Copy LDraw archive libraries to %PKG_TARGET_DIR%\extras folder...
IF NOT EXIST "%PKG_TARGET_DIR%\extras\%OfficialCONTENT%" (
  IF EXIST "%LDRAW_LIBS%\%OfficialCONTENT%" (
    COPY /V /Y "%LDRAW_LIBS%\%OfficialCONTENT%" "%PKG_TARGET_DIR%\extras\" /B | FINDSTR /i /v /r /c:"copied\>"
  ) ELSE (
    ECHO.
    ECHO - ERROR - LDraw archive lib %OfficialCONTENT% does not exist in %LDRAW_LIBS%\.
  )
) ELSE (
  ECHO.
  ECHO - Archive library %OfficialCONTENT% exist in %LP3D_PRODUCT%_%1\extras folder. Nothing to do.
)
IF NOT EXIST "%PKG_TARGET_DIR%\extras\%TenteCONTENT%" (
  IF EXIST "%LDRAW_LIBS%\%TenteCONTENT%" (
    COPY /V /Y "%LDRAW_LIBS%\%TenteCONTENT%" "%PKG_TARGET_DIR%\extras\" /B | FINDSTR /i /v /r /c:"copied\>"
  ) ELSE (
    ECHO.
    ECHO - ERROR - LDraw archive lib %TenteCONTENT% does not exist in %LDRAW_LIBS%\.
  )
) ELSE (
  ECHO.
  ECHO - Archive library %TenteCONTENT% exist in %LP3D_PRODUCT%_%1\extras folder. Nothing to do.
)
IF NOT EXIST "%PKG_TARGET_DIR%\extras\%VexiqCONTENT%" (
  IF EXIST "%LDRAW_LIBS%\%VexiqCONTENT%" (
    COPY /V /Y "%LDRAW_LIBS%\%VexiqCONTENT%" "%PKG_TARGET_DIR%\extras\" /B | FINDSTR /i /v /r /c:"copied\>"
  ) ELSE (
    ECHO.
    ECHO - ERROR - LDraw archive lib %VexiqCONTENT% does not exist in %LDRAW_LIBS%\.
  )
) ELSE (
  ECHO.
  ECHO - Archive library %VexiqCONTENT% exist in %LP3D_PRODUCT%_%1\extras folder. Nothing to do.
)
IF NOT EXIST "%PKG_TARGET_DIR%\extras\%LPub3DCONTENT%" (
  IF EXIST "%LDRAW_LIBS%\%LPub3DCONTENT%" (
    COPY /V /Y "%LDRAW_LIBS%\%LPub3DCONTENT%" "%PKG_TARGET_DIR%\extras\" /B | FINDSTR /i /v /r /c:"copied\>"
  ) ELSE (
    ECHO.
    ECHO - ERROR - LDraw archive lib %LPub3DCONTENT% does not exist in %LDRAW_LIBS%\.
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
>>%t% WScript.Echo "- Target: '" ^& target ^& "'...", vbLF
>>%t% WScript.Echo "- Source: '" ^& url ^& "'...", vbLF
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
ECHO - VBS file "%vbs%" at "%TEMP%\$\%vbs%" is done compiling
EXIT /b

:WD_ABS_PATH
IF [%1] EQU [] (EXIT /B) ELSE SET ABS_WD=%~f1
IF %ABS_WD:~-1%==\ SET ABS_WD=%ABS_WD:~0,-1%
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
>>%genFile% $env:LP3D_BUILD_PACKAGE = "${env:LP3D_PRODUCT}-Any-${env:LP3D_APP_VERSION_LONG}"
>>%genFile% $env:LP3D_BUILD_TARGET = "${env:LP3D_PACKAGE_PATH}\${env:LP3D_BUILD_PACKAGE}"
>>%genFile% $env:LP3D_DOWNLOAD_ASSETS = "${env:LP3D_BUILD_TARGET}\${env:LP3D_PRODUCT}_Download"
>>%genFile% $env:LP3D_UPDATE_ASSETS = "${env:LP3D_BUILD_TARGET}\${env:LP3D_PRODUCT}_Update"
>>%genFile% $env:LP3D_RUNLOG_ASSETS = "${env:LP3D_BUILD_TARGET}\${env:LP3D_PRODUCT}_Logs"
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
>>%genFile% #  Copyright ^(c^) 2020 - 2025 by Trevor SANDY
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
>>%genFile% export LP3D_BUILD_PACKAGE="%LP3D_PRODUCT%-Any-%LP3D_APP_VERSION_LONG%"
>>%genFile% export LP3D_BUILD_TARGET="%LP3D_PACKAGE_PATH_BASH%/${LP3D_BUILD_PACKAGE}"
>>%genFile% export LP3D_DOWNLOAD_ASSETS="${LP3D_BUILD_TARGET}/%PKG_DOWNLOAD_DIR%"
>>%genFile% export LP3D_UPDATE_ASSETS="${LP3D_BUILD_TARGET}/%PKG_UPDATE_DIR%"
>>%genFile% echo ^&^& echo "- Update-config-files environment variables set in Bash"
>>%genFile% echo ^&^& echo "- LP3D BUILD Environment Variables:" ^&^& env ^| sort ^| grep 'LP3D_BUILD_*'
IF EXIST "%set_bash_vars%" (
  ECHO   FILE set_bash_vars.sh..........[written to %set_bash_vars%]
) ELSE (
  ECHO   FILE set_bash_vars.sh..........[ERROR - file %set_bash_vars% not found]
)
EXIT /b

:MOVE_ASSETS_TO_DISTRO_PATH
CD %GITHUB_WORKSPACE%
ECHO.
ECHO - Move assets and logs to output folder...
SET LP3D_BUILD_PACKAGE=%LP3D_PRODUCT%-Any-%LP3D_APP_VERSION_LONG%
SET LP3D_PACKAGE_PATH=%CD%\builds\windows\%CONFIGURATION%
SET LP3D_BUILD_TARGET=%LP3D_PACKAGE_PATH%\%LP3D_BUILD_PACKAGE%
SET LP3D_DOWNLOAD_ASSETS=%LP3D_BUILD_TARGET%\%PKG_DOWNLOAD_DIR%
SET LP3D_UPDATE_ASSETS=%LP3D_BUILD_TARGET%\%PKG_UPDATE_DIR%
SET LP3D_RUNLOG_ASSETS=%LP3D_BUILD_TARGET%\%PKG_LOGS_DIR%
SET FILTER_LIST=LP3D_BUILD_* LP3D_PACKAGE_* LP3D_DOWNLOAD_* LP3D_UPDATE_* LP3D_RUNLOG_*
ECHO.
FOR /f "delims== tokens=1,2" %%a IN ('SET ^| FINDSTR / "%FILTER_LIST%"') DO (ECHO - %%a = %%b)
IF EXIST "%LP3D_DOWNLOAD_ASSETS%" (
  ECHO.
  ECHO - Move download assets to LP3D_DOWNLOADS_PATH %LP3D_DOWNLOADS_PATH%...
  FOR /D /R "%LP3D_DOWNLOAD_ASSETS%" %%C IN (*) DO (
    IF EXIST "%LP3D_DOWNLOADS_PATH%\%%~nC" (
      DIR "%%C" | FIND "0 File(s)" > NUL & IF ERRORLEVEL 1 (MOVE /Y "%%C\*.*" "%LP3D_DOWNLOADS_PATH%\%%~nC" | FINDSTR /i /v /r /c:"moved\>")
    ) ELSE (
      MOVE /Y "%%C" "%LP3D_DOWNLOADS_PATH%"
    )
  )
  MOVE /Y "%LP3D_DOWNLOAD_ASSETS%\*.*" "%LP3D_DOWNLOADS_PATH%" | FINDSTR /i /v /r /c:"moved\>"
  RD /S /Q "%LP3D_DOWNLOAD_ASSETS%"
) ELSE (
  ECHO.
  ECHO - ERROR - Download assets path %LP3D_DOWNLOAD_ASSETS% not found.
  GOTO :ERROR_END
)

IF "%LP3D_BUILD_ARCH%" EQU "ARM64" (GOTO :MOVE_RUNLOG_ASSETS)

IF EXIST "%LP3D_UPDATE_ASSETS%" (
  ECHO.
  ECHO - Move update assets to LP3D_UPDATES_PATH %LP3D_UPDATES_PATH%...
  FOR /D /R "%LP3D_UPDATE_ASSETS%" %%C IN (*) DO (
    IF EXIST "%LP3D_UPDATES_PATH%\%%~nC" (
      DIR "%%C" | FIND "0 File(s)" > NUL & IF ERRORLEVEL 1 (MOVE /Y "%%C\*.*" "%LP3D_UPDATES_PATH%\%%~nC" | FINDSTR /i /v /r /c:"moved\>")
    ) ELSE (
      MOVE /Y "%%C" "%LP3D_UPDATES_PATH%" | FINDSTR /i /v /r /c:"moved\>"
    )
  )
  MOVE /Y "%LP3D_UPDATE_ASSETS%\*.*" "%LP3D_UPDATES_PATH%" | FINDSTR /i /v /r /c:"moved\>"
  RD /S /Q "%LP3D_UPDATE_ASSETS%"
) ELSE (
  ECHO.
  ECHO - ERROR - Update assets path %LP3D_UPDATE_ASSETS% not found.
  GOTO :ERROR_END
)

:MOVE_RUNLOG_ASSETS
IF EXIST "%LP3D_RUNLOG_ASSETS%" (
  ECHO.
  ECHO - Move runlog assets to LP3D_DOWNLOADS_PATH %LP3D_DOWNLOADS_PATH%...
  FOR /D /R "%LP3D_RUNLOG_ASSETS%" %%C IN (*) DO (
    IF EXIST "%LP3D_DOWNLOADS_PATH%\%%~nC" (
      DIR "%%C" | FIND "0 File(s)" > NUL & IF ERRORLEVEL 1 (MOVE /Y "%%C\*.*" "%LP3D_DOWNLOADS_PATH%\%%~nC" | FINDSTR /i /v /r /c:"moved\>")
    ) ELSE (
      MOVE /Y "%%C" "%LP3D_DOWNLOADS_PATH%" | FINDSTR /i /v /r /c:"moved\>"
    )
  )
  MOVE /Y "%LP3D_RUNLOG_ASSETS%\*.*" "%LP3D_LOG_PATH%" | FINDSTR /i /v /r /c:"moved\>"
  RD /S /Q "%LP3D_RUNLOG_ASSETS%"
) ELSE (
  REM ECHO.
  REM ECHO - NOTICE - Run-log assets path %LP3D_RUNLOG_ASSETS% not found.
)
EXIT /b

:POSTPROCESS
ECHO.
ECHO - Post process...
ECHO.
ECHO - If everything went well Press any key to EXIT!
%SystemRoot%\explorer.exe "%WIN_PKG_DIR%\%CONFIGURATION%\%LP3D_PRODUCT_DIR%"
PAUSE >NUL
EXIT /b

:ELAPSED_BUILD_TIME
IF [%1] EQU [] (SET start=%build_start%) ELSE (
  IF "%1"=="Start" (
    SET build_start=%time%
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
ECHO   Elapsed build time %hours%:%mins%:%secs%.%ms%
ENDLOCAL
EXIT /b

:ERROR_END
ECHO.
ECHO - %~nx0 FAILED
ECHO - %~nx0 will terminate!
CALL :ELAPSED_BUILD_TIME
EXIT /b 3

:END
ECHO.
ECHO - %~nx0 finished
CALL :ELAPSED_BUILD_TIME
EXIT /b