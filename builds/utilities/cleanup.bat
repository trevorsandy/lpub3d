@ECHO OFF & SETLOCAL
FOR %%* IN (.) DO SET SCRIPT_DIR=%%~nx*
IF "%SCRIPT_DIR%" EQU "utilities" (
  rem Convert relative WD to abs WD and set 'ABS_WD'
  CALL :WD_REL_TO_ABS ../../
) ELSE (
  rem Set 'ABS_WD' to current directory
  SET ABS_WD=%CD%
)
ECHO. 
ECHO -Cleanup previous build config files
rem Change to build root
CD %ABS_WD%
rem Process files
FOR /R %%I IN (
  ".qmake.stash"
  "Makefile*"
  "ldrawini\Makefile*"
  "quazip\Makefile*"
  "quazip\object_script.*"
  "mainApp\Makefile*"
  "mainApp\object_script.*"
) DO DEL /S /Q "%%~I" >nul 2>&1
rem Process folders
FOR /D %%I IN (
  "ldrawini\*debug"
  "quazip\*debug"
  "mainApp\*debug"
  "ldrawini\*release"
  "quazip\*release"
  "mainApp\*release"  
) DO RD /S /Q "%%~I" >nul 2>&1
GOTO :END

:WD_REL_TO_ABS
rem If nothing to do, exit
IF [%1] EQU [] (EXIT /b) ELSE (SET REL_WD=%1)
rem Validate slashes
SET REL_WD=%REL_WD:/=\%
SET ABS_WD=
rem Save current directory and change to target directory
PUSHD %REL_WD%
rem Save value of CD variable (current directory)
SET ABS_WD=%CD%
rem Restore original directory
POPD
rem Done
EXIT /b

:END
ENDLOCAL
EXIT /b