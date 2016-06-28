@ECHO off
Title Build and create manual and automatic LPub3D install distributions
rem --
rem  Trevor SANDY <trevor.sandy@gmail.com>
rem  Last Update: June, 28, 2016
rem  Copyright (c) 2016 by Trevor Sandy
rem --
SETLOCAL
@break off
@color 0a
ECHO Build and create manual and automatic install distributions

rem ---------------------------------------
rem Change value to 1 to execute and 0 to ignore the respective command
SET RUN_NSIS=1
SET CLEANUP=1
SET GENBUILDVER=1
SET UCRT=0
rem ---------------------------------------

ECHO. 							                						>  ..\release\LPub3D.Release.build.log.txt
IF %RUN_NSIS% == 0 ECHO - Start NSIS test build process...      		>>  ..\release\LPub3D.Release.build.log.txt
IF %RUN_NSIS% == 0 ECHO.
IF %RUN_NSIS% == 0 ECHO - Start NSIS test build process...

IF %RUN_NSIS% == 1 ECHO - Start build process...      			   		>>  ..\release\LPub3D.Release.build.log.txt
IF %RUN_NSIS% == 1 ECHO.
IF %RUN_NSIS% == 1 ECHO - Start build process...

SET DeploymentAbsolutePath=C:\Users\Trevor\Downloads\LEGO\LPub\project\LPub\LPub3D\tools
SET Win32LPub3DBuildFileAbsolutePath="C:\Users\Trevor\Downloads\LEGO\LPub\project\LPub\LPub3D\build\Win32\Release\LPub3D.exe"
SET Win64LPub3DBuildFileAbsolutePath="C:\Users\Trevor\Downloads\LEGO\LPub\project\LPub\LPub3D\build\x64\Release\LPub3D.exe"

SET Win32LPub3DBuildFile="..\..\build\Win32\Release\LPub3D.exe"
SET Win32QuazipBuildFile="..\..\build\Win32\Release\quazip.dll"
SET Win32LdrawiniBuildFile="..\..\build\Win32\Release\ldrawini.dll"
SET Win64LPub3DBuildFile="..\..\build\x64\Release\LPub3D.exe"
SET Win64QuazipBuildFile="..\..\build\x64\Release\quazip.dll"
SET Win64LdrawiniBuildFile="..\..\build\x64\Release\ldrawini.dll"

SET Win32QtBinPath=C:\Qt\IDE\5.6\msvc2015\bin
SET Win32QtPluginPath=C:\Qt\IDE\5.6\msvc2015\plugins
SET Win32DevKit10UCRTRedist=C:\Program Files (x86)\Windows Kits\10\redist\ucrt\DLLs\x86
SET Win64QtBinPath=C:\Qt\IDE\5.6\msvc2015_64\bin
SET Win64QtPluginPath=C:\Qt\IDE\5.6\msvc2015_64\plugins
SET Win64DevKit10UCRTRedist=C:\Program Files (x86)\Windows Kits\10\redist\ucrt\DLLs\x64

SET NSISPath=C:\Program Files (x86)\NSIS
SET VirtualBoxPath=C:\Program Files (x86)\Enigma Virtual Box
SET devRootPath="..\..\mainApp"

SET zipWin64=C:\program files\7-zip
SET zipWin32=C:\Program Files (x86)\7-zip

SET zipExe=unknown

ECHO. 							                	>>  ..\release\LPub3D.Release.build.log.txt
ECHO - Environment check...        			   		>>  ..\release\LPub3D.Release.build.log.txt
ECHO.
ECHO - Environment check...

IF EXIST "%zipWin64%" (
	ECHO.
	SET zipExe="%zipWin64%\7z.exe"
    ECHO - Found zip exe at "%zipWin64%"
    GOTO MAIN
) 

ECHO.

IF EXIST "%zipWin32%" (
	ECHO.
	SET zipExe="%zipWin32%\7z.exe"
    ECHO - Found zip exe at "%zipWin32%"
    GOTO MAIN
)

ECHO.
SET /p zipExe=Could not find any zip exectutable. You can manually enter a location: 
SET /p OPTION=Zip exe at (%zipExe%) will be used to archive your library. Hit [1] to exit or enter to continue:
IF  %OPTION% == 1  EXIT

:MAIN
SET BUILD=unknown
SET VER_SP=unknown
SET PRODUCT=unknown
SET VERSION=unknown
SET VER_MAJOR=unknown
SET VER_MINOR=unknown
SET REVISION_CMS=unknown
SET AVAILVERSIONS=unknown
SET LASTVERSION=unknown
SET ALTVERSION=unknown

SET COMPANY=unknown
SET COMMENTS=unknown
SET FILENAME=unknown
SET PUBLISHER=unknown
SET COMPANYURL=unknown
SET BUILD_DATE=unknown
SET REVISION_FILE=unknown
SET SUPPORT_EMAIL=unknown

ECHO. 							                	>>  ..\release\LPub3D.Release.build.log.txt
ECHO - Setting up release variables...          	>>  ..\release\LPub3D.Release.build.log.txt
ECHO.
ECHO - Setting up release variables...

cd /D %devRootPath%
FOR /F "tokens=2"  %%i IN ('svn info -r HEAD ^| FINDSTR Revision:') DO SET REVISION_CMS=%%i
FOR /F "tokens=3*" %%i IN ('FINDSTR /c:"#define _BUILD_NUMBER" build.h') DO SET BUILD=%%i
FOR /F "tokens=3*" %%i IN ('FINDSTR /c:"#define VER_PRODUCTNAME_STR" version.h') DO SET PRODUCT=%%i
FOR /F "tokens=3*" %%i IN ('FINDSTR /c:"#define VER_MAJOR" version.h') DO SET VER_MAJOR=%%i
FOR /F "tokens=3*" %%i IN ('FINDSTR /c:"#define VER_MINOR" version.h') DO SET VER_MINOR=%%i
FOR /F "tokens=3*" %%i IN ('FINDSTR /c:"#define VER_SP" version.h') DO SET VER_SP=%%i

FOR /F "tokens=3*" %%i IN ('FINDSTR /c:"#define VER_COMPANYNAME_STR" version.h') DO SET COMPANY=%%i %%j
FOR /F "tokens=3*" %%i IN ('FINDSTR /c:"#define VER_PRODUCTNAME_STR" version.h') DO SET PRODUCT=%%i
FOR /F "tokens=3*" %%i IN ('FINDSTR /c:"#define VER_ORIGINALFILENAME_STR" version.h') DO SET FILENAME=%%i
FOR /F "tokens=3*" %%i IN ('FINDSTR /c:"#define VER_PUBLISHER_STR" version.h') DO SET PUBLISHER=%%i %%j
FOR /F "tokens=3*" %%i IN ('FINDSTR /c:"#define VER_COMPANYDOMAIN_STR" version.h') DO SET COMPANYURL=%%i
FOR /F "tokens=3*" %%i IN ('FINDSTR /c:"#define VER_FILEDESCRIPTION_STR" version.h') DO SET COMMENTS=%%i %%j
FOR /F "tokens=3*" %%i IN ('FINDSTR /c:"#define VER_PUBLISHER_SUPPORT_EMAIL_STR" version.h') DO SET SUPPORT_EMAIL=%%i

