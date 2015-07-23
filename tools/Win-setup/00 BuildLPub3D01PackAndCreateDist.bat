@ECHO off
Title Pack and create manual and automatic install distributions
rem --
rem  Trevor SANDY <trevor.sandy@gmail.com>
rem  Copyright (c) 2015 by Trevor Sandy
rem --
SETLOCAL
@break off
@color 0a
ECHO Pack and create manual and automatic install distributions

SET Win32BuildFile="..\..\..\build-LPub3D1.0Maint-Win32-Release\mainApp\build\release\LPub3D.exe"
SET Win64BuildFile="..\..\..\build-LPub3D1.0Maint-Win64-Release\mainApp\build\release\LPub3D.exe"
SET Win32QuazipFile="..\..\..\build-LPub3D1.0Maint-Win32-Release\quazip\build\release\quazip.dll"
SET Win64QuazipFile="..\..\..\build-LPub3D1.0Maint-Win64-Release\quazip\build\release\quazip.dll"

SET Win32QtPath="C:\Qt\qt-4.8.6-x86-mingw492r1-sjlj\qt-4.8.6-x86-mingw492r1-sjlj\bin"
SET Win64QtPath="C:\Qt\qt-4.8.6-x64-mingw492r1-sjlj\qt-4.8.6-x64-mingw492r1-sjlj\bin"

SET NSISPath=C:\Program Files (x86)\NSIS
SET VirtualBoxPath=C:\Program Files (x86)\Enigma Virtual Box
SET devRootPath="..\..\mainApp"

SET zipWin64=C:\program files\7-zip
SET zipWin32=C:\Program Files (x86)\7-zip

SET zipExe=unknown

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

ECHO. 							                	>>  ..\release\LPub3D.Release.build.log.txt
ECHO - Setting up release variables          		>>  ..\release\LPub3D.Release.build.log.txt
ECHO.
ECHO - Setting up release variables...
ECHO.

cd /D %devRootPath%
FOR /F "tokens=2"  %%i IN ('svn info -r HEAD ^| FINDSTR Revision:') DO SET REVISION_CMS=%%i
FOR /F "tokens=3*" %%i IN ('FINDSTR /c:"#define _BUILD_NUMBER" build.h') DO SET BUILD=%%i
FOR /F "tokens=3*" %%i IN ('FINDSTR /c:"#define VER_PRODUCTNAME_STR" version.h') DO SET PRODUCT=%%i
FOR /F "tokens=3*" %%i IN ('FINDSTR /c:"#define VER_MAJOR" version.h') DO SET VER_MAJOR=%%i
FOR /F "tokens=3*" %%i IN ('FINDSTR /c:"#define VER_MINOR" version.h') DO SET VER_MINOR=%%i
FOR /F "tokens=3*" %%i IN ('FINDSTR /c:"#define VER_SP" version.h') DO SET VER_SP=%%i

SET BUILD=%BUILD:"=%
SET PRODUCT=%PRODUCT:"=%
SET YEAR=%DATE:~-4%
ECHO YEAR=%YEAR%
SET MONTH=%DATE:~3,2%
IF "%MONTH:~0,1%" == " " SET MONTH=0%MONTH:~1,1%
ECHO MONTH=%MONTH%
SET DAY=%DATE:~0,2%
IF "%DAY:~0,1%" == " " SET DAY=0%DAY:~1,1%
ECHO DAY=%DAY%

SET VERSION=%VER_MAJOR%.%VER_MINOR%.%VER_SP%
SET BUILDVERSION=%VERSION%.%REVISION_CMS%.%BUILD%
SET FULLVERSION=%VERSION%.%REVISION_CMS%.%BUILD%_%YEAR%%MONTH%%DAY%
SET DOWNLOADPRODUCT=%PRODUCT%-%FULLVERSION%
SET WIN32PRODUCTDIR=%PRODUCT%_x32-%FULLVERSION%
SET WIN64PRODUCTDIR=%PRODUCT%_x64-%FULLVERSION%

ECHO APPLICATION=%PRODUCT%
ECHO MAJOR VERSION=%VER_MAJOR%
ECHO MINOR VERSION=%VER_MINOR%
ECHO SERVICE PACK=%VER_SP%
ECHO BUILD=%BUILD%
ECHO VERSION=%VER_MAJOR%.%VER_MINOR%.%VER_SP%
ECHO FULLVERSION=%FULLVERSION%
ECHO DOWNLOADPRODUCT=%PRODUCT%-%FULLVERSION%
ECHO WIN32PRODUCTDIR=%PRODUCT%_x32-%FULLVERSION%
ECHO WIN64PRODUCTDIR=%PRODUCT%_x64-%FULLVERSION%
cd /D ..\tools\Win-setup

