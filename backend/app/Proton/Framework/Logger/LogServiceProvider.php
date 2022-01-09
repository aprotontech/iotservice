<?php

namespace Proton\Framework\Logger;

class LogServiceProvider extends \Illuminate\Support\ServiceProvider {

    public function register()
    {
        $this->app->singleton('log', function () {
            return new LogManager($this->app);
        });
    }

}
