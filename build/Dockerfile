FROM registry.cn-beijing.aliyuncs.com/aproton/laravel:v1.0

COPY . /home/admin/laravel

RUN mkdir -o /home/admin/databases && sqlite3 aproton.sqlite -interactive ".open aproton.sqlite"; \
    cd /home/admin/laravel && composer update -vvv --no-interaction; \
    php artisan migrate

CMD ["php", "artisan", "serve", "--host=0.0.0.0", "--port=8080"]