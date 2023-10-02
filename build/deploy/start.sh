#!/bin/bash

set -ex

EMQX_ROOT=/root/emqx

PROJECT_ROOT=/root/apiservice

cd $EMQX_ROOT

# start emqx
./bin/emqx start
./bin/emqx_ctl status

cd $PROJECT_ROOT

if [ ! -f .env ]; then
  cp .env.example .env
  php artisan key:generate
fi

if [ "$SERVER_MYSQL_HOST" != "localhost" && "$SERVER_MYSQL_HOST" != "127.0.0.1" ]; then
  # use custome mysql
  # TODO: change mysql host, port, username, password, database
  echo "custom mysql-host not support now"
  exit
else
  service mysql start
fi

if [ ! -f /root/project-inited ]; then
  sleep 1
  # create database and add new user
  mysql -uroot -e "create database aproton"
  mysql -uroot -e "CREATE USER 'aproton' IDENTIFIED WITH mysql_native_password BY 'LDloAczOvBu7GkeP'"
  mysql -uroot -e "grant all privileges on aproton.* to 'aproton'@'%'"
  # init project tables and default admin user
  php artisan migrate
  php artisan db:seed ProtonSystemSeeder

  touch /root/project-inited
fi

php artisan serve --host=0.0.0.0 --port=8080
