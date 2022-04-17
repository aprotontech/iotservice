<?php

namespace App\Http\Controllers\Mqtt;

use App\Http\Controllers\IotApi\DeviceUtils;
use Illuminate\Http\Request;

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
        $admin_password = env('PROTON_MQTT_ADMIN_PASSWORD', '');

        if ($admin_password && $info->password == $admin_password) {
            return $this->success();
        }

        $device = DeviceUtils::getDeviceByClientId($info->clientid);
        if ($device && $device->session == $info->password) {
            return $this->success();
        }

        return response(rc_error("400", "password mismatch"), 400);
    }
}
