@ECHO OFF &SETLOCAL
Title Setup and launch LPub3D auto build script
rem --
rem  Trevor SANDY <trevor.sandy@gmail.com>
rem  Last Update: September 12, 2025
rem  Copyright (C) 2021 - 2025 by Trevor SANDY
rem --
rem --
rem This script is distributed in the hope that it will be useful,
rem but WITHOUT ANY WARRANTY; without even the implied warranty of
rem MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
rem
rem This script is called from .github/workflows/prod_ci_build.yml
rem
rem Run command:
rem .\builds\utilities\ci\github\windows-build.bat

IF "%LP3D_LOCAL_CI_BUILD%" == "1" (
  ECHO.
  ECHO ======================================================
  ECHO   -Start %~nx0...
  ECHO ------------------------------------------------------
)

SET GITHUB=True
SET GITHUB_CONFIG=release
SET BUILD_OPT=default
SET UPDATE_LDRAW_LIBS=False
SET LP3D_VSVERSION=2022
SET LP3D_QT32VCVERSION=2019
SET LP3D_QT64VCVERSION=2022
SET LP3D_CREATE_EXE_PKG=True
SET LP3D_PUBLISH_RENDERERS=False
SET LP3D_3RD_DIST_DIR=lpub3d_windows_3rdparty
SET LP3D_DIST_DIR_PATH=%LP3D_3RD_PARTY_PATH%\windows
SET LP3D_LDRAW_DIR_PATH=%LP3D_3RD_PARTY_PATH%\ldraw
SET LP3D_DOWNLOADS_PATH=%LP3D_BUILDPKG_PATH%\Downloads
SET LP3D_UPDATES_PATH=%LP3D_BUILDPKG_PATH%\Updates
SET LP3D_LOG_PATH=%LP3D_BUILDPKG_PATH%

IF [%LP3D_NO_3RD_RENDER%] EQU [] SET LP3D_3RD_RENDERS=-3rd
IF [%LP3D_NO_INSTALL%] EQU [] SET LP3D_INSTALL=-ins
IF [%LP3D_NO_CHECK%] EQU [] SET LP3D_CHECK=-chk

CD %GITHUB_WORKSPACE%

IF "%LP3D_LOCAL_CI_BUILD%" == "1" (
  GOTO :CONTINUE
)

rem Check if build is on stale commit
SET GH_AUTH=Authorization: Bearer %GITHUB_TOKEN%
IF NOT EXIST "repo.txt" (
  curl -s -H "%GH_AUTH%" https://api.github.com/repos/%GITHUB_REPOSITORY%/commits/master -o repo.txt
)
IF NOT EXIST "repo.txt" (
  ECHO.
  ECHO "WARNING - Failed to get repository response."
  GOTO :CONTINUE
)
FOR /F "delims=" %%q IN ('TYPE repo.txt ^| jq -r .sha') DO SET "LP3D_REMOTE=%%q"
FOR /F "delims=" %%c IN ('git rev-parse HEAD') DO SET "LP3D_LOCAL=%%c"
IF "%LP3D_REMOTE%" NEQ "%LP3D_LOCAL%" (
  ECHO.
  ECHO "WARNING - Build no longer current. Rmote: '%LP3D_REMOTE%', Local: '%LP3D_LOCAL%' - aborting build."
  IF EXIST "repo.txt" ( ECHO "Repo response:" && TYPE repo.txt )
  GOTO :END
)

ECHO.%GITHUB_REF% | FIND /I "refs/tags/" >NUL && (
  IF "%GITHUB_REF_NAME:~0,1%" EQU "v" (
    SET LP3D_PUBLISH_RENDERERS=True
    CALL :SET_BUILD_ALL
    ECHO.
    ECHO - New version tag %GITHUB_REF_NAME% confirmed.
    ECHO - Publish Renderers.
  )
)

:CONTINUE

ECHO.%LP3D_COMMIT_MSG% | FIND /I "QUICK_BUILD" >NUL && (
  ECHO.
  ECHO - NOTICE - Quick build detected, %~nx0 will not continue.
  GOTO :END
)

