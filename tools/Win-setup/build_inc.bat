@ECHO on
Title Configure Application Version Perimeters for NSIS Deployment
SETLOCAL
@break off
@color 0a

set BUILD=unknown
set COMPANY=unknown
set PRODUCT=unknown
set VERSION=unknown
set FILENAME=unknown
set PUBLISHER=unknown
set COMPANYURL=unknown
set REVISION_CMS=unknown
set REVISION_FILE=unknown

rem set devRootPath="../../mainApp"
set devRootPath="C:\Users\Trevor\Downloads\LEGO\LPub\project\LPub\LPub3D\mainApp"
rem set versionFile=../tools/Win-setup/AppVersion.nsh
set versionFile="C:\Users\Trevor\Downloads\LEGO\LPub\project\LPub\LPub3D\tools\Win-setup\AppVersion.nsh"

set buildFile=build.h

set genVersion=%versionFile% ECHO
set genBuild=%buildFile% ECHO

cd /D %devRootPath%

FOR /F "tokens=3*" %%i IN ('FINDSTR /c:"#define _BUILD_NUMBER" build.h') DO SET BUILD=%%i
FOR /F "tokens=3*" %%i IN ('FINDSTR /c:"#define _BUILD_REVISION" build.h') DO SET REVISION_FILE=%%i
FOR /F "tokens=2"  %%i IN ('svn info -r HEAD ^| FINDSTR Revision:') DO SET REVISION_CMS=%%i

SET BUILD=%BUILD:"=%
SET REVISION_FILE=%REVISION_FILE:"=%
IF %REVISION_CMS% EQU %REVISION_FILE% (
	SET /a BUILD= %BUILD%+1 
) ELSE (
	SET /a BUILD=1
)

FOR /F "tokens=3*" %%i IN ('FINDSTR /c:"#define VER_COMPANYNAME_STR" version.h') DO SET COMPANY=%%i %%j
FOR /F "tokens=3*" %%i IN ('FINDSTR /c:"#define VER_PRODUCTNAME_STR" version.h') DO SET PRODUCT=%%i
FOR /F "tokens=3*" %%i IN ('FINDSTR /c:"#define VER_BUILDINCVERSION_STR" version.h') DO SET VERSION=%%i
FOR /F "tokens=3*" %%i IN ('FINDSTR /c:"#define VER_ORIGINALFILENAME_STR" version.h') DO SET FILENAME=%%i
FOR /F "tokens=3*" %%i IN ('FINDSTR /c:"#define VER_PUBLISHER_STR" version.h') DO SET PUBLISHER=%%i %%j
FOR /F "tokens=3*" %%i IN ('FINDSTR /c:"#define VER_COMPANYDOMAIN_STR" version.h') DO SET COMPANYURL=%%i

:GENERATE AppVersion.nsh file
>%genVersion% !define Company %COMPANY% 
>>%genVersion% ; ${Company} 	
>>%genVersion%.	
>>%genVersion% !define ProductName %PRODUCT% 
>>%genVersion% ; ${ProductName}
>>%genVersion%.	
>>%genVersion% !define Version %VERSION% 
>>%genVersion% ; ${Version}
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
>>%genVersion% !define Publisher %PUBLISHER% 
>>%genVersion% ; ${Publisher}
>>%genVersion%.	
>>%genVersion% !define CompanyURL %COMPANYURL% 
>>%genVersion% ; ${CompanyURL}
>>%genVersion%.

:GENERATE build.h file
>%genBuild% #ifndef BUILD_H 
>>%genBuild% #define BUILD_H 
>>%genBuild%. 
>>%genBuild% #define _BUILD_REVISION "%REVISION_CMS%" 
>>%genBuild% #define _BUILD_NUMBER "%BUILD%"
>>%genBuild%.
>>%genBuild% #endif
>>%genBuild%.

rem Debug
rem PAUSE
ENDLOCAL