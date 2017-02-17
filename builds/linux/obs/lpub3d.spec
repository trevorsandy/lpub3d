#
# spec file for package lpub3d
#
# Copyright © 2017 Trevor SANDY
# Using RPM Spec file examples by Thomas Baumgart, Peter Bartfai and others
# This file and all modifications and additions to the pristine
# package are under the same license as the package itself.
#
# please send bugfixes or comments to Trevor SANDY <trevor.sandy@gmail.com>
#

# date time
%global changedate %(echo `date "+%a %b %d %Y"`)

# define distributions
%if 0%{?suse_version}
%define dist .openSUSE%(echo %{suse_version} | sed 's/0$//')
%endif

%if 0%{?sles_version}
%define dist .SUSE%(echo %{sles_version} | sed 's/0$//')
%endif

%if 0%{?fedora}
%define dist fc
%endif

%if 0%{?mdkversion}
%define dist mdk
%endif

%if 0%{?scientificlinux_version}
%define dist scl
%endif

%if 0%{?rhel_version}
%define dist rhel
%endif

%if 0%{?centos_ver}
%define centos_version %{centos_ver}00
%define dist cos
%endif

# distro group settings
%if 0%{?suse_version} || 0%{?sles_version}
Group: Productivity/Graphics/Viewers
%endif
%if 0%{?mdkversion} || 0%{?rhel_version} 
Group: Graphics
%endif
%if 0%{?suse_version} || 0%{?sles_version}
License: GPL-3.0+
BuildRequires: fdupes
%endif
%if 0%{?fedora} || 0%{?centos_version}
Group: Amusements/Graphics
%endif
%if 0%{?mdkversion} || 0%{?rhel_version} || 0%{?fedora} || 0%{?centos_version} || 0%{?scientificlinux_version}
License: GPLv3+
%endif

# define git version string from source
Source10: lpub3d.spec.git.version
%define gitversion %(tr -d '\n' < %{SOURCE10})

# set packing platform
%if "0%{?vendor}"
%define obsurl obs://build.opensuse.org/home:
%define obsurlprivate obs://private/home:
%define serviceprovider %(echo "%{vendor}")
%define packingplatform %(if [[ "%{vendor}" == *"%{obsurl}"* ]] || [[ "%{vendor}" == *"%{obsurlprivate}"* ]]; then echo "openSUSE OBS"; else echo "$HOSTNAME [`uname`]"; fi)
%if "%{packingplatform}" == "openSUSE OBS"
%define OBS 1
%endif
%endif

# set packer
%if 0%{?OBS}
%define distpacker "openSUSE OBS [abuild]"
%else
BuildRequires: finger
%define distpacker "%(finger -lp `echo "$USER"` | head -n 1 | cut -d: -f 3)"
%endif

# package attributes
Name: lpub3d
Icon: lpub3d.xpm
Summary: An LDraw Building Instruction Editor
Version: %{gitversion}
Release: %{?dist}
URL: https://trevorsandy.github.io/lpub3d
Vendor: Trevor SANDY
Packager: %{distpacker}
BuildRoot: %{_builddir}/%{name}
BuildArch: %{_arch}
Requires: unzip 
Source0: lpub3d-git.tar.gz

# package requirements
%if 0%{?fedora} || 0%{?rhel_version} || 0%{?centos_version} || 0%{?scientificlinux_version}
BuildRequires: qt5-qtbase-devel
%if 0%{?fedora}
BuildRequires: qt5-linguist
%endif
%if 0%{?OBS}!=1
BuildRequires: git
%endif
BuildRequires: gcc-c++, make
%endif

%if 0%{?fedora}
%if 0%{?OBS}
#BuildRequires: samba4-libs
%if 0%{?fedora_version}==22
BuildRequires: qca
%endif
%if 0%{?fedora_version}==23
BuildRequires: qca, gnu-free-sans-fonts
%endif
%endif
%endif

%if 0%{?suse_version} 
BuildRequires: libqt5-qtbase-devel, zlib-devel
%if 0%{?OBS}
#BuildRequires: -post-build-checks
%endif
%endif

%if 0%{?sles_version}
%if 0%{?OBS}
#BuildRequires: -post-build-checks
%endif
%endif

