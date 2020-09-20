;LPub3D Install Files Script Include
;Last Update: June 29, 2019
;Copyright (C) 2016 - 2020 by Trevor SANDY

StrCmp ${UniversalBuild} "1" 0 SingleArchitectureBuild
; Universal architecture (x86, x86_64)
; First, delete files with old names if exist
IfFileExists "quazip.dll" 0 DeleteLdrawini
Delete "quazip.dll"
DeleteLdrawini:
IfFileExists "ldrawini.dll" 0 ContinueInstall
Delete "ldrawini.dll"
ContinueInstall:
;executable requireds and readme
${If} ${RunningX64}
    ; delete files with old names if exist
    IfFileExists "${ProductName}_${ArchExt}.exe" 0 ContinueX64Install
    Delete "${ProductName}_${ArchExt}.exe"
    ContinueX64Install:

    ;MSVC 2015 Redistributable
    File "${Win64BuildDir}\vcredist\vcredist_x86_64.exe"

    ;Deposit new 64bit files...
    File "${Win64BuildDir}\${LPub3DBuildFile}"

    File "${Win64BuildDir}\Qt5Concurrent.dll"
    File "${Win64BuildDir}\Qt5Core.dll"
    File "${Win64BuildDir}\Qt5Gui.dll"
    File "${Win64BuildDir}\Qt5Network.dll"
    File "${Win64BuildDir}\Qt5OpenGL.dll"
    File "${Win64BuildDir}\Qt5PrintSupport.dll"
    File "${Win64BuildDir}\Qt5Svg.dll"
    File "${Win64BuildDir}\Qt5Widgets.dll"
    File "${Win64BuildDir}\Qt5Xml.dll"

    ;New Stuff - Qt Libraries
    File "${Win64BuildDir}\D3Dcompiler_47.dll"
    File "${Win64BuildDir}\libEGL.dll"
    File "${Win64BuildDir}\libGLESV2.dll"
    File "${Win64BuildDir}\opengl32sw.dll"

    ;New Stuff - Qt Plugins
    CreateDirectory "$INSTDIR\bearer"
    SetOutPath "$INSTDIR\bearer"
    File "${Win64BuildDir}\bearer\qgenericbearer.dll"
