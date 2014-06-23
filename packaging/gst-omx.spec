Name:           gst-omx
Summary:        GStreamer plug-in that allows communication with OpenMAX IL components
Version:        1.0.0
Release:        1
License:        LGPL-2.1+
Group:          Multimedia/Framework
Source0:        %{name}-%{version}.tar.gz
Source100:      common.tar.bz2
Source1001:     gst-omx.manifest
BuildRequires:  which
BuildRequires:  pkgconfig(gstreamer-1.0)
BuildRequires:  pkgconfig(gstreamer-plugins-base-1.0)

%description
gst-openmax is a GStreamer plug-in that allows communication with OpenMAX IL components.
Multiple OpenMAX IL implementations can be used.

%prep
%setup -q
%setup -q -T -D -a 100
cp %{SOURCE1001} .

%build
./autogen.sh --noconfigure

%ifarch %{arm}
export CFLAGS+=" -DEXYNOS_SPECIFIC"
%endif

%configure --disable-static --prefix=/usr

make %{?jobs:-j%jobs}


%install
rm -rf %{buildroot}
%make_install

%files
%manifest gst-omx.manifest
%{_libdir}/gstreamer-1.0/libgstomx.so

