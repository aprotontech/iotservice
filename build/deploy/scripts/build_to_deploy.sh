#!/bin/bash

set -ex

HERE=$(cd "$(dirname "$0")";pwd)
source $HERE/build_env.sh

cd $HERE
mkdir -p $TMP

bash build_apiservice.sh
bash build_website.sh
bash build_electron.sh

echo "all modules had builded and copyed to $TMP"

rm -rf $PROJECT_ROOT/.build-cache
mkdir -p $PROJECT_ROOT/.build-cache
mv $TMP/apiservice $PROJECT_ROOT/.build-cache
mv $TMP/website/* $PROJECT_ROOT/.build-cache/apiservice/public
mv $TMP/electron.so $PROJECT_ROOT/.build-cache