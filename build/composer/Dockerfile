FROM registry.cn-beijing.aliyuncs.com/aproton/php:v1.0

RUN apt install -y composer php-xml php-mbstring

RUN composer config -g repo.packagist composer https://mirrors.aliyun.com/composer/
RUN cd /root && composer global require laravel/installer

RUN ln -s /root/.composer/vendor/bin/laravel /usr/local/bin/laravel

CMD [ "laravel", "new", "blog" ]