SET BUILD=%BUILD:"=%
SET PRODUCT=%PRODUCT:"=%
SET YEAR=%DATE:~-4%
SET MONTH=%DATE:~3,2%
IF "%MONTH:~0,1%" == " " SET MONTH=0%MONTH:~1,1%
SET DAY=%DATE:~0,2%
IF "%DAY:~0,1%" == " " SET DAY=0%DAY:~1,1%

SET HOUR=%TIME:~0,2%
IF "%HOUR:~0,1%" == " " SET HOUR=0%HOUR:~1,1%
SET MIN=%TIME:~3,2%
IF "%MIN:~0,1%" == " " SET MIN=0%MIN:~1,1%
SET SECS=%TIME:~6,2%
IF "%SECS:~0,1%" == " " SET SECS=0%SECS:~1,1%

SET DATETIMEf=%YEAR% %MONTH% %DAY% %HOUR%:%MIN%:%SECS%

SET VERSION=%VER_MAJOR%.%VER_MINOR%.%VER_SP%
SET BUILDVERSION=%VERSION%.%REVISION_CMS%.%BUILD%
SET FULLVERSION=%VERSION%.%REVISION_CMS%.%BUILD%_%YEAR%%MONTH%%DAY%
SET DOWNLOADPRODUCT=%PRODUCT%-%FULLVERSION%

SET WIN32PRODUCTDIR=%PRODUCT%_x32-%FULLVERSION%
SET WIN64PRODUCTDIR=%PRODUCT%_x64-%FULLVERSION%
				
CD /D ..\tools\Win-setup

ECHO. 													 			>>  ../release/LPub3D.Release.build.log.txt
ECHO - Delete old media content and create new folders  			>>  ../release/LPub3D.Release.build.log.txt
ECHO. 	
ECHO - Delete old media content and create new folders...

IF EXIST "..\release\%VERSION%" (
  RD /Q /S ..\release\%VERSION%              						>>  ../release/LPub3D.Release.build.log.txt
)

IF EXIST "..\release\docs" (
  RD /Q /S ..\release\docs			                    			>>  ../release/LPub3D.Release.build.log.txt
)

IF EXIST "..\release\README.txt" (
  DEL /Q ..\release\README.txt										>>  ../release/LPub3D.Release.build.log.txt
)

IF NOT EXIST "..\release\%VERSION%\Download\" (
  MKDIR "..\release\%VERSION%\Download\"
)
IF NOT EXIST "..\release\%VERSION%\Update\" (
  MKDIR "..\release\%VERSION%\Update\"
)

ECHO. 													 			>>  ../release/LPub3D.Release.build.log.txt
ECHO - Copying change_log_%VERSION% to media folder...    			>>  ../release/LPub3D.Release.build.log.txt
ECHO. 	
ECHO - Copying change_log_%VERSION% to media folder...

SET file=README.txt
SET temp=temp.dat
SET targetln=1
SET productversion=%PRODUCT% %BUILDVERSION%
SET n=0

CD /D ..\docs\
RENAME %file% %temp%
FOR /F "tokens=*" %%i IN ('FINDSTR /n "^" "%temp%"') DO CALL :setversion %%i
DEL %temp%
CD /D ..\Win-setup\

COPY /V /Y ..\docs\README.txt ..\release\%VERSION%\Update\change_log_%VERSION%.txt /A							>>  ../release/LPub3D.Release.build.log.txt
COPY /V /Y ..\docs\README.txt ..\release\%VERSION%\Download\ /A													>>  ../release/LPub3D.Release.build.log.txt
COPY /V /Y ..\docs\README.txt ..\release\ /A																	>>  ../release/LPub3D.Release.build.log.txt
XCOPY /S /I /E /V /Y ..\docs\CREDITS.txt ..\release\docs\ /A													>>  ../release/LPub3D.Release.build.log.txt

ECHO. 																>>  ../release/LPub3D.Release.build.log.txt
ECHO - Generating lastVersionInsert.txt input file...				>>  ../release/LPub3D.Release.build.log.txt
ECHO. 	
ECHO - Generating lastVersionInsert.txt input file...

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

ECHO. 																>>  ../release/LPub3D.Release.build.log.txt
ECHO - Generating lpub3dupdates.json template file...				>>  ../release/LPub3D.Release.build.log.txt
ECHO. 	
ECHO - Generating lpub3dupdates.json template file...

SET updatesFile=..\release\json\lpub3dupdates.json
SET genLPub3DUpdates=%updatesFile% ECHO

:GENERATE lpub3dupdates.json template file
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

ECHO. 													 			>>  ../release/LPub3D.Release.build.log.txt
IF %GENBUILDVER% == 1 ECHO - Updating build versions input file... 	>>  ../release/LPub3D.Release.build.log.txt
IF %GENBUILDVER% == 1 ECHO. 	
IF %GENBUILDVER% == 1 ECHO - Upsdating build versions input file...

IF %GENBUILDVER% == 0 ECHO - Ignoring build versions input file... 	>>  ../release/LPub3D.Release.build.log.txt
IF %GENBUILDVER% == 0 ECHO. 	
IF %GENBUILDVER% == 0 ECHO - Ignoring build versions input file...

IF %GENBUILDVER% == 1 SET BuildVersions=.\BuildVersions.txt
IF %GENBUILDVER% == 1 SET genBuildVersions=%BuildVersions% ECHO

:GENERATE BuildVersions.txt file
IF %GENBUILDVER% == 1 >%genBuildVersions% _LAST_VERSION %VERSION%
IF %GENBUILDVER% == 1 >>%genBuildVersions% _ALT_VERSION %LASTVERSION%
IF %GENBUILDVER% == 1 >>%genBuildVersions% _AVAIL_VERSIONS %VERSION%,%AVAILVERSIONS%

ECHO. 																>>  ../release/LPub3D.Release.build.log.txt
ECHO - Generating lpub3dupdates.json version input file...			>>  ../release/LPub3D.Release.build.log.txt
ECHO. 	
ECHO - Generating lpub3dupdates.json version input file...

(
  FOR /F "tokens=*" %%i IN (..\release\json\lpub3dupdates.json) DO (
    IF "%%i" EQU ""alternate-version-%ALTVERSION%": {" (
      TYPE ..\release\json\lastVersionInsert.txt
    )
	ECHO %%i
  )
) >temp.txt
MOVE /y temp.txt ..\release\json\lpub3dupdates.json					>>  ../release/LPub3D.Release.build.log.txt
DEL /Q ..\release\json\lastVersionInsert.txt

