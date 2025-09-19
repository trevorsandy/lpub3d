@ECHO OFF &SETLOCAL

Title LPub3D Windows auto build script

rem This script uses Qt to configure and build LPub3D for Windows.
rem The primary purpose is to automatically build both the 32bit and 64bit
rem LPub3D distributions and package the build contents (exe, doc and
rem resources ) for distribution release.
rem --
rem  Trevor SANDY <trevor.sandy@gmail.com>
rem  Last Update: September 12, 2025
rem  Copyright (c) 2019 - 2025 by Trevor SANDY
rem --
rem This script is distributed in the hope that it will be useful,
rem but WITHOUT ANY WARRANTY; without even the implied warranty of
rem MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

CALL :ELAPSED_BUILD_TIME Start

%WINDIR%\system32\tzutil.exe /s "Central Europe Standard Time"

ECHO.
ECHO ======================================================
ECHO   -Start %~nx0 with commandline args: [%*]...
ECHO ------------------------------------------------------

FOR %%* IN (.) DO SET SCRIPT_RUN_DIR=%%~nx*
IF "%SCRIPT_RUN_DIR%" EQU "windows" (
  CALL :WD_ABS_PATH ..\..\
) ELSE (
  SET ABS_WD=%CD%
)

IF "%LP3D_VSVERSION%" == "" SET LP3D_VSVERSION=2022
IF "%LP3D_QT32VERSION%" == "" SET LP3D_QT32VERSION=5.15.2
IF "%LP3D_QT64VERSION%" == "" SET LP3D_QT64VERSION=6.9.2
IF "%LP3D_QT32VCVERSION%" == "" SET LP3D_QT32VCVERSION=2019
IF "%LP3D_QT64VCVERSION%" == "" SET LP3D_QT64VCVERSION=2022

IF "%GITHUB%" EQU "True" (
  SET "BUILD_WORKER=True"
  SET "BUILD_WORKER_ID=GITHUB"
  SET "BUILD_WORKER_CONFIG=%GITHUB_CONFIG%"
  SET "BUILD_WORKER_JOB=%GITHUB_JOB%"
  SET "BUILD_WORKER_REF=%GITHUB_REF%"
  SET "BUILD_WORKER_OS=%RUNNER_OS%"
  SET "BUILD_WORKER_REPO=%GITHUB_REPOSITORY%"
  SET "BUILD_WORKER_IMAGE=Visual Studio %LP3D_VSVERSION%"
  SET "BUILD_WORKER_HOST=GITHUB CONTINUOUS INTEGRATION SERVICE"
  SET "BUILD_WORKSPACE=%GITHUB_WORKSPACE%"
)

IF "%LP3D_CONDA_BUILD%" EQU "True" (
  SET "BUILD_WORKER=True"
  SET "BUILD_WORKER_ID=CONDA"
  SET "BUILD_WORKER_CONFIG=%LP3D_CONDA_CONFIG%"
  SET "BUILD_WORKER_JOB=%LP3D_CONDA_JOB%"
  SET "BUILD_WORKER_OS=%LP3D_CONDA_RUNNER_OS%"
  SET "BUILD_WORKER_REPO=%LP3D_CONDA_REPOSITORY%"
  SET "BUILD_WORKER_IMAGE=%CMAKE_GENERATOR%"
  SET "BUILD_WORKER_HOST=CONDA BUILD INTEGRATION SERVICE"
  SET "BUILD_WORKSPACE=%LP3D_CONDA_WORKSPACE%"
)

IF "%LP3D_INSTALL_PKG_ONLY%" == "1" (
  SET LP3D_COMMIT_MSG=Create installation packages only
)

IF "%LP3D_CONDA_BUILD%" NEQ "True" (
  IF EXIST "C:\Program Files\Microsoft Visual Studio\%LP3D_VSVERSION%\Community\VC\Auxiliary\Build" (
    SET LP3D_VCVARSALL_DIR=C:\Program Files\Microsoft Visual Studio\%LP3D_VSVERSION%\Community\VC\Auxiliary\Build
  )
  IF EXIST "C:\Program Files\Microsoft Visual Studio\%LP3D_VSVERSION%\Enterprise\VC\Auxiliary\Build" (
    SET LP3D_VCVARSALL_DIR=C:\Program Files\Microsoft Visual Studio\%LP3D_VSVERSION%\Enterprise\VC\Auxiliary\Build
  )
  IF EXIST "C:\Program Files (x86)\Microsoft Visual Studio\%LP3D_VSVERSION%\Professional\VC\Auxiliary\Build" (
    SET LP3D_VCVARSALL_DIR=C:\Program Files ^(x86^)\Microsoft Visual Studio\%LP3D_VSVERSION%\Professional\VC\Auxiliary\Build
  )
  IF EXIST "C:\Program Files (x86)\Microsoft Visual Studio\%LP3D_VSVERSION%\Community\VC\Auxiliary\Build" (
    SET LP3D_VCVARSALL_DIR=C:\Program Files ^(x86^)\Microsoft Visual Studio\%LP3D_VSVERSION%\Community\VC\Auxiliary\Build
  )
  IF EXIST "C:\Program Files (x86)\Microsoft Visual Studio\%LP3D_VSVERSION%\BuildTools\VC\Auxiliary\Build" (
    SET LP3D_VCVARSALL_DIR=C:\Program Files ^(x86^)\Microsoft Visual Studio\%LP3D_VSVERSION%\BuildTools\VC\Auxiliary\Build
  )
  IF EXIST "C:\Program Files (x86)\Microsoft Visual Studio\%LP3D_VSVERSION%\Enterprise\VC\Auxiliary\Build" (
    SET LP3D_VCVARSALL_DIR=C:\Program Files ^(x86^)\Microsoft Visual Studio\%LP3D_VSVERSION%\Enterprise\VC\Auxiliary\Build
  )
)
IF NOT EXIST "%LP3D_VCVARSALL_DIR%" (
  ECHO.
  ECHO  -ERROR - Microsoft Visual Studio C++ environment [%LP3D_VCVARSALL_DIR%] not defined.
  GOTO :ERROR_END
)

IF NOT EXIST "%ABS_WD%\mainApp" (
  ECHO.
  ECHO -ERROR - mainApp folder not found. Working directory %ABS_WD% is not valid.
  GOTO :ERROR_END
)

rem https://learn.microsoft.com/en-us/cpp/overview/compiler-versions
rem https://en.wikipedia.org/wiki/Microsoft_Visual_C++
rem Visual C++ 2012 -vcvars_ver=11.0 Toolset v110 VSVersion 11.0    _MSC_VER 1700
rem Visual C++ 2013 -vcvars_ver=12.0 Toolset v120 VSVersion 12.0    _MSC_VER 1800
rem Visual C++ 2015 -vcvars_ver=14.0 Toolset v140 VSVersion 14.0    _MSC_VER 1900
rem Visual C++ 2017 -vcvars_ver=14.1 Toolset v141 VSVersion 15.9    _MSC_VER 1916
rem Visual C++ 2019 -vcvars_ver=14.2 Toolset v142 VSVersion 16.11.3 _MSC_VER 1929
rem Visual C++ 2022 -vcvars_ver=14.4 Toolset v143 VSVersion 17.14.0 _MSC_VER 1944
IF "%LP3D_MSC32_VER%" == "" (
    SET LP3D_MSC32_VER=1942
) ELSE (
    IF %LP3D_MSC32_VER% LSS 1942 SET LP3D_MSC32_VER=1942
)
SETLOCAL ENABLEDELAYEDEXPANSION
IF "%LP3D_VC32SDKVER%" == ""  (
    SET LP3D_VC32SDKVER=10.0
) ELSE (
    SET LP3D_VCPARAM=%LP3D_VC32SDKVER:.=%
    IF !LP3D_VCPARAM! LSS 100 SET LP3D_VC32SDKVER=10.0
)
IF "%LP3D_VC32TOOLSET%" == "" (
    SET LP3D_VC32TOOLSET=v142
) ELSE (
    SET LP3D_VCPARAM=%LP3D_VC32TOOLSET:~1%
    IF !LP3D_VCPARAM! LSS 142 SET LP3D_VC32TOOLSET=v142
)
IF "%LP3D_VC32VARSALL_VER%" == "" (
    SET LP3D_VC32VARSALL_VER=-vcvars_ver=14.2
) ELSE (
    FOR /f "tokens=2 delims==" %%P IN ("%LP3D_VC32VARSALL_VER%") DO SET LP3D_VCPARAM=%%P
    SET LP3D_VCPARAM=!LP3D_VCPARAM:.=!
    IF !LP3D_VCPARAM! LSS 142 SET LP3D_VC32VARSALL_VER=-vcvars_ver=14.2
)
SETLOCAL DISABLEDELAYEDEXPANSION

