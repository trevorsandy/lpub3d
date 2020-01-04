@ECHO OFF &SETLOCAL

Title LPub3D Windows auto build script

rem This script uses Qt to configure and build LPub3D for Windows.
rem The primary purpose is to automatically build both the 32bit and 64bit
rem LPub3D distributions and package the build contents (exe, doc and
rem resources ) for distribution release.
rem --
rem  Trevor SANDY <trevor.sandy@gmail.com>
rem  Last Update: June 29, 2019
rem  Copyright (c) 2017 - 2020 by Trevor SANDY
rem --
rem This script is distributed in the hope that it will be useful,
rem but WITHOUT ANY WARRANTY; without even the implied warranty of
rem MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

CALL :ELAPSED_BUILD_TIME Start

FOR %%* IN (.) DO SET SCRIPT_DIR=%%~nx*
IF "%SCRIPT_DIR%" EQU "windows" (
  CALL :WD_REL_TO_ABS ..\..\
) ELSE (
  SET ABS_WD=%CD%
)

rem Variables - change these as required by your build environments
IF "%APPVEYOR%" EQU "True" (
  IF [%LP3D_DIST_DIR_PATH%] == [] (
    ECHO.
    ECHO  -ERROR: Distribution directory path not defined.
    ECHO  -%~nx0 terminated!
    GOTO :END
  )
  SET ABS_WD=%APPVEYOR_BUILD_FOLDER%
  SET DIST_DIR=%LP3D_DIST_DIR_PATH%
  SET PACKAGE=%LP3D_PACKAGE%
  SET CONFIGURATION=%configuration%
  SET LDRAW_INSTALL_ROOT=%APPVEYOR_BUILD_FOLDER%
  SET LDRAW_LIBS=%APPVEYOR_BUILD_FOLDER%\LDrawLibs
  SET LDRAW_DIR=%APPVEYOR_BUILD_FOLDER%\LDraw
  SET LP3D_QT32_MSVC=C:\Qt\5.11.3\msvc2015\bin
  SET LP3D_QT64_MSVC=C:\Qt\5.11.3\msvc2015_64\bin
  SET LP3D_VCVARSALL=C:\Program Files ^(x86^)\Microsoft Visual Studio 14.0\VC
  SET UPDATE_LDRAW_LIBS=%LP3D_UPDATE_LDRAW_LIBS_VAR%
) ELSE (
  CALL :DIST_DIR_REL_TO_ABS ..\lpub3d_windows_3rdparty
  SET PACKAGE=LPub3D
  SET CONFIGURATION=release
  SET LDRAW_INSTALL_ROOT=%USERPROFILE%
  SET LDRAW_LIBS=%USERPROFILE%
  SET LDRAW_DIR=%USERPROFILE%\LDraw
  SET LP3D_QT32_MSVC=C:\Qt\IDE\5.11.3\msvc2015\bin
  SET LP3D_QT64_MSVC=C:\Qt\IDE\5.11.3\msvc2015_64\bin
  SET LP3D_VCVARSALL=C:\Program Files ^(x86^)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build
  SET UPDATE_LDRAW_LIBS=unknown
)

SET LP3D_WIN_GIT=%ProgramFiles%\Git\cmd
SET LP3D_WIN_GIT_MSG=%LP3D_WIN_GIT%
SET SYS_DIR=%SystemRoot%\System32
SET zipWin64=C:\program files\7-zip

SET OfficialCONTENT=complete.zip
SET UnOfficialCONTENT=ldrawunf.zip
SET LPub3DCONTENT=lpub3dldrawunf.zip
SET TenteCONTENT=tenteparts.zip
SET VexiqCONTENT=vexiqparts.zip

SET BUILD_THIRD=unknown
SET INSTALL=unknown
SET INSTALL_32BIT=unknown
SET INSTALL_64BIT=unknown
SET PLATFORM=unknown
SET LDCONFIG_FILE=unknown
SET CHECK=unknown

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

rem Parse platform input flags
IF [%1]==[] (
  IF NOT EXIST "%LP3D_QT32_MSVC%" GOTO :LIBRARY_ERROR
  IF NOT EXIST "%LP3D_QT64_MSVC%" GOTO :LIBRARY_ERROR
  SET PLATFORM=-all
  GOTO :SET_CONFIGURATION
)

IF /I "%1"=="x86" (
  IF NOT EXIST "%LP3D_QT32_MSVC%" GOTO :LIBRARY_ERROR
  SET PLATFORM=x86
  GOTO :SET_CONFIGURATION
)

