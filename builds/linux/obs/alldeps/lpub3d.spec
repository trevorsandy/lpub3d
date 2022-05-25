#
# spec file for package lpub3d
#
# Last Update: July 23, 2020
# Copyright © 2017 - 2021 Trevor SANDY
# Using RPM Spec file examples by Thomas Baumgart, Peter Bartfai and others
# This file and all modifications and additions to the pristine
# package are under the same license as the package itself.
#
# please send bugfixes or comments to Trevor SANDY <trevor.sandy@gmail.com>
#

# set packing platform
%if %(if [[ "%{vendor}" == obs://* ]]; then echo 1; else echo 0; fi)
%define buildservice 1
%define usingbuildservice true
%define packingplatform %(echo openSUSE Build Service)
%else
%define usingbuildservice false
%define packingplatform %(. /etc/os-release 2>/dev/null; [ -n "$PRETTY_NAME" ] && echo "$PRETTY_NAME" || echo $HOSTNAME [`uname`])
%endif

# set packer
%define serviceprovider %(echo Trevor SANDY trevor.sandy@gmail.com)
%if 0%{?buildservice}
%define distpacker %(echo openSUSE Build Service [`whoami`])
%define targetplatform %{_target}
%else
%define distpacker %(echo `whoami`)
%define targetplatform %{packingplatform}
%endif

# set target platform id
# distinguish between SLE, openSUSE Leap and openSUSE
# SUSE Linux Enterprise Server
%if (0%{?sle_version}>=120000 && 0%{?sle_version}<=150000 && !0%{?is_opensuse})
%define dist .SLE%(echo %{sle_version} | sed 's/0$//')
%define suse_dist_name %(echo SUSE Linux Enterprise Server)
%define suse_dist_label %(echo %{suse_dist_name}...%{sle_version})
%define suse_dist_pretty_name %(echo %{suse_dist_name} %{sle_version})
%define suse_dist_version %{sle_version}
%define suse_platform_code sle
%define build_sdl2 1
%else
# openSUSE Leap
%if (0%{?sle_version}>=120100 && 0%{?sle_version}<=150000 && 0%{?is_opensuse})
%define dist .openSUSELeap%(echo %{sle_version} | sed 's/0$//')
%define suse_dist_name %(echo openSUSE Leap)
%define suse_dist_label %(echo %{suse_dist_name}..................%{sle_version})
%define suse_dist_pretty_name %(echo %{suse_dist_name} %{sle_version})
%define suse_dist_version %{sle_version}
%define suse_platform_code osl
%define build_sdl2 1
%else
# openSUSE
%if (0%{?suse_version}>=1320 && 0%{?suse_version}<=1550)
%define dist .openSUSE%(echo %{suse_version} | sed 's/0$//')
%define suse_dist_name %(echo openSUSE)
%define suse_dist_label %(echo %{suse_dist_name}.......................%{suse_version})
%define suse_dist_pretty_name %(echo %{suse_dist_name} %{suse_version})
%define suse_dist_version %{suse_version}
%define suse_platform_code os
%endif
%endif
%endif

%if 0%{?centos_ver}
%define centos_version %{centos_ver}00
%define build_sdl2 1
%endif

%if 0%{?rhel_version}
%define build_sdl2 1
%define get_local_libs 1
%endif

%if 0%{?scientificlinux_version}
%define build_sdl2 1
%endif

%if 0%{?fedora}
%define fedora_version %{fedora}
%endif

%if 0%{?mageia}
%define mageia_version %{mageia}
%endif

# distro group settings
%if 0%{?suse_version} || 0%{?sle_version}
Group: Productivity/Graphics/Viewers
%endif

%if 0%{?mageia} || 0%{?rhel_version} || 0%{?scientificlinux_version}
Group: Graphics
%endif

%if 0%{?centos_version} || 0%{?fedora}
Group: Amusements/Graphics
%endif

%if 0%{?centos_version} || 0%{?fedora} || 0%{?mageia} || 0%{?rhel_version} || 0%{?scientificlinux_version}
License: GPLv2+
%endif

%if 0%{?suse_version} || 0%{?sle_version}
License: GPL-2.0+
BuildRequires: fdupes
%endif

# set custom directory paths
%define _3rdexedir /opt/lpub3d/3rdParty
%define _iconsdir %{_datadir}/icons

# preamble
Summary: An LDraw Building Instruction Editor
Name: lpub3d
Icon: lpub3d.xpm
Version: 2.4.4.2596
Release: <B_CNT>%{?dist}
URL: https://trevorsandy.github.io/lpub3d
Vendor: Trevor SANDY
BuildRoot: %{_builddir}/%{name}
BuildRequires: unzip
Source0: lpub3d-git.tar.gz
Source10: lpub3d-rpmlintrc

# package requirements
%if 0%{?centos_version}==600 || 0%{?rhel_version} || 0%{?scientificlinux_version}
%define get_qt5 1
BuildRequires: cmake
%endif
%if 0%{?fedora} || 0%{?centos_version}==700
BuildRequires: qt5-qtbase-devel, qt5-qttools-devel
%endif
%if 0%{?fedora} || 0%{?centos_version} || 0%{?rhel_version} || 0%{?scientificlinux_version}
%if 0%{?fedora}
BuildRequires: hostname
%endif
%if !0%{?rhel_version}
BuildRequires: mesa-libOSMesa-devel, OpenEXR-devel
%endif
BuildRequires: freeglut-devel
BuildRequires: mesa-libGLU-devel
BuildRequires: boost-devel, libtiff-devel
BuildRequires: gcc-c++, make, libpng-devel
%if 0%{?buildservice}!=1
BuildRequires: git
%endif
%endif

%if 0%{?centos_version} || 0%{?rhel_version} || 0%{?scientificlinux_version}
%if 0%{?scientificlinux_version}
BuildRequires: gnu-free-sans-fonts
BuildRequires: kde-runtime
%endif
%if 0%{?centos_version}
BuildRequires: mesa-libwayland-egl
%endif
BuildRequires: libjpeg-turbo-devel
%if 0%{?rhel_version}
# Disable this as we have local OSMesa library
%define build_osmesa_foo foo
%endif
%define build_tinyxml 1
%define build_gl2ps 1
%endif

%if 0%{?fedora}
BuildRequires: libjpeg-turbo-devel, tinyxml-devel, gl2ps-devel
BuildRequires: qt5-linguist, SDL2-devel
%if 0%{?buildservice}
BuildRequires: samba4-libs
%if 0%{?fedora_version}==23
BuildRequires: qca, gnu-free-sans-fonts
%endif
%if 0%{?fedora_version}==25
BuildRequires: llvm-libs
%define build_osmesa 1
%endif
%if 0%{?fedora_version}==26
BuildRequires: openssl-devel, storaged
%define build_osmesa 1
%endif
%if 0%{?fedora_version}==27 || 0%{?fedora_version}==28
%define build_osmesa 1
%endif
%if 0%{?fedora_version}>30
BuildRequires: libXext-devel
%endif
%endif
%endif

%if 0%{?suse_version}
Requires(pre): gconf2
%if (0%{?sle_version}!=150000)
BuildRequires: freeglut-devel
%endif
BuildRequires: libqt5-qtbase-devel
BuildRequires: libOSMesa-devel, glu-devel, openexr-devel
BuildRequires: libpng16-compat-devel, libjpeg8-devel
BuildRequires: update-desktop-files
BuildRequires: zlib-devel
%if (0%{?suse_version}>1210 && 0%{?suse_version}!=1315 && 0%{?sle_version}!=150000 && 0%{?sle_version}!=150100 && 0%{?sle_version}!=150200)
BuildRequires: gl2ps-devel
%else
%define build_gl2ps 1
%endif
%if 0%{?suse_version}>1220
BuildRequires: glu-devel
%endif
%if 0%{?suse_version}>1300
BuildRequires: Mesa-devel
%endif
%if 0%{?suse_version}==1315
%define build_sdl2 1
%endif
%if 0%{?buildservice}
BuildRequires: -post-build-checks
%endif
%define build_tinyxml 1
%endif

%if 0%{?mageia}
BuildRequires: qttools5
%ifarch x86_64
BuildRequires: lib64qt5base5-devel, lib64sdl2.0-devel, lib64osmesa-devel, lib64mesaglu1-devel, lib64freeglut-devel
BuildRequires: lib64boost-devel, lib64tinyxml-devel, lib64gl2ps-devel, lib64tiff-devel
%if 0%{?mageia_version}>5
BuildRequires: lib64openexr-devel
%endif
%if 0%{?buildservice}
BuildRequires: lib64sane1, lib64proxy-webkit
%if 0%{?mageia} == 7
BuildRequires: lib64openssl-devel
%endif
%endif
%else
BuildRequires: libqt5base5-devel, libsdl2.0-devel, libosmesa-devel, libmesaglu1-devel, freeglut-devel
BuildRequires: libboost-devel, libtinyxml-devel, libgl2ps-devel, libtiff-devel
%if 0%{?mageia_version}>5
BuildRequires: libopenexr-devel
%endif
%if 0%{?buildservice}
BuildRequires: libsane1, libproxy-webkit
%if 0%{?mageia} == 7
BuildRequires: libopenssl-devel
%endif
%endif
%endif
%endif

%if 0%{?sle_version}
%define build_tinyxml 1
%define osmesa_found %(test -f /usr/lib/libOSMesa.so -o -f /usr/lib64/libOSMesa.so && echo 1 || echo 0)
%if 0%{osmesa_found} != 1
%define build_osmesa 1
%endif
%if 0%{?buildservice}
BuildRequires:  -post-build-checks
%endif
Requires(post): desktop-file-utils
%endif
%if 0%{?scientificlinux_version}
BuildRequires:  python-gobject
BuildRequires:  python-gobject-base
%endif

# POV-Ray dependencies - SUSE/CentOS builds
%if 0%{?suse_version} || 0%{?sle_version} || 0%{?centos_version} || 0%{?rhel_version} || 0%{?scientificlinux_version}
BuildRequires: autoconf
BuildRequires: automake
BuildRequires:  gcc-c++
BuildRequires:  dos2unix
BuildRequires:  libjpeg-devel
BuildRequires:  libpng-devel
BuildRequires:  libtiff-devel
BuildRequires:  libSM-devel
BuildRequires:  pkgconfig(zlib)
%if 0%{?suse_version} || 0%{?sle_version}
BuildRequires:  fdupes
%endif
%if 0%{?suse_version}>1325
BuildRequires:  libboost_system-devel
BuildRequires:  libboost_thread-devel
%else
BuildRequires:  boost-devel
%endif
%if 0%{?suse_version}
BuildRequires:  xorg-x11-libX11-devel
BuildRequires:  xorg-x11-libXpm-devel
%else
BuildRequires:  libXpm-devel
%endif
%if !0%{?rhel_version}
BuildRequires:  pkgconfig(OpenEXR)
%endif
%if (0%{?suse_version}!=1315 && !0%{?centos_version} && !0%{?rhel_version} && !0%{?scientificlinux_version})
BuildRequires:  pkgconfig(sdl2)
%endif
%endif

# ------------------------------
# Build from source dependencies
# ------------------------------
# OSMesa and libGLU dependencies
%if 0%{?build_osmesa}
%define buildosmesa yes
# libGLU build-from-source dependencies
BuildRequires:  gcc-c++
BuildRequires:  libtool
BuildRequires:  pkgconfig
BuildRequires:  pkgconfig(gl)
# libMesa build-from-source dependencies
%define _name_archive mesa
%define _version 17.2.6
BuildRequires:  autoconf >= 2.60
BuildRequires:  automake
BuildRequires:  bison
%if !0%{?rhel_version}
BuildRequires:  fdupes
%endif
BuildRequires:  flex
BuildRequires:  gcc-c++
BuildRequires:  imake
BuildRequires:  libtool
BuildRequires:  pkgconfig
%if 0%{?fedora} || 0%{?rhel_version} || 0%{?scientificlinux_version}
BuildRequires:  python
BuildRequires:  python-libs
BuildRequires:  pkgconfig(libdrm)
%else
BuildRequires:  python-xml
BuildRequires:  python-base
BuildRequires:  pkgconfig(libdrm) >= 2.4.75
%endif
BuildRequires:  python-mako
BuildRequires:  pkgconfig(dri2proto)
BuildRequires:  pkgconfig(dri3proto)
BuildRequires:  pkgconfig(expat)
BuildRequires:  pkgconfig(glproto)
BuildRequires:  pkgconfig(libkms) >= 1.0.0
BuildRequires:  pkgconfig(libudev) > 151
BuildRequires:  pkgconfig(openssl)
BuildRequires:  pkgconfig(presentproto)
%if !0%{?rhel_version}
BuildRequires:  pkgconfig(libva)
BuildRequires:  pkgconfig(vdpau) >= 1.1
BuildRequires:  pkgconfig(xcb-dri3)
BuildRequires:  pkgconfig(xcb-present)
BuildRequires:  pkgconfig(xshmfence)
BuildRequires:  pkgconfig(xvmc)
%endif
BuildRequires:  pkgconfig(x11)
BuildRequires:  pkgconfig(x11-xcb)
BuildRequires:  pkgconfig(xcb-dri2)
BuildRequires:  pkgconfig(xcb-glx)
BuildRequires:  pkgconfig(xdamage)
BuildRequires:  pkgconfig(xext)
BuildRequires:  pkgconfig(xfixes)
BuildRequires:  pkgconfig(xxf86vm)
BuildRequires:  pkgconfig(zlib)
%ifarch x86_64 %ix86
%if 0%{?fedora} || 0%{?rhel_version}
BuildRequires:  elfutils
BuildRequires:  elfutils-libelf-devel
BuildRequires:  libdrm-devel
%else
BuildRequires:  libelf-devel
BuildRequires:  pkgconfig(libdrm_intel) >= 2.4.75
%endif
%else
%endif
%if 0%{?suse_version} > 1320 || (0%{?sle_version} >= 120300 && 0%{?is_opensuse}) || 0%{?scientificlinux_version} == 700
# needed by gtk3
%if 0%{?scientificlinux_version}==700
BuildRequires:  wayland-devel
%endif
BuildRequires:  pkgconfig(wayland-client) >= 1.11
BuildRequires:  pkgconfig(wayland-protocols) >= 1.8
BuildRequires:  pkgconfig(wayland-server) >= 1.11
%endif
%ifarch %ix86 x86_64
%if !0%{?rhel_version}
BuildRequires:  llvm-devel
%endif
BuildRequires:  ncurses-devel
%endif
%endif

# SDL2 debendencies
%if 0%{?build_sdl2}
%define builsdl2 yes
BuildRequires:  cmake
BuildRequires:  dos2unix
BuildRequires:  gcc-c++
BuildRequires:  nasm
BuildRequires:  pkg-config
BuildRequires:  pkgconfig(alsa) >= 0.9.0
BuildRequires:  pkgconfig(dbus-1)
%if 0%{?fedora}
BuildRequires:  pkgconfig(fcitx)
%endif
%if !0%{?rhel_version} == 600
BuildRequires:  pkgconfig(egl)
%endif
BuildRequires:  pkgconfig(gl)
%if !0%{?centos_version}
%if !0%{?rhel_version} && !0%{?scientificlinux_version}
BuildRequires:  pkgconfig(glesv1_cm)
BuildRequires:  pkgconfig(wayland-server)
%endif
%if !0%{?rhel_version} && !0%{?scientificlinux_version} == 600
BuildRequires:  pkgconfig(glesv2)
%endif
%endif
BuildRequires:  pkgconfig(glib-2.0)
BuildRequires:  pkgconfig(glu)
BuildRequires:  pkgconfig(ice)
# KMS/DRM driver needs libdrm and libgbm
%if !0%{?rhel_version}
BuildRequires:  pkgconfig(ibus-1.0)
BuildRequires:  pkgconfig(gbm) >= 9.0.0
%endif
%if 0%{?rhel_version} == 600 || 0%{?scientificlinux_version} == 600
BuildRequires:  pkgconfig(libdrm)
%else
BuildRequires:  pkgconfig(libdrm) >= 2.4.46
%endif
%if 0%{?suse_version} > 1220
BuildRequires:  pkgconfig(tslib)
%endif
BuildRequires:  pkgconfig(libpulse-simple) >= 0.9
BuildRequires:  pkgconfig(libudev)
BuildRequires:  pkgconfig(udev)
BuildRequires:  pkgconfig(x11)
BuildRequires:  pkgconfig(xcursor)
BuildRequires:  pkgconfig(xext)
BuildRequires:  pkgconfig(xi)
BuildRequires:  pkgconfig(xinerama)
BuildRequires:  pkgconfig(xrandr)
BuildRequires:  pkgconfig(xscrnsaver)
BuildRequires:  pkgconfig(xxf86vm)
%endif

# configuration settings
%description
 LPub3D is an Open Source WYSIWYG editing application for creating
 LEGO® style digital building instructions. LPub3D is developed and
 maintained by Trevor SANDY. It uses the LDraw™ parts library, the
 most comprehensive library of digital Open Source LEGO® bricks
 available (www.ldraw.org/ ) and reads the LDraw LDR and MPD model
 file formats. LPub3D is available for free under the GNU Public License v3
 and runs on Windows, Linux and macOS Operating Systems.
 LPub3D is also availabe as a Linux 'no-install', multi-platform AppImage.
 Portions of LPub3D are based on LPUB© 2007-2009 Kevin Clague,
 LeoCAD© 2020 Leonardo Zide.and additional third party components.
 LEGO® is a trademark of the LEGO Group of companies which does not
 sponsor, authorize or endorse this application.
 © 2015-2021 Trevor SANDY

# workaround from fc27 onwards build error: Empty files file /home/abuild/rpmbuild/BUILD/lpub3d-git/debugsourcefiles.list
%if 0%{?fedora_version}>=27
%global debug_package %{nil}
%endif

%prep
set +x
%if 0%{?suse_version} || 0%{?sle_version}
echo "%{suse_dist_label}"
%endif
%if 0%{?centos_ver}
echo "CentOS.........................%{centos_version}"
%endif
%if 0%{?fedora}
echo "Fedora.........................%{fedora_version}"
%endif
%if 0%{?rhel_version}
echo "RedHat Enterprise Linux........%{rhel_version}"
%endif
%if 0%{?scientificlinux_version}
echo "Scientific Linux...............%{scientificlinux_version}"
%endif
%if 0%{?mageia}
echo "Mageia.........................%{mageia_version}"
%endif
%if 0%{?buildservice}
echo "Using OpenBuildService.........%{usingbuildservice}"
%endif
echo "Target.........................%{_target}"
echo "Target Vendor..................%{_target_vendor}"
echo "Target CPU.....................%{_target_cpu}"
echo "Name...........................%{name}"
echo "Summary........................%{summary}"
echo "Version........................%{version}"
echo "Vendor.........................%{vendor}"
echo "Release........................%{release}"
echo "Distribution packer............%{distpacker}"
echo "Source0........................%{SOURCE0}"
echo "Source20.......................%{SOURCE10}"
echo "Service Provider...............%{serviceprovider}"
echo "Packing Platform...............%{packingplatform}"
echo "Build Package..................%{name}-%{version}-%{release}-%{_arch}.rpm"
set -x
%setup -q -n %{name}-git

%build
echo "Current working directory: $PWD"
# copy ldraw archive libraries
LDrawLibOffical=../../SOURCES/complete.zip
LDrawLibUnofficial=../../SOURCES/lpub3dldrawunf.zip
LDrawLibTENTE=../../SOURCES/tenteparts.zip
LDrawLibVEXIQ=../../SOURCES/vexiqparts.zip
if [ -f ${LDrawLibOffical} ] ; then
  cp ${LDrawLibOffical} mainApp/extras && echo "LDraw archive library complete.zip copied to $(readlink -e mainApp/extras)"
  cp ${LDrawLibOffical} ../ && echo "LDraw archive library complete.zip copied to $(readlink -e ../)"
else
  echo "LDraw archive library complete.zip not found at $(readlink -e ../SOURCES)!"
fi
if [ -f ${LDrawLibUnofficial} ] ; then
  cp ${LDrawLibUnofficial} mainApp/extras && echo "LDraw archive library complete.zip copied to $(readlink -e mainApp/extras)"
else
  echo "LDraw archive library lpub3dldrawunf.zip not found at $(readlink -e ../SOURCES)!"
fi
if [ -f ${LDrawLibTENTE} ] ; then
  cp ${LDrawLibTENTE} mainApp/extras && echo "LDraw archive library tenteparts.zip copied to $(readlink -e mainApp/extras)"
else
  echo "LDraw archive library tenteparts.zip not found at $(readlink -e ../SOURCES)!"
fi
if [ -f ${LDrawLibVEXIQ} ] ; then
  cp ${LDrawLibVEXIQ} mainApp/extras && echo "LDraw archive library vexiqparts.zip copied to $(readlink -e mainApp/extras)"
else
  echo "LDraw archive library vexiqparts.zip not found at $(readlink -e ../SOURCES)!"
fi
# Copy 3rd party renderer source archives and Qt5 libraries
for TarballFile in \
  ../../SOURCES/ldglite.tar.gz \
  ../../SOURCES/ldview.tar.gz \
  ../../SOURCES/povray.tar.gz \
  ../../SOURCES/mesa-17.2.6.tar.gz \
  ../../SOURCES/mesa-18.3.5.tar.gz \
  ../../SOURCES/glu-9.0.0.tar.bz2 \
  ../../SOURCES/qt5-5.9.3-gcc_64-el.tar.gz \
  ../../SOURCES/locallibs.el.x86_64.tar.gz; do
  if [ -f "${TarballFile}" ]; then
    mv -f ${TarballFile} ../ && echo "$(basename ${TarballFile}) moved to $(readlink -e ../)"
  fi
done
# OBS Platform id and version
%if 0%{?buildservice}
%if 0%{?suse_version} || 0%{?sle_version}
export PLATFORM_PRETTY_OBS="%{suse_dist_pretty_name}"
export PLATFORM_VER_OBS=%{suse_dist_version}
export PLATFORM_CODE=%{suse_platform_code}
%endif
%if 0%{?centos_ver}
export PLATFORM_PRETTY_OBS="CentOS"
export PLATFORM_VER_OBS=%{centos_version}
export PLATFORM_CODE="cos"
%endif
%if 0%{?fedora}
export PLATFORM_PRETTY_OBS="Fedora"
export PLATFORM_VER_OBS=%{fedora_version}
export PLATFORM_CODE="fc"
%endif
%if 0%{?rhel_version}
export PLATFORM_PRETTY_OBS="RedHat Enterprise Linux"
export PLATFORM_VER_OBS=%{rhel_version}
export PLATFORM_CODE="rh"
%endif
%if 0%{?scientificlinux_version}
export PLATFORM_PRETTY_OBS="Scientific Linux"
export PLATFORM_VER_OBS=%{scientificlinux_version}
export PLATFORM_CODE="sl"
%endif
%if 0%{?mageia}
export PLATFORM_PRETTY_OBS="Mageia"
export PLATFORM_VER_OBS=%{mageia_version}
export PLATFORM_CODE="mga"
%endif
export PLATFORM_VER=${PLATFORM_VER_OBS}
set +x
# 3rd-party renderers build-from-source requirements
%if 0%{?build_osmesa}
echo "Build OSMesa from source.......yes"
export build_osmesa=%{build_osmesa}
%endif
%if 0%{?no_gallium}
echo "Gallium driver not available...yes"
export no_gallium=%{no_gallium}
%endif
%if 0%{?build_sdl2}
echo "Build SDL2 from source.........yes"
export build_sdl2=%{build_sdl2}
%endif
%if 0%{?build_gl2ps}
echo "Build GL2PS from source........yes"
export build_gl2ps=%{build_gl2ps}
%endif
%if 0%{?build_tinyxml}
echo "Build TinyXML from source......yes"
export build_tinyxml=%{build_tinyxml}
%endif
%if 0%{?get_qt5}
echo "Get Qt5 library................yes"
export get_qt5=%{get_qt5}
%endif
%if 0%{?get_local_libs}
echo "Get local libraries............yes"
export get_local_libs=%{get_local_libs}
%endif
set -x
%endif
# Indicate OBS status (should always be yes for this spec file)
export OBS=%{usingbuildservice}
# RPM exported variables
export TARGET_VENDOR=%{_target_vendor}
export TARGET_CPU=%{_target_cpu}
export RPM_LIBDIR="%{_libdir}"
export RPM_SYSCONFDIR="%{_sysconfdir}"
export RPM_OPTFLAGS="%{optflags}"
export RPM_BUILD=true
export RPM_STAGE=build
# instruct qmake to install 3rd-party renderers
export LP3D_BUILD_PKG=yes
# set Qt5 - Download Qt5 Library for CentOS 6, RHEL and Scientific Linux - these platforms don't have Qt5
%if 0%{?get_qt5}
source builds/linux/obs/alldeps/GetQt5Libs.sh
%else
export QT_SELECT=qt5
%endif
# set OSMesa, LLVM, OpenEXR and their dependent libs from locallibs.el.x86_64.tar.gz tarball - for RHEL
%if 0%{?get_local_libs}
source builds/linux/obs/alldeps/GetLocalLibs.sh
%endif
# build 3rd-party renderers
export WD=$(readlink -e ../); \
chmod a+x builds/utilities/CreateRenderers.sh && ./builds/utilities/CreateRenderers.sh
# option flags and qmake settings
%if 0%{?fedora_version}==23
%ifarch x86_64
export Q_CXXFLAGS="$Q_CXXFLAGS -fPIC"
%endif
%endif
# Qt setup
if which qmake-qt5 >/dev/null 2>/dev/null ; then
  QMAKE_EXEC=qmake-qt5
elif test -d "$LP3D_QT5_BIN" ; then
  QMAKE_EXEC=$LP3D_QT5_BIN/qmake
else
  QMAKE_EXEC=qmake
fi
echo && ${QMAKE_EXEC} -v && echo
# LDraw directory - build check
export LDRAWDIR=${HOME}/ldraw
# configure and build LPub3d
${QMAKE_EXEC} -makefile -nocache QMAKE_STRIP=: CONFIG+=release CONFIG+=build_check CONFIG-=debug_and_release CONFIG+=rpm DOCS_DIR=%{_docdir}/lpub3d
make clean
make %{?_smp_mflags}
# Check generated and updated config files
%if 0%{?get_qt5}
[ -f "mainApp/qt.conf" ] && echo "Check generated qt.conf..." && \
cat mainApp/qt.conf || echo "Could not find qt.conf"
[ -f "mainApp/lpub3d.qrc" ] && echo "Check updated lpub3d.qrc..." && \
tail mainApp/lpub3d.qrc || echo "Could not find lpub3d.qrc"
[ -f "mainApp/lpub3d-qtlibs.conf" ] && echo "Check generated qtlibs.conf..." && \
cat mainApp/lpub3d-qtlibs.conf || echo "Could not find lpub3d-qtlibs.conf"
%endif
%if 0%{?get_local_libs}
[ -f "mainApp/lpub3d-libs.conf" ] && echo "Check generated lpub3d-libs.conf..." && \
cat mainApp/lpub3d-libs.conf || echo "Could not find lpub3d-libs.conf"
echo "Check updated local library pc file..." && \
cat %{_builddir}/usr/lib64/pkgconfig/OpenEXR.pc || echo "Could not find %{_builddir}/usr/lib64/pkgconfig/OpenEXR.pc"
%endif
# set LDLibrary_Path if using local or custom libraries
%if 0%{?get_qt5} || 0%{?get_local_libs}
export LD_LIBRARY_PATH="$LD_LIBRARY_PATH:%{buildroot}%{_bindir}:%{buildroot}%{_libdir}"
%endif
# check lpub3d dependencies
lp3drelease="32bit_release" && [[ "${TARGET_CPU}" = "x86_64" || "${TARGET_CPU}" = "aarch64" ]] && lp3drelease="64bit_release"
versuffix=$(cat builds/utilities/version.info | cut -d " " -f 1-2 | sed s/" "//g)
validexe="mainApp/${lp3drelease}/lpub3d${versuffix}"
[ -f "${validexe}" ] && echo "LDD check lpub3d${versuffix}..." && ldd ${validexe} 2>/dev/null || \
echo "ERROR - LDD check failed for $(realpath ${validexe})"

%install
make INSTALL_ROOT=%buildroot install
%if 0%{?suse_version}
%suse_update_desktop_file lpub3d Graphics 3DGraphics Publishing Viewer Education Engineering
%endif
%if 0%{?suse_version} || 0%{?sle_version}
%fdupes %{buildroot}/%{_iconsdir}
# skip rpath check on 3rd-party binaries to avoid 'RPATH "" ... is not allowed' fail on SUSE builds
export NO_BRP_CHECK_RPATH=true
%endif

%clean
rm -rf $RPM_BUILD_ROOT

%files
%if 0%{?sle_version} || 0%{?suse_version}
%defattr(-,root,root)
%endif
%{_bindir}/*
%{_datadir}/lpub3d/*
%{_datadir}/icons/*
#  {_datadir}/metainfo/*
%{_datadir}/mime/packages/*
%{_datadir}/applications/*
%attr(755,-,-) %{_3rdexedir}/*
%attr(644,-,-) %{_mandir}/man1/*
%attr(644,-,-) %doc %{_docdir}/lpub3d/*
%attr(644,-,-) %{_iconsdir}/hicolor/scalable/mimetypes/*
%if 0%{?get_qt5} || 0%{?get_local_libs}
%{_libdir}/*
%config(noreplace) %{_sysconfdir}/ld.so.conf.d/*
%if 0%{?get_local_libs}
%config(noreplace) %{_exec_prefix}/lib/udev/rules.d/*
%endif
%endif

%post -p /sbin/ldconfig
%if 0%{?buildservice}!=1
update-mime-database  /usr/share/mime >/dev/null || true
update-desktop-database || true
%endif

%postun -p /sbin/ldconfig
%if 0%{?buildservice}!=1
update-mime-database /usr/share/mime >/dev/null || true
update-desktop-database || true
%endif

* Wed May 25 2022 - trevor.dot.sandy.at.gmail.dot.com 2.4.4.2596
- LPub3D Linux package (rpm) release
