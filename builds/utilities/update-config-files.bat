@ECHO OFF &SETLOCAL
Title Update LPub3D files with build version number
rem --
rem  Trevor SANDY <trevor.sandy@gmail.com>
rem  Last Update: February 19, 2017
rem  Copyright (c) 2015 - 2017 by Trevor Sandy
rem --		

SET ME=%~nx0
SET PWD=%1
SET WORK_DIR=%cd%

if [%PWD%] == [] (
  ECHO Error: Did not receive required argument _PRO_FILE_PWD_
  ECHO %ME% terminated!
  ECHO.
  EXIT /b 0
)
SET PWD=%PWD:"=%

:: logging stuff
SET LOG="%PWD%\..\builds\utilities\update-config-files.log"
ECHO. Start %ME% execution... >		%LOG%
ECHO  1. capture version info >>	%LOG%
ECHO  Start %ME% execution...
ECHO  1. capture version info

SET VERSION=unknown
SET VER_MAJOR=unknown
SET VER_MINOR=unknown
SET VER_BUILD=unknown
SET VER_PATCH=unknown
SET VER_REVISION=unknown
SET VER_SHA_HASH=unknown
SET WEEK_DAY=unknown
SET MONTH_OF_YEAR=

SET VERSION_INFO_FILE=%PWD%\..\builds\utilities\version.info
if NOT EXIST "%VERSION_INFO_FILE%" (
  ECHO Error: Cannot read "%VERSION_INFO_FILE%" from %PWD%
  ECHO %ME% terminated!
  ECHO.
  EXIT /b 0
)
FOR /f "tokens=1" %%i IN (%VERSION_INFO_FILE%) DO SET VER_MAJOR=%%i
FOR /f "tokens=2" %%i IN (%VERSION_INFO_FILE%) DO SET VER_MINOR=%%i
FOR /f "tokens=3" %%i IN (%VERSION_INFO_FILE%) DO SET VER_PATCH=%%i
FOR /f "tokens=4" %%i IN (%VERSION_INFO_FILE%) DO SET VER_REVISION=%%i
FOR /f "tokens=5" %%i IN (%VERSION_INFO_FILE%) DO SET VER_BUILD=%%i
FOR /f "tokens=6" %%i IN (%VERSION_INFO_FILE%) DO SET VER_SHA_HASH=%%i

CALL :GET_DATE_AND_TIME

SET TIME=%HOUR%:%MIN%:%SEC%
SET DATE_TIME=%DAY% %MONTH% %YEAR% %TIME%
SET BUILD_DATE=%YEAR%%MONTH%%DAY%
SET CHANGE_DATE_LONG=%WEEK_DAY%, %DAY% %MONTH_OF_YEAR% %YEAR% %TIME% +0100
SET CHANGE_DATE=%WEEK_DAY% %MONTH_OF_YEAR% %DAY% %YEAR%
SET VERSION=%VER_MAJOR%.%VER_MINOR%.%VER_PATCH%
SET APP_VERSION=%VERSION%.%VER_BUILD%
SET APP_VER_SUFFIX=%VER_MAJOR%%VER_MINOR%
SET APP_VERSION_LONG=%VERSION%.%VER_REVISION%.%VER_BUILD%_%BUILD_DATE%
SET BUILDVERSION=%VERSION%.%VER_REVISION%.%VER_BUILD% (%DATE_TIME%)
::ECHO   LOG.................[%LOG%]
ECHO   WORK_DIR............[%WORK_DIR%]
ECHO   VER_MAJOR...........[%VER_MAJOR%]
ECHO   VER_MINOR...........[%VER_MINOR%]
ECHO   VER_PATCH...........[%VER_PATCH%]
ECHO   VER_REVISION........[%VER_REVISION%]
ECHO   VER_BUILD...........[%VER_BUILD%]
ECHO   VER_SHA_HASH........[%VER_SHA_HASH%]
ECHO   VERSION.............[%VERSION%]
ECHO   APP_VERSION.........[%APP_VERSION%]
ECHO   APP_VER_SUFFIX......[%APP_VER_SUFFIX%]
ECHO   APP_VERSION_LONG....[%APP_VERSION_LONG%]
ECHO   BUILDVERSION........[%BUILDVERSION%]
ECHO   DATE_TIME...........[%DATE_TIME%]
ECHO   CHANGE_DATE_LONG....[%CHANGE_DATE_LONG%]