IF /I "%1"=="x86_64" (
  IF NOT EXIST "%LP3D_QT64_MSVC%" GOTO :LIBRARY_ERROR
  SET PLATFORM=x86_64
  GOTO :SET_CONFIGURATION
)

IF /I "%1"=="-all" (
  IF NOT EXIST "%LP3D_QT32_MSVC%" GOTO :LIBRARY_ERROR
  IF NOT EXIST "%LP3D_QT64_MSVC%" GOTO :LIBRARY_ERROR
  SET PLATFORM=-all
  GOTO :SET_CONFIGURATION
)

IF /I "%1"=="-help" (
  GOTO :USAGE
)

rem If we get here display invalid command message.
GOTO :COMMAND_ERROR

:SET_CONFIGURATION
rem Verify 2nd input flag options
IF NOT [%2]==[] (
  IF NOT "%2"=="-ins" (
    IF NOT "%2"=="-chk" (
      IF NOT "%2"=="-3rd" (
        IF NOT "%2"=="-ren" GOTO :CONFIGURATION_ERROR
      )
    )
  )
)

rem Verify 3rd input flag options
IF NOT [%3]==[] (
  IF NOT "%3"=="-ins" (
    IF NOT "%3"=="-chk" (
      IF NOT "%3"=="-uld" (
        IF NOT "%3"=="-official" (
          IF NOT "%3"=="-unofficial" (
            IF NOT "%3"=="-tente" (
              IF NOT "%3"=="-vexiq" GOTO :CONFIGURATION_ERROR
            )
          )
        )
      )
    )
  )
)

rem Verify 4th input flag options
IF NOT [%4]==[] (
  IF NOT "%4"=="-chk" (
    IF NOT "%4"=="-uld" (
      IF NOT "%4"=="-official" (
        IF NOT "%4"=="-unofficial" (
          IF NOT "%4"=="-tente" (
            IF NOT "%4"=="-vexiq" GOTO :CONFIGURATION_ERROR
          )
        )
      )
    )
  )
)

rem Set third party install as default behaviour
IF [%2]==[] (
  SET INSTALL=1
  GOTO :BUILD
)

IF /I "%2"=="-ins" (
  SET INSTALL=1
  GOTO :BUILD
)

rem Set build check flag
IF /I "%2"=="-chk" (
  SET CHECK=1
  GOTO :BUILD
)

IF /I "%2"=="-3rd" (
  SET BUILD_THIRD=1
  GOTO :BUILD
)

IF /I "%2"=="-ren" (
  SET RENDERERS_ONLY=1
  GOTO :BUILD
)

rem If we get here display invalid command message.
GOTO :COMMAND_ERROR

:BUILD
IF NOT EXIST "%LP3D_WIN_GIT%" (
  SET LP3D_WIN_GIT=
  SET LP3D_WIN_GIT_MSG=Not Found
)

rem Display build settings
ECHO.
IF "%APPVEYOR%" EQU "True" (
  ECHO   BUILD_HOST.....................[APPVEYOR CONTINUOUS INTEGRATION SERVICE]
  ECHO   BUILD_ID.......................[%APPVEYOR_BUILD_ID%]
  ECHO   BUILD_BRANCH...................[%APPVEYOR_REPO_BRANCH%]
  ECHO   PROJECT_NAME...................[%APPVEYOR_PROJECT_NAME%]
  ECHO   REPOSITORY_NAME................[%APPVEYOR_REPO_NAME%]
  ECHO   REPO_PROVIDER..................[%APPVEYOR_REPO_PROVIDER%]
  ECHO   LP3D_WIN_GIT_DIR...............[%LP3D_WIN_GIT_MSG%]
)
ECHO   PACKAGE........................[%PACKAGE%]
ECHO   CONFIGURATION..................[%CONFIGURATION%]
ECHO   LP3D_QT32_MSVC.................[%LP3D_QT32_MSVC%]
ECHO   LP3D_QT64_MSVC.................[%LP3D_QT64_MSVC%]
ECHO   WORKING_DIRECTORY_LPUB3D.......[%ABS_WD%]
ECHO   DISTRIBUTION_DIRECTORY.........[%DIST_DIR%]
ECHO   LDRAW_DIRECTORY................[%LDRAW_DIR%]
ECHO   LDRAW_INSTALL_ROOT.............[%LDRAW_INSTALL_ROOT%]
ECHO   LDRAW_LIBS.....................[%LDRAW_LIBS%]
ECHO.

rem set application version variables
SET _PRO_FILE_PWD_=%ABS_WD%\mainApp
CALL builds\utilities\update-config-files.bat %_PRO_FILE_PWD_%

rem Perform 3rd party content install
IF /I "%3"=="-ins" (
 SET INSTALL=1
)