;    File "${Win64BuildDir}\bearer\qgenericbearer.dll"
    CreateDirectory "$INSTDIR\iconengines"
    SetOutPath "$INSTDIR\iconengines"
    File "${Win64BuildDir}\iconengines\qsvgicon.dll"
    CreateDirectory "$INSTDIR\imageformats"
    SetOutPath "$INSTDIR\imageformats"
    File "${Win64BuildDir}\imageformats\qgif.dll"
    File "${Win64BuildDir}\imageformats\qicns.dll"
    File "${Win64BuildDir}\imageformats\qico.dll"
    File "${Win64BuildDir}\imageformats\qjpeg.dll"
    File "${Win64BuildDir}\imageformats\qsvg.dll"
    File "${Win64BuildDir}\imageformats\qtga.dll"
    File "${Win64BuildDir}\imageformats\qtiff.dll"
    File "${Win64BuildDir}\imageformats\qwbmp.dll"
    File "${Win64BuildDir}\imageformats\qwebp.dll"
    CreateDirectory "$INSTDIR\printsupport"
    SetOutPath "$INSTDIR\printsupport"
    File "${Win64BuildDir}\printsupport\windowsprintersupport.dll"
    CreateDirectory "$INSTDIR\platforms"
    SetOutPath "$INSTDIR\platforms"
    File "${Win64BuildDir}\platforms\qwindows.dll"
    SetOutPath "$INSTDIR\styles"
    File "${Win64BuildDir}\styles\qwindowsvistastyle.dll"

    ;3rd party renderer utility - LdgLite
    CreateDirectory "$INSTDIR\3rdParty\${LDGliteDir}\bin"
    SetOutPath "$INSTDIR\3rdParty\${LDGliteDir}\bin"
    File "${Win64BuildDir}\3rdParty\${LDGliteDir}\bin\${LDGliteExe}.exe"

    ;3rd party renderer utility - LDView
    CreateDirectory "$INSTDIR\3rdParty\${LDViewDir}\bin"
    SetOutPath "$INSTDIR\3rdParty\${LDViewDir}\bin"
    File "${Win64BuildDir}\3rdParty\${LDViewDir}\bin\${LDViewExe}64.exe"
    CreateDirectory "$INSTDIR\3rdParty\${LDViewDir}\resources\config"
    SetOutPath "$INSTDIR\3rdParty\${LDViewDir}\resources\config"
    ${If} $OverwriteUserDataParamFiles == 0
     SetOverwrite off
    ${EndIf}
    File "${Win64BuildDir}\3rdParty\${LDViewDir}\resources\config\ldview.ini"
    File "${Win64BuildDir}\3rdParty\${LDViewDir}\resources\config\ldviewPOV.ini"
    File "${Win64BuildDir}\3rdParty\${LDViewDir}\resources\config\LDViewCustomini"
    ${If} $OverwriteUserDataParamFiles == 0
     SetOverwrite on
    ${EndIf}
    ;3rd party renderer utility - LPub3D_Trace
    CreateDirectory "$INSTDIR\3rdParty\${LPub3D_TraceDir}\bin"
    SetOutPath "$INSTDIR\3rdParty\${LPub3D_TraceDir}\bin"
    File "${Win64BuildDir}\3rdParty\${LPub3D_TraceDir}\bin\${LPub3D_TraceExe}64.exe"
    CreateDirectory "$INSTDIR\3rdParty\${LPub3D_TraceDir}\resources\config"
    SetOutPath "$INSTDIR\3rdParty\${LPub3D_TraceDir}\resources\config"
    ${If} $OverwriteUserDataParamFiles == 0
     SetOverwrite off
    ${EndIf}
    File "${Win64BuildDir}\3rdParty\${LPub3D_TraceDir}\resources\config\povray.conf"
    File "${Win64BuildDir}\3rdParty\${LPub3D_TraceDir}\resources\config\povray.ini"
    ${If} $OverwriteUserDataParamFiles == 0
     SetOverwrite on
    ${EndIf}

