@ECHO OFF &SETLOCAL

Title Build, test and package LPub3D 3rdParty renderers.
rem --
rem  Trevor SANDY <trevor.sandy@gmail.com>
rem  Last Update: July 03, 2021
rem  Copyright (c) 2017 - 2021 by Trevor SANDY
rem --
rem This script is distributed in the hope that it will be useful,
rem but WITHOUT ANY WARRANTY; without even the implied warranty of
rem MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

CALL :ELAPSED_BUILD_TIME Start

ECHO.
ECHO -Start %~nx0 with commandline args: [%*]...

rem get the parent folder
FOR %%* IN (.) DO SET SCRIPT_DIR=%%~nx*
IF "%SCRIPT_DIR%" EQU "utilities" (
  rem get abs path to build 3rd party packages inside the LPub3D root dir
  IF "%APPVEYOR%" EQU "True" (
    CALL :WD_ABS_PATH ../../
  ) ELSE (
    CALL :WD_ABS_PATH ../../../
  )
) ELSE (
  rem get abs path to build 3rd party packages outside the LPub3D root dir
  IF "%APPVEYOR%" EQU "True" (
    SET ABS_WD=%CD%
  ) ELSE (
    CALL :WD_ABS_PATH ../
  )
)

rem Variables - change these as required by your build environments
SET LP3D_QTVERSION=5.15.2
SET LP3D_VSVERSION=2019

IF "%GITHUB%" EQU "True" (
  SET BUILD_OUTPUT_PATH=%LP3D_BUILD_BASE%
  SET LDRAW_DIR=%LP3D_LDRAW_DIR_PATH%
  SET DIST_DIR=%LP3D_DIST_DIR_PATH%
  SET BUILD_ARCH=%LP3D_TARGET_ARCH%
)
IF "%APPVEYOR%" EQU "True" (
  SET BUILD_OUTPUT_PATH=%APPVEYOR_BUILD_FOLDER%
  SET LDRAW_DIR=%APPVEYOR_BUILD_FOLDER%\LDraw
  SET DIST_DIR=%LP3D_DIST_DIR_PATH%
  SET BUILD_ARCH=%LP3D_TARGET_ARCH%
)
IF "%GITHUB%" NEQ "True" (
  IF "%APPVEYOR%" NEQ "True" (
    IF [%DIST_DIR%] == [] (
      CALL :DIST_DIR_ABS_PATH ..\lpub3d_windows_3rdparty
      ECHO.
      SETLOCAL ENABLEDELAYEDEXPANSION
      ECHO  -WARNING: Distribution not found. Using [!DIST_DIR!].
      SETLOCAL
    )
    SET BUILD_OUTPUT_PATH=%ABS_WD%
    SET LDRAW_DIR=%USERPROFILE%\LDraw
    SET LP3D_QT32_MSVC=C:\Qt\IDE\%LP3D_QTVERSION%\msvc%LP3D_VSVERSION%\bin
    SET LP3D_QT64_MSVC=C:\Qt\IDE\%LP3D_QTVERSION%\msvc%LP3D_VSVERSION%_64\bin
    SET LP3D_WIN_GIT=%ProgramFiles%\Git\cmd
  )
)
SET MAX_DOWNLOAD_ATTEMPTS=4
SET VER_LDGLITE=LDGLite-1.3
SET VER_LDVIEW=LDView-4.4
SET VER_POVRAY=lpub3d_trace_cui-3.8
SET SYS_DIR=%SystemRoot%cls\System32\System32
SET ZIP_DIR_64=C:\program files\7-zip
SET VALID_ZIP=0

rem Check if invalid platform flag
IF NOT [%1]==[] (
  IF NOT "%1"=="x86" (
    IF NOT "%1"=="x86_64" (
      IF NOT "%1"=="-all" (
        IF NOT "%1"=="-help" GOTO :COMMAND_ERROR
      )
    )
  )
)

