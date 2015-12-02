Name: ali-dds-kernel
Packager:linqing.zyd
Version:0.1.7
Release: %(echo $RELEASE)%{?dist} 
# if you want use the parameter of rpm_create on build time,
# uncomment below
Summary: Aliyun MongoDB open source document-oriented database system
Group: alibaba/application
License: AGPL 3.0
AutoReqProv: none

%define _root /u01
%define _prefix %{_root}/mongodb_install
%define _saved %{_root}/mongodb_%(date +%%Y%%m%%d)_%{version}
%define _current %{_root}/mongodb_current
%undefine _missing_build_ids_terminate_build


Source: src.tar.gz

%description
MongoDB is built for scalability, performance and high availability, scaling from single server deployments to large, complex multi-site architectures. By leveraging in-memory computing, MongoDB provides high performance for both reads and writes. MongoDB’s native replication and automated failover enable enterprise-grade reliability and operational flexibility.



%prep
%setup 

%build

# prepare your files

##debug_package

%install
# OLDPWD is the dir of rpm_create running
# _prefix is an inner var of rpmbuild,
# _lib is an inner var, maybe "lib" or "lib64" depend on OS
#cd $OLDPWD/../;
echo $PWD

# see http://aone.alibaba-inc.com/aone2/doubt/commonDetail?id=189
export PATH=/usr/local/gcc-4.8.2/bin:$PATH
scons --static-libstdc++=/usr/local/gcc-4.8.2/lib64/libstdc++ --ssl=SSL --nostrip=NOSTRIP --prefix=${RPM_BUILD_ROOT}/%{_prefix} install %{?_smp_mflags}

%files
%defattr(775,admin,admin)
%{_prefix}

%changelog
* Wed Oct 21 2015 yexiang.ych <yexiang.ych@taobao.com>
- Modify install root /u01/
* Mon Sep 28 2015 yexiang.ych <yexiang.ych@taobao.com>
- Wrote ali-dds-kernel.spec

%pre

%post

cp -rf %{_prefix} %{_saved}
rm -f %{_current}
ln -s %{_saved} %{_current}