ECHO  2. update desktop configuration - add version suffix 	>>   %LOG%
ECHO  2. update desktop configuration - add version suffix
SET FILE="%PWD%\lpub3d.desktop"
SET /a LineToReplace=10
SET "Replacement=Exec=lpub3d%APP_VER_SUFFIX% %%f"
(FOR /f "tokens=1*delims=:" %%a IN ('findstr /n "^" "%FILE%"') DO (
  SET "Line=%%b"
  IF %%a equ %LineToReplace% SET "Line=%Replacement%"
    SETLOCAL ENABLEDELAYEDEXPANSION
    ECHO(!Line!
    ENDLOCAL
))>"%FILE%.new"
MOVE /Y %FILE%.new %FILE%     >>		%LOG%

ECHO  3. update man page - add version suffix 	>>   %LOG%
ECHO  3. update man page - add version suffix
SET FILE="%PWD%\docs\lpub3d%APP_VER_SUFFIX%.1"
SET /a LineToReplace=61
SET "Replacement=     /usr/bin/lpub3d%APP_VER_SUFFIX%"
(FOR /f "tokens=1*delims=:" %%a IN ('findstr /n "^" "%FILE%"') DO (
  SET "Line=%%b"
  IF %%a equ %LineToReplace% SET "Line=%Replacement%"
    SETLOCAL ENABLEDELAYEDEXPANSION
    ECHO(!Line!
    ENDLOCAL
))>"%FILE%.new"
MOVE /Y %FILE%.new %FILE%     >>		%LOG%

ECHO  4. update PKGBUILD - add app version 	>>   %LOG%
ECHO  4. update PKGBUILD - add app version
SET FILE="%PWD%\..\builds\linux\obs\PKGBUILD"
SET /a LineToReplace=3
SET "Replacement=Exec=lpub3d%APP_VER_SUFFIX% %%f"
(FOR /f "tokens=1*delims=:" %%a IN ('findstr /n "^" "%FILE%"') DO (
  SET "Line=%%b"
  IF %%a equ %LineToReplace% SET "Line=%Replacement%"
    SETLOCAL ENABLEDELAYEDEXPANSION
    ECHO(!Line!
    ENDLOCAL
))>"%FILE%.new"
MOVE /Y %FILE%.new %FILE%     >>		%LOG%

ECHO  5. create changelog - add app version and change date 	>>   %LOG%
ECHO  5. create changelog - add app version and change date
SET FILE="%PWD%\..\builds\linux\obs\debian\changelog"
IF EXIST %FILE% DEL /Q %FILE%
SET createChangeLog=%FILE% ECHO
:GENERATE debian change log
>%createChangeLog% lpub3d ^(%APP_VERSION%^) trusty; urgency=medium
>>%createChangeLog%.
>>%createChangeLog%   * LPub3D version %APP_VERSION_LONG% for Linux
>>%createChangeLog%.
>>%createChangeLog%  -- Trevor SANDY ^<trevor.sandy@gmail.com^>  %CHANGE_DATE_LONG%

ECHO  6. update lpub3d.dsc - add app version 	>>   %LOG%
ECHO  6. update lpub3d.dsc - add app version
SET FILE="%PWD%\..\builds\linux\obs\debian\lpub3d.dsc"
SET /a LineToReplace=5
SET "Replacement=Version: %APP_VERSION%"
(FOR /f "tokens=1*delims=:" %%a IN ('findstr /n "^" "%FILE%"') DO (
  SET "Line=%%b"
  IF %%a equ %LineToReplace% SET "Line=%Replacement%"
    SETLOCAL ENABLEDELAYEDEXPANSION
    ECHO(!Line!
    ENDLOCAL
))>"%FILE%.new"
MOVE /Y %FILE%.new %FILE%     >>		%LOG%

ECHO  7. update README.txt - add build version 	>>   %LOG%
ECHO  7. update README.txt - add build version
SET FILE="%PWD%\..\mainApp\docs\README.txt"
SET /a LineToReplace=1
SET "Replacement=LPub3D %BUILDVERSION%"
(FOR /f "tokens=1*delims=:" %%a IN ('findstr /n "^" "%FILE%"') DO (
  SET "Line=%%b"
  IF %%a equ %LineToReplace% SET "Line=%Replacement%"
    SETLOCAL ENABLEDELAYEDEXPANSION
    ECHO(!Line!
    ENDLOCAL
))>"%FILE%.new"
MOVE /Y %FILE%.new %FILE%     >>		%LOG%

ECHO  8. update lpub3d.spec - add app version and change date 	>>   %LOG%
ECHO  8. update lpub3d.spec - add app version and change date
SET FILE="%PWD%\..\builds\linux\obs\lpub3d.spec"
SET /a LineToReplace=222
SET "Replacement=* %CHANGE_DATE% - trevor.dot.sandy.at.gmail.dot.com %APP_VERSION%"
(FOR /f "tokens=1*delims=:" %%a IN ('findstr /n "^" "%FILE%"') DO (
  SET "Line=%%b"
  IF %%a equ %LineToReplace% SET "Line=%Replacement%"
    SETLOCAL ENABLEDELAYEDEXPANSION
    ECHO(!Line!
    ENDLOCAL
))>"%FILE%.new"
MOVE /Y %FILE%.new %FILE%     >>		%LOG%

ECHO  9. create source 'lpub3d.spec.git.version' 	>>   %LOG%
ECHO  9. create source 'lpub3d.spec.git.version'
SET FILE="%PWD%\..\builds\linux\obs\lpub3d.spec.git.version"
IF EXIST %FILE% DEL /Q %FILE%
SET createSpecGitVersion=%FILE% ECHO
:GENERATE create lpub3d.spec.git.version
>%createSpecGitVersion% %APP_VERSION%

ECHO. Script %ME% execution finished.  >>	%LOG%
ECHO. Script %ME% execution finished.
EXIT /b 0

:GET_DATE_AND_TIME
SET DAY=unknown
SET MONTH=unknown
SET YEAR=unknown
SET HOUR=unknown
SET MIN=unknown
SET SEC=unknown
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
SET DAY=%_Day:~-2%
SET MONTH=%_Month:~-2%
SET YEAR=%_Year%
SET HOUR=%_Hour:~-2%
SET MIN=%_Min:~-2%
SET SEC=%_Sec:~-2%

IF %_WeekDay% == 0 SET WEEK_DAY=Sun
IF %_WeekDay% == 1 SET WEEK_DAY=Mon
IF %_WeekDay% == 2 SET WEEK_DAY=Tue
IF %_WeekDay% == 3 SET WEEK_DAY=Wed
IF %_WeekDay% == 4 SET WEEK_DAY=Thu
IF %_WeekDay% == 5 SET WEEK_DAY=Fri
IF %_WeekDay% == 6 SET WEEK_DAY=Sat

IF %MONTH% == 01 SET MONTH_OF_YEAR=Jan
IF %MONTH% == 02 SET MONTH_OF_YEAR=Feb
IF %MONTH% == 03 SET MONTH_OF_YEAR=Mar
IF %MONTH% == 04 SET MONTH_OF_YEAR=Apr
IF %MONTH% == 05 SET MONTH_OF_YEAR=May
IF %MONTH% == 06 SET MONTH_OF_YEAR=Jun
IF %MONTH% == 07 SET MONTH_OF_YEAR=Jul
IF %MONTH% == 08 SET MONTH_OF_YEAR=Aug
IF %MONTH% == 09 SET MONTH_OF_YEAR=Sep
IF %MONTH% == 10 SET MONTH_OF_YEAR=Oct
IF %MONTH% == 11 SET MONTH_OF_YEAR=Nov
IF %MONTH% == 12 SET MONTH_OF_YEAR=Dec
EXIT /b 0
