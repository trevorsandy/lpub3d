@ECHO off
Title Build and create manual and automatic LPub3D install distributions
rem --
rem  Trevor SANDY <trevor.sandy@gmail.com>
rem  Last Update: June, 17, 2016
rem  Copyright (c) 2016 by Trevor Sandy
rem --
SETLOCAL
@break off
@color 0a
ECHO Build and create manual and automatic install distributions

rem Change COMMAND to 'run' to execute and 'bypass' to disable NSIS script call and file delete (used for testing)
SET COMMAND=run

ECHO. 							                	>  ..\release\LPub3D.Release.build.log.txt
ECHO - Start build process...      			   		>>  ..\release\LPub3D.Release.build.log.txt
ECHO.
ECHO - Start build process...

SET Win32LPub3DBuildFile="..\..\build\Win32\Release\LPub3D.exe"
SET Win32QuazipBuildFile="..\..\build\Win32\Release\quazip.dll"
SET Win32LdrawiniBuildFile="..\..\build\Win32\Release\ldrawini.dll"
SET Win64LPub3DBuildFile="..\..\build\x64\Release\LPub3D.exe"
SET Win64QuazipBuildFile="..\..\build\x64\Release\quazip.dll"
SET Win64LdrawiniBuildFile="..\..\build\x64\Release\ldrawini.dll"

SET Win32QtPath="C:\Qt\IDE\5.6\msvc2015\bin"
SET Win64QtPath="C:\Qt\IDE\5.6\msvc2015_64\bin"

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

ECHO. 													 		>>  ../release/LPub3D.Release.build.log.txt
ECHO - Delete old media content and create new folders  		>>  ../release/LPub3D.Release.build.log.txt
ECHO. 	
ECHO - Delete old media content and create new folders...

IF EXIST "..\release\%VERSION%" (
  RD /Q /S ..\release\%VERSION%              					>>  ../release/LPub3D.Release.build.log.txt
)

IF EXIST "..\release\docs" (
  RD /Q /S ..\release\docs			                    		>>  ../release/LPub3D.Release.build.log.txt
)

IF EXIST "..\release\README.txt" (
  DEL /Q ..\release\README.txt									>>  ../release/LPub3D.Release.build.log.txt
)

IF NOT EXIST "..\release\%VERSION%\Download\" (
  MKDIR "..\release\%VERSION%\Download\"
)
IF NOT EXIST "..\release\%VERSION%\Update\" (
  MKDIR "..\release\%VERSION%\Update\"
)

ECHO. 													 		>>  ../release/LPub3D.Release.build.log.txt
ECHO - Copying change_log to media folder...    				>>  ../release/LPub3D.Release.build.log.txt
ECHO. 	
ECHO - Copying change_log to media folder...

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

COPY /V /Y ..\docs\README.txt ..\release\%VERSION%\Update\change_log.txt /A										>>  ../release/LPub3D.Release.build.log.txt
COPY /V /Y ..\docs\README.txt ..\release\%VERSION%\Download\ /A													>>  ../release/LPub3D.Release.build.log.txt
COPY /V /Y ..\docs\README.txt ..\release\ /A																	>>  ../release/LPub3D.Release.build.log.txt
XCOPY /S /I /E /V /Y ..\docs\CREDITS.txt ..\release\docs\ /A													>>  ../release/LPub3D.Release.build.log.txt

ECHO. 															>>  ../release/LPub3D.Release.build.log.txt
ECHO - Generating lpub3dupdates.json version input file...		>>  ../release/LPub3D.Release.build.log.txt
ECHO. 	
ECHO - Generating lpub3dupdates.json version input file...

SET updatesFile=..\release\json\lpub3dupdates.json
SET genLPub3DUpdates=%updatesFile% ECHO

SET VER_LATEST=%VER_MAJOR%.%VER_MINOR%.%VER_SP%