ECHO.%LP3D_COMMIT_MSG% | FIND /I "UPDATE_LDRAW" >NUL && (
  SET UPDATE_LDRAW_LIBS=True
)

ECHO.%LP3D_COMMIT_MSG% | FIND /I "RELEASE_BUILD" >NUL && (
  ECHO.
  ECHO - Build and deploy current revision detected.
  CALL :SET_BUILD_ALL
)

IF NOT EXIST "%LP3D_DIST_DIR_PATH%" (
  MKDIR "%LP3D_DIST_DIR_PATH%" >NUL 2>&1
  IF NOT EXIST "%LP3D_DIST_DIR_PATH%" (
    ECHO.
    ECHO - ERROR - Create %LP3D_DIST_DIR_PATH% failed
    GOTO :ERROR_END
  )
)

IF NOT EXIST "%LP3D_LDRAW_DIR_PATH%" (
  MKDIR "%LP3D_LDRAW_DIR_PATH%" >NUL 2>&1
  IF NOT EXIST "%LP3D_LDRAW_DIR_PATH%" (
    ECHO.
    ECHO - WARNING - Create %LP3D_LDRAW_DIR_PATH% failed
  )
)

IF NOT EXIST "%LP3D_UPDATES_PATH%" (
  MKDIR "%LP3D_UPDATES_PATH%" >NUL 2>&1
  IF NOT EXIST "%LP3D_UPDATES_PATH%" (
    ECHO.
    ECHO - ERROR - Create %LP3D_UPDATES_PATH% failed
    GOTO :ERROR_END
  )
)

IF NOT EXIST "%LP3D_DOWNLOADS_PATH%" (
  MKDIR "%LP3D_DOWNLOADS_PATH%" >NUL 2>&1
  IF NOT EXIST "%LP3D_DOWNLOADS_PATH%" (
    ECHO.
    ECHO - ERROR - Create %LP3D_DOWNLOADS_PATH% failed
    GOTO :ERROR_END
  )
)

IF NOT EXIST "%USERPROFILE%\LDraw" (
  IF EXIST "%LP3D_LDRAW_DIR_PATH%" (
    MKLINK /d %USERPROFILE%\LDraw %LP3D_LDRAW_DIR_PATH% 2>&1
    SET LDRAWDIR=%USERPROFILE%\LDraw
  ) ELSE (
    ECHO.
    ECHO - WARNING - %LP3D_LDRAW_DIR_PATH% path not defined
  )
) ELSE (
  SET LDRAWDIR=%USERPROFILE%\LDraw
)

PUSHD %LP3D_BUILD_BASE%
IF NOT EXIST "%LP3D_3RD_DIST_DIR%" (
  IF EXIST "%LP3D_DIST_DIR_PATH%" (
    MKLINK /d %LP3D_3RD_DIST_DIR% %LP3D_DIST_DIR_PATH% 2>&1
  ) ELSE (
    ECHO.
    ECHO - ERROR - %LP3D_DIST_DIR_PATH% path not defined
    GOTO :ERROR_END
  )
)
POPD

