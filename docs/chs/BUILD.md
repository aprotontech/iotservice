
# 编译

## 编译镜像

``` bash
docker rm -f iotservice
docker run --name=iotservice -v $PWD:/home/admin/iotservice -w /home/admin/iotservice -p 80:8000 \
    registry.cn-beijing.aliyuncs.com/aproton/php:v1.0 php artisan serve --host=0.0.0.0
```


## 前端页面
### 编译
```
rm -rf dist

npm install
npm run build
```


# 测试
## 运行后台服务

### 初始化

``` bash
# 创建数据表
php artisan migrate
# 初始化数据（创建默认用户，权限，菜单等信息）
php artisan db:seed ProtonSystemSeeder
```

### 启动服务
``` bash
cd backend
php artisan serve --host=0.0.0.0 --port=8080
```

## 运行前台服务
``` bash
cd website
npm run dev

# 服务地址
# http://localhost:9301/
```

默认用户
| | |
| -- | -- |
| 用户名 | admin |
| 密码 | admin123 |
