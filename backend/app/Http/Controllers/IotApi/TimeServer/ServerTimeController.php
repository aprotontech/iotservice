<?php

namespace App\Http\Controllers\IotApi\TimeServer;

use Illuminate\Http\Request;

class ServerTimeController extends \App\Http\Controllers\Controller
{
    use \Proton\Framework\ApiControllerTrait;

    public function getFilter()
    {
        return [
            'microSecond' => 'optional | is_bool'
        ];
    }

    public function handle(Request $request)
    {
        $info = $this->reqToObject($request);

        $microSecond = isset($info->microSecond) ? $info->microSecond : false;

        return $this->success([
            'now' => $microSecond ? microtime(true) : time()
        ]);
    }
}
