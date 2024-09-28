[![GitHub release][gh-rel-badge]][gh-rel-url]                        |[![GitHub commits (since release)][gh-comm-since-badge]][gh-comm-since-url]|[![GitHub last commit][gh-lst-commit-badge]][gh-lst-commit-url]
:--------------------------------------------------------------------|:---------------------------------------------------------------------------------|:---------------------------------------------------------------------------
[![SourceForge weekly downloads][sf-dw-badge]][sf-dw-badge-url]      |[![SourceForge total downloads][sf-dt-badge]][sf-dt-badge-url]                    |[![GitHub total downloads][gh-dl-badge]][gh-dl-url]
[![GitHub issues closed][gh-issue-closed-badge]][gh-issue-closed-url]|[![GitHub issues][gh-issue-badge]][gh-issue-url]                                  |[![GitHub pull requests][gh-pull-req-badge]][gh-pull-req-url]
[![GitHub build status][gha-build-badge]][gh-actions-url]            |[![GitHub quality check][gha-quality-badge]][gh-actions-url]                      |[![Maintained status][gh-maintained-badge]][gh-maintained-url]

**LPub3D** is an Open Source WYSIWYG editing application for creating [LEGO®][lego-url] style digital building instructions.

LPub3D **output** include <span style="color:#b03060">instruction document</span>, <span style="color:#008b45">document pages</span>, <span style="color:#0000ee">part list</span> and <span style="color:#ff4500">export content</span>.

:----------------------------------------------:|:-------------------:|:---------------------
<span style="color:#b03060">PDF</span>          |![PDF-Logo][pdf-logo]|*Portable Document Format* instruction document
<span style="color:#008b45">PNG</span>          |![PNG-Logo][png-logo]|*Portable Network Graphics* images, instruction document pages and POV-Ray rendered images
<span style="color:#008b45">JPEG</span>         |![JPG-Logo][jpg-logo]|*Joint Photographic Experts Group* instruction document pages
<span style="color:#008b45">BMP</span>          |![BMP-Logo][bmp-logo]|*Bitmap* instruction document pages
<span style="color:#0000ee">HTML</span>         |![HTM-Logo][htm-logo]|*Hypertext Markup Language* sortable graphic part list with element ID and online part lookup
<span style="color:#0000ee">BrickLink XML</span>|![XML-Logo][xml-logo]|*Extensible Markup Language* BrickLink part list
<span style="color:#0000ee">CSV</span>          |![CSV-Logo][csv-logo]|*Comma-Separated Values* plain text part list
<span style="color:#ff4500">POV</span>          |![POV-Logo][pov-logo]|*Persistence of Vision Ray Tracer* (POV-Ray) photo-realistic scene files
<span style="color:#ff4500">OBJ</span>          |![OBJ-Logo][obj-logo]|*Wavefront Object* files
<span style="color:#ff4500">STL</span>          |![STL-Logo][stl-logo]|*Stereolithography* object files
<span style="color:#ff4500">3DS</span>          |![3DS-Logo][3ds-logo]|*Autodesk 3DStudio Max* object files
<span style="color:#ff4500">DAE</span>          |![DAE-Logo][dae-logo]|*COLLAborative Design Activity* object files

LPub3D uses the LDraw standard and [LEGO®][lego-url] Geometrical Equivalent Objects (LGEO) photo-realistic **part libraries** and reads the LDraw DAT, LDR and MPD model file formats. The LDraw libraries are not required to be preinstalled. Archive (.zip) library files for both official and unofficial LDraw parts are bundled with LPub3D. Follow the linked image below to acquire the LGEO photo-realistic part library.

:-----------------|:------------------------------------:|:--------------------
[LDraw][ldraw-url]|[![LDraw-Logo][ldraw-logo]][ldraw-url]|[LDraw&trade;][ldraw-url] parts is an open standard library that allow users to create virtual models and scenes. Three LDraw part libraries are bundled with LPub3D.<br>![LEGO][lego-logo] [LDraw&trade;][ldraw-url] official and unofficial LEGO® part library.<br>![TENTE][tente-logo] TENTE® unofficial LDraw part library created by José Alfonso Solera (Jasolo). <br>![VEXIQ][vexiq-logo] VEX IQ® unofficial LDraw library created by Philippe Hurbain (Philo).<br>![LSynth][lsynth-logo] [LSynth][lsynth-url] bendable parts synthesis elements created by Kevin Clague and updated by Don Heyse (deeice).
[LGEO][lgeo-url]  |[![LGEO-Logo][lgeo-logo]][lgeo-url]   |POV-Ray photo-realistic part library created by Lutz Uhlmann (El Lutzo), updated by Owen Burgoyne (C3POwen) and extended with STL object files by Damien Roux (Darats).

LPub3D **Blender integration** uses the [LPub3D Blender LDraw Addon][blender-addon-url] which imports [LDraw&trade;][ldraw-url] models into Blender, exports models from Blender and renders imported models using the full range of Blender capabilities. LPub3D also provides "native" **3DViewer**, **POV scene file generation**, including POV-Ray PNG image rendering using integrated modules based on LeoCAD and LDView. Additionally, LDView, LDGLite and POV-Ray 3rd-party stand-alone **LDraw model renderers** are bundled in each LPub3D distribution. All stand-alone renderers are console applications custom adapted to LPub3D for tight integration, optimum configurability and robust performance.

:---------------------|:------------------------------------------:|:----------
[LeoCAD][leocad-url]  |[![LeoCAD-logo][leocad-logo]][leocad-url]   |LPub3D "native" renderer and 3D Viewer - performant, "true fade" rendering
[Blender][blender-url]|[![Blender-Logo][blender-logo]][blender-url]|Best in class, 3D computer graphics toolset for high quality photo-realistic renderings
[LDGLite][ldglite-url]|[![LDGLite-Logo][ldglite-logo]][ldglite-url]|fast image rendering ideal for proofing
[LDView][ldview-url]  |[![LDView-Logo][ldview-logo]][ldview-url]   |high quality rendering plus POV scene file generation
[<nobr>POV-Ray</nobr>][povray-url]  |[![POVRay-Logo][povray-logo]][povray-url]   |ray tracing renderer - photo-realistic, feature rich rendering

### Screenshots

{% include slider.html selector="slider1" %}

### Compatibility

:-----------------------:|:-------------------:|:-------------------:|:-------------------:|:---------------------
![Intel-Logo][intel-logo]|![AMD-Logo][amd-logo]|![APPLE-M1-Logo][apple-m1-logo]|![ARM-Logo][arm-logo]|![RaspberryPi-Logo][raspberrypi-logo]

LPub3D runs on Debian, Red Hat and Arch derived Linux distros, macOS and Windows Operating Systems.
LPub3D is also available as a Linux 'no-install', multi-platform [AppImage][appimage-site-url] and [Snap][snapcraft-site-url] or [Flatpak][flathub-site-url] 'sandbox' deployment. Builds are available for both x86 instruction set (32-bit and 64-bit) and Advanced RISC Machines (ARM) micorprocessors.

### Downloads

:---------------------------------------------|:-------------------------------
[![GitHub-Logo][github-logo]][github_releases]|[GitHub Releases][github_releases] - Linux, macOS and Windows Distributions
[![SF-Logo][sf-logo]][sf_file_releases]            |[Sourceforge Releases][sf_releases] - Linux, macOS and Windows Distributions
[![OBS-Logo][obs-logo]][obs_releases]         |[Open Build Service Releases][obs_pkg_releases] - Linux distributions

### Distributions