${Else}

    ; delete files with old names if exist
    IfFileExists "${ProductName}_${ArchExt}.exe" 0 ContinueX32Install
    Delete "${ProductName}_${ArchExt}.exe"
    ContinueX32Install:

    ;MSVC 2015 Redistributable
    File "${Win32BuildDir}\vcredist\vcredist_x86.exe"

    ;Deposit new 32bit files...
    ;File "${Win32BuildDir}\${LPub3D32bitBuildFile}"
    File "${Win32BuildDir}\${LPub3DBuildFile}"

    File "${Win32BuildDir}\Qt5Concurrent.dll"
    File "${Win32BuildDir}\Qt5Core.dll"
    File "${Win32BuildDir}\Qt5Gui.dll"
    File "${Win32BuildDir}\Qt5Network.dll"
    File "${Win32BuildDir}\Qt5OpenGL.dll"
    File "${Win32BuildDir}\Qt5PrintSupport.dll"
    File "${Win32BuildDir}\Qt5Svg.dll"
    File "${Win32BuildDir}\Qt5Widgets.dll"
    File "${Win32BuildDir}\Qt5Xml.dll"

    ;New Stuff - Qt Libraries
    File "${Win32BuildDir}\D3Dcompiler_47.dll"
    File "${Win32BuildDir}\libEGL.dll"
    File "${Win32BuildDir}\libGLESV2.dll"
    File "${Win32BuildDir}\opengl32sw.dll"

    ;New Stuff - Qt Plugins
    CreateDirectory "$INSTDIR\bearer"
    SetOutPath "$INSTDIR\bearer"
    File "${Win32BuildDir}\bearer\qgenericbearer.dll"
    File "${Win32BuildDir}\bearer\qgenericbearer.dll"
    CreateDirectory "$INSTDIR\iconengines"
    SetOutPath "$INSTDIR\iconengines"
    File "${Win32BuildDir}\iconengines\qsvgicon.dll"
    CreateDirectory "$INSTDIR\imageformats"
    SetOutPath "$INSTDIR\imageformats"
    ;File "${Win32BuildDir}\imageformats\qdds.dll"
    File "${Win32BuildDir}\imageformats\qgif.dll"
    File "${Win32BuildDir}\imageformats\qicns.dll"
    File "${Win32BuildDir}\imageformats\qico.dll"
    File "${Win32BuildDir}\imageformats\qjpeg.dll"
    File "${Win32BuildDir}\imageformats\qsvg.dll"
    File "${Win32BuildDir}\imageformats\qtga.dll"
    File "${Win32BuildDir}\imageformats\qtiff.dll"
    File "${Win32BuildDir}\imageformats\qwbmp.dll"
    File "${Win32BuildDir}\imageformats\qwebp.dll"
    CreateDirectory "$INSTDIR\printsupport"
    SetOutPath "$INSTDIR\printsupport"
    File "${Win32BuildDir}\printsupport\windowsprintersupport.dll"
    CreateDirectory "$INSTDIR\platforms"
    SetOutPath "$INSTDIR\platforms"
    File "${Win32BuildDir}\platforms\qwindows.dll"
    SetOutPath "$INSTDIR\styles"
    File "${Win32BuildDir}\styles\qwindowsvistastyle.dll"

    ;3rd party renderer utility - LdgLite
    CreateDirectory "$INSTDIR\3rdParty\${LDGliteDir}\bin"
    SetOutPath "$INSTDIR\3rdParty\${LDGliteDir}\bin"
    File "${Win32BuildDir}\3rdParty\${LDGliteDir}\bin\${LDGliteExe}.exe"

    ;3rd party renderer utility - LDView
    CreateDirectory "$INSTDIR\3rdParty\${LDViewDir}\bin"
    SetOutPath "$INSTDIR\3rdParty\${LDViewDir}\bin"
    File "${Win32BuildDir}\3rdParty\${LDViewDir}\bin\${LDViewExe}.exe"
    CreateDirectory "$INSTDIR\3rdParty\${LDViewDir}\resources\config"
    SetOutPath "$INSTDIR\3rdParty\${LDViewDir}\resources\config"
    ${If} $OverwriteUserDataParamFiles == 0
     SetOverwrite off
    ${EndIf}
    File "${Win32BuildDir}\3rdParty\${LDViewDir}\resources\config\ldview.ini"
    File "${Win32BuildDir}\3rdParty\${LDViewDir}\resources\config\ldviewPOV.ini"
    File "${Win32BuildDir}\3rdParty\${LDViewDir}\resources\config\LDViewCustomini"
    ${If} $OverwriteUserDataParamFiles == 0
     SetOverwrite on
    ${EndIf}

    ;3rd party renderer utility - LPub3D_Trace
    CreateDirectory "$INSTDIR\3rdParty\${LPub3D_TraceDir}\bin"
    SetOutPath "$INSTDIR\3rdParty\${LPub3D_TraceDir}\bin"
    File "${Win32BuildDir}\3rdParty\${LPub3D_TraceDir}\bin\${LPub3D_TraceExe}32.exe"
    CreateDirectory "$INSTDIR\3rdParty\${LPub3D_TraceDir}\resources\config"
    SetOutPath "$INSTDIR\3rdParty\${LPub3D_TraceDir}\resources\config"
    ${If} $OverwriteUserDataParamFiles == 0
     SetOverwrite off
    ${EndIf}
    File "${Win32BuildDir}\3rdParty\${LPub3D_TraceDir}\resources\config\povray.conf"
    File "${Win32BuildDir}\3rdParty\${LPub3D_TraceDir}\resources\config\povray.ini"
    ${If} $OverwriteUserDataParamFiles == 0
     SetOverwrite on
    ${EndIf}
