#!/bin/bash
pIDa=`lsof -i :12111|grep -v "PID" | awk '{print $2}'`
# echo $pIDa
if [ "$pIDa" != "" ];
then
    echo "memcached start!"
    # killall memcached 
else
    export LD_LIBRARY_PATH=$(pwd)/memcachedbin/libeventlib:$LD_LIBRARY_PATH
    memcachedbin/memcached -m 128 -p 12111 --max-item-size=5242880 -vv -d
    memcachedbin/memcached -m 128 -p 12112 --max-item-size=5242880 -vv -d
    memcachedbin/memcached -m 128 -p 12113 --max-item-size=5242880 -vv -d
    memcachedbin/memcached -m 128 -p 12114 --max-item-size=5242880 -vv -d
    memcachedbin/memcached -m 128 -p 12115 --max-item-size=5242880 -vv -d
    memcachedbin/memcached -m 128 -p 12116 --max-item-size=5242880 -vv -d
    memcachedbin/memcached -m 128 -p 12117 --max-item-size=5242880 -vv -d
    memcachedbin/memcached -m 128 -p 12118 --max-item-size=5242880 -vv -d
    memcachedbin/memcached -m 128 -p 12119 --max-item-size=5242880 -vv -d
    memcachedbin/memcached -m 128 -p 12120 --max-item-size=5242880 -vv -d
    memcachedbin/memcached -m 128 -p 12121 --max-item-size=5242880 -vv -d
    echo "memcached start!"
fi