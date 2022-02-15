FROM aproton/system:v1.0

COPY php.ini /etc/php/7.4/cli/php.ini

RUN apt install --fix-missing -y mysql-server php-mysql php-xml php-mbstring php-sqlite3 

COPY .build-cache/iotservice /root/iotservice
COPY .build-cache/website/* /root/iotservice/public
COPY .build-cache/electron.so /usr/lib/php/20190902/electron.so
COPY .build-cache/libuv.so.1 /usr/local/lib/libuv.so.1

COPY start.sh /root/iotservice/start.sh

ENV SERVER_MYSQL_HOST 127.0.0.1

EXPOSE 8080

CMD ["/bin/bash", "/root/iotservice/start.sh"]