ECHO.
ECHO ======================================================
ECHO   BUILD LPUB3D RENDERERS FOR %1 ARCHITECTURE...
ECHO ======================================================
ECHO.
ECHO   WORKING_DIRECTORY_RENDERERS....[%ABS_WD%]
ECHO   DISTRIBUTION DIRECTORY.........[%DIST_DIR:/=\%]
ECHO   LDRAW LIBRARY FOLDER...........[%LDRAW_DIR%]

IF EXIST "%ZIP_DIR_64%" (
  ECHO   7-ZIP EXECUTABLE...............[Found at %ZIP_DIR_64%]
  SET VALID_ZIP=1
) ELSE (
  ECHO   7-ZIP EXECUTABLE...............[ERROR - not found at %ZIP_DIR_64%]
  GOTO :ERROR_END
)

IF NOT EXIST "%DIST_DIR%\" (
  MKDIR "%DIST_DIR%\"
)

CALL :CHECK_LDRAW_LIB

IF [%1]==[] (
  GOTO :BUILD_ALL
)
IF /I "%1"=="-all" (
  GOTO :BUILD_ALL
)
IF /I "%1"=="x86" (
  SET BUILD_ARCH=%1
  GOTO :BUILD
)
IF /I "%1"=="x86_64" (
  SET BUILD_ARCH=%1
  GOTO :BUILD
)
IF /I "%1"=="-help" (
  GOTO :USAGE
)
rem If we get here display invalid command message.
GOTO :COMMAND_ERROR

:BUILD_ALL
FOR %%A IN ( x86, x86_64 ) DO (
  SET BUILD_ARCH=%%A
  CALL :BUILD
  rem Reset PATH_PREPENDED
  SET PATH_PREPENDED=False
)
GOTO :END

:BUILD
IF %BUILD_ARCH% EQU x86 (
  SET LP3D_LDGLITE=%DIST_DIR%\%VER_LDGLITE%\bin\i386\LDGLite.exe
  SET LP3D_LDVIEW=%DIST_DIR%\%VER_LDVIEW%\bin\i386\LDView.exe
  SET LP3D_LDVIEW_BIN=%DIST_DIR%\%VER_LDVIEW%\bin\i386\
  SET LP3D_POVRAY=%DIST_DIR%\%VER_POVRAY%\bin\i386\lpub3d_trace_cui32.exe
  IF "%PATH_PREPENDED%" NEQ "True" (
    SET PATH=%LP3D_QT32_MSVC%;%SYS_DIR%;%LP3D_WIN_GIT%
  )
) ELSE (
  SET LP3D_LDGLITE=%DIST_DIR%\%VER_LDGLITE%\bin\%BUILD_ARCH%\LDGLite.exe
  SET LP3D_LDVIEW=%DIST_DIR%\%VER_LDVIEW%\bin\%BUILD_ARCH%\LDView64.exe
  SET LP3D_LDVIEW_BIN=%DIST_DIR%\%VER_LDVIEW%\bin\%BUILD_ARCH%\
  SET LP3D_POVRAY=%DIST_DIR%\%VER_POVRAY%\bin\%BUILD_ARCH%\lpub3d_trace_cui64.exe
  IF "%PATH_PREPENDED%" NEQ "True" (
    SET PATH=%LP3D_QT64_MSVC%;%SYS_DIR%;%LP3D_WIN_GIT%
  )
)
ECHO.
SETLOCAL ENABLEDELAYEDEXPANSION
IF "%PATH_PREPENDED%" EQU "True" (
  ECHO   PATH_ALREADY_PREPENDED.........[!PATH!]
) ELSE (
  ECHO   PATH_PREPEND...................[!PATH!]
  SET PATH_PREPENDED=True
)
ENDLOCAL
CALL :SET_BUILD_ARGS
FOR %%I IN ( LDGLITE, LDVIEW, POVRAY ) DO (
  CALL :%%I_BUILD
  IF ERRORLEVEL 1 (GOTO :FATAL_ERROR)
)
GOTO :END

