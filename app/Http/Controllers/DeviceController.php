<?php

namespace App\Http\Controllers;

use Illuminate\Http\Request;
use App\Models\Device;

class DeviceController extends Controller
{
    const DEVICE_SESSION_TIMEOUT = 24 * 3600;

    public function store(Request $request)
    {
        $input = new ApiRequest($request, [
            'appId' => 'is_string',
            'deviceId' => 'is_string',
            'signature' => 'is_string',
        ]);

        $device = Device::where('app_id', $input->appId)
            ->where('device_id', $input->deviceId)
            ->first();

        if ($device) {
            $device->session_id = $this->_newSessionId($input);
            $device->session_timeout = strftime("%Y-%m-%d %T", time() + DeviceController::DEVICE_SESSION_TIMEOUT);
            $device->save();
        } else {
            $device = new Device();
            $device->app_id = $input->appId;
            $device->device_id = $input->deviceId;
            $device->client_id = strtoupper(substr(md5($input->appId.$input->deviceId.microtime(true)), 8, 16));
            $device->session_id = $this->_newSessionId($input);
            $device->session_timeout = strftime("%Y-%m-%d %T", time() + DeviceController::DEVICE_SESSION_TIMEOUT);
            $device->save();
        }
        
        return (new ApiResponse(ErrorCode::EC_SUCCESS))
            ->set('appId', $device->app_id)
            ->set('clientId', $device->client_id)
            ->set('session', $device->session_id)
            ->set('expire', strtotime($device->session_timeout))
            ->set('timeout', DeviceController::DEVICE_SESSION_TIMEOUT);
    }

    private function _newSessionId($input)
    {
        return md5(md5(microtime(true).$input->deviceId.$input->signature.rand(0, 10000)).$input->appId);
    }
}