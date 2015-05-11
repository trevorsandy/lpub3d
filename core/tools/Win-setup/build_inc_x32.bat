@ECHO on
Title Get SVN build number from head
SETLOCAL
@break off
@color 0a

set ARCH=x32
set BUILD=unknown
set COMPANY=unknown
set PRODUCT=unknown
set VERSION=unknown
set FILENAME=unknown
set PUBLISHER=unknown
set COMPANYURL=unknown
set REVISION_CMS=unknown
set REVISION_FILE=unknown

set devRootPath="../../"

set versionFile=./tools/Win-setup/AppVersion.nsh
set buildFile=build.h

set genVersion=%versionFile% ECHO
set genBuild=%buildFile% ECHO

cd /D %devRootPath%

FOR /F "tokens=3*" %%i IN ('FINDSTR /c:"#define _BUILD" build.h') DO SET BUILD=%%i
FOR /F "tokens=3*" %%i IN ('FINDSTR /c:"#define _REVISION" build.h') DO SET REVISION_FILE=%%i
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
FOR /F "tokens=3*" %%i IN ('FINDSTR /c:"#define VER_TEXT" version.h') DO SET VERSION=%%i
FOR /F "tokens=3*" %%i IN ('FINDSTR /c:"#define VER_PUBLISHER_STR" version.h') DO SET PUBLISHER=%%i %%j
FOR /F "tokens=3*" %%i IN ('FINDSTR /c:"#define VER_COMPANYDOMAIN_STR" version.h') DO SET COMPANYURL=%%i

SET FILENAME=%PRODUCT%
SET FILENAME=%FILENAME:"=%
SET FILENAME="%FILENAME%_%ARCH%.exe"

:GENERATE AppVersion.nsh file
>%genVersion% !define Arch "%ARCH%" 
>>%genVersion% ; ${Arch} 	
>>%genVersion%.	
>>%genVersion% !define Company %COMPANY% 
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
>>%genVersion% !define RevisionNumber "%REVISION_CMS%" 
>>%genVersion% ; ${RevisionNumber}
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
>>%genBuild% #define _REVISION "%REVISION_CMS%" 
>>%genBuild% #define _BUILD "%BUILD%"
>>%genBuild%.
>>%genBuild% #endif
>>%genBuild%.

rem Debug
rem PAUSE
ENDLOCAL