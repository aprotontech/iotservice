<?php

namespace Proton\Framework\Rpc;

class Client {

    public static function newCaller()
    {
        return null;
    }

    public static function newDefaultCaller($service)
    {
        return new HttpPost($service, config("rpc.".strtolower($service)));
    }
  
};
