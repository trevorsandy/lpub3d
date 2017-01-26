### ___________ BUILD FROM SOURCE INSTRUCTIONS  ___________

If you would like to create your own LPub3D install package, these notes might be helpful.


#### ___________ MAC: BUILD OSX ‘DMG’ PACKAGE ___________

**NOTE** *The build script CreateDmg.sh uses the non-native 3-rd party application [node-appdmg][appdmg] to perform final packaging tasks. If you will use the steps below to build LPub3D for OSX, be sure to install node-appdmg prior.*

- [ ] 1. enter executable directory
`$ cd ~/Projects/lpub3d/builds/osx`
- [ ] 2. execute package script
`$ ./CreateDmg.sh 2.0.20`
- [ ] 3. install the package
- [ ] 4. have a :beer:

**NOTE** *For Linux Distributions, LPub3D requires Qt5 to build successfully so you must have this
version of Qt or higher installed and configured in QtChooser. I used Qt 5.5.1. On Ubuntu, if Qt5
is not already bundled in your environment, you can install using:*
	DEB package: 		`$ sudo apt-get install qtbase5-dev`
	RPM package: 		`$ sudo yum install qt5-qtbase-devel`
	PKG.TAR.XZ package: 	`$ sudo pacman -S qt5-base qt5-tools` *(not required because makepkg flat will automatically download package dependencies.)*

*Source signing in the steps below uses* [GnuPG][gnupg] *also known as GPG.*

*The 'DEB' packaging steps below uses* [Bazaar][bazzar] *version control.*


#### ___________ LINUX: BUILD ARCH 'PKG.TAR.XZ' PACKAGE ___________

- [ ] 1. enter directory
`$ cd ~/pkgbuild`
- [ ] 2. make extraction and src directories [one-time only]
`$ mkdir upstream`
`$ cd upstream`
- [ ] 3. download source
`$ git clone https://github.com/trevorsandy/lpub3d.git`
- [ ] 4. create working tarball
```sh
$ tar -czvf ../lpub3d.git.tar.gz \
        --exclude="lpub3d/builds/linux/standard" \
        --exclude="lpub3d/builds/osx" \
        --exclude="lpub3d/.git" \
        --exclude="lpub3d/.gitattributes" \
        --exclude="lpub3d/LPub3D.pro.user" \
        --exclude="lpub3d/README.md" \
        --exclude="lpub3d/_config.yml" \
        --exclude="lpub3d/.gitignore" lpub3d
```
- [ ] 5. change directory
`$ cd ../`
- [ ] 6. copy PKGBUILD
`$ cp upstream/lpub3d/builds/linux/obs/PKGBUILD .`
- [ ] 7. create package, use '-s' flag to enabe automatic dependency handling
`$ makepkg -s`
- [ ] 8. sign package
`$ gpg --detach-sign lpub3d-2.0.20-1-x86_64.pkg.tar.xz`
- [ ] 9. install package
`$ pacman -U lpub3d-2.0.20-1-x86_64.pkg.tar.xz`
- [ ] 10. have a :beer:


#### ___________ LINUX: BUILD UBUNTU/DEBIAN 'DEB' PACKAGE ___________

- [ ] 1. enter directory
`$ cd ~/debbuild`
- [ ] 2. make extraction directory [one-time only]
`$ mkdir upstream`
`$ cd upstream`
- [ ] 3. download source
`$ git clone https://github.com/trevorsandy/lpub3d.git`
- [ ] 4. create working tarball
```sh
$ tar -czvf lpub3d.git.tar.gz lpub3d \
        --exclude="lpub3d/builds/linux/standard" \
        --exclude="lpub3d/builds/osx" \
        --exclude="lpub3d/.git" \
        --exclude="lpub3d/.gitattributes" \
        --exclude="lpub3d/LPub3D.pro.user" \
        --exclude="lpub3d/README.md" \
        --exclude="lpub3d/_config.yml" \
        --exclude="lpub3d/.gitignore"
```
- [ ] 5. change directory
`$ cd ../`
- [ ] 6. create package: Name, Version, Path to tarball
`$ bzr dh-make lpub3d 2.0.20 upstream/lpub3d.git.tar.gz`
- [ ] 7. delete unneeded debian example files
`$ cd lpub3d/debian`
`$ rm *ex *EX`
- [ ] 8. edit/copy debian build files as necessary - update app version
`$ cp -rf ../../upstream/lpub3d/builds/linux/obs/debian/* .`
- [ ] 9. add format to package source
`$ bzr add source/format`
- [ ] 10. test build the package
`$ bzr builddeb -- -us -uc`
- [ ] 11. review lintian report
  ...
- [ ] 12. rebuild package [if needed] - using 'nc' no clean
`$ bzr builddeb -- -nc -us -uc`
- [ ] 13. view contents
`$ cd ../../`
`$ lesspipe lpub3d_2.0.20-0ubuntu1_amd64.deb`
- [ ] 14. install the package
`$ sudo dpkg --install lpub3d_2.0.20-0ubuntu1_amd64.deb`
- [ ] 15. check all is well by running:
`$ sudo apt-get -f install`
- [ ] 17. launch LPub3D
`$ lpub3d`
- [ ] 18. uninstall
`$ sudo apt-get remove lpub3d`
- [ ] 19. sign build
`$ cd lpub3d/debian`
`$ bzr builddeb -S`
- [ ] 20. test build in clean environment
`$ cd ../../build-area`
`$ pbuilder-dist trusty build lpub3d_2.0.20-0ubuntu1.dsc`
- [ ] 21. have a :beer:


