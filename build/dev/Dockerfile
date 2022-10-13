FROM aproton/system:v1.0

MAINTAINER kuper <kuper@aproton.tech>

COPY php.ini /etc/php/7.4/cli/php.ini

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

WORKDIR /root

CMD [ "/bin/bash" ]

