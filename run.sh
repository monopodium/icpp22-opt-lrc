memcached -m 128 -p 12111 --max-item-size=5242880 -vv -d
memcached -m 128 -p 12112 --max-item-size=5242880 -vv -d
memcached -m 128 -p 12113 --max-item-size=5242880 -vv -d
memcached -m 128 -p 12114 --max-item-size=5242880 -vv -d
memcached -m 128 -p 12115 --max-item-size=5242880 -vv -d
memcached -m 128 -p 12116 --max-item-size=5242880 -vv -d
memcached -m 128 -p 12117 --max-item-size=5242880 -vv -d
memcached -m 128 -p 12118 --max-item-size=5242880 -vv -d
memcached -m 128 -p 12119 --max-item-size=5242880 -vv -d
memcached -m 128 -p 12120 --max-item-size=5242880 -vv -d
memcached -m 128 -p 12121 --max-item-size=5242880 -vv -d
LD_LIBRARY_PATH=/usr/local/lib; export LD_LIBRARY_PATH
LD_LIBRARY_PATH=/home/k8s/lrc_libmemcached-msdev/libmemcached-1.0.18/lib; export LD_LIBRARY_PATH
./mytest 12 6 3 4 12 1 #flat
./mytest 12 6 3 4 12 4
./mytest 12 6 3 4 12 16
./mytest 12 6 3 4 12 256
./mytest 12 6 3 4 12 1024
./mytest 12 6 3 4 12 4096

./mytest 12 6 3 4 4 1 #random
./mytest 12 6 3 4 4 4
./mytest 12 6 3 4 4 16
./mytest 12 6 3 4 4 256
./mytest 12 6 3 4 4 1024
./mytest 12 6 3 4 4 4096

./mytest 12 6 3 4 3 1 #our placement
./mytest 12 6 3 4 3 4
./mytest 12 6 3 4 3 16
./mytest 12 6 3 4 3 256
./mytest 12 6 3 4 3 1024
./mytest 12 6 3 4 3 4096

#(8,4,3)
./mytest 8 4 3 4 8 1024 #flat
./mytest 8 4 3 4 8 4096 

./mytest 8 4 3 4 5 1024 #random
./mytest 8 4 3 4 5 4096

./mytest 8 4 3 4 2 1024 #our
./mytest 8 4 3 4 2 4096

#(15,10,5)
./mytest 15 10 5 3 15 1024 #flat
./mytest 15 10 5 3 15 4096 

./mytest 15 10 5 3 8 1024 #random
./mytest 15 10 5 3 8 4096 

./mytest 15 10 5 3 5 1024 #our
./mytest 15 10 5 3 5 4096 



#带宽10：1
sudo wondershaper -a ens9 -d 952320
#带宽20：1
sudo wondershaper -a ens9 -d 476160
#带宽5：1
sudo wondershaper -a ens9 -d 1904640
#带宽15：1
sudo wondershaper -a ens9 -d 634880

sudo wondershaper -c -a ens9
iperf -s
iperf -c 10.0.0.53






