<?php

namespace App\Http\Controllers\IotApi\Property;

use App\Http\Controllers\IotApi\DeviceUtils;
use App\Models\DeviceAttribute;
use Illuminate\Http\Request;

class DeviceAttributeController extends \App\Http\Controllers\ApiController
{
    use \Proton\Framework\ApiControllerTrait;

    public function getFilter()
    {
        return [
            'clientId' => 'is_string',
            'timestamp' => 'is_int',
            'attrs' => 'is_object',
        ];
    }

    public function handle(Request $request)
    {
        $input = $this->reqToObject($request);

        $now = intval(microtime(true) * 1000);

        $device = DeviceUtils::getDeviceByClientId($input->clientId);

        foreach ($input->attrs as $key => $val) {
            $attr = DeviceAttribute::where('client_id', $input->clientId)
                ->where('attr', $key)
                ->first();
            $vtype = $this->getAttrType($val);
            if (!$attr) {
                $attr = new DeviceAttribute();
                $attr->app_id = $device->app_id;
                $attr->client_id = $input->clientId;
                $attr->attr = $key;
                $attr->type = $vtype;
                $attr->value = $vtype == 'bool' ? var_export($val, true) : "$val";
                $attr->report_time = $now;
                $attr->save();
            } else {
                if ($vtype != $attr->type) {
                    rclog_warning("device($input->clientId),attr($key) new value($val), type is ($vtype) not match org type($attr->type)");
                    continue;
                }
                $attr->value = $vtype == 'bool' ? var_export($val, true) : "$val";
                $attr->report_time = $now;
                $attr->save();
            }
        }

        return $this->success()
            ->set('timestamp', $now);
    }

    private function getAttrType($val)
    {
        switch (gettype($val)) {
            case "boolean":
                return 'bool';
            case "integer":
                return 'int';
            case "double":
            case "float":
                return "float";
            case "string":
                return "string";
            default:
                rclog_warning("not support type=" . gettype($val));
                return 'string';
        }
    }
}
