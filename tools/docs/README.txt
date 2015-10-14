LPub3D 1.3 XXXX
 
Features and enhancements 
------------ 
-Add Rotate Icon - indicate that the builder should "flip" the model
-Add line settings to border configuration
-Add gradients to background options
-Add LDCad UNOFFICIAL PART meta
-Change local page background - colour, gradient, picture, submodel, none
-Update 3DViewer to LeoCAD build 1867
-Generate warning message when LPub 0 !LPUB INSERT COVER_PAGE encountered. Message: 
 LPub3D does not support the LPub meta 0 !LPUB INSERT COVER_PAGE.
 Please update to 0 !LPUB INSERT COVER_PAGE FRONT (or BACK) accordingly.
-Remove final model if fadestep not enabled KO
-Cosmetic updates
-Fix: Changing step number attributes on multi-step pages are now working
-Fix: PLI/BOM sort routine, sort on 'part colour' part(s) appear out of place relative to
 part size and colour.
-Fix: Do not create instruction page(s) for unofficial part
-CHANGE: BOM default sort to sort by colour then size, previous default was size only (PLI default sort by size)

LPub3D 1.2.3.443.2 
Features and enhancements 
------------ 
-Fix Insert Text meta command not recognizing \n (new line) characters. 
 
LPub3D 1.2.2.442.3 
Features and enhancements 
------------ 
-Fix: Update fade parts "failed to archive..." error messages 
-Fix: Change fade step color from "Fade Step Setup" menu writes the correct meta command 
 but color does not change when steps generated. 
 
LPub3D 1.2.1.437.2 
 
Features and enhancements 
------------ 
-Fix: PLI/BOM sort routine, sort on 'part colour' part(s) appear out of place relative to 
 part size and colour. 
-CHANGE: BOM default sort to sort by colour then size, previous default was size only (PLI default sort by size) 
 
LPub3D 1.2.0.388.4 
 
Features and enhancements 
------------ 
-Sort PLI/BOM by part size (default), colour and category 
-Improved model navigation - display selected model from model drop-down. Ignored 'meta=IGN' 
 submodels are not displayed even though they are in the drop-down list because they are 
 not assigned a page number. 
-Display LDraw colour code on BOM/PLI tooltip - now showing colour description, colour code and 
 part description. 
-Go To Page navigation combo box - allow page navigation using drop-down box. 
-Show progress bar during display page processing. 
-Text entries can now accomodate quotes using escape character '\' - e.g. This \"Fat\" MOC 
 will display: This "Fat" MOC. Edits must always include the '\' escape character. 
-Fix: When changing the font on added text, the dialog starts with MS Shell Dlg 2 size 12. 
 The dialog now starts with display of the actual font perameters being changed. 
-Fix: Change to PLI Times Used attribue on multi-step page now affects only the selected step. 
 when use local option selected. 
-Fix: Change to PLI Text Annotation attribue on multi-step page now affects only the selected step. 
 when use local option selected. 
-Fix: Change to PLI Background attribue on multi-step page now affects only the selected step. 
 when use local option selected. 
-Fix: Change to PLI Border attribue on multi-step page now affects only the selected step. 
 when use local option selected. 
-Fix: Change to Step number attribue on multi-step page now affects only the selected step. PARTIAL - Use 
 local selection still not working. 
 