ECHO. 													 			>>  ../release/LPub3D.Release.build.log.txt
ECHO - Copying lpub3dupdats.json to media folder...    				>>  ../release/LPub3D.Release.build.log.txt
ECHO. 	
ECHO - Copying lpub3dupdates.json to media folder...

COPY /V /Y ..\release\json\lpub3dupdates.json ..\release\%VERSION%\Update\ /A									>>  ../release/LPub3D.Release.build.log.txt

ECHO.                                   																		>>  ../release/LPub3D.Release.build.log.txt
ECHO - Generating latest.txt version input file (backgward compatability)...                                    >>  ../release/LPub3D.Release.build.log.txt
ECHO.   
ECHO - Generating latest.txt version input file (backgward compatability)...

SET latestFile=..\release\%VERSION%\Update\latest.txt
SET genLatest=%latestFile% ECHO

:GENERATE latest.txt file
>%genLatest% %VERSION% 

ECHO. 																>>  ../release/LPub3D.Release.build.log.txt
ECHO - Generating AppVersion.nsh build input script...   			>>  ../release/LPub3D.Release.build.log.txt
ECHO. 	
ECHO - Generating AppVersion.nsh build input script...

SET versionFile=.\AppVersion.nsh
SET genVersion=%versionFile% ECHO

:GENERATE AppVersion.nsh NSIS build input file
>%genVersion% !define Company %COMPANY% 
>>%genVersion% ; ${Company} 	
>>%genVersion%.	
>>%genVersion% !define ProductName "%PRODUCT%" 
>>%genVersion% ; ${ProductName}
>>%genVersion%.	
>>%genVersion% !define Version "%VERSION%"
>>%genVersion% ; ${Version}
>>%genVersion%.	
>>%genVersion% !define CompleteVersion "%VERSION%.%REVISION_CMS%.%BUILD%_%YEAR%%MONTH%%DAY%"
>>%genVersion% ; ${CompleteVersion}
>>%genVersion%.		
>>%genVersion% !define Win32BuildDir "..\release\%VERSION%\%WIN32PRODUCTDIR%\%PRODUCT%_x32" 
>>%genVersion% ; ${Win32BuildDir}
>>%genVersion%.
>>%genVersion% !define Win64BuildDir "..\release\%VERSION%\%WIN64PRODUCTDIR%\%PRODUCT%_x64" 
>>%genVersion% ; ${Win64BuildDir}
>>%genVersion%.
>>%genVersion% !define FileName %FILENAME%
>>%genVersion% ; ${FileName}
>>%genVersion%.	
>>%genVersion% !define BuildRevision "%REVISION_CMS%" 
>>%genVersion% ; ${BuildRevision}
>>%genVersion%.	
>>%genVersion% !define BuildNumber "%BUILD%" 
>>%genVersion% ; ${BuildNumber}
>>%genVersion%.	
>>%genVersion% !define BuildDate "%DATETIMEf%" 
>>%genVersion% ; ${BuildDate}
>>%genVersion%.	
>>%genVersion% !define Publisher %PUBLISHER% 
>>%genVersion% ; ${Publisher}
>>%genVersion%.	
>>%genVersion% !define CompanyURL %COMPANYURL% 
>>%genVersion% ; ${CompanyURL}
>>%genVersion%.
>>%genVersion% !define Comments %COMMENTS% 
>>%genVersion% ; ${Comments}
>>%genVersion%.
>>%genVersion% !define SupportEmail %SUPPORT_EMAIL% %VERSION%.%REVISION_CMS%.%BUILD%_%YEAR%%MONTH%%DAY%"
>>%genVersion% ; ${SupportEmail}
>>%genVersion%.

ECHO. 																>>  ../release/LPub3D.Release.build.log.txt
ECHO - Copying %WIN32PRODUCTDIR% content to media folder...    		>>  ../release/LPub3D.Release.build.log.txt
ECHO. 	
ECHO - Copying %WIN32PRODUCTDIR% content to media folder...

XCOPY /S /I /E /V /Y %devRootPath%\extras ..\release\%VERSION%\%WIN32PRODUCTDIR%\%PRODUCT%_x32\extras			>>  ../release/LPub3D.Release.build.log.txt
XCOPY /S /I /E /V /Y ..\release\libraries ..\release\%VERSION%\%WIN32PRODUCTDIR%\%PRODUCT%_x32\libraries		>>  ../release/LPub3D.Release.build.log.txt 
XCOPY /S /I /E /V /Y ..\release\3rdParty ..\release\%VERSION%\%WIN32PRODUCTDIR%\%PRODUCT%_x32\3rdParty			>>  ../release/LPub3D.Release.build.log.txt 
XCOPY /S /I /E /V /Y ..\docs ..\release\%VERSION%\%WIN32PRODUCTDIR%\%PRODUCT%_x32\docs							>>  ../release/LPub3D.Release.build.log.txt 
XCOPY /S /I /E /V /Y ..\icons ..\release\%VERSION%\%WIN32PRODUCTDIR%\%PRODUCT%_x32\icons						>>  ../release/LPub3D.Release.build.log.txt

COPY /V /Y ..\docs\README.txt ..\release\%VERSION%\%WIN32PRODUCTDIR%\%PRODUCT%_x32\ /A                          >>  ../release/LPub3D.Release.build.log.txt
COPY /V /Y ..\docs\README.txt ..\release\%VERSION%\%WIN32PRODUCTDIR%\ /A                                        >>  ../release/LPub3D.Release.build.log.txt

COPY /V /Y %Win32LPub3DBuildFile% ..\release\%VERSION%\%WIN32PRODUCTDIR%\%PRODUCT%_x32\%PRODUCT%_x32.exe /B		>>  ../release/LPub3D.Release.build.log.txt
COPY /V /Y %Win32QuazipBuildFile% ..\release\%VERSION%\%WIN32PRODUCTDIR%\%PRODUCT%_x32\ /B                      >>  ../release/LPub3D.Release.build.log.txt
COPY /V /Y %Win32LdrawiniBuildFile% ..\release\%VERSION%\%WIN32PRODUCTDIR%\%PRODUCT%_x32\ /B                    >>  ../release/LPub3D.Release.build.log.txt

COPY /V /Y %Win32QtBinPath%\Qt5Core.dll ..\release\%VERSION%\%WIN32PRODUCTDIR%\%PRODUCT%_x32\ /B                >>  ../release/LPub3D.Release.build.log.txt
COPY /V /Y %Win32QtBinPath%\Qt5Gui.dll ..\release\%VERSION%\%WIN32PRODUCTDIR%\%PRODUCT%_x32\ /B                 >>  ../release/LPub3D.Release.build.log.txt
COPY /V /Y %Win32QtBinPath%\Qt5Network.dll ..\release\%VERSION%\%WIN32PRODUCTDIR%\%PRODUCT%_x32\ /B             >>  ../release/LPub3D.Release.build.log.txt
COPY /V /Y %Win32QtBinPath%\Qt5OpenGL.dll ..\release\%VERSION%\%WIN32PRODUCTDIR%\%PRODUCT%_x32\ /B              >>  ../release/LPub3D.Release.build.log.txt
COPY /V /Y %Win32QtBinPath%\Qt5PrintSupport.dll ..\release\%VERSION%\%WIN32PRODUCTDIR%\%PRODUCT%_x32\ /B        >>  ../release/LPub3D.Release.build.log.txt
COPY /V /Y %Win32QtBinPath%\Qt5Widgets.dll ..\release\%VERSION%\%WIN32PRODUCTDIR%\%PRODUCT%_x32\ /B             >>  ../release/LPub3D.Release.build.log.txt