:GENERATE lpub3dupdates.json version input file
>%genLPub3DUpdates% {
>>%genLPub3DUpdates%   "_comment": "LPub3D lpub3dupdates.json generated on %DATETIMEf%",
>>%genLPub3DUpdates%   "updates": {
>>%genLPub3DUpdates%     "windows": {
>>%genLPub3DUpdates%       "open-url": "https://sourceforge.net/projects/lpub3d/files/%VER_LATEST%/",
>>%genLPub3DUpdates%       "latest-version": "%VER_LATEST%",
>>%genLPub3DUpdates%       "download-url": "http://lpub3d.sourceforge.net/LPub3D-UpdateMaster.exe",
>>%genLPub3DUpdates%       "changelog-url": "http://lpub3d.sourceforge.net/change_log.txt"
>>%genLPub3DUpdates%     },
>>%genLPub3DUpdates%     "osx": {
>>%genLPub3DUpdates%       "open-url": "https://sourceforge.net/projects/lpub3d/files/%VER_LATEST%/",
>>%genLPub3DUpdates%       "latest-version": "%VER_LATEST%",
>>%genLPub3DUpdates%       "download-url": "http://lpub3d.sourceforge.net/LPub3D-UpdateMaster.exe",
>>%genLPub3DUpdates%       "changelog-url": "http://lpub3d.sourceforge.net/change_log.txt"
>>%genLPub3DUpdates%     },
>>%genLPub3DUpdates%     "linux": {
>>%genLPub3DUpdates%       "open-url": "https://sourceforge.net/projects/lpub3d/files/%VER_LATEST%/",
>>%genLPub3DUpdates%       "latest-version": "%VER_LATEST%",
>>%genLPub3DUpdates%       "download-url": "http://lpub3d.sourceforge.net/LPub3D-UpdateMaster.exe",
>>%genLPub3DUpdates%       "changelog-url": "http://lpub3d.sourceforge.net/change_log.txt"
>>%genLPub3DUpdates%     }
>>%genLPub3DUpdates%   }
>>%genLPub3DUpdates% }
>>%genLPub3DUpdates%.

ECHO. 													 		>>  ../release/LPub3D.Release.build.log.txt
ECHO - Copying lpub3dupdates.json to media folder...    		>>  ../release/LPub3D.Release.build.log.txt
ECHO. 	
ECHO - Copying lpub3dupdates.json to media folder...

COPY /V /Y ..\release\json\lpub3dupdates.json ..\release\%VERSION%\Update\ /A									>>  ../release/LPub3D.Release.build.log.txt

ECHO. 															>>  ../release/LPub3D.Release.build.log.txt
ECHO - Generating AppVersion.nsh build input script...   		>>  ../release/LPub3D.Release.build.log.txt
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

ECHO. 															>>  ../release/LPub3D.Release.build.log.txt
ECHO - Copying %WIN32PRODUCTDIR% content to media folder...    	>>  ../release/LPub3D.Release.build.log.txt
ECHO. 	
ECHO - Copying %WIN32PRODUCTDIR% content to media folder...

