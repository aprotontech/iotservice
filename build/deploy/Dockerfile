FROM aproton/system:v1.0


################ 编译代码 ################
## 1. 安装依赖

# 这个layer有1.2G，新安装的包最好新起一行，不要修改这一行
RUN apt-get update --fix-missing; \
    apt install --fix-missing -y git zip unzip net-tools vim screen mysql-server nodejs \
    npm cmake php-dev php-curl php-mysql php-xml php-mbstring php-sqlite3 sqlite3; \
    apt-get clean
RUN php -r "copy('https://getcomposer.org/installer', 'composer-setup.php');"; \
    php composer-setup.php --install-dir=/usr/bin --filename=composer; \
    composer self-update ; composer config -g repo.packagist composer https://mirrors.aliyun.com/composer/; \
    cd /root && composer global require laravel/installer

RUN ln -s /root/.composer/vendor/bin/laravel /usr/local/bin/laravel
COPY build/dev/php.ini /etc/php/7.4/cli/php.ini

## 2. 开始编译
COPY . /root/iotservice
WORKDIR /root/iotservice
RUN bash build/deploy/scripts/build_cleanup.sh ; bash build/deploy/scripts/build_to_deploy.sh


################ 编译镜像 ################
FROM aproton/system:v1.0

LABEL aproton.image.authors="kuper <kuper@aproton.tech>"


RUN apt-get update --fix-missing; apt-get install --fix-missing -y mysql-server php-mysql php-curl php-xml php-mbstring php-sqlite3 wget net-tools && rm -rf /var/lib/apt/lists/*

RUN cd /root/ && wget http://api.aproton.tech/emqx-4.4.0-otp23.3.4.9-3-ubuntu20.04-amd64.tar.gz && \
    tar zxvf emqx-4.4.0-otp23.3.4.9-3-ubuntu20.04-amd64.tar.gz && rm -f emqx-4.4.0-otp23.3.4.9-3-ubuntu20.04-amd64.tar.gz

COPY build/deploy/php.ini /etc/php/7.4/cli/php.ini

WORKDIR /root/apiservice

COPY build/deploy/start.sh /root/apiservice/start.sh
COPY build/deploy/loaded_plugins /root/emqx/data/loaded_plugins
COPY build/deploy/emqx_auth_http.conf /root/emqx/etc/plugins/emqx_auth_http.conf
COPY build/deploy/emqx_web_hook.conf /root/emqx/etc/plugins/emqx_web_hook.conf

COPY --from=0 /root/iotservice/.build-cache/electron.so /usr/lib/php/20190902/electron.so
COPY --from=0 /root/iotservice/.build-cache/apiservice /root/apiservice

ENV SERVER_MYSQL_HOST 127.0.0.1

EXPOSE 18083 8080 1883

CMD ["/bin/bash", "/root/apiservice/start.sh"]