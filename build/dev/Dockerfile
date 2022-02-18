FROM aproton/system:v1.0

MAINTAINER kuper <kuper@aproton.tech>

COPY php.ini /etc/php/7.4/cli/php.ini

RUN apt-get update --fix-missing; apt install --fix-missing -y git zip unzip net-tools vim screen mysql-server nodejs npm composer cmake php-dev php-mysql php-xml php-mbstring php-sqlite3 sqlite3 ; \
    composer config -g repo.packagist composer https://mirrors.aliyun.com/composer/; \
    cd /root && composer global require laravel/installer

RUN ln -s /root/.composer/vendor/bin/laravel /usr/local/bin/laravel

CMD [ "/bin/bash" ]

