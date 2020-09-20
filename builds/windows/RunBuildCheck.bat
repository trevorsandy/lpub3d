@ECHO OFF &SETLOCAL

Title Perform a stand-alone build check

rem  Trevor SANDY <trevor.sandy@gmail.com>
rem  Last Update: June 29, 2019
rem  Copyright (c) 2017 - 2020 by Trevor SANDY

rem  To Run:
rem  1. SET LP3D_APP_VERSION_LONG accordingly
rem  2. RunBuildCheck.bat

CALL :ELAPSED_BUILD_TIME Start

FOR %%* IN (.) DO SET SCRIPT_DIR=%%~nx*
IF "%SCRIPT_DIR%" EQU "windows" (
  CALL :WD_REL_TO_ABS ..\..\
) ELSE (
  SET ABS_WD=%CD%
)

CD /D %ABS_WD%

rem set application version variables
SET _PRO_FILE_PWD_=%ABS_WD%\mainApp
CALL builds\utilities\update-config-files.bat %_PRO_FILE_PWD_%

rem Initialize needed variables
SET PACKAGE=LPub3D
SET CONFIGURATION=release
SET LDRAW_DIR=%USERPROFILE%\LDraw
SET LDRAW_LIBS=%USERPROFILE%
SET LDRAW_INSTALL_ROOT=%LDRAW_LIBS%
CALL :DIST_DIR_REL_TO_ABS ..\lpub3d_windows_3rdparty

SET OfficialCONTENT=complete.zip
SET UnOfficialCONTENT=ldrawunf.zip
SET LPub3DCONTENT=lpub3dldrawunf.zip
SET TenteCONTENT=tenteparts.zip
SET VexiqCONTENT=vexiqparts.zip

SET PLATFORM=unknown
SET UPDATE_LDRAW_LIBS=unknown

rem Verify 1st input flag options
IF NOT [%1]==[] (
  IF NOT "%1"=="x86" (
    IF NOT "%1"=="x86_64" (
      IF NOT "%1"=="-all" (
        IF NOT "%1"=="-help" GOTO :PLATFORM_ERROR
      )
    )
  )
)

rem Verify 2nd input flag options
IF NOT [%2]==[] (
  IF NOT "%2"=="-uld" (
    IF NOT "%2"=="-official" (
      IF NOT "%2"=="-unofficial" (
        IF NOT "%2"=="-tente" (
          IF NOT "%2"=="-vexiq" GOTO :CONFIGURATION_ERROR
        )
      )
    )
  )
)

rem Parse platform input flags
IF [%1]==[] (
  SET PLATFORM=-all
)

IF /I "%1"=="x86" (
  SET PLATFORM=x86
)

IF /I "%1"=="x86_64" (
  SET PLATFORM=x86_64
)

IF NOT [%2]==[] (
  SET UPDATE_LDRAW_LIBS=%2
)

rem Check if build all platforms
IF /I "%PLATFORM%"=="-all" (
  GOTO :CHECK_ALL
)

SET platform_build_start=%time%
CALL :BUILD_CHECK %PLATFORM%
CALL :ELAPSED_BUILD_TIME %platform_build_start%
ECHO.
ECHO -Elapsed %PLATFORM% package build time %LP3D_ELAPSED_BUILD_TIME%
GOTO :END

:CHECK_ALL
ECHO.
ECHO -Check LPub3D x86 and x86_64 platforms...
FOR %%P IN ( x86, x86_64 ) DO (
  SETLOCAL ENABLEDELAYEDEXPANSION
  SET platform_build_start=%time%
  SET PLATFORM=%%P
  rem Perform build check if specified
  CALL :BUILD_CHECK %%P
  CALL :ELAPSED_BUILD_TIME !platform_build_start!
  ECHO.
  ECHO -Elapsed %%P package build time !LP3D_ELAPSED_BUILD_TIME!
  SETLOCAL DISABLEDELAYEDEXPANSION
)
GOTO :END

:BUILD_CHECK
ECHO.
ECHO -%PACKAGE% Build Check...
IF [%1] EQU [] (
  ECHO.
  ECHO -ERROR - No PLATFORM defined, build check will exit.
  EXIT /b
)
IF NOT EXIST "%DIST_DIR%" (
  ECHO.
  ECHO -ERROR - 3rd Party Renderer folder '%DIST_DIR%' not found, build check will exit.
  EXIT /b
)
SET PKG_PLATFORM=%1
CALL :REQUEST_LDRAW_DIR
rem run build checks
CALL builds\check\build_checks.bat
EXIT /b