COPY /V /Y %Win32QtBinPath%\libGLESV2.dll ..\release\%VERSION%\%WIN32PRODUCTDIR%\%PRODUCT%_x32\ /B             	>>  ../release/LPub3D.Release.build.log.txt
COPY /V /Y %Win32QtBinPath%\libEGL.dll ..\release\%VERSION%\%WIN32PRODUCTDIR%\%PRODUCT%_x32\ /B             	>>  ../release/LPub3D.Release.build.log.txt
COPY /V /Y %Win32QtBinPath%\opengl32sw.dll ..\release\%VERSION%\%WIN32PRODUCTDIR%\%PRODUCT%_x32\ /B             >>  ../release/LPub3D.Release.build.log.txt
COPY /V /Y "C:\Program Files (x86)\Windows Kits\10\redist\D3D\x86\d3dcompiler_47.dll" ..\release\%VERSION%\%WIN32PRODUCTDIR%\%PRODUCT%_x32\ /B             		>>  ../release/LPub3D.Release.build.log.txt
COPY /V /Y "..\release\vcredist\vc_redist.x86.exe" ..\release\%VERSION%\%WIN32PRODUCTDIR%\%PRODUCT%_x32\ /B       >>  ../release/LPub3D.Release.build.log.txt

IF NOT EXIST "..\release\%VERSION%\%WIN32PRODUCTDIR%\%PRODUCT%_x32\bearer\" (
  MKDIR "..\release\%VERSION%\%WIN32PRODUCTDIR%\%PRODUCT%_x32\bearer\"
)
IF NOT EXIST "..\release\%VERSION%\%WIN32PRODUCTDIR%\%PRODUCT%_x32\iconengines\" (
  MKDIR "..\release\%VERSION%\%WIN32PRODUCTDIR%\%PRODUCT%_x32\iconengines\"
)
IF NOT EXIST "..\release\%VERSION%\%WIN32PRODUCTDIR%\%PRODUCT%_x32\imageformats\" (
  MKDIR "..\release\%VERSION%\%WIN32PRODUCTDIR%\%PRODUCT%_x32\imageformats\"
)
IF NOT EXIST "..\release\%VERSION%\%WIN32PRODUCTDIR%\%PRODUCT%_x32\platforms\" (
  MKDIR "..\release\%VERSION%\%WIN32PRODUCTDIR%\%PRODUCT%_x32\platforms\"
)
IF NOT EXIST "..\release\%VERSION%\%WIN32PRODUCTDIR%\%PRODUCT%_x32\printsupport\" (
  MKDIR "..\release\%VERSION%\%WIN32PRODUCTDIR%\%PRODUCT%_x32\printsupport\"
)

COPY /V /Y %Win32QtPluginPath%\bearer\qgenericbearer.dll ..\release\%VERSION%\%WIN32PRODUCTDIR%\%PRODUCT%_x32\bearer\ /B  						>>  ../release/LPub3D.Release.build.log.txt
COPY /V /Y %Win32QtPluginPath%\bearer\qnativewifibearer.dll ..\release\%VERSION%\%WIN32PRODUCTDIR%\%PRODUCT%_x32\bearer\ /B  					>>  ../release/LPub3D.Release.build.log.txt
COPY /V /Y %Win32QtPluginPath%\iconengines\qsvgicon.dll ..\release\%VERSION%\%WIN32PRODUCTDIR%\%PRODUCT%_x32\iconengines\ /B  					>>  ../release/LPub3D.Release.build.log.txt
COPY /V /Y %Win32QtPluginPath%\imageformats\qdds.dll ..\release\%VERSION%\%WIN32PRODUCTDIR%\%PRODUCT%_x32\imageformats\ /B  					>>  ../release/LPub3D.Release.build.log.txt
COPY /V /Y %Win32QtPluginPath%\imageformats\qgif.dll ..\release\%VERSION%\%WIN32PRODUCTDIR%\%PRODUCT%_x32\imageformats\ /B  					>>  ../release/LPub3D.Release.build.log.txt
COPY /V /Y %Win32QtPluginPath%\imageformats\qicns.dll ..\release\%VERSION%\%WIN32PRODUCTDIR%\%PRODUCT%_x32\imageformats\ /B  					>>  ../release/LPub3D.Release.build.log.txt
COPY /V /Y %Win32QtPluginPath%\imageformats\qico.dll ..\release\%VERSION%\%WIN32PRODUCTDIR%\%PRODUCT%_x32\imageformats\ /B  					>>  ../release/LPub3D.Release.build.log.txt
COPY /V /Y %Win32QtPluginPath%\imageformats\qjpeg.dll ..\release\%VERSION%\%WIN32PRODUCTDIR%\%PRODUCT%_x32\imageformats\ /B  					>>  ../release/LPub3D.Release.build.log.txt
COPY /V /Y %Win32QtPluginPath%\imageformats\qsvg.dll ..\release\%VERSION%\%WIN32PRODUCTDIR%\%PRODUCT%_x32\imageformats\ /B  					>>  ../release/LPub3D.Release.build.log.txt
COPY /V /Y %Win32QtPluginPath%\imageformats\qtga.dll ..\release\%VERSION%\%WIN32PRODUCTDIR%\%PRODUCT%_x32\imageformats\ /B  					>>  ../release/LPub3D.Release.build.log.txt
COPY /V /Y %Win32QtPluginPath%\imageformats\qtiff.dll ..\release\%VERSION%\%WIN32PRODUCTDIR%\%PRODUCT%_x32\imageformats\ /B  					>>  ../release/LPub3D.Release.build.log.txt
COPY /V /Y %Win32QtPluginPath%\imageformats\qwbmp.dll ..\release\%VERSION%\%WIN32PRODUCTDIR%\%PRODUCT%_x32\imageformats\ /B  					>>  ../release/LPub3D.Release.build.log.txt
COPY /V /Y %Win32QtPluginPath%\imageformats\qwebp.dll ..\release\%VERSION%\%WIN32PRODUCTDIR%\%PRODUCT%_x32\imageformats\ /B  					>>  ../release/LPub3D.Release.build.log.txt
COPY /V /Y %Win32QtPluginPath%\platforms\qwindows.dll ..\release\%VERSION%\%WIN32PRODUCTDIR%\%PRODUCT%_x32\platforms\ /B  						>>  ../release/LPub3D.Release.build.log.txt
COPY /V /Y %Win32QtPluginPath%\printsupport\windowsprintersupport.dll ..\release\%VERSION%\%WIN32PRODUCTDIR%\%PRODUCT%_x32\printsupport\ /B  	>>  ../release/LPub3D.Release.build.log.txt

