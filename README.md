# Optimal_LRC_libmemcached

This is the code repository for the paper "Repair-Optimal Data Placement for Locally Repairable Codes
with Optimal Minimum Hamming Distance". The code consists of two main parts: simulation and prototype.
## Simulation 
The codes of simulation are in folder \pic_sim and the required runtime environment is as follows:
```shell
    python == 3.9
    numpy==1.20.3
    matplotlib==3.4.2
```
## Prototype
We implement our prototype on top on Libmemcached (v1.0.18)
, by adding about 3000 SLoC. We deploy the Libmemcached
as the proxy. We also deploy multiple Memcached servers as
storage servers. We leverage the Jerasure Library for erasure
coding. To show the improvements of the optimal data placements
over flat and random data placements, we also implement the flat
and random data placement.
## 1. Preparation

There are some required libraries that are listed as follows.

- make & cmake

- automake & aclocal & autoconf

- libevent

Users can use apt-get to install the required libraries.

Also, we give the versions of the libraries in our testbed for reference.

- gcc, g++, 5.4.0

- make, GNU 4.1, cmake, 3.8.1

- automake, aclocal, 1.14.1, autoconf, 2.69

- libevent, 2.1.11

## Installation of the memcached servers

In each physical machine that runs as a memcached server, do the following.

- Downloading the sourcecode of memcached server:

(https://memcached.org/downloads)

- Unzipping:

```
$tar -zvxf memcached-1.5.20.tar.gz
```
- Entering the memcached directory, compling and installing:

```
$./configure --prefix=$(pwd)
```

```
$make & sudo make install
```
```
./memcached -m 128 -p 8888 --max-item-size=5242880 -vv
```
## Installation of the proxy

- Downloading our sourcecode

- Unzipping:

- Entering the libmemcached-1.0.18/ directory, compiling and installing:

```shell
$./configure --prefix=$(pwd)
```

```shell
$make & sudo make install
```

- Going to the root directory, compiling the test program:

```shell
$make my_test
```
## Use of Docker
In order to avoid the trouble of environment configuration, we also provide Docker with well-configured environment.
### Quick Start

- Install docker and docker-compose

- Build a mirror
```shell
# In the same level of this README
sudo docker build -f ./Dockerfile -t lrcmemcached:base .
```

3. Run the image, enter the docker, and configure the environment
```shell
sudo docker run -it --name servers lrcmemcached:base /bin/bash
# enter the docker
cd /home
./install.sh
# A long time after the end of the execution
# Check the ip:port of the service
ip addr
# start Services
cd /home/memcached-1.5.20
./memcached -m 128 -p 8888 --max-item-size=5242880 -vv -u root
```

- Build a mirror
```shell
sudo docker build -f ./memcached_server/Dockerfile -t lrcmemcached:running .
```

- Run multiple memcached instances
```shell
# Method 1
sudo docker run -it --name memcached_server1 lrcmemcached:running
sudo docker run -it --name memcached_server2 lrcmemcached:running
sudo docker run -it --name memcached_server3 lrcmemcached:running
sudo docker run -it --name memcached_server4 lrcmemcached:running
ip addr

# Method 2
sudo docker-compose up
```

- Run another image to install lrc_libmemcached
```shell
sudo docker run -it --name lrc_libmemcached --mount type=bind,source=/home/axy/code/lrc_libmemcached,target=/home/code lrcmemcached:server
```
## Running and testing
Parameters of the test code, where $(n,k,r)$ is the parameters of Optimal-LRC $b = d-1$,cluster_number is the number of all clusters.
```shell
./mytest n k r b cluster_number
```

