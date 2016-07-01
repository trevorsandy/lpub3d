@ECHO on
Title Build json download input data file
SETLOCAL
@break off
@color 0a

SET VER_MAJOR=unknown
SET VER_MINOR=unknown
SET VER_SP=unknown
SET VER_LATEST=unknown

SET devRootPath="..\..\mainApp"

CD /D %devRootPath%
FOR /F "tokens=3*" %%i IN ('FINDSTR /c:"#define VER_MAJOR" version.h') DO SET VER_MAJOR=%%i
FOR /F "tokens=3*" %%i IN ('FINDSTR /c:"#define VER_MINOR" version.h') DO SET VER_MINOR=%%i
FOR /F "tokens=3*" %%i IN ('FINDSTR /c:"#define VER_SP" version.h') DO SET VER_SP=%%i

SET VERSION=%VER_MAJOR%.%VER_MINOR%.%VER_SP%

SET HOUR=%TIME:~0,2%
IF "%HOUR:~0,1%" == " " SET HOUR=0%HOUR:~1,1%
SET MIN=%TIME:~3,2%
IF "%MIN:~0,1%" == " " SET MIN=0%MIN:~1,1%
SET SECS=%TIME:~6,2%
IF "%SECS:~0,1%" == " " SET SECS=0%SECS:~1,1%

SET YEAR=%DATE:~-4%
SET MONTH=%DATE:~3,2%
IF "%MONTH:~0,1%" == " " SET MONTH=0%MONTH:~1,1%
SET DAY=%DATE:~0,2%
IF "%DAY:~0,1%" == " " SET DAY=0%DAY:~1,1%

SET DATETIMEf=%YEAR% %MONTH% %DAY% %HOUR%:%MIN%:%SECS%

CD /D ..\tools\Win-setup

rem --> everything above already exist <--
SET GENBUILDVER=0

SET AVAILVERSIONS=unknown
SET LASTVERSION=unknown
SET ALTVERSION=unknown

FOR /F "tokens=2*" %%i IN ('FINDSTR /c:"_LAST_VERSION" BuildVersions.txt') DO SET LASTVERSION=%%i
FOR /F "tokens=2*" %%i IN ('FINDSTR /c:"_ALT_VERSION" BuildVersions.txt') DO SET ALTVERSION=%%i
FOR /F "tokens=2*" %%i IN ('FINDSTR /c:"_AVAIL_VERSIONS" BuildVersions.txt') DO SET AVAILVERSIONS=%%i

SET lastVersionInsertFile=..\release\json\lastVersionInsert.txt
SET genLastVersionInsert=%lastVersionInsertFile% ECHO

:GENERATE lastVersionInsert.txt file
>%genLastVersionInsert%       "alternate-version-%LASTVERSION%": {
>>%genLastVersionInsert%         "open-url": "https://sourceforge.net/projects/lpub3d/files/%LASTVERSION%/",
>>%genLastVersionInsert%         "latest-version": "%LASTVERSION%",
>>%genLastVersionInsert%         "download-url": "http://lpub3d.sourceforge.net/LPub3D-UpdateMaster_%LASTVERSION%.exe",
>>%genLastVersionInsert%         "changelog-url": "http://lpub3d.sourceforge.net/change_log_%LASTVERSION%.txt"
>>%genLastVersionInsert%       },

rem //////////
SET updatesFile=..\release\json\lpub3dupdates.json
SET genLPub3DUpdates=%updatesFile% ECHO

