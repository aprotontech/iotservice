# electron
php extenstion


## Build PHP Image
```
cd build/base
docker build -t registry.cn-beijing.aliyuncs.com/aproton/php:v1.0 .
docker push registry.cn-beijing.aliyuncs.com/aproton/php:v1.0
```

## Build electron
```
```


## Build Composer
```
cd build/composer
docker build -t registry.cn-beijing.aliyuncs.com/aproton/composer:v1.3 .
docker push registry.cn-beijing.aliyuncs.com/aproton/composer:v1.3
```

Usage:
```
docker run -v $PWD:/projects -w /projects registry.cn-beijing.aliyuncs.com/aproton/composer:v1.3 newProjectName
```


## Build laravel
```bash
cd build/laravel

docker build -t registry.cn-beijing.aliyuncs.com/aproton/laravel:v1.0 .
docker push registry.cn-beijing.aliyuncs.com/aproton/laravel:v1.0
```