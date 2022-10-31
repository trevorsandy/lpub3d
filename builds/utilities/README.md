### ___________ BUILDING LPUB3D PACKAGE FROM SOURCE ___________

If you prefer to build your own LPub3D install package, these notes might be helpful.

Starting with **v2.0.0**, LPub3D build scripts builds the core applicaiton (LPub3D), Visual Editor (based on LeoCAD), LDV (based on LDView) along with its three bundled 3rd-party image renderers - [LDGlite][ldglite], [LDView][ldview] and [LPub3D-Trace (POV-Ray)][povray]. Furthermore, the LPub3D build process is now fully cloud-enabled and optimized for containerized builds. This means the build process is highly automated requiring zero intervention between pushing a build tag and downloading the built installation assets.  

However, as the build process is specifically designed to exploit cloud-based CI services and tools like [Travis-ci][travis-ci], [AppVeyor][appveyor-ci]  
and [Docker][dockerrepo], building LPub3D locally may require some presets and dependencies likely not availabe on your build environment. The details below hopefully address the necessary procedures to complete your build process but, it also goes without saying, that your local environment may present uinque configuration requirements not addressed in the steps below.

The LPub3D build process automatically resolves required build dependencies across all its supported platform builds. Execeptions are noted under **Platform-specific Dependencies** - which require manual resolution.

*It is a good idea resolve the following make dependencies before launching your build:*
* **All platforms:** git
* **Linux/macOS/bash:** wget, curl, unzip, tar
* **Windows:** 7-zip, Visual Studio 2017, msvc2015 or msvc2015_64 (from v2.3.3), MSYS2/Mingw64/Qt or MSYS2/Mingw64/Qt (Up to v2.3.2)
* **macOS**: X-Code, X11, Homebrew, Qt

**Note:** The LPub3D install archive LPub3D-2.4.5.0.3185_20221026... is just an example. The actual output file name will reflect the git version, revision, build number and date the of the LPub3D build.  

#### ___________ MAC: BUILD OSX ‘DMG’ PACKAGE ___________

**Platform-specific Dependencies:**
*  [Qt][qtwebsite]: [CreateDmg.sh][createdmg] assumes Qt 5.12.2 installed at ~/Qt/IDE/5.12.2/clang_64
*  [Xcode][xcode]:
*  [Homebrew][homebrew]: `$ /usr/bin/ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"`
*  [XQuartz (X11)][x11]: `$ brew cask list && brew cask install xquartz`

**Steps:**

- [ ] 1. create and enter working directory  
`$ cd ~/`
- [ ] 2. export Qt location and bin  
`export PATH=~/Qt/IDE/5.12.2/clang_64:~/Qt/IDE/5.12.2/clang_64/bin:$PATH`
- [ ] 3. get [CreateDmg.sh][createdmg] build and packaging script  
`$ curl "https://raw.githubusercontent.com/trevorsandy/lpub3d/master/builds/macx/CreateDmg.sh" -o "CreateDmg.sh"`
- [ ] 4. enable execute permissions and execute package script  
`$ chmod +x CreateDmg.sh && ./CreateDmg.sh`
- [ ] 5. mount and install package available in ./dmgbuild/DMGS  
`$ hdiutil attach dmgbuild/DMGS/LPub3D-2.4.5.0.3185_20221026-macos.dmg`
- [ ] 6. have a :beer:

#### ___________ LINUX: BUILD ARCH 'PKG.TAR.ZST' PACKAGE __________

**Steps:**
- [ ] 1. enter directory  
`$ cd ~/`
- [ ] 2. get [CreatePkg.sh][createpkg] build and packaging script  
`$  wget https://raw.githubusercontent.com/trevorsandy/lpub3d/master/builds/linux/CreatePkg.sh`
- [ ] 3. enable execute permissions and execute package script  
`$ chmod +x CreatePkg.sh && ./CreatePkg.sh`
- [ ] 4. install package availavble in ./pkgbuild  
`$ sudo pacman -U --needed --noconfirm pkgbuild/LPub3D-2.4.5.0.3185_20221026-x86_64.pkg.tar.zst`
- [ ] 5. have a :beer:

#### ___________ LINUX: BUILD UBUNTU/DEBIAN 'DEB' PACKAGE ___________