ECHO. 													 		>>  ../release/LPub3D.Release.build.log.txt
ECHO - Delete old media content and create new folders  		>>  ../release/LPub3D.Release.build.log.txt
ECHO. 	
ECHO - Delete old media content and create new folders...

RD /Q /S ..\release\%VERSION% ..\release\docs
DEL /Q ..\release\*.txt
DEL /Q ..\release\*.exe

IF NOT EXIST "..\release\%VERSION%\Download\" (
  MKDIR "..\release\%VERSION%\Download\"
)
IF NOT EXIST "..\release\%VERSION%\Update\" (
  MKDIR "..\release\%VERSION%\Update\"
)
ECHO. 													 		>>  ../release/LPub3D.Release.build.log.txt
ECHO - Copying latest and change_log to media folder      		>>  ../release/LPub3D.Release.build.log.txt
ECHO. 	
ECHO - Copying latest and change_log to media folder...
SET file=README.txt
SET temp=temp.dat
SET targetln=1
SET productversion=%PRODUCT% %BUILDVERSION%
SET n=0

cd /D ..\docs\
RENAME %file% %temp%
rem FOR /F "tokens=*" %%i IN (%temp%) DO CALL :setversion %%i
FOR /F "tokens=*" %%i IN ('FINDSTR /n "^" "%temp%"') DO CALL :setversion %%i
DEL %temp%
cd /D ..\Win-setup\
rem EXIT /b 0

COPY /V /Y ..\docs\README.txt ..\release\%VERSION%\Update\change_log.txt /A
COPY /V /Y ..\docs\README.txt ..\release\%VERSION%\Download\ /A
COPY /V /Y ..\docs\README.txt ..\release\ /A
XCOPY /S /I /E /V /Y ..\docs\CREDITS.txt ..\release\docs\ /A

SET latestFile=..\release\%VERSION%\Update\latest.txt
SET genLatestLog=%latestFile% ECHO

:GENERATE latest.txt file
>%genLatestLog% %VERSION% 

ECHO. 															>>  ../release/LPub3D.Release.build.log.txt
ECHO - Copying %WIN32PRODUCTDIR% content to media folder      	>>  ../release/LPub3D.Release.build.log.txt
ECHO. 	
ECHO - Copying %WIN32PRODUCTDIR% content to media folder...
ECHO.
XCOPY /S /I /E /V /Y ..\release\3rdParty ..\release\%VERSION%\%WIN32PRODUCTDIR%\%PRODUCT%_x32\3rdParty
ECHO. 
XCOPY /S /I /E /V /Y ..\docs ..\release\%VERSION%\%WIN32PRODUCTDIR%\%PRODUCT%_x32\docs
ECHO. 
XCOPY /S /I /E /V /Y ..\icons ..\release\%VERSION%\%WIN32PRODUCTDIR%\%PRODUCT%_x32\icons
ECHO. 
COPY /V /Y %Win32BuildFile% ..\release\%VERSION%\%WIN32PRODUCTDIR%\%PRODUCT%_x32\ /B
COPY /V /Y %Win32QuazipFile% ..\release\%VERSION%\%WIN32PRODUCTDIR%\%PRODUCT%_x32\ /B
COPY /V /Y %Win32QtPath%\libeay32.dll ..\release\%VERSION%\%WIN32PRODUCTDIR%\%PRODUCT%_x32\ /B
COPY /V /Y %Win32QtPath%\ssleay32.dll ..\release\%VERSION%\%WIN32PRODUCTDIR%\%PRODUCT%_x32\ /B
COPY /V /Y %Win32QtPath%\libgcc_s_sjlj-1.dll ..\release\%VERSION%\%WIN32PRODUCTDIR%\%PRODUCT%_x32\ /B
COPY /V /Y "%Win32QtPath%\libstdc++-6.dll" ..\release\%VERSION%\%WIN32PRODUCTDIR%\%PRODUCT%_x32\ /B
COPY /V /Y %Win32QtPath%\libwinpthread-1.dll ..\release\%VERSION%\%WIN32PRODUCTDIR%\%PRODUCT%_x32\ /B
COPY /V /Y %Win32QtPath%\QtCore4.dll ..\release\%VERSION%\%WIN32PRODUCTDIR%\%PRODUCT%_x32\ /B
COPY /V /Y %Win32QtPath%\QtGui4.dll ..\release\%VERSION%\%WIN32PRODUCTDIR%\%PRODUCT%_x32\ /B
COPY /V /Y %Win32QtPath%\QtNetwork4.dll ..\release\%VERSION%\%WIN32PRODUCTDIR%\%PRODUCT%_x32\ /B
COPY /V /Y %Win32QtPath%\QtOpenGL4.dll ..\release\%VERSION%\%WIN32PRODUCTDIR%\%PRODUCT%_x32\ /B
COPY /V /Y ..\docs\README.txt ..\release\%VERSION%\%WIN32PRODUCTDIR%\%PRODUCT%_x32\ /A
COPY /V /Y ..\docs\README.txt ..\release\%VERSION%\%WIN32PRODUCTDIR%\ /A

