%define _optdir	/opt
%define _appdir	%{_optdir}/apps


Name:       gst-openmax
Summary:    GStreamer plug-in that allows communication with OpenMAX IL components
Version:    0.10.1
Release:    0
Group:      TO_BE/FILLED_IN
License:    TO BE FILLED IN
Source0:    %{name}-%{version}.tar.gz
BuildRequires: which
BuildRequires: pkgconfig(gstreamer-0.10)
BuildRequires: pkgconfig(gstreamer-plugins-base-0.10)

%description
gst-openmax is a GStreamer plug-in that allows communication with OpenMAX IL components.
Multiple OpenMAX IL implementations can be used, i
including but not limited to Texas Instruments and Bellagio.


%prep
%setup -q

%build
./autogen.sh --noconfigure
%configure --disable-static --prefix=%{_prefix}

make %{?jobs:-j%jobs}


%install
rm -rf %{buildroot}
%make_install

%files
%defattr(-,root,root,-) 
%{_libdir}/gstreamer-0.10/libgstomx.so

