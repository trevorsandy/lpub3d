@ECHO off
Title Create manual and automatic install distributions
SETLOCAL
@break off
@color 0a
ECHO Create manual and automatic install distributions
ECHO.
ECHO - Start NSIS Master Update...
ECHO. 								>>  ../release/LPub3D.Release.build.log.txt
ECHO - Start NSIS Master Update   	>>  ../release/LPub3D.Release.build.log.txt
"C:\Program Files (x86)\NSIS\makensis.exe" /DUpdateMaster LPub3D.nsi 		 	>> ../release/LPub3D.Release.build.log.txt
ECHO. 								>>  ../release/LPub3D.Release.build.log.txt
ECHO - Finished NSIS Master Update 	>>  ../release/LPub3D.Release.build.log.txt
ECHO - Finished NSIS Master Update...
ECHO.
ECHO - Start NSIS Manual Install...
ECHO. 								>>  ../release/LPub3D.Release.build.log.txt
ECHO - Start NSIS Manual Install	>>  ../release/LPub3D.Release.build.log.txt
"C:\Program Files (x86)\NSIS\makensis.exe" LPub3D.nsi 							>> ../release/LPub3D.Release.build.log.txt
ECHO. 								>>  ../release/LPub3D.Release.build.log.txt
ECHO - Finished NSIS Manual Install >>  ../release/LPub3D.Release.build.log.txt
ECHO - Finished NSIS Manual Install...
%SystemRoot%\explorer.exe "C:\Users\Trevor\Downloads\LEGO\LPub\project\LPub\LPub3D\tools\release"
ECHO.
ECHO ~Finshed!~
ECHO.
ECHO If everything went well Press any key to EXIT!
PAUSE >NUL
ENDLOCAL
EXIT