%description
 LPub3D is an Open Source WYSIWYG editing application for creating 
 LEGO® style digital building instructions. LPub3D is developed and 
 maintained by Trevor SANDY. It uses the LDraw™ parts library, the 
 most comprehensive library of digital Open Source LEGO® bricks 
 available (www.ldraw.org/ ) and reads the LDraw LDR and MPD model 
 file formats. LPub3D is available for free under the GNU Public License v3 
 and runs on Windows, Linux and OSX Operating Systems.
 Portions of LPub3D are based on LPUB© 2007-2009 Kevin Clague, 
 LeoCAD© 2015 Leonardo Zide.and additional third party components.
 LEGO® is a trademark of the LEGO Group of companies which does not 
 sponsor, authorize or endorse this application.
 © 2015-2017 Trevor SANDY
 
%package ldrawini
Requires: lpub3d
Summary: LDrawDir and SearchDirs API
Group: Graphics
Provides: ldrawini

%description ldrawini
 This package provides an API to resolve LDrawDir (directory path
 to LDraw parts library), and SearchDirs (LPub3D additional parts
 search paths).

%package quazip
Requires: lpub3d
Summary: Qt C++ wrapper over Gilles Vollant's ZIP/UNZIP
Group: Graphics
Provides: quazip

%description: quazip
 This package provides a comprehensive range of archiving and
 unarchiving functions.


%prep
{ set +x; } 2>/dev/null
echo Target...................%{_target}
echo Target Vendor............%{_target_vendor}
echo Target CPU...............%{_target_cpu}
echo Name.....................%{name}
echo Summary..................%{summary}
echo Version..................%{version}
echo Vendor...................%{vendor}
echo Release..................%{release}
echo Packager.................%{distpacker}
echo Source0..................%{SOURCE0}
echo Source10.................%{SOURCE10}
echo Service Provider.........%{serviceprovider}
echo Packing Platform.........%{packingplatform}
echo Change Date..............%{changedate}
echo OpenBuildService Flag....%{OBS}
echo Build Package............%{name}-%{version}-%{release}-%{_arch}.rpm
{ set -x; } 2>/dev/null
%setup -q -n %{name}-git

%build
export QT_SELECT=qt5

# get ldraw archive libraries
LDrawLibOffical="../../SOURCES/complete.zip"
LDrawLibUnofficial="../../SOURCES/lpub3dldrawunf.zip"
if [ -f ${LDrawLibOffical} ] ; then 
	cp ${LDrawLibOffical} mainApp/extras
else
	echo "complete.zip not found!" 
fi
if [ -f ${LDrawLibUnofficial} ] ; then
	cp ${LDrawLibUnofficial} mainApp/extras
else
	echo "lpub3dldrawunf.zip not found!"
fi ;

# use Qt5
%if 0%{?fedora}==23
%ifarch x86_64
export Q_CXXFLAGS="$Q_CXXFLAGS -fPIC"
%endif
%endif
if which qmake-qt5 >/dev/null 2>/dev/null ; then 
	qmake-qt5 -makefile -nocache QMAKE_STRIP=: CONFIG+=release CONFIG+=rpm DOCS_DIR=%{_docdir}/lpub3d
else
	qmake -makefile -nocache QMAKE_STRIP=: CONFIG+=release CONFIG+=rpm DOCS_DIR=%{_docdir}/lpub3d
fi 
make clean
make %{?_smp_mflags}

%install
make INSTALL_ROOT=%buildroot install

%clean
rm -rf $RPM_BUILD_ROOT

%files
%if 0%{?sles_version} || 0%{?suse_version}
%defattr(-,root,root)
%endif
%{_bindir}/*
%{_datadir}/pixmaps/*
%{_datadir}/mime/packages/*
%{_datadir}/applications/*
%dir %{_datadir}/icons/hicolor/
%dir %{_datadir}/icons/hicolor/scalable/
%dir %{_datadir}/icons/hicolor/scalable/mimetypes/
%attr(644,-,-) %{_datadir}/icons/hicolor/scalable/mimetypes/*
%{_datadir}/lpub3d
%doc %{_docdir}/lpub3d
%{_mandir}/man1/*

%files ldrawini
%{_libdir}/libldrawini.so.16.1.8

%files quazip
%{_libdir}/libquazip.so.0.7.2

%post -p /sbin/ldconfig
%postun -p /sbin/ldconfig

%changelog
* %{changedate} - trevor.dot.sandy.at.gmail.dot.com %{version}
- LPub3D Linux package (rpm) release