${EndIf}

Goto DocumentsAndExtras

SingleArchitectureBuild:
; ;Delete files with old names if exist
; IfFileExists "${ProductName}_${ArchExt}.exe" 0
; Delete "${ProductName}_${ArchExt}.exe"
; IfFileExists "quazip.dll" 0
; Delete "quazip.dll"
; IfFileExists "ldrawini.dll" 0
; Delete "ldrawini.dll"

; ;Deposit new files...
; File "${WinBuildDir}\${LPub3DBuildFile}"

;File "${WinBuildDir}\Qt5Concurrent.dll"
;File "${WinBuildDir}\Qt5Core.dll"
;File "${WinBuildDir}\Qt5Gui.dll"
;File "${WinBuildDir}\Qt5Network.dll"
;File "${WinBuildDir}\Qt5OpenGL.dll"
;File "${WinBuildDir}\Qt5PrintSupport.dll"
;File "${WinBuildDir}\Qt5Svg.dll"
;File "${WinBuildDir}\Qt5Widgets.dll"
;File "${WinBuildDir}\Qt5Xml.dll"

; ;New Stuff - Qt Plugins
; CreateDirectory "$INSTDIR\bearer"
; SetOutPath "$INSTDIR\bearer"
; File "${WinBuildDir}\bearer\qgenericbearer.dll"
; File "${WinBuildDir}\bearer\qgenericbearer.dll"
; CreateDirectory "$INSTDIR\iconengines"
; SetOutPath "$INSTDIR\iconengines"
; File "${WinBuildDir}\iconengines\qsvgicon.dll"
; CreateDirectory "$INSTDIR\imageformats"
; SetOutPath "$INSTDIR\imageformats"
; ;File "${WinBuildDir}\imageformats\qdds.dll"
; File "${WinBuildDir}\imageformats\qgif.dll"
; File "${WinBuildDir}\imageformats\qicns.dll"
; File "${WinBuildDir}\imageformats\qico.dll"
; File "${WinBuildDir}\imageformats\qjpeg.dll"
; File "${WinBuildDir}\imageformats\qsvg.dll"
; File "${WinBuildDir}\imageformats\qtga.dll"
; File "${WinBuildDir}\imageformats\qtiff.dll"
; File "${WinBuildDir}\imageformats\qwbmp.dll"
; File "${WinBuildDir}\imageformats\qwebp.dll"
; CreateDirectory "$INSTDIR\printsupport"
; SetOutPath "$INSTDIR\printsupport"
; File "${WinBuildDir}\printsupport\windowsprintersupport.dll"
; CreateDirectory "$INSTDIR\platforms"
; SetOutPath "$INSTDIR\platforms"
; File "${WinBuildDir}\platforms\qwindows.dll"
;SetOutPath "$INSTDIR\styles"
;File "${WinBuildDir}\styles\qwindowsvistastyle.dll"

;File "${WinBuildDir}\D3Dcompiler_47.dll"
;File "${WinBuildDir}\libEGL.dll"
;File "${WinBuildDir}\libGLESV2.dll"
;File "${WinBuildDir}\opengl32sw.dll"

; Renderers-config:
; ;3rd party renderer utility - LdgLite
; CreateDirectory "$INSTDIR\3rdParty\${LDGliteDir}\bin"
; SetOutPath "$INSTDIR\3rdParty\${LDGliteDir}\bin"
; File "${WinBuildDir}\3rdParty\${LDGliteDir}\bin\${LDGliteExe}.exe"

; ;3rd party renderer utility - LDView
; CreateDirectory "$INSTDIR\3rdParty\${LDViewDir}\bin"
; SetOutPath "$INSTDIR\3rdParty\${LDViewDir}\bin"
; StrCmp ${ArchExt} "x64" 0 +2
; File "${WinBuildDir}\3rdParty\${LDViewDir}\bin\${LDViewExe}64.exe"
; ; File "${WinBuildDir}\3rdParty\${LDViewDir}\bin\${LDViewExe}.exe"
; CreateDirectory "$INSTDIR\3rdParty\${LDViewDir}\resources\config"
; SetOutPath "$INSTDIR\3rdParty\${LDViewDir}\resources\config"
; File "${WinBuildDir}\3rdParty\${LDViewDir}\resources\config\ldview.ini"
; File "${WinBuildDir}\3rdParty\${LDViewDir}\resources\config\ldviewPOV.ini"
; File "${WinBuildDir}\3rdParty\${LDViewDir}\resources\config\LDViewCustomini"

; ;3rd party renderer utility - LPub3D_Trace
; CreateDirectory "$INSTDIR\3rdParty\${LPub3D_TraceDir}\bin"
; SetOutPath "$INSTDIR\3rdParty\${LPub3D_TraceDir}\bin"
; StrCmp ${ArchExt} "x64" 0 +2
; File "${WinBuildDir}\3rdParty\${LPub3D_TraceDir}\bin\${LPub3D_TraceExe}64.exe"
; ; File "${WinBuildDir}\3rdParty\${LPub3D_TraceDir}\bin\${LPub3D_TraceExe}32.exe"
; CreateDirectory "$INSTDIR\3rdParty\${LPub3D_TraceDir}\resources\config"
; SetOutPath "$INSTDIR\3rdParty\${LPub3D_TraceDir}\resources\config"
; File "${WinBuildDir}\3rdParty\${LPub3D_TraceDir}\resources\config\povray.conf"
; File "${WinBuildDir}\3rdParty\${LPub3D_TraceDir}\resources\config\povray.ini"
; Renderers-32bit-exe:
; ;3rd party renderer utility - LdgLite
; File "${WinBuildDir}\3rdParty\${LDGliteDir}\bin\${LDGliteExe}.exe"
; ;3rd party renderer utility - LDView
; File "${WinBuildDir}\3rdParty\${LDViewDir}\bin\${LDViewExe}.exe"
; ;3rd party renderer utility - LPub3D_Trace
; File "${WinBuildDir}\3rdParty\${LPub3D_TraceDir}\bin\${LPub3D_TraceExe}32.exe"

DocumentsAndExtras:
SetOutPath "$INSTDIR"
File "${WinBuildDir}\docs\README.txt"
File "${WinBuildDir}\docs\RELEASE_NOTES.html"

;data depository folder
CreateDirectory "$INSTDIR\data"
SetOutPath "$INSTDIR\data"
File "${WinBuildDir}\extras\excludedParts.lst"
File "${WinBuildDir}\extras\fadeStepColorParts.lst"
File "${WinBuildDir}\extras\freeformAnnotations.lst"
File "${WinBuildDir}\extras\LDConfig.ldr"
File "${WinBuildDir}\extras\ldglite.ini"
File "${WinBuildDir}\extras\ldvExport.ini"
File "${WinBuildDir}\extras\PDFPrint.jpg"
File "${WinBuildDir}\extras\pli.mpd"
File "${WinBuildDir}\extras\pliSubstituteParts.lst"
File "${WinBuildDir}\extras\titleAnnotations.lst"
File "${WinBuildDir}\extras\ldvMessages.ini"

;ldraw libraries - data/portable location
File "${WinBuildDir}\extras\complete.zip"
File "${WinBuildDir}\extras\lpub3dldrawunf.zip"
File "${WinBuildDir}\extras\tenteparts.zip"
File "${WinBuildDir}\extras\vexiqparts.zip"

;documents
CreateDirectory "$INSTDIR\docs"
SetOutPath "$INSTDIR\docs"
File "${WinBuildDir}\docs\COPYING.txt"
File "${WinBuildDir}\docs\CREDITS.txt"
File "${WinBuildDir}\docs\README.txt"
File "${WinBuildDir}\docs\RELEASE_NOTES.html"
SetOutPath "$INSTDIR"