IF "%LP3D_MSC64_VER%" == "" SET LP3D_MSC64_VER=1944
IF "%LP3D_VC64SDKVER%" == "" SET LP3D_VC64SDKVER=10.0
IF "%LP3D_VC64TOOLSET%" == "" SET LP3D_VC64TOOLSET=v143
IF "%LP3D_VC64VARSALL_VER%" == "" SET LP3D_VC64VARSALL_VER=-vcvars_ver=14.4

IF "%LP3D_MSCARM64_VER%" == "" SET LP3D_MSCARM64_VER=1944
IF "%LP3D_VCARM64SDKVER%" == "" SET LP3D_VCARM64SDKVER=10.0
IF "%LP3D_VCARM64TOOLSET%" == "" SET LP3D_VCARM64TOOLSET=v143
IF "%LP3D_VCARM64VARSALL_VER%" == "" SET LP3D_VCARM64VARSALL_VER=-vcvars_ver=14.4

SET OfficialCONTENT=complete.zip
SET LPub3DCONTENT=lpub3dldrawunf.zip
SET TenteCONTENT=tenteparts.zip
SET VexiqCONTENT=vexiqparts.zip

SET BUILD_THIRD=unknown
SET PKG_INSTALL=unknown
SET INSTALL_32BIT=unknown
SET INSTALL_64BIT=unknown
SET PLATFORM_ARCH=unknown
SET LDCONFIG_FILE=unknown
SET CHECK=unknown
SET LP3D_GITHUB_URL=https://github.com/trevorsandy
SET LP3D_LIBS_BASE=%LP3D_GITHUB_URL%/lpub3d_libs/releases/download/v1.0.1
SET LP3D_AMD64_ARM64_CROSS=0

FOR /F "tokens=3*" %%i IN ('findstr /c:"#define VER_PRODUCTNAME_STR" %ABS_WD%\mainApp\version.h') DO SET LP3D_PRODUCT=%%i
SET PACKAGE=%LP3D_PRODUCT:"=%
IF [%PACKAGE%] == [] (
  ECHO.
  ECHO  -WARNING - PACKAGE value not retrieved from %ABS_WD%\mainApp\version.h.
  SET PACKAGE=LPub3D
)

rem Verify 1st input flag options
IF NOT [%1]==[] (
  IF /I NOT "%1"=="x86" (
    IF /I NOT "%1"=="x86_64" (
      IF /I NOT "%1"=="arm64" (
        IF /I NOT "%1"=="-all_amd" (
          IF /I NOT "%1"=="-help" GOTO :PLATFORM_ERROR
        )
      )
    )
  )
)

rem Parse platform input flags
IF [%1]==[] (
  SET PLATFORM_ARCH=-all_amd
  GOTO :SET_CONFIGURATION
)

IF /I "%1"=="x86" (
  SET PLATFORM_ARCH=x86
  GOTO :SET_CONFIGURATION
)

IF /I "%1"=="x86_64" (
  SET PLATFORM_ARCH=x86_64
  GOTO :SET_CONFIGURATION
)

IF /I "%1"=="arm64" (
  SET PLATFORM_ARCH=ARM64
  GOTO :SET_CONFIGURATION
)

IF /I "%1"=="-all_amd" (
  SET PLATFORM_ARCH=-all_amd
  GOTO :SET_CONFIGURATION
)

IF /I "%1"=="-help" (
  GOTO :USAGE
)

rem If we get here display invalid command message.
GOTO :COMMAND_ERROR

:SET_CONFIGURATION
rem Verify 2nd input flag options
IF NOT [%2]==[] (
  IF /I NOT "%2"=="-ins" (
    IF /I NOT "%2"=="-chk" (
      IF /I NOT "%2"=="-3rd" (
        IF /I NOT "%2"=="-ren" GOTO :CONFIGURATION_ERROR
      )
    )
  )
)

rem Verify 3rd input flag options
IF NOT [%3]==[] (
  IF /I NOT "%3"=="-ins" (
    IF /I NOT "%3"=="-chk" (
      IF /I NOT "%3"=="-asl" (
        IF /I NOT "%3"=="-ldraw" (
          IF /I NOT "%3"=="-official" (
            IF /I NOT "%3"=="-unofficial" (
              IF /I NOT "%3"=="-tente" (
                IF /I NOT "%3"=="-vexiq" GOTO :CONFIGURATION_ERROR
              )
            )
          )
        )
      )
    )
  )
)

rem Verify 4th input flag options
IF NOT [%4]==[] (
  IF /I NOT "%4"=="-chk" (
    IF /I NOT "%4"=="-asl" (
      IF /I NOT "%3"=="-ldraw" (
        IF /I NOT "%4"=="-official" (
          IF /I NOT "%4"=="-unofficial" (
            IF /I NOT "%4"=="-tente" (
              IF /I NOT "%4"=="-vexiq" GOTO :CONFIGURATION_ERROR
            )
          )
        )
      )
    )
  )
)

rem Setup library options and set ARM64 cross compilation
IF /I "%PLATFORM_ARCH%" == "ARM64" (
  IF /I "%PROCESSOR_ARCHITECTURE%" == "AMD64" (
    SET LP3D_AMD64_ARM64_CROSS=1
  )
  SET LP3D_QTARCH=arm64
) ELSE (
  SET LP3D_QTARCH=64
)
IF "%BUILD_WORKER%" EQU "True" (
  IF "%LP3D_DIST_DIR_PATH%" == "" (
    ECHO.
    ECHO  -ERROR - Distribution directory path not defined.
    GOTO :ERROR_END
  )
  IF "%LP3D_LOCAL_CI_BUILD%" == "1" (
    SET CI=True
    SET BUILD_WORKER_JOB=Local %BUILD_WORKER_ID% CI Build
  ) ELSE (
    :: Using 'Day MM/DD/YYYY' date format, default is 'DD/MM/YYYY'
    IF "%GITHUB%" EQU "True" (
      SET CONFIG_CI=github_ci_win
    )
    IF "%LP3D_CONDA_BUILD%" EQU "True" (
      :: Using default 'DD/MM/YYYY' date format for 'BUILD_OPT=local' build
      IF "%BUILD_OPT%" EQU "default" (
        SET CONFIG_CI=azure_ci_win
      )
    )
  )
  SET ABS_WD=%BUILD_WORKSPACE%
  SET DIST_DIR=%LP3D_DIST_DIR_PATH%
  SET CONFIGURATION=%BUILD_WORKER_CONFIG%
  SET LDRAW_INSTALL_ROOT=%LP3D_3RD_PARTY_PATH%
  SET LDRAW_LIBS=%LP3D_3RD_PARTY_PATH%
  SET LDRAW_DIR=%LP3D_LDRAW_DIR_PATH%
  SET LP3D_UPDATE_LDRAW_LIBS=%UPDATE_LDRAW_LIBS%
  IF "%LP3D_QT32_MSVC%" == "" (
    SET LP3D_QT32_MSVC=%LP3D_BUILD_BASE%\Qt\%LP3D_QT32VERSION%\msvc%LP3D_QT32VCVERSION%\bin
  )
  IF "%LP3D_QT64_MSVC%" == "" (
    SET LP3D_QT64_MSVC=%LP3D_BUILD_BASE%\Qt\%LP3D_QT64VERSION%\msvc%LP3D_QT64VCVERSION%_%LP3D_QTARCH%\bin
  )
)

