;LPub3D Uninstall Files Script Include
;Last Update: June 28, 2019
;Copyright (C) 2016 - 2020 by Trevor SANDY

;Deposit new files...
Delete "$INSTDIR\${LPub3DBuildFile}"

Delete "$INSTDIR\Qt5Concurrent.dll"
Delete "$INSTDIR\Qt5Core.dll"
Delete "$INSTDIR\Qt5Gui.dll"
Delete "$INSTDIR\Qt5Network.dll"
Delete "$INSTDIR\Qt5OpenGL.dll"
Delete "$INSTDIR\Qt5PrintSupport.dll"
Delete "$INSTDIR\Qt5Svg.dll"
Delete "$INSTDIR\Qt5Widgets.dll"
Delete "$INSTDIR\Qt5Xml.dll"

;New Stuff - Qt Libraries
Delete "$INSTDIR\D3Dcompiler_47.dll"
Delete "$INSTDIR\libEGL.dll"
Delete "$INSTDIR\libGLESV2.dll"
Delete "$INSTDIR\opengl32sw.dll"

;New Stuff - Qt Plugins
Delete "$INSTDIR\bearer\qgenericbearer.dll"
Delete "$INSTDIR\bearer\qgenericbearer.dll"
Delete "$INSTDIR\iconengines\qsvgicon.dll"
Delete "$INSTDIR\imageformats\qgif.dll"
Delete "$INSTDIR\imageformats\qicns.dll"
Delete "$INSTDIR\imageformats\qico.dll"
Delete "$INSTDIR\imageformats\qjpeg.dll"
Delete "$INSTDIR\imageformats\qsvg.dll"
Delete "$INSTDIR\imageformats\qtga.dll"
Delete "$INSTDIR\imageformats\qtiff.dll"
Delete "$INSTDIR\imageformats\qwbmp.dll"
Delete "$INSTDIR\imageformats\qwebp.dll"
Delete "$INSTDIR\printsupport\windowsprintersupport.dll"
Delete "$INSTDIR\platforms\qwindows.dll"
Delete "$INSTDIR\styles\qwindowsvistastyle.dll"

;3rd party renderer utility - LdgLite
Delete "$INSTDIR\3rdParty\${LDGliteDir}\bin\${LDGliteExe}.exe"

;3rd party renderer utility - LDView
Delete "$INSTDIR\3rdParty\${LDViewDir}\resources\config\ldview.ini"
Delete "$INSTDIR\3rdParty\${LDViewDir}\resources\config\ldviewPOV.ini"
Delete "$INSTDIR\3rdParty\${LDViewDir}\resources\config\LDViewCustomini"

;3rd party renderer utility - LPub3D_Trace
Delete "$INSTDIR\3rdParty\${LPub3D_TraceDir}\resources\config\povray.conf"
Delete "$INSTDIR\3rdParty\${LPub3D_TraceDir}\resources\config\povray.ini"

${If} ${RunningX64}
    ;MSVC 2015 Redistributables
    Delete "${INSTDIR}\vcredist_x86_64.exe"

    ;New 64bit Stuff - Qt Libraries
    Delete "$INSTDIR\libgcc_s_seh-1.dll"

	;64bit 3rd party renderer utility - LDView
	Delete "$INSTDIR\3rdParty\${LDViewDir}\bin\${LDViewExe}64.exe"

	;64bit 3rd party renderer utility - LPub3D_Trace
	Delete "$INSTDIR\3rdParty\${LPub3D_TraceDir}\bin\${LPub3D_TraceExe}64.exe"
${Else}
    ;MSVC 2015 Redistributables
    Delete "${INSTDIR}\vcredist_x86.exe"

	;New 32bit Stuff - Qt Libraries
	Delete "$INSTDIR\libgcc_s_dw2-1.dll"

	;32bit 3rd party renderer utility - LDView
	Delete "$INSTDIR\3rdParty\${LDViewDir}\bin\${LDViewExe}.exe"

	;32bit 3rd party renderer utility - LPub3D_Trace
	Delete "$INSTDIR\3rdParty\${LPub3D_TraceDir}\bin\${LPub3D_TraceExe}32.exe"
${EndIf}

;data repository folder
Delete "$INSTDIR\data\excludedParts.lst"
Delete "$INSTDIR\data\fadeStepColorParts.lst"
Delete "$INSTDIR\data\freeformAnnotations.lst"
Delete "$INSTDIR\data\LDConfig.ldr"
Delete "$INSTDIR\data\ldglite.ini"
Delete "$INSTDIR\data\ldvExport.ini"
Delete "$INSTDIR\data\PDFPrint.jpg"
Delete "$INSTDIR\data\pli.mpd"
Delete "$INSTDIR\data\pliSubstituteParts.lst"
Delete "$INSTDIR\data\titleAnnotations.lst"
Delete "$INSTDIR\data\ldvMessages.ini"

