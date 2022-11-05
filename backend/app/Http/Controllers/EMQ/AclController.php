<?php

namespace App\Http\Controllers\EMQ;

use Illuminate\Http\Request;

class AclController extends \App\Http\Controllers\ApiController
{
    use \Proton\Framework\ApiControllerTrait;

    public function getFilter()
    {
        return [
            'clientid' => 'is_string',
            'username' => 'is_string',
            'topic' => 'is_string'
        ];
    }

    public function handle(Request $request)
    {
        return $this->success();
    }
}