SET LP3D_LDGLITE=%LP3D_DIST_DIR_PATH%\ldglite-1.3
SET LP3D_LDVIEW=%LP3D_DIST_DIR_PATH%\ldview-4.6
SET LP3D_POVRAY=%LP3D_DIST_DIR_PATH%\lpub3d_trace_cui-3.8
ECHO.%LP3D_COMMIT_MSG% | FIND /I "ALL_RENDERERS" >NUL && (
  CALL :SET_BUILD_ALL_RENDERERS
)
ECHO.%LP3D_COMMIT_MSG% | FIND /I "BUILD_LDGLITE" >NUL && (
  ECHO - 'Build LDGLite' detected.
  IF EXIST "%LP3D_LDGLITE%" ( DEL /S /Q "%LP3D_LDGLITE%" >NUL 2>&1 )
  IF NOT EXIST "%LP3D_LDGLITE%" ( ECHO - Cached %LP3D_LDGLITE% deleted. )
)
ECHO.%LP3D_COMMIT_MSG% | FIND /I "BUILD_LDVIEW" >NUL && (
  ECHO - 'Build LDView' detected.
  IF EXIST "%LP3D_LDVIEW%" ( DEL /S /Q "%LP3D_LDVIEW%" >NUL 2>&1 )
  IF NOT EXIST "%LP3D_LDVIEW%" ( ECHO - Cached %LP3D_LDVIEW% deleted. )
)
ECHO.%LP3D_COMMIT_MSG% | FIND /I "BUILD_POVRAY" >NUL && (
  ECHO - 'Build POVRay' detected.
  IF EXIST "%LP3D_POVRAY%" ( DEL /S /Q "%LP3D_POVRAY%" >NUL 2>&1 )
  IF NOT EXIST "%LP3D_POVRAY%" ( ECHO - Cached %LP3D_POVRAY% deleted. )
)
ECHO.%LP3D_COMMIT_MSG% | FIND /I "PUBLISH_RENDERERS" >NUL && (
  ECHO - 'Publish Renderers' detected.
  SET LP3D_PUBLISH_RENDERERS=True
)

ECHO.%GITHUB_EVENT_NAME% | FIND /I "PUSH" >NUL && (
  ECHO.%LP3D_COMMIT_MSG% | FIND /V /I "BUILD_ALL" >NUL && (
    ECHO.
    IF "%LP3D_BUILD_ARCH%" EQU "" (
      ECHO - Build option verify ^(x86 architecture^) detected.
      SET LP3D_BUILD_ARCH=x86
    ) ELSE (
      ECHO - Build option verify ^(%LP3D_BUILD_ARCH% architecture^) detected.
    )
    SET LP3D_CREATE_EXE_PKG=False
    SET BUILD_OPT=verify
  )
)

SET LP3D_BUILD_COMMAND=%LP3D_BUILD_ARCH% %LP3D_3RD_RENDERS% %LP3D_INSTALL% %LP3D_CHECK%

ECHO.
ECHO - Create Distributions: %LP3D_CREATE_EXE_PKG%
ECHO - Commit Message: %LP3D_COMMIT_MSG%
ECHO - Build Command: builds\windows\AutoBuild.bat %LP3D_BUILD_COMMAND%

rem reset ErrorLevel to 0 from 1 for unsuccessful FIND
(CALL )

rem if we are cross compiling an ARM64 build
IF /I "%LP3D_BUILD_ARCH%" NEQ "arm64" (GOTO :AUTO_BUILD)
IF /I "%PROCESSOR_ARCHITECTURE%" NEQ "amd64" (GOTO :AUTO_BUILD)
SET LP3D_QMAKE_VER=0
SET LP3D_QMAKE_RUN=1
SET LP3D_QMAKE_CLEAN=1
SET LP3D_ARM64_QMAKE=1
SET LP3D_BUILD_COMMAND=%LP3D_BUILD_ARCH% %LP3D_INSTALL% %LP3D_CHECK%
CALL builds\windows\AutoBuild.bat %LP3D_BUILD_ARCH% %LP3D_3RD_RENDERS% || GOTO :ERROR_END

:ARM64_BUILD
IF "%ERRORLEVEL%" NEQ "0" (GOTO :ERROR_END)
SET LP3D_QMAKE_RUN=0
SET LP3D_QMAKE_CLEAN=0
SET LP3D_ARM64_BUILD=1

:AUTO_BUILD
CALL builds\windows\AutoBuild.bat %LP3D_BUILD_COMMAND% || GOTO :ERROR_END
IF "%ERRORLEVEL%" NEQ "0" (GOTO :ERROR_END)

IF "%LP3D_CREATE_EXE_PKG%" EQU "True" (
  CALL builds\windows\CreateExePkg.bat || GOTO :ERROR_END
  IF "%ERRORLEVEL%" NEQ "0" (GOTO :ERROR_END)
  CALL :GENERATE_HASH_FILES
)

GOTO :END

