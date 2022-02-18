<?php

namespace App\Http\Controllers\Mqtt;

use Illuminate\Http\Request;

class SuperUserController extends \App\Http\Controllers\ApiController
{
    use \Proton\Framework\ApiControllerTrait;

    public function getFilter()
    {
        return [
            'clientid' => 'is_string & is_not_empty',
            'username' => 'is_string',
        ];
    }

    public function handle(Request $request)
    {
        $info = $this->reqToObject($request);

        if (true) {
            return response(rc_error(400, "input client($info->clientid) is not super"), 400);
        }
        return $this->success();
    }
}