:REQUEST_LDRAW_DIR
ECHO.
ECHO -Request LDraw archive libraries download...

CALL :DOWNLOAD_LDRAW_LIBS

ECHO.
ECHO - Check for LDraw LEGO disk library...
IF NOT EXIST "%LDRAW_DIR%\parts" (
  ECHO.
  ECHO -LDraw directory %LDRAW_DIR% does not exist - creating...
  IF NOT EXIST "%LDRAW_INSTALL_ROOT%\%OfficialCONTENT%" (
    IF EXIST "%LDRAW_LIBS%\%OfficialCONTENT%" (
      COPY /V /Y "%LDRAW_LIBS%\%OfficialCONTENT%" "%LDRAW_INSTALL_ROOT%\" /A | findstr /i /v /r /c:"copied\>"
    ) ELSE (
      ECHO.
      ECHO -[WARNING] Could not find %LDRAW_LIBS%\%OfficialCONTENT%.
      SET CHECK=0
    )
  )
  IF EXIST "%zipWin64%" (
    ECHO.
    ECHO -7zip exectutable found at "%zipWin64%"
    ECHO.
    ECHO -Extracting %OfficialCONTENT%...
    ECHO.
    "%zipWin64%\7z.exe" x -o"%LDRAW_INSTALL_ROOT%\" "%LDRAW_INSTALL_ROOT%\%OfficialCONTENT%" | findstr /i /r /c:"^Extracting\>" /c:"^Everything\>"
    IF EXIST "%LDRAW_DIR%\parts" (
      ECHO.
      ECHO -LDraw directory %LDRAW_DIR% extracted.
      ECHO.
      ECHO -Cleanup %OfficialCONTENT%...
      DEL /Q "%LDRAW_INSTALL_ROOT%\%OfficialCONTENT%"
      ECHO.
      ECHO -Set LDRAWDIR to %LDRAW_DIR%.
      SET LDRAWDIR=%LDRAW_DIR%
    )
  ) ELSE (
    ECHO [WARNING] Could not find 7zip executable.
    SET CHECK=0
  )
) ELSE (
  ECHO.
  ECHO -LDraw directory exist at [%LDRAW_DIR%].
  ECHO.
  ECHO -Set LDRAWDIR to %LDRAW_DIR%.
  SET LDRAWDIR=%LDRAW_DIR%
)
EXIT /b

:DOWNLOAD_LDRAW_LIBS
ECHO.
ECHO - Download LDraw archive libraries...

IF NOT EXIST "%LDRAW_LIBS%\" (
  ECHO.
  ECHO - Create LDraw archive libs store %LDRAW_LIBS%
  MKDIR "%LDRAW_LIBS%\"
)

SET OutputPATH=%LDRAW_LIBS%

ECHO.
ECHO - Prepare BATCH to VBS to Web Content Downloader...

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
>>%t%   WScript.Echo "  The file may be in use by another process.", vbLF
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

ECHO.
ECHO - VBS file "%vbs%" is done compiling
ECHO.
ECHO - LDraw archive library download path: %OutputPATH%

IF "%UPDATE_LDRAW_LIBS%" EQU "-uld" (
  GOTO :UPDATE_ALL_LIBRARIES
)

