Name:           gst-omx
Summary:        GStreamer plug-in that allows communication with OpenMAX IL components
Version:        1.2.0
Release:        0
License:        LGPL-2.1+
Group:          Multimedia/Framework
Source0:        %{name}-%{version}.tar.gz
Source100:      common.tar.bz2
Source1001:     gst-omx.manifest
BuildRequires:  which
BuildRequires:  pkgconfig(gstreamer-1.0)
BuildRequires:  pkgconfig(gstreamer-plugins-base-1.0)
BuildRequires: pkgconfig(libtbm)
BuildRequires: pkgconfig(mm-common)
BuildRequires: model-build-features

%if "%{model_build_feature_formfactor}" == "circle"
ExcludeArch: %{arm}
%endif

%description
gst-openmax is a GStreamer plug-in that allows communication with OpenMAX IL components.
Multiple OpenMAX IL implementations can be used.

%prep
%setup -q
%setup -q -T -D -a 100
cp %{SOURCE1001} .

%build
./autogen.sh --noconfigure

export CFLAGS+=" -DEXYNOS_SPECIFIC"
export CFLAGS+=" -DGST_TIZEN_MODIFICATION"

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

