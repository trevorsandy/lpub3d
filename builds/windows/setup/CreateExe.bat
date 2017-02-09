@ECHO off
Title Create archive packaged and windows installer LPub3D distributions
rem --
rem  Trevor SANDY <trevor.sandy@gmail.com>
rem  Last Update: February 08, 2017
rem  Copyright (c) 2015 - 2017 by Trevor Sandy
rem --
SETLOCAL
@break off
@color 0a

IF NOT EXIST "..\release\" (
  MKDIR "..\release\"
)
SET BuildLog=..\release\LPub3D.Release.build.log.txt
ECHO.																			>    %BuildLog%
ECHO Create archive packaged and windows installer LPub3D distributions			>>   %BuildLog%
ECHO.
ECHO Create archive packaged and windows installer LPub3D distributions			

SET HOME=%cd%
SET RUN_NSIS=1
SET SIGN_APP=1
SET CLEANUP=1
SET CREATE_PORTABLE=1
SET VERSION_INPUT="0 0 0 0 0"
SET DATETIME_INPUT="0000 00 00 00:00:00.00"

SET AVAILABLE_VERS_EXE=2.0.19,1.3.5,1.2.3,1.0.0
SET AVAILABLE_VERS_DMG=2.0.19
SET AVAILABLE_VERS_DEB=2.0.19
SET AVAILABLE_VERS_RPM=2.0.19
SET AVAILABLE_VERS_PKG=2.0.19

ECHO.
SET /p RUN_NSIS= - Run NSIS: Type 1 to run, 0 to ignore or 'Enter' to accept default [%RUN_NSIS%]: 
IF %RUN_NSIS% == 0 (
   SET SIGN_APP=0 
   SET CLEANUP=0
   SET CREATE_PORTABLE=0
	)
IF %RUN_NSIS% == 1 (
	ECHO.						
	SET /p SIGN_APP= - Code Signing: Type 1 to run, 0 to ignore or 'Enter' to accept default [%SIGN_APP%]:
	ECHO.
	SET /p CLEANUP= - Cleanup: Type 1 to run, 0 to ignore or 'Enter' to accept default [%CLEANUP%]: 
	)
ECHO.																					>>  %BuildLog%
ECHO - Selected build options:															>>  %BuildLog%
ECHO.  		
IF %RUN_NSIS% == 0 ECHO - This configuration will allow you to test your NSIS scripts.	>>  %BuildLog%
IF %RUN_NSIS% == 1 ECHO   RUN_NSIS [Yes]												>>  %BuildLog%
IF %SIGN_APP% == 1 ECHO   SIGN_APP [Yes]												>>  %BuildLog%
IF %CLEANUP% == 1 ECHO   CLEANUP  [Yes]													>>  %BuildLog%
IF %RUN_NSIS% == 0 ECHO   RUN_NSIS [No]													>>  %BuildLog%
IF %SIGN_APP% == 0 ECHO   SIGN_APP [No]													>>  %BuildLog%
IF %CLEANUP% == 0 ECHO   CLEANUP  [No]													>>  %BuildLog%
ECHO.
ECHO - Selected build options:
ECHO.  	
IF %RUN_NSIS% == 0 ECHO - This configuration will allow you to test your NSIS scripts.
IF %RUN_NSIS% == 1 ECHO   RUN_NSIS [Yes]
IF %SIGN_APP% == 1 ECHO   SIGN_APP [Yes]
IF %CLEANUP% == 1 ECHO   CLEANUP  [Yes]
IF %RUN_NSIS% == 0 ECHO   RUN_NSIS [No]	
IF %SIGN_APP% == 0 ECHO   SIGN_APP [No]
IF %CLEANUP% == 0 ECHO   CLEANUP  [No]

IF %RUN_NSIS% == 0 ECHO. 							                	>>  %BuildLog%
IF %RUN_NSIS% == 0 ECHO - Start NSIS test build process...      		>>  %BuildLog%
IF %RUN_NSIS% == 0 ECHO.
IF %RUN_NSIS% == 0 ECHO - Start NSIS test build process...

IF %RUN_NSIS% == 1 ECHO.												>>  %BuildLog%
IF %RUN_NSIS% == 1 ECHO - Start build process...      			   		>>  %BuildLog%
IF %RUN_NSIS% == 1 ECHO.
IF %RUN_NSIS% == 1 ECHO - Start build process...

SET LDRAWINI_BUILD_FILE=LDrawIni161.dll
SET QUAZIP_BUILD_FILE=QuaZIP07.dll

SET Win32LPub3DBuildFilePath=..\..\..\..\build-LPub3D-Desktop_Qt_5_7_1_MinGW_32bit-Release\mainApp\release
SET Win32QuaZIPBuildFilePath=..\..\..\..\build-LPub3D-Desktop_Qt_5_7_1_MinGW_32bit-Release\quazip\release
SET Win32LDrawIniBuildFilePath=..\..\..\..\build-LPub3D-Desktop_Qt_5_7_1_MinGW_32bit-Release\ldrawini\release

SET Win64LPub3DBuildFilePath=..\..\..\..\build-LPub3D-Desktop_Qt_5_7_1_MinGW_64bit_Msys64-Release\mainApp\release
SET Win64QuaZIPBuildFilePath=..\..\..\..\build-LPub3D-Desktop_Qt_5_7_1_MinGW_64bit_Msys64-Release\quazip\release
SET Win64LDrawIniBuildFilePath=..\..\..\..\build-LPub3D-Desktop_Qt_5_7_1_MinGW_64bit_Msys64-Release\ldrawini\release

SET Win32QtBinPath=C:\Qt\IDE\5.7\mingw53_32\bin
SET Win32QtPluginPath=C:\Qt\IDE\5.7\mingw53_32\plugins

SET Win64QtBinPath=C:\Msys2\Msys64\mingw64\bin
SET Win64QtPluginPath=C:\Msys2\Msys64\mingw64\share\qt5\plugins

SET SignToolExe="C:\Program Files (x86)\Windows Kits\8.1\bin\x64\signtool.exe"
SET TimeStamp=http://timestamp.digicert.com
SET Sha2=Sha256
SET PKey="..\..\utilities\cert\lpub3dPrivateKey.p12"

SET NSISExe="C:\Program Files (x86)\NSIS\makensis.exe"

SET utilitiesPath="..\..\utilities"
SET setupPath="..\windows\setup"
SET devRootPath="..\..\..\mainApp"

SET zipWin64=C:\program files\7-zip
SET zipWin32=C:\Program Files (x86)\7-zip

SET zipExe=unknown

ECHO. 							                		>>  %BuildLog%
ECHO - Environment check...        			   			>>  %BuildLog%
ECHO.
ECHO - Environment check...

IF %RUN_NSIS% == 0 GOTO MAIN

IF EXIST %NSISExe% (
	ECHO.
    ECHO - NSIS executable found at %NSISExe%
	ECHO.												>>  %BuildLog%
    ECHO - NSIS executable found at %NSISExe%			>>  %BuildLog%
    GOTO SIGN
) 

SET %RUN_NSIS%=0
SET %SIGN_APP%=0
SET %CREATE_PORTABLE%=0
ECHO.																								>>  %BuildLog%
ECHO NSIS exectutable not at path defined. Only NSIS test configuration will be produced.			>>  %BuildLog%
ECHO.
ECHO NSIS exectutable not at path defined. Only NSIS test configuration will be produced.

GOTO MAIN

:SIGN
IF %SIGN_APP% == 0 GOTO CHK_ZIP

SET PwD=unknown
SET CHK_ZIP_GO=0
IF EXIST %SignToolExe% (
	ECHO.												>>  %BuildLog%
    ECHO - Signtool executable found at %SignToolExe%	>>  %BuildLog%
	ECHO.
    ECHO - Signtool executable found at %SignToolExe%
	FOR /f %%i IN (../../utilities/cert/credentials.txt) DO SET PwD=%%i
	SET CHK_ZIP_GO=1
) 

ECHO.												>>  %BuildLog%
ECHO - Code signing password captured.				>>  %BuildLog%	
ECHO.							
ECHO - Code signing password is %PWD%.						

IF %CHK_ZIP_GO% == 1 GOTO CHK_ZIP

SET %SIGN_APP%=0
ECHO.																			>>  %BuildLog%
ECHO Signtool exectutable not at path defined. Code signing will be ignored.	>>  %BuildLog%
ECHO.
ECHO Signtool exectutable not at path defined. Code signing will be ignored.

:CHK_ZIP
IF EXIST "%zipWin64%" (
	SET zipExe="%zipWin64%\7z.exe"
	ECHO.																		>>  %BuildLog%
    ECHO - Zip exectutable found at "%zipWin64%"								>>  %BuildLog%
	ECHO.	
    ECHO - Zip exectutable found at "%zipWin64%"
    GOTO MAIN
) 

ECHO.

IF EXIST "%zipWin32%" (
	SET zipExe="%zipWin32%\7z.exe"
	ECHO.																		>>  %BuildLog%
    ECHO - Zip exectutable found at "%zipWin32%"								>>  %BuildLog%	
	ECHO.
    ECHO - Zip exectutable found at "%zipWin32%"
    GOTO MAIN
)

ECHO.

IF NOT EXIST "%zipExe%" (
	ECHO.																		>>  %BuildLog%
	ECHO Could not find zip executable. Requested manual location entry.		>>  %BuildLog%
	ECHO.																		
	ECHO Could not find zip executable. Requested manual location entry.	
	SET /p zipExe=Could not find any zip executable. You can manually enter a location: 
)

IF EXIST "%zipExe%" (
	ECHO.																					>>  %BuildLog% 
	ECHO Zip exectutable at (%zipExe%) will be used to archive your portable distributions	>>  %BuildLog% 
	ECHO.
	ECHO Zip exectutable at (%zipExe%) will be used to archive your portable distributions.
	SET OPTION=1
	SET /p OPTION= Type [1] to exit or Enter to continue:
)

IF NOT EXIST "%zipExe%" (
	SET CREATE_PORTABLE=0
	ECHO.																		>>  %BuildLog%
	ECHO Could not find zip executable. Portable builds will be ignored.		>>  %BuildLog%
	ECHO.
	ECHO Could not find zip executable. Portable builds will be ignored.
	GOTO MAIN
)

