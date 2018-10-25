@ECHO OFF

Title LPub3D Windows build check script

rem  Trevor SANDY <trevor.sandy@gmail.com>
rem  Last Update: October 21, 2018
rem  Copyright (c) 2018 by Trevor SANDY
rem --
rem This script is distributed in the hope that it will be useful,
rem but WITHOUT ANY WARRANTY; without even the implied warranty of
rem MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

rem Construct the staged files path
SET PKG_DISTRO_DIR=%PACKAGE%_%PKG_PLATFORM%
SET PKG_PRODUCT_DIR=%PACKAGE%-Any-%LP3D_APP_VERSION_LONG%
SET PKG_TARGET_DIR=builds\windows\%CONFIGURATION%\%PKG_PRODUCT_DIR%\%PKG_DISTRO_DIR%
SET PKG_TARGET=%PKG_TARGET_DIR%\%PACKAGE%.exe

rem Checks
SET PKG_CHECK_FILE=%ABS_WD%\builds\check\build_checks.mpd
SET PKG_CHECK_OPTIONS=--process-file --liblego --preferred-renderer native
SET PKG_CHECK_NATIVE_COMMAND=%PKG_TARGET% %PKG_CHECK_OPTIONS% %PKG_CHECK_FILE%

SET PKG_CHECK_OPTIONS=--process-file --clear-cache --liblego --preferred-renderer ldview
SET PKG_CHECK_LDVIEW_COMMAND=%PKG_TARGET% %PKG_CHECK_OPTIONS% %PKG_CHECK_FILE%

SET PKG_CHECK_OPTIONS=--process-file --clear-cache --liblego --preferred-renderer ldview-sc
SET PKG_CHECK_LDVIEW_SINGLE_CALL_COMMAND=%PKG_TARGET% %PKG_CHECK_OPTIONS% %PKG_CHECK_FILE%

SET PKG_CHECK_OPTIONS=--process-export --range 1-3 --clear-cache --liblego --preferred-renderer ldglite
SET PKG_CHECK_RANGE_COMMAND=%PKG_TARGET% %PKG_CHECK_OPTIONS% %PKG_CHECK_FILE%

SET PKG_CHECK_OPTIONS=--process-file --clear-cache --liblego --preferred-renderer povray
SET PKG_CHECK_POV_COMMAND=%PKG_TARGET% %PKG_CHECK_OPTIONS% %PKG_CHECK_FILE%

SET PKG_CHECK_FILE=%ABS_WD%\builds\check\TENTE\astromovil.ldr
SET PKG_CHECK_OPTIONS=--process-file --clear-cache --libtente --preferred-renderer ldview
SET PKG_CHECK_TENTE_COMMAND=%PKG_TARGET% %PKG_CHECK_OPTIONS% %PKG_CHECK_FILE%

SET PKG_CHECK_FILE=%ABS_WD%\builds\check\VEXIQ\spider.mpd
SET PKG_CHECK_OPTIONS=--process-file --clear-cache --libvexiq --preferred-renderer native
SET PKG_CHECK_VEXIQ_COMMAND=%PKG_TARGET% %PKG_CHECK_OPTIONS% %PKG_CHECK_FILE%

ECHO.
ECHO   PKG_CHECK_OPTIONS.........[%PKG_CHECK_OPTIONS%]
ECHO   PKG_DISTRO_DIR............[%PKG_DISTRO_DIR%]
ECHO   PKG_PRODUCT_DIR...........[%PKG_PRODUCT_DIR%]
ECHO   PKG_TARGET_DIR............[%PKG_TARGET_DIR%]
ECHO   PKG_TARGET................[%PKG_TARGET%]

CALL :CHECK_LDRAW_DIR