[![Appimage-Icon][appimage-icon]][appimage-url]|[![Arch-Icon][arch-icon]][arch-url]                  |[![Centos-Icon][centos-icon]][centos-url]                   |[![Debian-Icon][debian-icon]][debian-url]             |[![Fedora-Icon][fedora-icon]][fedora-url]                |[![Mageia-Icon][mageia-icon]][mageia-url]            |[![Macos-Icon][macos-icon]][macos-url]
:---------------------------------------------:|:---------------------------------------------------:|:----------------------------------------------------------:|:----------------------------------------------------:|:-------------------------------------------------------:|:---------------------------------------------------:|:------------------------------------------:
[![Opensuse-Icon][opensuse-icon]][opensuse-url]|[![Raspbian-Icon][raspbian-icon]][raspbian-url]      |[![Rhel-Icon][rhel-icon]][rhel-url]                         |[![Sl-Icon][sl-icon]][sl-url]                         |[![Sle-Icon][sle-icon]][sle-url]                         |[![Ubuntu-Icon][ubuntu-icon]][ubuntu-url]            |[![Windows-Icon][windows-icon]][windows-url]
                                               |                                                     |[![AlmaLinux-Icon][almalinux-icon]][almalinux-url]          |[![openEuler-Icon][openeuler-icon]][openeuler-url]  |[![Univention-Icon][univention-icon]][univention-url]    |                                                     |
                                               |[![Elementary-Icon][elementary-icon]][elementary-url]|[![KDE_Neon-Icon][kde_neon-icon]][kde_neon-url]             |[![Pop_Os-Icon][pop_os-icon]][pop_os-url]             |[![Manjaro-Icon][manjaro-icon]][manjaro-url]             |[![Kubuntu-Icon][kubuntu-icon]][kubuntu-url]         |
                                               |                                                     |[![Snapcraft-Icon][snapcraft-site-icon]][snapcraft-site-url]|[![Flatpak-Icon][flathub-site-icon]][flathub-site-url]|[![AppImage-Icon][appimage-site-icon]][appimage-site-url]|                                                     |

 - Click icon above to download and/or install distribution release.

