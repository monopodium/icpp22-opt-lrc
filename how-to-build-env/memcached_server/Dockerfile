FROM lrcmemcached:server
EXPOSE 8888
WORKDIR /home/memcached-1.5.20
CMD ["ip", "addr"]
CMD ["./memcached", "-m", "128", "-p", "8888", "--max-item-size=5242880", "-vv", "-u", "root"]