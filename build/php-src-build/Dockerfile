FROM aproton/system:v1.0

LABEL aproton.authors="kuper@aproton.tech"

RUN  apt-get update --fix-missing;

RUN apt install build-essential libxml2 libxml2-dev autoconf bison re2c pkg-config \
    sqlite3 libsqlite3-dev libuv1-dev libuv1
RUN apt-get clean

RUN make -j && make install