ECHO. 													>>  ../release/LPub3D.Release.build.log.txt
IF %UCRT% == 1 ECHO - Adding x86 UCRT Dlls...    		>>  ../release/LPub3D.Release.build.log.txt
IF %UCRT% == 1 ECHO. 	
IF %UCRT% == 1 ECHO - Adding x86 UCRT Dlls...

IF %UCRT% == 0 ECHO - Ignoring x86 UCRT Dlls...    		>>  ../release/LPub3D.Release.build.log.txt
IF %UCRT% == 0 ECHO. 	
IF %UCRT% == 0 ECHO - Ignoring x86 UCRT Dlls...

IF %UCRT% == 1 COPY /V /Y "%Win32DevKit10UCRTRedist%\API-MS-WIN-CRT-FILESYSTEM-L1-1-0.DLL" ..\release\%VERSION%\%WIN32PRODUCTDIR%\%PRODUCT%_x32\ /B             >>  ../release/LPub3D.Release.build.log.txt
IF %UCRT% == 1 COPY /V /Y "%Win32DevKit10UCRTRedist%\API-MS-WIN-CRT-STRING-L1-1-0.DLL" ..\release\%VERSION%\%WIN32PRODUCTDIR%\%PRODUCT%_x32\ /B             	>>  ../release/LPub3D.Release.build.log.txt
IF %UCRT% == 1 COPY /V /Y "%Win32DevKit10UCRTRedist%\API-MS-WIN-CRT-STDIO-L1-1-0.DLL" ..\release\%VERSION%\%WIN32PRODUCTDIR%\%PRODUCT%_x32\ /B             		>>  ../release/LPub3D.Release.build.log.txt
IF %UCRT% == 1 COPY /V /Y "%Win32DevKit10UCRTRedist%\API-MS-WIN-CRT-MATH-L1-1-0.DLL" ..\release\%VERSION%\%WIN32PRODUCTDIR%\%PRODUCT%_x32\ /B             		>>  ../release/LPub3D.Release.build.log.txt
IF %UCRT% == 1 COPY /V /Y "%Win32DevKit10UCRTRedist%\API-MS-WIN-CRT-RUNTIME-L1-1-0.DLL" ..\release\%VERSION%\%WIN32PRODUCTDIR%\%PRODUCT%_x32\ /B             	>>  ../release/LPub3D.Release.build.log.txt
IF %UCRT% == 1 COPY /V /Y "%Win32DevKit10UCRTRedist%\API-MS-WIN-CRT-HEAP-L1-1-0.DLL" ..\release\%VERSION%\%WIN32PRODUCTDIR%\%PRODUCT%_x32\ /B             		>>  ../release/LPub3D.Release.build.log.txt
IF %UCRT% == 1 COPY /V /Y "%Win32DevKit10UCRTRedist%\API-MS-WIN-CRT-ENVIRONMENT-L1-1-0.DLL" ..\release\%VERSION%\%WIN32PRODUCTDIR%\%PRODUCT%_x32\ /B            >>  ../release/LPub3D.Release.build.log.txt
IF %UCRT% == 1 COPY /V /Y "%Win32DevKit10UCRTRedist%\API-MS-WIN-CRT-CONVERT-L1-1-0.DLL" ..\release\%VERSION%\%WIN32PRODUCTDIR%\%PRODUCT%_x32\ /B             	>>  ../release/LPub3D.Release.build.log.txt
IF %UCRT% == 1 COPY /V /Y "%Win32DevKit10UCRTRedist%\API-MS-WIN-CRT-UTILITY-L1-1-0.DLL" ..\release\%VERSION%\%WIN32PRODUCTDIR%\%PRODUCT%_x32\ /B             	>>  ../release/LPub3D.Release.build.log.txt
IF %UCRT% == 1 COPY /V /Y "%Win32DevKit10UCRTRedist%\API-MS-WIN-CRT-LOCALE-L1-1-0.DLL" ..\release\%VERSION%\%WIN32PRODUCTDIR%\%PRODUCT%_x32\ /B             	>>  ../release/LPub3D.Release.build.log.txt

ECHO. 																>>  ../release/LPub3D.Release.build.log.txt
ECHO - Copying %WIN64PRODUCTDIR% content to media folder...    		>>  ../release/LPub3D.Release.build.log.txt
ECHO. 	
ECHO - Copying %WIN64PRODUCTDIR% content to media folder...

XCOPY /S /I /E /V /Y %devRootPath%\extras ..\release\%VERSION%\%WIN64PRODUCTDIR%\%PRODUCT%_x64\extras			>>  ../release/LPub3D.Release.build.log.txt
XCOPY /S /I /E /V /Y ..\release\libraries ..\release\%VERSION%\%WIN64PRODUCTDIR%\%PRODUCT%_x64\libraries		>>  ../release/LPub3D.Release.build.log.txt
XCOPY /S /I /E /V /Y ..\release\3rdParty ..\release\%VERSION%\%WIN64PRODUCTDIR%\%PRODUCT%_x64\3rdParty			>>  ../release/LPub3D.Release.build.log.txt
XCOPY /S /I /E /V /Y ..\docs ..\release\%VERSION%\%WIN64PRODUCTDIR%\%PRODUCT%_x64\docs							>>  ../release/LPub3D.Release.build.log.txt 
XCOPY /S /I /E /V /Y ..\icons ..\release\%VERSION%\%WIN64PRODUCTDIR%\%PRODUCT%_x64\icons						>>  ../release/LPub3D.Release.build.log.txt

COPY /V /Y ..\docs\README.txt ..\release\%VERSION%\%WIN64PRODUCTDIR%\%PRODUCT%_x64\ /A							>>  ../release/LPub3D.Release.build.log.txt
COPY /V /Y ..\docs\README.txt ..\release\%VERSION%\%WIN64PRODUCTDIR% /A											>>  ../release/LPub3D.Release.build.log.txt

COPY /V /Y %Win64LPub3DBuildFile% ..\release\%VERSION%\%WIN64PRODUCTDIR%\%PRODUCT%_x64\%PRODUCT%_x64.exe /B		>>  ../release/LPub3D.Release.build.log.txt
COPY /V /Y %Win64QuazipBuildFile% ..\release\%VERSION%\%WIN64PRODUCTDIR%\%PRODUCT%_x64\ /B						>>  ../release/LPub3D.Release.build.log.txt
COPY /V /Y %Win64LdrawiniBuildFile% ..\release\%VERSION%\%WIN64PRODUCTDIR%\%PRODUCT%_x64\ /B					>>  ../release/LPub3D.Release.build.log.txt

