FROM ubuntu:20.04

MAINTAINER kuper <kuper@aproton.tech>

COPY sources.list /etc/apt/sources.list

RUN apt-get update --fix-missing
RUN env DEBIAN_FRONTEND=noninteractive; apt install -y tzdata; \
    apt install -y php php-fpm
    