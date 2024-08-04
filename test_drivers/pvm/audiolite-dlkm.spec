# If kversion isn't defined on the rpmbuild line, define it here.
%{!?kversion: %define kversion %(uname -r)}

%{!?with_oot_debug: %define with_oot_debug 0}

%define kmod_name audiolite-dlkm

%define debug_package %{nil}

%if %{with_oot_debug}
    %define kpackage kernel-automotive-debug
    %define kversion_with_debug %{kversion}+debug
%else
    %define kpackage kernel-automotive
    %define kversion_with_debug %{kversion}
%endif

Name: %{kmod_name}
Version: 1.0
Release:        1%{?dist}
Summary: audiolite kernel module

License: GPLv2
Source0: %{name}-%{version}.tar.gz

BuildRequires: kernel-automotive-devel-uname-r = %{kversion_with_debug}
Requires: %{kpackage}-core-uname-r = %{kversion_with_debug}

%description
This is a rpm contains audiolite out of tree kernel modules.

%prep
%setup -qn %{name}

%build
KSRC=%{_usrsrc}/kernels/%{kversion_with_debug}
make KERNEL_SRC=${KSRC} all

%post
depmod -a

%postun
depmod -a

%install
KSRC=%{_usrsrc}/kernels/%{kversion_with_debug}
make KERNEL_SRC=${KSRC} INSTALL_MOD_PATH=$RPM_BUILD_ROOT modules_install
rm -rf "$RPM_BUILD_ROOT/lib/modules/%{kversion_with_debug}/modules."*
%{__install} -d %{buildroot}%{_sysconfdir}/modules-load.d/
%{__install} %{kmod_name}.conf %{buildroot}%{_sysconfdir}/modules-load.d/

%clean
rm -rf $RPM_BUILD_ROOT

%files
%define kernel_module_path /lib/modules/%{kversion_with_debug}
%{kernel_module_path}/extra/ipcc_shmem_test_module.ko
%{_sysconfdir}/modules-load.d/%{kmod_name}.conf