ECHO. 															>>  ../release/LPub3D.Release.build.log.txt
ECHO - Copying %WIN64PRODUCTDIR% content to media folder      	>>  ../release/LPub3D.Release.build.log.txt
ECHO. 	
ECHO - Copying %WIN64PRODUCTDIR% content to media folder...
ECHO.
XCOPY /S /I /E /V /Y ..\release\3rdParty ..\release\%VERSION%\%WIN64PRODUCTDIR%\%PRODUCT%_x64\3rdParty
ECHO. 
XCOPY /S /I /E /V /Y ..\docs ..\release\%VERSION%\%WIN64PRODUCTDIR%\%PRODUCT%_x64\docs
ECHO. 
XCOPY /S /I /E /V /Y ..\icons ..\release\%VERSION%\%WIN64PRODUCTDIR%\%PRODUCT%_x64\icons
ECHO. 
COPY /V /Y %Win64BuildFile% ..\release\%VERSION%\%WIN64PRODUCTDIR%\%PRODUCT%_x64\ /B
COPY /V /Y %Win64QuazipFile% ..\release\%VERSION%\%WIN64PRODUCTDIR%\%PRODUCT%_x64\ /B
COPY /V /Y %Win64QtPath%\libeay32.dll ..\release\%VERSION%\%WIN64PRODUCTDIR%\%PRODUCT%_x64\ /B
COPY /V /Y %Win64QtPath%\ssleay32.dll ..\release\%VERSION%\%WIN64PRODUCTDIR%\%PRODUCT%_x64\ /B
COPY /V /Y %Win64QtPath%\libgcc_s_sjlj-1.dll ..\release\%VERSION%\%WIN64PRODUCTDIR%\%PRODUCT%_x64\ /B
COPY /V /Y "%Win64QtPath%\libstdc++-6.dll" ..\release\%VERSION%\%WIN64PRODUCTDIR%\%PRODUCT%_x64\ /B
COPY /V /Y %Win64QtPath%\libwinpthread-1.dll ..\release\%VERSION%\%WIN64PRODUCTDIR%\%PRODUCT%_x64\ /B
COPY /V /Y %Win64QtPath%\QtCore4.dll ..\release\%VERSION%\%WIN64PRODUCTDIR%\%PRODUCT%_x64\ /B
COPY /V /Y %Win64QtPath%\QtGui4.dll ..\release\%VERSION%\%WIN64PRODUCTDIR%\%PRODUCT%_x64\ /B
COPY /V /Y %Win64QtPath%\QtNetwork4.dll ..\release\%VERSION%\%WIN64PRODUCTDIR%\%PRODUCT%_x64\ /B
COPY /V /Y %Win64QtPath%\QtOpenGL4.dll ..\release\%VERSION%\%WIN64PRODUCTDIR%\%PRODUCT%_x64\ /B
COPY /V /Y ..\docs\README.txt ..\release\%VERSION%\%WIN64PRODUCTDIR%\%PRODUCT%_x64\ /A
COPY /V /Y ..\docs\README.txt ..\release\%VERSION%\%WIN64PRODUCTDIR% /A
ECHO. 															>>  ../release/LPub3D.Release.build.log.txt
ECHO - Finished coying content to media folders      			>>  ../release/LPub3D.Release.build.log.txt
ECHO. 	
ECHO - Finished coying content to media folders...