**Steps:**
- [ ] 1. enter directory  
`$ cd ~/`
- [ ] 2. get [CreateDeb.sh][createdeb] build and packaging script  
`$  wget https://raw.githubusercontent.com/trevorsandy/lpub3d/master/builds/linux/CreateDeb.sh`
- [ ] 3. enable execute permissions and execute package script  
`$ chmod +x CreateDeb.sh && ./CreateDeb.sh`
- [ ] 4. install package availavble in ./debbuild  
`$ sudo dpkg --install -y debbuild/LPub3D-2.4.5.0.3185_20221026-bionic-amd64.deb`
- [ ] 5. have a :beer:

#### ___________ LINUX: BUILD REDHAT/FEDORA/SUSE 'RPM' PACKAGE ___________

**Steps:**
- [ ] 1. enter directory  
`$ cd ~/`
- [ ] 2. get [CreateRpm.sh][createrpm] build and packaging script  
`$  wget https://raw.githubusercontent.com/trevorsandy/lpub3d/master/builds/linux/CreateRpm.sh`
- [ ] 3. enable execute permissions and execute package script  
`$ chmod +x CreateRpm.sh && ./CreateRpm.sh`
- [ ] 4. install package availavble in ./rpmbuild/RPMS/x86_64  
`$ sudo rpm -U rpmbuild/RPMS/x86_64/LPub3D-2.4.5.0.3185_20221026-1.fc26.x86_64.rpm`
- [ ] 5. have a :beer:

#### ___________ MICROSOFT WINDOWS PORTABLE (UNPACKED) PACKAGE ___________

**Platform-specific Dependencies:**
* [Git-For-Windows][git-for-windows]: Git Software Configuration Management for Windows
* [QtVersion][qtwebsite]: Qt 5.11.3 or later. I use Qt 5.15.2
* [QtMSVC][qtwebsite]: msvc2015 and/or Qt msvc2015_64 or later (From v2.3.3). I use msvc2019 and msvc2019_64
* [MSYS2][msys2]: Mingw64 - 32 and/or 64 bit platforms (Up to v2.3.2)
* [QtMinGW][mingw64]: Qt MinGW (Up to v2.3.2)
* [MSBuild - Visual Studio][vs2017]: Visual Stuio 2017 or later is required to build LPub3D plus image renderers [LDView][ldview] and [LPub3D-Trace (POV-Ray)][povray]. I use Visual Stuio 2019. 

**Note:** From v2.3.3, the OOTB Windows build script [AutoBuild.bat][autobuild] expects [Qt][qtwebsite] QtVersion\QtMSVC at the following locations:
* C:\Qt\IDE\QtVersion\QtMSVC\bin
* C:\Qt\IDE\QtVersion\QtMSVC\bin

**Note:** Up to v2.3.2, the OOTB Windows build script [AutoBuild.bat][autobuild] expects [MSYS2][msys2]/[Mingw32][mingw64] and [Mingw64][mingw64] [Qt][qtwebsite] at the following locations:
* C:\Msys2\Msys64\mingw32\bin
* C:\Msys2\Msys64\mingw64\bin

However, you may choose to download and install [Qt][qtwebsite] elsewhere, in which case you must change the [AutoBuild.bat][autobuild] script paths above to the Qt install path. Also be sure to **not** select the **-all** install parameter if you do not have both 64 and 32bit Qt platforms installed.

**Note:** I use [Visual Studio 2019][vs2019], but it is possible to use other versions - for example Visual Studio 2017 or Visual Studio 2022. There is a [msvc2017_64 bug][msvc2017-bug] that produces a [Qt bug][qtmscv2017-64-bug] (fixed in Visual Stuio 2019) which breaks the build. So for consistency, I build both 32 and 64bit distributions on msvc2019.

**Steps:**
- [ ] 1. launch the command prompt and enter your desired directory - the user's home directory is used below.  
`> CD %USERPROFILE%`
- [ ] 2. get lpub3d source  
`> git clone https:\\github.com\trevorsandy\lpub3d.git`
- [ ] 3. enter source directory and execute package script  
`> CD lpub3d & CALL builds\windows\AutoBuild.bat -x86 -3rd -ins -chk`
- [ ] 4. copy builds\windows\release\LPub3D-Any-2.4.5.0.3185_20221026\LPub3D_x86\* to final destination  
`> XCOPY /Q /S /I /E /V /Y builds\windows\release\LPub3D-Any-2.4.5.0.3185_20221026\LPub3D_x86 %USERPROFILE%`
- [ ] 5. have a :beer:

**Running LPub3D - GitHub Actions Build locally:** If you wish to replicate the GitHub actions build process locally (On Windows), you can do so with these steps - assuming you have configured the above sependencies and executed through step 2 above.
- [ ] 1. set your desired build path  
`> SET %AppInstancePath%=%USERPROFILE%`
- [ ] 2. cut and paste the following script code to the command line  
```
CLS
CD %AppInstancePath% && SET AppInstance=lpub3d
rem Set your desired version tag
SET LP3D_LOCAL_TAG=v2.4.5
SET RUNNER_OS=Windows
SET GITHUB_CONFIG=release
SET GITHUB_JOB=Local CI Build
SET GITHUB_WORKSPACE=%CD%\%AppInstance%
SET GITHUB_REPOSITORY=trevorsandy\%AppInstance%
PUSHD %GITHUB_WORKSPACE%
FOR /F "usebackq delims==" %G IN (`git describe`) DO (SET GITHUB_REF_NAME=%G)
FOR /F "usebackq delims==" %G IN (`git rev-parse HEAD`) DO (SET GITHUB_SHA=%G)
POPD
SET GITHUB_REF=refs/tags/%GITHUB_REF_NAME%
SET LP3D_LOCAL_CI_BUILD=1
rem Set LP3D_INSTALL_PKG_ONLY=1 to skip package build and perform install for packaging only
SET LP3D_INSTALL_PKG_ONLY=0
rem Set your desired commit message
SET LP3D_COMMIT_MSG=LPub3D 26.10.2022
SET LP3D_BUILD_BASE=%CD%
SET LP3D_3RD_PARTY=third_party
SET LP3D_3RD_PARTY_PATH=%LP3D_BUILD_BASE%\%LP3D_3RD_PARTY%
SET LP3D_BUILDPKG_PATH=builds\windows\%GITHUB_CONFIG%
rem Set your Visual Studio year
SET LP3D_VSVERSION=2019
rem Set your Qt Version
SET LP3D_QTVERSION=5.15.2
SET LP3D_QT32_MSVC=C:\Qt\IDE\%LP3D_QTVERSION%\msvc%LP3D_VSVERSION%\bin
SET LP3D_QT64_MSVC=C:\Qt\IDE\%LP3D_QTVERSION%\msvc%LP3D_VSVERSION%_64\bin
IF NOT EXIST "%LP3D_3RD_PARTY%" ( MKLINK /d %LP3D_3RD_PARTY% %LP3D_BUILD_BASE%\lpub3d_windows_3rdparty 2>&1)
IF NOT EXIST "%LP3D_3RD_PARTY_PATH%\windows" ( MKLINK /d "%LP3D_3RD_PARTY%\windows" %LP3D_BUILD_BASE%\lpub3d_windows_3rdparty 2>&1 )
IF NOT EXIST "%LP3D_3RD_PARTY_PATH%\ldraw" ( MKLINK /d "%LP3D_3RD_PARTY%\ldraw" %USERPROFILE%\LDraw 2>&1 )

CALL .\%AppInstance%\builds\utilities\ci\github\windows-build.bat > %AppInstance%_windows-build_log.txt 2>&1
```
- [ ] 3. have a :beer:

#### ___________ ALL LINUX DISTROS - DOCKER (WINDOWS) BUILD ___________
**Platform-specific Dependencies:**
* [Docker][dockerinstall]  
* [Docker Compose][dockercompose]  
* [WSL - Windows Subsystem for Linux][wsl]

**Note:** Be sure to set C as a Shared Drive available to your containers  
**Note:** Compose instructions below are quite likely the same for Docker on Linux or macOS  
**Note:** [Docker-compose.yml][dockercomposefile] and Docker yml files for [archlinux][dockerarch], [Ubuntu][dockerubuntu] and [Fedora 26][dockerfedora] are available.
**Note:** [WSL - Windows Subsystem for Linux][wsl] is not essential and can be substitued with Git Bash.  
**Note:** You can substitue [Fedora 26][dockerfedora] with [archlinux][dockerarch] or [Ubuntu][dockerubuntu] in the steps below.