XCOPY /S /I /E /V /Y %devRootPath%\extras ..\release\%VERSION%\%WIN32PRODUCTDIR%\%PRODUCT%_x32\extras			>>  ../release/LPub3D.Release.build.log.txt
XCOPY /S /I /E /V /Y ..\release\libraries ..\release\%VERSION%\%WIN32PRODUCTDIR%\%PRODUCT%_x32\libraries		>>  ../release/LPub3D.Release.build.log.txt 
XCOPY /S /I /E /V /Y ..\release\3rdParty ..\release\%VERSION%\%WIN32PRODUCTDIR%\%PRODUCT%_x32\3rdParty			>>  ../release/LPub3D.Release.build.log.txt 
XCOPY /S /I /E /V /Y ..\docs ..\release\%VERSION%\%WIN32PRODUCTDIR%\%PRODUCT%_x32\docs							>>  ../release/LPub3D.Release.build.log.txt 
XCOPY /S /I /E /V /Y ..\icons ..\release\%VERSION%\%WIN32PRODUCTDIR%\%PRODUCT%_x32\icons						>>  ../release/LPub3D.Release.build.log.txt
COPY /V /Y %Win32LPub3DBuildFile% ..\release\%VERSION%\%WIN32PRODUCTDIR%\%PRODUCT%_x32\%PRODUCT%_x32.exe /B		>>  ../release/LPub3D.Release.build.log.txt
COPY /V /Y %Win32QuazipBuildFile% ..\release\%VERSION%\%WIN32PRODUCTDIR%\%PRODUCT%_x32\ /B                      >>  ../release/LPub3D.Release.build.log.txt
COPY /V /Y %Win32LdrawiniBuildFile% ..\release\%VERSION%\%WIN32PRODUCTDIR%\%PRODUCT%_x32\ /B                    >>  ../release/LPub3D.Release.build.log.txt
COPY /V /Y %Win32QtPath%\Qt5OpenGL.dll ..\release\%VERSION%\%WIN32PRODUCTDIR%\%PRODUCT%_x32\ /B                 >>  ../release/LPub3D.Release.build.log.txt
COPY /V /Y %Win32QtPath%\Qt5PrintSupport.dll ..\release\%VERSION%\%WIN32PRODUCTDIR%\%PRODUCT%_x32\ /B           >>  ../release/LPub3D.Release.build.log.txt
COPY /V /Y %Win32QtPath%\Qt5Widgets.dll ..\release\%VERSION%\%WIN32PRODUCTDIR%\%PRODUCT%_x32\ /B                >>  ../release/LPub3D.Release.build.log.txt
COPY /V /Y %Win32QtPath%\Qt5Gui.dll ..\release\%VERSION%\%WIN32PRODUCTDIR%\%PRODUCT%_x32\ /B                 	>>  ../release/LPub3D.Release.build.log.txt
COPY /V /Y %Win32QtPath%\Qt5Network.dll ..\release\%VERSION%\%WIN32PRODUCTDIR%\%PRODUCT%_x32\ /B                >>  ../release/LPub3D.Release.build.log.txt
COPY /V /Y %Win32QtPath%\Qt5Core.dll ..\release\%VERSION%\%WIN32PRODUCTDIR%\%PRODUCT%_x32\ /B                 	>>  ../release/LPub3D.Release.build.log.txt
COPY /V /Y ..\docs\README.txt ..\release\%VERSION%\%WIN32PRODUCTDIR%\%PRODUCT%_x32\ /A                          >>  ../release/LPub3D.Release.build.log.txt
COPY /V /Y ..\docs\README.txt ..\release\%VERSION%\%WIN32PRODUCTDIR%\ /A                                        >>  ../release/LPub3D.Release.build.log.txt

ECHO. 															>>  ../release/LPub3D.Release.build.log.txt
ECHO - Copying %WIN64PRODUCTDIR% content to media folder...    	>>  ../release/LPub3D.Release.build.log.txt
ECHO. 	
ECHO - Copying %WIN64PRODUCTDIR% content to media folder...

