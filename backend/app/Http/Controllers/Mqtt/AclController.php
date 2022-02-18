<?php

namespace App\Http\Controllers\Mqtt;

use Illuminate\Http\Request;

class AclController extends \App\Http\Controllers\ApiController
{
    use \Proton\Framework\ApiControllerTrait;

    public function getFilter()
    {
        return [
            'clientid' => 'is_string',
            'username' => 'is_string',
            'password' => 'is_string',
            'clientip' => 'is_string'
        ];
    }

    public function handle(Request $request)
    {
        return $this->success();
    }
}
