#!/bin/bash

set -ex

HERE=$(cd "$(dirname "$0")";pwd)
PROJECT_ROOT=$(cd $HERE; cd ..; pwd)
echo $PROJECT_ROOT

TMP=$PROJECT_ROOT/build/deploy/.build-cache
rm -rf $TMP
mkdir -p $TMP

# build website
cd $PROJECT_ROOT/website
rm -rf dist
npm install
npm run build

cp -rf $PROJECT_ROOT/website/dist $TMP/website


# build laravel
cd $PROJECT_ROOT/backend
composer install -vvv


cp -rf $PROJECT_ROOT/backend $TMP/iotservice
rm -rf $TMP/iotservice/.env
rm -rf $TMP/iotservice/.git*

# build extension
cd $PROJECT_ROOT/extenstion/electron
phpize
./configure
make -j8
#make install

cp .libs/electron.so $TMP/electron.so
cp /usr/local/lib/libuv.so.1 $TMP/libuv.so.1

# TODO
echo "all modules had builded and copyed to $TMP"
echo "cd build/deploy && docker build -t aproton/iotservice:v1.0-alpha ."