LPub3D 2.0.0.
 
Features and enhancements 
------------  
-Increase PLI rendering speed with LDView using -SaveSnapshots=1 (r683)
-Add context menu to pages without steps - e.g. Cover Page, BOM Page (r681)
-Add missing context menu icons (r680)
-Add progress message updates to splash screen (r676)
-Supress rotateIcon context menu item if icon already inserted for step (r670)
-On single-step page, place PLI (Parts List) on top of CSI (Model) images (r664)
-Upon reset image and model caches, reload current file, return to first page (r663)
-Unique splash 3D model displayed during pdf printing, png, jpg and bmp image export (r657)
-Multi-step callouts are now movable (r656)
-ldglite update to 1.3.1 executable with -2g,2x option hardcoded for sharper images with offscreen rendering (r655)
-Force to empty the cache before creating a PDF (r654)
-Synchronize 'prev', 'next' and 'goto' page menu items (r653)
-Supress rotateIcon from CSI (model) item context menu if callout and callout is not unassembled (r652)
-Select local page size and global and local page orientation (Portrait and Landscape)
 Note: When manually editing the model file with either page size or orientation, it is recommended to insert both the size and orientation metas together. For example, even
 if you are only interested to add a page size, you shold update your file like the example 
 below - obviously selecting your own size values and orientation.
 0 !LPUB PAGE SIZE GLOBAL 8.2677 11.6929
 0 !LPUB PAGE ORIENTATION GLOBAL PORTRAIT (r518)
-Change: Update page background menu rearranged - see r641. (r642)
-Change: Page background context menu rearranged. "Change page background" and "Change Page Size or Orientation" now appear at the end of the menu list because they are likely to be least often used. (r641)
-Change: Point online manual to LPub3D content (r517)
-Change: Replace #if (_MSC_VER >= 1400) with #ifdef _MSC_VER (r679)
-Rafactor: Colour application output default set to off (r662)
-Refactor: Define strcasecmp, strncasecmp in lc_global.h for MinGW builds only (r661)
-Refactor: Replace 0x050000 with QT_VERSION_CHECK(5,0,0) (r660)
-Refactor: Remove stretch/tile logic from coverImage management (r659)
-Refactor: Remove QPainter::HighQualityAntialiasing obsolete renderHint (r658)
-Refactor: Update CSI image mousePressEvent logic. (r640)
-Refactor: Update file load progress messages (r636)
-Refactor: Rearrange printToPdfFile page count (r632)
-Refactor: Reposition rocessEvents statements (r631)
-Rafactor: Supress MSVC type conversion warning C4267 (r621)
-Update: Change copyright to 2016 (r666)
-Update: AboutDialog to recognize Win 10 and OSX El Capitan(r665/r668/r669)
-Update: application name value to support development environment (r639)
-Update: build assets (r624)
-Update: ldglite hard-coded default directory setting to ldglite1.3.1(r623)
-Update: zlib reference for Qt 5.6.0 (r618)
-Update: minifig settings (move to ini file) - LeoCAD Changeset 1870 (r617)
-Fix: Remove libpng warning: iCCP: known incorrect sRGB profile (r678)
-Fix: Convert special characters (copyright and trademark) from Wide Char to UTF8 for MSVC build (r677)
-Fix: Remove final colour model if exist when Fade Step is not enabled (r674)
-Fix: Clear cache when turning fade on/off (r674)
-Fix: fadeStep and preferCentimeter setting mixmatch (r674)
-Fix: Search directories not updated when directory added to Ldraw.ini (r673)
-Fix: When background is transparent context menu functionality is lost (fix is a hack which sets the bg-color to white with an alpha of 1.) (r672)
-Fix: When a CALLOUT allocation is changed, if you right-click a model in the callout and not the callout itself, the ALLOC meta is placed after the CALLOUT END and has no effect - meta appended but should be inserted (r650) 
-Fix: Converting an assembly to a part results in a parse error when there are spaces in the file name (r649)
-Fix: If a divided STEP GROUP allocation is changed from vertical (Display as columns - default) to horizontal (Display as rows), selecting again Display as columns has no effect - meta appended but should be replaced (r648)
-Fix: When the "Redraw" icon is clicked on the LDraw File Editor window, the file editor resizes to 255x190 pixels (r647)
-Fix: Split BOM duplicates one part onto current and subsequent BOM pages (r646)
-Fix: Inserting a front cover page when the current first page is a multi step page (r645)
-Fix: Bug when using callouts in Multistep sequences. When you place your callout right from assembly, it appears on the left side. When you place your callout bottom, it appears on the top. (r643)
-Fix: wrong text when you export to PNG the window title says "Print to pdf" (r638)
-Fix: When publishing instructions with the option 0 !LPUB PAGE BACKGROUND TRANSPARENT a drop shadow layer was added (r637) 
-Fix: Remove -w1 from default ldglite parms (r629)
-Fix: Periodic multi-step crash - 3DViewer image file line number mismatch (r628)
-Fix: Control manual page number entry (r627) 
-Fix: Cleanup 'Copyright' and 'Trademark' unicode (utf8) chars on MSVC2015 build (r622)
-Fix: QPainter alphaChannel management - use setCompositionMode (r619/r620)
-Upgrade: to Qt 5.6.0 Stable (debug build KO), Build and link own copy of zlib. (r616)
-Upgrade: base to Qt 5.6.0 (MSVC 2015) (r613)
-Upgrade: logging framework (r610)
-Upgrade: base to Qt 5.5.1 (MinGW) platform (r608)

