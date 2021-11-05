
# 运行

## 编译镜像
```bash



```

```
docker rm -f iotservice
docker run --name=iotservice -v $PWD:/home/admin/iotservice -w /home/admin/iotservice -p 80:8000 \
    registry.cn-beijing.aliyuncs.com/aproton/php:v1.0 php artisan serve --host=0.0.0.0
```


