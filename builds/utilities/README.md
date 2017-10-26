### ___________ BUILDING LPUB3D PACKAGE FROM SOURCE ___________

If you prefer to create your own LPub3D install package, these notes might be helpful.

**Cross-platform Dependencies:** *LPub3D requires Qt5. You can install, if not available, using:*
* DMG package: 		Download and install [Qt][qtwebsite] ([CreateDmg.sh][createdmg] assumes Qt 5.7 installed at ~/Qt/IDE/5.7)
* DEB package: 		`$ sudo apt-get install qtbase5-dev qt5-qmake`
* RPM package: 		`$ sudo dnf install qt5-qtbase-devel`
* PKG.TAR.XZ package: 	`$ sudo pacman -S qt5-base qt5-tools` *(not required, dependencies automatically downloaded.)*

*Additional dependencies include:*
* git
* tar
* wget

#### ___________ MAC: BUILD OSX ‘DMG’ PACKAGE ___________

**Platform-specific Dependencies:**
*  [Xcode][xcode]:
*  [Homebrew][homebrew]:         `/usr/bin/ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"`
*  [node-appdmg][appdmg]:        `npm install -g appdmg`

**Steps:**
- [ ] 1. enter directory
`$ cd ~/`
- [ ] 2. get [CreateDmg.sh][createdmg] packaging script
`$ curl "https://raw.githubusercontent.com/trevorsandy/lpub3d/master/builds/macx/CreateDmg.sh" -o "CreateDmg.sh"`
- [ ] 3. enable execute permissions
`$ chmod 755 CreateDmg.sh`
- [ ] 4. execute package script
`$ ./CreateDmg.sh`
- [ ] 5. mount and install package ./dmgbuild/DMGS/LPub3D_2.0.20.0.645_20170208_osx.dmg
- [ ] 6. have a :beer:

#### ___________ LINUX: BUILD ARCH 'PKG.TAR.XZ' PACKAGE ___________

**Platform-specific Dependencies:**

**Steps:**
- [ ] 1. enter directory
`$ cd ~/`
- [ ] 2. get [CreatePkg.sh][createpkg] packaging script
`$  wget https://raw.githubusercontent.com/trevorsandy/lpub3d/master/builds/linux/CreatePkg.sh`
- [ ] 3. enable execute permissions
`$ chmod 755 CreatePkg.sh`
- [ ] 4. execute package script
`$ ./CreatePkg.sh`
- [ ] 5. install package availavble in ./pkgbuild
`$ sudo pacman -U pkgbuild/lpub3d-2.0.20.0.645_20170208_1_x86_64.pkg.tar.xz`
- [ ] 6. have a :beer:

#### ___________ LINUX: BUILD UBUNTU/DEBIAN 'DEB' PACKAGE ___________

**Platform-specific Dependencies:**
*  build-essential:       `$ sudo apt-get install build-essential`

**Steps:**
- [ ] 1. enter directory
`$ cd ~/`
- [ ] 2. get [CreateDeb.sh][createdeb] packaging script
`$  wget https://raw.githubusercontent.com/trevorsandy/lpub3d/master/builds/linux/CreateDeb.sh`
- [ ] 3. enable execute permissions
`$ chmod 755 CreateDeb.sh`
- [ ] 4. execute package script
`$ ./CreateDeb.sh`
- [ ] 5. install package availavble in ./debbuild
`$ sudo dpkg --install debbuild/lpub3d_2.0.20.0.645_20170208_0ubuntu1_amd64.deb`
- [ ] 6. have a :beer:

#### ___________ LINUX: BUILD REDHAT/FEDORA/SUSE 'RPM' PACKAGE ___________

**Platform-specific Dependencies:**
*  qca:                  `sudo dnf install qca`
*  zlib-devel:           `sudo dnf install zlib-devel`
*  gnu-free-sans-fonts:  `sudo dnf install gnu-free-sans-fonts`

**Steps:**
- [ ] 1. enter directory
`$ cd ~/`
- [ ] 2. get [CreateRpm.sh][createrpm] packaging script
`$  wget https://raw.githubusercontent.com/trevorsandy/lpub3d/master/builds/linux/CreateRpm.sh`
- [ ] 3. enable execute permissions
`$ chmod 755 CreateRpm.sh`
- [ ] 4. execute package script
`$ ./CreateRpm.sh`
- [ ] 5. install either availavble in ./rpmbuild/RPMS/x86_64
`$ sudo rpm -U rpmbuild/RPMS/x86_64/lpub3d-2.0.20.0.645_20170208_1fedora.x86_64.rpm`
- [ ] 6. have a :beer:

[xcode]:        https://developer.apple.com/xcode/downloads/
[homebrew]:     http://brew.sh
[appdmg]: 		https://github.com/LinusU/node-appdmg
[qtwebsite]:    https://www.qt.io/download/
[createdmg]:    https://github.com/trevorsandy/lpub3d/blob/master/builds/macx/CreateDmg.sh
[createrpm]:	https://github.com/trevorsandy/lpub3d/blob/master/builds/linux/CreateRpm.sh
[createpkg]:	https://github.com/trevorsandy/lpub3d/blob/master/builds/linux/CreatePkg.sh
[createdeb]:	https://github.com/trevorsandy/lpub3d/blob/master/builds/linux/CreateDeb.sh