:SET_BUILD_ARGS
IF %BUILD_ARCH% EQU x86 (
  SET POVRAY_INSTALL_ARG=-allins
) ELSE (
  SET POVRAY_INSTALL_ARG=-ins
)
SET LDGLITE_BUILD_ARGS=%BUILD_ARCH% -ins -chk
SET LDVIEW_BUILD_ARGS=%BUILD_ARCH% -ins -chk -minlog
SET POVRAY_BUILD_ARGS=%BUILD_ARCH% %POVRAY_INSTALL_ARG% -chk -minlog
EXIT /b

:LDGLITE_BUILD
ECHO.
ECHO -Build %VER_LDGLITE%...
IF EXIST "%LP3D_LDGLITE%" (
  ECHO - Renderer %VER_LDGLITE% exist - build skipped.
  EXIT /b
)
SET BUILD_DIR=ldglite
SET VALID_SDIR=app
SET ARCHIVE_FILE_DIR=ldglite-master
SET WebNAME=https://github.com/trevorsandy/ldglite/archive/master.zip
CALL :CONFIGURE_BUILD_ENV
CALL build.cmd %LDGLITE_BUILD_ARGS%
IF NOT EXIST "%LP3D_LDGLITE%" (
  ECHO  ERROR - Renderer %VER_LDGLITE% was not successfully built.
  GOTO :ERROR_END
)
EXIT /b

:LDVIEW_BUILD
ECHO.
ECHO -Build %VER_LDVIEW%...
IF EXIST "%LP3D_LDVIEW%" (
  ECHO - Renderer %VER_LDVIEW% exist - build skipped.
  PUSHD "%LP3D_LDVIEW_BIN%"
  ECHO - Renderer %VER_LDVIEW% bin contents:
  FOR /f "delims=" %%f IN ('DIR /B /A-D-H-S') DO ECHO - %%f
  POPD
  EXIT /b
)
SET BUILD_DIR=ldview
SET VALID_SDIR=OSMesa
SET ARCHIVE_FILE_DIR=ldview-qmake-build
SET WebNAME=https://github.com/trevorsandy/ldview/archive/qmake-build.zip
CALL :CONFIGURE_BUILD_ENV
CALL build.cmd %LDVIEW_BUILD_ARGS%
IF NOT EXIST "%LP3D_LDVIEW%" (
  ECHO  ERROR - Renderer %VER_LDVIEW% was not successfully built.
  GOTO :ERROR_END
)
PUSHD "%LP3D_LDVIEW_BIN%"
ECHO - Renderer %VER_LDVIEW% bin contents:
FOR /f "delims=" %%f IN ('DIR /B /A-D-H-S') DO ECHO  - %%f
POPD
EXIT /b

:POVRAY_BUILD
ECHO.
ECHO -Build LPub3D-Trace-3.8 ^(POV-Ray^)...
IF EXIST "%LP3D_POVRAY%" (
  ECHO - Renderer %VER_POVRAY% exist - build skipped.
  EXIT /b
)
SET BUILD_DIR=povray
SET VALID_SDIR=windows
SET ARCHIVE_FILE_DIR=povray-lpub3d-raytracer-cui
SET WebNAME=https://github.com/trevorsandy/povray/archive/lpub3d/raytracer-cui.zip
CALL :CONFIGURE_BUILD_ENV
CD /D %VALID_SDIR%\vs2015
CALL autobuild.cmd %POVRAY_BUILD_ARGS%
IF NOT EXIST "%LP3D_POVRAY%" (
  ECHO  ERROR - Renderer %VER_POVRAY% was not successfully built.
  GOTO :ERROR_END
)
EXIT /b

