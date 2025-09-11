#
# spec file for LPub3D package
#
# Last Update: September 11, 2025
# Copyright © 2017 - 2025 Trevor SANDY
# Using RPM Spec file examples by Thomas Baumgart, Peter Bartfai and others
# This file and all modifications and additions to the pristine
# package are under the same license as the package itself.
#
# please send bugfixes or comments to Trevor SANDY <trevor.sandy@gmail.com>

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
# distinguish between openSUSE Leap, openSUSE, and SUSE Linux Enterprise Server
# openSUSE Leap Factory and versions equal to or greater than 16.0
%if (0%{?suse_version}>1599 && 0%{?suse_version}<=1699 && 0%{?is_opensuse})
%define dist .openSUSELeap%(echo %{suse_version} | sed 's/0$//')
%if (0%{?suse_version}==1699)
%define suse_dist_name %(echo openSUSE Leap Factory)
%define suse_dist_label %(echo %{suse_dist_name}..........%{suse_version})
%define suse_code osf
%else
%define suse_dist_name %(echo openSUSE Leap)
%define suse_dist_label %(echo %{suse_dist_name}..................%{suse_version})
%define suse_code osl
%endif
%define suse_dist_pretty_name %(echo %{suse_dist_name} %{suse_version})
%define suse_dist_version %{suse_version}
%define suse_platform_code %{suse_code}
%else
# openSUSE Leap up to 15.6
%if (0%{?sle_version}>=120000 && 0%{?sle_version}<=150600 && 0%{?is_opensuse})
%define dist .openSUSELeap%(echo %{sle_version} | sed 's/0$//')
%define suse_dist_name %(echo openSUSE Leap)
%define suse_dist_label %(echo %{suse_dist_name}..................%{sle_version})
%define suse_dist_pretty_name %(echo %{suse_dist_name} %{sle_version})
%define suse_dist_version %{sle_version}
%define suse_platform_code osl
%define build_sdl2 1
%else
# openSUSE
%if (0%{?suse_version}>=1320 && 0%{?suse_version}<=1560)
%define dist .openSUSE%(echo %{suse_version} | sed 's/0$//')
%define suse_dist_name %(echo openSUSE)
%define suse_dist_label %(echo %{suse_dist_name}.......................%{suse_version})
%define suse_dist_pretty_name %(echo %{suse_dist_name} %{suse_version})
%define suse_dist_version %{suse_version}
%define suse_platform_code os
%else
# SUSE Linux Enterprise Server
%if (0%{?sle_version}>=120000 && 0%{?sle_version}<=150600 && !0%{?is_opensuse})
%define dist .SLE%(echo %{sle_version} | sed 's/0$//')
%define suse_dist_name %(echo SUSE Linux Enterprise Server)
%define suse_dist_label %(echo %{suse_dist_name}...%{sle_version})
%define suse_dist_pretty_name %(echo %{suse_dist_name} %{sle_version})
%define suse_dist_version %{sle_version}
%define suse_platform_code sle
%define build_sdl2 1
%endif
%endif
%endif
%endif

# Until LDView converts to tinyxml2, build tinyxml from source
%define build_tinyxml 1

%if 0%{?rhel_version}
%define build_sdl2 1
%define get_local_libs 1
%endif

%if 0%{?centos_version}==800
%define get_local_libs 1
%define skip_local_POVRay_libs 1
%endif

%if 0%{?almalinux_version}
%define local_freeglut 1
%endif

%if 0%{?fedora}
%define fedora_version %{fedora}
%endif

# distro group settings
%if 0%{?suse_version} || 0%{?sle_version}
Group: Productivity/Graphics/Viewers
%endif

%if 0%{?rhel_version} || 0%{?openeuler_version} || 0%{?almalinux_version}
Group: Graphics
%endif

%if 0%{?centos_version} || 0%{?fedora_version}
Group: Amusements/Graphics
%endif

%if 0%{?centos_version} || 0%{?fedora_version} || 0%{?rhel_version} || 0%{?openeuler_version} || 0%{?almalinux_version}
License: GPLv2+
%endif

%if 0%{?suse_version} || 0%{?sle_version}
License: GPL-2.0+
BuildRequires: fdupes
%endif

# set directory paths
%if 0%{?buildservice}
%define _lp3d_3rd_exec_dir /usr/bin/lpub3d/3rdParty
%endif
%define _iconsdir %{_datadir}/icons