IF "%APPVEYOR%" EQU "True" (
  IF "%LP3D_DIST_DIR_PATH%" == "" (
    ECHO.
    ECHO  -ERROR - Distribution directory path not defined.
    GOTO :ERROR_END
  )
  IF "%LP3D_LOCAL_CI_BUILD%" == "1" (
    SET CI=True
    SET APPVEYOR_BUILD_ID=Local CI Build
  ) ELSE (
    :: Using 'Day MM/DD/YYYY' date format, default is 'DD/MM/YYYY'
    SET CONFIG_CI=appveyor_ci_win
  )
  SET ABS_WD=%APPVEYOR_BUILD_FOLDER%
  SET DIST_DIR=%LP3D_DIST_DIR_PATH%
  SET CONFIGURATION=%configuration%
  SET APPVEYOR_BUILD_WORKER_IMAGE=Visual Studio %LP3D_VSVERSION%
  SET LDRAW_INSTALL_ROOT=%APPVEYOR_BUILD_FOLDER%
  SET LDRAW_LIBS=%APPVEYOR_BUILD_FOLDER%\LDrawLibs
  SET LDRAW_DIR=%APPVEYOR_BUILD_FOLDER%\LDraw
  SET LP3D_UPDATE_LDRAW_LIBS=%LP3D_UPDATE_LDRAW_LIBS_VAR%
  IF "%LP3D_QT32_MSVC%" == "" (
    SET LP3D_QT32_MSVC=C:\Qt\%LP3D_QT32VERSION%\msvc%LP3D_QT32VCVERSION%\bin
  )
  IF "%LP3D_QT64_MSVC%" == "" (
    SET LP3D_QT64_MSVC=C:\Qt\%LP3D_QT64VERSION%\msvc%LP3D_QT64VCVERSION%_%LP3D_QTARCH%\bin
  )
)

IF "%BUILD_WORKER%" NEQ "True" (
  IF "%APPVEYOR%" NEQ "True" (
    CALL :DIST_DIR_ABS_PATH ..\lpub3d_windows_3rdparty
    SET CONFIGURATION=release
    SET LDRAW_INSTALL_ROOT=%USERPROFILE%
    SET LDRAW_LIBS=%USERPROFILE%
    SET LDRAW_DIR=%USERPROFILE%\LDraw
    IF "%LP3D_QT32_MSVC%" == "" (
      SET LP3D_QT32_MSVC=C:\Qt\IDE\%LP3D_QT32VERSION%\msvc%LP3D_QT32VCVERSION%\bin
    )
    IF "%LP3D_QT64_MSVC%" == "" (
      SET LP3D_QT64_MSVC=C:\Qt\IDE\%LP3D_QT64VERSION%\msvc%LP3D_QT64VCVERSION%_%LP3D_QTARCH%\bin
    )
    SET LP3D_UPDATE_LDRAW_LIBS=unknown
  )
)

rem Confirm Qt platform libraries available
IF /I "%PLATFORM_ARCH%"=="x86" (
  IF NOT EXIST "%LP3D_QT32_MSVC%" GOTO :LIBRARY_ERROR
)
IF /I "%PLATFORM_ARCH%"=="x86_64" (
  IF NOT EXIST "%LP3D_QT64_MSVC%" GOTO :LIBRARY_ERROR
)
IF /I "%PLATFORM_ARCH%"=="ARM64" (
  IF NOT EXIST "%LP3D_QT64_MSVC%" GOTO :LIBRARY_ERROR
)
IF /I "%PLATFORM_ARCH%"=="-all_amd" (
  IF NOT EXIST "%LP3D_QT32_MSVC%" GOTO :LIBRARY_ERROR
  IF NOT EXIST "%LP3D_QT64_MSVC%" GOTO :LIBRARY_ERROR
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
  SET "LP3D_WIN_TAR_MSG=%LP3D_WIN_TAR%"
)

rem Set GIT path
IF "%LP3D_WIN_GIT%" == "" SET "LP3D_WIN_GIT=%ProgramFiles%\Git\cmd"
IF NOT EXIST "%LP3D_WIN_GIT%" (
  IF "%LP3D_WIN_GIT_MSG%" == "" SET LP3D_WIN_GIT_MSG=Not Found
) ELSE (
  SET "LP3D_WIN_GIT_MSG=%LP3D_WIN_GIT%"
)

rem Initialize package install to - no install
IF [%2]==[] (
  SET PKG_INSTALL=0
  GOTO :BUILD
)

rem Set package install
IF /I "%2"=="-ins" (
  SET PKG_INSTALL=1
  GOTO :BUILD
)

rem Set build check flag
IF /I "%2"=="-chk" (
  SET CHECK=1
  GOTO :BUILD
)

IF /I "%2"=="-3rd" (
  SET BUILD_THIRD=1
  GOTO :BUILD
)

IF /I "%2"=="-ren" (
  SET RENDERERS_ONLY=1
  GOTO :BUILD
)

rem If we get here display invalid command message.
GOTO :COMMAND_ERROR

:BUILD
rem Display build settings
ECHO.
IF "%LP3D_ARM64_QMAKE%" EQU "1" (
  ECHO   AUTO_BUILD_MODE................[QMake Makefile]
  GOTO :ARM64_BUILD
)
IF "%BUILD_WORKER%" EQU "True" (
  ECHO   BUILD_HOST.....................[%BUILD_WORKER_HOST%]
  ECHO   BUILD_WORKER_IMAGE.............[%BUILD_WORKER_IMAGE%]
  ECHO   BUILD_WORKER_JOB...............[%BUILD_WORKER_JOB%]
  IF "%LP3D_CONDA_BUILD%" NEQ "True" (
  ECHO   BUILD_WORKER_REF...............[%BUILD_WORKER_REF%]
  )
  ECHO   BUILD_WORKER_OS................[%BUILD_WORKER_OS%]
  ECHO   PROJECT REPOSITORY.............[%BUILD_WORKER_REPO%]
)
IF "%APPVEYOR%" EQU "True" (
  ECHO   BUILD_HOST.....................[APPVEYOR CONTINUOUS INTEGRATION SERVICE]
  ECHO   BUILD_WORKER_IMAGE.............[%APPVEYOR_BUILD_WORKER_IMAGE%]
  ECHO   BUILD_ID.......................[%APPVEYOR_BUILD_ID%]
  ECHO   BUILD_BRANCH...................[%APPVEYOR_REPO_BRANCH%]
  ECHO   PROJECT_NAME...................[%APPVEYOR_PROJECT_NAME%]
  ECHO   REPOSITORY_NAME................[%APPVEYOR_REPO_NAME%]
  ECHO   REPO_PROVIDER..................[%APPVEYOR_REPO_PROVIDER%]
)
ECHO   PACKAGE........................[%PACKAGE%]
ECHO   CONFIGURATION..................[%CONFIGURATION%]
IF "%LP3D_CONDA_BUILD%" NEQ "True" (
ECHO   COMMIT MESSAGE.................[%LP3D_COMMIT_MSG%]
)
ECHO   WORKING_DIRECTORY_LPUB3D.......[%ABS_WD%]
ECHO   DISTRIBUTION_DIRECTORY.........[%DIST_DIR%]
IF [%LP3D_PUBLISH_RENDERERS%] NEQ [] (
  ECHO   PUBLISH_RENDERERS..............[%LP3D_PUBLISH_RENDERERS%]
)
ECHO   LDRAW_DIRECTORY................[%LDRAW_DIR%]
ECHO   LDRAW_INSTALL_ROOT.............[%LDRAW_INSTALL_ROOT%]
ECHO   LDRAW_LIBS_ROOT................[%LDRAW_LIBS%]
ECHO   BUILD_OPT......................[%BUILD_OPT%]
ECHO   LP3D_WIN_GIT_DIR...............[%LP3D_WIN_GIT_MSG%]
ECHO   LP3D_WIN_TAR...................[%LP3D_WIN_TAR_MSG%]
IF /I "%PLATFORM_ARCH%" == "ARM64" (
  ECHO   PROCESSOR_ARCH.................[%PROCESSOR_ARCHITECTURE%]
  IF %LP3D_AMD64_ARM64_CROSS% EQU 1 (
    ECHO   COMPILATION....................[Cross compile ARM64 build on AMD64 host]
  )
  IF "%LP3D_ARM64_BUILD%" == "1" (
    ECHO   AUTO_BUILD_MODE................[NMake Build]
  )
)

IF /I "%RENDERERS_ONLY%"=="1" (
  ECHO.
  ECHO -Build 3rdparty renderers only - %PACKAGE% not built
  ECHO.
)

rem set application version variables
:ARM64_BUILD
SET UPDATE_CONFIG_ARGS=%ABS_WD%\mainApp
IF "%LP3D_CONDA_BUILD%" EQU "True" (
  SET UPDATE_CONFIG_ARGS=%UPDATE_CONFIG_ARGS% ParseVersionInfoFile
) ELSE (
  IF "%LP3D_ARM64_QMAKE%" EQU "1" (
    SET UPDATE_CONFIG_ARGS=%UPDATE_CONFIG_ARGS% QuietConfiguration
  )
)
ECHO.
CALL builds\utilities\update-config-files.bat %UPDATE_CONFIG_ARGS%
IF ERRORLEVEL 1 (GOTO :ERROR_END)

