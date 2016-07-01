@echo off
Title Download LDraw Archives
rem --
rem  README 
rem  Change the options below by rem/unrem'ing 
rem  the code you want to execute as necessary
rem --
SETLOCAL
@break off
@color 0a
@cls

rem Alt path (from within Win-setup): ..\release\libraries\
SET OutputPATH=..\release\libraries\
rem SET OutputPATH=%~dp0

SET OfficialCONTENT=complete.zip
SET UnofficialCONTENT=ldrawunf.zip
SET Lpub3dCONTENT=lpub3dldrawunf.zip

rem To Download Official LDraw Archive unremark the following:
rem SET LibraryOPTION=Official
rem SET WebCONTENT="%~dp0%OfficialCONTENT%"
rem SET WebNAME=http://www.ldraw.org/library/updates/complete.zip

rem To Download Unofficial LDraw Archive unremark the following:
SET LibraryOPTION=Unofficial
SET WebCONTENT="%~dp0%UnofficialCONTENT%"
SET WebNAME=http://www.ldraw.org/library/unofficial/ldrawunf.zip

ECHO -Start Download LDraw %LibraryOPTION% Library Archive
ECHO.
ECHO.
ECHO -This file will download the most recent LDraw %LibraryOPTION% Library Archive.
ECHO.

SET /p OutputPATH=-Enter your LDraw directory or hit Enter to accept default [%OutputPATH%]: 
ECHO.
ECHO -You entered output path %OutputPATH%.
ECHO.
ECHO -Press Enter to continue.
PAUSE >NUL

rem can test here for entered zip exe then terminate if no suitable location found
GOTO FINISHED_SETUP

:FINISHED_SETUP
ECHO.
ECHO -LDraw path entered: %OutputPATH%

ECHO.
ECHO -Download archives from LDraw.org

IF NOT EXIST "%TEMP%\$" (
  MD "%TEMP%\$"
)
SET n=WebContentDownload.vbs
SET t=%TEMP%\$\%n% ECHO

IF EXIST %TEMP%\$\%n% (
 DEL %TEMP%\$\%n%
)
IF EXIST %WebCONTENT% (
 DEL %WebCONTENT%
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
>>%t% WScript.Echo vbLF
>>%t% WScript.Echo "-Getting '" ^& target ^& "' from '" ^& url ^& "'...", vbLF
>>%t%.
>>%t% http.Open "GET", url, False
>>%t% http.Send
>>%t% status = http.Status
>>%t%.
>>%t% If status ^<^> 200 Then
>>%t% WScript.Echo "-FAILED to download: HTTP Status " ^& status, vbLF
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
>>%t%   WScript.Echo "-Error - CANNOT DELETE: '" ^& target ^& "', " ^& Err.Description
>>%t%   WScript.Echo " The file may be in use by another process.", vbLF
>>%t%   adoStream.Close
>>%t%   Err.Clear
>>%t% Else
>>%t%  adoStream.SaveToFile target
>>%t%  adoStream.Close
>>%t%  WScript.Echo "-Download successful!"
>>%t% End If
>>%t%.
>>%t% 'WebContentDownload.vbs
>>%t% 'Title: BATCH to VBS to Web Content Downloader
>>%t% 'CMD ^> cscript //Nologo %TEMP%\$\%n% %WebNAME% %WebCONTENT%
>>%t% 'VBS Created on %date% at %time%
>>%t%.

ECHO. 
ECHO -BATCH to VBS to Web Content Downloader
ECHO.
ECHO -File "%n%" is done compiling.
ECHO.
ECHO -WEB URL: "%WebNAME%" 
ECHO.
ECHO -DOWNLOAD FILE: %WebCONTENT%
ECHO.
ECHO.
ECHO -If you are ready to continue and download the content, Press any key!
PAUSE >NUL
@echo on
cscript //Nologo %TEMP%\$\%n% %WebNAME% %WebCONTENT% && @echo off
ECHO.
IF "%LibraryOPTION%" EQU "Unofficial" (
	ECHO -Rename file %UnofficialCONTENT% to %Lpub3dCONTENT% and move to %outputPath%
	REN %UnofficialCONTENT% %Lpub3dCONTENT%
	MOVE /y .\%Lpub3dCONTENT% %outputPath%
)
ECHO.
ECHO -Finshed
ECHO.
ECHO -If everything went well then, Press any key to EXIT!
PAUSE >NUL
ENDLOCAL
EXIT