# preamble
Summary: An LDraw Building Instruction Editor
Name: lpub3d
Icon: lpub3d.xpm
Version: 2.4.9.4492
Release: <B_CNT>%{?dist}
URL: https://trevorsandy.github.io/lpub3d
Vendor: Trevor SANDY
BuildRoot: %{_builddir}/%{name}
BuildRequires: unzip
Source0: lpub3d-git.tar.gz
Source10: lpub3d-rpmlintrc

# package requirements
%if 0%{?fedora_version} || 0%{?centos_version}>=700 || 0%{?rhel_version}>=800 || 0%{?openeuler_version} || 0%{?almalinux_version}
BuildRequires: qt6-qtbase-devel, qt6-qttools-devel, qt6-qt5compat-devel
%endif

%if 0%{?fedora_version} || 0%{?centos_version} || 0%{?rhel_version} || 0%{?openeuler_version} || 0%{?almalinux_version}
BuildRequires: hostname
%if !0%{?rhel_version}
BuildRequires: OpenEXR-devel
%if 0%{?centos_version}!=800
BuildRequires: mesa-libOSMesa-devel
%endif
%endif
%if 0%{?local_freeglut}
BuildRequires: mesa-libGL-devel
BuildRequires: libXxf86vm-devel
BuildRequires: libXrandr-devel
BuildRequires: libXi-devel
%else
BuildRequires: freeglut-devel
%endif
BuildRequires: mesa-libGLU-devel
BuildRequires: boost-devel, libtiff-devel
BuildRequires: gcc-c++, make, libpng-devel
%if 0%{?buildservice}!=1
BuildRequires: git
%endif
%endif

%if 0%{?centos_version} || 0%{?rhel_version} || 0%{?openeuler_version} || 0%{?almalinux_version}
%if 0%{?openeuler_version}
BuildRequires: gnu-free-sans-fonts
%endif
%if 0%{?centos_version}==800
BuildRequires: mesa-libwayland-egl
%endif
BuildRequires: libjpeg-turbo-devel
%if 0%{?openeuler_version} || 0%{?almalinux_version} || 0%{?centos_version}>700
BuildRequires: libXext-devel
%endif
%if 0%{?centos_version}==700
%define build_sdl2 1
%endif
%define build_gl2ps 1
%endif

%if 0%{?fedora_version}
BuildRequires: libjpeg-turbo-devel, gl2ps-devel
BuildRequires: SDL2-devel
%if 0%{?fedora_version}>30
BuildRequires: autoconf >= 2.69
BuildRequires: automake
BuildRequires: libXext-devel
%endif
%if 0%{?buildservice}
BuildRequires: samba4-libs
%endif
%endif

%if 0%{?suse_version} || 0%{?sle_version}
%if (0%{?sle_version}!=150000)
BuildRequires: freeglut-devel
%endif
# exclude libOSMesa from openSUSE:Leap:Factory - suse_version 1699
%if (0%{?suse_version}==1699)
# set platform flags that will build OSMesa from Mesa-Amber - Mesa 21.3.9
%define build_osmesa 1
%define mesa_amber 1
# LLVM is not needed for default OSMesa-amber configuration
%define with_llvm 0
%else
BuildRequires: libOSMesa-devel
# update_desktop_file is deprecated
BuildRequires: update-desktop-files
%endif
BuildRequires: glu-devel, openexr-devel
BuildRequires: libpng16-compat-devel, libjpeg8-devel
BuildRequires: hostname
BuildRequires: zlib-devel
BuildRequires: Mesa-libEGL-devel
%if (0%{?suse_version}>1210 && 0%{?suse_version}!=1315 && 0%{?sle_version}!=150000 && 0%{?sle_version}!=150100 && 0%{?sle_version}!=150200 && 0%{?sle_version}!=150300 && 0%{?sle_version}!=150400)
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
%endif

# magia build on OBS only goes to Qt5


%if 0%{?sle_version}
%define osmesa_found %(test -f /usr/lib/libOSMesa.so -o -f /usr/lib64/libOSMesa.so && echo 1 || echo 0)
%if 0%{osmesa_found} != 1
%define build_osmesa 1
%endif
%if 0%{?buildservice}
BuildRequires: -post-build-checks
%endif
BuildRequires: hostname
Requires(post): desktop-file-utils
%endif