COPY /V /Y %Win64QtBinPath%\Qt5Core.dll ..\release\%VERSION%\%WIN64PRODUCTDIR%\%PRODUCT%_x64\ /B				>>  ../release/LPub3D.Release.build.log.txt
COPY /V /Y %Win64QtBinPath%\Qt5Gui.dll ..\release\%VERSION%\%WIN64PRODUCTDIR%\%PRODUCT%_x64\ /B					>>  ../release/LPub3D.Release.build.log.txt
COPY /V /Y %Win64QtBinPath%\Qt5Network.dll ..\release\%VERSION%\%WIN64PRODUCTDIR%\%PRODUCT%_x64\ /B				>>  ../release/LPub3D.Release.build.log.txt
COPY /V /Y %Win64QtBinPath%\Qt5OpenGL.dll ..\release\%VERSION%\%WIN64PRODUCTDIR%\%PRODUCT%_x64\ /B				>>  ../release/LPub3D.Release.build.log.txt
COPY /V /Y %Win64QtBinPath%\Qt5PrintSupport.dll ..\release\%VERSION%\%WIN64PRODUCTDIR%\%PRODUCT%_x64\ /B		>>  ../release/LPub3D.Release.build.log.txt
COPY /V /Y %Win64QtBinPath%\Qt5Widgets.dll ..\release\%VERSION%\%WIN64PRODUCTDIR%\%PRODUCT%_x64\ /B				>>  ../release/LPub3D.Release.build.log.txt

COPY /V /Y %Win64QtBinPath%\libGLESV2.dll ..\release\%VERSION%\%WIN64PRODUCTDIR%\%PRODUCT%_x64\ /B             	>>  ../release/LPub3D.Release.build.log.txt
COPY /V /Y %Win64QtBinPath%\libEGL.dll ..\release\%VERSION%\%WIN64PRODUCTDIR%\%PRODUCT%_x64\ /B             	>>  ../release/LPub3D.Release.build.log.txt
COPY /V /Y %Win64QtBinPath%\opengl32sw.dll ..\release\%VERSION%\%WIN64PRODUCTDIR%\%PRODUCT%_x64\ /B             >>  ../release/LPub3D.Release.build.log.txt
COPY /V /Y "C:\Program Files (x86)\Windows Kits\10\redist\D3D\x64\d3dcompiler_47.dll" ..\release\%VERSION%\%WIN64PRODUCTDIR%\%PRODUCT%_x64\ /B             	>>  ../release/LPub3D.Release.build.log.txt
COPY /V /Y "..\release\vcredist\vc_redist.x64.exe" ..\release\%VERSION%\%WIN64PRODUCTDIR%\%PRODUCT%_x64\ /B   	>>  ../release/LPub3D.Release.build.log.txt

IF NOT EXIST "..\release\%VERSION%\%WIN64PRODUCTDIR%\%PRODUCT%_x64\bearer\" (
  MKDIR "..\release\%VERSION%\%WIN64PRODUCTDIR%\%PRODUCT%_x64\bearer\"
)
IF NOT EXIST "..\release\%VERSION%\%WIN64PRODUCTDIR%\%PRODUCT%_x64\iconengines\" (
  MKDIR "..\release\%VERSION%\%WIN64PRODUCTDIR%\%PRODUCT%_x64\iconengines\"
)
IF NOT EXIST "..\release\%VERSION%\%WIN64PRODUCTDIR%\%PRODUCT%_x64\imageformats\" (
  MKDIR "..\release\%VERSION%\%WIN64PRODUCTDIR%\%PRODUCT%_x64\imageformats\"
)
IF NOT EXIST "..\release\%VERSION%\%WIN64PRODUCTDIR%\%PRODUCT%_x64\platforms\" (
  MKDIR "..\release\%VERSION%\%WIN64PRODUCTDIR%\%PRODUCT%_x64\platforms\"
)
IF NOT EXIST "..\release\%VERSION%\%WIN64PRODUCTDIR%\%PRODUCT%_x64\printsupport\" (
  MKDIR "..\release\%VERSION%\%WIN64PRODUCTDIR%\%PRODUCT%_x64\printsupport\"
)
COPY /V /Y %Win64QtPluginPath%\bearer\qgenericbearer.dll ..\release\%VERSION%\%WIN64PRODUCTDIR%\%PRODUCT%_x64\bearer\ /B						>>  ../release/LPub3D.Release.build.log.txt
COPY /V /Y %Win64QtPluginPath%\bearer\qnativewifibearer.dll ..\release\%VERSION%\%WIN64PRODUCTDIR%\%PRODUCT%_x64\bearer\ /B						>>  ../release/LPub3D.Release.build.log.txt
COPY /V /Y %Win64QtPluginPath%\iconengines\qsvgicon.dll ..\release\%VERSION%\%WIN64PRODUCTDIR%\%PRODUCT%_x64\iconengines\ /B					>>  ../release/LPub3D.Release.build.log.txt
COPY /V /Y %Win64QtPluginPath%\imageformats\qdds.dll ..\release\%VERSION%\%WIN64PRODUCTDIR%\%PRODUCT%_x64\imageformats\ /B						>>  ../release/LPub3D.Release.build.log.txt
COPY /V /Y %Win64QtPluginPath%\imageformats\qgif.dll ..\release\%VERSION%\%WIN64PRODUCTDIR%\%PRODUCT%_x64\imageformats\ /B						>>  ../release/LPub3D.Release.build.log.txt
COPY /V /Y %Win64QtPluginPath%\imageformats\qicns.dll ..\release\%VERSION%\%WIN64PRODUCTDIR%\%PRODUCT%_x64\imageformats\ /B						>>  ../release/LPub3D.Release.build.log.txt
COPY /V /Y %Win64QtPluginPath%\imageformats\qico.dll ..\release\%VERSION%\%WIN64PRODUCTDIR%\%PRODUCT%_x64\imageformats\ /B						>>  ../release/LPub3D.Release.build.log.txt
COPY /V /Y %Win64QtPluginPath%\imageformats\qjpeg.dll ..\release\%VERSION%\%WIN64PRODUCTDIR%\%PRODUCT%_x64\imageformats\ /B						>>  ../release/LPub3D.Release.build.log.txt
COPY /V /Y %Win64QtPluginPath%\imageformats\qsvg.dll ..\release\%VERSION%\%WIN64PRODUCTDIR%\%PRODUCT%_x64\imageformats\ /B						>>  ../release/LPub3D.Release.build.log.txt
COPY /V /Y %Win64QtPluginPath%\imageformats\qtga.dll ..\release\%VERSION%\%WIN64PRODUCTDIR%\%PRODUCT%_x64\imageformats\ /B						>>  ../release/LPub3D.Release.build.log.txt
COPY /V /Y %Win64QtPluginPath%\imageformats\qtiff.dll ..\release\%VERSION%\%WIN64PRODUCTDIR%\%PRODUCT%_x64\imageformats\ /B						>>  ../release/LPub3D.Release.build.log.txt
COPY /V /Y %Win64QtPluginPath%\imageformats\qwbmp.dll ..\release\%VERSION%\%WIN64PRODUCTDIR%\%PRODUCT%_x64\imageformats\ /B						>>  ../release/LPub3D.Release.build.log.txt
COPY /V /Y %Win64QtPluginPath%\imageformats\qwebp.dll ..\release\%VERSION%\%WIN64PRODUCTDIR%\%PRODUCT%_x64\imageformats\ /B						>>  ../release/LPub3D.Release.build.log.txt
COPY /V /Y %Win64QtPluginPath%\platforms\qwindows.dll ..\release\%VERSION%\%WIN64PRODUCTDIR%\%PRODUCT%_x64\platforms\ /B						>>  ../release/LPub3D.Release.build.log.txt
COPY /V /Y %Win64QtPluginPath%\printsupport\windowsprintersupport.dll ..\release\%VERSION%\%WIN64PRODUCTDIR%\%PRODUCT%_x64\printsupport\ /B		>>  ../release/LPub3D.Release.build.log.txt