;ldraw libraries - data/portable location
Delete "$INSTDIR\data\complete.zip"
Delete "$INSTDIR\data\lpub3dldrawunf.zip"
Delete "$INSTDIR\data\tenteparts.zip"
Delete "$INSTDIR\data\vexiqparts.zip"

;documents
Delete "$INSTDIR\docs\COPYING.txt"
Delete "$INSTDIR\docs\CREDITS.txt"
Delete "$INSTDIR\docs\README.txt"
Delete "$INSTDIR\docs\RELEASE_NOTES.html"

;3rd party renderer utility - LdgLite
Delete "$INSTDIR\3rdParty\${LDGliteDir}\doc\LICENCE"
Delete "$INSTDIR\3rdParty\${LDGliteDir}\doc\README.TXT"

;3rd party renderer utility - LDView
Delete "$INSTDIR\3rdParty\${LDViewDir}\doc\ChangeHistory.html"
Delete "$INSTDIR\3rdParty\${LDViewDir}\doc\Help.html"
Delete "$INSTDIR\3rdParty\${LDViewDir}\doc\License.txt"
Delete "$INSTDIR\3rdParty\${LDViewDir}\doc\Readme.txt"
Delete "$INSTDIR\3rdParty\${LDViewDir}\resources\8464.mpd"
Delete "$INSTDIR\3rdParty\${LDViewDir}\resources\LGEO.xml"
Delete "$INSTDIR\3rdParty\${LDViewDir}\resources\m6459.ldr"

;3rd party renderer utility - LPub3D_Trace
Delete "$INSTDIR\3rdParty\${LPub3D_TraceDir}\doc\AUTHORS.txt"
Delete "$INSTDIR\3rdParty\${LPub3D_TraceDir}\doc\ChangeLog.txt"
Delete "$INSTDIR\3rdParty\${LPub3D_TraceDir}\doc\CUI_README.txt"
Delete "$INSTDIR\3rdParty\${LPub3D_TraceDir}\doc\LICENSE.txt"