# -----minizip dependency------
%if ( 0%{?fedora} || 0%{?centos_version}>=700 || 0%{?rhel_version}>=700 || 0%{?almalinux})
BuildRequires: minizip-compat-devel
%endif

%if ( 0%{?suse_version} || 0%{?openeuler_version} )
BuildRequires: minizip-devel
%endif

# -----------------------------

# POV-Ray dependencies - SUSE/CentOS builds
%if 0%{?suse_version} || 0%{?sle_version} || 0%{?centos_version} || 0%{?rhel_version} || 0%{?openeuler_version} || 0%{?almalinux_version}
BuildRequires: autoconf
BuildRequires: automake
BuildRequires: gcc-c++
BuildRequires: dos2unix
BuildRequires: libjpeg-devel
BuildRequires: libpng-devel
BuildRequires: libtiff-devel
BuildRequires: libSM-devel
BuildRequires: pkgconfig(zlib)
%if 0%{?suse_version} || 0%{?sle_version}
BuildRequires: fdupes
%endif
%if 0%{?suse_version}>1325
BuildRequires: libboost_system-devel
BuildRequires: libboost_thread-devel
%else
BuildRequires: boost-devel
%endif
%if 0%{?suse_version}
BuildRequires: xorg-x11-libX11-devel
BuildRequires: xorg-x11-libXpm-devel
%else
BuildRequires: libXpm-devel
%endif
%if !0%{?rhel_version}
BuildRequires: pkgconfig(OpenEXR)
%endif
# We are building sdl2 for these instances, so do not load here
%if (0%{?suse_version}!=1315 && !0%{?rhel_version} && 0%{?centos_version}>700)
BuildRequires: pkgconfig(sdl2)
%endif
%endif

