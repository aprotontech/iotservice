#!/bin/bash

set -ex

PROJECT_ROOT=/root/iotservice

service mysql start

if [ ! -f .env ]; then
  cp .env.example .env
  php artisan key:generate
fi

php artisan serve --host=0.0.0.0 --port=8080