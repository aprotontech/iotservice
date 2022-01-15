FROM registry.cn-beijing.aliyuncs.com/aproton/php:v1.0

RUN apt install -y composer php-xml php-mbstring php-sqlite3 sqlite3; \
    composer config -g repo.packagist composer https://mirrors.aliyun.com/composer/

COPY php.ini /etc/php/7.4/cli/php.ini

WORKDIR /home/admin/laravel

EXPOSE 8080

CMD ["php", "artisan", "serve", "--host=0.0.0.0", "--port=8080"]