# ------------------------------
# Build from source dependencies
# ------------------------------
# Mesa and libGLU dependencies
%if 0%{?build_osmesa}
# libGLU build-from-source dependencies
BuildRequires: gcc-c++
BuildRequires: libtool
BuildRequires: pkgconfig
BuildRequires: pkgconfig(gl)
# libMesa build-from-source dependencies
%ifarch armv6l armv6hl
%define _lto_cflags %{nil}
%endif
# ---
%define drivers 0
%define glamor 1
%define _name_archive mesa
%define _version 21.3.9
%define with_opencl 0
%define with_vulkan 0
%define with_llvm 0
# ---
%ifarch %{ix86} x86_64 %{arm} aarch64
%define gallium_loader 1
%else
%define gallium_loader 0
%endif
# ---
%define xvmc_support 0
%define vdpau_nouveau 0
%define vdpau_radeon 0
# ---
%ifarch %{ix86} x86_64 aarch64 %{arm}
%define xvmc_support 1
%define vdpau_nouveau 1
%define vdpau_radeon 1
%endif
# ---
%ifarch %{ix86} x86_64 %{arm} aarch64
%define with_nine 1
%endif
# ---
%if 0%{gallium_loader}
%define with_opencl 1
%ifarch %{ix86} x86_64
%define with_vulkan 1
%define vulkan_drivers swrast,amd,intel
%endif
%ifarch %{arm} aarch64
%define with_vulkan 1
%define vulkan_drivers swrast,amd,broadcom,freedreno
%endif
%endif
# ---
%ifarch aarch64 %{arm}  s390x %{ix86} x86_64
%define with_llvm 1
%endif
# ---
%if 0%{with_opencl}
%define have_gallium 1
%else
%define have_gallium 0
%endif
# ---
%if %{drivers}
%define glamor 0
%else
# No llvm dependencies
%define with_llvm 0
# OpenCL requires clang (LLVM)
%define with_opencl 0
# nine requires at least one non-swrast gallium driver
%define with_nine 0
# Not built because radeon driver is not built.
%define vdpau_radeon 0
# Not built because nouveau driver is not built.
%define vdpau_nouveau 0
# Not built. (Why?)
%define xvmc_support 0
# Vulkan includes radv driver which requires llvm
%define with_vulkan 0
%endif
# ---
%if !0%{?rhel_version}
BuildRequires: fdupes
%endif
BuildRequires:  bison
BuildRequires:  fdupes
BuildRequires:  flex
BuildRequires:  gcc-c++
BuildRequires:  glslang-devel
BuildRequires:  imake
BuildRequires:  libtool
BuildRequires:  memory-constraints
BuildRequires:  meson
BuildRequires:  pkgconfig
BuildRequires:  python3-base
%if 0%{?suse_version} > 1320
BuildRequires:  python3-mako
%else
BuildRequires:  python3-Mako
%endif
BuildRequires:  python3-xml
BuildRequires:  pkgconfig(dri2proto)
BuildRequires:  pkgconfig(dri3proto)
BuildRequires:  pkgconfig(expat)
BuildRequires:  pkgconfig(glproto)
BuildRequires:  pkgconfig(libdrm) >= 2.4.75
BuildRequires:  pkgconfig(libdrm_amdgpu) >= 2.4.95
BuildRequires:  pkgconfig(libdrm_nouveau) >= 2.4.66
BuildRequires:  pkgconfig(libdrm_radeon) >= 2.4.71
BuildRequires:  pkgconfig(libglvnd) >= 0.1.0
%ifarch aarch64 %{ix86} x86_64 ppc64le s390x
BuildRequires:  pkgconfig(valgrind)
%endif
%if !0%{?suse_version} == 1699
BuildRequires:  pkgconfig(libkms) >= 1.0.0
%endif
BuildRequires:  pkgconfig(libva)
BuildRequires:  pkgconfig(presentproto)
%if %{drivers}
BuildRequires:  pkgconfig(vdpau) >= 1.1
%endif
BuildRequires:  pkgconfig(x11)
BuildRequires:  pkgconfig(x11-xcb)
BuildRequires:  pkgconfig(xcb-dri2)
BuildRequires:  pkgconfig(xcb-dri3)
BuildRequires:  pkgconfig(xcb-glx)
BuildRequires:  pkgconfig(xcb-present)
BuildRequires:  pkgconfig(xdamage)
BuildRequires:  pkgconfig(xext)
BuildRequires:  pkgconfig(xfixes)
BuildRequires:  pkgconfig(xrandr)
BuildRequires:  pkgconfig(xshmfence)
BuildRequires:  pkgconfig(xvmc)
BuildRequires:  pkgconfig(xxf86vm)
BuildRequires:  pkgconfig(zlib)
%ifarch %{arm} aarch64
BuildRequires:  pkgconfig(libdrm_etnaviv) >= 2.4.89
BuildRequires:  pkgconfig(libdrm_freedreno) >= 2.4.74
BuildRequires:  pkgconfig(libelf)
%endif
%ifarch x86_64 %{ix86}
%if 0%{?fedora_version} || 0%{?rhel_version}
BuildRequires: elfutils
BuildRequires: elfutils-libelf-devel
BuildRequires: libdrm-devel
%else
BuildRequires: libelf-devel
BuildRequires: pkgconfig(libdrm_intel) >= 2.4.75
%endif
%else
%if 0%{with_opencl}
BuildRequires:  libelf-devel
%endif
%endif
BuildRequires:  pkgconfig(wayland-client) >= 1.11
BuildRequires:  pkgconfig(wayland-protocols) >= 1.8
BuildRequires:  pkgconfig(wayland-server) >= 1.11
%if 0%{with_llvm}
%if 0%{?suse_version} >= 1550
BuildRequires:  llvm-devel
%else
%if 0%{?sle_version} >= 150300
BuildRequires:  llvm11-devel
%else
BuildRequires:  llvm9-devel
%endif
%endif
%ifarch %ix86 x86_64
BuildRequires: ncurses-devel
%endif
%endif
# ---
%if 0%{with_opencl}
%if 0%{?suse_version} >= 1550
BuildRequires:  clang-devel
%else
%if 0%{?sle_version} >= 150300
BuildRequires:  clang11-devel
%else
BuildRequires:  clang9-devel
%endif
%endif
BuildRequires:  libclc
%endif
%endif

