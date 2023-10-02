<?php

namespace Proton\Framework\KeepAlive;

class RpcServiceProvider extends \Illuminate\Support\ServiceProvider
{
    public function register()
    {
        $this->app->singleton('keepalive', function () {
            return new RpcService($this->app);
        });
    }
}
