#
# spec file for package glu
#
# Copyright (C) 2017 SUSE LINUX GmbH, Nuernberg, Germany.
#
# All modifications and additions to the file contributed by third parties
# remain the property of their copyright owners, unless otherwise agreed
# upon. The license for this file, and modifications and additions to the
# file, is the same license as for the pristine package itself (unless the
# license for the pristine package is not an Open Source License, in which
# case the license is the MIT License). An "Open Source License" is a
# license that conforms to the Open Source Definition (Version 1.9)
# published by the Open Source Initiative.

# Please submit bugfixes or comments via http://bugs.opensuse.org/
#


Name:           glu
%define lname   libGLU1
Version:        9.0.0
Release:        0
Summary:        OpenGL utility library
License:        SUSE-SGI-FreeB-2.0
Group:          Development/Libraries/C and C++
Url:            http://cgit.freedesktop.org/mesa/glu/

#Git-Clone: git://anongit.freedesktop.org/mesa/glu
#Git-Web:   http://cgit.freedesktop.org/mesa/glu/
Source:         http://cgit.freedesktop.org/mesa/glu/snapshot/%{name}-%{version}.tar.gz
Source1:        baselibs.conf
Patch0:         U_Add-D-N-DEBUG-to-CFLAGS-dependent-on-enable-debug.patch
Patch1:         u_autoconf-Don-t-modify-CFLAGS-in-configure-scripts.patch
BuildRoot:      %{_tmppath}/%{name}-%{version}-build
BuildRequires:  gcc-c++
BuildRequires:  libtool
BuildRequires:  pkgconfig
BuildRequires:  pkgconfig(gl)

%description
GLU offers simple interfaces for building mipmaps; checking for the
presence of extensions in the OpenGL (or other libraries which follow
the same conventions for advertising extensions); drawing
piecewise-linear curves, NURBS, quadrics and other primitives
(including, but not limited to, teapots); tesselating surfaces;
setting up projection matrices and unprojecting screen coordinates to
world coordinates.

%package -n %lname
Summary:        OpenGL utility library
# O/P since 12.3. This Obsoletes is special (since glu is in fact Mesa),
# and should not be copy-pasted without review.
Group:          System/Libraries
Obsoletes:      Mesa-libGLU1 < %version-%release
Provides:       Mesa-libGLU1 = %version-%release

%description -n %lname
GLU offers simple interfaces for building mipmaps; checking for the
presence of extensions in the OpenGL (or other libraries which follow
the same conventions for advertising extensions); drawing
piecewise-linear curves, NURBS, quadrics and other primitives
(including, but not limited to, teapots); tesselating surfaces;
setting up projection matrices and unprojecting screen coordinates to
world coordinates.

This package provides the SGI implementation of GLU previously shipped
with Mesa, but meanwhile developed separately.

%package devel
Summary:        Development files for the GLU API
Group:          Development/Libraries/C and C++
Requires:       %lname = %version
# O/P since 12.3
Obsoletes:      Mesa-libGLU-devel < %version-%release
Provides:       Mesa-libGLU-devel = %version-%release

%description devel
GLU offers simple interfaces for building mipmaps; checking for the
presence of extensions in the OpenGL (or other libraries which follow
the same conventions for advertising extensions); drawing
piecewise-linear curves, NURBS, quadrics and other primitives
(including, but not limited to, teapots); tesselating surfaces;
setting up projection matrices and unprojecting screen coordinates to
world coordinates.

This package contains includes headers and static libraries for
compiling programs with GLU.

%prep
%setup -q
%patch0 -p1
%patch1 -p1

%build
if [ ! -e configure ]; then
    NOCONFIGURE=1 ./autogen.sh;
fi;
%configure --disable-static
make %{?_smp_mflags}

%install
make install DESTDIR="%buildroot"
rm -f "%buildroot/%_libdir"/*.la

%post -n %lname -p /sbin/ldconfig

%postun -n %lname -p /sbin/ldconfig

%files -n %lname
%defattr(-,root,root)
%_libdir/libGLU.so.*

%files devel
%defattr(-,root,root)
%_includedir/GL
%_libdir/libGLU.so
%_libdir/pkgconfig/glu.pc

%changelog