#### ___________ LINUX: BUILD REDHAT/SUSE 'RPM' PACKAGE ___________

- [ ] 1. enter directory
`$ cd ~/`
- [ ] 2. create directory tree BUILD, BUILDROOT, RPMS, SOURCES, SPECS, SRPMS [one-time only]
`$ rpmdev-setuptree`
- [ ] 3. enter SOURCES directory
`$ cd rpmbuild/SOURCES`
- [ ] 4. download source
`$ git clone https://github.com/trevorsandy/lpub3d.git`
- [ ] 5. copy xpm icon to SOURCES/
`$ cp ~/rpmbuild/SOURCES/lpub3d/mainApp/images/lpub3d.xpm .`
- [ ] 6. copy spec to SPECS/
`$ cp ~/rpmbuild/SOURCES/lpub3d/builds/linux/obs/lpub3d.spec ../SPECS/`
- [ ] 7. check spec file and update app version
`$ cat ../SPECS/lpub3d.spec`
- [ ] 8. create working tarball
```sh
$ tar -czvf lpub3d.git.tar.gz lpub3d \
        --exclude="lpub3d/builds/linux/standard" \
        --exclude="lpub3d/builds/osx" \
        --exclude="lpub3d/.git" \
        --exclude="lpub3d/.gitattributes" \
        --exclude="lpub3d/LPub3D.pro.user" \
        --exclude="lpub3d/README.md" \
        --exclude="lpub3d/_config.yml" \
        --exclude="lpub3d/.gitignore"
```
- [ ] 9. remove cloned repository from SOURCES/
`$ rm -rf lpub3d`
- [ ] 10. build and sign the RPM package (success = 'exit 0')
`$ cd ../SPECS`
`$ rpmbuild -v -ba --sign lpub3d.spec`
**NOTE** *for details on how to sign an RPM package with GPG key
 see https://gist.github.com/fernandoaleman/1376720*
- [ ] 11. have a :beer:

[appdmg]: 	https://github.com/LinusU/node-appdmg
[gnupg]: 	https://www.gnupg.org
[bazzar]: 	bazaar.canonical.com/en/

- [ ] 9. add format to package source
`$ bzr add source/format`
- [ ] 10. test build the package
`$ bzr builddeb -- -us -uc`
- [ ] 11. review lintian report
  ...
- [ ] 12. rebuild packaging [if needed] - using 'nc' no clean
`$ bzr builddeb -- -nc -us -uc`
- [ ] 13. view contents
`$ cd ../../`
`$ lesspipe lpub3d_2.0.20-0ubuntu1_amd64.deb`
- [ ] 14. install the package
`$ sudo dpkg --install lpub3d_2.0.20-0ubuntu1_amd64.deb`
- [ ] 15. check all is well by running:
`$ sudo apt-get -f install`
- [ ] 17. launch LPub3D
`$ lpub3d`
- [ ] 18. uninstall
`$ sudo apt-get remove lpub3d`
- [ ] 19. sign build
`$ cd lpub3d/debian`
`$ bzr builddeb -S`
- [ ] 20. test build in clean environment
`$ cd ../../build-area`
`$ pbuilder-dist trusty build lpub3d_2.0.20-0ubuntu1.dsc`
- [ ] 21. have a :beer:


#### ___________ LINUX: BUILD REDHAT/SUSE RPM PACKAGE ___________

- [ ] 1. enter directory
`$ cd ~/`
- [ ] 2. create directory tree BUILD, BUILDROOT, RPMS, SOURCES, SPECS, SRPMS [one-time only]
`$ rpmdev-setuptree`
- [ ] 3. enter SOURCES directory
`$ cd rpmbuild/SOURCES`
- [ ] 4. download source
`$ git clone https://github.com/trevorsandy/lpub3d.git`
- [ ] 5. copy xpm icon to SOURCES/
`$ cp ~/rpmbuild/SOURCES/lpub3d/mainApp/images/lpub3d.xpm .`
- [ ] 6. copy spec to SPECS/
`$ cp ~/rpmbuild/SOURCES/lpub3d/builds/linux/obs/lpub3d.spec ../SPECS/`
- [ ] 7. check spec file and update app version
`$ cat ../SPECS/lpub3d.spec`
- [ ] 8. create working tarball
```sh
$ tar -czvf lpub3d.git.tar.gz lpub3d \
        --exclude="lpub3d/builds/linux/standard" \
        --exclude="lpub3d/builds/osx" \
        --exclude="lpub3d/.git" \
        --exclude="lpub3d/.gitattributes" \
        --exclude="lpub3d/LPub3D.pro.user" \
        --exclude="lpub3d/README.md" \
        --exclude="lpub3d/_config.yml" \
        --exclude="lpub3d/.gitignore"
```
- [ ] 9. remove cloned repository from SOURCES/
`$ rm -rf lpub3d`
- [ ] 10. build and sign the RPM package (success = 'exit 0')
`$ cd ../SPECS`
`$ rpmbuild -v -ba --sign lpub3d.spec`
**NOTE** *for details on how to sign an RPM package with GPG key
 see https://gist.github.com/fernandoaleman/1376720*
- [ ] 11. have a :beer:

[appdmg]: https://github.com/LinusU/node-appdmg
