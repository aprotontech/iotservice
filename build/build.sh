set -e

BRANCH=$1
DOCKER_TAG=$2

TMP=/tmp/laravel-$(cat /proc/sys/kernel/random/uuid  | md5sum | cut -c 1-9)

echo $TMP
echo $BRANCH
echo registry.cn-beijing.aliyuncs.com/aproton/iotservice:$DOCKER_TAG


mkdir -p $TMP
cd $TMP

git clone git@github.com:aprotontech/iotservice.git

if [ "$BRANCH" != "main" ]; then
    cd $TMP/iotservice
    git checkout -b $BRANCH origin/$BRANCH
fi

cd $TMP
mv iotservice/build .
cp build/production.env $TMP/iotservice/.env

cd $TMP/iotservice
rm -rf .git* .env.example 

docker build -f ../build/Dockerfile -t registry.cn-beijing.aliyuncs.com/aproton/iotservice:$DOCKER_TAG .
docker push registry.cn-beijing.aliyuncs.com/aproton/iotservice:$DOCKER_TAG
docker image registry.cn-beijing.aliyuncs.com/aproton/iotservice:$DOCKER_TAG

rm -rf $TMP