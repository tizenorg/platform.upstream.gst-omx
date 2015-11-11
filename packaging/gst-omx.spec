Name:           gst-omx
Summary:        GStreamer plug-in that allows communication with OpenMAX IL components
Version:        1.2.0
Release:        1
License:        LGPL-2.1+
Group:          Multimedia/Framework
Source:        %{name}-%{version}.tar.gz
BuildRequires:  which
BuildRequires:  pkgconfig(gstreamer-1.0)
BuildRequires:  pkgconfig(gstreamer-plugins-base-1.0)
BuildRequires: pkgconfig(libtbm)
BuildRequires: pkgconfig(mm-common)

%description
gst-openmax is a GStreamer plug-in that allows communication with OpenMAX IL components.
Multiple OpenMAX IL implementations can be used.

%prep
%setup -q -n %{name}-%{version}

%build
./autogen.sh --noconfigure

export CFLAGS+=" -DEXYNOS_SPECIFIC"
export CFLAGS+=" -DUSE_TBM"

%ifarch aarch64
%configure --disable-static --prefix=/usr --with-omx-target=exynos64
%else
%configure --disable-static --prefix=/usr --with-omx-target=exynos
%endif

make %{?jobs:-j%jobs}


%install
rm -rf %{buildroot}
mkdir -p %{buildroot}/usr/share/license
cp COPYING %{buildroot}/usr/share/license/%{name}
%make_install

%files
%manifest gst-omx.manifest
%defattr(-,root,root,-)
%{_libdir}/gstreamer-1.0/libgstomx.so
/etc/xdg/gstomx.conf
/usr/share/license/%{name}

