@ECHO OFF &SETLOCAL
Title Setup and launch LPub3D auto build script
rem --
rem  Trevor SANDY <trevor.sandy@gmail.com>
rem  Last Update: September 12, 2024
rem  Copyright (C) 2021 - 2024 by Trevor SANDY
rem --
rem --
rem This script is distributed in the hope that it will be useful,
rem but WITHOUT ANY WARRANTY; without even the implied warranty of
rem MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
rem
rem This script is called from .github/workflows/build.yml
rem
rem Run command:
rem .\builds\utilities\ci\github\windows-build.bat

SET GITHUB=True
SET GITHUB_CONFIG=release
SET BUILD_ARCH=-all
SET BUILD_OPT=default
SET UPDATE_LDRAW_LIBS=False
SET LP3D_VSVERSION=2022
SET LP3D_QTVCVERSION=2019
SET LP3D_3RD_DIST_DIR=lpub3d_windows_3rdparty
SET LP3D_DIST_DIR_PATH=%LP3D_3RD_PARTY_PATH%\windows
SET LP3D_LDRAW_DIR_PATH=%LP3D_3RD_PARTY_PATH%\ldraw
SET LP3D_DOWNLOADS_PATH=%LP3D_BUILDPKG_PATH%\Downloads
SET LP3D_UPDATES_PATH=%LP3D_BUILDPKG_PATH%\Updates

CD %GITHUB_WORKSPACE%

ECHO.%LP3D_COMMIT_MSG% | FIND /I "QUICK_BUILD" >NUL && (
  ECHO.
  ECHO - NOTICE - Quick build detected, %~nx0 will not continue.
  GOTO :END
)

ECHO.%LP3D_COMMIT_MSG% | FIND /I "UPDATE_LDRAW" >NUL && (
  SET UPDATE_LDRAW_LIBS=True
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
  ) ELSE (
    ECHO.
    ECHO - WARNING - %LP3D_LDRAW_DIR_PATH% path not defined
  )
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
SET LP3D_LDVIEW=%LP3D_DIST_DIR_PATH%\ldview-4.5
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

ECHO.%GITHUB_REF% | FIND /I "refs/tags/" >NUL && (
  IF "%GITHUB_REF_NAME:~0,1%" EQU "v" (
    CALL :SET_BUILD_ALL
    ECHO - New version tag %GITHUB_REF_NAME% confirmed.
  )
)

ECHO.%LP3D_COMMIT_MSG% | FIND /I "RELEASE_BUILD" >NUL && (
  ECHO - Build and deploy current revision detected.
  CALL :SET_BUILD_ALL
)

ECHO.%GITHUB_EVENT_NAME% | FIND /I "PUSH" >NUL && (
  ECHO.%LP3D_COMMIT_MSG% | FIND /V /I "BUILD_ALL" >NUL && (
    ECHO - Build option verify ^(x86 architecture only^) detected.
    SET BUILD_ARCH=x86
    SET BUILD_OPT=verify
  )
)

SET LP3D_BUILD_COMMAND=%BUILD_ARCH% -3rd -ins -chk

ECHO - Commit message: %LP3D_COMMIT_MSG%
ECHO - Build command: builds\windows\AutoBuild.bat %LP3D_BUILD_COMMAND%

CALL builds\windows\AutoBuild.bat %LP3D_BUILD_COMMAND% 2>&1 || GOTO :ERROR_END
IF "%ERRORLEVEL%" NEQ "0" (GOTO :ERROR_END)

IF "%BUILD_ARCH%" EQU "-all" (
  CALL builds\windows\CreateExePkg.bat 2>&1 || GOTO :ERROR_END
  IF "%ERRORLEVEL%" NEQ "0" (GOTO :ERROR_END)
)

CALL :GENERATE_HASH_FILES

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
SET BUILD_ARCH=-all
EXIT /b

:SET_BUILD_ALL_RENDERERS
ECHO.
ECHO - 'Build LDGLite, LDView and POV-Ray' detected.
SET LP3D_COMMIT_MSG=%LP3D_COMMIT_MSG% BUILD_LDGLITE BUILD_LDVIEW BUILD_POVRAY
EXIT /b

:ERROR_END
ECHO.
ECHO - %~nx0 FAILED.
ECHO - %~nx0 will terminate!
ENDLOCAL
EXIT /b 3

:END
ECHO.
ECHO - %~nx0 finished.
ENDLOCAL
EXIT /b