ECHO. 													 		>>  ../release/LPub3D.Release.build.log.txt
ECHO - Launch packing process									>>  ../release/LPub3D.Release.build.log.txt
ECHO. 	
ECHO - Launch packing process...
ECHO.
ECHO - Start Pack LPub3D x86_32...
ECHO - Start Pack LPub3D x86_32 	>  ..\release\LPub3D.Release.build.log.txt
"%VirtualBoxPath%\enigmavbconsole.exe" LPub3D_x32.evb >> ..\release\LPub3D.Release.build.log.txt
ECHO. 								>>  ..\release\LPub3D.Release.build.log.txt
ECHO - Finished Pack LPub3D x86_32 	>>  ..\release\LPub3D.Release.build.log.txt
ECHO - Finished Pack LPub3D x86_32...
ECHO.
ECHO - Start Pack LPub3D x86_64...
ECHO. 								>>  ..\release\LPub3D.Release.build.log.txt
ECHO - Start Pack LPub3D x86_64 	>>  ..\release\LPub3D.Release.build.log.txt
"%VirtualBoxPath%\enigmavbconsole.exe" LPub3D_x64.evb  >> ..\release\LPub3D.Release.build.log.txt
ECHO. 								>>  ..\release\LPub3D.Release.build.log.txt
ECHO - Finished Pack LPub3D x86_64 	>>  ..\release\LPub3D.Release.build.log.txt
ECHO - Finished Pack LPub3D x86_64...
ECHO.
ECHO - Start NSIS Master Update...
ECHO. 								>>  ..\release\LPub3D.Release.build.log.txt
ECHO - Start NSIS Master Update   	>>  ..\release\LPub3D.Release.build.log.txt
"%NSISPath%\makensis.exe" /DUpdateMaster LPub3D.nsi 	>> ..\release\LPub3D.Release.build.log.txt
ECHO. 								>>  ..\release\LPub3D.Release.build.log.txt
ECHO - Finished NSIS Master Update 	>>  ..\release\LPub3D.Release.build.log.txt
ECHO - Finished NSIS Master Update...
ECHO.
ECHO - Start NSIS Manual Install...
ECHO. 								>>  ..\release\LPub3D.Release.build.log.txt
ECHO - Start NSIS Manual Install	>>  ..\release\LPub3D.Release.build.log.txt
"%NSISPath%\makensis.exe" LPub3D.nsi 					>> ..\release\LPub3D.Release.build.log.txt
ECHO. 								>>  ..\release\LPub3D.Release.build.log.txt
ECHO - Finished NSIS Manual Install >>  ..\release\LPub3D.Release.build.log.txt
ECHO - Finished NSIS Manual Install...

ECHO. 															>>  ../release/LPub3D.Release.build.log.txt
ECHO - Creating portable media zip files		      			>>  ../release/LPub3D.Release.build.log.txt
ECHO. 	
ECHO - Creating portable media zip files...
%zipExe% a -tzip ..\release\%VERSION%\Download\%WIN32PRODUCTDIR%.zip ..\release\%VERSION%\%WIN32PRODUCTDIR%\ >> ../release/LPub3D.Release.build.log.txt
%zipExe% a -tzip ..\release\%VERSION%\Download\%WIN64PRODUCTDIR%.zip ..\release\%VERSION%\%WIN64PRODUCTDIR%\ >> ../release/LPub3D.Release.build.log.txt

ECHO. 															>>  ../release/LPub3D.Release.build.log.txt
ECHO - Cleanup (delete work folders)		      				>>  ../release/LPub3D.Release.build.log.txt
ECHO. 
ECHO - Cleanup (delete work folders)...
RD /Q /S ..\release\%VERSION%\%WIN32PRODUCTDIR%\ ..\release\%VERSION%\%WIN64PRODUCTDIR%\ 

ECHO. 															>>  ../release/LPub3D.Release.build.log.txt
ECHO - Moving NSIS-generated files to media folders				>>  ../release/LPub3D.Release.build.log.txt
ECHO. 	
ECHO - Moving NSIS-generated files to media folder...
MOVE /Y ..\release\%DOWNLOADPRODUCT%.exe ..\release\%VERSION%\Download\
MOVE /Y ..\release\%PRODUCT%-UpdateMaster.exe ..\release\%VERSION%\Update\

ECHO. 															>>  ../release/LPub3D.Release.build.log.txt
ECHO - Finished													>>  ../release/LPub3D.Release.build.log.txt
ECHO. 	
ECHO - Finished...

ECHO.
ECHO If everything went well Press any key to EXIT!
%SystemRoot%\explorer.exe "C:\Users\Trevor\Downloads\LEGO\LPub\project\LPub\LPub3D1.0Maint\tools\release\%VERSION%"
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
