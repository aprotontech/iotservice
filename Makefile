.DEFAULT_GOAL := all

TMPROOT=/tmp/aproton/build
TMP = $(TMPROOT)/output/.build-cache

tmp :
	@mkdir -p $(TMP)
	@mkdir -p $(TMPROOT)/.npm

baseimage :	
	echo "cd build/base && docker build -t aproton/system:v1.0 ."

devimage : baseimage
	echo "cd build/dev && docker build -t docker.io/aproton/iotservice-dev:latest ."

apiservice : tmp
	docker run --name=build-aproton-apiservice -it --rm -u $(shell id -u):$(shell id -g) \
	   -v $(PWD):/home/iotservice -v /tmp/aproton:/tmp/aproton -w /home/iotservice \
	   docker.io/aproton/iotservice-dev:latest bash ./build/deploy/scripts/build_apiservice.sh

website : tmp
	docker run --name=build-aproton-website -it --rm -u $(shell id -u):$(shell id -g) \
	   -v $(TMPROOT)/.npm:/.npm -v $(PWD):/home/iotservice -v /tmp/aproton:/tmp/aproton -w /home/iotservice \
	   docker.io/aproton/iotservice-dev:latest bash ./build/deploy/scripts/build_website.sh

electron: tmp
	docker run --name=build-aproton-electron -it --rm -u $(shell id -u):$(shell id -g) \
	   -v $(PWD):/home/iotservice -v /tmp/aproton:/tmp/aproton -w /home/iotservice \
	   docker.io/aproton/iotservice-dev:latest bash ./build/deploy/scripts/build_electron.sh

all : apiservice website electron
	@ echo "build finished"

image :
	docker build -t docker.io/aproton/iotservice:v1.0-alpha -f build/deploy/Dockerfile .


.PHONY : clean
clean :
	docker run --name=build-aproton-clean -it --rm -u $(shell id -u):$(shell id -g) \
	   -v $(PWD):/home/iotservice -v /tmp/aproton:/tmp/aproton -w /home/iotservice \
	   docker.io/aproton/iotservice-dev:latest bash ./build/deploy/build_cleanup.sh