IF  %OPTION% == 1  ECHO Option to exit seleced, the script will terminate.      >>  %BuildLog%
IF  %OPTION% == 1  ECHO Option to exit seleced, the script will terminate.
IF  %OPTION% == 1  EXIT

:MAIN
SET PRODUCT=unknown
SET VERSION=unknown
SET VER_MAJOR=unknown
SET VER_MINOR=unknown
SET VER_BUILD=unknown
SET VER_PATCH=unknown
SET VER_REVISION=unknown

SET COMPANY=unknown
SET COMMENTS=unknown
SET PUBLISHER=unknown
SET COMPANYURL=unknown
SET BUILD_DATE=unknown
SET REVISION_FILE=unknown
SET SUPPORT_EMAIL=unknown
SET LPUB3D_BUILD_FILE=unknown

SET LAST_VER_EXE=unknown
SET LAST_VER_DMG=unknown
SET LAST_VER_DEB=unknown
SET LAST_VER_RPM=unknown
SET LAST_VER_PKG=unknown

SET ALT_VER_EXE=unknown
SET ALT_VER_DMG=unknown
SET ALT_VER_DEB=unknown
SET ALT_VER_RPM=unknown
SET ALT_VER_PKG=unknown

ECHO. 							                		>>  %BuildLog%
ECHO - Setting up release build parameters...          	>>  %BuildLog%
ECHO.
ECHO - Setting up release build parameters...

CD /D %utilitiesPath%

SET VERSION_INFO_FILE=version_info_win.txt
FOR /f "tokens=1 delims=," %%i IN (%VERSION_INFO_FILE%) DO SET VERSION_INPUT=%%i
FOR /f "tokens=1" %%i IN (%VERSION_INPUT%) DO SET VER_MAJOR=%%i
FOR /f "tokens=2" %%i IN (%VERSION_INPUT%) DO SET VER_MINOR=%%i
FOR /f "tokens=3" %%i IN (%VERSION_INPUT%) DO SET VER_PATCH=%%i
FOR /f "tokens=4" %%i IN (%VERSION_INPUT%) DO SET VER_REVISION=%%i
FOR /f "tokens=5" %%i IN (%VERSION_INPUT%) DO SET VER_BUILD=%%i
FOR /f "tokens=2 delims=," %%i IN (%VERSION_INFO_FILE%) DO SET DATETIME_INPUT=%%i
FOR /f "tokens=1" %%i IN (%DATETIME_INPUT%) DO SET YEAR=%%i
FOR /f "tokens=2" %%i IN (%DATETIME_INPUT%) DO SET MONTH=%%i
FOR /f "tokens=3" %%i IN (%DATETIME_INPUT%) DO SET DAY=%%i
FOR /f "tokens=4" %%i IN (%DATETIME_INPUT%) DO SET TIME=%%i

CD /D %setupPath%
CD /D %devRootPath%

FOR /F "tokens=3*" %%i IN ('FINDSTR /c:"#define VER_COMPANYNAME_BLD_STR" version.h') DO SET COMPANY=%%i %%j
FOR /F "tokens=3*" %%i IN ('FINDSTR /c:"#define VER_PRODUCTNAME_STR" version.h') DO SET PRODUCT=%%i
FOR /F "tokens=3*" %%i IN ('FINDSTR /c:"#define VER_PUBLISHER_STR" version.h') DO SET PUBLISHER=%%i %%j
FOR /F "tokens=3*" %%i IN ('FINDSTR /c:"#define VER_COMPANYDOMAIN_STR" version.h') DO SET COMPANYURL=%%i
FOR /F "tokens=3*" %%i IN ('FINDSTR /c:"#define VER_FILEDESCRIPTION_STR" version.h') DO SET COMMENTS=%%i %%j
FOR /F "tokens=3*" %%i IN ('FINDSTR /c:"#define VER_PUBLISHER_SUPPORT_EMAIL_STR" version.h') DO SET SUPPORT_EMAIL=%%i

FOR /F "tokens=1 delims=," %%i IN ("%AVAILABLE_VERS_EXE%") DO SET LAST_VER_EXE=%%i
FOR /F "tokens=1 delims=," %%i IN ("%AVAILABLE_VERS_DMG%") DO SET LAST_VER_DMG=%%i
FOR /F "tokens=1 delims=," %%i IN ("%AVAILABLE_VERS_DEB%") DO SET LAST_VER_DEB=%%i
FOR /F "tokens=1 delims=," %%i IN ("%AVAILABLE_VERS_RPM%") DO SET LAST_VER_RPM=%%i
FOR /F "tokens=1 delims=," %%i IN ("%AVAILABLE_VERS_PKG%") DO SET LAST_VER_PKG=%%i

FOR /F "tokens=2 delims=," %%i IN ("%AVAILABLE_VERS_EXE%") DO SET ALT_VER_EXE=%%i
FOR /F "tokens=1 delims=," %%i IN ("%AVAILABLE_VERS_DMG%") DO SET ALT_VER_DMG=%%i
FOR /F "tokens=1 delims=," %%i IN ("%AVAILABLE_VERS_DEB%") DO SET ALT_VER_DEB=%%i
FOR /F "tokens=1 delims=," %%i IN ("%AVAILABLE_VERS_RPM%") DO SET ALT_VER_RPM=%%i
FOR /F "tokens=1 delims=," %%i IN ("%AVAILABLE_VERS_PKG%") DO SET ALT_VER_PKG=%%i

SET VER_BUILD=%VER_BUILD:"=%
SET PRODUCT=%PRODUCT:"=%

SET LPUB3D_BUILD_FILE=%PRODUCT%%VER_MAJOR%%VER_MINOR%.exe
SET DATETIME=%YEAR% %MONTH% %DAY% %TIME%
SET VERSION=%VER_MAJOR%.%VER_MINOR%.%VER_PATCH%
SET BUILDVERSION=%VERSION%.%VER_REVISION%.%VER_BUILD%
SET FULLVERSION=%VERSION%.%VER_REVISION%.%VER_BUILD%_%YEAR%%MONTH%%DAY%
SET DOWNLOADPRODUCT=%PRODUCT%-%FULLVERSION%
SET WIN32PRODUCTDIR=%PRODUCT%_x86-%FULLVERSION%
SET WIN64PRODUCTDIR=%PRODUCT%_x86_64-%FULLVERSION%
SET SUPPORT_EMAIL=%SUPPORT_EMAIL% %BUILDVERSION%

ECHO.
ECHO   VERSION_INFO_FILE...[%VERSION_INFO_FILE%]
ECHO.                    
ECHO   VERSION_INPUT.......[%VERSION_INPUT%]
ECHO   DATETIME_INPUT......[%DATETIME_INPUT%]
ECHO.                    
ECHO   VER_MAJOR...........[%VER_MAJOR%]
ECHO   VER_MINOR...........[%VER_MINOR%]
ECHO   VER_PATCH...........[%VER_PATCH%]
ECHO   VER_REVISION........[%VER_REVISION%]
ECHO   VER_BUILD...........[%VER_BUILD%]
ECHO.                    
ECHO   YEAR................[%YEAR%]
ECHO   MONTH...............[%MONTH%]
ECHO   DAY.................[%DAY%]
ECHO   TIME................[%TIME%]
ECHO   DATETIME............[%DATETIME%]
ECHO.                    
ECHO   COMPANY.............[%COMPANY%]
ECHO   PRODUCT.............[%PRODUCT%]
ECHO   LPUB3D_BUILD_FILE...[%LPUB3D_BUILD_FILE%]
ECHO   PUBLISHER...........[%PUBLISHER%]
ECHO   COMPANYURL..........[%COMPANYURL%]
ECHO   SUPPORT_EMAIL.......[%SUPPORT_EMAIL%]
ECHO   COMMENTS............[%COMMENTS%]
ECHO.                    
ECHO   VERSION.............[%VERSION%]
ECHO   BUILDVERSION........[%BUILDVERSION%]
ECHO   FULLVERSION.........[%FULLVERSION%]
ECHO   DOWNLOADPRODUCT.....[%DOWNLOADPRODUCT%]]
ECHO   WIN32PRODUCTDIR.....[%WIN32PRODUCTDIR%]
ECHO   WIN64PRODUCTDIR.....[%WIN64PRODUCTDIR%]
ECHO.                     
ECHO   ALT_VER_EXE.........[%ALT_VER_EXE%]
ECHO   ALT_VER_DMG.........[%ALT_VER_DMG%]
ECHO   ALT_VER_DEB.........[%ALT_VER_DEB%]
ECHO   ALT_VER_RPM.........[%ALT_VER_RPM%]
ECHO   ALT_VER_PKG.........[%ALT_VER_PKG%]                 
ECHO   LAST_VER_EXE........[%LAST_VER_EXE%]
ECHO   LAST_VER_DMG........[%LAST_VER_DMG%]
ECHO   LAST_VER_DEB........[%LAST_VER_DEB%]
ECHO   LAST_VER_RPM........[%LAST_VER_RPM%]
ECHO   LAST_VER_PKG........[%LAST_VER_PKG%]
ECHO   AVAILABLE_VERS_EXE..[%AVAILABLE_VERS_EXE%]
ECHO   AVAILABLE_VERS_DMG..[%AVAILABLE_VERS_DMG%]
ECHO   AVAILABLE_VERS_DEB..[%AVAILABLE_VERS_DEB%]
ECHO   AVAILABLE_VERS_RPM..[%AVAILABLE_VERS_RPM%]
ECHO   AVAILABLE_VERS_PKG..[%AVAILABLE_VERS_PKG%]

CD /D %HOME%

ECHO.
ECHO - If parameters are OK press Enter to continue.
PAUSE >NUL

ECHO. 													 			>>  %BuildLog%
ECHO - Delete old media content and create new folders  			>>  %BuildLog%
ECHO. 	
ECHO - Delete old media content and create new folders...

IF EXIST "..\release\%VERSION%" (
  RD /Q /S ..\release\%VERSION%              						>>  %BuildLog%
)

