@ECHO OFF &SETLOCAL
Title Update LPub3D files with build version number
rem --
rem  Trevor SANDY <trevor.sandy@gmail.com>
rem  Last Update: October 11, 2017
rem  Copyright (c) 2015 - 2017 by Trevor Sandy
rem --

SET LP3D_ME=%~nx0

SET LP3D_PWD=%1
if [%LP3D_PWD%] == [] (
  ECHO Error: Did not receive required argument _PRO_FILE_PWD_
  ECHO %LP3D_ME% terminated!
  ECHO.
  EXIT /b 0
)
SET LP3D_PWD=%LP3D_PWD:"=%
SET LP3D_VER_INFO_FILE=%LP3D_PWD%\..\builds\utilities\version.info

:: logging stuff
rem SET LP3D_LOG="%LP3D_PWD%\..\builds\utilities\update-config-files.log"
rem ECHO. Start %LP3D_ME% execution... >   %LP3D_LOG%
rem ECHO  1. capture version info >>  %LP3D_LOG%
ECHO  Start %LP3D_ME% execution...
ECHO  1. capture version info

SET LP3D_VERSION=unknown
SET LP3D_WEEK_DAY=unknown
SET LP3D_MONTH_OF_YEAR=
SET LP3D_AVAILABLE_VERSIONS=

SET LP3D_VER_MAJOR=%2
SET LP3D_VER_MINOR=%3
SET LP3D_VER_PATCH=%4
SET LP3D_VER_REVISION=%5
SET LP3D_VER_BUILD=%6
SET LP3D_VER_SHA_HASH=%7
rem SET LP3D_AVAILABLE_VERSIONS=%8

CALL :GET_DATE_AND_LP3D_TIME

SET LP3D_TIME=%LP3D_HOUR%:%LP3D_MIN%:%LP3D_SEC%
SET LP3D_BUILD_DATE=%LP3D_YEAR%%LP3D_MONTH%%LP3D_DAY%
SET LP3D_BUILD_DATE_TIME=%LP3D_DAY% %LP3D_MONTH% %LP3D_YEAR% %LP3D_TIME%
SET LP3D_CHANGE_DATE_LONG=%LP3D_WEEK_DAY%, %LP3D_DAY% %LP3D_MONTH_OF_YEAR% %LP3D_YEAR% %LP3D_TIME% +0100
SET LP3D_CHANGE_DATE=%LP3D_WEEK_DAY% %LP3D_MONTH_OF_YEAR% %LP3D_DAY% %LP3D_YEAR%
SET LP3D_VERSION=%LP3D_VER_MAJOR%.%LP3D_VER_MINOR%.%LP3D_VER_PATCH%
SET LP3D_APP_VERSION=%LP3D_VERSION%.%LP3D_VER_BUILD%
SET LP3D_APP_VER_SUFFIX=%LP3D_VER_MAJOR%%LP3D_VER_MINOR%
SET LP3D_APP_VERSION_LONG=%LP3D_VERSION%.%LP3D_VER_REVISION%.%LP3D_VER_BUILD%_%LP3D_BUILD_DATE%
SET LP3D_BUILD_VERSION=%LP3D_VERSION%.%LP3D_VER_REVISION%.%LP3D_VER_BUILD% (%LP3D_BUILD_DATE_TIME%)
rem SET LP3D_AVAILABLE_VERSIONS=%LP3D_AVAILABLE_VERSIONS:"=%

::ECHO   LP3D_LOG.......................[%LP3D_LOG%]
ECHO   LP3D_PWD.......................[%LP3D_PWD%]
ECHO   LP3D_VER_MAJOR.................[%LP3D_VER_MAJOR%]
ECHO   LP3D_VER_MINOR.................[%LP3D_VER_MINOR%]
ECHO   LP3D_VER_PATCH.................[%LP3D_VER_PATCH%]
ECHO   LP3D_VER_REVISION..............[%LP3D_VER_REVISION%]
ECHO   LP3D_VER_BUILD.................[%LP3D_VER_BUILD%]
ECHO   LP3D_VER_SHA_HASH..............[%LP3D_VER_SHA_HASH%]
ECHO   LP3D_VERSION...................[%LP3D_VERSION%]
ECHO   LP3D_APP_VERSION...............[%LP3D_APP_VERSION%]
ECHO   LP3D_APP_VER_SUFFIX............[%LP3D_APP_VER_SUFFIX%]
ECHO   LP3D_APP_VERSION_LONG..........[%LP3D_APP_VERSION_LONG%]
ECHO   LP3D_BUILD_VERSION.............[%LP3D_BUILD_VERSION%]
ECHO   LP3D_BUILD_DATE_TIME...........[%LP3D_BUILD_DATE_TIME%]
ECHO   LP3D_CHANGE_DATE_LONG..........[%LP3D_CHANGE_DATE_LONG%]
rem ECHO   LP3D_AVAILABLE_VERSIONS........[%LP3D_AVAILABLE_VERSIONS%]