rem Perform 3rd party package content install
IF /I "%3"=="-ins" (
  SET PKG_INSTALL=1
)

rem Perform build check
IF /I "%3"=="-chk" (
  SET CHECK=1
)

rem update all supported libraries when -asl defined
IF NOT [%3]==[] (
  IF /I NOT "%3"=="-ins" (
    IF /I NOT "%3"=="-chk" (
      IF /I NOT "%3"=="-asl" (
        SET LP3D_UPDATE_LDRAW_LIBS=%3
      ) ELSE (
        SET LP3D_UPDATE_LDRAW_LIBS=-true
      )
    )
  )
)

IF /I "%4"=="-chk" (
  SET CHECK=1
)

rem update all supported libraries when -asl defined
IF NOT [%4]==[] (
  IF /I NOT "%4"=="-chk" (
    IF /I NOT "%4"=="-asl" (
      SET LP3D_UPDATE_LDRAW_LIBS=%4
    ) ELSE (
      SET LP3D_UPDATE_LDRAW_LIBS=-true
    )
  )
)

rem Create distribution folder
IF NOT EXIST "%DIST_DIR%\" (
  MKDIR "%DIST_DIR%\"
)

rem Stage package install prior to build check
IF /I %CHECK%==1 (
  SET PKG_INSTALL=1
)

rem set debug suffix
IF NOT [%CONFIGURATION%]==[] (
  IF /I "%CONFIGURATION%"=="release" (
    SET d=
  )
  IF /I "%CONFIGURATION%"=="debug" (
    SET d=d
  )
)

SET platform_build_start=%time%

rem stop here if only running settings for CreateExePkg.bat
IF /I "%LP3D_INSTALL_PKG_ONLY%" == "1" (
  ECHO.
  SETLOCAL ENABLEDELAYEDEXPANSION
  SET EXE_PRODUCT_DIR=%PACKAGE%-Any-%LP3D_APP_VERSION_LONG%
  SET EXE_TARGET_DIR=builds\windows\%CONFIGURATION%\!EXE_PRODUCT_DIR!
  IF %PLATFORM_ARCH%==ARM64 (
    ECHO -Package %PACKAGE% ARM64 distribution...
    SET EXE=!EXE_TARGET_DIR!\%PACKAGE%_ARM64\%PACKAGE%%d%.exe
    IF NOT EXIST "!EXE!" (
      ECHO.
      ECHO -ERROR - !EXE! was not found. Cannot create install package.
      GOTO :ERROR_END
    )
  ) ELSE (
    ECHO -Package %PACKAGE% x86_64 and x86 distributions...
    SET EXE=!EXE_TARGET_DIR!\%PACKAGE%_x86_64\%PACKAGE%%d%.exe
    IF NOT EXIST "!EXE!" (
      ECHO.
      ECHO -ERROR - !EXE! was not found. Cannot create install package.
      GOTO :ERROR_END
    )
    SET EXE=!EXE_TARGET_DIR!\%PACKAGE%_x86\%PACKAGE%%d%.exe
    IF NOT EXIST "!EXE!" (
      ECHO.
      ECHO -ERROR - !EXE! was not found. Cannot create install package.
      GOTO :ERROR_END
    )
  )
  rem Perform build check if specified
  IF %CHECK%==1 (CALL :BUILD_CHECK x86) ELSE (CALL :ADD_LDRAW_LIBS_TO_EXTRAS)
  ENDLOCAL
  GOTO :END
)

rem Check if build renderers
IF /I "%RENDERERS_ONLY%"=="1" (
  GOTO :BUILD_RENDERERS
)

rem Check if build all platforms
IF /I "%PLATFORM_ARCH%"=="-all_amd" (
  GOTO :BUILD_ALL_AMD
)

rem If build Win32, set to vs2017 for WinXP compat
CALL :CONFIGURE_VCTOOLS %PLATFORM_ARCH%
rem Configure build arguments and set environment variables
CALL :CONFIGURE_BUILD_ENV
CD /D "%ABS_WD%"
ECHO.
ECHO -Building %PACKAGE% %PLATFORM_ARCH% platform, %CONFIGURATION% configuration...
ECHO.
rem Build 3rd party build from source
IF %BUILD_THIRD%==1 ECHO -----------------------------------------------------
IF %BUILD_THIRD%==1 (CALL builds\utilities\CreateRenderers.bat %PLATFORM_ARCH%)
IF NOT ERRORLEVEL 0 (GOTO :ERROR_END)
IF %BUILD_THIRD%==1 ECHO -----------------------------------------------------
IF %BUILD_THIRD%==1 ECHO.
rem Display QMake version
IF "%LP3D_QMAKE_VER%" EQU "" SET LP3D_QMAKE_VER=1
IF "%LP3D_QMAKE_VER%" EQU "1" (qmake -v & ECHO.)
rem Configure makefiles
IF "%LP3D_QMAKE_RUN%" EQU "" SET LP3D_QMAKE_RUN=1
IF "%LP3D_QMAKE_RUN%" EQU "1" (
  qmake %LPUB3D_CONFIG_ARGS%
  IF "%LP3D_ARM64_QMAKE%" EQU "1" (
    SET LP3D_ARM64_QMAKE=0
    ECHO.
    GOTO :ARM64_BUILD
  ) ELSE (
    GOTO :END
  )
)
rem perform build
nmake.exe %LPUB3D_MAKE_ARGS%
rem Check build status
IF %PLATFORM_ARCH%==x86 (SET EXE=mainApp\32bit_%CONFIGURATION%\%PACKAGE%%d%.exe)
IF %PLATFORM_ARCH%==x86_64 (SET EXE=mainApp\64bit_%CONFIGURATION%\%PACKAGE%%d%.exe)
IF %PLATFORM_ARCH%==ARM64 (SET EXE=mainApp\64bit_%CONFIGURATION%\%PACKAGE%%d%.exe)
IF NOT EXIST "%EXE%" (
  ECHO.
  ECHO -ERROR - %EXE% was not successfully built.
  GOTO :ERROR_END
)
rem Package 3rd party install content - this must come before check so check can use staged content for test
IF %PKG_INSTALL%==1 CALL :STAGE_PKG_INSTALL
CALL :ELAPSED_BUILD_TIME %platform_build_start%
ECHO.
ECHO -Elapsed %PACKAGE% %PLATFORM_ARCH% build time %LP3D_ELAPSED_BUILD_TIME%
rem Perform build check if specified
SET "PKG_TARGET_DIR=%BUILD_WORKSPACE%\builds\windows\%CONFIGURATION%\%PACKAGE%-Any-%LP3D_APP_VERSION_LONG%\%PACKAGE%_%PLATFORM_ARCH%"
IF %CHECK%==1 (CALL :BUILD_CHECK %PLATFORM_ARCH%) ELSE (CALL :ADD_LDRAW_LIBS_TO_EXTRAS)
IF ERRORLEVEL 0 (
  IF "%LP3D_CONDA_BUILD%" EQU "True" (
    PUSHD "%LIBRARY_PREFIX%"
    XCOPY /Q /S /I /E /V /Y /H "%PKG_TARGET_DIR%" bin >NUL 2>&1
    IF NOT ERRORLEVEL 0 (
      POPD
      ECHO.
      ECHO -ERROR - Failed to copy %PKG_TARGET_DIR% to %LIBRARY_PREFIX%\bin.
      GOTO :ERROR_END
    )
    POPD
  )
)
GOTO :END