Delete "$INSTDIR\3rdParty\${LPub3D_TraceDir}\resources\include\arrays.inc"
Delete "$INSTDIR\3rdParty\${LPub3D_TraceDir}\resources\include\ash.map"
Delete "$INSTDIR\3rdParty\${LPub3D_TraceDir}\resources\include\benediti.map"
Delete "$INSTDIR\3rdParty\${LPub3D_TraceDir}\resources\include\bubinga.map"
Delete "$INSTDIR\3rdParty\${LPub3D_TraceDir}\resources\include\bumpmap_.png"
Delete "$INSTDIR\3rdParty\${LPub3D_TraceDir}\resources\include\cedar.map"
Delete "$INSTDIR\3rdParty\${LPub3D_TraceDir}\resources\include\chars.inc"
Delete "$INSTDIR\3rdParty\${LPub3D_TraceDir}\resources\include\colors.inc"
Delete "$INSTDIR\3rdParty\${LPub3D_TraceDir}\resources\include\colors_ral.inc"
Delete "$INSTDIR\3rdParty\${LPub3D_TraceDir}\resources\include\consts.inc"
Delete "$INSTDIR\3rdParty\${LPub3D_TraceDir}\resources\include\crystal.ttf"
Delete "$INSTDIR\3rdParty\${LPub3D_TraceDir}\resources\include\cyrvetic.ttf"
Delete "$INSTDIR\3rdParty\${LPub3D_TraceDir}\resources\include\debug.inc"
Delete "$INSTDIR\3rdParty\${LPub3D_TraceDir}\resources\include\finish.inc"
Delete "$INSTDIR\3rdParty\${LPub3D_TraceDir}\resources\include\fract003.png"
Delete "$INSTDIR\3rdParty\${LPub3D_TraceDir}\resources\include\functions.inc"
Delete "$INSTDIR\3rdParty\${LPub3D_TraceDir}\resources\include\glass.inc"
Delete "$INSTDIR\3rdParty\${LPub3D_TraceDir}\resources\include\glass_old.inc"
Delete "$INSTDIR\3rdParty\${LPub3D_TraceDir}\resources\include\golds.inc"
Delete "$INSTDIR\3rdParty\${LPub3D_TraceDir}\resources\include\ior.inc"
Delete "$INSTDIR\3rdParty\${LPub3D_TraceDir}\resources\include\logo.inc"
Delete "$INSTDIR\3rdParty\${LPub3D_TraceDir}\resources\include\makegrass.inc"
Delete "$INSTDIR\3rdParty\${LPub3D_TraceDir}\resources\include\marbteal.map"
Delete "$INSTDIR\3rdParty\${LPub3D_TraceDir}\resources\include\math.inc"
Delete "$INSTDIR\3rdParty\${LPub3D_TraceDir}\resources\include\meshmaker.inc"
Delete "$INSTDIR\3rdParty\${LPub3D_TraceDir}\resources\include\metals.inc"
Delete "$INSTDIR\3rdParty\${LPub3D_TraceDir}\resources\include\Mount1.png"
Delete "$INSTDIR\3rdParty\${LPub3D_TraceDir}\resources\include\Mount2.png"
Delete "$INSTDIR\3rdParty\${LPub3D_TraceDir}\resources\include\mtmand.pot"
Delete "$INSTDIR\3rdParty\${LPub3D_TraceDir}\resources\include\mtmandj.png"
Delete "$INSTDIR\3rdParty\${LPub3D_TraceDir}\resources\include\orngwood.map"
Delete "$INSTDIR\3rdParty\${LPub3D_TraceDir}\resources\include\pinkmarb.map"
Delete "$INSTDIR\3rdParty\${LPub3D_TraceDir}\resources\include\plasma2.png"
Delete "$INSTDIR\3rdParty\${LPub3D_TraceDir}\resources\include\plasma3.png"
Delete "$INSTDIR\3rdParty\${LPub3D_TraceDir}\resources\include\povlogo.ttf"
Delete "$INSTDIR\3rdParty\${LPub3D_TraceDir}\resources\include\povmap.png"
Delete "$INSTDIR\3rdParty\${LPub3D_TraceDir}\resources\include\rad_def.inc"
Delete "$INSTDIR\3rdParty\${LPub3D_TraceDir}\resources\include\rand.inc"
Delete "$INSTDIR\3rdParty\${LPub3D_TraceDir}\resources\include\rdgranit.map"
Delete "$INSTDIR\3rdParty\${LPub3D_TraceDir}\resources\include\screen.inc"
Delete "$INSTDIR\3rdParty\${LPub3D_TraceDir}\resources\include\shapes.inc"
Delete "$INSTDIR\3rdParty\${LPub3D_TraceDir}\resources\include\shapes_old.inc"
Delete "$INSTDIR\3rdParty\${LPub3D_TraceDir}\resources\include\shapes2.inc"
Delete "$INSTDIR\3rdParty\${LPub3D_TraceDir}\resources\include\shapes3.inc"
Delete "$INSTDIR\3rdParty\${LPub3D_TraceDir}\resources\include\shapesq.inc"
Delete "$INSTDIR\3rdParty\${LPub3D_TraceDir}\resources\include\skies.inc"
Delete "$INSTDIR\3rdParty\${LPub3D_TraceDir}\resources\include\spiral.df3"
Delete "$INSTDIR\3rdParty\${LPub3D_TraceDir}\resources\include\stage1.inc"
Delete "$INSTDIR\3rdParty\${LPub3D_TraceDir}\resources\include\stars.inc"
Delete "$INSTDIR\3rdParty\${LPub3D_TraceDir}\resources\include\stdcam.inc"
Delete "$INSTDIR\3rdParty\${LPub3D_TraceDir}\resources\include\stdinc.inc"
Delete "$INSTDIR\3rdParty\${LPub3D_TraceDir}\resources\include\stoneold.inc"
Delete "$INSTDIR\3rdParty\${LPub3D_TraceDir}\resources\include\stones.inc"
Delete "$INSTDIR\3rdParty\${LPub3D_TraceDir}\resources\include\stones1.inc"
Delete "$INSTDIR\3rdParty\${LPub3D_TraceDir}\resources\include\stones2.inc"
Delete "$INSTDIR\3rdParty\${LPub3D_TraceDir}\resources\include\strings.inc"
Delete "$INSTDIR\3rdParty\${LPub3D_TraceDir}\resources\include\sunpos.inc"
Delete "$INSTDIR\3rdParty\${LPub3D_TraceDir}\resources\include\teak.map"
Delete "$INSTDIR\3rdParty\${LPub3D_TraceDir}\resources\include\test.png"
Delete "$INSTDIR\3rdParty\${LPub3D_TraceDir}\resources\include\textures.inc"
Delete "$INSTDIR\3rdParty\${LPub3D_TraceDir}\resources\include\timrom.ttf"
Delete "$INSTDIR\3rdParty\${LPub3D_TraceDir}\resources\include\transforms.inc"
Delete "$INSTDIR\3rdParty\${LPub3D_TraceDir}\resources\include\ttffonts.cat"
Delete "$INSTDIR\3rdParty\${LPub3D_TraceDir}\resources\include\whiteash.map"
Delete "$INSTDIR\3rdParty\${LPub3D_TraceDir}\resources\include\woodmaps.inc"
Delete "$INSTDIR\3rdParty\${LPub3D_TraceDir}\resources\include\woods.inc"