XCOPY /S /I /E /V /Y %devRootPath%\extras ..\release\%VERSION%\%WIN64PRODUCTDIR%\%PRODUCT%_x64\extras			>>  ../release/LPub3D.Release.build.log.txt
XCOPY /S /I /E /V /Y ..\release\libraries ..\release\%VERSION%\%WIN64PRODUCTDIR%\%PRODUCT%_x64\libraries		>>  ../release/LPub3D.Release.build.log.txt
XCOPY /S /I /E /V /Y ..\release\3rdParty ..\release\%VERSION%\%WIN64PRODUCTDIR%\%PRODUCT%_x64\3rdParty			>>  ../release/LPub3D.Release.build.log.txt
XCOPY /S /I /E /V /Y ..\docs ..\release\%VERSION%\%WIN64PRODUCTDIR%\%PRODUCT%_x64\docs							>>  ../release/LPub3D.Release.build.log.txt 
XCOPY /S /I /E /V /Y ..\icons ..\release\%VERSION%\%WIN64PRODUCTDIR%\%PRODUCT%_x64\icons						>>  ../release/LPub3D.Release.build.log.txt
COPY /V /Y %Win64LPub3DBuildFile% ..\release\%VERSION%\%WIN64PRODUCTDIR%\%PRODUCT%_x64\%PRODUCT%_x64.exe /B		>>  ../release/LPub3D.Release.build.log.txt
COPY /V /Y %Win64QuazipBuildFile% ..\release\%VERSION%\%WIN64PRODUCTDIR%\%PRODUCT%_x64\ /B						>>  ../release/LPub3D.Release.build.log.txt
COPY /V /Y %Win64LdrawiniBuildFile% ..\release\%VERSION%\%WIN64PRODUCTDIR%\%PRODUCT%_x64\ /B					>>  ../release/LPub3D.Release.build.log.txt
COPY /V /Y %Win64QtPath%\Qt5OpenGL.dll ..\release\%VERSION%\%WIN64PRODUCTDIR%\%PRODUCT%_x64\ /B					>>  ../release/LPub3D.Release.build.log.txt
COPY /V /Y %Win64QtPath%\Qt5PrintSupport.dll ..\release\%VERSION%\%WIN64PRODUCTDIR%\%PRODUCT%_x64\ /B			>>  ../release/LPub3D.Release.build.log.txt
COPY /V /Y %Win64QtPath%\Qt5Widgets.dll ..\release\%VERSION%\%WIN64PRODUCTDIR%\%PRODUCT%_x64\ /B				>>  ../release/LPub3D.Release.build.log.txt
COPY /V /Y %Win64QtPath%\Qt5Gui.dll ..\release\%VERSION%\%WIN64PRODUCTDIR%\%PRODUCT%_x64\ /B					>>  ../release/LPub3D.Release.build.log.txt
COPY /V /Y %Win64QtPath%\Qt5Network.dll ..\release\%VERSION%\%WIN64PRODUCTDIR%\%PRODUCT%_x64\ /B				>>  ../release/LPub3D.Release.build.log.txt
COPY /V /Y %Win64QtPath%\Qt5Core.dll ..\release\%VERSION%\%WIN64PRODUCTDIR%\%PRODUCT%_x64\ /B					>>  ../release/LPub3D.Release.build.log.txt
COPY /V /Y ..\docs\README.txt ..\release\%VERSION%\%WIN64PRODUCTDIR%\%PRODUCT%_x64\ /A							>>  ../release/LPub3D.Release.build.log.txt
COPY /V /Y ..\docs\README.txt ..\release\%VERSION%\%WIN64PRODUCTDIR% /A											>>  ../release/LPub3D.Release.build.log.txt

ECHO. 															>>  ../release/LPub3D.Release.build.log.txt
ECHO - Finished copying content to media folder...     			>>  ../release/LPub3D.Release.build.log.txt
ECHO. 
ECHO - Finished copying content to media folder...

IF %COMMAND% == "run" ECHO. 								        >>  ..\release\LPub3D.Release.build.log.txt
IF %COMMAND% == "run" ECHO - Start NSIS Master Update Build...  	>>  ..\release\LPub3D.Release.build.log.txt
IF %COMMAND% == "run" ECHO.
IF %COMMAND% == "run" ECHO - Start NSIS Master Update Build...

IF %COMMAND% == "run" IF %COMMAND% == "run" "%NSISPath%\makensis.exe" /DUpdateMaster LPub3DNoPack.nsi 		>> ..\release\LPub3D.Release.build.log.txt