:BUILD_ALL_AMD
rem Launch qmake/make across all platform builds
ECHO.
ECHO -Build LPub3D x86 and x86_64 platforms...
FOR %%P IN ( x86, x86_64 ) DO (
  SETLOCAL ENABLEDELAYEDEXPANSION
  SET platform_build_start=%time%
  SET PLATFORM_ARCH=%%P
  CALL :CONFIGURE_VCTOOLS
  CALL :CONFIGURE_BUILD_ENV
  CD /D "%ABS_WD%"
  IF %BUILD_THIRD%==1 ECHO.
  IF %BUILD_THIRD%==1 ECHO -----------------------------------------------------
  IF %BUILD_THIRD%==1 (CALL builds\utilities\CreateRenderers.bat %%P)
  IF ERRORLEVEL 3 (GOTO :ERROR_END)
  IF %BUILD_THIRD%==1 ECHO -----------------------------------------------------
  IF %BUILD_THIRD%==1 ECHO.
  ECHO -Building  %PACKAGE% %%P platform, %CONFIGURATION% configuration...
  ECHO.
  qmake -v & ECHO.
  qmake !LPUB3D_CONFIG_ARGS! & nmake.exe !LPUB3D_MAKE_ARGS!
  IF %%P==x86 (SET EXE=mainApp\32bit_%CONFIGURATION%\%PACKAGE%%d%.exe)
  IF %%P==x86_64 (SET EXE=mainApp\64bit_%CONFIGURATION%\%PACKAGE%%d%.exe)
  IF NOT EXIST "!EXE!" (
    ECHO.
    ECHO -ERROR - !EXE! was not successfully built.
    GOTO :ERROR_END
  )
  IF %PKG_INSTALL%==1 (CALL :STAGE_PKG_INSTALL)
  CALL :ELAPSED_BUILD_TIME !platform_build_start!
  ECHO.
  ECHO -Elapsed %%P package build time !LP3D_ELAPSED_BUILD_TIME!
  ENDLOCAL
  IF %CHECK%==1 (CALL :BUILD_CHECK %%P) ELSE (CALL :ADD_LDRAW_LIBS_TO_EXTRAS %%P)
)
GOTO :END

:BUILD_RENDERERS
rem Check if build all platforms
IF /I "%PLATFORM_ARCH%"=="-all_amd" (
  SET ALL_RENDERERS=x86, x86_64
  GOTO :BUILD_ALL_RENDERERS
)
IF /I "%PLATFORM_ARCH%"=="ARM64" (
  SET ALL_RENDERERS=ARM64
  GOTO :BUILD_ALL_RENDERERS
)

rem Configure buid arguments and set environment variables
CALL :CONFIGURE_BUILD_ENV
CD /D "%ABS_WD%"
rem Build renderer from source
ECHO.
ECHO -Building Renderers for %PLATFORM_ARCH% platform, %CONFIGURATION% configuration...
ECHO -----------------------------------------------------
CALL builds\utilities\CreateRenderers.bat %PLATFORM_ARCH%
IF %ERRORLEVEL% NEQ 0 (GOTO :ERROR_END)
ECHO -----------------------------------------------------
ECHO.
GOTO :END

:BUILD_ALL_RENDERERS
FOR %%P IN ( %ALL_RENDERERS% ) DO (
  SET PLATFORM_ARCH=%%P
  rem Configure build arguments and set environment variables
  CALL :CONFIGURE_BUILD_ENV
  CD /D "%ABS_WD%"
  rem Build renderer from source
  ECHO.
  ECHO -Building Renderers for %%P platform, %CONFIGURATION% configuration...
  ECHO -----------------------------------------------------
  CALL builds\utilities\CreateRenderers.bat %%P
  IF ERRORLEVEL 3 (GOTO :ERROR_END)
  ECHO -----------------------------------------------------
  ECHO.
)
GOTO :END

:CONFIGURE_VCTOOLS
ECHO.
ECHO -Set MSBuild platform toolset...
IF %PLATFORM_ARCH%==x86_64 (
  IF "%LP3D_CONDA_BUILD%" NEQ "True" (
    SET LP3D_MSC_VER=1944
    SET LP3D_VCSDKVER=%LP3D_VC64SDKVER%
    SET LP3D_VCTOOLSET=%LP3D_VC64TOOLSET%
    SET LP3D_VCVARSALL_VER=%LP3D_VC64VARSALL_VER%
  )
) ELSE (
  IF %PLATFORM_ARCH%==ARM64 (
    SET LP3D_MSC_VER=%LP3D_MSCARM64_VER%
    SET LP3D_VCSDKVER=%LP3D_VCARM64SDKVER%
    SET LP3D_VCTOOLSET=%LP3D_VCARM64TOOLSET%
    SET LP3D_VCVARSALL_VER=%LP3D_VCARM64VARSALL_VER%
  ) ELSE (
    SET LP3D_MSC_VER=%LP3D_MSC32_VER%
    SET LP3D_VCSDKVER=%LP3D_VC32SDKVER%
    SET LP3D_VCTOOLSET=%LP3D_VC32TOOLSET%
    SET LP3D_VCVARSALL_VER=%LP3D_VC32VARSALL_VER%
  )
)
ECHO.
ECHO   PLATFORM_ARCHITECTURE..[%PLATFORM_ARCH%]
ECHO   MSVS_VERSION...........[%LP3D_VSVERSION%]
IF %PLATFORM_ARCH%==-all_amd (
  ECHO   MSVC_QT32_VERSION......[%LP3D_QT32VCVERSION%]
  ECHO   MSVC_QT64_VERSION......[%LP3D_QT64VCVERSION%]
) ELSE (
  IF %PLATFORM_ARCH%==x86 (
    ECHO   MSVC_QT32_VERSION......[%LP3D_QT32VCVERSION%]
  )
  IF %PLATFORM_ARCH%==x86_64 (
    ECHO   MSVC_QT64_VERSION......[%LP3D_QT64VCVERSION%]
  )
  IF %PLATFORM_ARCH%==ARM64 (
    ECHO   MSVC_QT64_VERSION......[%LP3D_QT64VCVERSION%]
  )
)
ECHO   MSVC_MSC_VERSION.......[%LP3D_MSC_VER%]
ECHO   MSVC_SDK_VERSION.......[%LP3D_VCSDKVER%]
ECHO   MSVC_TOOLSET...........[%LP3D_VCTOOLSET%]
IF "%LP3D_CONDA_BUILD%" NEQ "True" (
  IF %PLATFORM_ARCH%==x86 (ECHO   LP3D_QT32_MSVC.........[%LP3D_QT32_MSVC%])
)
IF %PLATFORM_ARCH%==x86_64 (ECHO   LP3D_QT64_MSVC.........[%LP3D_QT64_MSVC%])
IF %PLATFORM_ARCH%==ARM64 (ECHO   LP3D_QT64_MSVC.........[%LP3D_QT64_MSVC%])
ECHO   MSVC_VCVARSALL_VER.....[%LP3D_VCVARSALL_VER%]
ECHO   MSVC_VCVARSALL_DIR.....[%LP3D_VCVARSALL_DIR%]
EXIT /b