-CHANGE: Move PLI annotations settings from application preferences to BOM/PLI Setup preferences. 
-CHANGE: LPub3D distribution no longer packed due to reported Win32 incompatability issues. 
 unpacked executable and dependencies are now deposited in installation folder 
 for both x32 and x64 distributions. 
 Windows Content Structure:
	<Install Path>LPub3D/											<installed>
					3rdParty/										<installed>
						l3p1.4WinB/									<installed>
							L3P.EXE									<installed>
						ldglite1.2.6Win/							<installed>
							plugins/								<installed>
								pluginldlist.dll					<installed>
							ldglite.exe								<installed>
							LICENCE									<installed>
							README.TXT								<installed>
					docs/											<installed>
						COPYING.txt									<installed>
						CREDITS.txt									<installed>
					libeay32.dll									<installed>
					libgcc_s_sjlj-1.dll								<installed>
					libstdc++-6.dll									<installed>
					libwinpthread-1.dll								<installed>
					LPub3D_x<64/32>.exe								<installed>
					QtCore4.dll										<installed>
					QtGui4.dll										<installed>
					QtNetwork4.dll									<installed>
					QtOpenGL4.dll									<installed>
					quazip.dll										<installed>
					README.txt										<installed>
					ssleay32.dll									<installed>
					Uninstall.exe									<installed>
	<Drive>\Users\<User>\AppData\Local\LPub3D Software\LPub3D\		<installed/automatically generated>
					cache/											<automatically generated>
						*											<automatically generated>
					dump/											<automatically generated>
						minidump.dmp								<automatically generated>
					extras/											<installed>
						fadeStepColorParts.lst						<installed>
						freeformAnnotations.lst						<installed>
						PDFPrint.jpg								<installed>
						pli.mpd										<installed>
						printbanner.ldr								<installed>
						titleAnnotations.lst						<installed>
					logs/											<automatically generaged>
						LPub3DLog.txt								<automatically generaged>
  
LPub3D 1.1.0.370.2 
 
Features and enhancements 
------------ 
-Enable/disable PLI annotations (Preference Dialog) 
-Edit part title annotations 
-Fix: source URL on about dialog 
-Fix: .dat subFiles not processed for 3D viewer - causes abnormal end (expected 
 dat files to be treated as part and placed in Official/unofficial parts). 
 Dat subfiles are now treated same as mpd/ldr subfiles. 
 
-CHANGE: isSubmodel check no longer observes meta !LDRAW_ORG as Submodel=False 
 
-KNOWN ISSUE: When an image (e.g. logo) is placed bottom left or bottom right 
 right of the page header, dragging the image produces unexpected placement - the same 
 may be true with regards to placement relative to the page footer. 
 WORKAROUND: If you want to reposition by dragging, set the image relative to the Page 
 (any relative position) or bottom center of the header/ top center of the footer. 
 
LPub3D 1.0.0.320.2 
 
Features and enhancements 
------------ 
-Integrated 3D Viewer powered by LeoCAD 
-Set ROTSTEP (step rotation) meta using 3D Viewer 
-Fade previous step's parts 
-Use any LDraw colour as fade colour 
-Manage colour parts for complete part fade 
-Unlimited PLI part annotations 
-Split the BOM (divide parts over number of BOM occurrences) 
-Formatted front and back cover pages 
-Automatically generate formatted cover pages 
-POV/L3P high-quality rendered instructions 
-Customizable front and back cover pages 
-Text and picture (e.g. logo) front and back page attributes 
-Customizable content page header and footer 
-LDraw editor line numbers 
-64/32-bit automatic installer 
-Automatic check for updates 
-Qt 4.8.6 
-Bugfixes and robustness improvements 
 
About LPub3D 
------------ 
LPub3D is an editing application for creating LEGO style digital building instructions. 
It's available for free under the GNU Public License v3 and works on the Windows 
Operating Systems. LPub3D is based on LPub written by Kevin Clague. 
 
Installation 
------------ 
You can download the latest version of LPub3D from 
https://sourceforge.net/projects/lpub3d 
 
It's recommended that you install the latest drivers for your video card 
since LPub3D uses LeoCAD which uses OpenGL to take advantage of hardware 
acceleration for rendering. 
 
- LPub3D for Windows: 
Download LPub3D to your computer, double click on the 
icon to launch the installer and follow the intructions. 
 
Updates 
------------ 
You can set your preference to automatically check for updates. 
It is also possible to manually check for updates. 
 
Online Resources 
---------------- 
- Website: 
https://sourceforge.net/projects/lpub3d 
 
Legal Disclaimer 
---------------- 
LEGO is a trademark of the LEGO Group of companies which does not sponsor, 
authorize or endorse this software. 
