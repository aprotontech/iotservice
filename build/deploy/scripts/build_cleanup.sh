#!/bin/bash

set -e

HERE=$(cd "$(dirname "$0")";pwd)
source $HERE/build_env.sh

rm -rf $TMP

rm -rf $PROJECT_ROOT/backend/vendor
rm -rf $PROJECT_ROOT/website/node_modules
rm -rf $PROJECT_ROOT/website/dist
rm -rf $PROJECT_ROOT/.build-cache


cd $PROJECT_ROOT/extenstion/electron
make clean
make distclean
phpize --clean
rm -f config.h*