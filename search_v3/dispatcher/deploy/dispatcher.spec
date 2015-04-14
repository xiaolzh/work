# DISPATCHER deployment structure:
#   /usr/local/dispatcher/bin/*       - dispatcher server binary files
#   /usr/local/dispatcher/data/*      - dispatcher server data files
#   /usr/local/dispatcher/log/*       - dispatcher log files

Name: dispatcher
Summary: Dispatcher: A data center.
Version: 1.0 
Release: 
Source0: %{name}-%{version}.tar.gz
License: commercial
Group: Development/Tools
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-%(%{__id} -u)
Requires: search_v3 >= 1.0 nginx >= 1.0

%define _prefix /usr/local
%define _tmppath /tmp/

%description
The dispatcher program is a data center for getting data in full or increase way. 
It is a component of search_v3 program.

%prep
%setup -q

%build
make clean
make

%install

rm -rf $RPM_BUILD_ROOT
make DESTDIR=$RPM_BUILD_ROOT/%{_prefix} install

%files
%defattr(-,root,root)
%{_prefix}/dispatcher

#%doc MEMO

%post
echo "post-install for dispatcher main package"
python %{_prefix}/dispatcher/bin/tools/deploy.py init

%preun
echo "pre-uninstall for dispatcher main package"
#python %{_prefix}/dispatcher/bin/tools/deploy.py co_init

%clean
rm -rf $RPM_BUILD_ROOT

%changelog
* Mon May 28 2012 Steve Lee <llwgod@gmail.com>
* - initial dispatcher spec file
