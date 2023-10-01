#!/bin/bash

set -e

HERE=$(cd "$(dirname "$0")";pwd)
source $HERE/build_env.sh

# build extension
cd $PROJECT_ROOT/extenstion/electron
phpize
./configure
make clean
make -j
#make install

cp .libs/electron.so $TMP/electron.so