ECHO.
ECHO   Build checks start...
ECHO ------------------------------------------------
ECHO.
IF NOT EXIST "%PKG_TARGET%" (
  ECHO -ERROR - %PKG_TARGET% does not exist, build check will exit.
  EXIT /b
) ELSE (
  ECHO -%PKG_TARGET% found.
  IF EXIST "Check.out" DEL /Q "Check.out"
  ECHO.
  ECHO   1 OF 7. PKG_CHECK_NATIVE_COMMAND...[%PKG_CHECK_NATIVE_COMMAND%]
  CALL %PKG_CHECK_NATIVE_COMMAND% > Check.out 2>&1
  FOR %%R IN (Check.out) DO (
    IF NOT %%~zR LSS 1 (
      ECHO -BUILD_CHECK_NATIVE Output...
      TYPE "Check.out"
      ECHO.
      DEL /Q "Check.out"
      ECHO.
    ) ELSE (
      ECHO. -ERROR - BUILD_CHECK_NATIVE failed.
    )
  )
  ECHO.
  ECHO   2 OF 7. PKG_CHECK_LDVIEW_COMMAND...[%PKG_CHECK_LDVIEW_COMMAND%]
  CALL %PKG_CHECK_LDVIEW_COMMAND% > Check.out 2>&1
  FOR %%R IN (Check.out) DO (
    IF NOT %%~zR LSS 1 (
      ECHO -BUILD_CHECK_LDVIEW Output...
      TYPE "Check.out"
      ECHO.
      DEL /Q "Check.out"
      ECHO.
    ) ELSE (
      ECHO. -ERROR - BUILD_CHECK_LDVIEW failed.
    )
  )
  ECHO.
  ECHO   3 OF 7. PKG_CHECK_LDVIEW_SINGLE_CALL_COMMAND...[%PKG_CHECK_LDVIEW_COMMAND%]
  CALL %PKG_CHECK_LDVIEW_SINGLE_CALL_COMMAND% > Check.out 2>&1
  FOR %%R IN (Check.out) DO (
    IF NOT %%~zR LSS 1 (
      ECHO -BUILD_CHECK_LDVIEW_SINGLE_CALL Output...
      TYPE "Check.out"
      ECHO.
      DEL /Q "Check.out"
      ECHO.
    ) ELSE (
      ECHO. -ERROR - BUILD_CHECK_LDVIEW_SINGLE_CALL failed.
    )
  )
  ECHO.
  ECHO   4 OF 7. PKG_CHECK_RANGE_COMMAND....[%PKG_CHECK_RANGE_COMMAND%]
  CALL %PKG_CHECK_RANGE_COMMAND% > Check.out 2>&1
  FOR %%R IN (Check.out) DO (
    IF NOT %%~zR LSS 1 (
      ECHO -BUILD_CHECK_RANGE Output...
      TYPE "Check.out"
      ECHO.
      DEL /Q "Check.out"
      ECHO.
    ) ELSE (
      ECHO. -ERROR - BUILD_CHECK_RANGE failed.
    )
  )
  ECHO.
  ECHO   5 OF 7. PKG_CHECK_POV_COMMAND......[%PKG_CHECK_POV_COMMAND%]
  CALL %PKG_CHECK_POV_COMMAND% > Check.out 2>&1
  FOR %%R IN (Check.out) DO (
    IF NOT %%~zR LSS 1 (
      ECHO -BUILD_CHECK_POV Output...
      TYPE "Check.out"
      ECHO.
      DEL /Q "Check.out"
      ECHO.
    ) ELSE (
      ECHO. -ERROR - BUILD_CHECK_POV failed.
    )
  )
  ECHO.
  ECHO   6 OF 7. PKG_CHECK_TENTE_COMMAND......[%PKG_CHECK_TENTE_COMMAND%]
  CALL %PKG_CHECK_TENTE_COMMAND% > Check.out 2>&1
  FOR %%R IN (Check.out) DO (
    IF NOT %%~zR LSS 1 (
      ECHO -BUILD_CHECK_TENTE Output...
      TYPE "Check.out"
      ECHO.
      DEL /Q "Check.out"
      ECHO.
    ) ELSE (
      ECHO. -ERROR - BUILD_CHECK_TENTE failed.
    )
  )
  ECHO.
  ECHO   7 OF 7. PKG_CHECK_VEXIQ_COMMAND......[%PKG_CHECK_VEXIQ_COMMAND%]
  CALL %PKG_CHECK_VEXIQ_COMMAND% > Check.out 2>&1
  FOR %%R IN (Check.out) DO (
    IF NOT %%~zR LSS 1 (
      ECHO -BUILD_CHECK_VEXIQ Output...
      TYPE "Check.out"
      ECHO.
      DEL /Q "Check.out"
      ECHO.
    ) ELSE (
      ECHO. -ERROR - BUILD_CHECK_VEXIQ failed.
    )
  )
  ECHO.
  ECHO   Build checks cleanup...
  RMDIR /S /Q %PKG_TARGET_DIR%\cache
  RMDIR /S /Q %PKG_TARGET_DIR%\logs
  RMDIR /S /Q %PKG_TARGET_DIR%\config
  RMDIR /S /Q %PKG_TARGET_DIR%\libraries

  ECHO.
  ECHO   Build checks completed.
  ECHO ------------------------------------------------
  ECHO.
)

