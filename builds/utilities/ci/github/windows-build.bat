@ECHO OFF &SETLOCAL
Title Setup and launch LPub3D auto build script
rem --
rem  Trevor SANDY <trevor.sandy@gmail.com>
rem  Last Update: May 22, 2022
rem  Copyright (C) 2021 - 2022 by Trevor SANDY
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

SET LP3D_ME=%~nx0
SET ABS_WD=%GITHUB_WORKSPACE%
SET BUILD_ARCH=-all

SET GITHUB=True
SET GITHUB_RUNNER_IMAGE=Visual Studio 2019
SET GITHUB_CONFIG=release
SET UPDATE_LDRAW_LIBS=False
SET LP3D_PACKAGE=LPub3D
SET LP3D_3RD_DIST_DIR=lpub3d_windows_3rdparty
SET LP3D_DIST_DIR_PATH=%LP3D_3RD_PARTY_PATH%\windows
SET LP3D_LDRAW_DIR_PATH=%LP3D_3RD_PARTY_PATH%\ldraw
SET LP3D_DOWNLOADS_PATH=%LP3D_BUILDPKG_PATH%\Downloads
SET LP3D_UPDATES_PATH=%LP3D_BUILDPKG_PATH%\Updates

CD %GITHUB_WORKSPACE%

ECHO.%LP3D_COMMIT_MSG% | FIND /I "QUICK_BUILD" >NUL && (
  ECHO -NOTICE - Quick build detected, %~nx0 will not continue.
  GOTO :END
)

ECHO.%LP3D_COMMIT_MSG% | FIND /I "UPDATE_LDRAW" >NUL && (
  SET UPDATE_LDRAW_LIBS=True
)

IF NOT EXIST "%LP3D_DIST_DIR_PATH%" (
  MKDIR "%LP3D_DIST_DIR_PATH%" >NUL 2>&1
  IF NOT EXIST "%LP3D_DIST_DIR_PATH%" (
    ECHO - ERROR - Create %LP3D_DIST_DIR_PATH% failed
    GOTO :ERROR_END
  )
)

IF NOT EXIST "%LP3D_LDRAW_DIR_PATH%" (
  MKDIR "%LP3D_LDRAW_DIR_PATH%" >NUL 2>&1
  IF NOT EXIST "%LP3D_LDRAW_DIR_PATH%" (
    ECHO -WARNING - Create %LP3D_LDRAW_DIR_PATH% failed
  )
)

IF NOT EXIST "%LP3D_UPDATES_PATH%" (
  MKDIR "%LP3D_UPDATES_PATH%" >NUL 2>&1
  IF NOT EXIST "%LP3D_UPDATES_PATH%" (
    ECHO - ERROR - Create %LP3D_UPDATES_PATH% failed
    GOTO :ERROR_END
  )
)

IF NOT EXIST "%LP3D_DOWNLOADS_PATH%" (
  MKDIR "%LP3D_DOWNLOADS_PATH%" >NUL 2>&1
  IF NOT EXIST "%LP3D_DOWNLOADS_PATH%" (
    ECHO - ERROR - Create %LP3D_DOWNLOADS_PATH% failed
    GOTO :ERROR_END
  )
)