LPub3D 1.3.5.615.2 
 
Features and enhancements 
------------ 
-Fix: Pdf output restored to vector graphics - was being formatted as bitmap because conflicting setting was exposed during printing. (r615) 
 
LPub3D 1.3.4.591.2 
 
Features and enhancements 
------------ 
-Fix: During installer installation, prompt user to overwrite existing configuration files (r591) 
 
LPub3D 1.3.3.590.2 
 
Features and enhancements 
------------ 
-Fix: Crash when pieceinfo is null - this occurs when a file has no FILE meta and is imported as an ldr (versus an mpd).(r575) 
-Fix: Periodic crash when changing margin of assembly (CSI) in multi-step page (r577) 
-Fix: Periodic crash when adding divider to multi-step page or callouts (r577) 
-Fix: Periodic crash every time a second successive rotation icon is added to multi-step page (r577) 
-Fix: If using LPub3D archive distribution (no installer), use distribution's extras folder instead of creating one in AppData (r579) 
-Fix: Print/export dialog progress bar (r585) 
-Fix: Upon "Add assembled image to parent page" a rotation icon is added to the callout if rotation icons were present in the callout step(s). Assembled and rotated callouts will not display rotate icons on the parent page. Only unassembled callouts will display rotate icons if present in the callout step(s). (r587) 
-Fix: 2 page refreshes when Parameters menu item is accpeted - only a single refresh needed. (r588) 
 
LPub3D 1.3.2.563.5 
 
Features and enhancements 
------------ 
-Fix: PLI Parts annotation restored to short value (r555) 
-Fix: Control manual page number entry. (r562) 
-Fix: Remove silent_alloc which would trap the Callout meta LPUB CALLOUT HORIZONTAL/VERTICAL and throw a parse error. 
 However silent_alloc was not fully implemented and does not serve any current purpose. 
 The correct meta to allocate a Callout Horizontally or Vertically is LPUB CALLOUT ALLOC HORIZONTAL/VERTICAL (r563)
 
LPub3D 1.3.1.516.3 
 