:CONFIGURE_BUILD_ENV
CD /D %BUILD_OUTPUT_PATH%
SET ARCHIVE_FILE=%ARCHIVE_FILE_DIR%.zip
SET WebCONTENT="%BUILD_OUTPUT_PATH%\%ARCHIVE_FILE%"
ECHO  ----------------------------------------------------
ECHO   BUILD_ARCH.....................[%BUILD_ARCH%]
ECHO   BUILD_DIRECTORY................[%BUILD_DIR%]
ECHO   VALID_SUB_DIRECTORY............[%VALID_SDIR%]
ECHO   REPOSITORHY_DIRECTORY..........[%ARCHIVE_FILE_DIR%]
ECHO   BUILD_OUTPUT_PATH..............[%BUILD_OUTPUT_PATH%]
ECHO   REPO_ARCHIVE_FILE..............[%ARCHIVE_FILE%]
ECHO   DOWNLOAD_URL.(WebNAME).........[%WebNAME%]
ECHO   DOWNLOAD_PATH.(WebCONTENT).....[%WebCONTENT%]
ECHO   COMPLETE_BUILD_PATH............[%BUILD_OUTPUT_PATH%\%BUILD_DIR%]
IF NOT EXIST "%BUILD_OUTPUT_PATH%\%BUILD_DIR%\%VALID_SDIR%" (
  IF NOT EXIST "%BUILD_OUTPUT_PATH%\%ARCHIVE_FILE%" (
    ECHO.
    ECHO -Directory %BUILD_DIR% does not exist - Downloading %BUILD_DIR% source archive...

    CALL :DOWNLOAD_ARCHIVE
  )

  CALL :EXTRACT_ARCHIVE
  CALL :SETUP_BUILD_DIR

) ELSE (
  ECHO.
  ECHO -Directory exist at [%CD%\%BUILD_DIR%] - Start build...
)
CD /D %BUILD_OUTPUT_PATH%\%BUILD_DIR%
EXIT /b

:CHECK_LDRAW_LIB
ECHO.
ECHO -Check for LDraw library (support image render tests)...
SET BUILD_OUTPUT_PATH_SAVE=%BUILD_OUTPUT_PATH%
IF "%GITHUB%" EQU "True" (
  SET BUILD_OUTPUT_PATH=%LP3D_3RD_PARTY_PATH%
)
IF "%GITHUB%" NEQ "True" (
  IF "%APPVEYOR%" NEQ "True" (
    SET BUILD_OUTPUT_PATH=%USERPROFILE%
  )
)
SET ARCHIVE_FILE_DIR=%LDRAW_DIR%
SET ARCHIVE_FILE=complete.zip
SET VALID_SDIR=parts
SET WebCONTENT="%BUILD_OUTPUT_PATH%\%ARCHIVE_FILE%"
SET WebNAME=http://www.ldraw.org/library/updates/complete.zip
IF NOT EXIST "%LDRAW_DIR%\%VALID_SDIR%" (
  IF NOT EXIST "%BUILD_OUTPUT_PATH%\%ARCHIVE_FILE%" (
    ECHO.
    ECHO -LDraw directory %LDRAW_DIR% does not exist - Downloading LDraw source archive...

    CALL :DOWNLOAD_ARCHIVE
  )

  CALL :EXTRACT_ARCHIVE

  IF EXIST "%LDRAW_DIR%\%VALID_SDIR%" (
    ECHO.
    ECHO -LDraw directory %LDRAW_DIR% successfully extracted.
    ECHO.
    ECHO -Set LDRAWDIR to %LDRAW_DIR%.
    SET LDRAWDIR=%LDRAW_DIR%
    REM ECHO.
    REM ECHO -Cleanup %ARCHIVE_FILE%...
    REM DEL /Q "%BUILD_OUTPUT_PATH%\%ARCHIVE_FILE%"
  ) ELSE (
      ECHO.
      ECHO -WARNING: LDraw library folder %LDRAW_DIR% is not valid.
      SET BUILD_OUTPUT_PATH=%BUILD_OUTPUT_PATH_SAVE%
      EXIT /b
  )
) ELSE (
  ECHO.
  ECHO -LDraw directory exist at [%LDRAW_DIR%].
  ECHO.
  ECHO -Set LDRAWDIR to %LDRAW_DIR%.
  SET LDRAWDIR=%LDRAW_DIR%
)
SET BUILD_OUTPUT_PATH=%BUILD_OUTPUT_PATH_SAVE%
EXIT /b

