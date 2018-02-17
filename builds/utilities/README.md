### ___________ BUILDING LPUB3D PACKAGE FROM SOURCE ___________

If you prefer to build your own LPub3D install package, these notes might be helpful.

Starting with [![GitHub release][gh-rel-badge]][gh-rel-2.1.0-url], LPub3D build scripts builds the main applicaiton (LPub3D) along with its three  
integrated image renderers - [LDGlite][ldglite], [LDView][ldview] and [LPub3D-Trace (POV-Ray)][povray]. Furthermore, the LPub3D build process  
is now fully cloud-enabled and optimized for containerized builds. This means the build process is highly automated  
requiring zero intervention between pushing a build tag and downloading the built installation assets.  

However, as the build process is specifically designed to exploit cloud-based CI services and tools like [Travis-ci][travis-ci], [AppVeyor][appveyor-ci]  
and [Docker][dockerrepo], building LPub3D locally may require some presets and dependencies likely not availabe on your build  
environment. The details below hopefully address the necessary procedures to complete your build process but, it also  
goes without saying, that your local environment may present uinque configuration requirements not addressed in the  
steps below.

The LPub3D build process automatically resolves required build dependencies across all its supported platform builds.  
Execeptions are noted under **Platform-specific Dependencies** - which require manual resolution.

*It is a good idea resolve the following make dependencies before launching your build:*
* **All platforms:** git
* **Linux/macOS/bash:** wget, curl, unzip, tar
* **Windows:** 7-zip, Visual Studio 2017, MSYS2/Mingw64/Qt or Qt(Mingw32)
* **macOS**: X-Code, X11, Homebrew, Qt

**Note:** The LPub3D install archive lpub3d-2.1.0.0.700_20180102... is just a example. The actual output file name will  
reflect the git version, revision, build number and date the of the LPub3D build.  

#### ___________ MAC: BUILD OSX ‘DMG’ PACKAGE ___________

**Platform-specific Dependencies:**
*  [Qt][qtwebsite]: [CreateDmg.sh][createdmg] assumes Qt 5.10.0 installed at ~/Qt/IDE/5.10.0/clang_64
*  [Xcode][xcode]:
*  [Homebrew][homebrew]: `$ /usr/bin/ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"`
*  [XQuartz (X11)][x11]: `$ brew cask list && brew cask install xquartz`

**Steps:**

- [ ] 1. create and enter working directory  
`$ cd ~/`
- [ ] 2. export Qt location and bin  
`export PATH=~/Qt/IDE/5.10.0/clang_64:~/Qt/IDE/5.10.0/clang_64/bin:$PATH`
- [ ] 3. get [CreateDmg.sh][createdmg] build and packaging script  
`$ curl "https://raw.githubusercontent.com/trevorsandy/lpub3d/master/builds/macx/CreateDmg.sh" -o "CreateDmg.sh"`
- [ ] 4. enable execute permissions and execute package script  
`$ chmod +x CreateDmg.sh && ./CreateDmg.sh`
- [ ] 5. mount and install package available in ./dmgbuild/DMGS  
`$ hdiutil attach dmgbuild/DMGS/LPub3D_2.1.0.0.700_20180102_osx.dmg`
- [ ] 6. have a :beer:

#### ___________ LINUX: BUILD ARCH 'PKG.TAR.XZ' PACKAGE ___________

**Steps:**
- [ ] 1. enter directory  
`$ cd ~/`
- [ ] 2. get [CreatePkg.sh][createpkg] build and packaging script  
`$  wget https://raw.githubusercontent.com/trevorsandy/lpub3d/master/builds/linux/CreatePkg.sh`
- [ ] 3. enable execute permissions and execute package script  
`$ chmod +x CreatePkg.sh && ./CreatePkg.sh`
- [ ] 4. install package availavble in ./pkgbuild  
`$ sudo pacman -U --needed --noconfirm pkgbuild/lpub3d-2.1.0.0.700_20180102_1_x86_64.pkg.tar.xz`
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
`$ sudo dpkg --install -y debbuild/lpub3d_2.1.0.0.700_20180102_0ubuntu1_amd64.deb`
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
`$ sudo rpm -U rpmbuild/RPMS/x86_64/lpub3d-2.1.0.0.700_20180102_1fedora.x86_64.rpm`
- [ ] 5. have a :beer:

#### ___________ MICROSOFT WINDOWS PORTABLE (UNPACKED) PACKAGE ___________

**Platform-specific Dependencies:**
* [MSYS2][msys2]: Mingw64 - 32 and 64 bit platforms preferred
* [MinGW64][mingw64]: Qt
* [MSBuild - Visual Studion 2017][vs2017]: VS2017 is required to build [LDView][ldview] and [LPub3D-Trace (POV-Ray)][povray] image renderers

**Note:** The OOTB Windows build script [AutoBuild.cmd][autobuild] expects [MSYS2][msys2]/[Mingw32][mingw64] and [Mingw64][mingw64] [Qt][qtwebsite] at the following locations:
* C:\Msys2\Msys64\mingw32\bin
* C:\Msys2\Msys64\mingw64\bin

However, you may choose to download and install [Qt][qtwebsite], in which case you must change the [AutoBuild.cmd][autobuild] script paths above  
to the Qt install path. Also be sure to **not** select the **-all** install parameter if you do not have both 64 and 32bit Qt platforms installed. 

**Note:** I use [Visual Studion 2017][vs2017], but it is possible to use other versions - for example Visual Studio 2015

