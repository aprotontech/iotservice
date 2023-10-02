#!/bin/bash

set -ex

CURRENT_FOLDER=$(cd "$(dirname "$0")";pwd)

cd $CURRENT_FOLDER

./vendor/bin/phpunit coroutine/
./vendor/bin/phpunit channel/
./vendor/bin/phpunit more/
./vendor/bin/phpunit tcp/
./vendor/bin/phpunit http/
./vendor/bin/phpunit mqtt/