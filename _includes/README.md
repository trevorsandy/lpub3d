[![GitHub release][gh-rel-badge]][gh-rel-url]                        |[![GitHub commits (since latest release)][gh-comm-since-badge]][gh-comm-since-url]|[![GitHub last commit][gh-lst-commit-badge]][gh-lst-commit-url]
:--------------------------------------------------------------------|:---------------------------------------------------------------------------------|:---------------------------------------------------------------------------
[![SourceForge weekly downloads][sf-dw-badge]][sf-dw-badge-url]      |[![SourceForge total downloads][sf-dt-badge]][sf-dt-badge-url]                    |[![GitHub total downloads][gh-dl-badge]][gh-dl-url]
[![GitHub issues closed][gh-issue-closed-badge]][gh-issue-closed-url]|[![GitHub issues][gh-issue-badge]][gh-issue-url]                                  |[![GitHub pull requests][gh-pull-req-badge]][gh-pull-req-url]
[![Travis-CI build status][travis-badge]][travis-url]                |[![AppVeyor build status][appveyor-badge]][appveyor-url]                          |[![Maintained status][gh-maintained-badge]][gh-maintained-url]

**LPub3D** is an Open Source WYSIWYG editing application for creating LEGO® style digital building instructions producing PDF, PNG, JPG and BMP formatted content.
It uses the LDraw parts library and reads the LDraw DAT, LDR and MPD model file formats.

LPub3D supports standard and high definition **part libraries** and do not require the LDraw library to be preinstalled. Archive (.zip) packages for both official and unofficial parts are bundled with LPub3D. Follow the linked image below to acquire the LGEO part library.

:-----------------|:------------------------------------:|:--------------------
[LDraw][ldraw-url]|[![LDraw-Logo][ldraw-logo]][ldraw-url]|LDraw™ ([LDraw.org][ldraw-url]) parts is an open standard library that allow users to create virtual models and scenes. Three LDraw part libraries are bundled with LPub3D.<br>![LEGO][lego-logo] LDraw.org official and unofficial LEGO® part library.<br>![TENTE][tente-logo] TENTE® unofficial LDraw part library created by (Jasolo). <br>![VEXIQ][vexiq-logo] VEX IQ® unofficial LDraw library created by Philippe Hurbain (Philo).<br>![LSynth][lsynth-logo] [LSynth][lsynth-url] bendable parts synthesis elements created by Don Heyse (deeice).
[LGEO][lgeo-url]  |[![LGEO-Logo][lgeo-logo]][lgeo-url]   |POV-Ray high-fedility parts library created by Lutz Uhlmann (El Lutzo), updated by Owen Burgoyne (C3POwen) and extended with STL object files by Damien Roux (Darats).

LPub3D provides "native" imbedded rendering for image and POV file generation plus three popular stand-alone **LDraw image renderers** - console applications custom adapted for tight integration, optimum configurability and robust performance.

:---------------------|:------------------------------------------:|:----------
[LeoCAD][leocad-url]  |[![LeoCAD-logo][leocad-logo]][leocad-url]   |LPub3D "native" image renderer - performant, "true fade" image rendering
[LDGLite][ldglite-url]|[![LDGLite-Logo][ldglite-logo]][ldglite-url]|fast image rendering ideal for proofing
[LDView][ldview-url]  |[![LDView-Logo][ldview-logo]][ldview-url]   |high quality image rendering plus 3rd-party and "native" POV file generation
[POVRay][povray-url]  |[![POVRay-Logo][povray-logo]][povray-url]   |LPub3D-Trace ([POV-Ray][povray-url]) - photorealistic, feature rich image rendering

### Screenshots

{% include slider.html selector="slider1" %}

### Compatibility

:-------------------:|:-------------------:|:---------------------:
![X86-Logo][x86-logo]|![ARM-Logo][arm-logo]|![RaspberryPi-Logo][raspberrypi-logo]

LPub3D runs on Debian, Red Hat and Arch derived Linux distros, macOS and Windows Operating Systems.
LPub3D is also available as a Linux 'no-install', multi-platform [AppImage][appimage-info-url]. Builds are available for both x86 instruction set (32-bit and 64-bit) and Advanced RISC Machines (ARM) micorprocessors.

### Downloads

:---------------------------------------------|:-------------------------------
[![GitHub-Logo][github-logo]][github_releases]|[GitHub Releases][githubreleases]
[![SF-Logo][sf-logo]][sf_releases]            |[Sourceforge Releases][sfreleases]
[![OBS-Logo][obs-logo]][obs_releases]         |[Open Build Service Releases][obsreleases] - Linux distributions

### Distributions

