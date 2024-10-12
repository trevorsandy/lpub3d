@ECHO OFF &SETLOCAL

Title LPub3D Windows auto build script

rem This script uses Qt to configure and build LPub3D for Windows.
rem The primary purpose is to automatically build both the 32bit and 64bit
rem LPub3D distributions and package the build contents (exe, doc and
rem resources ) for distribution release.
rem --
rem  Trevor SANDY <trevor.sandy@gmail.com>
rem  Last Update: September 12, 2024
rem  Copyright (c) 2019 - 2024 by Trevor SANDY
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
ECHO.

FOR %%* IN (.) DO SET SCRIPT_RUN_DIR=%%~nx*
IF "%SCRIPT_RUN_DIR%" EQU "windows" (
  CALL :WD_ABS_PATH ..\..\
) ELSE (
  SET ABS_WD=%CD%
)

IF "%LP3D_QTVERSION%" == "" SET "LP3D_QTVERSION=5.15.2"
IF "%LP3D_VSVERSION%" == "" SET "LP3D_VSVERSION=2022"
IF "%LP3D_QTVCVERSION%" == "" SET "LP3D_QTVCVERSION=2019"

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
      SET CONFIG_CI=azure_ci_win
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
    SET LP3D_QT32_MSVC=%LP3D_BUILD_BASE%\Qt\%LP3D_QTVERSION%\msvc%LP3D_QTVCVERSION%\bin
  )
  IF "%LP3D_QT64_MSVC%" == "" (
    SET LP3D_QT64_MSVC=%LP3D_BUILD_BASE%\Qt\%LP3D_QTVERSION%\msvc%LP3D_QTVCVERSION%_64\bin
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
    SET LP3D_QT32_MSVC=C:\Qt\%LP3D_QTVERSION%\msvc%LP3D_QTVCVERSION%\bin
  )
  IF "%LP3D_QT64_MSVC%" == "" (
    SET LP3D_QT64_MSVC=C:\Qt\%LP3D_QTVERSION%\msvc%LP3D_QTVCVERSION%_64\bin
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
      SET LP3D_QT32_MSVC=C:\Qt\IDE\%LP3D_QTVERSION%\msvc%LP3D_QTVCVERSION%\bin
    )
    IF "%LP3D_QT64_MSVC%" == "" (
      SET LP3D_QT64_MSVC=C:\Qt\IDE\%LP3D_QTVERSION%\msvc%LP3D_QTVCVERSION%_64\bin
    )
    SET LP3D_UPDATE_LDRAW_LIBS=unknown
  )
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
  ECHO -ERROR - mainApp folder not found. Working directory %ABS_WD% is not valid.
  GOTO :ERROR_END
)

rem Visual C++ 2012 -vcvars_ver=11.0 Toolset v110 VSVersion 11.0    _MSC_VER 1700
rem Visual C++ 2013 -vcvars_ver=12.0 Toolset v120 VSVersion 12.0    _MSC_VER 1800
rem Visual C++ 2015 -vcvars_ver=14.0 Toolset v140 VSVersion 14.0    _MSC_VER 1900
rem Visual C++ 2017 -vcvars_ver=14.1 Toolset v141 VSVersion 15.9    _MSC_VER 1916
rem Visual C++ 2019 -vcvars_ver=14.2 Toolset v142 VSVersion 16.11.3 _MSC_VER 1929
rem Visual C++ 2022 -vcvars_ver=14.4 Toolset v143 VSVersion 17.11.3 _MSC_VER 1941
IF "%LP3D_MSC_VER%" == "" SET LP3D_MSC_VER=1941
IF "%LP3D_VCSDKVER%" == "" SET LP3D_VCSDKVER=8.1
IF "%LP3D_VCTOOLSET%" == "" SET LP3D_VCTOOLSET=v141
IF "%LP3D_VCVARSALL_VER%" == "" SET LP3D_VCVARSALL_VER=-vcvars_ver=14.1
IF "%LP3D_VALID_7ZIP%" =="" SET LP3D_VALID_7ZIP=0