IF EXIST "..\release\README.txt" (
  DEL /Q ..\release\README.txt										>>  %BuildLog%
)

IF NOT EXIST "..\release\%VERSION%\Download\" (
  MKDIR "..\release\%VERSION%\Download\"
)

IF NOT EXIST "..\release\%VERSION%\Update\" (
  MKDIR "..\release\%VERSION%\Update\"
)

IF %RUN_NSIS% == 1 ECHO. 											>>  %BuildLog%
IF %RUN_NSIS% == 1 ECHO - Download LDraw archive libraries...    	>>  %BuildLog%
IF %RUN_NSIS% == 1 ECHO. 	
IF %RUN_NSIS% == 1 ECHO - Download LDraw archive libraries...
IF %RUN_NSIS% == 0 ECHO. 											>>  %BuildLog%
IF %RUN_NSIS% == 0 ECHO - Skipping library download...    			>>  %BuildLog%
IF %RUN_NSIS% == 0 ECHO. 	
IF %RUN_NSIS% == 0 ECHO - Skipping library download...

IF %RUN_NSIS% == 1 CALL :LDrawArchiveLibraryDownload

ECHO. 													 			>>  %BuildLog%
ECHO - Copying change_log_%VERSION% to media folder...    			>>  %BuildLog%
ECHO. 	
ECHO - Copying change_log_%VERSION% to media folder... 

SET file=README.txt
SET temp=temp.dat
SET targetln=1
SET productversion=%PRODUCT% %BUILDVERSION%
SET n=0

CD /D ..\..\..\mainApp\docs\
RENAME %file% %temp%
FOR /F "tokens=*" %%i IN ('FINDSTR /n "^" "%temp%"') DO CALL :SetVersion %%i
DEL %temp%

CD /D %HOME%
COPY /V /Y %devRootPath%\docs\README.txt ..\release\%VERSION%\Update\change_log.txt /A									>>  %BuildLog%
COPY /V /Y %devRootPath%\docs\README.txt ..\release\%VERSION%\Update\change_log_%VERSION%.txt /A						>>  %BuildLog%
COPY /V /Y %devRootPath%\docs\README.txt ..\release\%VERSION%\Download\ /A												>>  %BuildLog%
COPY /V /Y %devRootPath%\docs\README.txt ..\release\ /A																	>>  %BuildLog%

ECHO. 																>>  %BuildLog%
ECHO - Generating lastVersionInsert_Exe.txt input file...			>>  %BuildLog%
ECHO. 	
ECHO - Generating lastVersionInsert_Exe.txt input file...

SET lastVersionInsert_ExeFile=..\..\utilities\json\lastVersionInsert_Exe.txt
SET genlastVersionInsert_Exe=%lastVersionInsert_ExeFile% ECHO

:GENERATE lastVersionInsert_Exe.txt file
>%genlastVersionInsert_Exe% "alternate-version-%LAST_VER_EXE%": {
>>%genlastVersionInsert_Exe%   "open-url": "https://sourceforge.net/projects/lpub3d/files/%LAST_VER_EXE%/",
>>%genlastVersionInsert_Exe%   "latest-version": "%LAST_VER_EXE%",
>>%genlastVersionInsert_Exe%   "download-url": "http://lpub3d.sourceforge.net/LPub3D-UpdateMaster_%LAST_VER_EXE%.exe",
>>%genlastVersionInsert_Exe%   "changelog-url": "http://lpub3d.sourceforge.net/change_log_%LAST_VER_EXE%.txt"
>>%genlastVersionInsert_Exe% },

ECHO. 																>>  %BuildLog%
ECHO - Generating lastVersionInsert_Dmg.txt input file...			>>  %BuildLog%
ECHO. 	
ECHO - Generating lastVersionInsert_Dmg.txt input file...

SET lastVersionInsert_DmgFile=..\..\utilities\json\lastVersionInsert_Dmg.txt
SET genlastVersionInsert_Dmg=%lastVersionInsert_DmgFile% ECHO

:GENERATE lastVersionInsert_Dmg.txt file
>%genlastVersionInsert_Dmg% "alternate-version-%LAST_VER_DMG%": {
>>%genlastVersionInsert_Dmg%   "open-url": "https://sourceforge.net/projects/lpub3d/files/%LAST_VER_DMG%/",
>>%genlastVersionInsert_Dmg%   "latest-version": "%LAST_VER_DMG%",
>>%genlastVersionInsert_Dmg%   "download-url": "http://lpub3d.sourceforge.net/LPub3D-UpdateMaster_%LAST_VER_DMG%_osx.dmg",
>>%genlastVersionInsert_Dmg%   "changelog-url": "http://lpub3d.sourceforge.net/change_log_%LAST_VER_DMG%.txt"
>>%genlastVersionInsert_Dmg% },

ECHO. 																>>  %BuildLog%
ECHO - Generating lastVersionInsert_Deb.txt input file...			>>  %BuildLog%
ECHO. 	
ECHO - Generating lastVersionInsert_Deb.txt input file...

SET lastVersionInsert_DebFile=..\..\utilities\json\lastVersionInsert_Deb.txt
SET genlastVersionInsert_Deb=%lastVersionInsert_DebFile% ECHO

:GENERATE lastVersionInsert_Deb.txt file
>%genlastVersionInsert_Deb% "alternate-version-%LAST_VER_DEB%": {
>>%genlastVersionInsert_Deb%   "open-url": "https://sourceforge.net/projects/lpub3d/files/%LAST_VER_DEB%/",
>>%genlastVersionInsert_Deb%   "latest-version": "%LAST_VER_DEB%",
>>%genlastVersionInsert_Deb%   "download-url": "http://lpub3d.sourceforge.net/LPub3D-UpdateMaster_%LAST_VER_DEB%_0ubuntu1_amd64.deb",
>>%genlastVersionInsert_Deb%   "changelog-url": "http://lpub3d.sourceforge.net/change_log_%LAST_VER_DEB%.txt"
>>%genlastVersionInsert_Deb% },

ECHO. 																>>  %BuildLog%
ECHO - Generating lastVersionInsert_Rpm.txt input file...			>>  %BuildLog%
ECHO. 	
ECHO - Generating lastVersionInsert_Rpm.txt input file...

SET lastVersionInsert_RpmFile=..\..\utilities\json\lastVersionInsert_Rpm.txt
SET genlastVersionInsert_Rpm=%lastVersionInsert_RpmFile% ECHO

:GENERATE lastVersionInsert_Rpm.txt file
>%genlastVersionInsert_Rpm% "alternate-version-%LAST_VER_RPM%": {
>>%genlastVersionInsert_Rpm%   "open-url": "https://sourceforge.net/projects/lpub3d/files/%LAST_VER_RPM%/",
>>%genlastVersionInsert_Rpm%   "latest-version": "%LAST_VER_RPM%",
>>%genlastVersionInsert_Rpm%   "download-url": "http://lpub3d.sourceforge.net/LPub3D-UpdateMaster_%LAST_VER_RPM%_1fedora.x86_64.rpm",
>>%genlastVersionInsert_Rpm%   "changelog-url": "http://lpub3d.sourceforge.net/change_log_%LAST_VER_RPM%.txt"
>>%genlastVersionInsert_Rpm% },

ECHO. 																>>  %BuildLog%
ECHO - Generating lastVersionInsert_Pkg.txt input file...			>>  %BuildLog%
ECHO. 	
ECHO - Generating lastVersionInsert_Pkg.txt input file...

SET lastVersionInsert_PkgFile=..\..\utilities\json\lastVersionInsert_Pkg.txt
SET genlastVersionInsert_Pkg=%lastVersionInsert_PkgFile% ECHO

:GENERATE lastVersionInsert_Pkg.txt file
>%genlastVersionInsert_Pkg% "alternate-version-%LAST_VER_PKG%": {
>>%genlastVersionInsert_Pkg%   "open-url": "https://sourceforge.net/projects/lpub3d/files/%LAST_VER_PKG%/",
>>%genlastVersionInsert_Pkg%   "latest-version": "%LAST_VER_PKG%",
>>%genlastVersionInsert_Pkg%   "download-url": "http://lpub3d.sourceforge.net/LPub3D-UpdateMaster_%LAST_VER_PKG%_x86_64.pkg.tar.xz",
>>%genlastVersionInsert_Pkg%   "changelog-url": "http://lpub3d.sourceforge.net/change_log_%LAST_VER_PKG%.txt"
>>%genlastVersionInsert_Pkg% },

ECHO. 																>>  %BuildLog%
ECHO - Generating lpub3dupdates.json template file...				>>  %BuildLog%
ECHO. 	
ECHO - Generating lpub3dupdates.json template file...

SET updatesFile=..\..\utilities\json\lpub3dupdates.json
SET genLPub3DUpdates=%updatesFile% ECHO