### Verify Windows Distribution Hash using PowerShell
The following steps are using LPub3D Windows release hosted on [GitHub][github_releases].
These steps can be applied to any LPub3D release where a GPG signature file is available.
- Get the hash file [LPub3D-2.4.7.75.3551_20230905.exe.sha512](https://github.com/trevorsandy/lpub3d/releases/download/continuous/LPub3D-2.4.7.75.3551_20230905.exe.sha512)
- Open a PowerShell session
- Add the package download path to `$PkgUrl`
- Add the hash file sha512 hash to `$PublishedHash`.
```
$WebClient = [System.Net.WebClient]::new()
$PkgUrl = 'https://github.com/trevorsandy/lpub3d/releases/download/continuous/LPub3D-2.4.7.75.3551_20230905.exe'
$PublishedHash = 'f810fbe3394349486df4ef25675baad3963188b9a3e2ede56ca488a152de8028feab1ee3977ffbc052ca2cbfb36b21a5d2c99103f35d020b13408b46d6dff3f5'
$FileHash = Get-FileHash -Algorithm SHA512 -InputStream ($WebClient.OpenRead($PkgUrl))
$FileHash.Hash -eq $PublishedHash
```
- Cut and paste this code snippet into Powershell, a valid return value is `True`.

### Verify GPG Signature using Bash Unix Shell
The following steps are using LPub3D AppImage release hosted on [GitHub][github_releases].
These steps can be applied to any LPub3D release where a GPG signature file is available.
- Get the GPG public key: [pubring.auto](https://raw.github.com/trevorsandy/lpub3d/master/builds/utilities/ci/secure/pubring.auto)
- Get the release: [LPub3D-2.4.7.75.3551_20230905-x86_64.AppImage](https://github.com/trevorsandy/lpub3d/releases/download/continuous/LPub3D-2.4.7.75.3551_20230905-x86_64.AppImage)
- Get the hash file: [LPub3D-2.4.7.75.3551_20230905-x86_64.AppImage.sha512](https://github.com/trevorsandy/lpub3d/releases/download/continuous/LPub3D-2.4.7.75.3551_20230905-x86_64.AppImage.sha512)
- Get the GPG hash file signature: [LPub3D-2.4.7.75.3551_20230905-x86_64.AppImage.sha512.sig](https://github.com/trevorsandy/lpub3d/releases/download/continuous/LPub3D-2.4.7.75.3551_20230905-x86_64.AppImage.sha512.sig)
- Ensure GPG is installed and configured: `$> which gpg`
- Import the public key file into GPG: `$> gpg --import pubring.auto`
- Verify the against its hash file: `$> sha512sum -c LPub3D-2.4.7.75.3551_20230905-x86_64.AppImage.sha512`
- Verify the GPG hash file signature: `$> gpg --verify LPub3D-2.4.7.75.3551_20230905-x86_64.AppImage.sha512.sig`

### Quick Links
 - [Build and package LPub3D from source][buildfromsource]
 - [Release notes][docs-lpub3d-release-notes]
 - [Change log (Deprecated - use Release notes)][docs-lpub3d-changelog]
 - [Questions and discussion][sf-gen-discussion]
 - [Report an Issue][gh-new-issue]
 - [Credits][docs-lpub3d-credits]

### License

:--------------------------------------|:-------------------------------------------------------------------------------------------
[![GPLv3-logo][gplv3-logo]][lgplv3-url]|LPub3D is available for free under the [GNU Public License version 3.0 (GPLv3)][lgplv3-url].

### Disclaimers
LEGO® is a trademark of the LEGO Group of companies which does not sponsor, authorize or endorse this application.<br>
TENTE® is a construction game owned by Educa-Borrás SA which does not sponsor, authorize or endorse this application.<br>
VEX IQ® is a trademarks or service mark of Innovation First International, Inc, which does not sponsor, authorize or endorse this application.

### Documents
 - [LPub3D Metacommands][docs-lpub3d-meta]
 - [LPub3D macOS Library Dependencies][docs-lpub3d-macos-libs]
 - [LDView Help][docs-ldview-help]
 - [LDGLite Readme][docs-ldglite-readme]
 - [LPub3D-Trace (POV-Ray) Help][docs-povray-help]

### Tutorials
3rd-party tutorials:
 - [Working with LPub by Jaco van der Molen][working-with-lpub3d]
 - [LPub3D par J.C Tchang (en français)][jctchang-lpub3d]

### Resources
 - [LPub3D meta commands UDL for Notepad++ (zip download)][resources-lpub3d-npp-udl]

### Features and Enhancements
 - MPD file imbedded images with !DATA meta command [#722](https://github.com/trevorsandy/lpub3d/issues/722) <span style="color:red">(NEW!)</span>
 - Enhanced step part fade and highlight behaviour [#710](https://github.com/trevorsandy/lpub3d/issues/710) <span style="color:red">(NEW!)</span>
 - Display model step supports part lines [#704](https://github.com/trevorsandy/lpub3d/issues/704) <span style="color:red">(NEW!)</span>
 - Model load status available from file menu action [#698](https://github.com/trevorsandy/lpub3d/issues/698) <span style="color:red">(NEW!)</span>
 - Enhanced Blender LDraw model render, import, and export integration [#416](https://github.com/trevorsandy/lpub3d/issues/416) <span style="color:red">(NEW!)</span>
 - Build modification - visual editor build modification actions [#633](https://github.com/trevorsandy/lpub3d/issues/633)
 - Reset viewer assembly display menu action #652 [#652](https://github.com/trevorsandy/lpub3d/issues/652)
 - Specify viewpoint latitude and longitude angles #654 [#654](https://github.com/trevorsandy/lpub3d/issues/654)
 - LPub command snippet system [#602](https://github.com/trevorsandy/lpub3d/issues/602)
 - Expand WhatsThis help to preferences, dialogs and actions - Ctrl+F1 [#618](https://github.com/trevorsandy/lpub3d/issues/618)
 - Add shortcut to Zoom Extents - Shift+X [#470](https://github.com/trevorsandy/lpub3d/issues/470)
 - Append page step placement options [#491](https://github.com/trevorsandy/lpub3d/issues/491)
 - Bill of materials step placement options [#492](https://github.com/trevorsandy/lpub3d/issues/492)
 - Configurable Theme colors [#490](https://github.com/trevorsandy/lpub3d/issues/490)
 - Dynamically generate fade steps and/or highlight step final model step [#483](https://github.com/trevorsandy/lpub3d/issues/483)
 - Enable LEGO Digital Designer and set inventory import [#506](https://github.com/trevorsandy/lpub3d/issues/506)
 - Execute fade, highlight, and preferred renderer locally [#505](https://github.com/trevorsandy/lpub3d/issues/505)
 - Executable verification with exported SHA-512 checksum file for GitHub builds [#460](https://github.com/trevorsandy/lpub3d/issues/460)
 - Make generate pointer arrow optional when converting to steps to callout [#501](https://github.com/trevorsandy/lpub3d/issues/501)
 - Native renderer official, unofficial LDraw parts load/lookup order option [#509](https://github.com/trevorsandy/lpub3d/issues/509)
 - Set project start page and start step number for multi-document layouts [#477](https://github.com/trevorsandy/lpub3d/issues/477)
 - Build modification - BUFEXCHG replacement, modify part settings with ease [#411](https://github.com/trevorsandy/lpub3d/issues/411)
 - Double-click editor line, time-line item, part or submodel for 3D preview [#441](https://github.com/trevorsandy/lpub3d/issues/441)
 - Substitute part dialogue with part preview [#280](https://github.com/trevorsandy/lpub3d/issues/280)
 - Manage camera properties, including target, rotation and position, from 3DViewer [#344](https://github.com/trevorsandy/lpub3d/issues/344)
 - Streamline formatting and command reuse with Include File meta command [#396](https://github.com/trevorsandy/lpub3d/issues/396)
 - Add point, sun or area light to Blender rendering [#413](https://github.com/trevorsandy/lpub3d/issues/413)
 - Render wire, raised and rounded stud logo [#425](https://github.com/trevorsandy/lpub3d/issues/425)
 - Add bring-to-front and send-to-back for page items [#244](https://github.com/trevorsandy/lpub3d/issues/244)
 - Open model file with external program [#420](https://github.com/trevorsandy/lpub3d/issues/420)
 - Insert display model between steps [#415](https://github.com/trevorsandy/lpub3d/issues/415)
 - Colour picker and part lookup [#427](https://github.com/trevorsandy/lpub3d/issues/427)
 - Set PDF, PNG, JPG and BMP pixel ratio for output resolution [#333](https://github.com/trevorsandy/lpub3d/issues/333)
 - Center justify step group and callout steps [#333](https://github.com/trevorsandy/lpub3d/issues/333)
 - Page items bring to front and send to back [#244](https://github.com/trevorsandy/lpub3d/issues/244)
 - Add, remove, edit substitute part from PLI context menu [#244](https://github.com/trevorsandy/lpub3d/issues/244)
 - Automatically load LDCad generated flex parts [#272](https://github.com/trevorsandy/lpub3d/issues/272)
 - Configurable LEGO style CSI, PLI and BOM annotation formatting [#186](https://github.com/trevorsandy/lpub3d/issues/186) [#187](https://github.com/trevorsandy/lpub3d/issues/187)
 - Step and range dividers with configurable pointer arrows [#185](https://github.com/trevorsandy/lpub3d/issues/185)
 - Render configurable POV-Ray generated images [#207](https://github.com/trevorsandy/lpub3d/issues/207)
 - Continuous submodel step numbering [#179](https://github.com/trevorsandy/lpub3d/issues/179)
 - Generate Sortable HTML part list with element ID [#196](https://github.com/trevorsandy/lpub3d/issues/196)
 - OBJ, 3DS, STL, DAE, CSV and BrickLink XML exports [#188](https://github.com/trevorsandy/lpub3d/issues/188) [#195](https://github.com/trevorsandy/lpub3d/issues/195)
 - Show submodel image at first step
 - Pre-bundled LSynth bendable part elements. Add to archive and, optionally, extract to LDraw folder
 - Pre-bundled and fully integrated LDraw part libraries for LEGO, TENTE and VEXIQ models
 - Continuous page processing - step through your model with one click
 - Go Dark - Dark mode theme, switch on the fly
 - Multi-segment page, divider and callout pointers
 - Enhanced “True” previous step part fade
 - Current step silhouette part highlight
 - Scene horizontal and vertical ruler and axis guides
 - Console commands - enabling "Batch" mode
 - "Headless" mode on Linux, macOS and Windows platforms
 - "Native" POV file generation powered by LDView
 - Integrated 3D viewer and image renderer powered by [LeoCAD][leocad-url]
 - Integrated and custom adapted image renderers ([LDGlite][ldglite-url], [LDView][ldview-url] and LPub3D-Trace ([POV-Ray][povray-url])) - no more additional installs
 - Renderer config files accessible from the LPub3D user interface
 - Custom LDraw library LDConfig (part material) file can be passed to LDGlite and LDView renderers
 - Set ROTSTEP (step rotation) meta using 3D Viewer
 - Fade previous step's parts (Fade step)
 - Unlimited part list (PLI) annotations
 - Split the BOM (divide parts over number of BOM occurrences)
 - Sort PLI/BOM by part size, colour and category
 - "Go To" page navigation and submodel display via drop-down dialogue
 - Automatically generate formatted cover pages
 - POV-Ray high-quality rendered instructions
 - LGEO high-quality rendering library support and auto detection
 - Customizable content page header and footer
 - Text and picture (e.g. logo) front and back page attributes
 - Print/Export preview
 - PDF, PNG, JPG and BMP export formats
 - Standard and custom page size options
 - On demand, software and LDraw archive library updates
 - All features from legacy LPub - enhanced and stabilized
 - Linux, Linux AppImage, macOS and Microsoft Windows distributions
 - Windows automated installer and portable distributions available
 - Full capabilities to [build and package from source][buildfromsource] or Cloud Continuous Integration Service
 - Automated build and deploy from Cloud CI Service: [GitHub Actions][gh-actions-url] and [Open Build Service][obs-url]
 - [Docker][dockerinstall] container builds using [Docker Compose][dockercomposefile] available for [Arch][dockerarch], [Ubuntu][dockerubuntu] and [Fedora][dockerfedora] Linux distros

 Portions of LPub3D are based on LPUB© 2007-2009 Kevin Clague, LeoCAD© 2024 Leonardo Zide, LDView© 2024 Travis Cobbs & Peter Bartfai and additional third party components. See [Credits][docs-lpub3d-credits] for full details on contributing content.

 [![Built-With-Qt-Logo][builtwithqt-logo]][qt-url]

[docs-lpub3d-release-notes]:  {{ '/assets/docs/lpub3d/RELEASE_NOTES.html' | relative_url }} "Review LPub3D release features, enhancements, fixes and changes"
[docs-lpub3d-changelog]:      {{ '/assets/docs/lpub3d/README.txt' | relative_url }} "Review LPub3D changes and enhancements (Deprecated)"
[docs-lpub3d-credits]:        {{ '/assets/docs/lpub3d/CREDITS.txt' | relative_url }} "See whose content has contributed to LPub3D"
[docs-lpub3d-copying]:        {{ '/assets/docs/lpub3d/COPYING.txt' | relative_url }} "LPub3D open source license document"

[docs-lpub3d-meta]:           {{ '/assets/docs/lpub3d/metacommands.html' | relative_url }}
[docs-lpub3d-macos-libs]:     {{ '/assets/docs/lpub3d/macOS_libs.html' | relative_url }}
[docs-ldview-help]:           {{ '/assets/docs/ldview/Help.html' | relative_url }}
[docs-ldglite-readme]:        {{ '/assets/docs/ldglite/README.TXT' | relative_url }}
[docs-povray-help]:           {{ '/assets/docs/povray/index.html' | relative_url }}

[resources-lpub3d-npp-udl]:   {{ '/assets/resources/LPub3D_Npp_UDL.xml.zip' | relative_url }} "User defined language for Notepad++ editor"

[blender-logo]:               {{ '/assets/images/blender_logo.png' | relative_url }}
[leocad-logo]:                {{ '/assets/images/leocad_logo.png' | relative_url }}
[ldview-logo]:                {{ '/assets/images/ldview_logo.png' | relative_url }}
[ldglite-logo]:               {{ '/assets/images/ldglite_logo.png' | relative_url }}
[povray-logo]:                {{ '/assets/images/povray_logo.png' | relative_url }}

[lego-url]:            https://www.lego.com
[blender-url]:         https://www.blender.org
[leocad-url]:          https://github.com/trevorsandy/lpub3d/tree/master/lclib
[ldglite-url]:         https://github.com/trevorsandy/ldglite
[ldview-url]:          https://github.com/trevorsandy/ldview/tree/qmake-build
[povray-url]:          https://github.com/trevorsandy/povray/tree/lpub3d/raytracer-cui
[lsynth-url]:          https://github.com/deeice/lsynth
[blender-addon-url]:   https://github.com/trevorsandy/blenderldrawrender

[github-logo]:         {{ '/assets/images/github_logo.png' | relative_url }} "AppImage, Windows, macOS, Ubuntu, Arch, Fedora, openSUSE ARM and Rasberry Pi distributions"
[sf-logo]:             {{ '/assets/images/sf_logo.png' | relative_url }} "AppImage, Windows, macOS, Ubuntu, Arch, Fedora, openSUSE ARM and Rasberry Pi distributions"
[obs-logo]:            {{ '/assets/images/obs_logo.png' | relative_url }} "All listed AppImage and Linux distributions"

[pdf-logo]:            {{ '/assets/images/pdf_logo.png' | relative_url }} "instruction document"

[png-logo]:            {{ '/assets/images/png_logo.png' | relative_url }} "document pages"
[jpg-logo]:            {{ '/assets/images/jpg_logo.png' | relative_url }} "document pages"
[bmp-logo]:            {{ '/assets/images/bmp_logo.png' | relative_url }} "document pages"

[htm-logo]:            {{ '/assets/images/htm_logo.png' | relative_url }} "part list"
[xml-logo]:            {{ '/assets/images/xml_logo.png' | relative_url }} "part list"
[csv-logo]:            {{ '/assets/images/csv_logo.png' | relative_url }} "part list"

[pov-logo]:            {{ '/assets/images/pov_logo.png' | relative_url }} "export content"
[obj-logo]:            {{ '/assets/images/obj_logo.png' | relative_url }} "export content"
[stl-logo]:            {{ '/assets/images/stl_logo.png' | relative_url }} "export content"
[3ds-logo]:            {{ '/assets/images/3ds_logo.png' | relative_url }} "export content"
[dae-logo]:            {{ '/assets/images/dae_logo.png' | relative_url }} "export content"

[ldraw-logo]:          {{ '/assets/images/ldraw_logo.png' | relative_url }}
[lego-logo]:           {{ '/assets/images/lego_logo.png' | relative_url }}
[tente-logo]:          {{ '/assets/images/tente_logo.png' | relative_url }}
[vexiq-logo]:          {{ '/assets/images/vexiq_logo.png' | relative_url }}
[lsynth-logo]:         {{ '/assets/images/lsynth_logo.png' | relative_url }}
[lgeo-logo]:           {{ '/assets/images/lgeo_logo.png' | relative_url }}
[gplv3-logo]:          {{ '/assets/images/gplv3_logo.png' | relative_url }}

[intel-logo]:          {{ '/assets/images/intel_logo.png' | relative_url }}
[amd-logo]:            {{ '/assets/images/amd_logo.png' | relative_url }}
[arm-logo]:            {{ '/assets/images/arm_logo.png' | relative_url }}
[apple-m1-logo]:       {{ '/assets/images/apple_m1_logo.png' | relative_url }}
[raspberrypi-logo]:    {{ '/assets/images/raspberrypi_logo.png' | relative_url }}
[builtwithqt-logo]:    {{ '/assets/images/builtwithqt_logo.png' | relative_url }}

[sf-gen-discussion]:   https://sourceforge.net/p/lpub3d/discussion/general "Ask a question or review past discussions"
[gh-new-issue]:        https://github.com/trevorsandy/lpub3d/issues/new "Report an issue with LPub3D"
[working-with-lpub3d]: https://sites.google.com/view/workingwithlpub3d/
[jctchang-lpub3d]:     http://jc-tchang.philohome.com/model/LPub3D.htm

[ldraw-url]:           http://www.ldraw.org/
[lgeo-url]:            https://www.eurobricks.com/forum/index.php?/forums/topic/108739-new-parts-for-lgeo-library/

[appimage-icon]:       {{ '/assets/images/appimage.png' | relative_url }} "AppImage - Ubuntu Jammy Jellyfish 22.04"
[macos-icon]:          {{ '/assets/images/macos.png' | relative_url }} "macOS Sonoma 14 (arm64), Ventura 13 (x86_64)"
[windows-icon]:        {{ '/assets/images/windows.png' | relative_url }} "Microsoft Windows Server 2022 10.0.20348"

[windows-url]:         https://github.com/trevorsandy/lpub3d/releases/download/continuous/LPub3D-2.4.7.75.3551_20230905.exe
[macos-url]:           https://github.com/trevorsandy/lpub3d/releases/download/continuous/LPub3D-2.4.7.75.3551_20230905-x86_64-macos.dmg
[appimage-url]:        https://github.com/trevorsandy/lpub3d/releases/download/continuous/LPub3D-2.4.7.75.3551_20230905-x86_64.AppImage

[almalinux-icon]:      {{ '/assets/images/almalinux_logo.png' | relative_url }} "Alma Linux 9"
[arch-icon]:           {{ '/assets/images/arch.png' | relative_url }} "Arch Linux"
[centos-icon]:         {{ '/assets/images/centos.png' | relative_url }} "CentOS 8"
[debian-icon]:         {{ '/assets/images/debian.png' | relative_url }} "Debian 11"
[fedora-icon]:         {{ '/assets/images/fedora.png' | relative_url }} "Fedora 36"
[mageia-icon]:         {{ '/assets/images/mageia.png' | relative_url }} "Mageia 8"
[opensuse-icon]:       {{ '/assets/images/opensuse.png' | relative_url }} "openSUSE Leap 42.3"
[openeuler-icon]:      {{ '/assets/images/openeuler_logo.png' | relative_url }} "openEuler 21.03"
[raspbian-icon]:       {{ '/assets/images/raspberrypi_logo.png' | relative_url }} "Raspbian 11 Noble Numbat" 
[rhel-icon]:           {{ '/assets/images/rhel.png' | relative_url }} "Red Hat Enterprise Linux 8"
[sl-icon]:             {{ '/assets/images/sl.png' | relative_url }} "Scientific Linux 7"
[sle-icon]:            {{ '/assets/images/sle.png' | relative_url }} "SUSE Enterprise Linux 15 SP4"
[ubuntu-icon]:         {{ '/assets/images/ubuntu.png' | relative_url }} "Ubuntu Noble Numbat 24.04"
[univention-icon]:     {{ '/assets/images/univention_logo.png' | relative_url }} "Univention Corporate Server 5.0"

[//]: # Snap icons
[elementary-icon]:     {{ '/assets/images/elementary_logo.png' | relative_url }} "Elementary OS Snap"
[kde_neon-icon]:       {{ '/assets/images/kde_neon_logo.png' | relative_url }} "KDE Neon Snap"
[pop_os-icon]:         {{ '/assets/images/pop_os_logo.png' | relative_url }} "Pop! OS Snap"
[manjaro-icon]:        {{ '/assets/images/manjaro_logo.png' | relative_url }} "Manjaro Linux Snap"
[kubuntu-icon]:        {{ '/assets/images/kubuntu_logo.png' | relative_url }} "Kubuntu Snap"

[almalinux-url]:       https://download.opensuse.org/repositories/home:/trevorsandy/AlmaLinux_9/
[arch-url]:            https://download.opensuse.org/repositories/home:/trevorsandy/ArchOST/
[centos-url]:          https://download.opensuse.org/repositories/home:/trevorsandy/CentOS_8/
[debian-url]:          https://download.opensuse.org/repositories/home:/trevorsandy/Debian_11/
[fedora-url]:          https://download.opensuse.org/repositories/home:/trevorsandy/Fedora_36/
[mageia-url]:          https://download.opensuse.org/repositories/home:/trevorsandy/Mageia_8/
[opensuse-url]:        https://download.opensuse.org/repositories/home:/trevorsandy/openSUSE_Leap_42.3/
[openeuler-url]:       https://download.opensuse.org/repositories/home:/trevorsandy/openEuler_21.03/
[raspbian-url]:        https://download.opensuse.org/repositories/home:/trevorsandy/Raspbian_11/
[rhel-url]:            https://download.opensuse.org/repositories/home:/trevorsandy/RHEL_8/
[sl-url]:              https://download.opensuse.org/repositories/home:/trevorsandy/ScientificLinux_7/
[sle-url]:             https://download.opensuse.org/repositories/home:/trevorsandy/SLE_15_SP4/
[ubuntu-url]:          https://download.opensuse.org/repositories/home:/trevorsandy/xUbuntu_23.04/
[univention-url]:      https://download.opensuse.org/repositories/home:/trevorsandy/Univention_5.0/

[//]: # Snap urls
[elementary-url]:      https://snapcraft.io/install/lpub3d/elementary/
[kde_neon-url]:        https://snapcraft.io/install/lpub3d/kde-neon/
[pop_os-url]:          https://snapcraft.io/install/lpub3d/pop/
[manjaro-url]:         https://snapcraft.io/install/lpub3d/manjaro/
[kubuntu-url]:         https://snapcraft.io/install/lpub3d/kubuntu/

[snapcraft-site-icon]: {{ '/assets/images/snapcraft_logo.png' | relative_url }} "Canonical Snapcraft"
[flathub-site-icon]:   {{ '/assets/images/flathub_logo.png' | relative_url }} "Flatpak Flathub"
[appimage-site-icon]:  {{ '/assets/images/appimage_logo.png' | relative_url }} "AppImage"

[snapcraft-site-url]:  https://snapcraft.io/lpub3d
[flathub-site-url]:    https://flathub.org/apps/details/io.github.trevorsandy.LPub3D
[appimage-site-url]:   https://appimage.github.io/LPub3D/

[lgplv3-url]:          https://www.gnu.org/licenses/gpl-3.0.en.html
[buildfromsource]:     https://github.com/trevorsandy/lpub3d/blob/master/builds/utilities/README.md
[qt-url]:              https://www.qt.io

[github_releases]:     https://github.com/trevorsandy/lpub3d/releases
[github_tag_releases]: https://github.com/trevorsandy/lpub3d/releases/tag/v2.4.7
[sf_releases]:         https://sourceforge.net/projects/lpub3d/files/Continuous
[sf_file_releases]:    https://sourceforge.net/projects/lpub3d/files
[obs_pkg_releases]:    https://software.opensuse.org/download.html?project=home:trevorsandy&package=lpub3d
[obs_releases]:        https://download.opensuse.org/repositories/home:/trevorsandy

[gha-build-badge]:     https://github.com/trevorsandy/lpub3d/actions/workflows/build.yml/badge.svg?event=push
[gha-quality-badge]:   https://github.com/trevorsandy/lpub3d/actions/workflows/codeql.yml/badge.svg?event=push
[gh-actions-url]:      https://github.com/trevorsandy/lpub3d/actions

[gh-rel-badge]:        https://img.shields.io/github/release/trevorsandy/lpub3d.svg?logo=data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSI0MCIgaGVpZ2h0PSI0MCIgdmlld0JveD0iMTIgMTIgNDAgNDAiPjxwYXRoIGZpbGw9IiNmZmYiIGQ9Ik0zMiwxMy40Yy0xMC41LDAtMTksOC41LTE5LDE5YzAsOC40LDUuNSwxNS41LDEzLDE4YzEsMC4yLDEuMy0wLjQsMS4zLTAuOWMwLTAuNSwwLTEuNywwLTMuMiBjLTUuMywxLjEtNi40LTIuNi02LjQtMi42QzIwLDQxLjYsMTguOCw0MSwxOC44LDQxYy0xLjctMS4yLDAuMS0xLjEsMC4xLTEuMWMxLjksMC4xLDIuOSwyLDIuOSwyYzEuNywyLjksNC41LDIuMSw1LjUsMS42IGMwLjItMS4yLDAuNy0yLjEsMS4yLTIuNmMtNC4yLTAuNS04LjctMi4xLTguNy05LjRjMC0yLjEsMC43LTMuNywyLTUuMWMtMC4yLTAuNS0wLjgtMi40LDAuMi01YzAsMCwxLjYtMC41LDUuMiwyIGMxLjUtMC40LDMuMS0wLjcsNC44LTAuN2MxLjYsMCwzLjMsMC4yLDQuNywwLjdjMy42LTIuNCw1LjItMiw1LjItMmMxLDIuNiwwLjQsNC42LDAuMiw1YzEuMiwxLjMsMiwzLDIsNS4xYzAsNy4zLTQuNSw4LjktOC43LDkuNCBjMC43LDAuNiwxLjMsMS43LDEuMywzLjVjMCwyLjYsMCw0LjYsMCw1LjJjMCwwLjUsMC40LDEuMSwxLjMsMC45YzcuNS0yLjYsMTMtOS43LDEzLTE4LjFDNTEsMjEuOSw0Mi41LDEzLjQsMzIsMTMuNHoiLz48L3N2Zz4=
[gh-rel-url]:          https://github.com/trevorsandy/lpub3d/releases/latest "LPub3D-2.4.7.75.3551_20230905"

[gh-dl-badge]:         https://img.shields.io/github/downloads/trevorsandy/lpub3d/total.svg?logo=data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSI0MCIgaGVpZ2h0PSI0MCIgdmlld0JveD0iMTIgMTIgNDAgNDAiPjxwYXRoIGZpbGw9IiNmZmYiIGQ9Ik0zMiwxMy40Yy0xMC41LDAtMTksOC41LTE5LDE5YzAsOC40LDUuNSwxNS41LDEzLDE4YzEsMC4yLDEuMy0wLjQsMS4zLTAuOWMwLTAuNSwwLTEuNywwLTMuMiBjLTUuMywxLjEtNi40LTIuNi02LjQtMi42QzIwLDQxLjYsMTguOCw0MSwxOC44LDQxYy0xLjctMS4yLDAuMS0xLjEsMC4xLTEuMWMxLjksMC4xLDIuOSwyLDIuOSwyYzEuNywyLjksNC41LDIuMSw1LjUsMS42IGMwLjItMS4yLDAuNy0yLjEsMS4yLTIuNmMtNC4yLTAuNS04LjctMi4xLTguNy05LjRjMC0yLjEsMC43LTMuNywyLTUuMWMtMC4yLTAuNS0wLjgtMi40LDAuMi01YzAsMCwxLjYtMC41LDUuMiwyIGMxLjUtMC40LDMuMS0wLjcsNC44LTAuN2MxLjYsMCwzLjMsMC4yLDQuNywwLjdjMy42LTIuNCw1LjItMiw1LjItMmMxLDIuNiwwLjQsNC42LDAuMiw1YzEuMiwxLjMsMiwzLDIsNS4xYzAsNy4zLTQuNSw4LjktOC43LDkuNCBjMC43LDAuNiwxLjMsMS43LDEuMywzLjVjMCwyLjYsMCw0LjYsMCw1LjJjMCwwLjUsMC40LDEuMSwxLjMsMC45YzcuNS0yLjYsMTMtOS43LDEzLTE4LjFDNTEsMjEuOSw0Mi41LDEzLjQsMzIsMTMuNHoiLz48L3N2Zz4=
[gh-dl-url]:           https://github.com/trevorsandy/lpub3d/releases

[gh-issue-badge]:      https://img.shields.io/github/issues/trevorsandy/lpub3d.svg?label=tickets&logo=data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSI0MCIgaGVpZ2h0PSI0MCIgdmlld0JveD0iMTIgMTIgNDAgNDAiPjxwYXRoIGZpbGw9IiNmZmYiIGQ9Ik0zMiwxMy40Yy0xMC41LDAtMTksOC41LTE5LDE5YzAsOC40LDUuNSwxNS41LDEzLDE4YzEsMC4yLDEuMy0wLjQsMS4zLTAuOWMwLTAuNSwwLTEuNywwLTMuMiBjLTUuMywxLjEtNi40LTIuNi02LjQtMi42QzIwLDQxLjYsMTguOCw0MSwxOC44LDQxYy0xLjctMS4yLDAuMS0xLjEsMC4xLTEuMWMxLjksMC4xLDIuOSwyLDIuOSwyYzEuNywyLjksNC41LDIuMSw1LjUsMS42IGMwLjItMS4yLDAuNy0yLjEsMS4yLTIuNmMtNC4yLTAuNS04LjctMi4xLTguNy05LjRjMC0yLjEsMC43LTMuNywyLTUuMWMtMC4yLTAuNS0wLjgtMi40LDAuMi01YzAsMCwxLjYtMC41LDUuMiwyIGMxLjUtMC40LDMuMS0wLjcsNC44LTAuN2MxLjYsMCwzLjMsMC4yLDQuNywwLjdjMy42LTIuNCw1LjItMiw1LjItMmMxLDIuNiwwLjQsNC42LDAuMiw1YzEuMiwxLjMsMiwzLDIsNS4xYzAsNy4zLTQuNSw4LjktOC43LDkuNCBjMC43LDAuNiwxLjMsMS43LDEuMywzLjVjMCwyLjYsMCw0LjYsMCw1LjJjMCwwLjUsMC40LDEuMSwxLjMsMC45YzcuNS0yLjYsMTMtOS43LDEzLTE4LjFDNTEsMjEuOSw0Mi41LDEzLjQsMzIsMTMuNHoiLz48L3N2Zz4=
[gh-issue-url]:        https://github.com/trevorsandy/lpub3d/issues

[gh-issue-closed-badge]: https://img.shields.io/github/issues-closed/trevorsandy/lpub3d.svg?label=tickets&color=brightgreen&logo=data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSI0MCIgaGVpZ2h0PSI0MCIgdmlld0JveD0iMTIgMTIgNDAgNDAiPjxwYXRoIGZpbGw9IiNmZmYiIGQ9Ik0zMiwxMy40Yy0xMC41LDAtMTksOC41LTE5LDE5YzAsOC40LDUuNSwxNS41LDEzLDE4YzEsMC4yLDEuMy0wLjQsMS4zLTAuOWMwLTAuNSwwLTEuNywwLTMuMiBjLTUuMywxLjEtNi40LTIuNi02LjQtMi42QzIwLDQxLjYsMTguOCw0MSwxOC44LDQxYy0xLjctMS4yLDAuMS0xLjEsMC4xLTEuMWMxLjksMC4xLDIuOSwyLDIuOSwyYzEuNywyLjksNC41LDIuMSw1LjUsMS42IGMwLjItMS4yLDAuNy0yLjEsMS4yLTIuNmMtNC4yLTAuNS04LjctMi4xLTguNy05LjRjMC0yLjEsMC43LTMuNywyLTUuMWMtMC4yLTAuNS0wLjgtMi40LDAuMi01YzAsMCwxLjYtMC41LDUuMiwyIGMxLjUtMC40LDMuMS0wLjcsNC44LTAuN2MxLjYsMCwzLjMsMC4yLDQuNywwLjdjMy42LTIuNCw1LjItMiw1LjItMmMxLDIuNiwwLjQsNC42LDAuMiw1YzEuMiwxLjMsMiwzLDIsNS4xYzAsNy4zLTQuNSw4LjktOC43LDkuNCBjMC43LDAuNiwxLjMsMS43LDEuMywzLjVjMCwyLjYsMCw0LjYsMCw1LjJjMCwwLjUsMC40LDEuMSwxLjMsMC45YzcuNS0yLjYsMTMtOS43LDEzLTE4LjFDNTEsMjEuOSw0Mi41LDEzLjQsMzIsMTMuNHoiLz48L3N2Zz4=
[gh-issue-closed-url]: https://github.com/trevorsandy/lpub3d/issues?q=is%3Aissue+is%3Aclosed

[gh-pull-req-badge]:   https://img.shields.io/github/issues-pr/trevorsandy/lpub3d.svg?logo=data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSI0MCIgaGVpZ2h0PSI0MCIgdmlld0JveD0iMTIgMTIgNDAgNDAiPjxwYXRoIGZpbGw9IiNmZmYiIGQ9Ik0zMiwxMy40Yy0xMC41LDAtMTksOC41LTE5LDE5YzAsOC40LDUuNSwxNS41LDEzLDE4YzEsMC4yLDEuMy0wLjQsMS4zLTAuOWMwLTAuNSwwLTEuNywwLTMuMiBjLTUuMywxLjEtNi40LTIuNi02LjQtMi42QzIwLDQxLjYsMTguOCw0MSwxOC44LDQxYy0xLjctMS4yLDAuMS0xLjEsMC4xLTEuMWMxLjksMC4xLDIuOSwyLDIuOSwyYzEuNywyLjksNC41LDIuMSw1LjUsMS42IGMwLjItMS4yLDAuNy0yLjEsMS4yLTIuNmMtNC4yLTAuNS04LjctMi4xLTguNy05LjRjMC0yLjEsMC43LTMuNywyLTUuMWMtMC4yLTAuNS0wLjgtMi40LDAuMi01YzAsMCwxLjYtMC41LDUuMiwyIGMxLjUtMC40LDMuMS0wLjcsNC44LTAuN2MxLjYsMCwzLjMsMC4yLDQuNywwLjdjMy42LTIuNCw1LjItMiw1LjItMmMxLDIuNiwwLjQsNC42LDAuMiw1YzEuMiwxLjMsMiwzLDIsNS4xYzAsNy4zLTQuNSw4LjktOC43LDkuNCBjMC43LDAuNiwxLjMsMS43LDEuMywzLjVjMCwyLjYsMCw0LjYsMCw1LjJjMCwwLjUsMC40LDEuMSwxLjMsMC45YzcuNS0yLjYsMTMtOS43LDEzLTE4LjFDNTEsMjEuOSw0Mi41LDEzLjQsMzIsMTMuNHoiLz48L3N2Zz4=
[gh-pull-req-url]:     https://github.com/trevorsandy/lpub3d/pulls

[gh-lst-commit-badge]: https://img.shields.io/github/last-commit/trevorsandy/lpub3d.svg?logo=data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSI0MCIgaGVpZ2h0PSI0MCIgdmlld0JveD0iMTIgMTIgNDAgNDAiPjxwYXRoIGZpbGw9IiNmZmYiIGQ9Ik0zMiwxMy40Yy0xMC41LDAtMTksOC41LTE5LDE5YzAsOC40LDUuNSwxNS41LDEzLDE4YzEsMC4yLDEuMy0wLjQsMS4zLTAuOWMwLTAuNSwwLTEuNywwLTMuMiBjLTUuMywxLjEtNi40LTIuNi02LjQtMi42QzIwLDQxLjYsMTguOCw0MSwxOC44LDQxYy0xLjctMS4yLDAuMS0xLjEsMC4xLTEuMWMxLjksMC4xLDIuOSwyLDIuOSwyYzEuNywyLjksNC41LDIuMSw1LjUsMS42IGMwLjItMS4yLDAuNy0yLjEsMS4yLTIuNmMtNC4yLTAuNS04LjctMi4xLTguNy05LjRjMC0yLjEsMC43LTMuNywyLTUuMWMtMC4yLTAuNS0wLjgtMi40LDAuMi01YzAsMCwxLjYtMC41LDUuMiwyIGMxLjUtMC40LDMuMS0wLjcsNC44LTAuN2MxLjYsMCwzLjMsMC4yLDQuNywwLjdjMy42LTIuNCw1LjItMiw1LjItMmMxLDIuNiwwLjQsNC42LDAuMiw1YzEuMiwxLjMsMiwzLDIsNS4xYzAsNy4zLTQuNSw4LjktOC43LDkuNCBjMC43LDAuNiwxLjMsMS43LDEuMywzLjVjMCwyLjYsMCw0LjYsMCw1LjJjMCwwLjUsMC40LDEuMSwxLjMsMC45YzcuNS0yLjYsMTMtOS43LDEzLTE4LjFDNTEsMjEuOSw0Mi41LDEzLjQsMzIsMTMuNHoiLz48L3N2Zz4=
[gh-lst-commit-url]:   https://github.com/trevorsandy/lpub3d/commits/master

[gh-top-lang-badge]:   https://img.shields.io/github/languages/top/trevorsandy/lpub3d.svg?logo=data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSI0MCIgaGVpZ2h0PSI0MCIgdmlld0JveD0iMTIgMTIgNDAgNDAiPjxwYXRoIGZpbGw9IiNmZmYiIGQ9Ik0zMiwxMy40Yy0xMC41LDAtMTksOC41LTE5LDE5YzAsOC40LDUuNSwxNS41LDEzLDE4YzEsMC4yLDEuMy0wLjQsMS4zLTAuOWMwLTAuNSwwLTEuNywwLTMuMiBjLTUuMywxLjEtNi40LTIuNi02LjQtMi42QzIwLDQxLjYsMTguOCw0MSwxOC44LDQxYy0xLjctMS4yLDAuMS0xLjEsMC4xLTEuMWMxLjksMC4xLDIuOSwyLDIuOSwyYzEuNywyLjksNC41LDIuMSw1LjUsMS42IGMwLjItMS4yLDAuNy0yLjEsMS4yLTIuNmMtNC4yLTAuNS04LjctMi4xLTguNy05LjRjMC0yLjEsMC43LTMuNywyLTUuMWMtMC4yLTAuNS0wLjgtMi40LDAuMi01YzAsMCwxLjYtMC41LDUuMiwyIGMxLjUtMC40LDMuMS0wLjcsNC44LTAuN2MxLjYsMCwzLjMsMC4yLDQuNywwLjdjMy42LTIuNCw1LjItMiw1LjItMmMxLDIuNiwwLjQsNC42LDAuMiw1YzEuMiwxLjMsMiwzLDIsNS4xYzAsNy4zLTQuNSw4LjktOC43LDkuNCBjMC43LDAuNiwxLjMsMS43LDEuMywzLjVjMCwyLjYsMCw0LjYsMCw1LjJjMCwwLjUsMC40LDEuMSwxLjMsMC45YzcuNS0yLjYsMTMtOS43LDEzLTE4LjFDNTEsMjEuOSw0Mi41LDEzLjQsMzIsMTMuNHoiLz48L3N2Zz4=
[gh-top-lang-url]:     https://github.com/trevorsandy/lpub3d

[gh-comm-since-badge]: https://img.shields.io/github/commits-since/trevorsandy/lpub3d/v2.4.7.svg?label=revs&nbsp;since&nbsp;v2.4.7&logo=data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSI0MCIgaGVpZ2h0PSI0MCIgdmlld0JveD0iMTIgMTIgNDAgNDAiPjxwYXRoIGZpbGw9IiNmZmYiIGQ9Ik0zMiwxMy40Yy0xMC41LDAtMTksOC41LTE5LDE5YzAsOC40LDUuNSwxNS41LDEzLDE4YzEsMC4yLDEuMy0wLjQsMS4zLTAuOWMwLTAuNSwwLTEuNywwLTMuMiBjLTUuMywxLjEtNi40LTIuNi02LjQtMi42QzIwLDQxLjYsMTguOCw0MSwxOC44LDQxYy0xLjctMS4yLDAuMS0xLjEsMC4xLTEuMWMxLjksMC4xLDIuOSwyLDIuOSwyYzEuNywyLjksNC41LDIuMSw1LjUsMS42IGMwLjItMS4yLDAuNy0yLjEsMS4yLTIuNmMtNC4yLTAuNS04LjctMi4xLTguNy05LjRjMC0yLjEsMC43LTMuNywyLTUuMWMtMC4yLTAuNS0wLjgtMi40LDAuMi01YzAsMCwxLjYtMC41LDUuMiwyIGMxLjUtMC40LDMuMS0wLjcsNC44LTAuN2MxLjYsMCwzLjMsMC4yLDQuNywwLjdjMy42LTIuNCw1LjItMiw1LjItMmMxLDIuNiwwLjQsNC42LDAuMiw1YzEuMiwxLjMsMiwzLDIsNS4xYzAsNy4zLTQuNSw4LjktOC43LDkuNCBjMC43LDAuNiwxLjMsMS43LDEuMywzLjVjMCwyLjYsMCw0LjYsMCw1LjJjMCwwLjUsMC40LDEuMSwxLjMsMC45YzcuNS0yLjYsMTMtOS43LDEzLTE4LjFDNTEsMjEuOSw0Mi41LDEzLjQsMzIsMTMuNHoiLz48L3N2Zz4=
[gh-comm-since-url]:   https://github.com/trevorsandy/lpub3d/releases/tag/continuous "Revisions since v2.4.7"

[gh-maintained-badge]: https://img.shields.io/maintenance/yes/2024.svg?logo=data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSI0MCIgaGVpZ2h0PSI0MCIgdmlld0JveD0iMTIgMTIgNDAgNDAiPjxwYXRoIGZpbGw9IiNmZmYiIGQ9Ik0zMiwxMy40Yy0xMC41LDAtMTksOC41LTE5LDE5YzAsOC40LDUuNSwxNS41LDEzLDE4YzEsMC4yLDEuMy0wLjQsMS4zLTAuOWMwLTAuNSwwLTEuNywwLTMuMiBjLTUuMywxLjEtNi40LTIuNi02LjQtMi42QzIwLDQxLjYsMTguOCw0MSwxOC44LDQxYy0xLjctMS4yLDAuMS0xLjEsMC4xLTEuMWMxLjksMC4xLDIuOSwyLDIuOSwyYzEuNywyLjksNC41LDIuMSw1LjUsMS42IGMwLjItMS4yLDAuNy0yLjEsMS4yLTIuNmMtNC4yLTAuNS04LjctMi4xLTguNy05LjRjMC0yLjEsMC43LTMuNywyLTUuMWMtMC4yLTAuNS0wLjgtMi40LDAuMi01YzAsMCwxLjYtMC41LDUuMiwyIGMxLjUtMC40LDMuMS0wLjcsNC44LTAuN2MxLjYsMCwzLjMsMC4yLDQuNywwLjdjMy42LTIuNCw1LjItMiw1LjItMmMxLDIuNiwwLjQsNC42LDAuMiw1YzEuMiwxLjMsMiwzLDIsNS4xYzAsNy4zLTQuNSw4LjktOC43LDkuNCBjMC43LDAuNiwxLjMsMS43LDEuMywzLjVjMCwyLjYsMCw0LjYsMCw1LjJjMCwwLjUsMC40LDEuMSwxLjMsMC45YzcuNS0yLjYsMTMtOS43LDEzLTE4LjFDNTEsMjEuOSw0Mi41LDEzLjQsMzIsMTMuNHoiLz48L3N2Zz4=
[gh-maintained-url]:   https://github.com/trevorsandy/lpub3d/projects/1 "Last edited 31-03-2024"

[sf-dw-badge]:         https://img.shields.io/sourceforge/dw/lpub3d.svg?logo=data:image/svg+xml;base64,PCFET0NUWVBFIHN2ZyBQVUJMSUMgIi0vL1czQy8vRFREIFNWRyAyMDAxMDkwNC8vRU4iICJodHRwOi8vd3d3LnczLm9yZy9UUi8yMDAxL1JFQy1TVkctMjAwMTA5MDQvRFREL3N2ZzEwLmR0ZCI+PHN2ZyB2ZXJzaW9uPSIxLjAiIHhtbG5zPSJodHRwOi8vd3d3LnczLm9yZy8yMDAwL3N2ZyIgd2lkdGg9IjMzMHB4IiBoZWlnaHQ9IjMzMHB4IiB2aWV3Qm94PSIwIDAgMzMwMCAzMzAwIiBwcmVzZXJ2ZUFzcGVjdFJhdGlvPSJ4TWlkWU1pZCBtZWV0Ij48ZyBpZD0ibGF5ZXIxMDEiIGZpbGw9IiNmZmYiIHN0cm9rZT0ibm9uZSI+IDxwYXRoIGQ9Ik0xNTI4IDMwMTkgYy0xMCAtNSAtMTggLTIwIC0xOCAtMzIgMCAtMTYgMTczIC0xOTUgNjA3IC02MjkgNTYyIC01NjIgNjA2IC02MDkgNjA1IC02MzkgLTEgLTI5IC00OSAtODEgLTQ4MSAtNTEzIC0zMjMgLTMyMyAtNDgxIC00ODggLTQ4MSAtNTAyIDAgLTIzIDE5OCAtMjI0IDIyMSAtMjI0IDE5IDAgMTIzOSAxMjIxIDEyMzkgMTI0MCAwIDggLTI5MSAzMDYgLTY0NyA2NjIgbC02NDggNjQ4IC0xOTAgMCBjLTExMCAwIC0xOTcgLTUgLTIwNyAtMTF6Ii8+IDxwYXRoIGQ9Ik02ODIgMjIwNiBjLTQwMSAtNDAwIC02MTMgLTYxOSAtNjExIC02MjkgNCAtMTggMTI2MiAtMTI4MiAxMjkxIC0xMjk4IDIzIC0xMyAzNzUgLTEyIDM5OSAxIDEwIDYgMTkgMjEgMTkgMzMgMCAxNSAtMTcyIDE5NCAtNjA0IDYyNyAtMzMzIDMzMyAtNjA1IDYxMiAtNjA2IDYyMCAtMiA4IC0yIDI0IC0xIDM1IDEgMTIgMTkzIDIxMiA0ODEgNTAwIDMwOCAzMDggNDgwIDQ4NyA0ODAgNTAwIDAgMjMgLTE5NyAyMjUgLTIyMCAyMjUgLTggMCAtMjkxIC0yNzYgLTYyOCAtNjE0eiIvPiA8cGF0aCBkPSJNMTU5MiAyMjM5IGMtMTM5IC0yMyAtMjY5IC0xMjMgLTMzNiAtMjYwIC00NiAtOTUgLTYwIC0xNjkgLTUyIC0yODkgMTAgLTE2MiA1MSAtMjU4IDE4NiAtNDMxIDEwOCAtMTM4IDEzOCAtMTk2IDE1MyAtMjg4IDEyIC04MyAyNiAtOTAgNzMgLTM4IDgxIDg2IDEzNyAxODYgMTc5IDMxNyA0MCAxMjYgNTUgMjE2IDY2IDQwMCA2IDkxIDE2IDE3NiAyMiAxOTAgMTggMzcgNTEgMzcgNzYgMSA0OCAtNjYgNTUgLTEwNiA1NSAtMjg0IDAgLTEwOSA0IC0xNjYgMTEgLTE2NCAxNiA1IDUzIDkxIDgwIDE4NCA5MSAzMTIgLTg3IDYyMCAtMzgxIDY2MyAtMzggNSAtNzEgOSAtNzQgOSAtMyAtMSAtMjkgLTUgLTU4IC0xMHoiLz4gPC9nPjwvc3ZnPg==
[sf-dw-badge-url]:     https://sourceforge.net/projects/lpub3d

[sf-dt-badge]:         https://img.shields.io/sourceforge/dt/lpub3d.svg?logo=data:image/svg+xml;base64,PCFET0NUWVBFIHN2ZyBQVUJMSUMgIi0vL1czQy8vRFREIFNWRyAyMDAxMDkwNC8vRU4iICJodHRwOi8vd3d3LnczLm9yZy9UUi8yMDAxL1JFQy1TVkctMjAwMTA5MDQvRFREL3N2ZzEwLmR0ZCI+PHN2ZyB2ZXJzaW9uPSIxLjAiIHhtbG5zPSJodHRwOi8vd3d3LnczLm9yZy8yMDAwL3N2ZyIgd2lkdGg9IjMzMHB4IiBoZWlnaHQ9IjMzMHB4IiB2aWV3Qm94PSIwIDAgMzMwMCAzMzAwIiBwcmVzZXJ2ZUFzcGVjdFJhdGlvPSJ4TWlkWU1pZCBtZWV0Ij48ZyBpZD0ibGF5ZXIxMDEiIGZpbGw9IiNmZmYiIHN0cm9rZT0ibm9uZSI+IDxwYXRoIGQ9Ik0xNTI4IDMwMTkgYy0xMCAtNSAtMTggLTIwIC0xOCAtMzIgMCAtMTYgMTczIC0xOTUgNjA3IC02MjkgNTYyIC01NjIgNjA2IC02MDkgNjA1IC02MzkgLTEgLTI5IC00OSAtODEgLTQ4MSAtNTEzIC0zMjMgLTMyMyAtNDgxIC00ODggLTQ4MSAtNTAyIDAgLTIzIDE5OCAtMjI0IDIyMSAtMjI0IDE5IDAgMTIzOSAxMjIxIDEyMzkgMTI0MCAwIDggLTI5MSAzMDYgLTY0NyA2NjIgbC02NDggNjQ4IC0xOTAgMCBjLTExMCAwIC0xOTcgLTUgLTIwNyAtMTF6Ii8+IDxwYXRoIGQ9Ik02ODIgMjIwNiBjLTQwMSAtNDAwIC02MTMgLTYxOSAtNjExIC02MjkgNCAtMTggMTI2MiAtMTI4MiAxMjkxIC0xMjk4IDIzIC0xMyAzNzUgLTEyIDM5OSAxIDEwIDYgMTkgMjEgMTkgMzMgMCAxNSAtMTcyIDE5NCAtNjA0IDYyNyAtMzMzIDMzMyAtNjA1IDYxMiAtNjA2IDYyMCAtMiA4IC0yIDI0IC0xIDM1IDEgMTIgMTkzIDIxMiA0ODEgNTAwIDMwOCAzMDggNDgwIDQ4NyA0ODAgNTAwIDAgMjMgLTE5NyAyMjUgLTIyMCAyMjUgLTggMCAtMjkxIC0yNzYgLTYyOCAtNjE0eiIvPiA8cGF0aCBkPSJNMTU5MiAyMjM5IGMtMTM5IC0yMyAtMjY5IC0xMjMgLTMzNiAtMjYwIC00NiAtOTUgLTYwIC0xNjkgLTUyIC0yODkgMTAgLTE2MiA1MSAtMjU4IDE4NiAtNDMxIDEwOCAtMTM4IDEzOCAtMTk2IDE1MyAtMjg4IDEyIC04MyAyNiAtOTAgNzMgLTM4IDgxIDg2IDEzNyAxODYgMTc5IDMxNyA0MCAxMjYgNTUgMjE2IDY2IDQwMCA2IDkxIDE2IDE3NiAyMiAxOTAgMTggMzcgNTEgMzcgNzYgMSA0OCAtNjYgNTUgLTEwNiA1NSAtMjg0IDAgLTEwOSA0IC0xNjYgMTEgLTE2NCAxNiA1IDUzIDkxIDgwIDE4NCA5MSAzMTIgLTg3IDYyMCAtMzgxIDY2MyAtMzggNSAtNzEgOSAtNzQgOSAtMyAtMSAtMjkgLTUgLTU4IC0xMHoiLz4gPC9nPjwvc3ZnPg==
[sf-dt-badge-url]:     https://sourceforge.net/projects/lpub3d

[appimage-info-url]:   https://appimage.org/
[obs-url]:             https://build.opensuse.org/package/show/home:trevorsandy/lpub3d
[dockerinstall]:       https://www.docker.com/get-docker
[dockercomposefile]:   https://github.com/trevorsandy/lpub3d/blob/master/builds/linux/docker-compose/docker-compose-cibuild-linux.yml
[dockerarch]:          https://github.com/trevorsandy/lpub3d/blob/master/builds/linux/docker-compose/dockerfiles/Dockerfile-cibuild-archlinux
[dockerubuntu]:        https://github.com/trevorsandy/lpub3d/blob/master/builds/linux/docker-compose/dockerfiles/Dockerfile-cibuild-ubuntu
[dockerfedora]:        https://github.com/trevorsandy/lpub3d/blob/master/builds/linux/docker-compose/dockerfiles/Dockerfile-cibuild-fedora

Copyright (c) 2015 - 2024 by Trevor SANDY
