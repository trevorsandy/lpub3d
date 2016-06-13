@echo off
Title Create LPub3DViewer unofficial library archive
rem --
rem  README
rem  How to setup LPub3DViewer libraries for LPub 3D Viewer
rem	 1. create a sub-directory in your LDRAW directory and name it LPub3DViewer-Library.
rem  2. place this file in your LDRAW directory and run it (double-click).
rem  3. review command window and "LDRAW directory"\LPub3DViewerLibrary-archive-log.txt details
rem  4. copy and paste (or download from LDraw.org) complete.zip to LPub3DViewer-Library
rem --
SETLOCAL
@break off
@color 0a
@cls
ECHO -Start Create LDraw Unofficial Library Archive
ECHO.

ECHO.
ECHO This file will download the most recent LDraw unofficial library archive.
ECHO It will then integrate it with your current LDraw unofficial library archive
ECHO updating the content in the downloaded library archive (zip) file. The updated
ECHO unofficial archive will be stored in your LPub3DViewer-Library subdirectory
ECHO under your LDraw directory.
ECHO.

set LDRAWDIR=%~dp0
set /p LDRAWDIR=Enter your LDraw directory or hit Enter to accept default [%LDRAWDIR%]: 
ECHO.
ECHO -You entered "%LDRAWDIR%" which will also be your working directory.
ECHO -Press Enter to continue.
PAUSE >NUL

set ldrawPath="%LDRAWDIR%"
set outputPath="%LDRAWDIR%"

set zipWin64=C:\Program files\7-zip
set zipWin32=C:\Program Files (x86)\7-zip

set zipExe=unknown

if exist "%zipWin64%" (
	ECHO.
	set zipExe="%zipWin64%\7z.exe"
    ECHO -Found zip exe at "%zipWin64%"
    GOTO FINISHED_SETUP
) 

ECHO.

if exist "%zipWin32%" (
	ECHO.
	set zipExe="%zipWin32%\7z.exe"
    ECHO -Found zip exe at "%zipWin32%"
    GOTO FINISHED_SETUP
)

ECHO.
set /p zipExe=Could not find any zip exectutable. You can manually enter a location: 
set /p OPTION=Zip exe at [%zipExe%] will be used to archive your library. Hit [1] to exit or Enter to continue:
if  %OPTION% == 1  EXIT

rem can test here for entered zip exe then terminate if no suitable location found
GOTO FINISHED_SETUP

:FINISHED_SETUP
ECHO.
ECHO -LDraw path entered: %LDRAWDIR%
ECHO -Zip exe location: %zipExe%
ECHO.
ECHO -Update and archive unofficial parts content
ECHO.
cd /D %outputPath%\
ECHO.
ECHO -Create 'LPub3DViewer-Library' folder
ECHO.
if not exist ".\LPub3DViewer-Library\" (
  mkdir ".\LPub3DViewer-Library\"
)
ECHO -Download unofficial content from LDraw.org
ECHO.
rem -------------------------------------------------------------------------
if not exist "%TEMP%\$" (
  md "%TEMP%\$"
)
set n=WebContentDownload.vbs
set t=%TEMP%\$\%n% ECHO
set WebCONTENT="%~dp0LPub3DViewer-Library\ldrawunf.zip"
set WebNAME=http://www.ldraw.org/library/unofficial/ldrawunf.zip
rem http://www.ldraw.org/library/updates/complete.zip
if exist %TEMP%\$\%n% (
 del %TEMP%\$\%n%
)
if exist %WebCONTENT% (
 del %WebCONTENT%
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
>>%t% WScript.Echo "Getting '" ^& target ^& "' from '" ^& url ^& "'..."
>>%t%.
>>%t% http.Open "GET", url, False
>>%t% http.Send
>>%t% status = http.Status
>>%t%.
>>%t% If status ^<^> 200 Then
>>%t% WScript.Echo "FAILED to download: HTTP Status " ^& status
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
>>%t%   WScript.Echo "Error - CANNOT DELETE: '" ^& target ^& "', " ^& Err.Description
>>%t%   WScript.Echo "The file may be in use by another process."
>>%t%   adoStream.Close
>>%t%   Err.Clear
>>%t% Else
>>%t%  adoStream.SaveToFile target
>>%t%  adoStream.Close
>>%t%  WScript.Echo "Download successful!"
>>%t% End If
>>%t%.
>>%t% 'WebContentDownload.vbs
>>%t% 'Title: BATCH to VBS to Web Content Downloader
>>%t% 'CMD ^> cscript //Nologo %TEMP%\$\%n% %WebNAME% %WebCONTENT%
>>%t% 'VBS Created on %date% at %time%
>>%t%.
rem -------------------------------------------------------------------------
ECHO. 
ECHO -BATCH to VBS to Web Content Downloader
ECHO.
ECHO -File "%n%" is done compiling.
ECHO.
ECHO -WEB URL: "%WebNAME%" 
ECHO  is ready to be downloaded to...
ECHO  DIRECTORY: %WebCONTENT%
ECHO.
ECHO.
ECHO -If you are ready to continue and download the content, Press any key!
PAUSE >NUL
@echo on
cscript //Nologo %TEMP%\$\%n% %WebNAME% %WebCONTENT% && @echo off
ECHO.
ECHO -Extract downloaded unofficial library to Unofficial directory 
ECHO.
cd /D %ldrawPath%\Unofficial\
%zipExe% x %outputPath%\LPub3DViewer-Library\ldrawunf.zip -aoa > %outputPath%\LPub3DViewerLibrary-archive-log.txt
ECHO.
ECHO -Merge unofficial content into new Unofficial archive [ldrawunf.zip]
ECHO.
%zipExe% a -tzip %outputPath%\LPub3DViewer-Library\ldrawunf.zip p\ >> %outputPath%\LPub3DViewerLibrary-archive-log.txt
%zipExe% a -tzip %outputPath%\LPub3DViewer-Library\ldrawunf.zip parts\ >> %outputPath%\LPub3DViewerLibrary-archive-log.txt
ECHO -Finshed
ECHO.
ECHO -If everything went well then, Press any key to EXIT!
PAUSE >NUL
ENDLOCAL
EXIT

