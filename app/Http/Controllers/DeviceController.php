<?php

namespace App\Http\Controllers;

use Illuminate\Http\Request;

class DeviceController extends Controller
{
    public function create(Request $request)
    {
        $input = new ApiRequest($request, [
            'appId' => 'is_string',
            'deviceId' => 'is_string',
            'deviceSecret' => 'is_string',
        ]);

        
        
    }
}