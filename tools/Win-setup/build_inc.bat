@ECHO on
Title Configure Application Version Perimeters for NSIS Deployment
SETLOCAL
@break off
@color 0a

set BUILD=unknown
set VER_SP=unknown
set COMPANY=unknown
set PRODUCT=unknown
set VERSION=unknown
set COMMENTS=unknown
set FILENAME=unknown
set VER_MAJOR=unknown
set VER_MINOR=unknown
set PUBLISHER=unknown
set COMPANYURL=unknown
set BUILD_DATE=unknown
set REVISION_CMS=unknown
set REVISION_FILE=unknown
set SUPPORT_EMAIL=unknown

set devRootPath="../../mainApp"
set versionFile=../tools/Win-setup/AppVersion.nsh

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
FOR /F "tokens=3*" %%i IN ('FINDSTR /c:"#define VER_MAJOR" version.h') DO SET VER_MAJOR=%%i
FOR /F "tokens=3*" %%i IN ('FINDSTR /c:"#define VER_MINOR" version.h') DO SET VER_MINOR=%%i
FOR /F "tokens=3*" %%i IN ('FINDSTR /c:"#define VER_SP" version.h') DO SET VER_SP=%%i
FOR /F "tokens=3*" %%i IN ('FINDSTR /c:"#define VER_ORIGINALFILENAME_STR" version.h') DO SET FILENAME=%%i
FOR /F "tokens=3*" %%i IN ('FINDSTR /c:"#define VER_PUBLISHER_STR" version.h') DO SET PUBLISHER=%%i %%j
FOR /F "tokens=3*" %%i IN ('FINDSTR /c:"#define VER_COMPANYDOMAIN_STR" version.h') DO SET COMPANYURL=%%i
FOR /F "tokens=3*" %%i IN ('FINDSTR /c:"#define VER_FILEDESCRIPTION_STR" version.h') DO SET COMMENTS=%%i %%j
FOR /F "tokens=3*" %%i IN ('FINDSTR /c:"#define VER_PUBLISHER_SUPPORT_EMAIL_STR" version.h') DO SET SUPPORT_EMAIL=%%i

SET VERSION=%VER_MAJOR%.%VER_MINOR%.%VER_SP%

SET HOUR=%TIME:~0,2%
IF "%HOUR:~0,1%" == " " SET HOUR=0%HOUR:~1,1%
ECHO HOUR=%HOUR%
SET MIN=%TIME:~3,2%
IF "%MIN:~0,1%" == " " SET MIN=0%MIN:~1,1%
ECHO MIN=%MIN%
SET SECS=%TIME:~6,2%
IF "%SECS:~0,1%" == " " SET SECS=0%SECS:~1,1%
ECHO SECS=%SECS%

SET YEAR=%DATE:~-4%
ECHO YEAR=%YEAR%
SET MONTH=%DATE:~3,2%
IF "%MONTH:~0,1%" == " " SET MONTH=0%MONTH:~1,1%
ECHO MONTH=%MONTH%
SET DAY=%DATE:~0,2%
IF "%DAY:~0,1%" == " " SET DAY=0%DAY:~1,1%
ECHO DAY=%DAY%

SET DATETIMEf=%YEAR% %MONTH% %DAY% %HOUR%:%MIN%:%SECS%

:GENERATE AppVersion.nsh file
>%genVersion% !define Company %COMPANY% 
>>%genVersion% ; ${Company} 	
>>%genVersion%.	
>>%genVersion% !define ProductName %PRODUCT% 
>>%genVersion% ; ${ProductName}
>>%genVersion%.	
>>%genVersion% !define Version "%VERSION%"
>>%genVersion% ; ${Version}
>>%genVersion%.	
>>%genVersion% !define CompleteVersion "%VERSION%.%REVISION_CMS%.%BUILD%_%YEAR%%MONTH%%DAY%"
>>%genVersion% ; ${CompleteVersion}
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

:GENERATE build.h file
>%genBuild%  /****************************************************************************
>>%genBuild% **
>>%genBuild% ** Copyright (C) 2015 Trevor SANDY. All rights reserved.
>>%genBuild% **
>>%genBuild% ** This file may be used under the terms of the
>>%genBuild% ** GNU General Public Liceense (GPL) version 3.0
>>%genBuild% ** which accompanies this distribution, and is
>>%genBuild% ** available at http://www.gnu.org/licenses/gpl.html
>>%genBuild% **
>>%genBuild% ** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
>>%genBuild% ** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
>>%genBuild% **
>>%genBuild%.
>>%genBuild% #ifndef BUILD_H 
>>%genBuild% #define BUILD_H 
>>%genBuild%. 
>>%genBuild% #define _BUILD_REVISION "%REVISION_CMS%" 
>>%genBuild% #define _BUILD_NUMBER "%BUILD%"
>>%genBuild% #define _BUILD_DATE "%DATETIMEf%"
>>%genBuild%.
>>%genBuild% #endif
>>%genBuild%.

rem Debug
rem PAUSE
ENDLOCAL