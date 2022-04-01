# ers_libmemcached环境配置

> 更多：https://github.com/aoxy/ers_libmemcached

## 快速开始

1. 安装docker和docker-compose

2. 构建镜像
```shell
# 在该README同级目录下
sudo docker build -f ./Dockerfile -t lrcmemcached:base .
```

3. 运行镜像，进入容器，配置环境
```shell
sudo docker run -it --name servers lrcmemcached:base /bin/bash
# 以下在容器内
cd /home
./install.sh
# 很长一段时间结束执行后
# 查看该服务的ip:port
ip addr
# 可以手动启动服务
cd /home/memcached-1.5.20
./memcached -m 128 -p 8888 --max-item-size=5242880 -vv -u root
```

4. commit上述镜像
```shell
# 查看容器ip
sudo docker ps -a
# 提交
sudo docker commit -m="lrc memcached base env"  -a="aoxuyang" <容器id> lrcmemcached:server
```

5. 构建镜像
```shell
# 在该README同级目录下
sudo docker build -f ./memcached_server/Dockerfile -t lrcmemcached:running .
```

6. 运行多个memcached实例
```shell
# 方法一：手动从镜像生成容器(每个shell开一个)
sudo docker run -it --name memcached_server1 lrcmemcached:running
sudo docker run -it --name memcached_server2 lrcmemcached:running
sudo docker run -it --name memcached_server3 lrcmemcached:running
sudo docker run -it --name memcached_server4 lrcmemcached:running
# 在容器内获得ip记录下来
ip addr

# 方法二：更方便
# 在本项目根目录下
sudo docker-compose up
# ip在docker-compose.yml中指定了
```

7. 再运行一个上述镜像，安装lrc_libmemcached
```shell
# 便于修改代码，可以挂载目录（容器内和本地修改双向同步）
sudo docker run -it --name lrc_libmemcached --mount type=bind,source=/home/axy/code/lrc_libmemcached,target=/home/code lrcmemcached:server
```