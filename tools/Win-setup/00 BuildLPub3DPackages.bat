@ECHO off
Title Create manual and automatic install distributions
SETLOCAL
@break off
@color 0a
ECHO Create manual and automatic install distributions
ECHO.
ECHO - Start Pack LPub3D x86_32...
ECHO - Start Pack LPub3D x86_32 	>  ../release/LPub3D.Release.build.log.txt
"C:\Program Files (x86)\Enigma Virtual Box\enigmavbconsole.exe" LPub3D_x32.evb >> ../release/LPub3D.Release.build.log.txt
ECHO. 								>>  ../release/LPub3D.Release.build.log.txt
ECHO - Finished Pack LPub3D x86_32 	>>  ../release/LPub3D.Release.build.log.txt
ECHO - Finished Pack LPub3D x86_32...
ECHO.
ECHO - Start Pack LPub3D x86_64...
ECHO. 								>>  ../release/LPub3D.Release.build.log.txt
ECHO - Start Pack LPub3D x86_64 	>>  ../release/LPub3D.Release.build.log.txt
"C:\Program Files (x86)\Enigma Virtual Box\enigmavbconsole.exe" LPub3D_x64.evb  >> ../release/LPub3D.Release.build.log.txt
ECHO. 								>>  ../release/LPub3D.Release.build.log.txt
ECHO - Finished Pack LPub3D x86_64 	>>  ../release/LPub3D.Release.build.log.txt
ECHO - Finished Pack LPub3D x86_64...
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
ECHO.
ECHO ~Finshed!~
ECHO.
ECHO If everything went well Press any key to EXIT!
PAUSE >NUL
ENDLOCAL
EXIT