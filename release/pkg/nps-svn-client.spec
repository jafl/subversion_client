Summary: NPS Subversion Client provides a simple, graphical Subversion client.
Name: %app_name
Version: %pkg_version
Release: 1
License: Copyright John Lindal
Group: Development/Tools/Version Control
Source: %pkg_name
Requires: libX11, libXinerama, libXpm, libXft, libxml2, gd, libjpeg, libpng, libicu, pcre, boost-devel, curl

%description
NPS Subversion Client provides a simple, graphical Subversion client.

%prep
%setup

%install

%define svn_client_doc_dir /usr/local/share/doc/nps-svn-client
%define gnome_app_path     /usr/local/share/applications
%define gnome_icon_path    /usr/local/share/pixmaps

./install $RPM_BUILD_ROOT/usr/local

%files

%docdir %svn_client_doc_dir

/usr/local/bin/nps-svn-client
%svn_client_doc_dir

%gnome_app_path/nps-svn-client.desktop
%gnome_icon_path/nps-svn-client.png