# SDL2 debendencies
%if 0%{?build_sdl2}
%define builsdl2 yes
BuildRequires: cmake
BuildRequires: dos2unix
BuildRequires: gcc-c++
%if 0%{?rhel_version}!=800
BuildRequires: nasm
%endif
BuildRequires: pkg-config
BuildRequires: pkgconfig(alsa) >= 0.9.0
BuildRequires: pkgconfig(dbus-1)
%if 0%{?fedora_version}
BuildRequires: pkgconfig(fcitx)
%endif
%if 0%{?rhel_version}!=600
BuildRequires: pkgconfig(egl)
%endif
BuildRequires: pkgconfig(gl)
%if !0%{?centos_version}
%if !0%{?rhel_version}
BuildRequires: pkgconfig(glesv1_cm)
BuildRequires: pkgconfig(wayland-server)
%endif
%if !0%{?rhel_version}
BuildRequires: pkgconfig(glesv2)
%endif
%endif
BuildRequires: pkgconfig(glib-2.0)
BuildRequires: pkgconfig(glu)
BuildRequires: pkgconfig(ice)
# KMS/DRM driver needs libdrm and libgbm
%if !0%{?rhel_version}
BuildRequires: pkgconfig(ibus-1.0)
BuildRequires: pkgconfig(gbm) >= 9.0.0
%endif
%if 0%{?rhel_version}==600
BuildRequires: pkgconfig(libdrm)
%else
BuildRequires: pkgconfig(libdrm) >= 2.4.46
%endif
%if 0%{?suse_version}>1220
BuildRequires: pkgconfig(tslib)
%endif
BuildRequires: pkgconfig(libpulse-simple) >= 0.9
BuildRequires: pkgconfig(libudev)
BuildRequires: pkgconfig(udev)
BuildRequires: pkgconfig(x11)
BuildRequires: pkgconfig(xcursor)
BuildRequires: pkgconfig(xext)
BuildRequires: pkgconfig(xi)
BuildRequires: pkgconfig(xinerama)
BuildRequires: pkgconfig(xrandr)
BuildRequires: pkgconfig(xscrnsaver)
BuildRequires: pkgconfig(xxf86vm)
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
 LeoCAD© 2022 Leonardo Zide.and additional third party components.
 LEGO® is a trademark of the LEGO Group of companies which does not
 sponsor, authorize or endorse this application.
 Copyright © 2015 - 2025 Trevor SANDY

# workaround from fc27 onwards build error: Empty files file /home/abuild/rpmbuild/BUILD/lpub3d-git/debugsourcefiles.list
%if 0%{?fedora_version}>=27
%global debug_package %{nil}
%endif

%prep
set +x
%if 0%{?suse_version} || 0%{?sle_version}
echo "%{suse_dist_label}"
%endif
%if 0%{?centos_version}
echo "CentOS.........................%{centos_version}"
%endif
%if 0%{?fedora_version}
echo "Fedora.........................%{fedora_version}"
%endif
%if 0%{?almalinux_version}
echo "Alma Linux.....................%{almalinux_version}"
%endif
%if 0%{?openeuler_version}
echo "openEuler......................%{openeuler_version}"
%endif
%if 0%{?rhel_version}
echo "RedHat Enterprise Linux........%{rhel_version}"
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
set +x
echo "Build Working Directory: $PWD"
[ -f "../../SOURCES/complete.zip" ] && SrcPath=../../SOURCES || SrcPath=../../../SOURCES
# move ldraw archive libraries to extras
for LDrawLibFile in \
  ${SrcPath}/complete.zip \
  ${SrcPath}/lpub3dldrawunf.zip \
  ${SrcPath}/tenteparts.zip \
  ${SrcPath}/vexiqparts.zip; do
  LibFile="$(basename ${LDrawLibFile})"
  if [ -f "${LDrawLibFile}" ]; then
    if [ "${LibFile}" = "complete.zip" ]; then
      cp -f ${LDrawLibFile} ../ || \
      echo "Error: ${LibFile} copy to $(readlink -e ../) failed."
    fi
    mv -f ${LDrawLibFile} mainApp/extras/ || \
    echo "Error: ${LibFile} move to $(readlink -e mainApp/extras) failed."
  else
    echo "Error: ${LDrawLibFile} not found."
  fi
done
# move 3rd party renderer source archives and Qt libraries
for TarballFile in \
  ${SrcPath}/ldglite.tar.gz \
  ${SrcPath}/ldview.tar.gz \
  ${SrcPath}/povray.tar.gz \
  ${SrcPath}/mesa-21.3.9.tar.xz \
  ${SrcPath}/glu-9.0.1.tar.xz \
  ${SrcPath}/zstd-1.5.7.tar.gz; do
  LibFile="$(basename ${TarballFile})"
  if [ -f "${TarballFile}" ]; then
    mv -f ${TarballFile} ../ || \
    echo "Error: ${LibFile} move to $(readlink -e ../) failed."
  else
    echo "Error: ${TarballFile} not found."
  fi