IF NOT EXIST "%OutputPATH%\%OfficialCONTENT%" (
  CALL :GET_OFFICIAL_LIBRARY
)  ELSE (
  IF "%UPDATE_LDRAW_LIBS%" EQU "-official" (
    DEL /Q "%OutputPATH%\%OfficialCONTENT%"
    CALL :GET_OFFICIAL_LIBRARY
  ) ELSE (
    ECHO.
    ECHO - LDraw archive library %OfficialCONTENT% exist. Nothing to do.
  )
)
IF NOT EXIST "%OutputPATH%\%TenteCONTENT%" (
  CALL :GET_TENTE_LIBRARY
) ELSE (
  IF "%UPDATE_LDRAW_LIBS%" EQU "-tente" (
    DEL /Q "%OutputPATH%\%TenteCONTENT%"
    CALL :GET_TENTE_LIBRARY
  ) ELSE (
    ECHO.
    ECHO - LDraw archive library %TenteCONTENT% exist. Nothing to do.
  )
)
IF NOT EXIST "%OutputPATH%\%VexiqCONTENT%" (
  CALL :GET_VEXIQ_LIBRARY
) ELSE (
  IF "%UPDATE_LDRAW_LIBS%" EQU "-vexiq" (
    DEL /Q "%OutputPATH%\%VexiqCONTENT%"
    CALL :GET_VEXIQ_LIBRARY
  ) ELSE (
    ECHO.
    ECHO - LDraw archive library %VexiqCONTENT% exist. Nothing to do.
  )
)
IF NOT EXIST "%OutputPATH%\%LPub3DCONTENT%" (
  CALL :GET_UNOFFICIAL_LIBRARY
) ELSE (
  IF "%UPDATE_LDRAW_LIBS%" EQU "-unofficial" (
    DEL /Q "%OutputPATH%\%LPub3DCONTENT%"
    CALL :GET_UNOFFICIAL_LIBRARY
  ) ELSE (
    ECHO.
    ECHO - LDraw archive library %LPub3DCONTENT% exist. Nothing to do.
  )
)
EXIT /b

:UPDATE_ALL_LIBRARIES
ECHO.
ECHO - Update all libraries...

IF EXIST "%OutputPATH%\%OfficialCONTENT%" (
  DEL /Q "%OutputPATH%\%OfficialCONTENT%"
  CALL :GET_OFFICIAL_LIBRARY
)
IF EXIST "%OutputPATH%\%LPub3DCONTENT%" (
  DEL /Q "%OutputPATH%\%LPub3DCONTENT%"
  CALL :GET_UNOFFICIAL_LIBRARY
)
IF EXIST "%OutputPATH%\%TenteCONTENT%" (
  DEL /Q "%OutputPATH%\%TenteCONTENT%"
  CALL :GET_TENTE_LIBRARY
)
IF EXIST "%OutputPATH%\%VexiqCONTENT%" (
  DEL /Q "%OutputPATH%\%VexiqCONTENT%
  CALL :GET_VEXIQ_LIBRARY
)
EXIT /b

:GET_OFFICIAL_LIBRARY
SET WebCONTENT="%OutputPATH%\%OfficialCONTENT%"
SET WebNAME=http://www.ldraw.org/library/updates/%OfficialCONTENT%

ECHO.
ECHO - Download archive file: %WebCONTENT%...
ECHO.
cscript //Nologo %TEMP%\$\%vbs% %WebNAME% %WebCONTENT% && @ECHO off
IF EXIST "%OutputPATH%\%OfficialCONTENT%" (
  ECHO.
  ECHO - LDraw archive library %OfficialCONTENT% is availble
)
EXIT /b

:GET_TENTE_LIBRARY
SET WebCONTENT="%OutputPATH%\%TenteCONTENT%"
SET WebNAME=https://github.com/trevorsandy/lpub3d_libs/releases/download/v1.0.1/%TenteCONTENT%

ECHO.
ECHO - Download archive file: %WebCONTENT%...
ECHO.
cscript //Nologo %TEMP%\$\%vbs% %WebNAME% %WebCONTENT% && @ECHO off
IF EXIST "%OutputPATH%\%TenteCONTENT%" (
  ECHO.
  ECHO - LDraw archive library %TenteCONTENT% is availble
)
EXIT /b

:GET_VEXIQ_LIBRARY
SET WebCONTENT="%OutputPATH%\%VexiqCONTENT%"
SET WebNAME=https://github.com/trevorsandy/lpub3d_libs/releases/download/v1.0.1/%VexiqCONTENT%

ECHO.
ECHO - Download archive file: %WebCONTENT%...
ECHO.
cscript //Nologo %TEMP%\$\%vbs% %WebNAME% %WebCONTENT% && @ECHO off
IF EXIST "%OutputPATH%\%VexiqCONTENT%" (
  ECHO.
  ECHO - LDraw archive library %VexiqCONTENT% is availble
)
EXIT /b

:GET_UNOFFICIAL_LIBRARY
SET WebCONTENT="%OutputPATH%\%UnofficialCONTENT%"
SET WebNAME=http://www.ldraw.org/library/unofficial/%UnofficialCONTENT%

