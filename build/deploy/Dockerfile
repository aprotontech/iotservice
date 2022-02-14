FROM aproton/system:v1.0

COPY php.ini /etc/php/7.4/cli/php.ini

RUN apt install --fix-missing -y mysql-server php-mysql php-xml php-mbstring php-sqlite3 

COPY .build-cache/iotservice /root/iotservice
COPY .build-cache/website/* /root/iotservice/public
COPY .build-cache/electron.so /usr/lib/php/20190902/electron.so

COPY start.sh /root/iotservice/start.sh


CMD ["/bin/bash", "/root/iotservice/start.sh"]