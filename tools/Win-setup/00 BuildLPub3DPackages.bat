@ECHO on
Title Create manual and automatic install distributions
echo.
echo Start Pack LPub3D x86_32
@ECHO off
echo Start Pack LPub3D x86_32 		>  ../release/LPub3D.Release.build.log.txt
"C:\Program Files (x86)\Enigma Virtual Box\enigmavbconsole.exe" LPub3D_x32.evb >> ../release/LPub3D.Release.build.log.txt
echo. 								>>  ../release/LPub3D.Release.build.log.txt
echo Finished Pack LPub3D x86_32 	>>  ../release/LPub3D.Release.build.log.txt
@ECHO on
echo Finished Pack LPub3D x86_32
echo.
echo Start Pack LPub3D x86_64
@ECHO off
echo. 								>>  ../release/LPub3D.Release.build.log.txt
echo Start Pack LPub3D x86_64 		>>  ../release/LPub3D.Release.build.log.txt
"C:\Program Files (x86)\Enigma Virtual Box\enigmavbconsole.exe" LPub3D_x64.evb >> ../release/LPub3D.Release.build.log.txt
echo. 								>>  ../release/LPub3D.Release.build.log.txt
echo Finished Pack LPub3D x86_64 	>>  ../release/LPub3D.Release.build.log.txt
@ECHO on
echo Finished Pack LPub3D x86_64
echo.
echo Start NSIS Master Update
@ECHO off
echo. 								>>  ../release/LPub3D.Release.build.log.txt
echo Start NSIS Master Update   	>>  ../release/LPub3D.Release.build.log.txt
"C:\Program Files (x86)\NSIS\makensis.exe" /DUpdateMaster LPub3D.nsi 		 	>> ../release/LPub3D.Release.build.log.txt
echo. 								>>  ../release/LPub3D.Release.build.log.txt
echo Finished NSIS Master Update 	>>  ../release/LPub3D.Release.build.log.txt
@ECHO on
echo Finished NSIS Master Update
echo.
echo Start NSIS Manual Install
@ECHO off
echo. 								>>  ../release/LPub3D.Release.build.log.txt
echo Start NSIS Manual Install		>>  ../release/LPub3D.Release.build.log.txt
"C:\Program Files (x86)\NSIS\makensis.exe" LPub3D.nsi 							>> ../release/LPub3D.Release.build.log.txt
echo. 								>>  ../release/LPub3D.Release.build.log.txt
echo Finished NSIS Manual Install 	>>  ../release/LPub3D.Release.build.log.txt
@ECHO on
echo Finished NSIS Manual Install