rem Perform build check
IF /I "%3"=="-chk" (
  SET CHECK=1
)

rem Force update LDraw libraries
IF NOT [%3]==[] (
  IF NOT "%3"=="-ins" (
    IF NOT "%3"=="-chk" (
      IF NOT "%3"=="-uld" (
        SET UPDATE_LDRAW_LIBS=%3
      ) ELSE (
        SET UPDATE_LDRAW_LIBS=-true
      )
    )
  )
)

IF /I "%4"=="-chk" (
  SET CHECK=1
)

rem Force update LDraw libraries
IF NOT [%4]==[] (
  IF NOT "%4"=="-chk" (
    IF NOT "%4"=="-uld" (
      SET UPDATE_LDRAW_LIBS=%4
    ) ELSE (
      SET UPDATE_LDRAW_LIBS=-true
    )
  )
)

rem Create distribution folder
IF NOT EXIST "%DIST_DIR%\" (
  MKDIR "%DIST_DIR%\"
)

rem Stage Install prior to build check
IF /I %CHECK%==1 (
  SET INSTALL=1
)

rem Check if build renderers
IF /I "%RENDERERS_ONLY%"=="1" (
  GOTO :BUILD_RENDERERS
)

rem Check if build all platforms
IF /I "%PLATFORM%"=="-all" (
  GOTO :BUILD_ALL
)

SET platform_build_start=%time%

rem Configure build arguments and set environment variables
CALL :CONFIGURE_BUILD_ENV
CD /D "%ABS_WD%"
ECHO.
ECHO -Building %PACKAGE% %PLATFORM% platform, %CONFIGURATION% configuration...
rem Build 3rd party build from source
IF %BUILD_THIRD%==1 CALL builds\utilities\CreateRenderers.bat %PLATFORM%
rem Display QMake version
ECHO.
qmake -v & ECHO.
rem Configure makefiles
qmake %LPUB3D_CONFIG_ARGS%
rem perform build
nmake.exe
rem Package 3rd party install content - this must come before check so check can use staged content for test
IF %INSTALL%==1 CALL :STAGE_INSTALL
CALL :ELAPSED_BUILD_TIME %platform_build_start%
ECHO.
ECHO -Elapsed %PLATFORM% package build time %LP3D_ELAPSED_BUILD_TIME%
rem Perform build check if specified
IF %CHECK%==1 CALL :BUILD_CHECK %PLATFORM%
GOTO :END

:BUILD_ALL
rem Launch qmake/make across all platform builds
ECHO.
ECHO -Build LPub3D x86 and x86_64 platforms...
FOR %%P IN ( x86, x86_64 ) DO (
  SETLOCAL ENABLEDELAYEDEXPANSION
  SET platform_build_start=%time%
  SET PLATFORM=%%P
  rem Configure buid arguments and set environment variables
  CALL :CONFIGURE_BUILD_ENV
  CD /D "%ABS_WD%"
  rem Build 3rd party build from source
  IF %BUILD_THIRD%==1 ECHO.
  IF %BUILD_THIRD%==1 ECHO -----------------------------------------------------
  IF %BUILD_THIRD%==1 CALL builds\utilities\CreateRenderers.bat %%P
  ECHO.
  ECHO -Building  %PACKAGE% %%P platform, %CONFIGURATION% configuration...
  ECHO.
  rem Display QMake version
  qmake -v & ECHO.
  rem Configure makefiles and launch make
  qmake !LPUB3D_CONFIG_ARGS! & nmake.exe !LPUB3D_MAKE_ARGS!
  rem Package 3rd party install content - this must come before check so check can use staged content for test
  IF %INSTALL%==1 CALL :STAGE_INSTALL
  CALL :ELAPSED_BUILD_TIME !platform_build_start!
  ECHO.
  ECHO -Elapsed %%P package build time !LP3D_ELAPSED_BUILD_TIME!
  SETLOCAL DISABLEDELAYEDEXPANSION
  rem Perform build check if specified
  IF %CHECK%==1 CALL :BUILD_CHECK %%P
)
GOTO :END

:BUILD_RENDERERS
rem Check if build all platforms
IF /I "%PLATFORM%"=="-all" (
  GOTO :BUILD_ALL_RENDERERS
)

rem Configure buid arguments and set environment variables
CALL :CONFIGURE_BUILD_ENV
CD /D "%ABS_WD%"
rem Build renderer from source
ECHO.
ECHO -Building Renderers for %PLATFORM% platform, %CONFIGURATION% configuration...
ECHO -----------------------------------------------------
CALL builds\utilities\CreateRenderers.bat %PLATFORM%
GOTO :END