:GENERATE lpub3dupdates.json template file
>%genLPub3DUpdates% {
>>%genLPub3DUpdates%   "_comment": "LPub3D lpub3dupdates.json generated on %DATETIME%",
>>%genLPub3DUpdates%   "updates": {
>>%genLPub3DUpdates%     "windows": {
>>%genLPub3DUpdates%       "open-url": "https://sourceforge.net/projects/lpub3d/files/%VERSION%/",
>>%genLPub3DUpdates%       "latest-version": "%VERSION%",
>>%genLPub3DUpdates%       "download-url": "http://lpub3d.sourceforge.net/LPub3D-UpdateMaster.exe",
>>%genLPub3DUpdates%       "changelog-url": "http://lpub3d.sourceforge.net/change_log.txt",
>>%genLPub3DUpdates%       "download-url-": "http://lpub3d.sourceforge.net/LPub3D-UpdateMaster_%VERSION%.exe",
>>%genLPub3DUpdates%       "changelog-url-": "http://lpub3d.sourceforge.net/change_log_%VERSION%.txt",
>>%genLPub3DUpdates%       "available-versions": "%VERSION%,%AVAILABLE_VERS_EXE%",
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
>>%genLPub3DUpdates%       "download-url": "http://lpub3d.sourceforge.net/LPub3D-UpdateMaster_%VERSION%_osx.dmg",
>>%genLPub3DUpdates%       "changelog-url": "http://lpub3d.sourceforge.net/change_log_%VERSION%.txt",
>>%genLPub3DUpdates%       "available-versions": "%VERSION%,%AVAILABLE_VERS_DMG%",  
>>%genLPub3DUpdates%       "alternate-version-2.0.19": {
>>%genLPub3DUpdates%         "open-url": "https://sourceforge.net/projects/lpub3d/files/2.0.19/",
>>%genLPub3DUpdates%      	 "latest-version": "2.0.19",
>>%genLPub3DUpdates%      	 "download-url": "http://lpub3d.sourceforge.net/LPub3D-UpdateMaster_2.0.19_osx.dmg",
>>%genLPub3DUpdates%      	 "changelog-url": "http://lpub3d.sourceforge.net/change_log_2.0.19.txt"
>>%genLPub3DUpdates%       }
>>%genLPub3DUpdates%     },
>>%genLPub3DUpdates%     "linux-deb": {
>>%genLPub3DUpdates%       "open-url": "https://sourceforge.net/projects/lpub3d/files/%VERSION%/",
>>%genLPub3DUpdates%       "latest-version": "%VERSION%",
>>%genLPub3DUpdates%       "download-url": "http://lpub3d.sourceforge.net/LPub3D-UpdateMaster_%VERSION%_0ubuntu1_amd64.deb",
>>%genLPub3DUpdates%       "changelog-url": "http://lpub3d.sourceforge.net/change_log_%VERSION%.txt",
>>%genLPub3DUpdates%       "available-versions": "%VERSION%,%AVAILABLE_VERS_DEB%",
>>%genLPub3DUpdates%       "alternate-version-2.0.19": {
>>%genLPub3DUpdates%         "open-url": "https://sourceforge.net/projects/lpub3d/files/2.0.19/",
>>%genLPub3DUpdates%      	 "latest-version": "2.0.19",
>>%genLPub3DUpdates%      	 "download-url": "http://lpub3d.sourceforge.net/LPub3D-UpdateMaster_2.0.19_0ubuntu1_amd64.deb",
>>%genLPub3DUpdates%      	 "changelog-url": "http://lpub3d.sourceforge.net/change_log_2.0.19.txt"
>>%genLPub3DUpdates%       }
>>%genLPub3DUpdates%     },
>>%genLPub3DUpdates%     "linux-rpm": {
>>%genLPub3DUpdates%       "open-url": "https://sourceforge.net/projects/lpub3d/files/%VERSION%/",
>>%genLPub3DUpdates%       "latest-version": "%VERSION%",
>>%genLPub3DUpdates%       "download-url": "http://lpub3d.sourceforge.net/LPub3D-UpdateMaster_%VERSION%_1fedora.x86_64.rpm",
>>%genLPub3DUpdates%       "changelog-url": "http://lpub3d.sourceforge.net/change_log_%VERSION%.txt",
>>%genLPub3DUpdates%       "available-versions": "%VERSION%,%AVAILABLE_VERS_DEB%",
>>%genLPub3DUpdates%       "alternate-version-2.0.19": {
>>%genLPub3DUpdates%         "open-url": "https://sourceforge.net/projects/lpub3d/files/2.0.19/",
>>%genLPub3DUpdates%      	 "latest-version": "2.0.19",
>>%genLPub3DUpdates%      	 "download-url": "http://lpub3d.sourceforge.net/LPub3D-UpdateMaster_2.0.19_1fedora.x86_64.rpm",
>>%genLPub3DUpdates%      	 "changelog-url": "http://lpub3d.sourceforge.net/change_log_2.0.19.txt"
>>%genLPub3DUpdates%       }
>>%genLPub3DUpdates%     },
>>%genLPub3DUpdates%     "linux-pkg": {
>>%genLPub3DUpdates%       "open-url": "https://sourceforge.net/projects/lpub3d/files/%VERSION%/",
>>%genLPub3DUpdates%       "latest-version": "%VERSION%",
>>%genLPub3DUpdates%       "download-url": "http://lpub3d.sourceforge.net/LPub3D-UpdateMaster_%VERSION%_x86_64.pkg.tar.xz",
>>%genLPub3DUpdates%       "changelog-url": "http://lpub3d.sourceforge.net/change_log_%VERSION%.txt",
>>%genLPub3DUpdates%       "available-versions": "%VERSION%,%AVAILABLE_VERS_DEB%",
>>%genLPub3DUpdates%       "alternate-version-2.0.19": {
>>%genLPub3DUpdates%         "open-url": "https://sourceforge.net/projects/lpub3d/files/2.0.19/",
>>%genLPub3DUpdates%      	 "latest-version": "2.0.19",
>>%genLPub3DUpdates%      	 "download-url": "http://lpub3d.sourceforge.net/LPub3D-UpdateMaster_2.0.19_x86_64.pkg.tar.xz",
>>%genLPub3DUpdates%      	 "changelog-url": "http://lpub3d.sourceforge.net/change_log_2.0.19.txt"
>>%genLPub3DUpdates%       }
>>%genLPub3DUpdates%     }
>>%genLPub3DUpdates%   }
>>%genLPub3DUpdates% }
>>%genLPub3DUpdates%.

ECHO. 																>>  %BuildLog%
ECHO - Generating lpub3dupdates.json version input file...			>>  %BuildLog%
ECHO. 	
ECHO - Generating lpub3dupdates.json version input file...

(
  FOR /F "tokens=*" %%i IN (..\..\utilities\json\lpub3dupdates.json) DO (
    IF "%%i" EQU ""alternate-version-%ALT_VER_EXE%": {" (
      TYPE ..\..\utilities\json\lastVersionInsert_Exe.txt
    )
    IF "%%i" EQU ""alternate-version-%ALT_VER_DMG%": {" (
      TYPE ..\..\utilities\json\lastVersionInsert_Dmg.txt
    )	
    IF "%%i" EQU ""alternate-version-%ALT_VER_DEB%": {" (
      TYPE ..\..\utilities\json\lastVersionInsert_Deb.txt
    )	
    IF "%%i" EQU ""alternate-version-%ALT_VER_RPM%": {" (
      TYPE ..\..\utilities\json\lastVersionInsert_Rpm.txt
    )	
    IF "%%i" EQU ""alternate-version-%ALT_VER_PKG%": {" (
      TYPE ..\..\utilities\json\lastVersionInsert_Pkg.txt
    )	
	ECHO %%i
  )
) >temp.txt
MOVE /y temp.txt ..\..\utilities\json\lpub3dupdates.json			>>  %BuildLog%
DEL /Q ..\..\utilities\json\lastVersionInsert_Exe.txt
DEL /Q ..\..\utilities\json\lastVersionInsert_Dmg.txt
DEL /Q ..\..\utilities\json\lastVersionInsert_Deb.txt
DEL /Q ..\..\utilities\json\lastVersionInsert_Rpm.txt
DEL /Q ..\..\utilities\json\lastVersionInsert_Pkg.txt

ECHO. 													 			>>  %BuildLog%
ECHO - Copying lpub3dupdats.json to media folder...    				>>  %BuildLog%
ECHO. 	
ECHO - Copying lpub3dupdates.json to media folder...

COPY /V /Y ..\..\utilities\json\lpub3dupdates.json ..\release\%VERSION%\Update\ /A						>>  %BuildLog%

ECHO.                                   																>>  %BuildLog%
ECHO - Generating latest.txt version input file (backgward compatability)...                            >>  %BuildLog%
ECHO.   
ECHO - Generating latest.txt version input file (backgward compatability)...

SET latestFile=..\release\%VERSION%\Update\latest.txt
SET genLatest=%latestFile% ECHO

:GENERATE latest.txt file
>%genLatest% %VERSION% 

ECHO. 																>>  %BuildLog%
ECHO - Generating AppVersion.nsh build input script...   			>>  %BuildLog%
ECHO. 	
ECHO - Generating AppVersion.nsh build input script...

SET versionFile=bin\AppVersion.nsh
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
>>%genVersion% !define CompleteVersion "%VERSION%.%VER_REVISION%.%VER_BUILD%_%YEAR%%MONTH%%DAY%"
>>%genVersion% ; ${CompleteVersion}
>>%genVersion%.	
>>%genVersion% !define Win32BuildDir "..\..\release\%VERSION%\%WIN32PRODUCTDIR%\%PRODUCT%_x86" 
>>%genVersion% ; ${Win32BuildDir}
>>%genVersion%.
>>%genVersion% !define Win64BuildDir "..\..\release\%VERSION%\%WIN64PRODUCTDIR%\%PRODUCT%_x86_64" 
>>%genVersion% ; ${Win64BuildDir}
>>%genVersion%.
>>%genVersion% !define LPub3DBuildFile "%LPUB3D_BUILD_FILE%"
>>%genVersion% ; ${LPub3DBuildFile}
>>%genVersion%.
>>%genVersion% !define QuaZipBuildFile "%QUAZIP_BUILD_FILE%"
>>%genVersion% ; ${QuaZipBuildFile}
>>%genVersion%.
>>%genVersion% !define LDrawIniBuildFile "%LDRAWINI_BUILD_FILE%"
>>%genVersion% ; ${LDrawIniBuildFile}
>>%genVersion%.	
>>%genVersion% !define BuildRevision "%VER_REVISION%" 
>>%genVersion% ; ${BuildRevision}
>>%genVersion%.	
>>%genVersion% !define BuildNumber "%VER_BUILD%" 
>>%genVersion% ; ${BuildNumber}
>>%genVersion%.	
>>%genVersion% !define BuildDate "%DATETIME%" 
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
>>%genVersion% !define SupportEmail %SUPPORT_EMAIL% %VERSION%.%VER_REVISION%.%VER_BUILD%_%YEAR%%MONTH%%DAY%"
>>%genVersion% ; ${SupportEmail}
>>%genVersion%.

ECHO. 																																			>>  %BuildLog%
ECHO - Copying %WIN32PRODUCTDIR% content to media folder...    																					>>  %BuildLog%
ECHO. 	
ECHO - Copying %WIN32PRODUCTDIR% content to media folder...