**Steps:**
- [ ] 1. make and enter build direcories  
`PS...> cd $home; md dockerbuild; cd dockerbuild; md dockerfiles; md lpub3d_linux_3rdparty`
- [ ] 2. download Docker-compose yml  
`PS...> bash curl -sL https://raw.githubusercontent.com/trevorsandy/lpub3d/master/builds/linux/docker-compose/docker-compose-cibuild-linux.yml -o docker-compose-cibuild-linux.yml`
- [ ] 3. download Dockerfile yml  
`PS...> bash curl -sL https://raw.githubusercontent.com/trevorsandy/lpub3d/master/builds/linux/docker-compose/dockerfiles/Dockerfile-cibuild-fedora -o dockerfiles/Dockerfile-cibuild-fedora`
- [ ] 4. execute Docker-compose command  
`PS...> docker-compose -f docker-compose-cibuild-linux.yml run fedora`
- [ ] 5. transfer to destination and install package LPub3D-2.4.5.0.3185_20221026-1.fc26.x86_64.rpm  
`$ sudo rpm -U LPub3D-2.4.5.0.3185_20221026-1.fc26.x86_64.rpm`
- [ ] 6. have a :beer:  

[git-for-windows]:   https://gitforwindows.org/
[xcode]:             https://developer.apple.com/xcode/downloads/
[x11]:               https://www.xquartz.org/
[homebrew]:          http://brew.sh
[appdmg]:            https://github.com/LinusU/node-appdmg
[qtwebsite]:         https://www.qt.io/download/
[createdmg]:         https://github.com/trevorsandy/lpub3d/blob/master/builds/macx/CreateDmg.sh
[createrpm]:         https://github.com/trevorsandy/lpub3d/blob/master/builds/linux/CreateRpm.sh
[createpkg]:         https://github.com/trevorsandy/lpub3d/blob/master/builds/linux/CreatePkg.sh
[createdeb]:         https://github.com/trevorsandy/lpub3d/blob/master/builds/linux/CreateDeb.sh
[autobuild]:         https://github.com/trevorsandy/lpub3d/blob/master/builds/windows/AutoBuild.bat
[msys2]:             http://www.msys2.org/
[mingw64]:           https://github.com/Alexpux/mingw-w64
[dockerinstall]:     https://www.docker.com/get-docker
[dockercompose]:     https://docs.docker.com/compose/install/
[dockercomposefile]: https://github.com/trevorsandy/lpub3d/blob/master/builds/linux/docker-compose/docker-compose-cibuild-linux.yml
[dockerarch]:        https://github.com/trevorsandy/lpub3d/blob/master/builds/linux/docker-compose/dockerfiles/Dockerfile-cibuild-archlinux
[dockerubuntu]:      https://github.com/trevorsandy/lpub3d/blob/master/builds/linux/docker-compose/dockerfiles/Dockerfile-cibuild-ubuntu
[dockerfedora]:      https://github.com/trevorsandy/lpub3d/blob/master/builds/linux/docker-compose/dockerfiles/Dockerfile-cibuild-fedora
[dockerrepo]:        https://hub.docker.com/r/trevorsandy/lpub3d/
[wsl]:               https://docs.microsoft.com/en-us/windows/wsl/install-win10
[vs2019]:            https://www.visualstudio.com/downloads/
[qtmscv2017-64-bug]: https://bugreports.qt.io/browse/QTBUG-72073
[msvc2017-bug]:      https://developercommunity.visualstudio.com/content/problem/406329/compiler-error-c2666-when-using-stdbitset-and-cust.html
[travis-ci]:         https://travis-ci.com/trevorsandy/lpub3d
[appveyor-ci]:       https://ci.appveyor.com/project/trevorsandy/lpub3d
[gh-rel-badge]:      https://img.shields.io/github/release/trevorsandy/lpub3d.svg

[gh-rel-2.1.0-url]:  https://github.com/trevorsandy/lpub3d/releases/v2.3.9
[gh-rel-url]:        https://github.com/trevorsandy/lpub3d/releases/latest

[ldglite]:           https://github.com/trevorsandy/ldglite
[ldview]:            https://github.com/trevorsandy/ldview
[povray]:            https://github.com/trevorsandy/povray

Copyright (C) 2015 - 2022 by Trevor SANDY