IF NOT EXIST "%USERPROFILE%\LDraw" (
  IF EXIST "%LP3D_LDRAW_DIR_PATH%" (
    MKLINK /d %USERPROFILE%\LDraw %LP3D_LDRAW_DIR_PATH% 2>&1
  ) ELSE (
    ECHO.
    ECHO -WARNING - %LP3D_LDRAW_DIR_PATH% path not defined
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

SET INVALID_TAG=FALSE
SET IS_PUB_TAG=FALSE
ECHO.%GITHUB_REF% | FIND /I "refs/tags/" >NUL && (
  ECHO -Commit tag %GITHUB_REF_NAME% detected.
  SET "VER_TAG=%GITHUB_REF_NAME%"
  SETLOCAL ENABLEDELAYEDEXPANSION
  SET "VER_PREFIX=!VER_TAG:~0,1!"
  IF "!VER_PREFIX!" EQU "v" (
    SET "VER_TAG=!VER_TAG:.= !"
    SET "VER_TAG=!VER_TAG:v=!"
    FOR /F "tokens=1" %%i IN ("!VER_TAG!") DO SET VER_MAJOR=%%i
    FOR /F "tokens=2" %%i IN ("!VER_TAG!") DO SET VER_MINOR=%%i
    FOR /F "tokens=3" %%i IN ("!VER_TAG!") DO SET VER_PATCH=%%i
    CALL :IS_VALID_NUMBER !VER_MAJOR!
    CALL :IS_VALID_NUMBER !VER_MINOR!
    CALL :IS_VALID_NUMBER !VER_PATCH!
  )
  IF !IS_PUB_TAG! EQU TRUE (
    CALL :SET_COMMIT_MSG
    ECHO -Publish tag %GITHUB_REF_NAME% confirmed. 
    ECHO -Commit message: !LP3D_COMMIT_MSG!
  )
  SETLOCAL
)

SET LP3D_LDGLITE=%LP3D_DIST_DIR_PATH%\ldglite-1.3
SET LP3D_LDVIEW=%LP3D_DIST_DIR_PATH%\ldview-4.4
SET LP3D_POVRAY=%LP3D_DIST_DIR_PATH%\lpub3d_trace_cui-3.8
ECHO.%LP3D_COMMIT_MSG% | FIND /I "ALL_RENDERERS" >NUL && (
  CALL :SET_BUILD_ALL_RENDERERS
)
ECHO.%LP3D_COMMIT_MSG% | FIND /I "BUILD_LDGLITE" >NUL && (
  ECHO -'Build LDGLite' detected.
  IF EXIST "%LP3D_LDGLITE%" ( DEL /S /Q "%LP3D_LDGLITE%" >NUL 2>&1 )
  IF NOT EXIST "%LP3D_LDGLITE%" ( ECHO -Cached %LP3D_LDGLITE% deleted. )
)
ECHO.%LP3D_COMMIT_MSG% | FIND /I "BUILD_LDVIEW" >NUL && (
  ECHO -'Build LDView' detected.
  IF EXIST "%LP3D_LDVIEW%" ( DEL /S /Q "%LP3D_LDVIEW%" >NUL 2>&1 )
  IF NOT EXIST "%LP3D_LDVIEW%" ( ECHO -Cached %LP3D_LDVIEW% deleted. )
)
ECHO.%LP3D_COMMIT_MSG% | FIND /I "BUILD_POVRAY" >NUL && (
  ECHO -'Build POVRay' detected.
  IF EXIST "%LP3D_POVRAY%" ( DEL /S /Q "%LP3D_POVRAY%" >NUL 2>&1 )
  IF NOT EXIST "%LP3D_POVRAY%" ( ECHO -Cached %LP3D_POVRAY% deleted. )
)

ECHO.%GITHUB_EVENT_NAME% | FIND /I "PUSH" >NUL && (
  ECHO.%LP3D_COMMIT_MSG% | FIND /V /I "BUILD_ALL" >NUL && (
    ECHO -Build option verify ^(x86 architecture^) only detected.
    SET BUILD_ARCH=x86
  )
)

CALL builds\windows\AutoBuild.bat %BUILD_ARCH% -3rd -ins -chk 2>&1 || GOTO :ERROR_END

IF "%BUILD_ARCH%" EQU "-all" (
  CALL builds\windows\CreateExePkg.bat 2>&1 || GOTO :ERROR_END
) ELSE (
  GOTO :END
)

PUSHD %LP3D_DOWNLOADS_PATH%
SET gen_hash=gen_hash.sh
SET f=%gen_hash% ECHO
>%f% #!/bin/bash
>>%f% # generate package hash files
>>%f% LP3D_PKGS=$^(find . -type f^)
>>%f% echo
>>%f% for LP3D_PKG in ${LP3D_PKGS}; do
>>%f%   LP3D_PKG_EXT=".${LP3D_PKG##*.}"
>>%f%   case "${LP3D_PKG_EXT}" in
>>%f%     ".exe"^|".zip"^)
>>%f%     [[ -f "${LP3D_PKG}.sha512" ]] ^&^& rm -f "${LP3D_PKG}.sha512" ^|^| :
>>%f%     [[ ! "${LP3D_PKG}" == *"-debug"* ]] ^&^& \
>>%f%     sha512sum "${LP3D_PKG}" ^> "${LP3D_PKG}.sha512" ^&^& \
>>%f%     echo "- Created hash file ${LP3D_PKG}.sha512" ^|^| \
>>%f%     echo "- WARNING - Failed to create hash file ${LP3D_PKG}.sha512"
>>%f%     ;;
>>%f%   esac
>>%f% done
bash -lc "sed -i -e 's/\r$//' gen_hash.sh"
bash -lc "chmod a+x gen_hash.sh; ./gen_hash.sh"
DEL /Q gen_hash.sh
POPD
GOTO :END

:IS_VALID_NUMBER
IF %INVALID_TAG% EQU TRUE EXIT /b
IF %1 EQU +%1 (
  SET IS_PUB_TAG=TRUE
  EXIT /b
)
SET IS_PUB_TAG=FALSE
SET INVALID_TAG=TRUE
ECHO -Version number '%1' is invalid.
EXIT /b

:SET_COMMIT_MSG
SET LP3D_COMMIT_MSG=%LP3D_COMMIT_MSG% BUILD_ALL
EXIT /b

:SET_BUILD_ALL_RENDERERS
ECHO.
ECHO -'Build LDGLite, LDView and POV-Ray' detected.
SET LP3D_COMMIT_MSG=%LP3D_COMMIT_MSG% BUILD_LDGLITE BUILD_LDVIEW BUILD_POVRAY
EXIT /b

:ERROR_END
ECHO.
ECHO - %~nx0 FAILED.
ENDLOCAL
EXIT /b 3

:END
ECHO.
ECHO - %~nx0 finished.
ENDLOCAL
EXIT /b