done
# copy version.info to utilities
if [ -f "${SrcPath}/version.info" ]; then
  cp -f ${SrcPath}/version.info builds/utilities
  [ -f "builds/utilities/version.info" ] && \
  echo "Copied version.info into $(readlink -e builds/utilities)" || \
  echo "Error: version.info not copied to $(readlink -e builds/utilities)."
else
  echo "Error: ${SrcPath}/version.info not found."
fi
%if 0%{?suse_version}
# apply suse_update_desktop_file diff
categories="Categories=Graphics;3DGraphics;Publishing;Engineering;Graphics;Viewer;Education;"
sed "s/Categories=.*/${categories}/" -i mainApp/lpub3d.desktop
%endif
set -x
%if 0%{?buildservice}
# OBS Platform id and version
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
%if 0%{?fedora_version}
export PLATFORM_PRETTY_OBS="Fedora"
export PLATFORM_VER_OBS=%{fedora_version}
export PLATFORM_CODE="fc"
%endif
%if 0%{?almalinux_version}
export PLATFORM_PRETTY_OBS="Alma Linux"
export PLATFORM_VER_OBS=%{almalinux_version}
export PLATFORM_CODE="al"
%endif
%if 0%{?openeuler_version}
export PLATFORM_PRETTY_OBS="OpenEuler"
export PLATFORM_VER_OBS=%{openeuler_version}
export PLATFORM_CODE="oe"
%endif
%if 0%{?rhel_version}
export PLATFORM_PRETTY_OBS="RedHat Enterprise Linux"
export PLATFORM_VER_OBS=%{rhel_version}
export PLATFORM_CODE="rh"
%endif
export PLATFORM_VER=${PLATFORM_VER_OBS}
set +x
# 3rd-party renderers build-from-source requirements
%if 0%{?build_osmesa}
echo "Build OSMesa from source.......%([[ "%{mesa_amber}" == 1 ]] && echo Mesa-amber || echo Mesa)"
export build_osmesa=%{build_osmesa}
%if 0%{?mesa_amber}
export mesa_amber=%{mesa_amber}
%endif
echo "Build OSMesa with LLVM.........%([[ "%{with_llvm}" == 1 ]] && echo yes || echo no)"
%if 0%{?with_llvm}
export llvm_not_used=1
%endif
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
%if 0%{?prebuilt_3ds}
echo "Use prebuilt 3DS library.......yes"
export prebuilt_3ds=%{prebuilt_3ds}
%else
echo "Build 3DS from source..........yes"
%endif
%if 0%{?get_local_libs}
echo "Get local libraries............yes"
export get_local_libs=%{get_local_libs}
%endif
%if 0%{?local_freeglut}
echo "Use local freeglut library.....yes"
export local_freeglut=%{local_freeglut}
%endif
%if 0%{?skip_local_POVRay_libs}
echo "Skip local POVRay libraries....yes"
export skip_local_POVRay_libs=%{skip_local_POVRay_libs}
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
export LPUB3D=%{name}
export RPM_BUILD=true
export RPM_STAGE=build
export LDRAWDIR=${HOME}/ldraw
# set OSMesa, LLVM, OpenEXR and their dependent libs from locallibs.el.x86_64.tar.gz tarball
%if 0%{?get_local_libs}
source builds/linux/obs/alldeps/GetLocalLibs.sh
%endif
# remove lcLib pre-compiled header to enable build on hardened systems
%if 0%{?rhel_version}>700 || 0%{?centos_version}>700 || 0%{?openeuler_version}
source builds/linux/obs/alldeps/LcLibPCH.sh
%endif
# build 3rd-party renderers
export WD=$(readlink -e ../)
%if 0%{?buildservice}
%define _lp3d_cpu_cores %(echo `nproc`)
%define _lp3d_log_path %(echo `pwd`)
%define _lp3d_3rd_dist_dir %(echo lpub3d_linux_3rdparty)
%endif
export LP3D_LOG_PATH="%{_lp3d_log_path}"
export LP3D_CPU_CORES="%{_lp3d_cpu_cores}"
export LP3D_3RD_DIST_DIR="%{_lp3d_3rd_dist_dir}"
# parent path for 3rdParty distribution directory
export LP3D_DIST_DIR_PATH="$(cd ${SrcPath}/.. && pwd)"
chmod a+x builds/utilities/CreateRenderers.sh && ./builds/utilities/CreateRenderers.sh
# option flags and qmake settings
%if 0%{?fedora_version}==23
%ifarch x86_64
export Q_CXXFLAGS="$Q_CXXFLAGS -fPIC"
%endif
%endif
# append 3rdParty distribution directory
export LP3D_DIST_DIR_PATH="${LP3D_DIST_DIR_PATH}/${LP3D_3RD_DIST_DIR}"
# Qt setup
if which qmake6 >/dev/null 2>/dev/null ; then
  QMAKE_EXEC=qmake6