IF "%LP3D_SYS_DIR%" == "" (
  SET LP3D_SYS_DIR=%WINDIR%\System32
)
IF "%LP3D_7ZIP_WIN64%" == "" (
  SET LP3D_7ZIP_WIN64=%ProgramFiles%\7-zip\7z.exe
)
IF "%LP3D_WIN_GIT%" == "" (
  SET LP3D_WIN_GIT=%ProgramFiles%\Git\cmd
)
SET LP3D_WIN_GIT_MSG=%LP3D_WIN_GIT%

SET OfficialCONTENT=complete.zip
SET LPub3DCONTENT=lpub3dldrawunf.zip
SET TenteCONTENT=tenteparts.zip
SET VexiqCONTENT=vexiqparts.zip

SET BUILD_THIRD=unknown
SET INSTALL=unknown
SET INSTALL_32BIT=unknown
SET INSTALL_64BIT=unknown
SET PLATFORM_ARCH=unknown
SET LDCONFIG_FILE=unknown
SET CHECK=unknown

FOR /F "tokens=3*" %%i IN ('findstr /c:"#define VER_PRODUCTNAME_STR" %ABS_WD%\mainApp\version.h') DO SET LP3D_PRODUCT=%%i
SET PACKAGE=%LP3D_PRODUCT:"=%
IF [%PACKAGE%] == [] (
  ECHO  -WARNING - PACKAGE value not retrieved from %ABS_WD%\mainApp\version.h.
  SET PACKAGE=LPub3D
)

rem Verify 1st input flag options
IF NOT [%1]==[] (
  IF NOT "%1"=="x86" (
    IF NOT "%1"=="x86_64" (
      IF NOT "%1"=="-all" (
        IF NOT "%1"=="-help" GOTO :PLATFORM_ERROR
      )
    )
  )
)

rem Parse platform input flags
IF [%1]==[] (
  IF NOT EXIST "%LP3D_QT32_MSVC%" GOTO :LIBRARY_ERROR
  IF NOT EXIST "%LP3D_QT64_MSVC%" GOTO :LIBRARY_ERROR
  SET PLATFORM_ARCH=-all
  GOTO :SET_CONFIGURATION
)

IF /I "%1"=="x86" (
  IF NOT EXIST "%LP3D_QT32_MSVC%" GOTO :LIBRARY_ERROR
  SET PLATFORM_ARCH=x86
  GOTO :SET_CONFIGURATION
)

IF /I "%1"=="x86_64" (
  IF NOT EXIST "%LP3D_QT64_MSVC%" GOTO :LIBRARY_ERROR
  SET PLATFORM_ARCH=x86_64
  GOTO :SET_CONFIGURATION
)

IF /I "%1"=="-all" (
  IF NOT EXIST "%LP3D_QT32_MSVC%" GOTO :LIBRARY_ERROR
  IF NOT EXIST "%LP3D_QT64_MSVC%" GOTO :LIBRARY_ERROR
  SET PLATFORM_ARCH=-all
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
  IF NOT "%2"=="-ins" (
    IF NOT "%2"=="-chk" (
      IF NOT "%2"=="-3rd" (
        IF NOT "%2"=="-ren" GOTO :CONFIGURATION_ERROR
      )
    )
  )
)