:CONFIGURE_BUILD_ENV
CD /D %ABS_WD%
IF %PLATFORM_ARCH%==x86 (SET _AR=32) ELSE (SET _AR=64)
IF "%LP3D_QMAKE_CLEAN%" EQU "" SET LP3D_QMAKE_CLEAN=1
IF "%LP3D_QMAKE_CLEAN%" EQU "1" (
  ECHO.
  ECHO -Configure LPub3D %PLATFORM_ARCH% build environment...
  ECHO.
  ECHO -Cleanup any previous LPub3D qmake config files...
  FOR %%I IN (
    ".qmake.stash"
    "Makefile*"
    "lclib\Makefile*"
    "ldrawini\Makefile*"
    "ldvlib\LDVQt\Makefile*"
    "ldvlib\LDVQt\LDView\3rdParty\gl2ps\Makefile*"
    "ldvlib\LDVQt\LDView\3rdParty\lib3ds\Makefile*"
    "ldvlib\LDVQt\LDView\3rdParty\libjpeg\Makefile*"
    "ldvlib\LDVQt\LDView\3rdParty\libpng\Makefile*"
    "ldvlib\LDVQt\LDView\3rdParty\minizip\Makefile*"
    "ldvlib\LDVQt\LDView\3rdParty\tinyxml\Makefile*"
    "ldvlib\LDVQt\LDView\3rdParty\zlib\Makefile*"
    "ldvlib\LDVQt\LDView\LDExporter\Makefile*"
    "ldvlib\LDVQt\LDView\LDLib\Makefile*"
    "ldvlib\LDVQt\LDView\LDLoader\Makefile*"
    "ldvlib\LDVQt\LDView\TCFoundation\Makefile*"
    "ldvlib\LDVQt\LDView\TRE\Makefile*"
    "mainApp\Makefile*"
    "quazip\Makefile*"
    "waitingspinner\Makefile*"
  ) DO IF EXIST "%%~I" (ECHO   Clean %%~I & DEL /S /Q "%%~I" >NUL 2>&1)
  ECHO.
  ECHO -Cleanup any previous LPub3D %_AR%bit build folders...
  FOR %%I IN (
    "lclib\%_AR%bit_%CONFIGURATION%"
    "ldrawini\%_AR%bit_%CONFIGURATION%"
    "ldvlib\LDVQt\%_AR%bit_%CONFIGURATION%"
    "ldvlib\LDVQt\LDView\3rdParty\gl2ps\%_AR%bit_%CONFIGURATION%"
    "ldvlib\LDVQt\LDView\3rdParty\lib3ds\%_AR%bit_%CONFIGURATION%"
    "ldvlib\LDVQt\LDView\3rdParty\libjpeg\%_AR%bit_%CONFIGURATION%"
    "ldvlib\LDVQt\LDView\3rdParty\libpng\%_AR%bit_%CONFIGURATION%"
    "ldvlib\LDVQt\LDView\3rdParty\minizip\%_AR%bit_%CONFIGURATION%"
    "ldvlib\LDVQt\LDView\3rdParty\tinyxml\%_AR%bit_%CONFIGURATION%"
    "ldvlib\LDVQt\LDView\3rdParty\zlib\%_AR%bit_%CONFIGURATION%"
    "ldvlib\LDVQt\LDView\LDExporter\%_AR%bit_%CONFIGURATION%"
    "ldvlib\LDVQt\LDView\LDLib\%_AR%bit_%CONFIGURATION%"
    "ldvlib\LDVQt\LDView\LDLoader\%_AR%bit_%CONFIGURATION%"
    "ldvlib\LDVQt\LDView\TCFoundation\%_AR%bit_%CONFIGURATION%"
    "ldvlib\LDVQt\LDView\TRE\%_AR%bit_%CONFIGURATION%"
    "mainApp\%_AR%bit_%CONFIGURATION%"
    "quazip\%_AR%bit_%CONFIGURATION%"
    "waitingspinner\%_AR%bit_%CONFIGURATION%"
  ) DO IF EXIST "%%~I" (ECHO   Clean %%~I & RD /S /Q "%%~I" >NUL 2>&1)
)
ECHO.
IF "%LP3D_ARM64_QMAKE%" NEQ "1" (
  IF %CHECK% EQU 1 (
    IF "%PLATFORM_ARCH%" EQU "x86_64" (
      IF "%BUILD_WORKER%" EQU "True" (
        IF "%LP3D_CONDA_BUILD%" EQU "True" (
          ECHO   LP3D_BUILD_CHECK...............[Yes]
        ) ELSE (
          ECHO   -Disable Build Check for %PLATFORM_ARCH% %BUILD_WORKER_ID% build
          ECHO.
          ECHO   LP3D_BUILD_CHECK...............[No]
        )
      ) ELSE (
        ECHO   LP3D_BUILD_CHECK...............[Yes]
      )
    ) ELSE (
      ECHO   LP3D_BUILD_CHECK...............[Yes]
    )
  ) ELSE (
    ECHO   LP3D_BUILD_CHECK...............[No]
  )
)

SET "LPUB3D_CONFIG_ARGS=CONFIG+=%CONFIGURATION% CONFIG-=debug_and_release"
SET LPUB3D_MAKE_ARGS=-c -f Makefile
IF "%LP3D_ARM64_BUILD%" NEQ "1" (
  ECHO   LPUB3D_MAKE_ARGS...............[%LPUB3D_MAKE_ARGS%]
)
IF "%BUILD_WORKER%" EQU "True" (
  SET LPUB3D_CONFIG_ARGS=%LPUB3D_CONFIG_ARGS% CONFIG+=%CONFIG_CI%
)
IF "%APPVEYOR%" EQU "True" (
  SET LPUB3D_CONFIG_ARGS=%LPUB3D_CONFIG_ARGS% CONFIG+=%CONFIG_CI%
)
IF "%BUILD_WORKER%" NEQ "True" (
  IF "%APPVEYOR%" NEQ "True" (
    SET LP3D_DIST_DIR_PATH=%CD%\%DIST_DIR%
  )
)
IF "%LP3D_CONDA_BUILD%" EQU "True" (
  SET "LPUB3D_CONFIG_ARGS=%LPUB3D_CONFIG_ARGS% CONFIG+=con"
  SET "PATH=%PATH%"
  GOTO :COMPILER_SETTINGS
) ELSE (
  SET "LPUB3D_CONFIG_ARGS=%LPUB3D_CONFIG_ARGS% CONFIG+=exe"
)
rem Set vcvars for AppVeyor or local build environments
SET "PATH_ADDITIONS=%LP3D_SYS_DIR%"
IF "%LP3D_WIN_GIT%" NEQ "" (
  SET "PATH_ADDITIONS=%PATH_ADDITIONS%;%LP3D_WIN_GIT%"
)
SET "PATH=%PATH_ADDITIONS%;%PATH%"
IF "%PLATFORM_ARCH%" EQU "x86_64" (
  SET LP3D_VCVARS=vcvars64.bat
)
IF "%PLATFORM_ARCH%" EQU "ARM64" (
  SET LP3D_VCVARS=vcvarsamd64_arm64.bat
  SET LPUB3D_CONFIG_ARGS=-config %CONFIGURATION% %LPUB3D_CONFIG_ARGS%
)
IF "%LP3D_ARM64_BUILD%" NEQ "1" (
  ECHO   LPUB3D_CONFIG_ARGS.............[%LPUB3D_CONFIG_ARGS%]
)
ECHO.
IF "%PLATFORM_ARCH%" EQU "x86" (
  IF EXIST "%LP3D_VCVARSALL_DIR%\vcvars32.bat" (
    SET "LP3D_VCVARSALL_BAT=%LP3D_VCVARSALL_DIR%\vcvars32.bat"
  ) ELSE (
    ECHO.
    ECHO -ERROR - vcvars32.bat not found.
    GOTO :ERROR_END
  )
  IF EXIST "%LP3D_QT32_MSVC%\qtenv2.bat" (
    SET "LP3D_QTENV_BAT=%LP3D_QT32_MSVC%\qtenv2.bat"
  ) ELSE (
    SET "LP3D_QT_MSVC_PATH=%LP3D_QT32_MSVC%"
  )
) ELSE (
  IF EXIST "%LP3D_VCVARSALL_DIR%\%LP3D_VCVARS%" (
    SET "LP3D_VCVARSALL_BAT=%LP3D_VCVARSALL_DIR%\%LP3D_VCVARS%"
  ) ELSE (
    ECHO.
    ECHO -ERROR - %LP3D_VCVARS% not found.
    GOTO :ERROR_END
  )
  IF EXIST "%LP3D_QT64_MSVC%\qtenv2.bat" (
    SET "LP3D_QTENV_BAT=%LP3D_QT64_MSVC%\qtenv2.bat"
  ) ELSE (
    SET "LP3D_QT_MSVC_PATH=%LP3D_QT64_MSVC%"
  )
)

SET PATH_PREPENDED=True
CALL "%LP3D_VCVARSALL_BAT%" %LP3D_VCVARSALL_VER%
ECHO.
IF "%LP3D_QTENV_BAT%" EQU "" (GOTO :ADD_QT_MSVC_PATH)
CALL "%LP3D_QTENV_BAT%"
ECHO(   PATH_QTENV_PREPEND......[%PATH%])
GOTO :COMPILER_SETTINGS

:ADD_QT_MSVC_PATH
IF "%LP3D_QT_MSVC_PATH%" EQU "" (GOTO :COMPILER_SETTINGS)
SET "PATH=%LP3D_QT_MSVC_PATH%;%PATH%"
ECHO(   PATH_QTMSVC_PREPEND.....[%PATH%])

:COMPILER_SETTINGS
IF "%LP3D_ARM64_QMAKE%" NEQ "1" (EXIT /b)
rem Display MSVC Compiler settings
ECHO.
ECHO -Display _MSC_VER %LP3D_MSC_VER% compiler settings
ECHO.
ECHO.%LP3D_MSC_VER% > %TEMP%\settings.c
cl.exe -Bv -EP %TEMP%\settings.c >NUL
EXIT /b

