@ECHO on
Title Generate build.h 
SETLOCAL
@break off
@color 0a

SET BUILD=unknown
SET REVISION_CMS=unknown
SET REVISION_FILE=unknown

SET devRootPath="..\..\mainApp"

SET buildFile=build.h
set genBuild=%buildFile% ECHO

CD /D %devRootPath%

FOR /F "tokens=3*" %%i IN ('FINDSTR /c:"#define _BUILD_NUMBER" build.h') DO SET BUILD=%%i
FOR /F "tokens=2"  %%i IN ('svn info -r HEAD ^| FINDSTR Revision:') DO SET REVISION_CMS=%%i
FOR /F "tokens=3*" %%i IN ('FINDSTR /c:"#define _BUILD_REVISION" build.h') DO SET REVISION_FILE=%%i

SET BUILD=%BUILD:"=%
SET REVISION_FILE=%REVISION_FILE:"=%
IF %REVISION_CMS% EQU %REVISION_FILE% (
	SET /a BUILD= %BUILD%+1 
) ELSE (
	SET /a BUILD=1
)

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

:GENERATE build.h file
>%genBuild% /****************************************************************************
>>%genBuild% **
>>%genBuild% ** Copyright (C) 2016 Trevor SANDY. All rights reserved.
>>%genBuild% **
>>%genBuild% ** This file may be used under the terms of the
>>%genBuild% ** GNU General Public Liceense (GPL) version 3.0
>>%genBuild% ** which accompanies this distribution, and is
>>%genBuild% ** available at http://www.gnu.org/licenses/gpl.html
>>%genBuild% **
>>%genBuild% ** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
>>%genBuild% ** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
>>%genBuild% **
>>%genBuild% ****************************************************************************/
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