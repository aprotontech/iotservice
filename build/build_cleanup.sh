#!/bin/bash

set -ex

HERE=$(cd "$(dirname "$0")";pwd)
PROJECT_ROOT=$(cd $HERE; cd ..; pwd)
echo $PROJECT_ROOT

TMP=$PROJECT_ROOT/build/deploy/.build-cache

rm -rf $TMP
rm -rf $PROJECT_ROOT/backend/vendor
rm -rf $PROJECT_ROOT/website/node_modules
rm -rf $PROJECT_ROOT/website/dist


cd $PROJECT_ROOT/extenstion/electron
make clean
make distclean
phpize --clean
rm -f config.h*