FROM ubuntu:20.04

MAINTAINER kuper <kuper@aproton.tech>

COPY sources.list /etc/apt/sources.list

RUN apt-get update --fix-missing; DEBIAN_FRONTEND=noninteractive apt-get install -y tzdata php php-fpm --fix-missing && rm -rf /var/lib/apt/lists/*
    