elif which qmake-qt5 >/dev/null 2>/dev/null ; then
  QMAKE_EXEC=qmake-qt5
elif test -d "$LP3D_QT5_BIN" ; then
  QMAKE_EXEC=$LP3D_QT5_BIN/qmake
elif which qmake >/dev/null 2>/dev/null ; then
  QMAKE_EXEC=qmake
fi
echo && ${QMAKE_EXEC} -v && echo
# configure and build LPub3d
${QMAKE_EXEC} -makefile -nocache CONFIG+=release CONFIG+=build_check CONFIG-=debug_and_release CONFIG+=rpm DOCS_DIR=%{_docdir}/lpub3d LPub3D.pro
make clean
make %{?_smp_mflags}
%if 0%{?get_local_libs}
[ -f "mainApp/lpub3d-libs.conf" ] && echo "Check generated lpub3d-libs.conf..." && \
cat mainApp/lpub3d-libs.conf || echo "ERROR - Could not find lpub3d-libs.conf"
%if !0%{?skip_local_POVRay_libs}
echo "Check updated local OpenEXR library pc file..." && \
cat %{_builddir}/usr/lib64/pkgconfig/OpenEXR.pc || \
echo "ERROR - Could not find %{_builddir}/usr/lib64/pkgconfig/OpenEXR.pc"
%endif
# set LDLibrary_Path if using local or custom libraries
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
%if 0%{?suse_version} || 0%{?sle_version}
%fdupes %{buildroot}/%{_iconsdir}
# skip rpath check on 3rd-party binaries to avoid 'RPATH "" ... is not allowed' fail on SUSE builds
export NO_BRP_CHECK_RPATH=true
%endif