rem args: $1 = <build dir>, $2 = <valid subdir>
:EXTRACT_ARCHIVE
IF NOT EXIST "%BUILD_OUTPUT_PATH%\%ARCHIVE_FILE_DIR%" (
  IF EXIST "%BUILD_OUTPUT_PATH%\%ARCHIVE_FILE%" (
    ECHO.
    ECHO -Extracting %ARCHIVE_FILE%...
    ECHO.
    IF %VALID_ZIP% EQU 1 "%ZIP_DIR_64%\7z.exe" x -o"%BUILD_OUTPUT_PATH%\" "%BUILD_OUTPUT_PATH%\%ARCHIVE_FILE%" | findstr /i /r /c:"^Extracting\>" /c:"^Everything\>"
  ) ELSE (
    ECHO.
    ECHO -ERROR: Could not find %BUILD_OUTPUT_PATH%\%ARCHIVE_FILE%.
    GOTO :ERROR_END
  )
)
EXIT /b

:SETUP_BUILD_DIR
IF EXIST "%BUILD_OUTPUT_PATH%\%ARCHIVE_FILE_DIR%\%VALID_SDIR%" (
  ECHO.
  ECHO -Renaming %ARCHIVE_FILE_DIR% to %BUILD_DIR% at %CD%...
  REN "%ARCHIVE_FILE_DIR%" "%BUILD_DIR%"
  ECHO.
  ECHO -Directory %BUILD_DIR% successfully extracted.
  ECHO.
  ECHO -Cleanup %ARCHIVE_FILE%...
  DEL /Q "%BUILD_OUTPUT_PATH%\%ARCHIVE_FILE%"
) ELSE (
  ECHO.
  ECHO -ERROR: Build folder %BUILD_OUTPUT_PATH%\%ARCHIVE_FILE_DIR% is not valid.
  GOTO :ERROR_END
)
EXIT /b

:DOWNLOAD_ARCHIVE
ECHO.
ECHO - Download ARCHIVE %ARCHIVE_FILE%...
ECHO.
ECHO - Prepare BATCH to VBS to Web Content Downloader...

IF NOT EXIST "%TEMP%\$" (
  MD "%TEMP%\$"
)

SET RETRIES=0
SET /a MAX_RETRIES=%MAX_DOWNLOAD_ATTEMPTS%-1
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
>>%t% End If
>>%t%.
>>%t% 'WebContentDownload.vbs
>>%t% 'Title: BATCH to VBS to Web Content Downloader
>>%t% 'CMD ^> cscript //Nologo %TEMP%\$\%vbs% WebNAME WebCONTENT
>>%t% 'VBS Created on %date% at %time%
>>%t%.

ECHO.
ECHO - VBS file "%vbs%" is done compiling

:DO_DOWNLOAD
REM ECHO.
REM ECHO - File download path: %BUILD_OUTPUT_PATH%
ECHO.
ECHO - [%date% %time%] Download file: %WebCONTENT%...

IF EXIST %WebCONTENT% (
  DEL %WebCONTENT%
)

ECHO.
cscript //Nologo %TEMP%\$\%vbs% %WebNAME% %WebCONTENT% && @ECHO off

IF ERRORLEVEL 1 (GOTO :RETRY_DOWNLOAD "Error level 1")
IF NOT EXIST %WebCONTENT% (GOTO :RETRY_DOWNLOAD "File %ARCHIVE_FILE% not found")
SET RETRIES=0
EXIT /b

:RETRY_DOWNLOAD
SET /a RETRIES=%RETRIES%+1
IF %RETRIES% EQU %MAX_RETRIES% (GOTO :DOWNLOAD_ERROR)
IF %RETRIES% LSS %MAX_RETRIES% (
  ECHO.
  ECHO - WARNING - Download %ARCHIVE_FILE% failed with message %1.
  ECHO - Attempting %RETRIES% of %MAX_RETRIES% retries...
  GOTO :DO_DOWNLOAD
)

:WD_ABS_PATH
IF [%1] EQU [] (EXIT /B) ELSE SET ABS_PATH=%~f1
IF %ABS_PATH:~-1%==\ SET ABS_PATH=%ABS_PATH:~0,-1%
EXIT /b

:DIST_DIR_ABS_PATH
IF [%1] EQU [] (EXIT /B) ELSE SET DIST_DIR=%~f1
IF %DIST_DIR:~-1%==\ SET DIST_DIR=%DIST_DIR:~0,-1%
EXIT /b