:GENERATE lpub3dupdates.json version input file
>%genLPub3DUpdates% {
>>%genLPub3DUpdates%   "_comment": "LPub3D lpub3dupdates.json generated on %DATETIMEf%",
>>%genLPub3DUpdates%   "updates": {
>>%genLPub3DUpdates%     "windows": {
>>%genLPub3DUpdates%       "open-url": "https://sourceforge.net/projects/lpub3d/files/%VERSION%/",
>>%genLPub3DUpdates%       "latest-version": "%VERSION%",
>>%genLPub3DUpdates%       "download-url": "http://lpub3d.sourceforge.net/LPub3D-Update_%VERSION%.exe",
>>%genLPub3DUpdates%       "changelog-url": "http://lpub3d.sourceforge.net/change_log_%VERSION%.txt",
>>%genLPub3DUpdates%       "available-versions": "%VERSION%,%AVAILVERSIONS%",
>>%genLPub3DUpdates%       "alternate-version-1.3.5": {
>>%genLPub3DUpdates%         "open-url": "https://sourceforge.net/projects/lpub3d/files/1.3.5/",
>>%genLPub3DUpdates%      	 "latest-version": "1.3.5",
>>%genLPub3DUpdates%      	 "download-url": "http://lpub3d.sourceforge.net/LPub3D-UpdateMaster_1.3.5.exe",
>>%genLPub3DUpdates%      	 "changelog-url": "http://lpub3d.sourceforge.net/change_log_1.3.5.txt"
>>%genLPub3DUpdates%       },
>>%genLPub3DUpdates%       "alternate-version-1.2.3": {
>>%genLPub3DUpdates%         "open-url": "https://sourceforge.net/projects/lpub3d/files/1.2.3/",
>>%genLPub3DUpdates%      	 "latest-version": "1.2.3",
>>%genLPub3DUpdates%      	 "download-url": "http://lpub3d.sourceforge.net/LPub3D-UpdateMaster_1.2.3.exe",
>>%genLPub3DUpdates%      	 "changelog-url": "http://lpub3d.sourceforge.net/change_log_1.2.3.txt"
>>%genLPub3DUpdates%       },
>>%genLPub3DUpdates%       "alternate-version-1.0.0": {
>>%genLPub3DUpdates%         "open-url": "https://sourceforge.net/projects/lpub3d/files/1.0.0/",
>>%genLPub3DUpdates%      	 "latest-version": "1.0.0",
>>%genLPub3DUpdates%      	 "download-url": "http://lpub3d.sourceforge.net/LPub3D-UpdateMaster_1.0.0.exe",
>>%genLPub3DUpdates%      	 "changelog-url": "http://lpub3d.sourceforge.net/change_log_1.0.0.txt"
>>%genLPub3DUpdates%       }
>>%genLPub3DUpdates%     },
>>%genLPub3DUpdates%     "osx": {
>>%genLPub3DUpdates%       "open-url": "https://sourceforge.net/projects/lpub3d/files/%VERSION%/",
>>%genLPub3DUpdates%       "latest-version": "%VERSION%",
>>%genLPub3DUpdates%       "download-url": "http://lpub3d.sourceforge.net/LPub3D-Update_%VERSION%.exe",
>>%genLPub3DUpdates%       "changelog-url": "http://lpub3d.sourceforge.net/change_log_%VERSION%.txt",
>>%genLPub3DUpdates%       "available-versions": ""  
>>%genLPub3DUpdates%     },
>>%genLPub3DUpdates%     "linux": {
>>%genLPub3DUpdates%       "open-url": "https://sourceforge.net/projects/lpub3d/files/%VERSION%/",
>>%genLPub3DUpdates%       "latest-version": "%VERSION%",
>>%genLPub3DUpdates%       "download-url": "http://lpub3d.sourceforge.net/LPub3D-Update_%VERSION%.exe",
>>%genLPub3DUpdates%       "changelog-url": "http://lpub3d.sourceforge.net/change_log_%VERSION%.txt",
>>%genLPub3DUpdates%       "available-versions": ""
>>%genLPub3DUpdates%     }
>>%genLPub3DUpdates%   }
>>%genLPub3DUpdates% }
>>%genLPub3DUpdates%.

rem //////////

IF %GENBUILDVER% == 1 SET BuildVersions=.\BuildVersions.txt
IF %GENBUILDVER% == 1 SET genBuildVersions=%BuildVersions% ECHO

:GENERATE BuildVersions.txt file
IF %GENBUILDVER% == 1 >%genBuildVersions% _LAST_VERSION %VERSION%
IF %GENBUILDVER% == 1 >>%genBuildVersions% _ALT_VERSION %LASTVERSION%
IF %GENBUILDVER% == 1 >>%genBuildVersions% _AVAIL_VERSIONS %VERSION%,%AVAILVERSIONS%

rem //////////
@ECHO OFF
(
  FOR /F "tokens=*" %%i IN (..\release\json\lpub3dupdates.json) DO (
    IF "%%i" EQU ""alternate-version-%ALTVERSION%": {" (
      TYPE ..\release\json\lastVersionInsert.txt
    )
	ECHO %%i
  )
) >temp.txt
MOVE /y temp.txt ..\release\json\lpub3dupdates.json
DEL /Q ..\release\json\lastVersionInsert.txt

ECHO.
pause
ENDLOCAL