:BUILD_CHECK
ECHO.
REM DEBUG============
IF "%1" EQU "x86" (
  ECHO -%PACKAGE% %1 Build Check...
) ELSE (
  IF "%1" EQU "x86_64" (
    IF "%BUILD_WORKER%" EQU "True" (
      IF "%LP3D_CONDA_BUILD%" EQU "True" (
        ECHO -%PACKAGE% %1 Build Check...
      ) ELSE (
        ECHO -%PACKAGE% %1 Build Check disabled for %PLATFORM_ARCH% %BUILD_WORKER_ID% build
        EXIT /b
      )
    ) ELSE (
      ECHO -%PACKAGE% %1 Build Check...
    )
  ) ELSE (
    IF "%1" EQU "ARM64" (
      IF %LP3D_AMD64_ARM64_CROSS% EQU 1 (
        ECHO -%PACKAGE% %1 Build Check skipped on ARM64 cross compilation.
        EXIT /b
      ) ELSE (
        ECHO -%PACKAGE% %1 Build Check...
      )
    )
  )
)
REM DEBUG============
IF [%1] EQU [] (
  ECHO.
  ECHO -ERROR - No Platform defined, build check will exit.
  EXIT /b
)
IF NOT EXIST "%DIST_DIR%" (
  ECHO.
  ECHO -ERROR - 3rd Party Renderer folder '%DIST_DIR%' not found, build check will exit.
  EXIT /b
)

SET PKG_PLATFORM=%1
IF %CHECK% == 1 CALL :REQUEST_LDRAW_DIR
rem run build checks
CALL builds\check\build_checks.bat
EXIT /b

:STAGE_PKG_INSTALL
ECHO.
ECHO -Staging distribution files...
ECHO.
rem Perform build and stage package components
nmake.exe %LPUB3D_MAKE_ARGS% install
EXIT /b

:ADD_LDRAW_LIBS_TO_EXTRAS
IF NOT EXIST "%LDRAW_LIBS%\" (
  ECHO.
  ECHO -ERROR - LDraw store folder %LDRAW_LIBS% does not exist.
  EXIT /b 1
)
SET PLATFORM_ARCH=%1
IF [%PLATFORM_ARCH%] EQU [] (
  IF [%PKG_TARGET_DIR%] EQU [] (
    ECHO.
    ECHO -ERROR - Unable to define package directory. Platform architecture was not provided.
    EXIT /b 1
  )
) ELSE (
  IF [%PKG_TARGET_DIR%] EQU [] (
    SET "PKG_TARGET_DIR=%BUILD_WORKSPACE%\builds\windows\%CONFIGURATION%\%PACKAGE%-Any-%LP3D_APP_VERSION_LONG%\%PACKAGE%_%PLATFORM_ARCH%"
  )
)
ECHO.
ECHO - Copy LDraw archive libraries to extras folder...
IF NOT EXIST "%PKG_TARGET_DIR%\extras\%OfficialCONTENT%" (
  IF EXIST "%LDRAW_LIBS%\%OfficialCONTENT%" (
    COPY /V /Y /B "%LDRAW_LIBS%\%OfficialCONTENT%" "%PKG_TARGET_DIR%\extras\" >NUL 2>&1
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
    COPY /V /Y /B "%LDRAW_LIBS%\%TenteCONTENT%" "%PKG_TARGET_DIR%\extras\"  >NUL 2>&1
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
    COPY /V /Y /B "%LDRAW_LIBS%\%VexiqCONTENT%" "%PKG_TARGET_DIR%\extras\" >NUL 2>&1
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
    COPY /V /Y /B "%LDRAW_LIBS%\%LPub3DCONTENT%" "%PKG_TARGET_DIR%\extras\" >NUL 2>&1
  ) ELSE (
    ECHO.
    ECHO -ERROR - LDraw archive lib %LPub3DCONTENT% does not exist in %LDRAW_LIBS%\.
  )
) ELSE (
  ECHO.
  ECHO - Archive library %LPub3DCONTENT% exist in extras folder. Nothing to do.
)
EXIT /b

:REQUEST_LDRAW_DIR
ECHO.
ECHO -Request LDraw archive libraries download...
CALL :DOWNLOAD_LDRAW_LIBS
ECHO.
ECHO -Check for LDraw LEGO disk library...
IF NOT EXIST "%LDRAW_DIR%\parts" (
  ECHO.
  ECHO -LDraw directory %LDRAW_DIR% does not exist - creating...
  REM SET CHECK=0
  IF NOT EXIST "%LDRAW_INSTALL_ROOT%\%OfficialCONTENT%" (
    IF EXIST "%LDRAW_LIBS%\%OfficialCONTENT%" (
      COPY /V /Y /B "%LDRAW_LIBS%\%OfficialCONTENT%" "%LDRAW_INSTALL_ROOT%\" >NUL 2>&1
    ) ELSE (
      ECHO.
      ECHO -[WARNING] Could not find %LDRAW_LIBS%\%OfficialCONTENT%.
      SET CHECK=0
    )
  )
  IF %LP3D_VALID_TAR% == 1 (
    ECHO.
    ECHO -Extracting %OfficialCONTENT%...
    ECHO.
    PUSHD "%LDRAW_INSTALL_ROOT%"
    "%LP3D_WIN_TAR%" -xf "%OfficialCONTENT%"
    POPD
    IF EXIST "%LDRAW_DIR%\parts" (
      ECHO.
      ECHO -LDraw directory %LDRAW_DIR% extracted.
      ECHO.
      ECHO -Cleanup %OfficialCONTENT%...
      DEL /Q "%LDRAW_INSTALL_ROOT%\%OfficialCONTENT%"
      ECHO.
      ECHO -Set LDRAWDIR to %LDRAW_DIR%.
      SET LDRAWDIR=%LDRAW_DIR%
    )
  )
) ELSE (
  ECHO.
  ECHO -LDraw directory exist at [%LDRAW_DIR%].
  ECHO.
  ECHO -Set LDRAWDIR to %LDRAW_DIR%.
  SET LDRAWDIR=%LDRAW_DIR%
)
EXIT /b

:DOWNLOAD_LDRAW_LIBS
ECHO.
ECHO - Download LDraw archive libraries...

IF NOT EXIST "%LDRAW_LIBS%\" (
  ECHO.
  ECHO - Create LDraw archive libs store %LDRAW_LIBS%
  MKDIR "%LDRAW_LIBS%\"
)

SET OutputPATH=%LDRAW_LIBS%

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
ECHO.
ECHO - LDraw archive library download path: %OutputPATH%

IF "%BUILD_WORKER%" EQU "True" (
  IF NOT DEFINED LP3D_UPDATE_LDRAW_LIBS (
    SET LP3D_UPDATE_LDRAW_LIBS=-unofficial
  )
)

IF "%APPVEYOR%" EQU "True" (
  IF NOT DEFINED LP3D_UPDATE_LDRAW_LIBS (
    SET LP3D_UPDATE_LDRAW_LIBS=-unofficial
  )
)

IF "%LP3D_UPDATE_LDRAW_LIBS%" EQU "-true" (
  GOTO :UPDATE_ALL_LIBRARIES
)

IF "%LP3D_UPDATE_LDRAW_LIBS%" EQU "-ldraw" (
  GOTO :UPDATE_LDRAW_LIBRARIES
)

IF NOT EXIST "%OutputPATH%\%OfficialCONTENT%" (
  CALL :GET_OFFICIAL_LIBRARY
)  ELSE (
  IF "%LP3D_UPDATE_LDRAW_LIBS%" EQU "-official" (
    DEL /Q "%OutputPATH%\%OfficialCONTENT%"
    CALL :GET_OFFICIAL_LIBRARY
  ) ELSE (
    ECHO.
    ECHO - LDraw archive library %OfficialCONTENT% exist. Nothing to do.
  )
)
IF NOT EXIST "%OutputPATH%\%TenteCONTENT%" (
  CALL :GET_TENTE_LIBRARY
) ELSE (
  IF "%LP3D_UPDATE_LDRAW_LIBS%" EQU "-tente" (
    DEL /Q "%OutputPATH%\%TenteCONTENT%"
    CALL :GET_TENTE_LIBRARY
  ) ELSE (
    ECHO.
    ECHO - LDraw archive library %TenteCONTENT% exist. Nothing to do.
  )
)
IF NOT EXIST "%OutputPATH%\%VexiqCONTENT%" (
  CALL :GET_VEXIQ_LIBRARY
) ELSE (
  IF "%LP3D_UPDATE_LDRAW_LIBS%" EQU "-vexiq" (
    DEL /Q "%OutputPATH%\%VexiqCONTENT%"
    CALL :GET_VEXIQ_LIBRARY
  ) ELSE (
    ECHO.
    ECHO - LDraw archive library %VexiqCONTENT% exist. Nothing to do.
  )
)
IF NOT EXIST "%OutputPATH%\%LPub3DCONTENT%" (
  CALL :GET_UNOFFICIAL_LIBRARY
) ELSE (
  IF "%LP3D_UPDATE_LDRAW_LIBS%" EQU "-unofficial" (
    DEL /Q "%OutputPATH%\%LPub3DCONTENT%"
    CALL :GET_UNOFFICIAL_LIBRARY
  ) ELSE (
    ECHO.
    ECHO - LDraw archive library %LPub3DCONTENT% exist. Nothing to do.
  )
)
EXIT /b

