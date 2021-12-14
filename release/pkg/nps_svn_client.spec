Summary: NPS Subversion Client provides a simple, graphical Subversion client.
Name: %app_name
Version: %pkg_version
Release: 1
License: Copyright John Lindal
Group: Development/Tools/Version Control
Source: %pkg_name
Requires: libX11, libXinerama, libXpm, libXft, libxml2, gd, libjpeg, libpng, libicu, pcre

%description
NPS Subversion Client provides a simple, graphical Subversion client.
(http://www.newplanetsoftware.com/svn_client/)

%prep
%setup

%install

%define svn_client_doc_dir /usr/share/doc/nps_svn_client
%define gnome_app_path     /usr/share/applications
%define gnome_icon_path    /usr/share/pixmaps

./install "$RPM_BUILD_ROOT"/usr

%files

%docdir %svn_client_doc_dir

/usr/bin/nps_svn_client
%svn_client_doc_dir

%gnome_app_path/nps_svn_client.desktop
%gnome_icon_path/nps_svn_client.xpm
