@ECHO off
Title Get SVN build number from head
SETLOCAL
@break off
@color 0a

set COMPANY=unknown
set PRODUCT=unknown
set VERSION=unknown
set FILENAME=unknown
set PUBLISHER=unknown
set COMPANYURL=unknown
set REVISION=unknown

set devRootPath="../../"
set versionFile=./tools/Win-setup/AppVersion.nsh
set buildFile=build.h

set genVersion=%versionFile% ECHO
set genBuild=%buildFile% ECHO

set /p BUILD= < build.d 
set /a BUILD= %BUILD%+1 
ECHO %BUILD% > build.d

cd /D %devRootPath%

FOR /F "tokens=3*" %%i IN ('FINDSTR VER_COMPANYNAME_STR version.h') DO SET COMPANY=%%i %%j
FOR /F "tokens=3*" %%i IN ('FINDSTR VER_PRODUCTNAME_STR version.h') DO SET PRODUCT=%%i %%j
FOR /F "tokens=3*" %%i IN ('FINDSTR VER_TEXT version.h') DO SET VERSION=%%i
FOR /F "tokens=3*" %%i IN ('FINDSTR VER_ORIGINALFILENAME_STR version.h') DO SET FILENAME=%%i
FOR /F "tokens=3*" %%i IN ('FINDSTR VER_PUBLISHER_STR version.h') DO SET PUBLISHER=%%i %%j
FOR /F "tokens=3*" %%i IN ('FINDSTR VER_COMPANYDOMAIN_STR version.h') DO SET COMPANYURL=%%i
FOR /F "tokens=2"  %%i IN ('svn info -r HEAD ^| FINDSTR Revision:') DO SET REVISION=%%i

:GENERATE AppVersion.nsh file
>%genVersion% !define Company %COMPANY% 					
>>%genVersion% !define ProductName %PRODUCT% 
>>%genVersion% !define Version %VERSION% 
>>%genVersion% !define FileName %FILENAME%
>>%genVersion% !define RevisionNumber "%REVISION%" 
>>%genVersion% !define BuildNumber "%BUILD%" 
>>%genVersion% !define Publisher %PUBLISHER% 
>>%genVersion% !define CompanyURL %COMPANYURL% 
>>%genVersion%.

:GENERATE build.h file
>%genBuild% #ifndef BUILD_H 
>>%genBuild% #define BUILD_H 
>>%genBuild%. 
>>%genBuild% #define _BUILD %BUILD%
>>%genBuild% #define _REVISION %REVISION%
>>%genBuild%.
>>%genBuild% #endif
>>%genBuild%.

rem Debug
rem notepad ./tools/Win-setup/AppVersion.nsh
rem PAUSE
ENDLOCAL