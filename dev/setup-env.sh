set -ex

cd /root/iotservice/dev

apt install wget
wget http://api.aproton.tech/emqx-4.4.0-otp23.3.4.9-3-ubuntu20.04-amd64.tar.gz 
tar zxvf emqx-4.4.0-otp23.3.4.9-3-ubuntu20.04-amd64.tar.gz
rm -f emqx-4.4.0-otp23.3.4.9-3-ubuntu20.04-amd64.tar.gz

cp ../build/deploy/loaded_plugins ./emqx/data/loaded_plugins
cp ../build/deploy/emqx_auth_http.conf ./emqx/etc/plugins/emqx_auth_http.conf
cp ../build/deploy/emqx_web_hook.conf ./emqx/etc/plugins/emqx_web_hook.conf


# ./bin/emqx start