:CHECK_LDRAW_DIR
ECHO.
ECHO -Check for LDraw archive libraries

SET OfficialCONTENT=complete.zip
SET UnOfficialCONTENT=ldrawunf.zip
SET LPub3DCONTENT=lpub3dldrawunf.zip
SET TenteCONTENT=tenteparts.zip
SET VexiqCONTENT=vexiqparts.zip

CALL :SET_LDRAW_LIBS

ECHO.
ECHO -Check for LDraw LEGO disk library...
IF NOT EXIST "%LDRAW_DIR%\parts" (
  ECHO.
  ECHO -LDraw directory %LDRAW_DIR% does not exist - creating...
  REM SET CHECK=0

  COPY /V /Y "%LDRAW_LIBS%\%OfficialCONTENT%" "%LDRAW_DOWNLOAD_DIR%\" /A | findstr /i /v /r /c:"copied\>"

  IF EXIST "%LDRAW_DOWNLOAD_DIR%\%OfficialCONTENT%" (
    IF EXIST "%zipWin64%" (
      ECHO.
      ECHO -7zip exectutable found at "%zipWin64%"
      ECHO.
      ECHO -Extracting %OfficialCONTENT%...
      ECHO.
      "%zipWin64%\7z.exe" x -o"%LDRAW_DOWNLOAD_DIR%\" "%LDRAW_DOWNLOAD_DIR%\%OfficialCONTENT%" | findstr /i /r /c:"^Extracting\>" /c:"^Everything\>"
      IF EXIST "%LDRAW_DIR%\parts" (
        ECHO.
        ECHO -LDraw directory %LDRAW_DIR% extracted.
        ECHO.
        ECHO -Cleanup %OfficialCONTENT%...
        DEL /Q "%LDRAW_DOWNLOAD_DIR%\%OfficialCONTENT%"
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
    ECHO -[WARNING] Could not find %LDRAW_DOWNLOAD_DIR%\%OfficialCONTENT%.
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

:SET_LDRAW_LIBS
ECHO.
ECHO -Requesting LDraw archive libraries download...

CALL :DOWNLOAD_LDRAW_LIBS for_build_check

IF NOT EXIST "%LDRAW_LIBS%\" (
  ECHO.
  ECHO -Create LDraw archive libs store %LDRAW_LIBS%
  MKDIR "%LDRAW_LIBS%\"
)
IF NOT EXIST "%LDRAW_LIBS%\%OfficialCONTENT%" (
  IF EXIST "%LDRAW_DOWNLOAD_DIR%\%OfficialCONTENT%" (
    MOVE /Y %LDRAW_DOWNLOAD_DIR%\%OfficialCONTENT% %LDRAW_LIBS%\%OfficialCONTENT% | findstr /i /v /r /c:"moved\>"
  ) ELSE (
    ECHO.
    ECHO -ERROR - LDraw archive libs does not exist at %LDRAW_DOWNLOAD_DIR%\%OfficialCONTENT%.
  )
)
IF NOT EXIST "%LDRAW_LIBS%\%TenteCONTENT%" (
  IF EXIST "%LDRAW_DOWNLOAD_DIR%\%TenteCONTENT%" (
    MOVE /Y %LDRAW_DOWNLOAD_DIR%\%TenteCONTENT% %LDRAW_LIBS%\%TenteCONTENT% | findstr /i /v /r /c:"moved\>"
  ) ELSE (
    ECHO.
    ECHO -ERROR - LDraw archive libs does not exist at %LDRAW_DOWNLOAD_DIR%\%TenteCONTENT%.
  )
)
IF NOT EXIST "%LDRAW_LIBS%\%VexiqCONTENT%" (
  IF EXIST "%LDRAW_DOWNLOAD_DIR%\%VexiqCONTENT%" (
    MOVE /Y %LDRAW_DOWNLOAD_DIR%\%VexiqCONTENT% %LDRAW_LIBS%\%VexiqCONTENT% | findstr /i /v /r /c:"moved\>"
  ) ELSE (
    ECHO.
    ECHO -ERROR - LDraw archive libs does not exist at %LDRAW_DOWNLOAD_DIR%\%VexiqCONTENT%.
  )
)
IF NOT EXIST "%LDRAW_LIBS%\%LPub3DCONTENT%" (
  IF EXIST "%LDRAW_DOWNLOAD_DIR%\%LPub3DCONTENT%" (
    MOVE /Y %LDRAW_DOWNLOAD_DIR%\%LPub3DCONTENT% %LDRAW_LIBS%\%LPub3DCONTENT% | findstr /i /v /r /c:"moved\>"
  ) ELSE (
    ECHO -ERROR - LDraw archive libs does not exist at %LDRAW_DOWNLOAD_DIR%\%LPub3DCONTENT%.
  )
)
IF EXIST "%LDRAW_LIBS%\%OfficialCONTENT%" (
  COPY /V /Y "%LDRAW_LIBS%\%OfficialCONTENT%" "%PKG_TARGET_DIR%\extras\" /A | findstr /i /v /r /c:"copied\>"
)
IF EXIST "%LDRAW_LIBS%\%TenteCONTENT%" (
  COPY /V /Y "%LDRAW_LIBS%\%TenteCONTENT%" "%PKG_TARGET_DIR%\extras\" /A | findstr /i /v /r /c:"copied\>"
)
IF EXIST "%LDRAW_LIBS%\%VexiqCONTENT%" (
  COPY /V /Y "%LDRAW_LIBS%\%VexiqCONTENT%" "%PKG_TARGET_DIR%\extras\" /A | findstr /i /v /r /c:"copied\>"
)
IF EXIST "%LDRAW_LIBS%\%LPub3DCONTENT%" (
  COPY /V /Y "%LDRAW_LIBS%\%LPub3DCONTENT%" "%PKG_TARGET_DIR%\extras\" /A | findstr /i /v /r /c:"copied\>"
)
EXIT /b

:DOWNLOAD_LDRAW_LIBS
ECHO.
ECHO - Download LDraw archive libraries...

SET OutputPATH=%LDRAW_DOWNLOAD_DIR%

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

IF NOT EXIST "%OutputPATH%\%OfficialCONTENT%" (
  CALL :GET_OFFICIAL_LIBRARY
)
IF NOT EXIST "%OutputPATH%\%UnOfficialCONTENT%" (
  CALL :GET_UNOFFICIAL_LIBRARY
)
IF NOT EXIST "%OutputPATH%\%TenteCONTENT%" (
  CALL :GET_TENTE_LIBRARY
)
IF NOT EXIST "%OutputPATH%\%VexiqCONTENT%" (
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
IF EXIST %OfficialCONTENT% (
  ECHO.
  ECHO - LDraw archive library %OfficialCONTENT% availble
)
EXIT /b

:GET_TENTE_LIBRARY
SET WebCONTENT="%OutputPATH%\%TenteCONTENT%"
SET WebNAME=https://github.com/trevorsandy/lpub3d_libs/releases/download/v1.0.1/%TenteCONTENT%

ECHO.
ECHO - Download archive file: %WebCONTENT%...
ECHO.
cscript //Nologo %TEMP%\$\%vbs% %WebNAME% %WebCONTENT% && @ECHO off
IF EXIST %TenteCONTENT% (
  ECHO.
  ECHO - LDraw archive library %TenteCONTENT% availble
)
EXIT /b

:GET_VEXIQ_LIBRARY
SET WebCONTENT="%OutputPATH%\%VexiqCONTENT%"
SET WebNAME=https://github.com/trevorsandy/lpub3d_libs/releases/download/v1.0.1/%VexiqCONTENT%

ECHO.
ECHO - Download archive file: %WebCONTENT%...
ECHO.
cscript //Nologo %TEMP%\$\%vbs% %WebNAME% %WebCONTENT% && @ECHO off
IF EXIST %VexiqCONTENT% (
  ECHO.
  ECHO - LDraw archive library %VexiqCONTENT% availble
)
EXIT /b

:GET_UNOFFICIAL_LIBRARY
SET WebCONTENT="%OutputPATH%\%UnofficialCONTENT%"
SET WebNAME=http://www.ldraw.org/library/unofficial/%UnofficialCONTENT%

ECHO.
ECHO - Download archive file: %WebCONTENT%...
ECHO.
cscript //Nologo %TEMP%\$\%vbs% %WebNAME% %WebCONTENT% && @ECHO off
REN %UnofficialCONTENT% %LPub3DCONTENT%
IF EXIST %LPub3DCONTENT% (
  ECHO.
  ECHO - LDraw archive library %LPub3DCONTENT% availble
)
EXIT /b