<?php

namespace Proton\Framework;

require_once __DIR__ . '/defines.php';

class ProtonServiceProvider extends Logger\LogServiceProvider
{
    public function boot()
    {
    }

    public function register()
    {
        parent::register();
    }
}
