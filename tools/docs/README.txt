LPub3D 2.0.7.770.5 
 
Features and enhancements 
------------ 
-Fix: Elapsed timer on file open (r771) 
 *Display elapsed time to load a file. 
-Fix: Archive library copy function not working if [empty] library directoy exist (r770) 
 *If user data libraries directory exist, library copy from installed base is ignored. This is an issue if there are no libraries in the library directory. The correct behaviour is to verify that libraries exist and copy if they don't. 
-Digitally sign LPub3D executable distributions (r769) 
 *Secure installation content and reduce the likelihood of triggering antivirus quarantine. 
-Fix: Inconsistent fade behaviour when using BUFEXCHG parts and added parts in the same step (r764) 
 *Behaviour previously used the size of the previous step's CSI to determine the fade position index of the current step in all cases. This approach could lead to an inconsistent fade position after retrieving a buffer. Behaviour corrected to use the size of the previous buffer parts list (versus the CSI) to determine the current step's fade position when BUFEXCHG RETRIEVE meta command is used. This approach removes the necessity to follow the BUFEXCHG RETRIEVE meta command with a STEP/ROTSTEP meta command to process the fade sequence which will unnecessarily render the buffered items twice, in the buffered view and the modelled view. Usually only the buffered view render is desired in the current step (that's why the assembly is buffered in the first place) but the modelled view CSI should be carried forward to the next step. Here are two examples: 
 Example 1: No unbuffered parts in step 1, render buffered skeleton with arrow in step 1 but render only modelled skeleton faded and the current parts in step 2 
	0 !LPUB ASSEM MODEL_SCALE LOCAL  0.6500 
	0 BUFEXCHG A STORE 
	0 GHOST 1 0 201.2 -844.25 0 1 0 0 0 1 0 0 0 1 skeleton.ldr 
	0 BUFEXCHG B STORE 
	0 BUFEXCHG A RETRIEVE 
	0 !LPUB PART BEGIN IGN 
	0 GHOST 1 0 201.2 -960.25 0 1 0 0 0 1 0 0 0 1 skeleton.ldr 
	0 GHOST 1 4 -131.2 -216.25 -70 -1 0 0 0 1 0 0 0 -1 arrow88.dat 
	0 !LPUB PART  END 
	0 STEP 
	0 !LPUB PLI CONSTRAIN LOCAL HEIGHT 5.67 
	0 !LPUB PLI PLACEMENT LOCAL LEFT PAGE INSIDE 0.0115693 -0.118771 
	0 BUFEXCHG B RETRIEVE 
	1 0 -130 -232 -70 -1 0 0 0 1 0 0 0 -1 3069b.dat 
	1 0 -130 -232 70 -1 0 0 0 1 0 0 0 -1 3069b.dat 
	0 STEP 
 Example 2: Unbuffered (modelled) parts in step 1, render hobspine, crossbrace, and outerrib with arrow (buffered) in step 1 but exclude arrow and show faded, step 1 modelled parts plus current parts in step 2. Step 1 terminates with ROTSTEP 
	0 GHOST 1 0 0 0 0 1 0 0 0 1 0 0 0 1 hobspine.ldr 
	0 !LPUB CALLOUT BEGIN 
	0 !LPUB CALLOUT POINTER BOTTOM_LEFT 0.608 0.763 0 
	1 0 0 0 0 1 0 0 0 1 0 0 0 1 crossbrace.ldr 
	0 !LPUB CALLOUT PLACEMENT RIGHT ASSEM INSIDE 0.41159 0.062474 
	0 !LPUB CALLOUT END 
	0 BUFEXCHG A STORE 
	1 0 0 0 -40 1 0 0 0 1 0 0 0 1 outerrib.ldr 
	1 71 -70.196 804.976 -55 -0.924 -0.383 0 -0.383 0.924 0 0 0 -1 32123a.dat 
	1 71 -218.11 743.75 -55 -0.707 -0.707 0 -0.707 0.707 0 0 0 -1 32123a.dat 
	0 BUFEXCHG B STORE 
	0 BUFEXCHG A RETRIEVE 
	0 !LPUB PART BEGIN IGN 
	0 GHOST 1 0 0 0 -120 1 0 0 0 1 0 0 0 1 outerrib.ldr 
	0 GHOST 1 71 -70.196 804.976 -165 -0.924 -0.383 0 -0.383 0.924 0 0 0 -1 32123a.dat 
	0 GHOST 1 71 -218.11 743.75 -165 -0.707 -0.707 0 -0.707 0.707 0 0 0 -1 32123a.dat 
	0 GHOST 1 4 -70.196 804.976 -95 1.00023 0.000246369 0 0 0 -1 -0.000246369 1.00023 0 arrow88.dat 
	0 !LPUB PART END 
	0 ROTSTEP 0 75 0 ABS 
	0 BUFEXCHG B RETRIEVE 
	0 !LPUB PLI CONSTRAIN LOCAL HEIGHT 3.51333 
	1 0 -378.303 402.398 -50 0 1 0 -1 0 0 0 0 1 3460.dat 
	1 0 -378.303 402.398 50 0 1 0 -1 0 0 0 0 1 3460.dat 
	0 STEP 
-Fix: LDView single call render crash on multi-step page generation (763) 
 *Crash if multi-step page's steps contain more than PLI and CSI components. Corrected. 
-Fix: Preference dialog version change log cleared on update check when there is no available update (r762) 
 *If no update available, ignore updating the change log dialog. 
 
LPub3D 2.0.6.761.3 
 
Features and enhancements 
------------ 
-Fix: Set radial and conical background gradient parse fail with 'Malformed background...' (r759) 
  *Parse fail for radial and conical gradient meta commands. Linear, radial and conical gradients are all now working as designed. 
-Fix: Scale in Page Globals Setup dialog not working (r756) 
 *Cover Image and Logo double spin control not working. Issue corrected. 
-Fix: Save and restore application window state and geometry (r755) 
 *Was causing crash on launch before update to Qt5.7. 
-Fix: Application crash on launch (r754) 
 *Update to Qt5.7 on MSVC2015 and MinGW5.3. There must have been a big, nasty bug in Qt 5.5/5.6 because the code that consistently generated the crash immediately resolved upon update to Qt5.7 
-Fix: Crash on application close on Windows7 (r753) 
 *Expected scoped pointer main window to destruct all children on close but it seems like 3d viewer application and mainwindow were not treated as children and were not deleted at application close by the scoped pointer on Windows7 machines. Manually delete 3D viewer application instance and mainwindow at LPub3D termination. 
-Fix: Substitute parts use only file name; file path not required (r752) 
 *When editing substitute parts list, it is not necessary to enter the absolute file path for the substitute file. Just entering the substitute file name will be sufficient. 
-Fix: Title annotations displays when only Freeform annotation selected in setup preference (r751) 
 *Logic processed title annotations when it should not have. Corrected. 
-Fix: setGeometry: Unable to set geometry 600x800 warning message (r749) 
 *Use QDesktopWidget.availableGeometry(this) setting to support single and multi-screen configurations. 
-Fix: Parameter file edit window highlighting part description containing '#' (r748) 
 *Highlight only lines where first character is '#'. 
-Fix: Generate fade colour parts list crash (r747) 
 *Redesigned functionality to process parts from archive libraries (unofficial and official) versus LDraw disc directories. This approach improves performance and reliability as all parts, including those from additional search directories, are collected in the archive libraries. Working with archive files is much faster than working with disc directories. 
 
LPub3D 2.0.5.744.3 
 
Features and enhancements 
------------ 
-Fix: Portable distribution create and populate user data directory when installed under Program Files/(x86) (r744) 
-Fix: Revert to deposit LPub3D icons in Programs menu (versus startup menu) (r743) 
-Fix: Remove MAINWINDOW registry group if exist (r742) 
 *I believe the settings in this group are contributing to the intermittent crash at startup. 
-Fix: LDConfig load order; first, LDraw directory; second, extras directory; third, resource cache (r741) 
-Fix: Automatically load ldglite during installation/application launch (r740) 
 
LPub3D 2.0.4.737.2 
 
Features and enhancements 
------------ 
-Fix: Release Windows MinGW x32 builds (r737) 
 *Update deployment utiliites to produce both MinGW and MSVC builds. MinGW will only support x32 architecture for the momenbt. 
-Fix: Search directories for LDGlite (LDSEARCHDIRS) not loaded as expected at startup (r735) 
 *Loading the LDGLite search directories at startup occurred out of order (before general search directories) so the LDGlite routine did not pass the conditional test to actually load search directories. This behavior would cause a crash if parts to be loaded were in the standard LDraw official or unofficial directories - for example under ..\Unofficial\myParts - and LDrawini was not in use. LDGLite would not be able to find the part and; consequently, would not be able to generate a part image. 
-Fix: Fade steps skips the second step in a model. Fading starts on the third step. (r734) 
 *No fade parts index generated on the first step because nothing was faded; however, we still need an index to know where to start on the second step. Fade step routine fixed to generate an index as long as there are valid parts in the step. 
-Fix: Installer program configured to deposit a master copy of usder data (libraries, lists, etc...) in the installation root directory. (r733) 
 *Allow user data creation at initial launch. Upon initial application launch, if user data does not exist, it will be created. This will address the issue of Windows standard users not having access to user data after installation. Additioinally, this design allows for multiple users on a single machine to have their individual user settings and data. 
-Fix: Set progress dialog to nonmodal. (r732) 
 *Prevent the progress dialog from blocking input to other windows. 
-Fix: Data directory installed under Administrator AppData path instead of logged in user which is likely to be a standard user (r731) 
 *User data - LDraw archive libraries, logs, extras and other updatable data items - will be installed at initial application launch by default. Because Administrator privilates are required to install LPub3D, user data installed during installation will be deposited under the Administrator user's AppData path. This data will not be accessible to standard users. User data can be installed during application installation as a checked option. This option may be desirable if the logged in user installing LPub3D is also the Administrator. If user data is installed during installation, user data for standard users will be automatically created during initial application launch. On initial application launch, the standard user will be given the options to select, copy (from the installation directory) or download the LDraw archive libraries. (Known Issue: Standard user incons are not being generated at the moment. Also, The uninstall routine will not remove standard user data created at application startup. I'm still working on improving the deployment package to handle these items). 
 
