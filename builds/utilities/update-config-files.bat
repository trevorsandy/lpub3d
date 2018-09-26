@ECHO OFF &SETLOCAL
Title Update LPub3D files with build version number
rem --
rem  Trevor SANDY <trevor.sandy@gmail.com>
rem  Last Update: March 29, 2017
rem  Copyright (c) 2015 - 2018 by Trevor SANDY
rem --
rem --
rem This script is distributed in the hope that it will be useful,
rem but WITHOUT ANY WARRANTY; without even the implied warranty of
rem MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

SET LP3D_ME=%~nx0

CALL :FIXUP_PWD %1

IF [%LP3D_BUILDS_DIR%] == [] (
  ECHO Error: Did not receive required argument _PRO_FILE_PWD_
  ECHO %LP3D_ME% terminated!
  ECHO.
  GOTO :END
)

SET LINE_README_TXT=1
SET LINE_README_MD_VER=70
SET LINE_RELEASE_NOTES_HTM=2

SET LP3D_GIT_DEPTH=1000
SET LP3D_PAST_RELEASES=1.3.5,1.2.3,1.0.0
SET LP3D_BUILDS_DIR=%LP3D_BUILDS_DIR:"=%
SET LP3D_CALL_DIR=%CD%


ECHO  Start %LP3D_ME% execution at %CD%...
IF [%3] EQU [] (
  ECHO  capture version info using git queries...
  CALL :GET_GIT_VERSION
) ELSE (
  ECHO  capture version info using version arguments...
  SET LP3D_VER_MAJOR=%2
  SET LP3D_VER_MINOR=%3
  SET LP3D_VER_PATCH=%4
  SET LP3D_VER_REVISION=%5
  SET LP3D_VER_BUILD=%6
  SET LP3D_VER_SHA_HASH=%7
  IF [%8] NEQ [] (SET LP3D_VER_SUFFIX=%8)
)

SET LP3D_VERSION=unknown
SET LP3D_WEEK_DAY=unknown
SET LP3D_MONTH_OF_YEAR=unknown
SET LP3D_AVAILABLE_VERSIONS=unknown

CALL :GET_DATE_AND_TIME

SET LP3D_TIME=%LP3D_HOUR%:%LP3D_MIN%:%LP3D_SEC%
SET LP3D_BUILD_DATE=%LP3D_YEAR%%LP3D_MONTH%%LP3D_DAY%
SET LP3D_BUILD_DATE_TIME=%LP3D_DAY% %LP3D_MONTH% %LP3D_YEAR% %LP3D_TIME%
SET LP3D_CHANGE_DATE_LONG=%LP3D_WEEK_DAY%, %LP3D_DAY% %LP3D_MONTH_OF_YEAR% %LP3D_YEAR% %LP3D_TIME% +0100
SET LP3D_CHANGE_DATE=%LP3D_WEEK_DAY% %LP3D_MONTH_OF_YEAR% %LP3D_DAY% %LP3D_YEAR%
SET LP3D_VERSION=%LP3D_VER_MAJOR%.%LP3D_VER_MINOR%.%LP3D_VER_PATCH%
SET LP3D_APP_VERSION=%LP3D_VERSION%.%LP3D_VER_BUILD%
SET LP3D_APP_VERSION_TAG=v%LP3D_VERSION%
SET LP3D_APP_VER_SUFFIX=%LP3D_VER_MAJOR%%LP3D_VER_MINOR%
SET LP3D_APP_VERSION_LONG=%LP3D_VERSION%.%LP3D_VER_REVISION%.%LP3D_VER_BUILD%_%LP3D_BUILD_DATE%
SET LP3D_BUILD_VERSION=%LP3D_VERSION%.%LP3D_VER_REVISION%.%LP3D_VER_BUILD% ^(%LP3D_BUILD_DATE_TIME%^)

CALL :GET_AVAILABLE_VERSIONS %*

SET LP3D_VERSION_INFO=%LP3D_VER_MAJOR% %LP3D_VER_MINOR% %LP3D_VER_PATCH% %LP3D_VER_REVISION% %LP3D_VER_BUILD% %LP3D_VER_SHA_HASH%
IF [%LP3D_VER_SUFFIX%] NEQ [] (
  SET LP3D_VERSION_INFO=%LP3D_VERSION_INFO% %LP3D_VER_SUFFIX%
  SET LP3D_APP_VERSION_TAG=v%LP3D_VERSION%_%LP3D_VER_SUFFIX%
)