CALL :GET_AVAILABLE_VERSIONS %*

CD /D "%LP3D_PWD%\..\builds\utilities"
ECHO %LP3D_VER_MAJOR% %LP3D_VER_MINOR% %LP3D_VER_PATCH% %LP3D_VER_REVISION% %LP3D_VER_BUILD% %LP3D_VER_SHA_HASH% %LP3D_BUILD_DATE_TIME% %LP3D_AVAILABLE_VERSIONS% > version.info
IF EXIST version.info (ECHO   FILE version.info..............[written to .\builds\utilities\version.info]) ELSE (ECHO   FILE version.info..............[Error, file not found])
IF [%APPVEYOR%] == [] (
  CD /D "%LP3D_PWD%\..\builds\linux\obs"
  ECHO %LP3D_VERSION%.%$$LP3D_VER_BUILD% > lpub3d.spec.git.version
  IF EXIST lpub3d.spec.git.version (ECHO   FILE lpub3d.spec.git.version...[written to .\builds\linux\obs\lpub3d.spec.git.version]) ELSE (ECHO   FILE lpub3d.spec.git.version...[Error, file not found])
) ELSE (
  CD /D "%LP3D_PWD%\.."
  ECHO %LP3D_APP_VERSION_LONG% > av.version.info
  IF EXIST av.version.info (ECHO   FILE av.version.info...........[written to .\av.version.info]) ELSE (ECHO   FILE av.version.info...........[Error, file not found])
  GOTO :END
)

