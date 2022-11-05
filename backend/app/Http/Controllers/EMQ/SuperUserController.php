<?php

namespace App\Http\Controllers\EMQ;

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

        $admin_user_prefix = env('PROTON_MQTT_ADMIN_USERNAME_PREFIX', '');

        if ($admin_user_prefix && str_starts_with($info->username, $admin_user_prefix)) {
            return $this->success();
        }

        return response(rc_error(400, "input client($info->clientid) is not super"), 400);
    }
}