[![Appimage-Icon][appimage-icon]][appimage-url]|[![Arch-Icon][arch-icon]][arch-url]|[![Centos-Icon][centos-icon]][centos-url]|[![Debian-Icon][debian-icon]][debian-url]|[![Fedora-Icon][fedora-icon]][fedora-url]|[![Mageia-Icon][mageia-icon]][mageia-url]            |[![Macos-Icon][macos-icon]][macos-url]
:---------------------------------------------:|:---------------------------------:|:---------------------------------------:|:---------------------------------------:|:---------------------------------------:|:---------------------------------------------------:|:------------------------------------------:
[![Opensuse-Icon][opensuse-icon]][opensuse-url]|[![Rhel-Icon][rhel-icon]][rhel-url]|[![Sl-Icon][sl-icon]][sl-url]            |[![Sle-Icon][sle-icon]][sle-url]         |[![Ubuntu-Icon][ubuntu-icon]][ubuntu-url]|[![Univention-Icon][univention-icon]][univention-url]|[![Windows-Icon][windows-icon]][windows-url]

 - Click icon above to download distribution asset.

### Quick Links
 - [Build and package LPub3D from source][buildfromsource]
 - [Release notes <span style="color:red">(NEW!)</span>][docs-lpub3d-release-notes]
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