XCOPY /S /I /E /V /Y %devRootPath%\extras ..\release\%VERSION%\%WIN32PRODUCTDIR%\%PRODUCT%_x86\extras											>>  %BuildLog%
XCOPY /S /I /E /V /Y ..\..\utilities\ldrawlibraries ..\release\%VERSION%\%WIN32PRODUCTDIR%\%PRODUCT%_x86\libraries								>>  %BuildLog% 
XCOPY /S /I /E /V /Y ..\3rdParty ..\release\%VERSION%\%WIN32PRODUCTDIR%\%PRODUCT%_x86\3rdParty													>>  %BuildLog% 
XCOPY /S /I /E /V /Y %devRootPath%\docs ..\release\%VERSION%\%WIN32PRODUCTDIR%\%PRODUCT%_x86\docs												>>  %BuildLog% 
XCOPY /S /I /E /V /Y ..\..\utilities\icons ..\release\%VERSION%\%WIN32PRODUCTDIR%\%PRODUCT%_x86\icons											>>  %BuildLog%

COPY /V /Y %devRootPath%\docs\README.txt ..\release\%VERSION%\%WIN32PRODUCTDIR%\%PRODUCT%_x86\ /A                          						>>  %BuildLog%
COPY /V /Y %devRootPath%\docs\README.txt ..\release\%VERSION%\%WIN32PRODUCTDIR%\ /A                                        	  					>>  %BuildLog%

COPY /V /Y "%Win32LPub3DBuildFilePath%\%LPUB3D_BUILD_FILE%" ..\release\%VERSION%\%WIN32PRODUCTDIR%\%PRODUCT%_x86\ /B							>>  %BuildLog%
COPY /V /Y "%Win32QuaZIPBuildFilePath%\%QUAZIP_BUILD_FILE%" ..\release\%VERSION%\%WIN32PRODUCTDIR%\%PRODUCT%_x86\ /B                      		>>  %BuildLog%
COPY /V /Y "%Win32LDrawIniBuildFilePath%\%LDRAWINI_BUILD_FILE%" ..\release\%VERSION%\%WIN32PRODUCTDIR%\%PRODUCT%_x86\ /B                    	>>  %BuildLog%

COPY /V /Y "%Win32QtBinPath%\libgcc_s_dw2-1.dll" ..\release\%VERSION%\%WIN32PRODUCTDIR%\%PRODUCT%_x86\ /B       								>>  %BuildLog%
COPY /V /Y "%Win32QtBinPath%\libstdc++-6.dll" ..\release\%VERSION%\%WIN32PRODUCTDIR%\%PRODUCT%_x86\ /B          								>>  %BuildLog%
COPY /V /Y "%Win32QtBinPath%\libwinpthread-1.dll" ..\release\%VERSION%\%WIN32PRODUCTDIR%\%PRODUCT%_x86\ /B      								>>  %BuildLog%

COPY /V /Y %Win32QtBinPath%\Qt5Core.dll ..\release\%VERSION%\%WIN32PRODUCTDIR%\%PRODUCT%_x86\ /B                								>>  %BuildLog%
COPY /V /Y %Win32QtBinPath%\Qt5Gui.dll ..\release\%VERSION%\%WIN32PRODUCTDIR%\%PRODUCT%_x86\ /B                 								>>  %BuildLog%
COPY /V /Y %Win32QtBinPath%\Qt5Network.dll ..\release\%VERSION%\%WIN32PRODUCTDIR%\%PRODUCT%_x86\ /B             								>>  %BuildLog%
COPY /V /Y %Win32QtBinPath%\Qt5OpenGL.dll ..\release\%VERSION%\%WIN32PRODUCTDIR%\%PRODUCT%_x86\ /B              								>>  %BuildLog%
COPY /V /Y %Win32QtBinPath%\Qt5PrintSupport.dll ..\release\%VERSION%\%WIN32PRODUCTDIR%\%PRODUCT%_x86\ /B        								>>  %BuildLog%
COPY /V /Y %Win32QtBinPath%\Qt5Widgets.dll ..\release\%VERSION%\%WIN32PRODUCTDIR%\%PRODUCT%_x86\ /B             								>>  %BuildLog%

IF NOT EXIST "..\release\%VERSION%\%WIN32PRODUCTDIR%\%PRODUCT%_x86\bearer\" (
  MKDIR "..\release\%VERSION%\%WIN32PRODUCTDIR%\%PRODUCT%_x86\bearer\"
)
IF NOT EXIST "..\release\%VERSION%\%WIN32PRODUCTDIR%\%PRODUCT%_x86\iconengines\" (
  MKDIR "..\release\%VERSION%\%WIN32PRODUCTDIR%\%PRODUCT%_x86\iconengines\"
)
IF NOT EXIST "..\release\%VERSION%\%WIN32PRODUCTDIR%\%PRODUCT%_x86\imageformats\" (
  MKDIR "..\release\%VERSION%\%WIN32PRODUCTDIR%\%PRODUCT%_x86\imageformats\"
)
IF NOT EXIST "..\release\%VERSION%\%WIN32PRODUCTDIR%\%PRODUCT%_x86\platforms\" (
  MKDIR "..\release\%VERSION%\%WIN32PRODUCTDIR%\%PRODUCT%_x86\platforms\"
)
IF NOT EXIST "..\release\%VERSION%\%WIN32PRODUCTDIR%\%PRODUCT%_x86\printsupport\" (
  MKDIR "..\release\%VERSION%\%WIN32PRODUCTDIR%\%PRODUCT%_x86\printsupport\"
)

COPY /V /Y %Win32QtPluginPath%\bearer\qgenericbearer.dll ..\release\%VERSION%\%WIN32PRODUCTDIR%\%PRODUCT%_x86\bearer\ /B  						>>  %BuildLog%
COPY /V /Y %Win32QtPluginPath%\bearer\qnativewifibearer.dll ..\release\%VERSION%\%WIN32PRODUCTDIR%\%PRODUCT%_x86\bearer\ /B  					>>  %BuildLog%
COPY /V /Y %Win32QtPluginPath%\iconengines\qsvgicon.dll ..\release\%VERSION%\%WIN32PRODUCTDIR%\%PRODUCT%_x86\iconengines\ /B  					>>  %BuildLog%
COPY /V /Y %Win32QtPluginPath%\imageformats\qdds.dll ..\release\%VERSION%\%WIN32PRODUCTDIR%\%PRODUCT%_x86\imageformats\ /B  					>>  %BuildLog%
COPY /V /Y %Win32QtPluginPath%\imageformats\qgif.dll ..\release\%VERSION%\%WIN32PRODUCTDIR%\%PRODUCT%_x86\imageformats\ /B  					>>  %BuildLog%
COPY /V /Y %Win32QtPluginPath%\imageformats\qicns.dll ..\release\%VERSION%\%WIN32PRODUCTDIR%\%PRODUCT%_x86\imageformats\ /B  					>>  %BuildLog%
COPY /V /Y %Win32QtPluginPath%\imageformats\qico.dll ..\release\%VERSION%\%WIN32PRODUCTDIR%\%PRODUCT%_x86\imageformats\ /B  					>>  %BuildLog%
COPY /V /Y %Win32QtPluginPath%\imageformats\qjpeg.dll ..\release\%VERSION%\%WIN32PRODUCTDIR%\%PRODUCT%_x86\imageformats\ /B  					>>  %BuildLog%
COPY /V /Y %Win32QtPluginPath%\imageformats\qsvg.dll ..\release\%VERSION%\%WIN32PRODUCTDIR%\%PRODUCT%_x86\imageformats\ /B  					>>  %BuildLog%
COPY /V /Y %Win32QtPluginPath%\imageformats\qtga.dll ..\release\%VERSION%\%WIN32PRODUCTDIR%\%PRODUCT%_x86\imageformats\ /B  					>>  %BuildLog%
COPY /V /Y %Win32QtPluginPath%\imageformats\qtiff.dll ..\release\%VERSION%\%WIN32PRODUCTDIR%\%PRODUCT%_x86\imageformats\ /B  					>>  %BuildLog%
COPY /V /Y %Win32QtPluginPath%\imageformats\qwbmp.dll ..\release\%VERSION%\%WIN32PRODUCTDIR%\%PRODUCT%_x86\imageformats\ /B  					>>  %BuildLog%
COPY /V /Y %Win32QtPluginPath%\imageformats\qwebp.dll ..\release\%VERSION%\%WIN32PRODUCTDIR%\%PRODUCT%_x86\imageformats\ /B  					>>  %BuildLog%
COPY /V /Y %Win32QtPluginPath%\platforms\qwindows.dll ..\release\%VERSION%\%WIN32PRODUCTDIR%\%PRODUCT%_x86\platforms\ /B  						>>  %BuildLog%
COPY /V /Y %Win32QtPluginPath%\printsupport\windowsprintersupport.dll ..\release\%VERSION%\%WIN32PRODUCTDIR%\%PRODUCT%_x86\printsupport\ /B  	>>  %BuildLog%

ECHO. 																																			>>  %BuildLog%
ECHO - Copying %WIN64PRODUCTDIR% content to media folder...    																					>>  %BuildLog%
ECHO. 	
ECHO - Copying %WIN64PRODUCTDIR% content to media folder...

XCOPY /S /I /E /V /Y %devRootPath%\extras ..\release\%VERSION%\%WIN64PRODUCTDIR%\%PRODUCT%_x86_64\extras										>>  %BuildLog%
XCOPY /S /I /E /V /Y ..\..\utilities\ldrawlibraries ..\release\%VERSION%\%WIN64PRODUCTDIR%\%PRODUCT%_x86_64\libraries							>>  %BuildLog% 
XCOPY /S /I /E /V /Y ..\3rdParty ..\release\%VERSION%\%WIN64PRODUCTDIR%\%PRODUCT%_x86_64\3rdParty												>>  %BuildLog% 
XCOPY /S /I /E /V /Y %devRootPath%\docs ..\release\%VERSION%\%WIN64PRODUCTDIR%\%PRODUCT%_x86_64\docs											>>  %BuildLog% 
XCOPY /S /I /E /V /Y ..\..\utilities\icons ..\release\%VERSION%\%WIN64PRODUCTDIR%\%PRODUCT%_x86_64\icons										>>  %BuildLog%