;3rd party renderer utility - LdgLite
CreateDirectory "$INSTDIR\3rdParty\${LDGliteDir}\doc"
SetOutPath "$INSTDIR\3rdParty\${LDGliteDir}\doc"
File "${WinBuildDir}\3rdParty\${LDGliteDir}\doc\LICENCE"
File "${WinBuildDir}\3rdParty\${LDGliteDir}\doc\README.TXT"

;3rd party renderer utility - LDView
CreateDirectory "$INSTDIR\3rdParty\${LDViewDir}\doc"
SetOutPath "$INSTDIR\3rdParty\${LDViewDir}\doc"
File "${WinBuildDir}\3rdParty\${LDViewDir}\doc\ChangeHistory.html"
File "${WinBuildDir}\3rdParty\${LDViewDir}\doc\Help.html"
File "${WinBuildDir}\3rdParty\${LDViewDir}\doc\License.txt"
File "${WinBuildDir}\3rdParty\${LDViewDir}\doc\Readme.txt"
SetOutPath "$INSTDIR\3rdParty\${LDViewDir}\resources"
File "${WinBuildDir}\3rdParty\${LDViewDir}\resources\8464.mpd"
File "${WinBuildDir}\3rdParty\${LDViewDir}\resources\LGEO.xml"
File "${WinBuildDir}\3rdParty\${LDViewDir}\resources\m6459.ldr"
;3rd party renderer utility - LPub3D_Trace
CreateDirectory "$INSTDIR\3rdParty\${LPub3D_TraceDir}\doc"
SetOutPath "$INSTDIR\3rdParty\${LPub3D_TraceDir}\doc"
File "${WinBuildDir}\3rdParty\${LPub3D_TraceDir}\doc\AUTHORS.txt"
File "${WinBuildDir}\3rdParty\${LPub3D_TraceDir}\doc\ChangeLog.txt"
File "${WinBuildDir}\3rdParty\${LPub3D_TraceDir}\doc\CUI_README.txt"
File "${WinBuildDir}\3rdParty\${LPub3D_TraceDir}\doc\LICENSE.txt"
CreateDirectory "$INSTDIR\3rdParty\${LPub3D_TraceDir}\resources\include"
SetOutPath "$INSTDIR\3rdParty\${LPub3D_TraceDir}\resources\include"
File "${WinBuildDir}\3rdParty\${LPub3D_TraceDir}\resources\include\arrays.inc"
File "${WinBuildDir}\3rdParty\${LPub3D_TraceDir}\resources\include\ash.map"
File "${WinBuildDir}\3rdParty\${LPub3D_TraceDir}\resources\include\benediti.map"
File "${WinBuildDir}\3rdParty\${LPub3D_TraceDir}\resources\include\bubinga.map"
File "${WinBuildDir}\3rdParty\${LPub3D_TraceDir}\resources\include\bumpmap_.png"
File "${WinBuildDir}\3rdParty\${LPub3D_TraceDir}\resources\include\cedar.map"
File "${WinBuildDir}\3rdParty\${LPub3D_TraceDir}\resources\include\chars.inc"
File "${WinBuildDir}\3rdParty\${LPub3D_TraceDir}\resources\include\colors.inc"
File "${WinBuildDir}\3rdParty\${LPub3D_TraceDir}\resources\include\colors_ral.inc"
File "${WinBuildDir}\3rdParty\${LPub3D_TraceDir}\resources\include\consts.inc"
File "${WinBuildDir}\3rdParty\${LPub3D_TraceDir}\resources\include\crystal.ttf"
File "${WinBuildDir}\3rdParty\${LPub3D_TraceDir}\resources\include\cyrvetic.ttf"
File "${WinBuildDir}\3rdParty\${LPub3D_TraceDir}\resources\include\debug.inc"
File "${WinBuildDir}\3rdParty\${LPub3D_TraceDir}\resources\include\finish.inc"
File "${WinBuildDir}\3rdParty\${LPub3D_TraceDir}\resources\include\fract003.png"
File "${WinBuildDir}\3rdParty\${LPub3D_TraceDir}\resources\include\functions.inc"
File "${WinBuildDir}\3rdParty\${LPub3D_TraceDir}\resources\include\glass.inc"
File "${WinBuildDir}\3rdParty\${LPub3D_TraceDir}\resources\include\glass_old.inc"
File "${WinBuildDir}\3rdParty\${LPub3D_TraceDir}\resources\include\golds.inc"
File "${WinBuildDir}\3rdParty\${LPub3D_TraceDir}\resources\include\ior.inc"
File "${WinBuildDir}\3rdParty\${LPub3D_TraceDir}\resources\include\logo.inc"
File "${WinBuildDir}\3rdParty\${LPub3D_TraceDir}\resources\include\makegrass.inc"
File "${WinBuildDir}\3rdParty\${LPub3D_TraceDir}\resources\include\marbteal.map"
File "${WinBuildDir}\3rdParty\${LPub3D_TraceDir}\resources\include\math.inc"
File "${WinBuildDir}\3rdParty\${LPub3D_TraceDir}\resources\include\meshmaker.inc"
File "${WinBuildDir}\3rdParty\${LPub3D_TraceDir}\resources\include\metals.inc"
File "${WinBuildDir}\3rdParty\${LPub3D_TraceDir}\resources\include\Mount1.png"
File "${WinBuildDir}\3rdParty\${LPub3D_TraceDir}\resources\include\Mount2.png"
File "${WinBuildDir}\3rdParty\${LPub3D_TraceDir}\resources\include\mtmand.pot"
File "${WinBuildDir}\3rdParty\${LPub3D_TraceDir}\resources\include\mtmandj.png"
File "${WinBuildDir}\3rdParty\${LPub3D_TraceDir}\resources\include\orngwood.map"
File "${WinBuildDir}\3rdParty\${LPub3D_TraceDir}\resources\include\pinkmarb.map"
File "${WinBuildDir}\3rdParty\${LPub3D_TraceDir}\resources\include\plasma2.png"
File "${WinBuildDir}\3rdParty\${LPub3D_TraceDir}\resources\include\plasma3.png"
File "${WinBuildDir}\3rdParty\${LPub3D_TraceDir}\resources\include\povlogo.ttf"
File "${WinBuildDir}\3rdParty\${LPub3D_TraceDir}\resources\include\povmap.png"
File "${WinBuildDir}\3rdParty\${LPub3D_TraceDir}\resources\include\rad_def.inc"
File "${WinBuildDir}\3rdParty\${LPub3D_TraceDir}\resources\include\rand.inc"
File "${WinBuildDir}\3rdParty\${LPub3D_TraceDir}\resources\include\rdgranit.map"
File "${WinBuildDir}\3rdParty\${LPub3D_TraceDir}\resources\include\screen.inc"
File "${WinBuildDir}\3rdParty\${LPub3D_TraceDir}\resources\include\shapes.inc"
File "${WinBuildDir}\3rdParty\${LPub3D_TraceDir}\resources\include\shapes2.inc"
File "${WinBuildDir}\3rdParty\${LPub3D_TraceDir}\resources\include\shapes3.inc"
File "${WinBuildDir}\3rdParty\${LPub3D_TraceDir}\resources\include\shapesq.inc"
File "${WinBuildDir}\3rdParty\${LPub3D_TraceDir}\resources\include\shapes_old.inc"
File "${WinBuildDir}\3rdParty\${LPub3D_TraceDir}\resources\include\skies.inc"
File "${WinBuildDir}\3rdParty\${LPub3D_TraceDir}\resources\include\spiral.df3"
File "${WinBuildDir}\3rdParty\${LPub3D_TraceDir}\resources\include\stage1.inc"
File "${WinBuildDir}\3rdParty\${LPub3D_TraceDir}\resources\include\stars.inc"
File "${WinBuildDir}\3rdParty\${LPub3D_TraceDir}\resources\include\stdcam.inc"
File "${WinBuildDir}\3rdParty\${LPub3D_TraceDir}\resources\include\stdinc.inc"
File "${WinBuildDir}\3rdParty\${LPub3D_TraceDir}\resources\include\stoneold.inc"
File "${WinBuildDir}\3rdParty\${LPub3D_TraceDir}\resources\include\stones.inc"
File "${WinBuildDir}\3rdParty\${LPub3D_TraceDir}\resources\include\stones1.inc"
File "${WinBuildDir}\3rdParty\${LPub3D_TraceDir}\resources\include\stones2.inc"
File "${WinBuildDir}\3rdParty\${LPub3D_TraceDir}\resources\include\strings.inc"
File "${WinBuildDir}\3rdParty\${LPub3D_TraceDir}\resources\include\sunpos.inc"
File "${WinBuildDir}\3rdParty\${LPub3D_TraceDir}\resources\include\teak.map"
File "${WinBuildDir}\3rdParty\${LPub3D_TraceDir}\resources\include\test.png"
File "${WinBuildDir}\3rdParty\${LPub3D_TraceDir}\resources\include\textures.inc"
File "${WinBuildDir}\3rdParty\${LPub3D_TraceDir}\resources\include\timrom.ttf"
File "${WinBuildDir}\3rdParty\${LPub3D_TraceDir}\resources\include\transforms.inc"
File "${WinBuildDir}\3rdParty\${LPub3D_TraceDir}\resources\include\ttffonts.cat"
File "${WinBuildDir}\3rdParty\${LPub3D_TraceDir}\resources\include\whiteash.map"
File "${WinBuildDir}\3rdParty\${LPub3D_TraceDir}\resources\include\woodmaps.inc"
File "${WinBuildDir}\3rdParty\${LPub3D_TraceDir}\resources\include\woods.inc"
CreateDirectory "$INSTDIR\3rdParty\${LPub3D_TraceDir}\resources\ini"
SetOutPath "$INSTDIR\3rdParty\${LPub3D_TraceDir}\resources\ini"
File "${WinBuildDir}\3rdParty\${LPub3D_TraceDir}\resources\ini\allanim.ini"
File "${WinBuildDir}\3rdParty\${LPub3D_TraceDir}\resources\ini\allstill.ini"
File "${WinBuildDir}\3rdParty\${LPub3D_TraceDir}\resources\ini\low.ini"
File "${WinBuildDir}\3rdParty\${LPub3D_TraceDir}\resources\ini\pngflc.ini"
File "${WinBuildDir}\3rdParty\${LPub3D_TraceDir}\resources\ini\pngfli.ini"
File "${WinBuildDir}\3rdParty\${LPub3D_TraceDir}\resources\ini\povray.ini"
File "${WinBuildDir}\3rdParty\${LPub3D_TraceDir}\resources\ini\quickres.ini"
File "${WinBuildDir}\3rdParty\${LPub3D_TraceDir}\resources\ini\res120.ini"
File "${WinBuildDir}\3rdParty\${LPub3D_TraceDir}\resources\ini\res1k.ini"
File "${WinBuildDir}\3rdParty\${LPub3D_TraceDir}\resources\ini\res320.ini"
File "${WinBuildDir}\3rdParty\${LPub3D_TraceDir}\resources\ini\res640.ini"
File "${WinBuildDir}\3rdParty\${LPub3D_TraceDir}\resources\ini\res800.ini"
File "${WinBuildDir}\3rdParty\${LPub3D_TraceDir}\resources\ini\slow.ini"
File "${WinBuildDir}\3rdParty\${LPub3D_TraceDir}\resources\ini\tgaflc.ini"
File "${WinBuildDir}\3rdParty\${LPub3D_TraceDir}\resources\ini\tgafli.ini"
File "${WinBuildDir}\3rdParty\${LPub3D_TraceDir}\resources\ini\zipflc.ini"
File "${WinBuildDir}\3rdParty\${LPub3D_TraceDir}\resources\ini\zipfli.ini"
SetOutPath "$INSTDIR"