### Features
 - Show submodel image at first step [#52](https://github.com/trevorsandy/lpub3d/issues/52) <span style="color:red">(NEW!)</span>
 - Pre-bundled LSynth bendable part elements. Add to archive and, optionally, extract to LDraw folder [#165](https://github.com/trevorsandy/lpub3d/issues/165) <span style="color:red">(NEW!)</span>
 - Pre-bundled and fully integrated LDraw part libraries for LEGO, TENTE and VEXIQ models [#132](https://github.com/trevorsandy/lpub3d/issues/132) <span style="color:red">(NEW!)</span>
 - Continuous page processing - step through your model with one click [#76](https://github.com/trevorsandy/lpub3d/issues/76) <span style="color:red">(NEW!)</span>
 - Go Dark - Dark mode theme, switch on the fly [#73](https://github.com/trevorsandy/lpub3d/issues/73) <span style="color:red">(NEW!)</span>
 - Multi-segment page, divider and callout pointers [#67](https://github.com/trevorsandy/lpub3d/issues/67) <span style="color:red">(NEW!)</span>
 - Enhanced “True” previous step part fade [#47](https://github.com/trevorsandy/lpub3d/issues/47) <span style="color:red">(NEW!)</span>
 - Current step silhouette part highlight [#47](https://github.com/trevorsandy/lpub3d/issues/47) <span style="color:red">(NEW!)</span>
 - Scene horizontal and vertical ruler and axis guides [#68](https://github.com/trevorsandy/lpub3d/issues/68) <span style="color:red">(NEW!)</span>
 - Console commands - enabling "Batch" mode [#12](https://github.com/trevorsandy/lpub3d/issues/12) <span style="color:red">(NEW!)</span>
 - "Headless" mode on Linux, macOS and Windows platforms [#56](https://github.com/trevorsandy/lpub3d/issues/56) <span style="color:red">(NEW!)</span>
 - Best performance with "Native" image rendering [#71](https://github.com/trevorsandy/lpub3d/issues/71) and POV file generation [#77](https://github.com/trevorsandy/lpub3d/issues/77) <span style="color:red">(NEW!)</span>
 - Integrated 3D viewer and image renderer powered by [LeoCAD][leocad-url]
 - Integrated and custom adapted image renderers ([LDGlite][ldglite-url], [LDView][ldview-url] and LPub3D-Trace ([POV-Ray][povray-url])) - no more additional installs
 - Renderer config files accessible from the LPub3D user interface
 - Custom LDraw library LDConfig (part material) file can be passed to LDGlite and LDView renderers
 - Set ROTSTEP (step rotation) meta using 3D Viewer
 - Fade previous step's parts (Fadestep)
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
 - Automated build and deploy from Cloud CI Service: [Travis-CI][travis-url], [AppVeyor][appveyor-url] and [Open Build Service][obs-url]
 - [Docker][dockerinstall] container builds using [Docker Compose][dockercomposefile] available for [Arch][dockerarch], [Xenial][dockerubuntu] and [Fedora 26][dockerfedora] Linux distros

 Portions of LPub3D are based on LPUB© 2007-2009 Kevin Clague, LeoCAD© 2018 Leonardo Zide and additional third party components. See [Credits][docs-lpub3d-credits] for full details on contributing content.

[docs-lpub3d-release-notes]:  {{ '/assets/docs/lpub3d/RELEASE_NOTES.html' | relative_url }} "Review LPub3D release features, enhancements, fixes and changes"
[docs-lpub3d-changelog]:      {{ '/assets/docs/lpub3d/README.txt' | relative_url }} "Review LPub3D changes and enhancements (Deprecated)"
[docs-lpub3d-credits]:        {{ '/assets/docs/lpub3d/CREDITS.txt' | relative_url }} "See whose content has contributed to LPub3D"
[docs-lpub3d-copying]:        {{ '/assets/docs/lpub3d/COPYING.txt' | relative_url }} "LPub3D open source license document"

[docs-lpub3d-meta]:           {{ '/assets/docs/lpub3d/metacommands.html' | relative_url }}
[docs-lpub3d-macos-libs]:     {{ '/assets/docs/lpub3d/macOS_libs.html' | relative_url }}
[docs-ldview-help]:           {{ '/assets/docs/ldview/Help.html' | relative_url }}
[docs-ldglite-readme]:        {{ '/assets/docs/ldglite/README.TXT' | relative_url }}
[docs-povray-help]:           {{ '/assets/docs/povray/index.html' | relative_url }}

[leocad-logo]:                {{ '/assets/images/leocad_logo.png' | relative_url }}
[ldview-logo]:                {{ '/assets/images/ldview_logo.png' | relative_url }}
[ldglite-logo]:               {{ '/assets/images/ldglite_logo.png' | relative_url }}
[povray-logo]:                {{ '/assets/images/povray_logo.png' | relative_url }}

[leocad-url]:          https://github.com/trevorsandy/lpub3d/tree/master/lclib
[ldglite-url]:         https://github.com/trevorsandy/ldglite
[ldview-url]:          https://github.com/trevorsandy/ldview/tree/qmake-build
[povray-url]:          https://github.com/trevorsandy/povray/tree/lpub3d/raytracer-cui
[lsynth-url]:          https://github.com/deeice/lsynth

[github-logo]:         {{ '/assets/images/github_logo.png' | relative_url }}
[sf-logo]:             {{ '/assets/images/sf_logo.png' | relative_url }}
[obs-logo]:            {{ '/assets/images/obs_logo.png' | relative_url }}

[ldraw-logo]:          {{ '/assets/images/ldraw_logo.png' | relative_url }}
[lego-logo]:           {{ '/assets/images/lego_logo.png' | relative_url }}
[tente-logo]:          {{ '/assets/images/tente_logo.png' | relative_url }}
[vexiq-logo]:          {{ '/assets/images/vexiq_logo.png' | relative_url }}
[lsynth-logo]:         {{ '/assets/images/lsynth_logo.png' | relative_url }}
[lgeo-logo]:           {{ '/assets/images/lgeo_logo.png' | relative_url }}
[gplv3-logo]:          {{ '/assets/images/gplv3_logo.png' | relative_url }}

[arm-logo]:            {{ '/assets/images/arm_logo.png' | relative_url }}
[x86-logo]:            {{ '/assets/images/x86_logo.png' | relative_url }}
[raspberrypi-logo]:    {{ '/assets/images/raspberrypi_logo.png' | relative_url }}

[sf-gen-discussion]:   https://sourceforge.net/p/lpub3d/discussion/general "Ask a question or review past discussions"
[gh-new-issue]:        https://github.com/trevorsandy/lpub3d/issues/new "Report an issue with LPub3D"
[working-with-lpub3d]: https://sites.google.com/site/workingwithlpub/lpub3d
[jctchang-lpub3d]:     http://jc-tchang.philohome.com/model/LPub3D.htm

[ldraw-url]:           http://www.ldraw.org/
[lgeo-url]:            https://www.eurobricks.com/forum/index.php?/forums/topic/108739-new-parts-for-lgeo-library/

[appimage-icon]:       https://raw.githubusercontent.com/trevorsandy/lpub3d/master/builds/utilities/icons/appimage.png "AppImage Ubuntu Trusty"
[macos-icon]:          https://raw.githubusercontent.com/trevorsandy/lpub3d/master/builds/utilities/icons/macos.png "macOS OSX 10.13.3"
[windows-icon]:        https://raw.githubusercontent.com/trevorsandy/lpub3d/master/builds/utilities/icons/windows.png "Windows 10"

[windows-url]:         https://github.com/trevorsandy/lpub3d/releases/download/v2.3.4/LPub3D-2.3.4.0.1085_20181210.exe
[macos-url]:           https://github.com/trevorsandy/lpub3d/releases/download/v2.3.4/LPub3D-2.3.4.0.1085_20181210-macos.dmg
[appimage-url]:        https://github.com/trevorsandy/lpub3d/releases/download/v2.3.4/LPub3D-2.3.4.0.1085_20181210-x86_64.AppImage

[arch-icon]:           https://raw.githubusercontent.com/trevorsandy/lpub3d/master/builds/utilities/icons/arch.png "Arch"
[centos-icon]:         https://raw.githubusercontent.com/trevorsandy/lpub3d/master/builds/utilities/icons/centos.png "CentOS 7"
[debian-icon]:         https://raw.githubusercontent.com/trevorsandy/lpub3d/master/builds/utilities/icons/debian.png "Debian 9"
[fedora-icon]:         https://raw.githubusercontent.com/trevorsandy/lpub3d/master/builds/utilities/icons/fedora.png "Fedora 27"
[mageia-icon]:         https://raw.githubusercontent.com/trevorsandy/lpub3d/master/builds/utilities/icons/mageia.png "Mageia 6"
[opensuse-icon]:       https://raw.githubusercontent.com/trevorsandy/lpub3d/master/builds/utilities/icons/opensuse.png "openSUSE Leap 42.3"
[rhel-icon]:           https://raw.githubusercontent.com/trevorsandy/lpub3d/master/builds/utilities/icons/rhel.png "Red Hat Enterprise Linux 7"
[sl-icon]:             https://raw.githubusercontent.com/trevorsandy/lpub3d/master/builds/utilities/icons/sl.png "Scientific Linux 7"
[sle-icon]:            https://raw.githubusercontent.com/trevorsandy/lpub3d/master/builds/utilities/icons/sle.png "SUSE Enterprise Linux 12 SP3"
[ubuntu-icon]:         https://raw.githubusercontent.com/trevorsandy/lpub3d/master/builds/utilities/icons/ubuntu.png "Ubuntu 18.10"
[univention-icon]:     https://raw.githubusercontent.com/trevorsandy/lpub3d/master/builds/utilities/icons/univention.png "Univention 4.2"

[arch-url]:            https://download.opensuse.org/repositories/home:/trevorsandy/Arch_Extra/
[centos-url]:          https://download.opensuse.org/repositories/home:/trevorsandy/CentOS_7/
[debian-url]:          https://download.opensuse.org/repositories/home:/trevorsandy/Debian_9.0/
[fedora-url]:          https://download.opensuse.org/repositories/home:/trevorsandy/Fedora_27/
[mageia-url]:          https://download.opensuse.org/repositories/home:/trevorsandy/Mageia_6/
[opensuse-url]:        https://download.opensuse.org/repositories/home:/trevorsandy/openSUSE_Leap_42.3/
[rhel-url]:            https://download.opensuse.org/repositories/home:/trevorsandy/RHEL_7/
[sl-url]:              https://download.opensuse.org/repositories/home:/trevorsandy/ScientificLinux_7/
[sle-url]:             https://download.opensuse.org/repositories/home:/trevorsandy/SLE_12_SP3/
[ubuntu-url]:          https://download.opensuse.org/repositories/home:/trevorsandy/xUbuntu_18.10/
[univention-url]:      https://download.opensuse.org/repositories/home:/trevorsandy/Univention_4.2/

[lgplv3-url]:          https://www.gnu.org/licenses/gpl-3.0.en.html
[buildfromsource]:     https://github.com/trevorsandy/lpub3d/blob/master/builds/utilities/README.md

[sfreleases]:          https://sourceforge.net/projects/lpub3d/files/2.3.4
[githubreleases]:      https://github.com/trevorsandy/lpub3d/releases/tag/v2.3.4
[obsreleases]:         https://software.opensuse.org/download.html?project=home:trevorsandy&package=lpub3d

[sf_releases]:         https://sourceforge.net/projects/lpub3d/files
[github_releases]:     https://github.com/trevorsandy/lpub3d/releases
[obs_releases]:        https://download.opensuse.org/repositories/home:/trevorsandy

[travis-badge]:        https://img.shields.io/travis/trevorsandy/lpub3d.svg?label=travis&logo=travis
[travis-url]:          https://travis-ci.org/trevorsandy/lpub3d

[appveyor-badge]:      https://img.shields.io/appveyor/ci/trevorsandy/lpub3d.svg?label=appveyor&logo=appveyor
[appveyor-url]:        https://ci.appveyor.com/project/trevorsandy/lpub3d

[gh-rel-badge]:        https://img.shields.io/github/release/trevorsandy/lpub3d.svg?logo=data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSI0MCIgaGVpZ2h0PSI0MCIgdmlld0JveD0iMTIgMTIgNDAgNDAiPjxwYXRoIGZpbGw9IiNmZmYiIGQ9Ik0zMiwxMy40Yy0xMC41LDAtMTksOC41LTE5LDE5YzAsOC40LDUuNSwxNS41LDEzLDE4YzEsMC4yLDEuMy0wLjQsMS4zLTAuOWMwLTAuNSwwLTEuNywwLTMuMiBjLTUuMywxLjEtNi40LTIuNi02LjQtMi42QzIwLDQxLjYsMTguOCw0MSwxOC44LDQxYy0xLjctMS4yLDAuMS0xLjEsMC4xLTEuMWMxLjksMC4xLDIuOSwyLDIuOSwyYzEuNywyLjksNC41LDIuMSw1LjUsMS42IGMwLjItMS4yLDAuNy0yLjEsMS4yLTIuNmMtNC4yLTAuNS04LjctMi4xLTguNy05LjRjMC0yLjEsMC43LTMuNywyLTUuMWMtMC4yLTAuNS0wLjgtMi40LDAuMi01YzAsMCwxLjYtMC41LDUuMiwyIGMxLjUtMC40LDMuMS0wLjcsNC44LTAuN2MxLjYsMCwzLjMsMC4yLDQuNywwLjdjMy42LTIuNCw1LjItMiw1LjItMmMxLDIuNiwwLjQsNC42LDAuMiw1YzEuMiwxLjMsMiwzLDIsNS4xYzAsNy4zLTQuNSw4LjktOC43LDkuNCBjMC43LDAuNiwxLjMsMS43LDEuMywzLjVjMCwyLjYsMCw0LjYsMCw1LjJjMCwwLjUsMC40LDEuMSwxLjMsMC45YzcuNS0yLjYsMTMtOS43LDEzLTE4LjFDNTEsMjEuOSw0Mi41LDEzLjQsMzIsMTMuNHoiLz48L3N2Zz4=
[gh-rel-url]:          https://github.com/trevorsandy/lpub3d/releases/latest

[gh-dl-badge]:         https://img.shields.io/github/downloads/trevorsandy/lpub3d/total.svg?logo=data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSI0MCIgaGVpZ2h0PSI0MCIgdmlld0JveD0iMTIgMTIgNDAgNDAiPjxwYXRoIGZpbGw9IiNmZmYiIGQ9Ik0zMiwxMy40Yy0xMC41LDAtMTksOC41LTE5LDE5YzAsOC40LDUuNSwxNS41LDEzLDE4YzEsMC4yLDEuMy0wLjQsMS4zLTAuOWMwLTAuNSwwLTEuNywwLTMuMiBjLTUuMywxLjEtNi40LTIuNi02LjQtMi42QzIwLDQxLjYsMTguOCw0MSwxOC44LDQxYy0xLjctMS4yLDAuMS0xLjEsMC4xLTEuMWMxLjksMC4xLDIuOSwyLDIuOSwyYzEuNywyLjksNC41LDIuMSw1LjUsMS42IGMwLjItMS4yLDAuNy0yLjEsMS4yLTIuNmMtNC4yLTAuNS04LjctMi4xLTguNy05LjRjMC0yLjEsMC43LTMuNywyLTUuMWMtMC4yLTAuNS0wLjgtMi40LDAuMi01YzAsMCwxLjYtMC41LDUuMiwyIGMxLjUtMC40LDMuMS0wLjcsNC44LTAuN2MxLjYsMCwzLjMsMC4yLDQuNywwLjdjMy42LTIuNCw1LjItMiw1LjItMmMxLDIuNiwwLjQsNC42LDAuMiw1YzEuMiwxLjMsMiwzLDIsNS4xYzAsNy4zLTQuNSw4LjktOC43LDkuNCBjMC43LDAuNiwxLjMsMS43LDEuMywzLjVjMCwyLjYsMCw0LjYsMCw1LjJjMCwwLjUsMC40LDEuMSwxLjMsMC45YzcuNS0yLjYsMTMtOS43LDEzLTE4LjFDNTEsMjEuOSw0Mi41LDEzLjQsMzIsMTMuNHoiLz48L3N2Zz4=
[gh-dl-url]:           https://github.com/trevorsandy/lpub3d/releases/latest

[gh-issue-badge]:      https://img.shields.io/github/issues/trevorsandy/lpub3d.svg?label=tickets&logo=data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSI0MCIgaGVpZ2h0PSI0MCIgdmlld0JveD0iMTIgMTIgNDAgNDAiPjxwYXRoIGZpbGw9IiNmZmYiIGQ9Ik0zMiwxMy40Yy0xMC41LDAtMTksOC41LTE5LDE5YzAsOC40LDUuNSwxNS41LDEzLDE4YzEsMC4yLDEuMy0wLjQsMS4zLTAuOWMwLTAuNSwwLTEuNywwLTMuMiBjLTUuMywxLjEtNi40LTIuNi02LjQtMi42QzIwLDQxLjYsMTguOCw0MSwxOC44LDQxYy0xLjctMS4yLDAuMS0xLjEsMC4xLTEuMWMxLjksMC4xLDIuOSwyLDIuOSwyYzEuNywyLjksNC41LDIuMSw1LjUsMS42IGMwLjItMS4yLDAuNy0yLjEsMS4yLTIuNmMtNC4yLTAuNS04LjctMi4xLTguNy05LjRjMC0yLjEsMC43LTMuNywyLTUuMWMtMC4yLTAuNS0wLjgtMi40LDAuMi01YzAsMCwxLjYtMC41LDUuMiwyIGMxLjUtMC40LDMuMS0wLjcsNC44LTAuN2MxLjYsMCwzLjMsMC4yLDQuNywwLjdjMy42LTIuNCw1LjItMiw1LjItMmMxLDIuNiwwLjQsNC42LDAuMiw1YzEuMiwxLjMsMiwzLDIsNS4xYzAsNy4zLTQuNSw4LjktOC43LDkuNCBjMC43LDAuNiwxLjMsMS43LDEuMywzLjVjMCwyLjYsMCw0LjYsMCw1LjJjMCwwLjUsMC40LDEuMSwxLjMsMC45YzcuNS0yLjYsMTMtOS43LDEzLTE4LjFDNTEsMjEuOSw0Mi41LDEzLjQsMzIsMTMuNHoiLz48L3N2Zz4=
[gh-issue-url]:        https://github.com/trevorsandy/lpub3d/issues

[gh-issue-closed-badge]: https://img.shields.io/github/issues-closed/trevorsandy/lpub3d.svg?label=tickets&logo=data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSI0MCIgaGVpZ2h0PSI0MCIgdmlld0JveD0iMTIgMTIgNDAgNDAiPjxwYXRoIGZpbGw9IiNmZmYiIGQ9Ik0zMiwxMy40Yy0xMC41LDAtMTksOC41LTE5LDE5YzAsOC40LDUuNSwxNS41LDEzLDE4YzEsMC4yLDEuMy0wLjQsMS4zLTAuOWMwLTAuNSwwLTEuNywwLTMuMiBjLTUuMywxLjEtNi40LTIuNi02LjQtMi42QzIwLDQxLjYsMTguOCw0MSwxOC44LDQxYy0xLjctMS4yLDAuMS0xLjEsMC4xLTEuMWMxLjksMC4xLDIuOSwyLDIuOSwyYzEuNywyLjksNC41LDIuMSw1LjUsMS42IGMwLjItMS4yLDAuNy0yLjEsMS4yLTIuNmMtNC4yLTAuNS04LjctMi4xLTguNy05LjRjMC0yLjEsMC43LTMuNywyLTUuMWMtMC4yLTAuNS0wLjgtMi40LDAuMi01YzAsMCwxLjYtMC41LDUuMiwyIGMxLjUtMC40LDMuMS0wLjcsNC44LTAuN2MxLjYsMCwzLjMsMC4yLDQuNywwLjdjMy42LTIuNCw1LjItMiw1LjItMmMxLDIuNiwwLjQsNC42LDAuMiw1YzEuMiwxLjMsMiwzLDIsNS4xYzAsNy4zLTQuNSw4LjktOC43LDkuNCBjMC43LDAuNiwxLjMsMS43LDEuMywzLjVjMCwyLjYsMCw0LjYsMCw1LjJjMCwwLjUsMC40LDEuMSwxLjMsMC45YzcuNS0yLjYsMTMtOS43LDEzLTE4LjFDNTEsMjEuOSw0Mi41LDEzLjQsMzIsMTMuNHoiLz48L3N2Zz4=
[gh-issue-closed-url]: https://github.com/trevorsandy/lpub3d/issues?q=is%3Aissue+is%3Aclosed

[gh-pull-req-badge]:   https://img.shields.io/github/issues-pr/trevorsandy/lpub3d.svg?logo=data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSI0MCIgaGVpZ2h0PSI0MCIgdmlld0JveD0iMTIgMTIgNDAgNDAiPjxwYXRoIGZpbGw9IiNmZmYiIGQ9Ik0zMiwxMy40Yy0xMC41LDAtMTksOC41LTE5LDE5YzAsOC40LDUuNSwxNS41LDEzLDE4YzEsMC4yLDEuMy0wLjQsMS4zLTAuOWMwLTAuNSwwLTEuNywwLTMuMiBjLTUuMywxLjEtNi40LTIuNi02LjQtMi42QzIwLDQxLjYsMTguOCw0MSwxOC44LDQxYy0xLjctMS4yLDAuMS0xLjEsMC4xLTEuMWMxLjksMC4xLDIuOSwyLDIuOSwyYzEuNywyLjksNC41LDIuMSw1LjUsMS42IGMwLjItMS4yLDAuNy0yLjEsMS4yLTIuNmMtNC4yLTAuNS04LjctMi4xLTguNy05LjRjMC0yLjEsMC43LTMuNywyLTUuMWMtMC4yLTAuNS0wLjgtMi40LDAuMi01YzAsMCwxLjYtMC41LDUuMiwyIGMxLjUtMC40LDMuMS0wLjcsNC44LTAuN2MxLjYsMCwzLjMsMC4yLDQuNywwLjdjMy42LTIuNCw1LjItMiw1LjItMmMxLDIuNiwwLjQsNC42LDAuMiw1YzEuMiwxLjMsMiwzLDIsNS4xYzAsNy4zLTQuNSw4LjktOC43LDkuNCBjMC43LDAuNiwxLjMsMS43LDEuMywzLjVjMCwyLjYsMCw0LjYsMCw1LjJjMCwwLjUsMC40LDEuMSwxLjMsMC45YzcuNS0yLjYsMTMtOS43LDEzLTE4LjFDNTEsMjEuOSw0Mi41LDEzLjQsMzIsMTMuNHoiLz48L3N2Zz4=
[gh-pull-req-url]:     https://github.com/trevorsandy/lpub3d/pulls

[gh-lst-commit-badge]: https://img.shields.io/github/last-commit/trevorsandy/lpub3d.svg?logo=data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSI0MCIgaGVpZ2h0PSI0MCIgdmlld0JveD0iMTIgMTIgNDAgNDAiPjxwYXRoIGZpbGw9IiNmZmYiIGQ9Ik0zMiwxMy40Yy0xMC41LDAtMTksOC41LTE5LDE5YzAsOC40LDUuNSwxNS41LDEzLDE4YzEsMC4yLDEuMy0wLjQsMS4zLTAuOWMwLTAuNSwwLTEuNywwLTMuMiBjLTUuMywxLjEtNi40LTIuNi02LjQtMi42QzIwLDQxLjYsMTguOCw0MSwxOC44LDQxYy0xLjctMS4yLDAuMS0xLjEsMC4xLTEuMWMxLjksMC4xLDIuOSwyLDIuOSwyYzEuNywyLjksNC41LDIuMSw1LjUsMS42IGMwLjItMS4yLDAuNy0yLjEsMS4yLTIuNmMtNC4yLTAuNS04LjctMi4xLTguNy05LjRjMC0yLjEsMC43LTMuNywyLTUuMWMtMC4yLTAuNS0wLjgtMi40LDAuMi01YzAsMCwxLjYtMC41LDUuMiwyIGMxLjUtMC40LDMuMS0wLjcsNC44LTAuN2MxLjYsMCwzLjMsMC4yLDQuNywwLjdjMy42LTIuNCw1LjItMiw1LjItMmMxLDIuNiwwLjQsNC42LDAuMiw1YzEuMiwxLjMsMiwzLDIsNS4xYzAsNy4zLTQuNSw4LjktOC43LDkuNCBjMC43LDAuNiwxLjMsMS43LDEuMywzLjVjMCwyLjYsMCw0LjYsMCw1LjJjMCwwLjUsMC40LDEuMSwxLjMsMC45YzcuNS0yLjYsMTMtOS43LDEzLTE4LjFDNTEsMjEuOSw0Mi41LDEzLjQsMzIsMTMuNHoiLz48L3N2Zz4=
[gh-lst-commit-url]:   https://github.com/trevorsandy/lpub3d/commits/master

[gh-top-lang-badge]:   https://img.shields.io/github/languages/top/trevorsandy/lpub3d.svg?logo=data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSI0MCIgaGVpZ2h0PSI0MCIgdmlld0JveD0iMTIgMTIgNDAgNDAiPjxwYXRoIGZpbGw9IiNmZmYiIGQ9Ik0zMiwxMy40Yy0xMC41LDAtMTksOC41LTE5LDE5YzAsOC40LDUuNSwxNS41LDEzLDE4YzEsMC4yLDEuMy0wLjQsMS4zLTAuOWMwLTAuNSwwLTEuNywwLTMuMiBjLTUuMywxLjEtNi40LTIuNi02LjQtMi42QzIwLDQxLjYsMTguOCw0MSwxOC44LDQxYy0xLjctMS4yLDAuMS0xLjEsMC4xLTEuMWMxLjksMC4xLDIuOSwyLDIuOSwyYzEuNywyLjksNC41LDIuMSw1LjUsMS42IGMwLjItMS4yLDAuNy0yLjEsMS4yLTIuNmMtNC4yLTAuNS04LjctMi4xLTguNy05LjRjMC0yLjEsMC43LTMuNywyLTUuMWMtMC4yLTAuNS0wLjgtMi40LDAuMi01YzAsMCwxLjYtMC41LDUuMiwyIGMxLjUtMC40LDMuMS0wLjcsNC44LTAuN2MxLjYsMCwzLjMsMC4yLDQuNywwLjdjMy42LTIuNCw1LjItMiw1LjItMmMxLDIuNiwwLjQsNC42LDAuMiw1YzEuMiwxLjMsMiwzLDIsNS4xYzAsNy4zLTQuNSw4LjktOC43LDkuNCBjMC43LDAuNiwxLjMsMS43LDEuMywzLjVjMCwyLjYsMCw0LjYsMCw1LjJjMCwwLjUsMC40LDEuMSwxLjMsMC45YzcuNS0yLjYsMTMtOS43LDEzLTE4LjFDNTEsMjEuOSw0Mi41LDEzLjQsMzIsMTMuNHoiLz48L3N2Zz4=
[gh-top-lang-url]:     https://github.com/trevorsandy/lpub3d

[gh-comm-since-badge]: https://img.shields.io/github/commits-since/trevorsandy/lpub3d/latest.svg?logo=data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSI0MCIgaGVpZ2h0PSI0MCIgdmlld0JveD0iMTIgMTIgNDAgNDAiPjxwYXRoIGZpbGw9IiNmZmYiIGQ9Ik0zMiwxMy40Yy0xMC41LDAtMTksOC41LTE5LDE5YzAsOC40LDUuNSwxNS41LDEzLDE4YzEsMC4yLDEuMy0wLjQsMS4zLTAuOWMwLTAuNSwwLTEuNywwLTMuMiBjLTUuMywxLjEtNi40LTIuNi02LjQtMi42QzIwLDQxLjYsMTguOCw0MSwxOC44LDQxYy0xLjctMS4yLDAuMS0xLjEsMC4xLTEuMWMxLjksMC4xLDIuOSwyLDIuOSwyYzEuNywyLjksNC41LDIuMSw1LjUsMS42IGMwLjItMS4yLDAuNy0yLjEsMS4yLTIuNmMtNC4yLTAuNS04LjctMi4xLTguNy05LjRjMC0yLjEsMC43LTMuNywyLTUuMWMtMC4yLTAuNS0wLjgtMi40LDAuMi01YzAsMCwxLjYtMC41LDUuMiwyIGMxLjUtMC40LDMuMS0wLjcsNC44LTAuN2MxLjYsMCwzLjMsMC4yLDQuNywwLjdjMy42LTIuNCw1LjItMiw1LjItMmMxLDIuNiwwLjQsNC42LDAuMiw1YzEuMiwxLjMsMiwzLDIsNS4xYzAsNy4zLTQuNSw4LjktOC43LDkuNCBjMC43LDAuNiwxLjMsMS43LDEuMywzLjVjMCwyLjYsMCw0LjYsMCw1LjJjMCwwLjUsMC40LDEuMSwxLjMsMC45YzcuNS0yLjYsMTMtOS43LDEzLTE4LjFDNTEsMjEuOSw0Mi41LDEzLjQsMzIsMTMuNHoiLz48L3N2Zz4=
[gh-comm-since-url]:   https://github.com/trevorsandy/lpub3d/commits/master

[gh-maintained-badge]: https://img.shields.io/maintenance/yes/2018.svg?logo=data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSI0MCIgaGVpZ2h0PSI0MCIgdmlld0JveD0iMTIgMTIgNDAgNDAiPjxwYXRoIGZpbGw9IiNmZmYiIGQ9Ik0zMiwxMy40Yy0xMC41LDAtMTksOC41LTE5LDE5YzAsOC40LDUuNSwxNS41LDEzLDE4YzEsMC4yLDEuMy0wLjQsMS4zLTAuOWMwLTAuNSwwLTEuNywwLTMuMiBjLTUuMywxLjEtNi40LTIuNi02LjQtMi42QzIwLDQxLjYsMTguOCw0MSwxOC44LDQxYy0xLjctMS4yLDAuMS0xLjEsMC4xLTEuMWMxLjksMC4xLDIuOSwyLDIuOSwyYzEuNywyLjksNC41LDIuMSw1LjUsMS42IGMwLjItMS4yLDAuNy0yLjEsMS4yLTIuNmMtNC4yLTAuNS04LjctMi4xLTguNy05LjRjMC0yLjEsMC43LTMuNywyLTUuMWMtMC4yLTAuNS0wLjgtMi40LDAuMi01YzAsMCwxLjYtMC41LDUuMiwyIGMxLjUtMC40LDMuMS0wLjcsNC44LTAuN2MxLjYsMCwzLjMsMC4yLDQuNywwLjdjMy42LTIuNCw1LjItMiw1LjItMmMxLDIuNiwwLjQsNC42LDAuMiw1YzEuMiwxLjMsMiwzLDIsNS4xYzAsNy4zLTQuNSw4LjktOC43LDkuNCBjMC43LDAuNiwxLjMsMS43LDEuMywzLjVjMCwyLjYsMCw0LjYsMCw1LjJjMCwwLjUsMC40LDEuMSwxLjMsMC45YzcuNS0yLjYsMTMtOS43LDEzLTE4LjFDNTEsMjEuOSw0Mi41LDEzLjQsMzIsMTMuNHoiLz48L3N2Zz4=
[gh-maintained-url]: https://github.com/trevorsandy/lpub3d/projects/1 "Last edited 10-12-2018"

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

Copyright (c) 2015 - 2018 by Trevor SANDY
