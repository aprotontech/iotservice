<?php

namespace App\Http\Controllers\Mqtt;

use Illuminate\Http\Request;

use App\Jobs\DeviceAlive;

class WebHookController extends \App\Http\Controllers\ApiController
{
    use \Proton\Framework\ApiControllerTrait;

    public function getFilter()
    {
        return [
            'action' => 'is_string',
            'clientid' => 'is_string'
        ];
    }

    public function handle(Request $request)
    {
        $info = $this->reqToObject($request);

        $clientId = $info->clientid;

        switch ($info->action) {
            case 'client_connected':
                DeviceAlive::dispatch($clientId, true);
                break;
            case 'client_disconnected':
                DeviceAlive::dispatch($clientId, false);
                break;
        }

        return $this->success();
    }
}