# this superfluous clean should not be used any longer. RPM provides its own clean logic
%if 0%{?suse_version}<1699
%clean
rm -rf $RPM_BUILD_ROOT
%endif

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
%if %([ "`ls -A %{_lp3d_3rd_exec_dir} 2>/dev/null`" ] && echo 1 || echo 0)
%attr(755,-,-) %{_lp3d_3rd_exec_dir}/*
%endif
%attr(644,-,-) %{_mandir}/man1/*
%attr(644,-,-) %doc %{_docdir}/lpub3d/*
%attr(644,-,-) %{_iconsdir}/hicolor/scalable/mimetypes/*
%if 0%{?get_local_libs}
%{_libdir}/*
%config(noreplace) %{_sysconfdir}/ld.so.conf.d/*
%if 0%{?get_local_libs} && !0%{?skip_local_POVRay_libs}
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

%changelog
* Thu Sep 11 2025 - trevor.dot.sandy.at.gmail.dot.com 2.4.9.4492
- LPub3D 2.4.9 enhancements and fixes - see RELEASE_NOTES for details

* Tue Jan 07 2025 - trevor dot sandy at gmail dot com 2.4.9.4047
- LPub3D 2.4.9 enhancements and fixes - see RELEASE_NOTES for details

* Fri Oct 18 2024 - trevor dot sandy at gmail dot com 2.4.8.3953
- LPub3D 2.4.8 enhancements and fixes - see RELEASE_NOTES for details

* Wed Jun 21 2023 - trevor dot sandy at gmail dot com 2.4.7.3476
- LPub3D 2.4.7 enhancements and fixes - see RELEASE_NOTES for details

* Wed Nov 30 2022 - trevor dot sandy at gmail dot com 2.4.6.3001
- LPub3D 2.4.6 enhancements and fixes - see RELEASE_NOTES for details

* Tue Nov 22 2022 - trevor dot sandy at gmail dot com 2.4.5.2977
- LPub3D 2.4.5 enhancements and fixes - see RELEASE_NOTES for details

* Tue May 31 2022 - trevor dot sandy at gmail dot com 2.4.4.2670
- LPub3D 2.4.4 enhancements and fixes - see RELEASE_NOTES for details

* Fri May 14 2021 - trevor dot sandy at gmail dot com 2.4.3.2557
- LPub3D 2.4.3 enhancements and fixes - see RELEASE_NOTES for details

* Wed Mar 31 2021 - trevor dot sandy at gmail dot com 2.4.2.2432
- LPub3D 2.4.2 enhancements and fixes - see RELEASE_NOTES for details

* Mon Mar 15 2021 - trevor dot sandy at gmail dot com 2.4.1.2389
- LPub3D 2.4.1 enhancements and fixes - see RELEASE_NOTES for details

* Tue Mar 10 2020 - trevor dot sandy at gmail dot com 2.4.0.1990
- LPub3D 2.4.0 enhancements and fixes - see RELEASE_NOTES for details

* Tue Sep 10 2019 - trevor dot sandy at gmail dot com 2.3.13.1511
- LPub3D 2.3.13 enhancements and fixes - see RELEASE_NOTES for details

* Thu Jun 20 2019 - trevor dot sandy at gmail dot com 2.3.12.1356
- LPub3D 2.3.12 enhancements and fixes - see RELEASE_NOTES for details

* Fri May 17 2019 - trevor dot sandy at gmail dot com 2.3.11.1269
- LPub3D 2.3.11 enhancements and fixes - see RELEASE_NOTES for details

* Tue Apr 23 2019 - trevor dot sandy at gmail dot com 2.3.10.1240
- LPub3D 2.3.10 enhancements and fixes - see RELEASE_NOTES for details

* Thu Apr 18 2019 - trevor dot sandy at gmail dot com 2.3.9.1228
- LPub3D 2.3.9 enhancements and fixes - see RELEASE_NOTES for details

* Mon Apr 08 2019 - trevor dot sandy at gmail dot com 2.3.8.1210
- LPub3D 2.3.8 enhancements and fixes - see RELEASE_NOTES for details

* Wed Mar 27 2019 - trevor dot sandy at gmail dot com 2.3.7.1195
- LPub3D 2.3.7 enhancements and fixes - see RELEASE_NOTES for details

* Wed Dec 19 2018 - trevor dot sandy at gmail dot com 2.3.6.1101
- LPub3D 2.3.6 enhancements and fixes - see RELEASE_NOTES for details

* Mon Dec 17 2018 - trevor dot sandy at gmail dot com 2.3.5.1092
- LPub3D 2.3.5 enhancements and fixes - see RELEASE_NOTES for details

* Mon Dec 10 2018 - trevor dot sandy at gmail dot com 2.3.4.1084
- LPub3D 2.3.4 enhancements and fixes - see RELEASE_NOTES for details

* Sat Oct 27 2018 - trevor dot sandy at gmail dot com 2.3.3.1034
- LPub3D 2.3.3 enhancements and fixes - see RELEASE_NOTES for details

* Fri Oct 05 2018 - trevor dot sandy at gmail dot com 2.3.2.998
- LPub3D 2.3.2 enhancements and fixes - see RELEASE_NOTES for details

* Wed Oct 03 2018 - trevor dot sandy at gmail dot com 2.3.1.977
- LPub3D 2.3.1 enhancements and fixes - see RELEASE_NOTES for details

* Tue Sep 04 2018 - trevor dot sandy at gmail dot com 2.3.0.887
- LPub3D 2.3.0 enhancements and fixes - see RELEASE_NOTES for details

* Thu Apr 12 2018 - trevor dot sandy at gmail dot com 2.2.2.848
- LPub3D 2.2.2 enhancements and fixes - see RELEASE_NOTES for details

* Sat Mar 31 2018 - trevor dot sandy at gmail dot com 2.2.1.824
- LPub3D 2.2.1 enhancements and fixes - see RELEASE_NOTES for details

* Fri Mar 16 2018 - trevor dot sandy at gmail dot com 2.2.0.795
- LPub3D 2.2.0 enhancements and fixes - see RELEASE_NOTES for details

* Mon Feb 12 2018 - trevor dot sandy at gmail dot com 2.1.0.775
- LPub3D 2.1.0 enhancements and fixes - see RELEASE_NOTES for details

* Fri Feb 10 2017 - trevor dot sandy at gmail dot com 2.0.20.645
- LPub3D 2.0.20 enhancements and fixes - see RELEASE_NOTES for details