:BUILD_ALL_RENDERERS
FOR %%P IN ( x86, x86_64 ) DO (
  SET PLATFORM=%%P
  rem Configure build arguments and set environment variables
  CALL :CONFIGURE_BUILD_ENV
  CD /D "%ABS_WD%"
  rem Build renderer from source
  ECHO.
  ECHO -Building Renderers for %%P platform, %CONFIGURATION% configuration...
  ECHO -----------------------------------------------------
  CALL builds\utilities\CreateRenderers.bat %%P
)
GOTO :END

:CONFIGURE_BUILD_ENV
CD /D %ABS_WD%
ECHO.
ECHO -Configure LPub3D %PLATFORM% build environment...
ECHO.
ECHO -Cleanup any previous LPub3D qmake config files...
FOR /R %%I IN (
  ".qmake.stash"
  "Makefile*"
  "lclib\Makefile*"
  "ldrawini\Makefile*"
  "ldvlib\LDVQt\Makefile*"
  "mainApp\Makefile*"
  "quazip\Makefile*"
) DO DEL /S /Q "%%~I" >NUL 2>&1
ECHO.
ECHO   PLATFORM (BUILD_ARCH)..........[%PLATFORM%]
SET PATH=%SYS_DIR%;%LP3D_WIN_GIT%
SET LPUB3D_CONFIG_ARGS=CONFIG+=%CONFIGURATION% CONFIG-=debug_and_release
IF "%APPVEYOR%" EQU "True" (
  IF "%LP3D_BUILD_PKG%" EQU "yes" (
    ECHO   LP3D_BUILD_PKG.................[%LP3D_BUILD_PKG%]
  )
  IF %CHECK% EQU 1 (
    ECHO   LP3D_BUILD_CHECK...............[Yes]
    SET LPUB3D_CONFIG_ARGS=%LPUB3D_CONFIG_ARGS% CONFIG+=exe CONFIG+=appveyor_ci
  ) ELSE (
    ECHO   LP3D_BUILD_CHECK...............[No]
    SET LPUB3D_CONFIG_ARGS=%LPUB3D_CONFIG_ARGS% CONFIG+=appveyor_ci
  )
) ELSE (
  SET LPUB3D_CONFIG_ARGS=%LPUB3D_CONFIG_ARGS% CONFIG+=exe
  SET LP3D_DIST_DIR_PATH=%CD%\%DIST_DIR%
)
rem Set vcvars for AppVeyor or local build environments
IF %PLATFORM% EQU x86 (
  ECHO.
  CALL "%LP3D_QT32_MSVC%\qtenv2.bat"
  IF "%APPVEYOR%" EQU "True" (
    CALL "%LP3D_VCVARSALL%\bin\vcvars32.bat"
  ) ELSE (
    CALL "%LP3D_VCVARSALL%\vcvars32.bat" -vcvars_ver=14.0
  )
) ELSE (
  ECHO.
  CALL "%LP3D_QT64_MSVC%\qtenv2.bat"
  IF "%APPVEYOR%" EQU "True" (
    CALL "%LP3D_VCVARSALL%\bin\amd64\vcvars64.bat"
  ) ELSE (
    CALL "%LP3D_VCVARSALL%\vcvars64.bat" -vcvars_ver=14.0
  )
)
rem Display MSVC Compiler settings
echo _MSC_VER > %TEMP%\settings.c
cl -Bv -EP %TEMP%\settings.c >NUL
ECHO.
SET LPUB3D_MAKE_ARGS=-c -f Makefile
SET PATH_PREPENDED=True

ECHO.
ECHO   LPUB3D_CONFIG_ARGS.............[%LPUB3D_CONFIG_ARGS%]
SETLOCAL ENABLEDELAYEDEXPANSION
ECHO(  PATH_PREPEND...................[!PATH!]
  ENDLOCAL
)
EXIT /b

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

:STAGE_INSTALL
ECHO.
ECHO -Staging distribution files...
ECHO.
rem Perform build and stage package components
nmake.exe %LPUB3D_MAKE_ARGS% install
EXIT /b

:REQUEST_LDRAW_DIR
ECHO.
ECHO -Request LDraw archive libraries download...

CALL :DOWNLOAD_LDRAW_LIBS

