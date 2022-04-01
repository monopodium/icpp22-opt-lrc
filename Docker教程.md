# Docker安装和命令

[Docker 入门指南：如何在 Ubuntu 上安装和使用 Docker ](https://kalasearch.cn/community/tutorials/how-to-install-and-use-docker-on-ubuntu/)

[Docker快速入门总结笔记-CSDN博客](https://blog.csdn.net/huangjhai/article/details/118854733)

作者：USTC 敖旭扬

日期：2021年9月26日

## Docker是什么

- [ Docker官网](https://www.docker.com/)

- [Docker中文社区](https://www.docker.org.cn/index.html)

## 安装

> 在Ubuntu 20.04.3 LTS上尝试过

参考[Install Docker Engine on Ubuntu | Docker Documentation](https://docs.docker.com/engine/install/ubuntu/)

```shell
# 卸载旧版本
$ sudo apt-get remove docker docker-engine docker.io containerd runc

# 通过仓库安装
$ sudo apt-get update
$ sudo apt-get install \
    apt-transport-https \
    ca-certificates \
    curl \
    gnupg \
    lsb-release
$ curl -fsSL https://download.docker.com/linux/ubuntu/gpg | sudo gpg --dearmor -o /usr/share/keyrings/docker-archive-keyring.gpg

# 该命令参考http://mirrors.ustc.edu.cn/help/docker-ce.html 修改过
$ echo \
  "deb [arch=amd64 signed-by=/usr/share/keyrings/docker-archive-keyring.gpg] https://mirrors.ustc.edu.cn/docker-ce/linux/ubuntu \
  $(lsb_release -cs) stable" | sudo tee /etc/apt/sources.list.d/docker.list > /dev/null

# 安装Docker
$ sudo apt-get update # 不可缺少的一步
$ sudo apt-get install docker-ce docker-ce-cli containerd.io
```

## 查看版本和运行状态、帮助

```shell
# 查看版本
$ sudo docker version
# 更多信息
$ sudo docker info
# 查看运行状态
$ sudo systemctl status docker
# 查看命令用法（以search命令为例）
$ sudo docker search --help
# Hello World，本地没有的镜像会去Docker Hub上pull
$ sudo docker run hello-world
```


## 命令速查

### 镜像操作

#### 查询

```shell
$ sudo docker search centos
```

可以去[Docker Hub Container Image Library | App Containerization](https://hub.docker.com/)上搜索，查看有哪些tag

#### 拉取镜像

```shell
$ sudo docker pull centos:7
```

#### 查看本地镜像

```shell
$ sudo docker images
```

#### 删除镜像

```shell
$ sudo docker rmi -f 镜像id
```

### 容器操作

#### 运行容器

```shell
$ sudo docker run -it centos:7 /bin/bash
# 或者sudo docker run -it centos:7
# exit退出容器
$ sudo docker run [可选参数] image

#参数说明
--name="名字"           指定容器名字
-d                     后台方式运行
-it                    使用交互方式运行,进入容器查看内容
-p                     指定容器的端口
    (
    -p ip:主机端口:容器端口  配置主机端口映射到容器端口
    -p 主机端口:容器端口
    -p 容器端口
    )
-P                     随机指定端口(大写的P)
```

#### 列出容器

```shell
$ sudo docker ps -a
#docker ps 
     # 列出当前正在运行的容器
-a   # 列出所有容器的运行记录
-n=? # 显示最近创建的n个容器
-q   # 只显示容器的编号
```

#### 删除容器

```shell
$ sudo docker rm 容器id
# 强制删除正在运行的容器
$ sudo docker rm -f 容器id
```

#### 启动和停止容器

```shell
$ sudo docker start 容器id          #启动容器
$ sudo docker restart 容器id        #重启容器
$ sudo docker stop 容器id           #停止当前运行的容器
$ sudo docker kill 容器id           #强制停止当前容器
```

### 其他例子

```shell
$ sudo docker run -d centos /bin/sh -c "while true;do echo hi;sleep 5;done"
$ sudo docker logs -tf --tail 10(最后10条) 容器id
$ sudo docker top 容器id
```

#### 进入当前正在运行的容器

```shell
# 进入容器后开启一个新的终端
$ sudo docker exec -it 容器id /bin/bash
# 可以进入后`ps -ef`查看

# 进入容器正在执行的终端
$ sudo docker attach 容器id
```

### 文件复制

#### Docker容器向宿主机传送文件

```shell
$ sudo docker cp container_id:<docker容器内的路径> <本地保存文件的路径>
# 比如
$ sudo docker cp 10704c9eb7bb:/root/test.text /home/vagrant/test.txt
```

#### 宿主机向Docker容器传送文件


```shell
$ sudo docker cp 本地文件的路径 container_id:<docker容器内的路径>
# 比如
$ sudo docker cp  /home/vagrant/test.txt 10704c9eb7bb:/root/test.text
```

### 查看镜像结构

```shell
$ sudo docker image inspect 镜像名
# 比如
$ sudo docker image inspect centos:7
```

### 提交镜像

```shell
$ sudo docker commit -m="提交的描述信息"  -a="作者" 容器id 目标镜像名:[TAG]
```

## Nginx部署

```shell
$ sudo docker pull nginx
$ sudo docker run -d --name nginx01 -p 3334:80 nginx
```

## 数据卷

```shell
$ sudo docker run -it --mount type=bind,source=/home/axy/code/mem,target=/home/code memcached:v1
```

## Dockfile

```shell
$ sudo docker build -f ./Dockerfile -t ustccas:1.0 .
```

## 其他

```shell
# 1. 想要删除容器，则要先停止所有容器（当然，也可以加-f强制删除，但是不推荐）：
$ sudo docker stop $(sudo docker ps -a -q)

# 2. 删除所有容器
$ sudo docker rm $(sudo docker ps -a -q)

# 3.删除所有镜像（慎重）
$ sudo docker rmi $(sudo docker images -q)
```