ECHO. 													>>  ../release/LPub3D.Release.build.log.txt
IF %UCRT% == 1 ECHO - Adding x64 UCRT Dlls...    		>>  ../release/LPub3D.Release.build.log.txt
IF %UCRT% == 1 ECHO. 	
IF %UCRT% == 1 ECHO - Adding x64  UCRT Dlls...			

IF %UCRT% == 0 ECHO - Ignoring x64 UCRT Dlls...    		>>  ../release/LPub3D.Release.build.log.txt
IF %UCRT% == 0 ECHO. 	
IF %UCRT% == 0 ECHO - Ignoring x64 UCRT Dlls...

IF %UCRT% == 1 COPY /V /Y "%Win64DevKit10UCRTRedist%\API-MS-WIN-CRT-FILESYSTEM-L1-1-0.DLL" ..\release\%VERSION%\%WIN64PRODUCTDIR%\%PRODUCT%_x64\ /B         >>  ../release/LPub3D.Release.build.log.txt
IF %UCRT% == 1 COPY /V /Y "%Win64DevKit10UCRTRedist%\API-MS-WIN-CRT-STRING-L1-1-0.DLL" ..\release\%VERSION%\%WIN64PRODUCTDIR%\%PRODUCT%_x64\ /B             >>  ../release/LPub3D.Release.build.log.txt
IF %UCRT% == 1 COPY /V /Y "%Win64DevKit10UCRTRedist%\API-MS-WIN-CRT-STDIO-L1-1-0.DLL" ..\release\%VERSION%\%WIN64PRODUCTDIR%\%PRODUCT%_x64\ /B             	>>  ../release/LPub3D.Release.build.log.txt
IF %UCRT% == 1 COPY /V /Y "%Win64DevKit10UCRTRedist%\API-MS-WIN-CRT-MATH-L1-1-0.DLL" ..\release\%VERSION%\%WIN64PRODUCTDIR%\%PRODUCT%_x64\ /B             	>>  ../release/LPub3D.Release.build.log.txt
IF %UCRT% == 1 COPY /V /Y "%Win64DevKit10UCRTRedist%\API-MS-WIN-CRT-RUNTIME-L1-1-0.DLL" ..\release\%VERSION%\%WIN64PRODUCTDIR%\%PRODUCT%_x64\ /B            >>  ../release/LPub3D.Release.build.log.txt
IF %UCRT% == 1 COPY /V /Y "%Win64DevKit10UCRTRedist%\API-MS-WIN-CRT-HEAP-L1-1-0.DLL" ..\release\%VERSION%\%WIN64PRODUCTDIR%\%PRODUCT%_x64\ /B             	>>  ../release/LPub3D.Release.build.log.txt
IF %UCRT% == 1 COPY /V /Y "%Win64DevKit10UCRTRedist%\API-MS-WIN-CRT-ENVIRONMENT-L1-1-0.DLL" ..\release\%VERSION%\%WIN64PRODUCTDIR%\%PRODUCT%_x64\ /B        >>  ../release/LPub3D.Release.build.log.txt
IF %UCRT% == 1 COPY /V /Y "%Win64DevKit10UCRTRedist%\API-MS-WIN-CRT-CONVERT-L1-1-0.DLL" ..\release\%VERSION%\%WIN64PRODUCTDIR%\%PRODUCT%_x64\ /B            >>  ../release/LPub3D.Release.build.log.txt
IF %UCRT% == 1 COPY /V /Y "%Win64DevKit10UCRTRedist%\API-MS-WIN-CRT-UTILITY-L1-1-0.DLL" ..\release\%VERSION%\%WIN64PRODUCTDIR%\%PRODUCT%_x64\ /B            >>  ../release/LPub3D.Release.build.log.txt
IF %UCRT% == 1 COPY /V /Y "%Win64DevKit10UCRTRedist%\API-MS-WIN-CRT-LOCALE-L1-1-0.DLL" ..\release\%VERSION%\%WIN64PRODUCTDIR%\%PRODUCT%_x64\ /B             >>  ../release/LPub3D.Release.build.log.txt

ECHO. 															>>  ../release/LPub3D.Release.build.log.txt
ECHO - Finished copying content to media folder...     			>>  ../release/LPub3D.Release.build.log.txt
ECHO. 
ECHO - Finished copying content to media folder...

IF %RUN_NSIS% == 1 ECHO. 								        >>  ..\release\LPub3D.Release.build.log.txt
IF %RUN_NSIS% == 1 ECHO - Start NSIS Master Update Build...  	>>  ..\release\LPub3D.Release.build.log.txt
IF %RUN_NSIS% == 1 ECHO.
IF %RUN_NSIS% == 1 ECHO - Start NSIS Master Update Build...

IF %RUN_NSIS% == 0 ECHO - Ignore NSIS Master Update Build...  	>>  ..\release\LPub3D.Release.build.log.txt
IF %RUN_NSIS% == 0 ECHO.
IF %RUN_NSIS% == 0 ECHO - Ignore NSIS Master Update Build...

IF %RUN_NSIS% == 1 "%NSISPath%\makensis.exe" /DUpdateMaster LPub3DNoPack.nsi 		>> ..\release\LPub3D.Release.build.log.txt

IF %RUN_NSIS% == 1 ECHO. 										>>  ..\release\LPub3D.Release.build.log.txt
IF %RUN_NSIS% == 1 ECHO - Finished NSIS Master Update 	Build...>>  ..\release\LPub3D.Release.build.log.txt
IF %RUN_NSIS% == 1 ECHO.
IF %RUN_NSIS% == 1 ECHO - Finished NSIS Master Update  Build...