:DOWNLOAD_ERROR
ECHO.
ECHO - [%date% %time%] - ERROR - Download %ARCHIVE_FILE% failed after %MAX_DOWNLOAD_ATTEMPTS% attempts.
GOTO :ERROR_END

:FATAL_ERROR
ECHO  %LP3D_ME% execution failed.
GOTO :ERROR_END

:COMMAND_ERROR
ECHO.
ECHO -01. (COMMAND ERROR) Invalid command string [%~nx0 %*].
ECHO      See Usage.
ECHO.

:USAGE
ECHO ----------------------------------------------------------------
ECHO.
ECHO Build, test and package LPub3D 3rdParty renderers.
ECHO.
ECHO ----------------------------------------------------------------
ECHO Usage:
ECHO  build [ -help]
ECHO  build [ x86 ^| x86_64 ^| -all ]
ECHO.
ECHO ----------------------------------------------------------------
ECHO Build 64bit, Releases
ECHO build x86_64
ECHO.
ECHO Build 32bit, Releases
ECHO build x86
ECHO.
ECHO Build 64bit and32bit, Releases
ECHO build -all
ECHO.
ECHO Commands:
ECHO ----------------------------------------------------------------
ECHO [PowerShell]: cmd.exe /c builds\utilities\CreateRenderers.bat
ECHO [cmd.exe]   : builds\utilities\CreateRenderers.bat
ECHO.
ECHO Flags:
ECHO ----------------------------------------------------------------
ECHO ^| Flag    ^| Pos ^| Type             ^| Description
ECHO ----------------------------------------------------------------
ECHO  -help......1......Useage flag         [Default=Off] Display useage.
ECHO  x86........1......Platform flag       [Default=Off] Build 32bit architecture.
ECHO  x86_64.....1......Platform flag       [Default=Off] Build 64bit architecture.
ECHO  -all.......1......Configuraiton flag  [Default=On ] Build both 32bit and 64bit architectures
ECHO.
ECHO Flags are case sensitive, use lowere case.
ECHO.
ECHO If no flag is supplied, 64bit platform, Release Configuration built by default.
ECHO ----------------------------------------------------------------
EXIT /b

:ELAPSED_BUILD_TIME
IF [%1] EQU [] (SET start=%build_start%) ELSE (
  IF "%1"=="Start" (
    SET build_start=%time%
    EXIT /b
  ) ELSE (
    SET start=%1
  )
)
SET end=%time%
SET options="tokens=1-4 delims=:.,"
FOR /f %options% %%a IN ("%start%") DO SET start_h=%%a&SET /a start_m=100%%b %% 100&SET /a start_s=100%%c %% 100&SET /a start_ms=100%%d %% 100
FOR /f %options% %%a IN ("%end%") DO SET end_h=%%a&SET /a end_m=100%%b %% 100&SET /a end_s=100%%c %% 100&SET /a end_ms=100%%d %% 100

SET /a hours=%end_h%-%start_h%
SET /a mins=%end_m%-%start_m%
SET /a secs=%end_s%-%start_s%
SET /a ms=%end_ms%-%start_ms%
IF %ms% lss 0 SET /a secs = %secs% - 1 & SET /a ms = 100%ms%
IF %secs% lss 0 SET /a mins = %mins% - 1 & SET /a secs = 60%secs%
IF %mins% lss 0 SET /a hours = %hours% - 1 & SET /a mins = 60%mins%
IF %hours% lss 0 SET /a hours = 24%hours%
IF 1%ms% lss 100 SET ms=0%ms%
ECHO -Elapsed build time %hours%:%mins%:%secs%.%ms%
ECHO ======================================================
ECHO.
ENDLOCAL
EXIT /b

:ERROR_END
ECHO.
ECHO -%~nx0 [platform %*] FAILED.
CALL :ELAPSED_BUILD_TIME
EXIT /b 3

:END
ECHO.
ECHO -%~nx0 [platform %*] finished.
CALL :ELAPSED_BUILD_TIME
EXIT /b
