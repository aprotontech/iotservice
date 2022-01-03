<?php

namespace App\Http\Controllers;

use App\Models\Device;
use App\Models\DeviceAttribute;

use Illuminate\Http\Request;

class DebugerController extends Controller
{

    public function getDevice(Request $request)
    {
        $input = new ApiRequest($request, [
            'clientId' => 'is_string',
        ]);

        $device = Device::where('client_id', $input->clientId)->first();
        if (!$device) {
            return new ApiResponse(404, "not found device($input->clientId)");
        }

        $attrs = DeviceAttribute::where('client_id', $input->clientId)->get();

        return (new ApiResponse(200))
            ->set($device->toArray())
            ->set('attrs', $attrs)
        ;
    }

    public function getSession(Request $request)
    {
        $input = new ApiRequest($request, [
            'session' => 'is_string',
        ]);

        $device = Device::where('session_id', $input->session)->first();
        if (!$device) {
            return new ApiResponse(404, "not found device by session($input->session)");
        }

        $attrs = DeviceAttribute::where('client_id', $device->client_id)->get();

        return (new ApiResponse(200))
            ->set($device->toArray())
            ->set('attrs', $attrs)
        ;
    }

}