ECHO. 									   						>>  ..\release\LPub3D.Release.build.log.txt
IF %RUN_NSIS% == 1 ECHO - Start NSIS Manual Install Build... 	>>  ..\release\LPub3D.Release.build.log.txt
IF %RUN_NSIS% == 1 ECHO.
IF %RUN_NSIS% == 1 ECHO - Start NSIS Manual Install Build...

IF %RUN_NSIS% == 0 ECHO - Ignore NSIS Manual Install Build... 	>>  ..\release\LPub3D.Release.build.log.txt
IF %RUN_NSIS% == 0 ECHO.
IF %RUN_NSIS% == 0 ECHO - Ignore NSIS Manual Install Build...

rem IF %RUN_NSIS% == 1 "%NSISPath%\makensis.exe" LPub3DNoPack.nsi 						>> ..\release\LPub3D.Release.build.log.txt
IF %RUN_NSIS% == 1 COPY /V /Y ..\release\%DOWNLOADPRODUCT%.exe ..\release\%PRODUCT%-UpdateMaster_%VERSION%.exe 							>>  ../release/LPub3D.Release.build.log.txt

IF %RUN_NSIS% == 1 ECHO. 										>>  ..\release\LPub3D.Release.build.log.txt
IF %RUN_NSIS% == 1 ECHO - Finished NSIS Manual Install Build... >>  ..\release\LPub3D.Release.build.log.txt
IF %RUN_NSIS% == 1 ECHO.
IF %RUN_NSIS% == 1 ECHO - Finished NSIS Manual Install Build...

IF %RUN_NSIS% == 1 ECHO. 															>>  ../release/LPub3D.Release.build.log.txt
IF %RUN_NSIS% == 1 ECHO - Creating portable media zip files...		      			>>  ../release/LPub3D.Release.build.log.txt
IF %RUN_NSIS% == 1 ECHO. 	
IF %RUN_NSIS% == 1 ECHO - Creating portable media zip files...

IF %RUN_NSIS% == 1 %zipExe% a -tzip ..\release\%VERSION%\Download\%WIN32PRODUCTDIR%.zip ..\release\%VERSION%\%WIN32PRODUCTDIR%\ 		>>  ../release/LPub3D.Release.build.log.txt
IF %RUN_NSIS% == 1 %zipExe% a -tzip ..\release\%VERSION%\Download\%WIN64PRODUCTDIR%.zip ..\release\%VERSION%\%WIN64PRODUCTDIR%\ 		>>  ../release/LPub3D.Release.build.log.txt

ECHO. 																				>>  ../release/LPub3D.Release.build.log.txt
IF %CLEANUP% == 1 ECHO - Remove %PRODUCT% %VERSION% build files...					>>  ../release/LPub3D.Release.build.log.txt
IF %CLEANUP% == 1 ECHO. 	
IF %CLEANUP% == 1 ECHO - Remove %PRODUCT% %VERSION% build files...

IF %CLEANUP% == 0 ECHO - Ignore remove %PRODUCT% %VERSION% build files...			>>  ../release/LPub3D.Release.build.log.txt
IF %CLEANUP% == 0 ECHO. 	
IF %CLEANUP% == 0 ECHO - Ignore remove %PRODUCT% %VERSION% build files...

IF %CLEANUP% == 1 RD /Q /S ..\release\%VERSION%\%WIN32PRODUCTDIR%\ ..\release\%VERSION%\%WIN64PRODUCTDIR%\ 								>>  ../release/LPub3D.Release.build.log.txt

IF %RUN_NSIS% == 1 ECHO.												 																>>  ../release/LPub3D.Release.build.log.txt
IF %RUN_NSIS% == 1 ECHO - Moving NSIS-generated %DOWNLOADPRODUCT%.exe to Download\ media folder...										>>  ../release/LPub3D.Release.build.log.txt
IF %RUN_NSIS% == 1 ECHO. 																												>>  ../release/LPub3D.Release.build.log.txt
IF %RUN_NSIS% == 1 ECHO - Moving NSIS-generated %PRODUCT%-UpdateMaster_%VERSION%.exe to Update\ media folder...							>>  ../release/LPub3D.Release.build.log.txt
IF %RUN_NSIS% == 1 ECHO.	
IF %RUN_NSIS% == 1 ECHO - Moving NSIS-generated %DOWNLOADPRODUCT%.exe to Download\ media folder...		
IF %RUN_NSIS% == 1 ECHO.	
IF %RUN_NSIS% == 1 ECHO - Moving NSIS-generated %PRODUCT%-UpdateMaster_%VERSION%.exe to Update\ media folder...	

IF %RUN_NSIS% == 0 ECHO - Ignoring NSIS-generated %DOWNLOADPRODUCT%.exe to Download\ media folder...									>>  ../release/LPub3D.Release.build.log.txt
IF %RUN_NSIS% == 0 ECHO. 																												>>  ../release/LPub3D.Release.build.log.txt
IF %RUN_NSIS% == 0 ECHO - Ignoring NSIS-generated %PRODUCT%-UpdateMaster_%VERSION%.exe to Update\ media folder...						>>  ../release/LPub3D.Release.build.log.txt
IF %RUN_NSIS% == 0 ECHO. 	
IF %RUN_NSIS% == 0 ECHO - Ignoring NSIS-generated %DOWNLOADPRODUCT%.exe to Download\ media folder...	
IF %RUN_NSIS% == 0 ECHO. 			
IF %RUN_NSIS% == 0 ECHO - Ignoring NSIS-generated %PRODUCT%-UpdateMaster_%VERSION%.exe to Update\ media folder...	

IF %RUN_NSIS% == 1 MOVE /Y ..\release\%DOWNLOADPRODUCT%.exe ..\release\%VERSION%\Download\												>>  ../release/LPub3D.Release.build.log.txt		
IF %RUN_NSIS% == 1 MOVE /Y ..\release\%PRODUCT%-UpdateMaster_%VERSION%.exe ..\release\%VERSION%\Update\									>>  ../release/LPub3D.Release.build.log.txt		

ECHO. 																>>  ../release/LPub3D.Release.build.log.txt
ECHO - Finished.													>>  ../release/LPub3D.Release.build.log.txt
ECHO. 	
ECHO - Finished.

ECHO.
ECHO If everything went well Press any key to EXIT!
%SystemRoot%\explorer.exe "C:\Users\Trevor\Downloads\LEGO\LPub\project\LPub\LPub3D\tools\release\%VERSION%"
PAUSE >NUL
ENDLOCAL
EXIT /b 0

:setversion
SET /a n+=1
SET "ln=%*"
SETLOCAL enableDelayedExpansion
SET "ln=!ln:*:=!"
IF %n%==%targetln% (ECHO %productversion% >> %file%) ELSE ECHO(!ln! >> %file%
REM IF %n%==%targetln% (ECHO %productversion% >> %file%) ELSE ECHO %* >> %file%
ENDLOCAL
EXIT /b 0
