FROM ubuntu:20.04

COPY sources.list /etc/apt/sources.list

RUN apt-get update
RUN env DEBIAN_FRONTEND=noninteractive; apt install -y tzdata; \
    apt install -y php php-fpm
    