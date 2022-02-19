<?php

namespace App\Http\Controllers\Mqtt;

use Illuminate\Http\Request;

use App\Models\Device;

use App\Jobs\DeviceAlive;

class AuthController extends \App\Http\Controllers\ApiController
{
    use \Proton\Framework\ApiControllerTrait;

    public function getFilter()
    {
        return [
            'clientid' => 'is_string & is_not_empty',
            'username' => 'is_string',
            'password' => 'is_string & is_not_empty',
            'clientip' => 'optional | is_string'
        ];
    }

    public function handle(Request $request)
    {
        $info = $this->reqToObject($request);

        if ($info->password != "_aproton_super_admin") {
            return response(rc_error("400", "password mismatch"), 400);
        }

        $clientId = $info->clientid;

        DeviceAlive::dispatch($clientId);

        return $this->success();
    }
}