COPY /V /Y %devRootPath%\docs\README.txt ..\release\%VERSION%\%WIN64PRODUCTDIR%\%PRODUCT%_x86_64\ /A                          					>>  %BuildLog%
COPY /V /Y %devRootPath%\docs\README.txt ..\release\%VERSION%\%WIN64PRODUCTDIR%\ /A                                        	  					>>  %BuildLog%

COPY /V /Y "%Win64LPub3DBuildFilePath%\%LPUB3D_BUILD_FILE%"  ..\release\%VERSION%\%WIN64PRODUCTDIR%\%PRODUCT%_x86_64\ /B							>>  %BuildLog%
COPY /V /Y "%Win64QuaZIPBuildFilePath%\%QUAZIP_BUILD_FILE%" ..\release\%VERSION%\%WIN64PRODUCTDIR%\%PRODUCT%_x86_64\ /B                      							>>  %BuildLog%
COPY /V /Y "%Win64LDrawIniBuildFilePath%\%LDRAWINI_BUILD_FILE%" ..\release\%VERSION%\%WIN64PRODUCTDIR%\%PRODUCT%_x86_64\ /B                    							>>  %BuildLog%

COPY /V /Y "%Win64QtBinPath%\libbz2-1.dll" ..\release\%VERSION%\%WIN64PRODUCTDIR%\%PRODUCT%_x86_64\ /B       									>>  %BuildLog%
COPY /V /Y "%Win64QtBinPath%\libfreetype-6.dll" ..\release\%VERSION%\%WIN64PRODUCTDIR%\%PRODUCT%_x86_64\ /B       								>>  %BuildLog%
COPY /V /Y "%Win64QtBinPath%\libgcc_s_seh-1.dll" ..\release\%VERSION%\%WIN64PRODUCTDIR%\%PRODUCT%_x86_64\ /B       								>>  %BuildLog%
COPY /V /Y "%Win64QtBinPath%\libglib-2.0-0.dll" ..\release\%VERSION%\%WIN64PRODUCTDIR%\%PRODUCT%_x86_64\ /B       								>>  %BuildLog%
COPY /V /Y "%Win64QtBinPath%\libgraphite2.dll" ..\release\%VERSION%\%WIN64PRODUCTDIR%\%PRODUCT%_x86_64\ /B       								>>  %BuildLog%
COPY /V /Y "%Win64QtBinPath%\libharfbuzz-0.dll" ..\release\%VERSION%\%WIN64PRODUCTDIR%\%PRODUCT%_x86_64\ /B       								>>  %BuildLog%
COPY /V /Y "%Win64QtBinPath%\libiconv-2.dll" ..\release\%VERSION%\%WIN64PRODUCTDIR%\%PRODUCT%_x86_64\ /B       									>>  %BuildLog%
COPY /V /Y "%Win64QtBinPath%\libicudt57.dll" ..\release\%VERSION%\%WIN64PRODUCTDIR%\%PRODUCT%_x86_64\ /B       									>>  %BuildLog%
COPY /V /Y "%Win64QtBinPath%\libicuin57.dll" ..\release\%VERSION%\%WIN64PRODUCTDIR%\%PRODUCT%_x86_64\ /B       									>>  %BuildLog%
COPY /V /Y "%Win64QtBinPath%\libicuuc57.dll" ..\release\%VERSION%\%WIN64PRODUCTDIR%\%PRODUCT%_x86_64\ /B       									>>  %BuildLog%
COPY /V /Y "%Win64QtBinPath%\libintl-8.dll" ..\release\%VERSION%\%WIN64PRODUCTDIR%\%PRODUCT%_x86_64\ /B       									>>  %BuildLog%
COPY /V /Y "%Win64QtBinPath%\libpcre-1.dll" ..\release\%VERSION%\%WIN64PRODUCTDIR%\%PRODUCT%_x86_64\ /B       									>>  %BuildLog%
COPY /V /Y "%Win64QtBinPath%\libpcre16-0.dll" ..\release\%VERSION%\%WIN64PRODUCTDIR%\%PRODUCT%_x86_64\ /B       								>>  %BuildLog%
COPY /V /Y "%Win64QtBinPath%\libpng16-16.dll" ..\release\%VERSION%\%WIN64PRODUCTDIR%\%PRODUCT%_x86_64\ /B       								>>  %BuildLog%
COPY /V /Y "%Win64QtBinPath%\libstdc++-6.dll" ..\release\%VERSION%\%WIN64PRODUCTDIR%\%PRODUCT%_x86_64\ /B       								>>  %BuildLog%
COPY /V /Y "%Win64QtBinPath%\libwinpthread-1.dll" ..\release\%VERSION%\%WIN64PRODUCTDIR%\%PRODUCT%_x86_64\ /B       							>>  %BuildLog%
COPY /V /Y "%Win64QtBinPath%\zlib1.dll" ..\release\%VERSION%\%WIN64PRODUCTDIR%\%PRODUCT%_x86_64\ /B       										>>  %BuildLog%

COPY /V /Y %Win64QtBinPath%\Qt5Core.dll ..\release\%VERSION%\%WIN64PRODUCTDIR%\%PRODUCT%_x86_64\ /B                								>>  %BuildLog%
COPY /V /Y %Win64QtBinPath%\Qt5Gui.dll ..\release\%VERSION%\%WIN64PRODUCTDIR%\%PRODUCT%_x86_64\ /B                 								>>  %BuildLog%
COPY /V /Y %Win64QtBinPath%\Qt5Network.dll ..\release\%VERSION%\%WIN64PRODUCTDIR%\%PRODUCT%_x86_64\ /B             								>>  %BuildLog%
COPY /V /Y %Win64QtBinPath%\Qt5OpenGL.dll ..\release\%VERSION%\%WIN64PRODUCTDIR%\%PRODUCT%_x86_64\ /B              								>>  %BuildLog%
COPY /V /Y %Win64QtBinPath%\Qt5PrintSupport.dll ..\release\%VERSION%\%WIN64PRODUCTDIR%\%PRODUCT%_x86_64\ /B        								>>  %BuildLog%
COPY /V /Y %Win64QtBinPath%\Qt5Widgets.dll ..\release\%VERSION%\%WIN64PRODUCTDIR%\%PRODUCT%_x86_64\ /B             								>>  %BuildLog%

IF NOT EXIST "..\release\%VERSION%\%WIN64PRODUCTDIR%\%PRODUCT%_x86_64\bearer\" (
  MKDIR "..\release\%VERSION%\%WIN64PRODUCTDIR%\%PRODUCT%_x86_64\bearer\"
)
IF NOT EXIST "..\release\%VERSION%\%WIN64PRODUCTDIR%\%PRODUCT%_x86_64\iconengines\" (
  MKDIR "..\release\%VERSION%\%WIN64PRODUCTDIR%\%PRODUCT%_x86_64\iconengines\"
)
IF NOT EXIST "..\release\%VERSION%\%WIN64PRODUCTDIR%\%PRODUCT%_x86_64\imageformats\" (
  MKDIR "..\release\%VERSION%\%WIN64PRODUCTDIR%\%PRODUCT%_x86_64\imageformats\"
)
IF NOT EXIST "..\release\%VERSION%\%WIN64PRODUCTDIR%\%PRODUCT%_x86_64\platforms\" (
  MKDIR "..\release\%VERSION%\%WIN64PRODUCTDIR%\%PRODUCT%_x86_64\platforms\"
)
IF NOT EXIST "..\release\%VERSION%\%WIN64PRODUCTDIR%\%PRODUCT%_x86_64\printsupport\" (
  MKDIR "..\release\%VERSION%\%WIN64PRODUCTDIR%\%PRODUCT%_x86_64\printsupport\"
)

COPY /V /Y %Win64QtPluginPath%\bearer\qgenericbearer.dll ..\release\%VERSION%\%WIN64PRODUCTDIR%\%PRODUCT%_x86_64\bearer\ /B  						>>  %BuildLog%
COPY /V /Y %Win64QtPluginPath%\bearer\qnativewifibearer.dll ..\release\%VERSION%\%WIN64PRODUCTDIR%\%PRODUCT%_x86_64\bearer\ /B  					>>  %BuildLog%
COPY /V /Y %Win64QtPluginPath%\iconengines\qsvgicon.dll ..\release\%VERSION%\%WIN64PRODUCTDIR%\%PRODUCT%_x86_64\iconengines\ /B  					>>  %BuildLog%
COPY /V /Y %Win64QtPluginPath%\imageformats\qdds.dll ..\release\%VERSION%\%WIN64PRODUCTDIR%\%PRODUCT%_x86_64\imageformats\ /B  						>>  %BuildLog%
COPY /V /Y %Win64QtPluginPath%\imageformats\qgif.dll ..\release\%VERSION%\%WIN64PRODUCTDIR%\%PRODUCT%_x86_64\imageformats\ /B  						>>  %BuildLog%
COPY /V /Y %Win64QtPluginPath%\imageformats\qicns.dll ..\release\%VERSION%\%WIN64PRODUCTDIR%\%PRODUCT%_x86_64\imageformats\ /B  					>>  %BuildLog%
COPY /V /Y %Win64QtPluginPath%\imageformats\qico.dll ..\release\%VERSION%\%WIN64PRODUCTDIR%\%PRODUCT%_x86_64\imageformats\ /B  						>>  %BuildLog%
COPY /V /Y %Win64QtPluginPath%\imageformats\qjpeg.dll ..\release\%VERSION%\%WIN64PRODUCTDIR%\%PRODUCT%_x86_64\imageformats\ /B  					>>  %BuildLog%
COPY /V /Y %Win64QtPluginPath%\imageformats\qsvg.dll ..\release\%VERSION%\%WIN64PRODUCTDIR%\%PRODUCT%_x86_64\imageformats\ /B  						>>  %BuildLog%
COPY /V /Y %Win64QtPluginPath%\imageformats\qtga.dll ..\release\%VERSION%\%WIN64PRODUCTDIR%\%PRODUCT%_x86_64\imageformats\ /B  						>>  %BuildLog%
COPY /V /Y %Win64QtPluginPath%\imageformats\qtiff.dll ..\release\%VERSION%\%WIN64PRODUCTDIR%\%PRODUCT%_x86_64\imageformats\ /B  					>>  %BuildLog%
COPY /V /Y %Win64QtPluginPath%\imageformats\qwbmp.dll ..\release\%VERSION%\%WIN64PRODUCTDIR%\%PRODUCT%_x86_64\imageformats\ /B  					>>  %BuildLog%
COPY /V /Y %Win64QtPluginPath%\imageformats\qwebp.dll ..\release\%VERSION%\%WIN64PRODUCTDIR%\%PRODUCT%_x86_64\imageformats\ /B  					>>  %BuildLog%
COPY /V /Y %Win64QtPluginPath%\platforms\qwindows.dll ..\release\%VERSION%\%WIN64PRODUCTDIR%\%PRODUCT%_x86_64\platforms\ /B  						>>  %BuildLog%
COPY /V /Y %Win64QtPluginPath%\printsupport\windowsprintersupport.dll ..\release\%VERSION%\%WIN64PRODUCTDIR%\%PRODUCT%_x86_64\printsupport\ /B  	>>  %BuildLog%

