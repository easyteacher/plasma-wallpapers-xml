#
# spec file for package plasma-wallpapers-xml
#
# Please submit bugfixes or comments via https://github.com/easyteacher/plasma-wallpapers-xml/issues
#


Name:           plasma-wallpapers-xml
Version:        1.1.1
Release:        1%{?dist}
Summary:        A wallpaper plugin for KDE Plasma
License:        GPLv2+
Group:          System/GUI/KDE
URL:            https://github.com/easyteacher/plasma-wallpapers-xml
Source0:	    https://github.com/easyteacher/plasma-wallpapers-xml/archive/refs/heads/main.tar.gz
BuildRequires:  cmake(KF5Config)
BuildRequires:  cmake(KF5Declarative)
BuildRequires:  cmake(KF5I18n)
BuildRequires:  cmake(KF5KIO)
BuildRequires:  cmake(KF5NewStuff)
BuildRequires:  cmake(KF5Notifications)
BuildRequires:  cmake(KF5Package)
BuildRequires:  cmake(KF5Plasma)
BuildRequires:  cmake(Qt5Core) >= 5.15.0
BuildRequires:  cmake(Qt5Qml) >= 5.15.0
BuildRequires:  cmake(Qt5Quick) >= 5.15.0

%description
A image wallpaper plugin for KDE Plasma that supports GNOME XML wallpaper format.

%prep
%autosetup -n plasma-wallpapers-xml-main

%build
%if 0%{?suse_version}
%cmake_kf5 -d build -- -DCMAKE_INSTALL_LOCALEDIR=%{_kf5_localedir}
%else
%cmake
%endif
%cmake_build

%install
%if 0%{?suse_version}
%kf5_makeinstall -C build
%else
%cmake_install
%endif

%files
%license LICENSE
%dir %{_datadir}/plasma/wallpapers
%{_datadir}/plasma/wallpapers/com.github.easyteacher.plasma.wallpapers.xml/
%{_datadir}/plasma/wallpapers/com.github.easyteacher.plasma.wallpapers.xml.slideshow/

%{_datadir}/metainfo/com.github.easyteacher.plasma.wallpapers.xml.appdata.xml
%{_datadir}/metainfo/com.github.easyteacher.plasma.wallpapers.xml.slideshow.appdata.xml

%{_datadir}/kservices5/plasma-wallpaper-com.github.easyteacher.plasma.wallpapers.xml.desktop
%{_datadir}/kservices5/plasma-wallpaper-com.github.easyteacher.plasma.wallpapers.xml.slideshow.desktop

%dir %{_libdir}/qt5/qml/com/github/easyteacher/plasma/wallpapers/xml
%{_libdir}/qt5/qml/com/github/easyteacher/plasma/wallpapers/xml/

%changelog