LPub3D 2.0.3.730.2 
 
Features and enhancements 
------------ 
-Change: Check for update settings are now enabled by default. Previously, it was necessary to configure update settings in the Preferences/Other tab before one could visually confirm an update check. (r722) 
-Fix: Warnings are displayed when there are no unofficial subdirectories under the LDraw directory. (r723) 
 *When no search directories are defined (i.e. No Unofficial or Models directories exist) and the user opens the Preferences dialog, teh LDraw Content Search Directories dialog will validate an empty dialog with a warning that the 'entry' is an invalid search directory. In such case no warnings should be displayed. 
-Fix: Installation program presents options to delete 'old' library directory on new installation when no old directory existed. (r724) 
 *Comparison of old and new are not the same evaluated to true. Comparison improved to check that old is actually a directory also. 
-Change: Preferences/Other/Check for updates/Version now presents all valid updatable versions in a dropdown list. (r725,r728) 
 *Manage better the update dialog. Restrict entries to only valid update versions. 
-Fix: Crash when last line in main model of mpd file is a part type line - i.e. line starts with 1 to 5. (r726) 
 *This behavior will be seen when the user loads a model file without the meta tag '0 STEP' or '0 NOFILE'. 
-Fix: Export and PDF generation produces "Failed to create CSI" and does not produce model images in the generated document.(r727) 
 *Temporary testing code blocked the creation of CSI images - my apoligies:-( 
 
LPub3D 2.0.2.721.2 
 
Features and enhancements 
------------ 
-Fix: Add all Visual C++ dependencies to installation and portable distributions including VC++ 2015 redistributable runtime libraries (r720) 
 *Repackaged all distributions to incorporate all MSVC 2015 required dependency libraries. The Visual C++ Redistributable for Visual Studio 2015 is also included in the portable distributions. For the executable installation distribution, the installation program will check if the required libraries exist before silently installing the respective Visual C++ redistributable. 
 -Known Issue: Storing and retrieving the screen pos[ition], size, state and geometry settings appear to be causing intermittent crash on startup - at the end of the splash screen display. (r718/r719) 
 *This functionality has been disabled for the moment while I investigate. This unexpected behavior was introduced with the Qt5.6/MSVC 2015 development platform. The change you may notice is that the application no longer starts on the screen you last close it. Instead, it will will always start at the same location. While I have experienced this behavior on a multi-screen display configuraion, there are reports that this behavior also exist on single-screen displays. 
-Fix: Disable search directory validation in Preferences dialog - temporary workaround (r721) 
 *Warnings are displayed when there are no unofficial subdirectories under the LDraw directory. In such case no warnings should be displayed. 
 
LPub3D 2.0.1.717.2 
 
Features and enhancements 
------------ 
-Fix: In support of archive library move to AppData (see r707), the installation process will delete the 'old' LDRaw library archive directory even if it is actually the new directory. For example this can be reproduced if one attempts to reinstall LPub3D where the actual 'old' directory would have been deleted during the initial installation. The installation will now compare the 'old' directory to the new one and present the user the option to delete if the comparison does not match. (r715) 
-Fix: At applicaton launch, the 'Check for updates' does not detect the installed version. As a result, if the user performs a check for update or update checks are set to automatic, the user is presented with a message dialog stating a new update version is available when in fact this is not the case. The correct message dialog is now presented. Runaway eventloop when newest version is same as installed version. (r716) 
 
LPub3D 2.0.0.714.2 
 
Features and enhancements 
------------ 
-Up to 60 percent increase rendering speed with configuration to render multiple files using a single call to LDView. 
 *All PLI (part list/BOM) parts for a given step are rendered in a single call versus individually. For CSI (Step models), all multi-step images on a page are rendered in a single call including callout(s). Single step page images are rendered with a single call for all model images including those in callout(s). This new confiuration will default as checked in the preferences dialog. 
 *To achieve this behavior, input files (csi.ldr, pli.ldr) are now uniquely named because rendering multiple files is done by using -SaveSnapshots=1 instead of -SaveSnapshot=somefile.png and then listing all the LDR files at the end of the command line. There is no control over the output file names. Instead, LDView will automatically use the input base filename changing only the output filename extension from .ldr to .png. 
 *Enabling this feature is optonal and can be selected on the Configuration/Preferences/Rendering tab by checking the box 'Use multiple files single call rendering' in the 'LDView is installed' group box. LDView must be installed and configured to enable this option. 
 *Notice: As this feature required a significant rewrite of the core image generation logic, it is likely to generate some unexpected behavior as not every scenario has been tested. Please report any unexptected behavior. Contact details can be found under the Help/About menu. 
-Move LPub3D Ldraw archive libraries to AppData and rename unofficial library to lpub3dldrawunf.zip (r707) 
 *Archive libraries loaded automatically by ldraw during ldraw installation and is also distributed with portable media 
 *Archive libraries stored in user data (i.e. on Windows user/AppData/Local/Lpub3D Software/Lpub/libraries) 
 *LPub3D does not use parts from LDraw directory and ldconfig will fallback to the imbedded resource version if no LDraw directory is detected (This feature is not yet fully enabled. It is targeting a future enhancement) 
 *With exception of 'p' and 'parts', parts stored under .\LDraw\Unofficial are automatically added to lpub3d archive during application startup 
 *Lpub3d expects the official and unofficial library archive in the same directory. 
 *Lpub3d will prompt for archive file on startup if none is detected in the default expected location - this feature is targeted to support portable media and repackaged distributions (i.e. AIOI installations) 
 *If portable media installed in Program Files/(x86) folder, LPub3D will warn that it cannot place writable files at the default data location (folders under the installation directory) because UAC will prevent update access. This is useful for repackaged distributions (i.e. AIOI installations) 
-Multi-step callouts are now movable (r656) 
 *Callouts on multi-step pages are now freely movable on the page. 
-Add menu option to download official LDraw archive - unofficial archive was already available in v1.3.5 (r708) 
 *You can refresh both your official and unofficial LDraw library archives from the UI - As you may see from reading the change log, the aim is to over time allow LPub3D to be completely autonomous while maintaining it's ability to detect and consume unique and individual LDraw environment configurations. 
-Add callouts, refactor and optimize LDView single call image generation, see r683,r684,r685 (r698) 
 *The LDView 'Single Call' rendering performance improvement will address parts list and model rendering for single step page, multi-step page and callouts - including divided pages/callouts. For example, a callout with 6 models, divided into 3 sections will send a single call to LDView to render all 6 model (CSI) images. 
-Add progress bar to mpd/ldr file load (r690) 
 *Users can now see the progress of loading models. This is particularly useful for large models or models which include a large number of lines due to generated flex-parts etc... 
-Enable 'Use multiple files single call rendering' for LDView from Preferences/Rendering/LDView is installed/Use Multiple files single call rendering. (r687) 
 *LDView 'Single Call' rendering performance improvement can be selected or unselected from the preferences dialog. Although I tested the as many scenarios as I can think of. If this new feature encounters some unique model file configuration that causes unexpected behavior, the user can revert back to the legacy functionality if s/he so desire. 
-Increase PLI (Part list/BOM) and CSI (Model) rendering speed with LDView using -SaveSnapshots=1 (r683/r684/r685) 
 *'Single Call' performance improvement for LDView 
-Redesign software and library update functionality (r710) 
 *Update architecture improved to allow archive library download during installation if no library is defined. This feature is positioned for mobile/packaged distributions (i.e. AIOI) distributions. 
-Display application load progress during splash screen display (r676/r709) 
 *User can follow the application loading progress during startup launch 
-Add context menu to pages without steps - e.g. Cover Page, BOM Page (r681) 
 *Pages without steps (e.g. Cover pages, BOM pages etc.) can be modified using the context menu. For example, to change background colour. 
-Add missing context menu icons (r680) 
 *Small beautification enhancement - all context menu items now show a representative icon - this can be helpful when generating user guides for example. 
-Supress rotateIcon context menu item if rotate icon already inserted for step (r670) 
 *If a step already has an inserted rotate icon, the page/callout context menu will not present the option to add a rotate icon. Note: The option to delete a rotate icon presents from the context menu of the rotate icon itself and not from the Step/Callout's context menu. 
-On a single-step page, place PLI (Parts List) on top of CSI (Model) images (r664) 
 *If a CIS (model) overlaps the PLI (parts list) the parts list will be presented on top so the instructions consumer can see all the parts implicated for the step presented. The reverse was previously the case. 
-Upon reset image and model caches, reload current file, return to first page (r663) 
 *Reset cache options which clears all cache components (ldr, images, 3D views) will also reset the loaded file to the first page. 
-Unique splash 3D model displayed during pdf printing, png, jpg and bmp image export (r657) 
 *Updated the 3D Viewer splash to reflect the format of output being generated - so an image depicting a pdf file will show when generating a pdf file and so on... 
-ldglite update to 1.3.1 executable with -2g,2x option hardcoded for sharper images with offscreen rendering (r655) 
 *Incorporated the new ldglite renderer with 2x sampling for improved image quality 
-Disable Clear Cache menu button when no file is loaded (r686) 
 *Just a little poka yoke. The cache locations are specific to the file loaded, so it does not help to have this button active when no file is loaded. 
-Force to empty the cache before creating a PDF (r654) 
 *Added option to empty the image ldr and image cache before printing. With this feature the user can select the option on the print configuration dialog to clear the cache and regenerate images as the print/export job executes. 
-Synchronize 'prev', 'next' and 'goto' page menu items (r653) 
 *User can enter directly any page number within range into the menu dialog. Clicking either the Previous or Next button will navigate to the page indicated regardless of if the page is the next increment backward (for Previous) or forward (for Next) or a page further backward or forward. The user can still hit the enter key to navigate to the indicated page - if not the current page of course. 
-Supress rotateIcon from CSI (model) item context menu if we have a callout and the callout assembled (r652) 
 *The menu option to insert a rotate step icon is not presented for assembled callouts. 
-Select local page size and global and local page orientation - Portrait and Landscape (r518) 
 *Note: When manually editing the model file with either page size or orientation, it is recommended to insert both the size and orientation metas together. For example, even if you are only interested to add a page size, you shold update your file like the example 
 below - obviously selecting your own size values and orientation. 
 0 !LPUB PAGE SIZE GLOBAL 8.2677 11.6929 
 0 !LPUB PAGE ORIENTATION GLOBAL PORTRAIT 
-Change: Page background context menu rearranged. "Change page background" and "Change Page Size or Orientation" now appear at the end of the menu list because they are likely to be least often used. (r641) 
-Change: Point online manual to LPub3D content - was previously pointing to legacy LPub (r517) 
-Refactor: Move library archives to AppData stabilization and robustness (r711) 
-Refactor: Optimize fadeStep routines; change LDView logging details (r695) 
-Refactor: Replace 0x050000 with QT_VERSION_CHECK(5,0,0) (r660) 
-Refactor: Remove stretch/tile logic from coverImage management (r659) 
-Refactor: Update CSI image mousePressEvent logic. (r640) 
-Refactor: Update file load progress messages (r636) 
-Refactor: Rearrange printToPdfFile page count (r632) 
-Update: Clear 3D Window when there is no model to display - e.g. for cover or inserted pages (r701) 
-Update: Set download dialog header to Library Updater when downloading library - otherwise Software Updater (r700) 
-Update: AboutDialog window recognizes and displays Win 10 and OSX El Capitan(r665/r668/r669) 
-Update: ldglite hard-coded default directory setting to ldglite1.3.1(r623) 
-Update: 3DViewer minifig settings moved to ini file - LeoCAD Changeset 1870 (r617x) 
-Fix: No image after initial generation when renderer other than LDView selected (r704) 
-Fix: Set ldr load file to unofficial item by default (r691) 
-Fix: Load inline submodels break (r688) 
-Fix: Remove libpng warning: iCCP: known incorrect sRGB profile (r678) 
-Fix: Convert special characters (copyright and trademark) from Wide Char to UTF8 for MSVC build (r677) 
-Fix: Remove final colour model if exist when Fade Step is not enabled (r674) 
-Fix: Clear cache when turning fade on/off (r674) 
-Fix: FadeStep and preferCentimeter setting mixmatch (r674) 
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
-Upgrade: With upgrade to Qt5x, moved to MSVC 2015 release builds to continue providing x85 and x64 builds as MinGW does not officially support x64 on Windows 
-Upgrade: Updated development platform to Qt 5.6.1 (MSVC 2015) (r613) 
-Upgrade: Updated Qt logging framework from Qt4x to Qt5x (r610) 
-Upgrade: Updated development platform to Qt 5.5.1 (MinGW) (r608) 
 
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
