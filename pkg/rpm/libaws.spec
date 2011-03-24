
Summary:        Amazon AWS Library
Name:           libaws
Version:        0.9.3
Release:        1
URL:            http://aws.28msec.com/
Source:         %{name}-%{version}.tar.gz
License:        Apache 2.0
Group:          Applications/Communications
BuildRoot:      %{_tmppath}/%{name}-%{version}-root-%(%{__id_u} -n)

%description
Amazon AWS Library

%prep
%setup -q

%build
mkdir build
cd build
cmake ..
make clean
make all

%install
cd build
make install DESTDIR=$RPM_BUILD_ROOT
        
%clean
rm -rf $RPM_BUILD_ROOT

%post

%files
%dir /usr/local/include/libaws/
/usr/local/bin/s3
/usr/local/bin/sdb
/usr/local/bin/sqs
/usr/local/include/libaws/aws.h
/usr/local/include/libaws/awsconnectionfactory.h
/usr/local/include/libaws/awstime.h
/usr/local/include/libaws/awsversion.h
/usr/local/include/libaws/common.h
/usr/local/include/libaws/config.h
/usr/local/include/libaws/connectionpool.h
/usr/local/include/libaws/exception.h
/usr/local/include/libaws/mutex.h
/usr/local/include/libaws/s3connection.h
/usr/local/include/libaws/s3exception.h
/usr/local/include/libaws/s3response.h
/usr/local/include/libaws/sdbconnection.h
/usr/local/include/libaws/sdbexception.h
/usr/local/include/libaws/sdbresponse.h
/usr/local/include/libaws/smart_ptr.h
/usr/local/include/libaws/sqsconnection.h
/usr/local/include/libaws/sqsexception.h
/usr/local/include/libaws/sqsresponse.h
/usr/local/lib/libaws.so
/usr/local/lib/libaws.so.0.9.3


%changelog
* Wed Mar 22 2011 Matthew G Williams <mwilliams@teltechcorp.com> 0.9.3
- Specs for 0.9.3