ECHO.
ECHO - Download archive file: %WebCONTENT%...
ECHO.
cscript //Nologo %TEMP%\$\%vbs% %WebNAME% %WebCONTENT% && @ECHO off
ECHO.
ECHO - Rename %WebCONTENT% to %LPub3DCONTENT%
REN "%WebCONTENT%" %LPub3DCONTENT%
IF EXIST "%OutputPATH%\%LPub3DCONTENT%" (
  ECHO.
  ECHO - LDraw archive library %LPub3DCONTENT% is availble
)
EXIT /b

:WD_REL_TO_ABS
IF [%1] EQU [] (EXIT /b) ELSE (SET REL_WD=%1)
SET REL_WD=%REL_WD:/=\%
SET ABS_WD=
PUSHD %REL_WD%
SET ABS_WD=%CD%
POPD
EXIT /b

:DIST_DIR_REL_TO_ABS
IF [%1] EQU [] (EXIT /b) ELSE (SET REL_DIST_DIR=%1)
SET REL_DIST_DIR=%REL_DIST_DIR:/=\%
SET DIST_DIR=
PUSHD %REL_DIST_DIR%
SET DIST_DIR=%CD%
POPD
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
SET LP3D_ELAPSED_BUILD_TIME=%hours%:%mins%:%secs%.%ms%
EXIT /b

:PLATFORM_ERROR
ECHO.
CALL :USAGE
ECHO.
ECHO -01. (FLAG ERROR) Platform or usage flag is invalid. Use x86, x86_64 or -all [%~nx0 %*].
ECHO      See Usage.
GOTO :END

:CONFIGURATION_ERROR
ECHO.
CALL :USAGE
ECHO.
ECHO -02. (FLAG ERROR) Configuration flag is invalid [%~nx0 %*].
ECHO      See Usage.
GOTO :END

:USAGE
ECHO ----------------------------------------------------------------
ECHO.
ECHO %PACKAGE% Windows build check script.
ECHO.
ECHO ----------------------------------------------------------------
ECHO Usage:
ECHO  RunBuildCheck.bat [ -help]
ECHO  RunBuildCheck.bat [ x86 ^| x86_64 ^| -all ] [ -uld  ^| -official ^| -unofficial ^| -tente ^| -vexiq ]
ECHO.
ECHO Commands:
ECHO ----------------------------------------------------------------
ECHO [PowerShell]: cmd.exe /c builds\windows\RunBuildCheck.bat.cmd [Flags]
ECHO [cmd.exe]   : builds\windows\RunBuildCheck.bat.cmd [Flags]
ECHO.
ECHO Flags:
ECHO ----------------------------------------------------------------
ECHO ^| Flag      ^| Pos ^| Type              ^| Description
ECHO ----------------------------------------------------------------
ECHO  -help........1..Useage flag         [Default=Off] Display useage.
ECHO  x86..........1..Platform flag       [Default=Off] Check 32bit platform.
ECHO  x86_64.......1..Platform flag       [Default=On ] Check 64bit platform.
ECHO  -all.........1..Configuraiton flag  [Default=On ] Check both 32bit and 64bit platforms
ECHO  -uld.........2..Project flag        [Default=Off] Force update LDraw libraries
ECHO  -unofficial..2..Project flag        [Default=Off] Force update Unofficial LDraw library
ECHO  -official....2..Project flag        [Default=Off] Force update Official LDraw library
ECHO  -tente.......2..Project flag        [Default=Off] Force update Tente LDraw library
ECHO  -vexiq.......2..Project flag        [Default=Off] Force update VEXiQ LDraw library
ECHO.
ECHO Be sure the set your LDraw directory in the variables section above before running this script.
ECHO.
ECHO Flags are case sensitive, use lowere case.
ECHO.
ECHO If no flag is supplied, 64bit platform, is checked by default.
ECHO ----------------------------------------------------------------
EXIT /b

:END
ECHO.
ECHO -%PACKAGE% %~nx0 finished.
CALL :ELAPSED_BUILD_TIME
ECHO -Elapsed %PACKAGE% build time %LP3D_ELAPSED_BUILD_TIME%
ENDLOCAL
EXIT /b
