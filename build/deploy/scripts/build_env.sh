#!/bin/bash

set -ex

HERE=$(cd "$(dirname "$0")";pwd)
PROJECT_ROOT=$(cd $HERE; cd ../../..; pwd)
echo $PROJECT_ROOT

TMP=/tmp/aproton/build/output/.build-cache

APISERVICE=$TMP/apiservice
WEBSITE=$TMP/website

#if [ "$(which systemd-detect-virt)" != "" ]; then
#    MYVIRT=$(systemd-detect-virt)
#    if [ "$MYVIRT" == "docker" ]; then
#        GROUPSTR=":x:$(id -g):"
#        if [ "$(cat /etc/group | grep $GROUPSTR)" == "" ]; then
#            groupadd aproton -g $(id -g)
#        fi
#
#        USERSTR=:x:$(id -u):
#        if [ "$(cat /etc/passwd | grep $USERSTR)" == "" ]; then
#            useradd aproton -u $(id -u) -g $(id -g)
#        fi
#    fi
#fi
