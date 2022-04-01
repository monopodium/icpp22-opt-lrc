#!/bin/bash

apt-get update

# gcc
apt-get install gcc-5=5.4.0-6ubuntu1~16.04.12 -y
update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-5 50

# g++
apt-get install g++-5=5.4.0-6ubuntu1~16.04.12 -y
update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-5 50

# make
apt-get install make=4.1-6 -y

# cmake
cd /home
tar -zxf cmake-3.8.1.tar.gz
cd cmake-3.8.1
./configure
make
make install

# perl(important step!)
apt-get install perl -y

# m4
cd /home
tar -zxf m4-1.4.18.tar.gz
cd m4-1.4.18
./configure
make
make install

# autoconf
cd /home
tar -zxf autoconf-2.69.tar.gz
cd autoconf-2.69
./configure
make
make install

# automake
cd /home
tar -zxf automake-1.14.1.tar.gz
cd automake-1.14.1
./configure
make
make install

# libevent
cd /home
tar -zxf libevent-2.1.11-stable.tar.gz
cd libevent-2.1.11-stable
./configure
make
make install

# memcached
cd /home
tar -zxf memcached-1.5.20.tar.gz
cd memcached-1.5.20
./configure --prefix=$(pwd)
make
make install
ln -s /usr/local/lib/libevent-2.1.so.7 /usr/lib/libevent-2.1.so.7

# run
# ./memcached -m 128 -p 8888 --max-item-size=5242880 -vv -u root