rem Verify 3rd input flag options
IF NOT [%3]==[] (
  IF NOT "%3"=="-ins" (
    IF NOT "%3"=="-chk" (
      IF NOT "%3"=="-asl" (
        IF NOT "%3"=="-ldraw" (
          IF NOT "%3"=="-official" (
            IF NOT "%3"=="-unofficial" (
              IF NOT "%3"=="-tente" (
                IF NOT "%3"=="-vexiq" GOTO :CONFIGURATION_ERROR
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
  IF NOT "%4"=="-chk" (
    IF NOT "%4"=="-asl" (
      IF NOT "%3"=="-ldraw" (
        IF NOT "%4"=="-official" (
          IF NOT "%4"=="-unofficial" (
            IF NOT "%4"=="-tente" (
              IF NOT "%4"=="-vexiq" GOTO :CONFIGURATION_ERROR
            )
          )
        )
      )
    )
  )
)

IF [%2]==[] (
  SET INSTALL=0
  GOTO :BUILD
)

IF /I "%2"=="-ins" (
  SET INSTALL=1
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
IF NOT EXIST "%LP3D_WIN_GIT%" (
  SET LP3D_WIN_GIT=
  SET LP3D_WIN_GIT_MSG=Not Found
)

IF /I "%RENDERERS_ONLY%"=="1" (
  ECHO.
  ECHO -Build 3rdparty renderers only - %PACKAGE% not built
  ECHO.
)

rem set application version variables
SET UPDATE_CONFIG_ARGS=%ABS_WD%\mainApp
IF "%LP3D_CONDA_BUILD%" EQU "True" (
  SET UPDATE_CONFIG_ARGS=%UPDATE_CONFIG_ARGS% ParseVersionInfoFile
)
CALL builds\utilities\update-config-files.bat %UPDATE_CONFIG_ARGS%
IF ERRORLEVEL 1 (GOTO :ERROR_END)

rem Display build settings
ECHO.
IF "%BUILD_WORKER%" EQU "True" (
  ECHO   BUILD_HOST.....................[%BUILD_WORKER_HOST%]
  ECHO   BUILD_WORKER_IMAGE.............[%BUILD_WORKER_IMAGE%]
  ECHO   BUILD_WORKER_JOB...............[%BUILD_WORKER_JOB%]
  ECHO   BUILD_WORKER_REF...............[%BUILD_WORKER_REF%]
  ECHO   BUILD_WORKER_OS................[%BUILD_WORKER_OS%]
  ECHO   PROJECT REPOSITORY.............[%BUILD_WORKER_REPO%]
  ECHO   LP3D_WIN_GIT_DIR...............[%LP3D_WIN_GIT_MSG%]
)
IF "%APPVEYOR%" EQU "True" (
  ECHO   BUILD_HOST.....................[APPVEYOR CONTINUOUS INTEGRATION SERVICE]
  ECHO   BUILD_WORKER_IMAGE.............[%APPVEYOR_BUILD_WORKER_IMAGE%]
  ECHO   BUILD_ID.......................[%APPVEYOR_BUILD_ID%]
  ECHO   BUILD_BRANCH...................[%APPVEYOR_REPO_BRANCH%]
  ECHO   PROJECT_NAME...................[%APPVEYOR_PROJECT_NAME%]
  ECHO   REPOSITORY_NAME................[%APPVEYOR_REPO_NAME%]
  ECHO   REPO_PROVIDER..................[%APPVEYOR_REPO_PROVIDER%]
  ECHO   LP3D_WIN_GIT_DIR...............[%LP3D_WIN_GIT_MSG%]
)
ECHO   PACKAGE........................[%PACKAGE%]
ECHO   CONFIGURATION..................[%CONFIGURATION%]
ECHO   COMMIT MESSAGE.................[%LP3D_COMMIT_MSG%]
ECHO   WORKING_DIRECTORY_LPUB3D.......[%ABS_WD%]
ECHO   DISTRIBUTION_DIRECTORY.........[%DIST_DIR%]
ECHO   LDRAW_DIRECTORY................[%LDRAW_DIR%]
ECHO   LDRAW_INSTALL_ROOT.............[%LDRAW_INSTALL_ROOT%]
ECHO   LDRAW_LIBS_ROOT................[%LDRAW_LIBS%]
ECHO   BUILD_OPT......................[%BUILD_OPT%]
ECHO.

rem Perform 3rd party content install
IF /I "%3"=="-ins" (
  SET INSTALL=1
)

rem Perform build check
IF /I "%3"=="-chk" (
  SET CHECK=1
)

rem update all supported libraries when -asl defined
IF NOT [%3]==[] (
  IF NOT "%3"=="-ins" (
    IF NOT "%3"=="-chk" (
      IF NOT "%3"=="-asl" (
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
  IF NOT "%4"=="-chk" (
    IF NOT "%4"=="-asl" (
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

rem Stage Install prior to build check
IF /I %CHECK%==1 (
  SET INSTALL=1
)

rem set debug suffix
IF NOT [%CONFIGURATION%]==[] (
  IF "%CONFIGURATION%"=="release" (
    SET "d="
  )
  IF "%CONFIGURATION%"=="debug" (
    SET "d=d"
  )
)

SET platform_build_start=%time%

rem stop here if only running settings for CreateExePkg.bat
IF /I "%LP3D_INSTALL_PKG_ONLY%" == "1" (
  ECHO.
  ECHO -Package %PACKAGE% x86_64 and x86 distributions...
  SETLOCAL ENABLEDELAYEDEXPANSION
  SET EXE_PRODUCT_DIR=%PACKAGE%-Any-%LP3D_APP_VERSION_LONG%
  SET EXE_TARGET_DIR=builds\windows\%CONFIGURATION%\!EXE_PRODUCT_DIR!
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
IF /I "%PLATFORM_ARCH%"=="-all" (
  GOTO :BUILD_ALL
)

rem If build Win32, set to vs2017 for WinXP compat
CALL :CONFIGURE_VCTOOLS %PLATFORM_ARCH%
rem Configure build arguments and set environment variables
CALL :CONFIGURE_BUILD_ENV
CD /D "%ABS_WD%"
ECHO.
ECHO -Building %PACKAGE% %PLATFORM_ARCH% platform, %CONFIGURATION% configuration...
rem Build 3rd party build from source
IF %BUILD_THIRD%==1 ECHO.
IF %BUILD_THIRD%==1 ECHO -----------------------------------------------------
IF %BUILD_THIRD%==1 (CALL builds\utilities\CreateRenderers.bat %PLATFORM_ARCH%)
IF NOT ERRORLEVEL 0 (GOTO :ERROR_END)
IF %BUILD_THIRD%==1 ECHO -----------------------------------------------------
IF %BUILD_THIRD%==1 ECHO.
rem Display QMake version
qmake -v & ECHO.
rem Configure makefiles
qmake %LPUB3D_CONFIG_ARGS%
rem perform build
nmake.exe %LPUB3D_MAKE_ARGS%
rem Check build status
IF %PLATFORM_ARCH%==x86 (SET EXE=mainApp\32bit_%CONFIGURATION%\%PACKAGE%%d%.exe)
IF %PLATFORM_ARCH%==x86_64 (SET EXE=mainApp\64bit_%CONFIGURATION%\%PACKAGE%%d%.exe)
IF NOT EXIST "%EXE%" (
  ECHO.
  ECHO -ERROR - %EXE% was not successfully built.
  GOTO :ERROR_END
)
rem Package 3rd party install content - this must come before check so check can use staged content for test
IF %INSTALL%==1 CALL :STAGE_INSTALL
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

:BUILD_ALL
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
  IF %INSTALL%==1 (CALL :STAGE_INSTALL)
  CALL :ELAPSED_BUILD_TIME !platform_build_start!
  ECHO.
  ECHO -Elapsed %%P package build time !LP3D_ELAPSED_BUILD_TIME!
  ENDLOCAL
  IF %CHECK%==1 (CALL :BUILD_CHECK %%P) ELSE (CALL :ADD_LDRAW_LIBS_TO_EXTRAS)
)
GOTO :END

:BUILD_RENDERERS
rem Check if build all platforms
IF /I "%PLATFORM_ARCH%"=="-all" (
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
FOR %%P IN ( x86, x86_64 ) DO (
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
    SET LP3D_MSC_VER=1941
    SET LP3D_VCSDKVER=10.0
    SET LP3D_VCTOOLSET=v143
    SET LP3D_VCVARSALL_VER=-vcvars_ver=14.4
  )
) ELSE (
  SET LP3D_VCSDKVER=8.1
  SET LP3D_VCTOOLSET=v141
  SET LP3D_VCVARSALL_VER=-vcvars_ver=14.1
)
ECHO.
ECHO   PLATFORM_ARCHITECTURE..........[%PLATFORM_ARCH%]
ECHO   MSVS_VERSION...................[%LP3D_VSVERSION%]
ECHO   MSVC_QT_VERSION................[%LP3D_QTVCVERSION%]
ECHO   MSVC_SDK_VERSION...............[%LP3D_VCSDKVER%]
ECHO   MSVC_TOOLSET...................[%LP3D_VCTOOLSET%]
IF "%LP3D_CONDA_BUILD%" NEQ "True" (
  IF %PLATFORM_ARCH%==x86 (ECHO   LP3D_QT32_MSVC.................[%LP3D_QT32_MSVC%])
)
IF %PLATFORM_ARCH%==x86_64 (ECHO   LP3D_QT64_MSVC.................[%LP3D_QT64_MSVC%])
ECHO   MSVC_VCVARSALL_VER.............[%LP3D_VCVARSALL_VER%]
ECHO   MSVC_VCVARSALL_DIR.............[%LP3D_VCVARSALL_DIR%]
EXIT /b

:CONFIGURE_BUILD_ENV
CD /D %ABS_WD%
ECHO.
ECHO -Configure LPub3D %PLATFORM_ARCH% build environment...
ECHO.
ECHO -Cleanup any previous LPub3D qmake config files...
FOR /R %%I IN (
  ".qmake.stash"
  "Makefile*"
  "lclib\Makefile*"
  "ldrawini\Makefile*"
  "ldvlib\LDVQt\Makefile*"
  "mainApp\Makefile*"
  "quazip\Makefile*"
  "waitingspinner\Makefile*"
) DO DEL /S /Q "%%~I" >NUL 2>&1
ECHO.
SET "LPUB3D_CONFIG_ARGS=CONFIG+=%CONFIGURATION% CONFIG-=debug_and_release"
IF "%LP3D_CONDA_BUILD%" EQU "True" (
  SET "LPUB3D_CONFIG_ARGS=%LPUB3D_CONFIG_ARGS% CONFIG+=con"
  SET "PATH=%PATH%"
) ELSE (
  SET "LPUB3D_CONFIG_ARGS=%LPUB3D_CONFIG_ARGS% CONFIG+=exe"
  SET "PATH=%LP3D_SYS_DIR%;%LP3D_WIN_GIT%"
)
IF %CHECK% EQU 1 (
REM DEBUG============
  IF %PLATFORM_ARCH% EQU x86_64 (
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
REM DEBUG============
) ELSE (
  ECHO   LP3D_BUILD_CHECK...............[No]
)
IF "%BUILD_WORKER%" EQU "True" (
  SET LPUB3D_CONFIG_ARGS=%LPUB3D_CONFIG_ARGS% CONFIG+=%CONFIG_CI%
)
IF "%APPVEYOR%" EQU "True" (
  SET LPUB3D_CONFIG_ARGS=%LPUB3D_CONFIG_ARGS% CONFIG+=%CONFIG_CI%
  IF "%LP3D_BUILD_PKG%" EQU "yes" (
    ECHO.
    ECHO   LP3D_BUILD_PKG.................[%LP3D_BUILD_PKG%]
  )
)
IF "%BUILD_WORKER%" NEQ "True" (
  IF "%APPVEYOR%" NEQ "True" (
    SET LP3D_DIST_DIR_PATH=%CD%\%DIST_DIR%
  )
)

rem Set vcvars for AppVeyor or local build environments
IF "%LP3D_CONDA_BUILD%" NEQ "True" (
  IF %PLATFORM_ARCH% EQU x86 (
    ECHO.
    IF EXIST "%LP3D_QT32_MSVC%\qtenv2.bat" (
      CALL "%LP3D_QT32_MSVC%\qtenv2.bat"
    ) ELSE (
      SET "PATH=%LP3D_QT32_MSVC%;%PATH%"
    )
    IF EXIST "%LP3D_VCVARSALL_DIR%\vcvars32.bat" (
      CALL "%LP3D_VCVARSALL_DIR%\vcvars32.bat" %LP3D_VCVARSALL_VER%
    ) ELSE (
      ECHO -ERROR - vcvars32.bat not found.
      GOTO :ERROR_END
    )
  ) ELSE (
    ECHO.
    IF EXIST "%LP3D_QT64_MSVC%\qtenv2.bat" (
      CALL "%LP3D_QT64_MSVC%\qtenv2.bat"
    ) ELSE (
      SET "PATH=%LP3D_QT64_MSVC%;%PATH%"
    )
    IF EXIST "%LP3D_VCVARSALL_DIR%\vcvars64.bat" (
      CALL "%LP3D_VCVARSALL_DIR%\vcvars64.bat" %LP3D_VCVARSALL_VER%
    ) ELSE (
      ECHO -ERROR - vcvars64.bat not found.
      GOTO :ERROR_END
    )
  )
)

rem Display MSVC Compiler settings
ECHO.
ECHO -Display _MSC_VER %LP3D_MSC_VER% compiler settings
ECHO.
ECHO.%LP3D_MSC_VER% > %TEMP%\settings.c
cl.exe -Bv -EP %TEMP%\settings.c >NUL

SET LPUB3D_MAKE_ARGS=-c -f Makefile
SET PATH_PREPENDED=True

ECHO.
ECHO   LPUB3D_MAKE_ARGS...............[%LPUB3D_MAKE_ARGS%]
ECHO   LPUB3D_CONFIG_ARGS.............[%LPUB3D_CONFIG_ARGS%]
SETLOCAL ENABLEDELAYEDEXPANSION
ECHO(  PATH_PREPEND...................[!PATH!]
  ENDLOCAL
)
EXIT /b

:BUILD_CHECK
ECHO.
REM DEBUG============
IF %1 EQU x86_64 (
  IF "%BUILD_WORKER%" EQU "True" (
    IF "%LP3D_CONDA_BUILD%" EQU "True" (
      ECHO -%PACKAGE% Build Check...
    ) ELSE (
      ECHO -%PACKAGE% Build Check disabled for %PLATFORM_ARCH% %BUILD_WORKER_ID% build
      EXIT /b
    )
  ) ELSE (
    ECHO -%PACKAGE% Build Check...
  )
) ELSE (
  ECHO -%PACKAGE% Build Check...
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
CALL :REQUEST_LDRAW_DIR
rem run build checks
CALL builds\check\build_checks.bat
EXIT /b

:STAGE_INSTALL
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
IF  "%PKG_TARGET_DIR%" EQU "" (
  SET "PKG_TARGET_DIR=%BUILD_WORKSPACE%\builds\windows\%CONFIGURATION%\%PACKAGE%-Any-%LP3D_APP_VERSION_LONG%\%PACKAGE%_%PLATFORM_ARCH%"
)
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

:REQUEST_LDRAW_DIR
ECHO.
ECHO -Request LDraw archive libraries download...

CALL :DOWNLOAD_LDRAW_LIBS

ECHO.
ECHO -Check for LDraw LEGO disk library...
"%LP3D_7ZIP_WIN64%" > %TEMP%\output.tmp 2>&1
FOR /f "usebackq eol= delims=" %%a IN (%TEMP%\output.tmp) DO (
  ECHO.%%a | findstr /C:"7-Zip">NUL && (
    SET LP3D_VALID_7ZIP=1
    ECHO.
    ECHO -7zip exectutable found at "%LP3D_7ZIP_WIN64%"
  ) || (
    ECHO.
    ECHO [WARNING] Could not find 7zip executable at %LP3D_7ZIP_WIN64%.
  )
  GOTO :END_7ZIP_LOOP
)
:END_7ZIP_LOOP
IF NOT EXIST "%LDRAW_DIR%\parts" (
  ECHO.
  ECHO -LDraw directory %LDRAW_DIR% does not exist - creating...
  REM SET CHECK=0
  IF NOT EXIST "%LDRAW_INSTALL_ROOT%\%OfficialCONTENT%" (
    IF EXIST "%LDRAW_LIBS%\%OfficialCONTENT%" (
      COPY /V /Y "%LDRAW_LIBS%\%OfficialCONTENT%" "%LDRAW_INSTALL_ROOT%\" /A | findstr /i /v /r /c:"copied\>"
    ) ELSE (
      ECHO.
      ECHO -[WARNING] Could not find %LDRAW_LIBS%\%OfficialCONTENT%.
      SET CHECK=0
    )
  )
  IF %LP3D_VALID_7ZIP% == 1 (
    ECHO.
    ECHO -Extracting %OfficialCONTENT%...
    ECHO.
    "%LP3D_7ZIP_WIN64%" x -o"%LDRAW_INSTALL_ROOT%\" "%LDRAW_INSTALL_ROOT%\%OfficialCONTENT%" | findstr /i /r /c:"^Extracting\>" /c:"^Everything\>"
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
SET WebNAME=https://library.ldraw.org/library/updates/%OfficialCONTENT%

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
SET WebCONTENT="%OutputPATH%\%LPub3DCONTENT%"
SET WebNAME=https://github.com/trevorsandy/lpub3d_libs/releases/download/v1.0.1/%LPub3DCONTENT%

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
CALL :USAGE
ECHO.
ECHO -01. (FLAG ERROR) Platform or usage flag is invalid. Use x86, x86_64 or -all [%~nx0 %*].
ECHO      See Usage.
GOTO :ERROR_END

:CONFIGURATION_ERROR
ECHO.
CALL :USAGE
ECHO.
ECHO -02. (FLAG ERROR) Configuration flag is invalid [%~nx0 %*].
ECHO      See Usage.
GOTO :ERROR_END

:COMMAND_ERROR
ECHO.
CALL :USAGE
ECHO.
ECHO -03. (COMMAND ERROR) Invalid command string [%~nx0 %*].
ECHO      See Usage.
GOTO :ERROR_END

:LIBRARY_ERROR
ECHO.
CALL :USAGE
ECHO.
ECHO -04. (LIBRARY ERROR) Qt MSVC library [%LP3D_QT32_MSVC%] or [%LP3D_QT64_MSVC%] required for command not found [%~nx0 %*].
ECHO      See Usage.
GOTO :ERROR_END

:USAGE
ECHO ----------------------------------------------------------------
ECHO.
ECHO %PACKAGE% Windows auto build script.
ECHO.
ECHO NOTE: To successfully run all options of this script locally,
ECHO you must have both Win32 and Win64 Qt for MSVC. Qt for MSVC 2015
ECHO supports both 32 and 64bit so this version is a convenient choice.
ECHO.
ECHO ----------------------------------------------------------------
ECHO Usage:
ECHO  build [ -help]
ECHO  build [ x86 ^| x86_64 ^| -all ] [ -chk ^| -ins ^| -3rd ^| -ren ] [ -chk ^| -ins ^| -asl  ^| -ldraw  ^| -official ^| -unofficial ^| -tente ^| -vexiq ] [ -chk ^| -asl ^| -ldraw  ^| -official ^| -unofficial ^| -tente ^| -vexiq ]
ECHO.
ECHO ----------------------------------------------------------------
ECHO Build 64bit, Release and perform build check
ECHO build x86_64 -chk
ECHO.
ECHO Build 64bit, Release and perform install and build check
ECHO build x86_64 -ins -chk
ECHO.
ECHO Build 32bit, Release and perform build check
ECHO build x86 -chk
ECHO.
ECHO Build 64bit and 32bit, 3rdParty renderers
ECHO build -all -ren
ECHO.
ECHO Build 64bit and 32bit, Release and perform build check
ECHO build -all -ren
ECHO.
ECHO Build 64bit and 32bit, Release, perform install and build check
ECHO build -all -ins -chk
ECHO.
ECHO Build 64bit and 32bit, Release, build 3rd party renderers, perform install and build check
ECHO build -all -ins -chk
ECHO.
ECHO Commands:
ECHO ----------------------------------------------------------------
ECHO [PowerShell]: cmd.exe /c builds\windows\AutoBuild.cmd [Flags]
ECHO [cmd.exe]   : builds\windows\AutoBuild.cmd [Flags]
ECHO.
ECHO Flags:
ECHO ----------------------------------------------------------------
ECHO ^| Flag     ^| Pos  ^| Type              ^| Description
ECHO ----------------------------------------------------------------
ECHO  -help.......1........Useage flag         [Default=Off] Display useage.
ECHO  x86.........1........Platform flag       [Default=Off] Build 32bit platform.
ECHO  x86_64......1........Platform flag       [Default=Off] Build 64bit platform.
ECHO  -all........1........Configuraiton flag  [Default=On ] Build both 32bit and 64bit PLATFORM_ARCHs - Requries Qt libraries for both PLATFORMs.
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
ECHO Flags are case sensitive, use lowere case.
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
ECHO -%PACKAGE% %~nx0 FAILED.
ECHO -%~nx0 will terminate!
CALL :END_STATUS
EXIT /b 3

:END
ECHO.
ECHO -%PACKAGE% v%LP3D_VERSION% %~nx0 finished.
CALL :END_STATUS
EXIT /b