ECHO. 																																		>>  %BuildLog%
ECHO - Finished copying content to media folder...     																						>>  %BuildLog%
ECHO. 
ECHO - Finished copying content to media folder...

IF %RUN_NSIS% == 1 ECHO. 								        																			>>  %BuildLog%
IF %RUN_NSIS% == 1 ECHO - Start NSIS Master Update Build...  																				>>  %BuildLog%
IF %RUN_NSIS% == 1 ECHO.
IF %RUN_NSIS% == 1 ECHO - Start NSIS Master Update Build...

IF %RUN_NSIS% == 0 ECHO - Ignore NSIS Master Update Build...  																				>>  %BuildLog%
IF %RUN_NSIS% == 0 ECHO.
IF %RUN_NSIS% == 0 ECHO - Ignore NSIS Master Update Build...

IF %RUN_NSIS% == 1 %NSISExe% /DUpdateMaster bin\LPub3DNoPack.nsi 																				>> %BuildLog%

IF %RUN_NSIS% == 1 ECHO. 																													>>  %BuildLog%
IF %RUN_NSIS% == 1 ECHO - Finished NSIS Master Update 	Build...																			>>  %BuildLog%
IF %RUN_NSIS% == 1 ECHO.
IF %RUN_NSIS% == 1 ECHO - Finished NSIS Master Update  Build...

ECHO. 									   																									>>  %BuildLog%
IF %RUN_NSIS% == 1 ECHO - Start NSIS Manual Install Build... 																				>>  %BuildLog%
IF %RUN_NSIS% == 1 ECHO.
IF %RUN_NSIS% == 1 ECHO - Start NSIS Manual Install Build...

IF %RUN_NSIS% == 0 ECHO - Ignore NSIS Manual Install Build... 																				>>  %BuildLog%
IF %RUN_NSIS% == 0 ECHO.
IF %RUN_NSIS% == 0 ECHO - Ignore NSIS Manual Install Build...

IF %RUN_NSIS% == 1 COPY /V /Y ..\release\%PRODUCT%-UpdateMaster_%VERSION%.exe ..\release\%DOWNLOADPRODUCT%.exe  							>>  %BuildLog%
IF %RUN_NSIS% == 1 COPY /V /Y ..\release\%PRODUCT%-UpdateMaster_%VERSION%.exe ..\release\%PRODUCT%-UpdateMaster.exe  						>>  %BuildLog%

IF %RUN_NSIS% == 1 ECHO. 																													>>  %BuildLog%
IF %RUN_NSIS% == 1 ECHO - Finished NSIS Manual Install Build... 																			>>  %BuildLog%
IF %RUN_NSIS% == 1 ECHO.
IF %RUN_NSIS% == 1 ECHO - Finished NSIS Manual Install Build...

IF %RUN_NSIS% == 1 ECHO.												 																	>>  %BuildLog%
IF %RUN_NSIS% == 1 ECHO - Moving NSIS-generated %DOWNLOADPRODUCT%.exe to Download\ media folder...											>>  %BuildLog%
IF %RUN_NSIS% == 1 ECHO. 																													>>  %BuildLog%
IF %RUN_NSIS% == 1 ECHO - Moving NSIS-generated %PRODUCT%-UpdateMaster_%VERSION%.exe to Update\ media folder...								>>  %BuildLog%
IF %RUN_NSIS% == 1 ECHO.	
IF %RUN_NSIS% == 1 ECHO - Moving NSIS-generated %DOWNLOADPRODUCT%.exe to Download\ media folder...	
IF %RUN_NSIS% == 1 ECHO.	
IF %RUN_NSIS% == 1 ECHO - Moving NSIS-generated %PRODUCT%-UpdateMaster_%VERSION%.exe to Update\ media folder...	

IF %RUN_NSIS% == 0 ECHO - Ignore moving NSIS-generated %DOWNLOADPRODUCT%.exe to Download\ media folder...									>>  %BuildLog%
IF %RUN_NSIS% == 0 ECHO. 																													>>  %BuildLog%
IF %RUN_NSIS% == 0 ECHO - Ignore moving NSIS-generated %PRODUCT%-UpdateMaster_%VERSION%.exe to Update\ media folder...						>>  %BuildLog%
IF %RUN_NSIS% == 0 ECHO. 	
IF %RUN_NSIS% == 0 ECHO - Ignore moving NSIS-generated %DOWNLOADPRODUCT%.exe to Download\ media folder...	
IF %RUN_NSIS% == 0 ECHO. 			
IF %RUN_NSIS% == 0 ECHO - Ignore moving NSIS-generated %PRODUCT%-UpdateMaster_%VERSION%.exe to Update\ media folder...	

IF %RUN_NSIS% == 1 MOVE /Y ..\release\%DOWNLOADPRODUCT%.exe ..\release\%VERSION%\Download\													>>  %BuildLog%		
IF %RUN_NSIS% == 1 MOVE /Y ..\release\%PRODUCT%-UpdateMaster_%VERSION%.exe ..\release\%VERSION%\Update\										>>  %BuildLog%		
IF %RUN_NSIS% == 1 MOVE /Y ..\release\%PRODUCT%-UpdateMaster.exe ..\release\%VERSION%\Update\												>>  %BuildLog%		

ECHO. 									   																									>>  %BuildLog%
IF %SIGN_APP% == 1 ECHO - Start Application Code Signing Build... 																			>>  %BuildLog%
IF %SIGN_APP% == 1 ECHO.
IF %SIGN_APP% == 1 ECHO - Start Application Code Signing Build...

IF %SIGN_APP% == 0 ECHO - Ignore Application Code Signing Build... 																			>>  %BuildLog%
IF %SIGN_APP% == 0 ECHO.
IF %SIGN_APP% == 0 ECHO - Ignore Application Code Signing Build...

IF %SIGN_APP% == 1 %SignToolExe% sign /tr %TimeStamp% /td %Sha2% /fd %Sha2% /f %PKey% /p %PwD% ..\release\%VERSION%\Download\%DOWNLOADPRODUCT%.exe							>>  %BuildLog%
IF %SIGN_APP% == 1 %SignToolExe% sign /tr %TimeStamp% /td %Sha2% /fd %Sha2% /f %PKey% /p %PwD% ..\release\%VERSION%\Update\%PRODUCT%-UpdateMaster_%VERSION%.exe            	>>  %BuildLog%
IF %SIGN_APP% == 1 %SignToolExe% sign /tr %TimeStamp% /td %Sha2% /fd %Sha2% /f %PKey% /p %PwD% ..\release\%VERSION%\Update\%PRODUCT%-UpdateMaster.exe                      	>>  %BuildLog%
IF %SIGN_APP% == 1 %SignToolExe% sign /tr %TimeStamp% /td %Sha2% /fd %Sha2% /f %PKey% /p %PwD% ..\release\%VERSION%\%WIN32PRODUCTDIR%\%PRODUCT%_x86\%LPUB3D_BUILD_FILE%     >>  %BuildLog%
IF %SIGN_APP% == 1 %SignToolExe% sign /tr %TimeStamp% /td %Sha2% /fd %Sha2% /f %PKey% /p %PwD% ..\release\%VERSION%\%WIN64PRODUCTDIR%\%PRODUCT%_x86_64\%LPUB3D_BUILD_FILE%  >>  %BuildLog%

ECHO. 									   																									>>  %BuildLog%
IF %SIGN_APP% == 1 ECHO - Generating hash checksum listing... 																				>>  %BuildLog%
IF %SIGN_APP% == 1 ECHO.
IF %SIGN_APP% == 1 ECHO - Generating hash checksum listing...
                                      
IF %SIGN_APP% == 1 CertUtil -hashfile ..\release\%VERSION%\Download\%DOWNLOADPRODUCT%.exe SHA256							>>  ../release/%VERSION%\Download\LPub3D.%VERSION%.Checksums.txt
IF %SIGN_APP% == 1 CertUtil -hashfile ..\release\%VERSION%\Update\%PRODUCT%-UpdateMaster_%VERSION%.exe SHA256  				>>  ../release/%VERSION%\Download\LPub3D.%VERSION%.Checksums.txt
IF %SIGN_APP% == 1 CertUtil -hashfile ..\release\%VERSION%\Update\%PRODUCT%-UpdateMaster.exe SHA256  						>>  ../release/%VERSION%\Download\LPub3D.%VERSION%.Checksums.txt
IF %SIGN_APP% == 1 CertUtil -hashfile ..\release\%VERSION%\%WIN32PRODUCTDIR%\%PRODUCT%_x86\%LPUB3D_BUILD_FILE% SHA256		>>  ../release/%VERSION%\Download\LPub3D.%VERSION%.Checksums.txt
IF %SIGN_APP% == 1 CertUtil -hashfile ..\release\%VERSION%\%WIN64PRODUCTDIR%\%PRODUCT%_x86_64\%LPUB3D_BUILD_FILE% SHA256	>>  ../release/%VERSION%\Download\LPub3D.%VERSION%.Checksums.txt