Features and enhancements 
------------ 
-Fix: Update ldglite hard-coded default directory setting (r505) 
-Fix: Remove hard coded reference to LPub3DViewer-Library folder - this was added in 1.3.0 unofficial archive refresh functionality (r506) 
-Enhancements (r507) 
-Fix: Add pliSubstituteParts.lst to media build - sorry, I forgot to include in the NSIS build script:-( (r515) 
 
LPub3D 1.3.0.504.2 
 
Features and enhancements 
------------ 
-Add dialog to print or export to image all pages, current page and custom range of pages (e.g. 1,3,5,7-9) 
-Add ability to detect additional ldraw content search directories using ldrawini c api 
 I implemented the ldrawini c api to maintain compatability with LPub3D 3rd Party renderers - LDView, L3P(PovRay) and Ldglite. Additional directories must have either the same tree as LDraw Unofficial directory (i.e. parts and/or p subdirectories...) or alternatively, content can be deposited at the root of the additional directory (i.e. C:/ldrawFiles/*.dat). Content from all defined ldraw search directories are archived in the ldrawunf.zip archive and loaded into memory - enabling access to the 3DViewer. 
 If no ldraw.ini file is detected. LPub3D will automatically search all subdirectories under the ldraw/Unofficial directory - except directories p and parts. Unofficial subdirectories p and parts as well as official p and parts subdirectories are never searched because they are atomatically loaded during default behavior during startup. 
 There are 2 ways to define search directories: 1. using the Ldraw.ini file (there is now a menu button to edit the ldraw.ini file) and 2. editing the 'Ldraw Content Search Directories text edit dialog under the 'Other' tab in Preferences. If you are using an LDraw.ini file, the preferences dialog will be read only - you must use the menu edit button under 'Configurtion' to edit the ldraw.ini file. The ldraw.ini edit button only appears if a ldraw.ini file is detectec. If you are not using ldraw.ini, it is possible to add,remove and reset to the default search directories in the Preferences tab. 
 ldglite renderer updated with the ability to process additional directories beyond official/unofficial parts, p and Model. I implemented this 
 enhancement to allow all 3 LPub3D renderers(LDView, Ldglite, L3P/PoV Ray) the same functionality supporting additional ldraw content search directories. LPub3D now passes 2 env variables to ldglite - LDRAWDIR and LDSEARCHDIRS. LdView and L3P already uses the ldrawini.c library. They can also be configured to detect additional ldraw content search directories if no ldraw.ini file is configured. I also upgraded ldglite's openGL API from glut (deprecated) to freeglut 3.0 released in June 2015. Ldglite os not versioned at 1.3.0 (from 1.2.7). 
-Add ability to manage additional ldraw content search directories whether using Ldraw.ini or not. 
 If not using Ldraw.ini file, automatically detected search directories are limited to those under the Unofficial directory. The user has the abilty to exclude and reset search directories within this edit list. Unofficial P and Parts directories are automatically excluded from the search directory list as they are loaded automatically by the applciation. 
-Add Rotate Icon - indicate that the builder should "flip" the model 
-Add line type attribute to border configuration 
 0=none,1=solid,2=dash,3=dot,4=dashDot,5=dashDotDot. So an old meta line like this: 
 0 !LPUB PLI BORDER GLOBAL ROUND Black 0 15 MARGINS 0.472439 0.07 should be updated to this: 
 0 !LPUB PLI BORDER GLOBAL ROUND 1 Black 0 15 MARGINS 0.472439 0.07 
-Add PLI/BOM part substitution - substitute modeled part in PLI/BOM with alternate 
 This feature is useful when you have a modeled part (e.g. Power Functions Light) that will 
 take alot of space in the BOM/PLI, you can substitute the modeled version with an alternate 
 representation that is suitable for the PLI/BOM view. This feature is complementary to LDCad's 
 template functionality which allows you to model adjustable parts as needed. To use this 
 functionality, simply edit the substituation list from Configuration= BOM/PLI Substitute Parts List 
-Add LDCad PART/UNOFFICIAL PART meta to recognize LDCad template generated parts 
-Add gradients to background options 
-Add Change local page background - colour, gradient, picture, submodel, none 
-Add memu item 'Refresh Unofficial Parts' which downloads and replaces the ldrawunf.zip archive 
 in the  Ldraw/LPub3DVoiewer-Library. LPub3D places all search directory parts in the ldrawunf.zip archive so they 
 can be made available for the LPub3D Viewer. This feature allows you to reset the archive file to 
 it's latest default content if desired. The ldrawunf.zip unofficial archive is used, along with the complete.zip, 
 by the 3DViewer. 
-Add menu item 'Reset Fade Files Cache' which deletes all generated fade files. Fade files automatically 
 generated as needed when you open a model with the 'Enable Fade Steps' preference option checked. 
-Update 3DViewer to LeoCAD build 1867 
-Remove PartsList class, use instead lcPiecesLibrary class to verify parts and capture part title. This is a consolidation to 
 improve the application's performance. 
-Move process fade colour parts to separate thread. 
-Move fade parts directory from under LDraw/Unofficial/parts and LDraw/Unofficial/p to as single directory 
 under LDraw/Unofficial. So from this version of LPub3D, the fade directory will be autogenerated and populated 
 as LDraw/Unofficial/fade. Old fade directories under Unofficial/parts and p must be manually removed if desired. 
-Redesign color part list and archive classes - rename and separate generage color parts list 
-Cosmetic and performance updates 
-Fix: Changing step number attributes on multi-step pages are now working 
-Fix: PLI/BOM sort routine, sort on 'part colour' part(s) appear out of place relative to 
 part size and colour. 
-Fix: Do not create instruction page(s) for unofficial part 
-Fix: Create s/8/48 subdirectory in lpub/tmp directory when needed. This fix will resolve the issue of LPub3D not being able to create inline unofficial subparts and 8/48 primitives when rendering models where these parts are defined in the model file. 
-CHANGE: In previous versions of LPub3D, custom and fade parts were loaded under the Unofficial directory allowing detection by all renderers and the 3D viewer. From this version of LPub3D (v1.3.0), the fade directory will reside at the root of the the Unofficial directory. Custom content added to Unofficial P and Parts directories will not detected by LPub3D. 
-CHANGE: Change part count routine to use ldraw archive files to look at '!LDRAW ORG...' part type meta tag. I think there are still some issue with 
 this routine however - especially with large complex models using in-lined unofficial parts that may not be in ldrawunf.zip 
-CHANGE: Change 'Reset All Caches' to 'Reset Image and Model' Caches 
-CHANGE: BOM default sort to sort by colour then size, previous default was size only (PLI default sort by size) 
-CHANGE: Change 'Update' tab in preferences dialog to 'Other'. 
 
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