:UPDATE_ALL_LIBRARIES
ECHO.
ECHO - Update all libraries...

IF EXIST "%OutputPATH%\%OfficialCONTENT%" (
  DEL /Q "%OutputPATH%\%OfficialCONTENT%"
  CALL :GET_OFFICIAL_LIBRARY
)
IF EXIST "%OutputPATH%\%LPub3DCONTENT%" (
  DEL /Q "%OutputPATH%\%LPub3DCONTENT%"
  CALL :GET_UNOFFICIAL_LIBRARY
)
IF EXIST "%OutputPATH%\%TenteCONTENT%" (
  DEL /Q "%OutputPATH%\%TenteCONTENT%"
  CALL :GET_TENTE_LIBRARY
)
IF EXIST "%OutputPATH%\%VexiqCONTENT%" (
  DEL /Q "%OutputPATH%\%VexiqCONTENT%
  CALL :GET_VEXIQ_LIBRARY
)
EXIT /b

:UPDATE_LDRAW_LIBRARIES
ECHO.
ECHO - Update official and unofficial LDraw libraries...

IF EXIST "%OutputPATH%\%OfficialCONTENT%" (
  DEL /Q "%OutputPATH%\%OfficialCONTENT%"
  CALL :GET_OFFICIAL_LIBRARY
)
IF EXIST "%OutputPATH%\%LPub3DCONTENT%" (
  DEL /Q "%OutputPATH%\%LPub3DCONTENT%"
  CALL :GET_UNOFFICIAL_LIBRARY
)
EXIT /b

:GET_OFFICIAL_LIBRARY
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

:WD_ABS_PATH
IF [%1] EQU [] (EXIT /B) ELSE SET ABS_WD=%~f1
IF %ABS_WD:~-1%==\ SET ABS_WD=%ABS_WD:~0,-1%
EXIT /b

:DIST_DIR_ABS_PATH
IF [%1] EQU [] (EXIT /B) ELSE SET DIST_DIR=%~f1
IF %DIST_DIR:~-1%==\ SET DIST_DIR=%DIST_DIR:~0,-1%
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
SET LP3D_ELAPSED_BUILD_TIME=%hours%:%mins%:%secs%.%ms%
EXIT /b

:PLATFORM_ERROR
ECHO.
ECHO -01. (PLATFORM_ERROR) Platform or usage flag is invalid. Use x86, x86_64, arm64 or -all_amd [%~nx0 %*].
ECHO      See Usage.
ECHO.
CALL :USAGE
GOTO :ERROR_END

:CONFIGURATION_ERROR
ECHO.
ECHO -02. (CONFIGURATION_ERROR) Configuration flag is invalid [%~nx0 %*].
ECHO      See Usage.
ECHO.
CALL :USAGE
GOTO :ERROR_END

:COMMAND_ERROR
ECHO.
ECHO -03. (COMMAND_ERROR) Invalid command string [%~nx0 %*].
ECHO      See Usage.
ECHO.
CALL :USAGE
GOTO :ERROR_END

:LIBRARY_ERROR
ECHO.
ECHO -04. (LIBRARY_ERROR) Qt MSVC library [%LP3D_QT32_MSVC%] or [%LP3D_QT64_MSVC%] required for command not found [%~nx0 %*].
ECHO      See Usage.
CALL :USAGE
ECHO.
GOTO :ERROR_END

:USAGE
ECHO ----------------------------------------------------------------
ECHO.
ECHO %PACKAGE% Windows auto build script.
ECHO.
ECHO NOTE: To successfully run all options of this script locally,
ECHO you must have Win32, Win64 and ARM64 Qt. Qt5.15.2 MSVC 2019
ECHO supports 32bit AMD and Qt6.2 MSVC 2022 supports both 64bit AMD
ECHO and ARM64 so these versions are a convenient choice.
ECHO.
ECHO ----------------------------------------------------------------
ECHO Usage:
ECHO  build [ -help]
ECHO  build [ x86 ^| x86_64 ^| arm64 ^| -all_amd ] [ -chk ^| -ins ^| -3rd ^| -ren ] [ -chk ^| -ins ^| -asl  ^| -ldraw  ^| -official ^| -unofficial ^| -tente ^| -vexiq ] [ -chk ^| -asl ^| -ldraw  ^| -official ^| -unofficial ^| -tente ^| -vexiq ]
ECHO.
ECHO ----------------------------------------------------------------
ECHO Build AMD 64bit, Release and perform build check
ECHO build x86_64 -chk
ECHO.
ECHO Build ARM 64bit, Release and perform install and build check
ECHO build arm64 -ins -chk
ECHO.
ECHO Build AMD 32bit, Release and perform build check
ECHO build x86 -chk
ECHO.
ECHO Build AMD 64bit and 32bit, 3rdParty renderers
ECHO build -all_amd -ren
ECHO.
ECHO Build AMD 64bit and 32bit, Release and perform build check
ECHO build -all_amd -ren
ECHO.
ECHO Build AMD 64bit and 32bit, Release, perform install and build check
ECHO build -all_amd -ins -chk
ECHO.
ECHO Build AMD 64bit and 32bit, Release, build 3rd party renderers, perform install and build check
ECHO build -all_amd -ins -chk
ECHO.
ECHO Commands:
ECHO ----------------------------------------------------------------
ECHO [PowerShell]: cmd.exe /c builds\windows\AutoBuild.bat [Flags]
ECHO [cmd.exe]   : builds\windows\AutoBuild.bat [Flags]
ECHO.
ECHO Flags:
ECHO ----------------------------------------------------------------
ECHO ^| Flag     ^| Pos  ^| Type              ^| Description
ECHO ----------------------------------------------------------------
ECHO  -help.......1........Useage flag         [Default=Off] Display useage.
ECHO  x86.........1........Platform flag       [Default=Off] Build AMD 32bit platform.
ECHO  x86_64......1........Platform flag       [Default=Off] Build AMD 64bit platform.
ECHO  arm64.......1........Platform flag       [Default=Off] Build ARM 64bit platform.
ECHO  -all_amd....1........Configuraiton flag  [Default=On ] Build both AMD 32bit and 64bit PLATFORM_ARCHs - Requries Qt libraries for both PLATFORMs.
ECHO  -3rd..........2......Project flag        [Default=Off] Build 3rdparty renderers - LDGLite, LDView, and LPub3D-Trace (POV-Ray) from source
ECHO  -ren..........2......Project flag        [Default=Off] Build 3rdparty renderers only - LPub3D not built
ECHO  -ins..........2,3....Project flag        [Default=Off] Install LPub3D distribution and 3rd party artefacts to 'product' folder
ECHO  -chk..........2,3,4..Project flag        [Default=Off] Perform a build check
ECHO  -asl............3,4..Project flag        [Default=Off] Force update all supported libraries
ECHO  -ldraw..........3,4..Project flag        [Default=Off] Force update LDraw Official and Unofficial libraries
ECHO  -unofficial.....3,4..Project flag        [Default=Off] Force update Unofficial LDraw library
ECHO  -official.......3,4..Project flag        [Default=Off] Force update Official LDraw library
ECHO  -tente..........3,4..Project flag        [Default=Off] Force update Tente LDraw library
ECHO  -vexiq..........3,4..Project flag        [Default=Off] Force update VEXiQ LDraw library
ECHO.
ECHO Be sure the set your LDraw directory in the variables section above if you expect to use the '-chk' option.
ECHO.
ECHO Flags are case insensitive; however, it better to use lowere case.
ECHO.
ECHO If no flag is supplied, 64bit platform, Release Configuration built by default.
ECHO ----------------------------------------------------------------
EXIT /b

:END_STATUS
CALL :ELAPSED_BUILD_TIME
ECHO  Elapsed build time %LP3D_ELAPSED_BUILD_TIME%
EXIT /b

:ERROR_END
ECHO.
ECHO -%PACKAGE% %~nx0 FAILED with return code %ERRORLEVEL%.
ECHO -%~nx0 will terminate!
CALL :END_STATUS
EXIT /b 3

:END
ECHO.
ECHO -%PACKAGE% v%LP3D_VERSION% %~nx0 finished with return code %ERRORLEVEL%.
CALL :END_STATUS
EXIT /b