ECHO. 									   																									>>  %BuildLog%
IF %SIGN_APP% == 1 ECHO - Finished Application Code Signing Build... 																		>>  %BuildLog%
IF %SIGN_APP% == 1 ECHO.
IF %SIGN_APP% == 1 ECHO - Finished Application Code Signing Build...

IF %CREATE_PORTABLE% == 1 ECHO. 																											>>  %BuildLog%
IF %CREATE_PORTABLE% == 1 ECHO - Create portable media zip files...		      																>>  %BuildLog%
IF %CREATE_PORTABLE% == 1 ECHO. 	
IF %CREATE_PORTABLE% == 1 ECHO - Create portable media zip files...

IF %CREATE_PORTABLE% == 0 ECHO. 																											>>  %BuildLog%
IF %CREATE_PORTABLE% == 0 ECHO - Ignore create portable media zip files...		    														>>  %BuildLog%
IF %CREATE_PORTABLE% == 0 ECHO. 	
IF %CREATE_PORTABLE% == 0 ECHO - Ignore create portable media zip files...

IF %CREATE_PORTABLE% == 1 %zipExe% a -tzip ..\release\%VERSION%\Download\%WIN32PRODUCTDIR%.zip ..\release\%VERSION%\%WIN32PRODUCTDIR%\ 		>>  %BuildLog%
IF %CREATE_PORTABLE% == 1 %zipExe% a -tzip ..\release\%VERSION%\Download\%WIN64PRODUCTDIR%.zip ..\release\%VERSION%\%WIN64PRODUCTDIR%\ 		>>  %BuildLog%

ECHO. 																																		>>  %BuildLog%
IF %CLEANUP% == 1 ECHO - Remove %PRODUCT% %VERSION% build files...																			>>  %BuildLog%
IF %CLEANUP% == 1 ECHO. 	
IF %CLEANUP% == 1 ECHO - Remove %PRODUCT% %VERSION% build files...

IF %CLEANUP% == 0 ECHO - Ignore remove %PRODUCT% %VERSION% build files...																	>>  %BuildLog%
IF %CLEANUP% == 0 ECHO. 	
IF %CLEANUP% == 0 ECHO - Ignore remove %PRODUCT% %VERSION% build files...

IF %CLEANUP% == 1 RD /Q /S ..\release\%VERSION%\%WIN32PRODUCTDIR%\ ..\release\%VERSION%\%WIN64PRODUCTDIR%\ 									>>  %BuildLog%	

ECHO. 																																		>>  %BuildLog%
ECHO - Finished.																															>>  %BuildLog%
ECHO. 	
ECHO - Finished.

ECHO.
ECHO If everything went well Press any key to EXIT!
%SystemRoot%\explorer.exe "C:\Users\Trevor\Downloads\LEGO\LPub\project\LPub\LPub3D\builds\windows\release\%VERSION%"
PAUSE >NUL
ENDLOCAL
EXIT /b 0

:SetVersion
SET /a n+=1
SET "ln=%*"
SETLOCAL enableDelayedExpansion
SET "ln=!ln:*:=!"
IF %n%==%targetln% (ECHO %productversion% >> %file%) ELSE ECHO(!ln! >> %file%
REM IF %n%==%targetln% (ECHO %productversion% >> %file%) ELSE ECHO %* >> %file%
ENDLOCAL
EXIT /b 0

:LDrawArchiveLibraryDownload
ECHO. 																		>>  %BuildLog%
ECHO - Prepare BATCH to VBS to Web Content Downloader...					>>  %BuildLog%
ECHO. 
ECHO - Prepare BATCH to VBS to Web Content Downloader...

SET OfficialCONTENT=complete.zip
SET UnofficialCONTENT=ldrawunf.zip
SET Lpub3dCONTENT=lpub3dldrawunf.zip
SET OutputPATH=..\..\utilities\ldrawlibraries\

IF NOT EXIST "%TEMP%\$" (
  MD "%TEMP%\$"
)

SET vbs=WebContentDownload.vbs
SET t=%TEMP%\$\%vbs% ECHO

IF EXIST %TEMP%\$\%vbs% (
 DEL %TEMP%\$\%vbs%
)

:WEB CONTENT SAVE-AS Download-- VBS
>%t% Option Explicit
>>%t% On Error Resume Next
>>%t%.
>>%t% Dim args, http, fileSystem, adoStream, url, target, status
>>%t%.
>>%t% Set args = Wscript.Arguments
>>%t% Set http = CreateObject("WinHttp.WinHttpRequest.5.1")
>>%t% url = args(0)
>>%t% target = args(1)
>>%t% WScript.Echo "- Getting '" ^& target ^& "' from '" ^& url ^& "'...", vbLF
>>%t%.
>>%t% http.Open "GET", url, False
>>%t% http.Send
>>%t% status = http.Status
>>%t%.
>>%t% If status ^<^> 200 Then
>>%t% WScript.Echo "- FAILED to download: HTTP Status " ^& status, vbLF
>>%t% WScript.Quit 1
>>%t% End If
>>%t%.
>>%t% Set adoStream = CreateObject("ADODB.Stream")
>>%t% adoStream.Open
>>%t% adoStream.Type = 1
>>%t% adoStream.Write http.ResponseBody
>>%t% adoStream.Position = 0
>>%t%.
>>%t% Set fileSystem = CreateObject("Scripting.FileSystemObject")
>>%t% If fileSystem.FileExists(target) Then fileSystem.DeleteFile target
>>%t% If Err.Number ^<^> 0 Then
>>%t%   WScript.Echo "- Error - CANNOT DELETE: '" ^& target ^& "', " ^& Err.Description
>>%t%   WScript.Echo " The file may be in use by another process.", vbLF
>>%t%   adoStream.Close
>>%t%   Err.Clear
>>%t% Else
>>%t%  adoStream.SaveToFile target
>>%t%  adoStream.Close
>>%t%  WScript.Echo "- Download successful!"
>>%t% End If
>>%t%.
>>%t% 'WebContentDownload.vbs
>>%t% 'Title: BATCH to VBS to Web Content Downloader
>>%t% 'CMD ^> cscript //Nologo %TEMP%\$\%vbs% WebNAME WebCONTENT
>>%t% 'VBS Created on %date% at %time%
>>%t%.

ECHO.																		>>  %BuildLog%
ECHO - VBS file "%vbs%" is done compiling.									>>  %BuildLog%
ECHO.
ECHO - VBS file "%vbs%" is done compiling.
ECHO.																		>>  %BuildLog%
ECHO - LDraw archive library download path: %OutputPATH%.					>>  %BuildLog%
ECHO.
ECHO - LDraw archive library download path: %OutputPATH%.

SET LibraryOPTION=Unofficial
SET WebCONTENT="%~dp0%UnofficialCONTENT%"
SET WebNAME=http://www.ldraw.org/library/unofficial/ldrawunf.zip

ECHO.																		>>  %BuildLog%
ECHO - Download LDraw %LibraryOPTION% library archive...					>>  %BuildLog%
ECHO.
ECHO - Download LDraw %LibraryOPTION% library archive...

ECHO.																		>>  %BuildLog%
ECHO - Web URL: "%WebNAME%" 												>>  %BuildLog%
ECHO.																		>>  %BuildLog%
ECHO - Download file: %WebCONTENT%											>>  %BuildLog%
ECHO.
ECHO - Web URL: "%WebNAME%" 
ECHO.
ECHO - Download file: %WebCONTENT%

IF EXIST %WebCONTENT% (
 DEL %WebCONTENT%
)

rem @ECHO on
ECHO.
cscript //Nologo %TEMP%\$\%vbs% %WebNAME% %WebCONTENT% && @ECHO off
IF "%LibraryOPTION%" EQU "Unofficial" (
	ECHO.																					>>  %BuildLog%
	ECHO - Rename file %UnofficialCONTENT% to %Lpub3dCONTENT% and move to %outputPath%		>>  %BuildLog%
	ECHO.
	ECHO - Rename file %UnofficialCONTENT% to %Lpub3dCONTENT% and move to %outputPath% 			
	REN %UnofficialCONTENT% %Lpub3dCONTENT%									
	MOVE /y .\%Lpub3dCONTENT% %outputPath%									>>  %BuildLog%
)
ECHO.																		>>  %BuildLog%
ECHO - LDraw archive library %UnofficialCONTENT% downloaded					>>  %BuildLog%
ECHO.
ECHO - LDraw archive library %UnofficialCONTENT% downloaded

SET LibraryOPTION=Official
SET WebCONTENT="%~dp0%OfficialCONTENT%"
SET WebNAME=http://www.ldraw.org/library/updates/complete.zip

ECHO.																		>>  %BuildLog%
ECHO - Download LDraw %LibraryOPTION% library archive...					>>  %BuildLog%
ECHO.
ECHO - Download LDraw %LibraryOPTION% library archive...

ECHO.																		>>  %BuildLog%
ECHO - Web URL: "%WebNAME%" 												>>  %BuildLog%
ECHO.																		>>  %BuildLog%
ECHO - Download file: %WebCONTENT%											>>  %BuildLog%
ECHO.
ECHO - Web URL: "%WebNAME%" 
ECHO.
ECHO - Download file: %WebCONTENT%

IF EXIST %WebCONTENT% (
 DEL %WebCONTENT%
)

rem @ECHO on
ECHO.
cscript //Nologo %TEMP%\$\%vbs% %WebNAME% %WebCONTENT% && @ECHO off
ECHO.																		>>  %BuildLog%
ECHO - Move file %OfficialCONTENT% to %outputPath%							>>  %BuildLog%
ECHO.
ECHO - Move file %OfficialCONTENT% to %outputPath% 
MOVE /y .\%OfficialCONTENT% %outputPath%									>>  %BuildLog%

ECHO.																		>>  %BuildLog%
ECHO - LDraw archive library %OfficialCONTENT% downloaded					>>  %BuildLog%
ECHO.																		>>  %BuildLog%
ECHO - LDraw archive libraries download finshed								>>  %BuildLog%
ECHO.
ECHO - LDraw archive library %OfficialCONTENT% downloaded
ECHO.
ECHO - LDraw archive libraries download finshed
ENDLOCAL
EXIT /b 0