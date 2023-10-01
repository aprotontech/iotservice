#!/bin/bash

set -e

HERE=$(cd "$(dirname "$0")";pwd)
source $HERE/build_env.sh

mkdir -p $WEBSITE

# build website
cd $PROJECT_ROOT/website
rm -rf dist
npm install
npm run build

cp -rf $PROJECT_ROOT/website/dist/* $WEBSITE