SET LP3D_FILE="%LP3D_MAIN_APP%\docs\RELEASE_NOTES.html"
ECHO  update RELEASE_NOTES.html build version [%LP3D_FILE%]
SET /a LineToReplace=%LINE_RELEASE_NOTES_HTM%
SET "Replacement=^<h5^>^<a id="LPub3D_0"^>^</a^>LPub3D %LP3D_BUILD_VERSION%^</h5^>"
ECHO DEBUG Replacement VAR IS %Replacement%
(FOR /f "tokens=1*delims=:" %%a IN ('findstr /n "^" "%LP3D_FILE%"') DO (
  SET "Line=%%b"
  IF %%a equ %LineToReplace% SET "Line=%Replacement:^=%"
    SETLOCAL ENABLEDELAYEDEXPANSION
    ECHO(!Line!
    ENDLOCAL
))>"%LP3D_FILE%.new"
MOVE /Y %LP3D_FILE%.new %LP3D_FILE% | findstr /i /v /r /c:"moved\>"

SET LP3D_FILE="%LP3D_MAIN_APP%\docs\README.txt"
ECHO  update README.txt build version [%LP3D_FILE%]
SET /a LineToReplace=%LINE_README_TXT%
SET "Replacement=LPub3D %LP3D_BUILD_VERSION%"
(FOR /f "tokens=1*delims=:" %%a IN ('findstr /n "^" "%LP3D_FILE%"') DO (
  SET "Line=%%b"
  IF %%a equ %LineToReplace% SET "Line=%Replacement%"
    SETLOCAL ENABLEDELAYEDEXPANSION
    ECHO(!Line!
    ENDLOCAL
))>"%LP3D_FILE%.new"
MOVE /Y %LP3D_FILE%.new %LP3D_FILE% | findstr /i /v /r /c:"moved\>"

SET LP3D_FILE="%LP3D_MAIN_APP%\..\README.md"
ECHO  update README.md version        [%LP3D_FILE%]
SET /a LineToReplace=%LINE_README_MD_VER%
SET "Replacement=[sfreleases]:       https://sourceforge.net/projects/lpub3d/files/%LP3D_VERSION%/"
(FOR /f "tokens=1*delims=:" %%a IN ('findstr /n "^" "%LP3D_FILE%"') DO (
  SET "Line=%%b"
  IF %%a equ %LineToReplace% SET "Line=%Replacement%"
    SETLOCAL ENABLEDELAYEDEXPANSION
    ECHO(!Line!
    ENDLOCAL
))>"%LP3D_FILE%.new"
MOVE /Y %LP3D_FILE%.new %LP3D_FILE% | findstr /i /v /r /c:"moved\>"

ECHO   LPUB3D_DIR.....................[%LPUB3D%]
ECHO   LP3D_BUILDS_DIR................[%LP3D_BUILDS_DIR%]
ECHO   LP3D_CALL_DIR..................[%LP3D_CALL_DIR%]
ECHO   LP3D_GIT_DEPTH.................[%LP3D_GIT_DEPTH%]

ECHO   LP3D_VERSION_INFO..............[%LP3D_VERSION_INFO%]
ECHO   LP3D_VER_MAJOR.................[%LP3D_VER_MAJOR%]
ECHO   LP3D_VER_MINOR.................[%LP3D_VER_MINOR%]
ECHO   LP3D_VER_PATCH.................[%LP3D_VER_PATCH%]
ECHO   LP3D_VER_REVISION..............[%LP3D_VER_REVISION%]
ECHO   LP3D_VER_BUILD.................[%LP3D_VER_BUILD%]
ECHO   LP3D_VER_SHA_HASH..............[%LP3D_VER_SHA_HASH%]
IF [%LP3D_VER_SUFFIX%] NEQ [] (
  ECHO   LP3D_VER_SUFFIX................[%LP3D_VER_SUFFIX%]
)
ECHO   LP3D_APP_VER_SUFFIX............[%LP3D_APP_VER_SUFFIX%]
ECHO   LP3D_BUILD_VERSION.............[%LP3D_BUILD_VERSION%]
ECHO   LP3D_BUILD_DATE_TIME...........[%LP3D_BUILD_DATE_TIME%]
ECHO   LP3D_CHANGE_DATE_LONG..........[%LP3D_CHANGE_DATE_LONG%]

ECHO   LP3D_VERSION...................[%LP3D_VERSION%]
ECHO   LP3D_APP_VERSION...............[%LP3D_APP_VERSION%]
ECHO   LP3D_APP_VERSION_LONG..........[%LP3D_APP_VERSION_LONG%]
REM ECHO   LP3D_APP_VERSION_TAG...........[%LP3D_APP_VERSION_TAG%]

ECHO   LP3D_SOURCE_DIR................[%LPUB3D%-%LP3D_APP_VERSION%]
ECHO   LP3D_AVAILABLE_VERSIONS........[%LP3D_AVAILABLE_VERSIONS%]

SET LP3D_VER_INFO_FILE=%LP3D_BUILDS_DIR%\utilities\version.info
IF EXIST "%LP3D_VER_INFO_FILE%" DEL /Q "%LP3D_VER_INFO_FILE%"
ECHO %LP3D_VERSION_INFO% > %LP3D_VER_INFO_FILE%
IF EXIST "%LP3D_VER_INFO_FILE%" (
  ECHO   FILE version.info..............[written to builds\utilities\version.info]
) ELSE (
  ECHO   FILE version.info..............[ERROR - file %LP3D_VER_INFO_FILE% not found]
)
GOTO :END

:FIXUP_PWD
SET TEMP=%CD%
IF [%1] NEQ [] CD /D "%1\..\builds"
SET LP3D_MAIN_APP=%1
SET LP3D_BUILDS_DIR=%CD%
CD "%1\.."
FOR %%* IN (%CD%) DO SET LPUB3D=%%~nx*
CD %TEMP%
EXIT /b

:GET_GIT_VERSION
CD /D "%LP3D_BUILDS_DIR%\.."
IF "%APPVEYOR%" EQU "True" (
  REM Update refs and tags
  git fetch -qfup --depth=%LP3D_GIT_DEPTH% origin +%APPVEYOR_REPO_BRANCH% +refs/tags/*:refs/tags/*
  git checkout -qf %APPVEYOR_REPO_COMMIT%
)

REM Extract Revision Number
FOR /F "usebackq delims==" %%G IN (`git describe --tags --long`) DO SET lp3d_git_ver_tag_long=%%G
SET "tag_input=%lp3d_git_ver_tag_long%"

REM Remove everything before and including "-"
SET "tag_val_1=%tag_input:*-=%"
IF "%tag_val_1%"=="%tag_input%" ECHO ERROR - revision prefix ending in "-" not found in %tag_input%
FOR /F "delims=\" %%a IN ("%tag_val_1%") DO SET "tag_val_1=%%~a"

REM Remove everything after and including "-"
SET "lp3d_revision_=%tag_val_1%"
FOR /F "tokens=1 delims=-" %%a IN ("%lp3d_revision_%") DO SET LP3D_VER_REVISION=%%~a

REM Extract commit count (build)
FOR /F "usebackq delims==" %%G IN (`git rev-list HEAD --count`) DO SET LP3D_VER_BUILD=%%G

REM Extract short sha hash
FOR /F "usebackq delims==" %%G IN (`git rev-parse --short HEAD`) DO SET LP3D_VER_SHA_HASH=%%G

REM Extract version
FOR /F "usebackq delims==" %%G IN (`git describe --tags --abbrev^=0`) DO SET lp3d_git_ver_tag_short=%%G

REM Remove version prefix 'v'
SET "lp3d_version_=%lp3d_git_ver_tag_short:v=%"

REM Capture version suffix - everything after "_" if it exist
FOR /F "tokens=2 delims=_" %%a IN ("%lp3d_version_%") DO SET LP3D_VER_SUFFIX=%%~a

REM Remove version suffix - everything after and including "_" if it exist
IF [%LP3D_VER_SUFFIX%] NEQ [] (
  FOR /F "tokens=1 delims=_" %%a IN ("%lp3d_version_%") DO SET lp3d_version_=%%~a
)

REM Replace version '.' with ' '
SET "lp3d_version_=%lp3d_version_:.= %"

REM Parse version
FOR /F "tokens=1" %%i IN ("%lp3d_version_%") DO SET LP3D_VER_MAJOR=%%i
FOR /F "tokens=2" %%i IN ("%lp3d_version_%") DO SET LP3D_VER_MINOR=%%i
FOR /F "tokens=3" %%i IN ("%lp3d_version_%") DO SET LP3D_VER_PATCH=%%i
EXIT /b

:GET_DATE_AND_TIME
SET LP3D_DAY=unknown
SET LP3D_MONTH=unknown
SET LP3D_YEAR=unknown
SET LP3D_HOUR=unknown
SET LP3D_MIN=unknown
SET LP3D_SEC=unknown
FOR /F "skip=1 delims=" %%F IN ('
    wmic PATH Win32_LocalTime GET Day^,DayOfWeek^,Hour^,Minute^,Month^,Second^,Year /FORMAT:TABLE
') DO (
    FOR /F "tokens=1-7" %%L IN ("%%F") DO (
        SET _Day=0%%L
        SET _WeekDay=%%M
        SET _Hour=0%%N
        SET _Min=0%%O
        SET _Month=0%%P
        SET _Sec=0%%Q
        SET _Year=%%R
    )
)
SET LP3D_DAY=%_Day:~-2%
SET LP3D_MONTH=%_Month:~-2%
SET LP3D_YEAR=%_Year%
SET LP3D_HOUR=%_Hour:~-2%
SET LP3D_MIN=%_Min:~-2%
SET LP3D_SEC=%_Sec:~-2%

IF %_WeekDay% == 0 SET LP3D_WEEK_DAY=Sun
IF %_WeekDay% == 1 SET LP3D_WEEK_DAY=Mon
IF %_WeekDay% == 2 SET LP3D_WEEK_DAY=Tue
IF %_WeekDay% == 3 SET LP3D_WEEK_DAY=Wed
IF %_WeekDay% == 4 SET LP3D_WEEK_DAY=Thu
IF %_WeekDay% == 5 SET LP3D_WEEK_DAY=Fri
IF %_WeekDay% == 6 SET LP3D_WEEK_DAY=Sat

IF %LP3D_MONTH% == 01 SET LP3D_MONTH_OF_YEAR=Jan
IF %LP3D_MONTH% == 02 SET LP3D_MONTH_OF_YEAR=Feb
IF %LP3D_MONTH% == 03 SET LP3D_MONTH_OF_YEAR=Mar
IF %LP3D_MONTH% == 04 SET LP3D_MONTH_OF_YEAR=Apr
IF %LP3D_MONTH% == 05 SET LP3D_MONTH_OF_YEAR=May
IF %LP3D_MONTH% == 06 SET LP3D_MONTH_OF_YEAR=Jun
IF %LP3D_MONTH% == 07 SET LP3D_MONTH_OF_YEAR=Jul
IF %LP3D_MONTH% == 08 SET LP3D_MONTH_OF_YEAR=Aug
IF %LP3D_MONTH% == 09 SET LP3D_MONTH_OF_YEAR=Sep
IF %LP3D_MONTH% == 10 SET LP3D_MONTH_OF_YEAR=Oct
IF %LP3D_MONTH% == 11 SET LP3D_MONTH_OF_YEAR=Nov
IF %LP3D_MONTH% == 12 SET LP3D_MONTH_OF_YEAR=Dec
EXIT /b

:GET_AVAILABLE_VERSIONS
SET LP3D_AVAILABLE_VERSIONS=%LP3D_VERSION%,%LP3D_PAST_RELEASES%
SET LP3D_PREVIOUS_TAG=unknown
IF [%9] == [] (
  FOR /F "usebackq" %%G IN (`git describe --abbrev^=0 %lp3d_git_ver_tag_short%^^^^ 2^> nul`) DO (
    IF [%%G] NEQ [] SET LP3D_PREVIOUS_TAG=%%G
  )
) ELSE (
  FOR /F "tokens=9*" %%G IN ("%*") DO SET LP3D_PREVIOUS_VERSION=%%G
)
IF "%LP3D_PREVIOUS_TAG%" NEQ "unknown" (
  FOR /F "usebackq" %%G IN (`git describe --abbrev^=0 %LP3D_PREVIOUS_TAG%^^^^ 2^> nul`) DO (
    IF [%%G] NEQ [] SET LP3D_PREVIOUS_VERSIONS=%LP3D_PREVIOUS_TAG%,%%G
  )
)
IF "%LP3D_PREVIOUS_VERSIONS%" NEQ "" (
  rem ECHO   PREVIOUS VERSIONS   = %LP3D_PREVIOUS_VERSIONS%
  SET LP3D_AVAILABLE_VERSIONS=%LP3D_VERSION%,%LP3D_PREVIOUS_VERSIONS:v=%,%LP3D_PAST_RELEASES%
)
CD /D "%LP3D_BUILDS_DIR%"
EXIT /b

:FIND_REPLACE <findstr> <replstr> <file>
SET tmp="%temp%\tmp.txt"
IF NOT EXIST %temp%\_.vbs CALL :MAKE_REPLACE
FOR /F "tokens=*" %%a IN ('DIR "%3" /s /b /a-d /on') do (
  FOR /F "usebackq" %%b IN (`Findstr /mic:"%~1" "%%a"`) do (
    ECHO(&ECHO     Replacing "%~1" with "%~2" in file %%~nxa
    <%%a cscript //nologo %temp%\_.vbs "%~1" "%~2">%tmp%
    IF EXIST %tmp% MOVE /Y %tmp% "%%~dpnxa">nul
  )
)
DEL %temp%\_.vbs
EXIT /b

:MAKE_REPLACE
>%temp%\_.vbs echo with Wscript
>>%temp%\_.vbs echo set args=.arguments
>>%temp%\_.vbs echo .StdOut.Write _
>>%temp%\_.vbs echo Replace(.StdIn.ReadAll,args(0),args(1),1,-1,1)
>>%temp%\_.vbs echo end with
EXIT /b

:END
ECHO  %LP3D_ME% execution finished.
ENDLOCAL & (
  SET LP3D_SOURCE_DIR=%LP3D_SOURCE_DIR%
  SET LP3D_CALL_DIR=%LP3D_CALL_DIR%

  SET LP3D_DAY=%LP3D_DAY%
  SET LP3D_MONTH=%LP3D_MONTH%
  SET LP3D_YEAR=%LP3D_YEAR%
  SET LP3D_HOUR=%LP3D_HOUR%
  SET LP3D_MIN=%LP3D_MIN%
  SET LP3D_SEC=%LP3D_SEC%

  SET LP3D_TIME=%LP3D_TIME%
  SET LP3D_WEEK_DAY=%LP3D_WEEK_DAY%
  SET LP3D_MONTH_OF_YEAR=%LP3D_MONTH_OF_YEAR%

  SET LP3D_VER_MAJOR=%LP3D_VER_MAJOR%
  SET LP3D_VER_MINOR=%LP3D_VER_MINOR%
  SET LP3D_VER_PATCH=%LP3D_VER_PATCH%
  SET LP3D_VER_REVISION=%LP3D_VER_REVISION%
  SET LP3D_VER_BUILD=%LP3D_VER_BUILD%
  SET LP3D_VER_SHA_HASH=%LP3D_VER_SHA_HASH%
  IF [%LP3D_VER_SUFFIX%] NEQ [] (
    SET LP3D_VER_SUFFIX=%LP3D_VER_SUFFIX%
  )
  SET LP3D_VERSION=%LP3D_VERSION%
  SET LP3D_APP_VERSION=%LP3D_APP_VERSION%
  SET LP3D_APP_VERSION_TAG=%LP3D_APP_VERSION_TAG%
  SET LP3D_APP_VER_SUFFIX=%LP3D_APP_VER_SUFFIX%
  SET LP3D_APP_VERSION_LONG=%LP3D_APP_VERSION_LONG%
  SET LP3D_VERSION_INFO=%LP3D_VERSION_INFO%
  SET LP3D_BUILD_DATE_TIME=%LP3D_BUILD_DATE_TIME%
  SET LP3D_CHANGE_DATE_LONG=%LP3D_CHANGE_DATE_LONG%
  SET LP3D_AVAILABLE_VERSIONS=%LP3D_AVAILABLE_VERSIONS%
  SET LP3D_BUILD_VERSION=%LP3D_VERSION%.%LP3D_VER_REVISION%.%LP3D_VER_BUILD% ^(%LP3D_BUILD_DATE_TIME%^)
)
EXIT /b 0