**Steps:**
- [ ] 1. enter directory  
`> CD %USERPROFILE%`
- [ ] 2. get lpub3d source  
`> git clone https:\\github.com\trevorsandy\lpub3d.git`
- [ ] 3. enter source directory and execute package script  
`> CD lpub3d & CALL builds\windows\AutoBuild.cmd -x86 -3rd -ins -chk`
- [ ] 4. copy builds\windows\release\LPub3D-Any-2.1.0.0.700_20180102\LPub3D_x86\\* to final destination  
`> XCOPY /Q /S /I /E /V /Y builds\windows\release\LPub3D-Any-2.1.0.0.700_20180102\LPub3D_x86 %USERPROFILE%`
- [ ] 5. have a :beer:

#### ___________ ALL LINUX DISTROS - DOCKER (WINDOWS) BUILD ___________
**Platform-specific Dependencies:**
* [Docker][dockerinstall]
* [Docker Compose][dockercompose]  
* [WSL - Windows Subsystem for Linux][wsl]

**Note:** Be sure to set C as a Shared Drive available to your containers  
**Note:** Compose instructions below are quite likely the same for Docker on Linux or macOS  
**Note:** [Docker-compose.yml][dockercomposefile] and Docker yml files for [archlinux][dockerarch], [Ubuntu Xenial][dockerxenial] and [Fedora 25][dockerfedora] are available.  
**Note:** [WSL - Windows Subsystem for Linux][wsl] is not essential and can be substitued with Git Bash.  
**Note:** You can substitue [Fedora 25][dockerfedora] with [archlinux][dockerarch] or [Ubuntu Xenial][dockerxenial] in the steps below.

**Steps:**
- [ ] 1. make and enter build direcories  
`PS...> cd $home; md dockerbuild; cd dockerbuild; md dockerfiles; md lpub3d_linux_3rdparty`
- [ ] 2. download Docker-compose yml  
`PS...> bash curl -sL https://raw.githubusercontent.com/trevorsandy/lpub3d/master/builds/linux/docker-compose/docker-compose-cibuild-linux.yml -o docker-compose-cibuild-linux.yml`
- [ ] 3. download Dockerfile yml  
`PS...> bash curl -sL https://raw.githubusercontent.com/trevorsandy/lpub3d/master/builds/linux/docker-compose/dockerfiles/Dockerfile-cibuild-fedora_25 -o dockerfiles/Dockerfile-cibuild-fedora_25`
- [ ] 4. execute Docker-compose command  
`PS...> docker-compose -f docker-compose-cibuild-linux.yml run fedora_25`
- [ ] 5. transfer to destination and install package lpub3d-2.1.0.0.700_20180102_1fedora.x86_64.rpm  
`$ sudo rpm -U lpub3d-2.1.0.0.700_20180102_1fedora.x86_64.rpm`
- [ ] 6. have a :beer:

[xcode]:             https://developer.apple.com/xcode/downloads/
[x11]:               https://www.xquartz.org/
[homebrew]:          http://brew.sh
[appdmg]:            https://github.com/LinusU/node-appdmg
[qtwebsite]:         https://www.qt.io/download/
[createdmg]:         https://github.com/trevorsandy/lpub3d/blob/master/builds/macx/CreateDmg.sh
[createrpm]:	     https://github.com/trevorsandy/lpub3d/blob/master/builds/linux/CreateRpm.sh
[createpkg]:	     https://github.com/trevorsandy/lpub3d/blob/master/builds/linux/CreatePkg.sh
[createdeb]:	     https://github.com/trevorsandy/lpub3d/blob/master/builds/linux/CreateDeb.sh
[autobuild]:         https://github.com/trevorsandy/lpub3d/blob/master/builds/windows/AutoBuild.cmd
[msys2]:             http://www.msys2.org/
[mingw64]:           https://github.com/Alexpux/mingw-w64
[dockerinstall]:     https://www.docker.com/get-docker
[dockercompose]:     https://docs.docker.com/compose/install/
[dockercomposefile]: https://github.com/trevorsandy/lpub3d/blob/master/builds/linux/docker-compose/docker-compose-cibuild-linux.yml
[dockerarch]:        https://github.com/trevorsandy/lpub3d/blob/master/builds/linux/docker-compose/dockerfiles/Dockerfile-cibuild-archlinux
[dockerxenial]:      https://github.com/trevorsandy/lpub3d/blob/master/builds/linux/docker-compose/dockerfiles/Dockerfile-cibuild-ubuntu_xenial
[dockerfedora]:      https://github.com/trevorsandy/lpub3d/blob/master/builds/linux/docker-compose/dockerfiles/Dockerfile-cibuild-fedora_25
[dockerrepo]:        https://hub.docker.com/r/trevorsandy/lpub3d/
[wsl]:               https://docs.microsoft.com/en-us/windows/wsl/install-win10
[vs2017]:            https://www.visualstudio.com/downloads/
[travis-ci]:         https://travis-ci.org/trevorsandy/lpub3d
[appveyor-ci]:       https://ci.appveyor.com/project/trevorsandy/lpub3d
[gh-rel-badge]:      https://img.shields.io/github/release/trevorsandy/lpub3d.svg

[gh-rel-2.1.0-url]:  https://github.com/trevorsandy/lpub3d/releases/v2.1.0
[gh-rel-url]:        https://github.com/trevorsandy/lpub3d/releases/latest

[ldglite]:          https://github.com/trevorsandy/ldglite
[ldview]:           https://github.com/trevorsandy/ldview
[povray]:           https://github.com/trevorsandy/povray

Copyright (c) 2015 - 2018 by Trevor SANDY
