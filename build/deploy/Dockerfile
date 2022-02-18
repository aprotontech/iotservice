FROM aproton/system:v1.0

MAINTAINER kuper <kuper@aproton.tech>

RUN apt-get update --fix-missing; apt-get install --fix-missing -y mysql-server php-mysql php-xml php-mbstring php-sqlite3 wget net-tools && rm -rf /var/lib/apt/lists/*

RUN cd /root/ && wget http://api.aproton.tech/emqx-4.4.0-otp23.3.4.9-3-ubuntu20.04-amd64.tar.gz && \
    tar zxvf emqx-4.4.0-otp23.3.4.9-3-ubuntu20.04-amd64.tar.gz && rm -f emqx-4.4.0-otp23.3.4.9-3-ubuntu20.04-amd64.tar.gz

COPY php.ini /etc/php/7.4/cli/php.ini

COPY .build-cache/electron.so /usr/lib/php/20190902/electron.so
COPY .build-cache/libuv.so.1 /usr/local/lib/libuv.so.1

COPY .build-cache/iotservice /root/iotservice
COPY .build-cache/website/* /root/iotservice/public

COPY start.sh /root/iotservice/start.sh
COPY loaded_plugins /root/emqx/data/loaded_plugins
COPY emqx_auth_http.conf /root/emqx/etc/plugins/emqx_auth_http.conf

ENV SERVER_MYSQL_HOST 127.0.0.1

EXPOSE 18083 8080 1883

CMD ["/bin/bash", "/root/iotservice/start.sh"]