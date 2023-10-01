#!/bin/bash

set -e

HERE=$(cd "$(dirname "$0")";pwd)
source $HERE/build_env.sh

rm -rf $APISERVICE

# build laravel
cd $PROJECT_ROOT/backend
composer install -vvv

cp -rf $PROJECT_ROOT/backend $APISERVICE
rm -rf $APISERVICE/.env
rm -rf $APISERVICE/.git*
rm -rf $APISERVICE/storage/logs/*
rm -rf $APISERVICE/storage/framework/cache/data/*
rm -rf $APISERVICE/storage/framework/sessions/*
rm -rf $APISERVICE/storage/framework/views/*
rm -rf $APISERVICE/storage/app/*