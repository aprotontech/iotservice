<?php

namespace App\Http\Controllers\IotApi\KeepAlive;

use Illuminate\Http\Request;

class RpcController extends \App\Http\Controllers\ApiController
{
    use \Proton\Framework\ApiControllerTrait;

    public function getFilter()
    {
        return [
            'clientId' => 'is_string',
            'command' => 'is_string',
            'content' => is_any_of([
                'is_string',
                'is_array',
                'is_object'
            ])
        ];
    }

    public function handle(Request $request)
    {
        $input = $this->reqToObject($request);

        return $this->success([
            'now' => microtime(true)
        ]);
    }
}
