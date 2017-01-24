### ___________ BUILD FROM SOURCE INSTRUCTIONS

**NOTE** *LPub3D requires Qt5 to build successfully so you must have this version of Qt or higher* 
 *installed and configured in QtChooser. I used Qt 5.5.1. On Ubuntu, if Qt5 is not already bundled*
 *in your environment, you can install using $ sudo apt-get install qtbase5-dev.*

#### ___________ BUILD UBUNTU/DEBIAN 'DEB' PACKAGE ___________

- [ ]  Enter directory
`$ cd ~/debbuild`
- [ ] Make extraction directory [one-time only]
`$ mkdir upstream`
`$ cd upstream`
- [ ] download source
`$ git clone https://github.com/trevorsandy/lpub3d.git`
- [ ] create working tarball
`$ tar -czvf lpub3d.git.tar.gz lpub3d \
      --exclude="lpub3d/builds/linux/standard" \
      --exclude="lpub3d/builds/osx" \
      --exclude="lpub3d/.git" \
	  --exclude="lpub3d/.gitattributes" \
      --exclude="lpub3d/LPub3D.pro.user" \
	  --exclude="lpub3d/README.md" \
	  --exclude="lpub3d/_config.yml" \
      --exclude="lpub3d/.gitignore"`
- [ ] change directory
`$ cd ../`
- [ ] create package: Name, Version, Path to tarball
`$ bzr dh-make lpub3d 2.0.20 upstream/lpub3d.git.tar.gz`
- [ ] delete unneeded debian example files
`$ cd lpub3d/debian`
`$ rm *ex *EX`
- [ ] edit/copy debian build files as necessary - update app version
`$ cp -rf ../../upstream/lpub3d/builds/linux/obs/debian/* .`
- [ ] add format to package source
`$ bzr add source/format`
- [ ] test build the package
`$ bzr builddeb -- -us -uc`
- [ ] review lintian report
  ...
- [ ] rebuild packaging [if needed] - using 'nc' no clean
`$ bzr builddeb -- -nc -us -uc`
- [ ] view contents
`$ cd ../../`
`$ lesspipe lpub3d_2.0.20-0ubuntu1_amd64.deb`
- [ ] install the package
`$ sudo dpkg --install lpub3d_2.0.20-0ubuntu1_amd64.deb`
- [ ] check all is well by running:
`$ sudo apt-get -f install`
- [ ] lauch LPub3D
`$ lpub3d`
- [ ] uninstall
`$ sudo apt-get remove lpub3d`
- [ ] sign build
`$ cd lpub3d/debian`
`$ bzr builddeb -S`
- [ ] test build in clean environment
`$ cd ../../build-area`
`$ pbuilder-dist trusty build lpub3d_2.0.20-0ubuntu1.dsc`
- [ ] Have a :beer:


#### ___________ BUILD REDHAT/SUSE RPM PACKAGE ___________

- [ ] enter directory
`$ cd ~/`
- [ ] create directory tree BUILD, BUILDROOT, RPMS, SOURCES, SPECS, SRPMS [one-time only]
`$ rpmdev-setuptree`
- [ ] enter SOURCES directory
`$ cd rpmbuild/SOURCES`
- [ ] download source
`$ git clone https://github.com/trevorsandy/lpub3d.git`
- [ ] copy xpm icon to SOURCES/
`$ cp ~/rpmbuild/SOURCES/lpub3d/mainApp/images/lpub3d.xpm .`
- [ ] copy spec to SPECS/
`$ cp ~/rpmbuild/SOURCES/lpub3d/builds/linux/obs/lpub3d.spec ../SPECS/`
- [ ] check spec file and update app version
`$ cat ../SPECS/lpub3d.spec`
- [ ] create working tarball
`$ tar -czvf lpub3d.git.tar.gz lpub3d \
      --exclude="lpub3d/builds/linux/standard" \
      --exclude="lpub3d/builds/osx" \
      --exclude="lpub3d/.git" \
	  --exclude="lpub3d/.gitattributes" \
      --exclude="lpub3d/LPub3D.pro.user" \
	  --exclude="lpub3d/README.md" \
	  --exclude="lpub3d/_config.yml" \	  
      --exclude="lpub3d/.gitignore"`
- [ ] remove cloned repository from SOURCES/
`$ rm -rf lpub3d`
- [ ] build and sign the RPM package (success = 'exit 0')
`$ cd ../SPECS`
`$ rpmbuild -v -ba --sign lpub3d.spec`
**NOTE** *for details on how to sign an RPM package with GPG key
 see https://gist.github.com/fernandoaleman/1376730*
- [ ] Have a :beer: