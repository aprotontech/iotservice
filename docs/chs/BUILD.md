
# 编译


## 编译部署镜像
如果只是想快速编译部署镜像，并不期望关注编译细节，可以在根目录中执行下面的命令生成部署用镜像，在结合[部署](./INSTALL.md) 快速启动
```bash
cd {PROJECT_ROOT}
make image
```

## 开发镜像

参考文档： [开发镜像](../../build/README.md)

## 启动开发容器
使用下面的命令启动开发容器
``` bash
# 注意将PROJECT_ROOT调整为实际项目根目录
PROJECT_ROOT=$PWD
docker run --name=iotservice -v $PROJECT_ROOT:/root/iotservice \
    -w /root/iotservice -p 8080:8080 -it aproton/iotservice-dev:latest /bin/bash
```

### 更新开发镜像
开发镜像使用的Dockerfile 参考 [这里](../../build/dev/Dockerfile)
如果有更新，请执行
```bash
cd ../../build/dev
export TAG=v$(date "+%Y%m%d%H%M%S")
docker build --rm -t aproton/iotservice-dev:$TAG .
docker tag aproton/iotservice-dev:$TAG aproton/iotservice-dev:latest
docker push aproton/iotservice-dev:$TAG 
docker push aproton/iotservice-dev:latest
```


## 前端页面
### 编译
``` bash

cd ./website

rm -rf dist

npm install

npm run build
```


## 后台代码vendor更新
``` bash

cd ./backend

composer update -vvv

```

## PHP扩展编译（可选）
electron扩展是一个基于libuv的协程扩展，自带http服务，用来加速后台PHP服务的访问，且支持MQTT协议，将整个后台服务和MQTT broker统一到一个服务中，省去了部署多个服务的困扰，且性能更快。 如果有需要可以编译，也可以不编译，使用普通的HTTP,MQTT服务
``` bash

cd ./extenstion/electron

phpize
./configure
make -j8
make install

# 如果需要electron扩展生效，需要修改文件 /etc/php/7.4/cli/php.ini 增加一行 extension=electron
# echo "extension=electron" >> /etc/php/7.4/cli/php.ini
```



# 测试
## 运行后台服务

### 初始化

``` bash
cd ./backend
# 创建Laravel配置文件： 将.env.example 文件复制到 .env
cp .env.example .env

# 启动mysql服务
service mysql start

# 创建测试数据库
# PS： 默认情况下初始安装的mysql，root账号没有密码，如果自己修改了密码，或者使用其他数据库可自行调整命令
mysql -uroot -e "create database aproton"

# 创建数据库用户密码
# 如果已有用户密码，可以跳过这一步，但是需要注意后续命令中的用户名密码使用已有的
# 同时需要关注 .env 中的 DB_USERNAME/DB_PASSWORD 配置
mysql -uroot -e "CREATE USER 'aproton' IDENTIFIED WITH mysql_native_password BY 'LDloAczOvBu7GkeP'"
mysql -uroot -e "grant all privileges on aproton.* to 'aproton'@'%'"

# 生成laravel key
php artisan key:generate

# 创建数据表
php artisan migrate
# 初始化数据（创建默认用户，权限，菜单等信息）
php artisan db:seed ProtonSystemSeeder
```

### 启动服务
``` bash
cd ./backend
php artisan serve --host=0.0.0.0 --port=8080
```

## 运行前台服务
``` bash

cd ./website
npm run dev

# 页面地址
# http://localhost:9301/
```

默认用户（参考上文中初始化数据步骤）
| | |
| -- | -- |
| 用户名 | admin |
| 密码 | admin123 |


## 启动MQTT服务
目前项目使用EMQX作为mqtt broker，具体使用参考EMQX官方文档安装[EMQX](https://www.emqx.io/docs/zh/v4.4/)；

认证配置文件可以参考： {PROJECT_ROOT}/build/deploy/emqx_auth_http.conf
