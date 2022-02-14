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
composer update -vvv


cp -rf $PROJECT_ROOT/backend $TMP/iotservice


# build extension
cd $PROJECT_ROOT/extenstion/electron
phpize
./configure
make -j8
make install

cp /usr/lib/php/20190902/electron.so $TMP/electron.so


# TODO
echo "all modules had build and copyed to $TMP"
echo "cd $TMP && docker build -t aproton/iotservice:v1.0 ."