rem ECHO  2. update desktop configuration - add version suffix 	>>   %LP3D_LOG%
ECHO  2. update desktop configuration - add version suffix
SET LP3D_FILE="%LP3D_PWD%\lpub3d.desktop"
SET /a LineToReplace=10
SET "Replacement=Exec=lpub3d%LP3D_APP_VER_SUFFIX% %%f"
(FOR /f "tokens=1*delims=:" %%a IN ('findstr /n "^" "%LP3D_FILE%"') DO (
  SET "Line=%%b"
  IF %%a equ %LineToReplace% SET "Line=%Replacement%"
    SETLOCAL ENABLEDELAYEDEXPANSION
    ECHO(!Line!
    ENDLOCAL
))>"%LP3D_FILE%.new"
MOVE /Y %LP3D_FILE%.new %LP3D_FILE%

rem ECHO  3. update man page - add version suffix 	>>   %LP3D_LOG%
ECHO  3. update man page - add version suffix
SET LP3D_FILE="%LP3D_PWD%\docs\lpub3d%LP3D_APP_VER_SUFFIX%.1"
SET /a LineToReplace=61
SET "Replacement=     /usr/bin/lpub3d%LP3D_APP_VER_SUFFIX%"
(FOR /f "tokens=1*delims=:" %%a IN ('findstr /n "^" "%LP3D_FILE%"') DO (
  SET "Line=%%b"
  IF %%a equ %LineToReplace% SET "Line=%Replacement%"
    SETLOCAL ENABLEDELAYEDEXPANSION
    ECHO(!Line!
    ENDLOCAL
))>"%LP3D_FILE%.new"
MOVE /Y %LP3D_FILE%.new %LP3D_FILE%

rem ECHO  4. update PKGBUILD - add app version 	>>   %LP3D_LOG%
ECHO  4. update PKGBUILD - add app version
SET LP3D_FILE="%LP3D_PWD%\..\builds\linux\obs\PKGBUILD"
SET /a LineToReplace=3
SET "Replacement=Exec=lpub3d%LP3D_APP_VER_SUFFIX% %%f"
(FOR /f "tokens=1*delims=:" %%a IN ('findstr /n "^" "%LP3D_FILE%"') DO (
  SET "Line=%%b"
  IF %%a equ %LineToReplace% SET "Line=%Replacement%"
    SETLOCAL ENABLEDELAYEDEXPANSION
    ECHO(!Line!
    ENDLOCAL
))>"%LP3D_FILE%.new"
MOVE /Y %LP3D_FILE%.new %LP3D_FILE%

rem ECHO  5. create changelog - add app version and change date 	>>   %LP3D_LOG%
ECHO  5. create changelog - add app version and change date
SET LP3D_FILE="%LP3D_PWD%\..\builds\linux\obs\debian\changelog"
IF EXIST %LP3D_FILE% DEL /Q %LP3D_FILE%
SET createChangeLog=%LP3D_FILE% ECHO
:GENERATE debian change log
>%createChangeLog% lpub3d ^(%LP3D_APP_VERSION%^) trusty; urgency=medium
>>%createChangeLog%.
>>%createChangeLog%   * LPub3D version %LP3D_APP_VERSION_LONG% for Linux
>>%createChangeLog%.
>>%createChangeLog%  -- Trevor SANDY ^<trevor.sandy@gmail.com^>  %LP3D_CHANGE_DATE_LONG%

rem ECHO  6. update lpub3d.dsc - add app version 	>>   %LP3D_LOG%
ECHO  6. update lpub3d.dsc - add app version
SET LP3D_FILE="%LP3D_PWD%\..\builds\linux\obs\debian\lpub3d.dsc"
SET /a LineToReplace=5
SET "Replacement=Version: %LP3D_APP_VERSION%"
(FOR /f "tokens=1*delims=:" %%a IN ('findstr /n "^" "%LP3D_FILE%"') DO (
  SET "Line=%%b"
  IF %%a equ %LineToReplace% SET "Line=%Replacement%"
    SETLOCAL ENABLEDELAYEDEXPANSION
    ECHO(!Line!
    ENDLOCAL
))>"%LP3D_FILE%.new"
MOVE /Y %LP3D_FILE%.new %LP3D_FILE%

rem ECHO  7. update README.txt - add build version 	>>   %LP3D_LOG%
ECHO  7. update README.txt - add build version
SET LP3D_FILE="%LP3D_PWD%\..\mainApp\docs\README.txt"
SET /a LineToReplace=1
SET "Replacement=LPub3D %LP3D_BUILD_VERSION%"
(FOR /f "tokens=1*delims=:" %%a IN ('findstr /n "^" "%LP3D_FILE%"') DO (
  SET "Line=%%b"
  IF %%a equ %LineToReplace% SET "Line=%Replacement%"
    SETLOCAL ENABLEDELAYEDEXPANSION
    ECHO(!Line!
    ENDLOCAL
))>"%LP3D_FILE%.new"
MOVE /Y %LP3D_FILE%.new %LP3D_FILE%

rem ECHO  8. update lpub3d.spec - add app version and change date 	>>   %LP3D_LOG%
ECHO  8. update lpub3d.spec - add app version and change date
SET LP3D_FILE="%LP3D_PWD%\..\builds\linux\obs\lpub3d.spec"
SET /a LineToReplace=222
SET "Replacement=* %LP3D_CHANGE_DATE% - trevor.dot.sandy.at.gmail.dot.com %LP3D_APP_VERSION%"
(FOR /f "tokens=1*delims=:" %%a IN ('findstr /n "^" "%LP3D_FILE%"') DO (
  SET "Line=%%b"
  IF %%a equ %LineToReplace% SET "Line=%Replacement%"
    SETLOCAL ENABLEDELAYEDEXPANSION
    ECHO(!Line!
    ENDLOCAL
))>"%LP3D_FILE%.new"
MOVE /Y %LP3D_FILE%.new %LP3D_FILE%

rem ECHO  9. create source 'lpub3d.spec.git.version' 	>>   %LP3D_LOG%
ECHO  9. create source 'lpub3d.spec.git.version'
SET LP3D_FILE="%LP3D_PWD%\..\builds\linux\obs\lpub3d.spec.git.version"
IF EXIST %LP3D_FILE% DEL /Q %LP3D_FILE%
SET createSpecGitVersion=%LP3D_FILE% ECHO
:GENERATE create lpub3d.spec.git.version
>%createSpecGitVersion% %LP3D_APP_VERSION%

ENDLOCAL
GOTO :END

:GET_DATE_AND_LP3D_TIME
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
EXIT /b 0

:GET_AVAILABLE_VERSIONS
FOR /f "tokens=8,*" %%a IN ("%*") DO SET LP3D_AVAILABLE_VERSIONS=%%a
ECHO   LP3D_AVAILABLE_VERSIONS........[%LP3D_AVAILABLE_VERSIONS%]
EXIT /b

:END
rem ECHO Script %LP3D_ME% execution finished.  >> %LP3D_LOG%
ECHO  Script %LP3D_ME% execution finished.
EXIT /b 0