Delete "$INSTDIR\3rdParty\${LPub3D_TraceDir}\resources\ini\allanim.ini"
Delete "$INSTDIR\3rdParty\${LPub3D_TraceDir}\resources\ini\allstill.ini"
Delete "$INSTDIR\3rdParty\${LPub3D_TraceDir}\resources\ini\low.ini"
Delete "$INSTDIR\3rdParty\${LPub3D_TraceDir}\resources\ini\pngflc.ini"
Delete "$INSTDIR\3rdParty\${LPub3D_TraceDir}\resources\ini\pngfli.ini"
Delete "$INSTDIR\3rdParty\${LPub3D_TraceDir}\resources\ini\povray.ini"
Delete "$INSTDIR\3rdParty\${LPub3D_TraceDir}\resources\ini\quickres.ini"
Delete "$INSTDIR\3rdParty\${LPub3D_TraceDir}\resources\ini\res120.ini"
Delete "$INSTDIR\3rdParty\${LPub3D_TraceDir}\resources\ini\res1k.ini"
Delete "$INSTDIR\3rdParty\${LPub3D_TraceDir}\resources\ini\res320.ini"
Delete "$INSTDIR\3rdParty\${LPub3D_TraceDir}\resources\ini\res640.ini"
Delete "$INSTDIR\3rdParty\${LPub3D_TraceDir}\resources\ini\res800.ini"
Delete "$INSTDIR\3rdParty\${LPub3D_TraceDir}\resources\ini\slow.ini"
Delete "$INSTDIR\3rdParty\${LPub3D_TraceDir}\resources\ini\tgaflc.ini"
Delete "$INSTDIR\3rdParty\${LPub3D_TraceDir}\resources\ini\tgafli.ini"
Delete "$INSTDIR\3rdParty\${LPub3D_TraceDir}\resources\ini\zipflc.ini"
Delete "$INSTDIR\3rdParty\${LPub3D_TraceDir}\resources\ini\zipfli.ini"

; Remove Qt plugin folders
RMDir "$INSTDIR\bearer"
RMDir "$INSTDIR\iconengines"
RMDir "$INSTDIR\imageformats"
RMDir "$INSTDIR\printsupport"
RMDir "$INSTDIR\platforms"

; Remove LPub3D data folders
RMDir "$INSTDIR\data"
RMDir "$INSTDIR\docs"

; Remove 3rdParty folders
RMDir "$INSTDIR\3rdParty\${LDGliteDir}\bin"
RMDir "$INSTDIR\3rdParty\${LDGliteDir}\doc"
RMDir "$INSTDIR\3rdParty\${LDGliteDir}"

RMDir "$INSTDIR\3rdParty\${LDViewDir}\bin"
RMDir "$INSTDIR\3rdParty\${LDViewDir}\doc"
RMDir "$INSTDIR\3rdParty\${LDViewDir}\resources\config"
RMDir "$INSTDIR\3rdParty\${LDViewDir}\resources"
RMDir "$INSTDIR\3rdParty\${LDViewDir}"

RMDir "$INSTDIR\3rdParty\${LPub3D_TraceDir}\bin"
RMDir "$INSTDIR\3rdParty\${LPub3D_TraceDir}\doc"
RMDir "$INSTDIR\3rdParty\${LPub3D_TraceDir}\resources\ini"
RMDir "$INSTDIR\3rdParty\${LPub3D_TraceDir}\resources\include"
RMDir "$INSTDIR\3rdParty\${LPub3D_TraceDir}\resources\config"
RMDir "$INSTDIR\3rdParty\${LPub3D_TraceDir}\resources"
RMDir "$INSTDIR\3rdParty\${LPub3D_TraceDir}"
RMDir "$INSTDIR\3rdParty"
