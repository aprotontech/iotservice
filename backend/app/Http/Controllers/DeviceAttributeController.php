<?php

namespace App\Http\Controllers;

use App\Models\DeviceAttribute;
use Illuminate\Http\Request;

class DeviceAttributeController extends Controller
{
    /**
     * Display a listing of the resource.
     *
     * @return \Illuminate\Http\Response
     */
    public function index()
    {
        //
    }

    public function store(Request $request)
    {
        $input = new ApiRequest($request, [
            'clientId' => 'is_string',
            'attrs' => 'is_array',
            'timestamp' => 'is_int',
        ]);

        $now = intval(microtime(true) * 1000);

        foreach ($input->attrs as $key => $val) {
            $attr = DeviceAttribute::where('client_id', $input->clientId)
                ->where('attr', $key)
                ->first();
            $vtype = $this->getAttrType($val);
            if (!$attr) {
                $attr = new DeviceAttribute();
                $attr->client_id = $input->clientId;
                $attr->attr = $key;
                $attr->type = $vtype;
                $attr->value = var_export($val, true);
                $attr->device_report_time = $now;
                $attr->save();
            } else {
                if ($vtype != $attr->type) {
                    Log::warning("device($input->clientId),attr($key) new value($val), type is ($vtype) not match org type($attr->type)");
                    continue;
                }
                $attr->value = var_export($val, true);
                $attr->device_report_time = $now;
                $attr->save();
            }
        }

        return (new ApiResponse(200))
            ->set('timestamp', $now)
        ;
    }

    private function getAttrType($val)
    {
        switch (gettype($val)) {
        case "boolean": return 0;
        case "integer": return 1;
        case "double": return 2;
        case "float": return 2;
        case "string": return 3;
        default:
            Log::warning("not support type=".gettype($val));
            return -1;
        }
    }

    /**
     * Display the specified resource.
     *
     * @param  \App\Models\DeviceAttribute  $deviceAttribute
     * @return \Illuminate\Http\Response
     */
    public function show(DeviceAttribute $deviceAttribute)
    {
        //
    }

    /**
     * Update the specified resource in storage.
     *
     * @param  \Illuminate\Http\Request  $request
     * @param  \App\Models\DeviceAttribute  $deviceAttribute
     * @return \Illuminate\Http\Response
     */
    public function update(Request $request, DeviceAttribute $deviceAttribute)
    {
        //
    }

    /**
     * Remove the specified resource from storage.
     *
     * @param  \App\Models\DeviceAttribute  $deviceAttribute
     * @return \Illuminate\Http\Response
     */
    public function destroy(DeviceAttribute $deviceAttribute)
    {
        //
    }
}