IF %COMMAND% == "run" ECHO. 										    >>  ..\release\LPub3D.Release.build.log.txt
IF %COMMAND% == "run" ECHO - Finished NSIS Master Update 	Build...	>>  ..\release\LPub3D.Release.build.log.txt
IF %COMMAND% == "run" ECHO.
IF %COMMAND% == "run" ECHO - Finished NSIS Master Update  Build...

IF %COMMAND% == "run" ECHO. 									    >>  ..\release\LPub3D.Release.build.log.txt
IF %COMMAND% == "run" ECHO - Start NSIS Manual Install Build... 	>>  ..\release\LPub3D.Release.build.log.txt
IF %COMMAND% == "run" ECHO.
IF %COMMAND% == "run" ECHO - Start NSIS Manual Install Build...

IF %COMMAND% == "run" IF %COMMAND% == "run" "%NSISPath%\makensis.exe" LPub3DNoPack.nsi 						>> ..\release\LPub3D.Release.build.log.txt

IF %COMMAND% == "run" ECHO. 										 >>  ..\release\LPub3D.Release.build.log.txt
IF %COMMAND% == "run" ECHO - Finished NSIS Manual Install Build... >>  ..\release\LPub3D.Release.build.log.txt
IF %COMMAND% == "run" ECHO.
IF %COMMAND% == "run" ECHO - Finished NSIS Manual Install Build...

IF %COMMAND% == "run" ECHO. 															>>  ../release/LPub3D.Release.build.log.txt
IF %COMMAND% == "run" ECHO - Creating portable media zip files...		      			>>  ../release/LPub3D.Release.build.log.txt
IF %COMMAND% == "run" ECHO. 	
IF %COMMAND% == "run" ECHO - Creating portable media zip files...

IF %COMMAND% == "run" %zipExe% a -tzip ..\release\%VERSION%\Download\%WIN32PRODUCTDIR%.zip ..\release\%VERSION%\%WIN32PRODUCTDIR%\ 		>>  ../release/LPub3D.Release.build.log.txt
IF %COMMAND% == "run" %zipExe% a -tzip ..\release\%VERSION%\Download\%WIN64PRODUCTDIR%.zip ..\release\%VERSION%\%WIN64PRODUCTDIR%\ 		>>  ../release/LPub3D.Release.build.log.txt

IF %COMMAND% == "run" ECHO. 															>>  ../release/LPub3D.Release.build.log.txt
IF %COMMAND% == "run" ECHO - Remove %PRODUCT% %VERSION% build files...		      	>>  ../release/LPub3D.Release.build.log.txt
IF %COMMAND% == "run" ECHO. 	
IF %COMMAND% == "run" ECHO - Remove %PRODUCT% %VERSION% build files...

IF %COMMAND% == "run" RD /Q /S ..\release\%VERSION%\%WIN32PRODUCTDIR%\ ..\release\%VERSION%\%WIN64PRODUCTDIR%\ 							>>  ../release/LPub3D.Release.build.log.txt

IF %COMMAND% == "run" ECHO. 													 	>>  ../release/LPub3D.Release.build.log.txt
IF %COMMAND% == "run" ECHO - Moving NSIS-generated files to media folder...			>>  ../release/LPub3D.Release.build.log.txt
IF %COMMAND% == "run" ECHO. 	
IF %COMMAND% == "run" ECHO - Moving NSIS-generated files to media folder...

IF %COMMAND% == "run" MOVE /Y ..\release\%DOWNLOADPRODUCT%.exe ..\release\%VERSION%\Download\												>>  ../release/LPub3D.Release.build.log.txt		
IF %COMMAND% == "run" MOVE /Y ..\release\%PRODUCT%-UpdateMaster.exe ..\release\%VERSION%\Update\											>>  ../release/LPub3D.Release.build.log.txt		

ECHO. 															>>  ../release/LPub3D.Release.build.log.txt
ECHO - Finished.												>>  ../release/LPub3D.Release.build.log.txt
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
