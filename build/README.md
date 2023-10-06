# docker images


## Build Basic System Image
``` bash
cd base
docker build -t aproton/system:v1.0 .
# docker push aproton/system:v1.0
```



## Build Dev Image
```bash
cd dev

# dependence basic system image
docker build -t aproton/iotservice-dev:v1.0 .
# docker push aproton/iotservice-dev:v1.0
```


## Build Deploy Image

```bash
# first: goto project root directory
cd {PROJECT_ROOT}

docker build -t docker.io/aproton/iotservice:v1.0-alpha -f build/deploy/Dockerfile .
```