ECHO.
ECHO -Check for LDraw LEGO disk library...
IF NOT EXIST "%LDRAW_DIR%\parts" (
  ECHO.
  ECHO -LDraw directory %LDRAW_DIR% does not exist - creating...
  REM SET CHECK=0
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

IF "%UPDATE_LDRAW_LIBS%" EQU "-true" (
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

:COMMAND_ERROR
ECHO.
CALL :USAGE
ECHO.
ECHO -03. (COMMAND ERROR) Invalid command string [%~nx0 %*].
ECHO      See Usage.
GOTO :END

:LIBRARY_ERROR
ECHO.
CALL :USAGE
ECHO.
ECHO -04. (LIBRARY ERROR) Qt MSVC library [%LP3D_QT32_MSVC%] or [%LP3D_QT64_MSVC%] required for command not found [%~nx0 %*].
ECHO      See Usage.
GOTO :END

:USAGE
ECHO ----------------------------------------------------------------
ECHO.
ECHO %PACKAGE% Windows auto build script.
ECHO.
ECHO NOTE: To successfully run all options of this script locally,
ECHO you must have both Win32 and Win64 Qt for MSVC. Qt for MSVC 2015
ECHO supports both 32 and 64bit so this version is a convenient choice.
ECHO.
ECHO ----------------------------------------------------------------
ECHO Usage:
ECHO  build [ -help]
ECHO  build [ x86 ^| x86_64 ^| -all ] [ -chk ^| -ins ^| -3rd ^| -ren ] [ -chk ^| -ins ^| -uld  ^| -official ^| -unofficial ^| -tente ^| -vexiq ] [ -chk ^| -uld ^| -official ^| -unofficial ^| -tente ^| -vexiq ]
ECHO.
ECHO ----------------------------------------------------------------
ECHO Build 64bit, Release and perform build check
ECHO build x86_64 -chk
ECHO.
ECHO Build 64bit, Release and perform install and build check
ECHO build x86_64 -ins -chk
ECHO.
ECHO Build 32bit, Release and perform build check
ECHO build x86 -chk
ECHO.
ECHO Build 64bit and 32bit, 3rdParty renderers
ECHO build -all -ren
ECHO.
ECHO Build 64bit and 32bit, Release and perform build check
ECHO build -all -ren
ECHO.
ECHO Build 64bit and 32bit, Release, perform install and build check
ECHO build -all -ins -chk
ECHO.
ECHO Build 64bit and 32bit, Release, build 3rd party renderers, perform install and build check
ECHO build -all -ins -chk
ECHO.
ECHO Commands:
ECHO ----------------------------------------------------------------
ECHO [PowerShell]: cmd.exe /c builds\windows\AutoBuild.cmd [Flags]
ECHO [cmd.exe]   : builds\windows\AutoBuild.cmd [Flags]
ECHO.
ECHO Flags:
ECHO ----------------------------------------------------------------
ECHO ^| Flag    ^| Pos ^| Type             ^| Description
ECHO ----------------------------------------------------------------
ECHO  -help........1......Useage flag         [Default=Off] Display useage.
ECHO  x86..........1......Platform flag       [Default=Off] Build 32bit architecture.
ECHO  x86_64.......1......Platform flag       [Default=Off] Build 64bit architecture.
ECHO  -all.........1......Configuraiton flag  [Default=On ] Build both 32bit and 64bit architectures - Requries Qt libraries for both architectures.
ECHO  -3rd.........2......Project flag        [Default=Off] Build 3rdparty renderers - LDGLite, LDView, and LPub3D-Trace (POV-Ray) from source
ECHO  -ren.........2......Project flag        [Default=Off] Build 3rdparty renderers only - LPub3D not built
ECHO  -ins.........2,3....Project flag        [Default=On ] Install distribution as LPub3D 3rd party installation
ECHO  -chk.........2,3,4..Project flag        [Default=Off] Perform a build check
ECHO  -uld.........3,4....Project flag        [Default=Off] Force update LDraw libraries
ECHO  -unofficial..3,4....Project flag        [Default=Off] Force update Unofficial LDraw library
ECHO  -official....3,4....Project flag        [Default=Off] Force update Official LDraw library
ECHO  -tente.......3,4....Project flag        [Default=Off] Force update Tente LDraw library
ECHO  -vexiq.......3,4....Project flag        [Default=Off] Force update VEXiQ LDraw library
ECHO.
ECHO Be sure the set your LDraw directory in the variables section above if you expect to use the '-chk' option.
ECHO.
ECHO Flags are case sensitive, use lowere case.
ECHO.
ECHO If no flag is supplied, 64bit platform, Release Configuration built by default.
ECHO ----------------------------------------------------------------
EXIT /b

:END
ECHO.
ECHO -%PACKAGE% v%LP3D_VERSION% %~nx0 finished.
CALL :ELAPSED_BUILD_TIME
ECHO -Elapsed build time %LP3D_ELAPSED_BUILD_TIME%
ENDLOCAL
EXIT /b
