#!/bin/bash

set -ex

HERE=$(cd "$(dirname "$0")";pwd)
PROJECT_ROOT=$(cd $HERE; cd ../../..; pwd)
echo $PROJECT_ROOT

TMP=/tmp/aproton/build/output/.build-cache

APISERVICE=$TMP/apiservice
WEBSITE=$TMP/website