:GENERATE_HASH_FILES
SET BUILD_EXIST=0
CD %GITHUB_WORKSPACE%
IF EXIST "%LP3D_DOWNLOADS_PATH%\*.exe" ( SET BUILD_EXIST=1 )
IF EXIST "%LP3D_DOWNLOADS_PATH%\*.zip" ( SET BUILD_EXIST=1 )
IF %BUILD_EXIST% == 0 ( EXIT /b )
ECHO.
ECHO - Generate sha512 hash files in %CD%\%LP3D_DOWNLOADS_PATH%...
PUSHD %LP3D_DOWNLOADS_PATH%
SET gen_hash=Gen_Hash.sh
SET genHash=%gen_hash% ECHO
>%genHash% #!/bin/bash
>>%genHash% # redirect stdout/stderr to a log file
>>%genHash% exec ^>%gen_hash%.log 2^>^&1
>>%genHash% # confirm sha512 is installed
>>%genHash% sha512=$^(which sha512sum^)
>>%genHash% if test -z "$sha512"; then
>>%genHash%   echo "  -ERROR - sha512sum was not found."
>>%genHash% else
>>%genHash%   echo "  -$sha512 found."
>>%genHash%   # generate package hash files
>>%genHash%   LP3D_WIN_ASSETS=$^(find . -type f -not -path "./*_Assets.zip*"^)
>>%genHash%   for LP3D_WIN_ASSET in ${LP3D_WIN_ASSETS}; do
>>%genHash%     LP3D_WIN_ASSET_EXT=".${LP3D_WIN_ASSET##*.}"
>>%genHash%     case "${LP3D_WIN_ASSET_EXT}" in
>>%genHash%       ".exe"^|".zip"^)
>>%genHash%       [[ -f "${LP3D_WIN_ASSET}.sha512" ]] ^&^& rm -f "${LP3D_WIN_ASSET}.sha512" ^|^| :
>>%genHash%       $sha512 "${LP3D_WIN_ASSET}" ^> "${LP3D_WIN_ASSET}.sha512" ^&^& \
>>%genHash%       echo "  -Created hash file ${LP3D_WIN_ASSET}.sha512" ^|^| \
>>%genHash%       echo "  -ERROR - Failed to create hash file ${LP3D_WIN_ASSET}.sha512"
>>%genHash%       ;;
>>%genHash%     esac
>>%genHash%   done
>>%genHash% fi
SET run_cmd=Gen_Hash.bat
SET runCmd=%run_cmd% ECHO
>%runCmd% @ECHO OFF
>>%runCmd% bash -lc "sed -i -e 's/\r$//' %gen_hash%"
>>%runCmd% bash -lc "chmod a+x %gen_hash%; ./%gen_hash%"
START /w /b "Gen Hash" CMD /c %run_cmd%
IF EXIST "%gen_hash%.log" ( TYPE %gen_hash%.log )
DEL /Q %run_cmd% %gen_hash% %gen_hash%.log
POPD
EXIT /b

:SET_BUILD_ALL
ECHO.%LP3D_COMMIT_MSG% | FIND /V /I "BUILD_ALL" >NUL && (
  SET LP3D_COMMIT_MSG=%LP3D_COMMIT_MSG% BUILD_ALL
)
IF "%LP3D_BUILD_ARCH%" EQU "" SET LP3D_BUILD_ARCH=-all_amd
IF "%LP3D_BUILD_ARCH%" NEQ "-all_amd" (
  IF "%LP3D_BUILD_ARCH%" NEQ "arm64" (
    SET LP3D_CREATE_EXE_PKG=False
  )
)
EXIT /b

:SET_BUILD_ALL_RENDERERS
ECHO.
ECHO - 'Build LDGLite, LDView and POV-Ray' detected.
SET LP3D_COMMIT_MSG=%LP3D_COMMIT_MSG% BUILD_LDGLITE BUILD_LDVIEW BUILD_POVRAY
EXIT /b

:ERROR_END
ECHO.
ECHO - %~nx0 FAILED with return code %ERRORLEVEL%.
ECHO - %~nx0 will terminate!
ENDLOCAL
EXIT /b 3

:END
ECHO.
ECHO - %~nx0 finished.
ENDLOCAL
EXIT /b
