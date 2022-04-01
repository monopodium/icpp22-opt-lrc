# lrc_libmemcached

- 环境配置看[how-to-build-env/README.md](how-to-build-env/README.md)
- ers文档看[ers_README.md](ers_README.md)

## 启动 memcached

`memcachedbin`目录下是编译好的memcached和需要的库(libevent-2.1.so.7  libevent-2.1.so.7.0.0)

```shell
export LD_LIBRARY_PATH=/home/k8s/memcachedbin/libeventlib:$LD_LIBRARY_PATH
./memcached -m 128 -p 8888 --max-item-size=5242880 -vv
```