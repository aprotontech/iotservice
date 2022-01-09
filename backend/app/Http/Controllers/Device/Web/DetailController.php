<?php

namespace App\Http\Controllers\Device\Web;

use DB;
use Illuminate\Http\Request;

class DetailController extends \App\Http\Controllers\WebController
{
    use \Proton\Framework\ApiControllerTrait;

    public $permission = 'device_view;appId=app_id';

    public function getFilter()
    {
        return [
            'app_id' => 'is_string',
            'client_id' => 'is_string',
        ];
    }

    public function handle(Request $request)
    {
        $info = $this->reqToObject($request);
        $basic = DB::table('devices')
            ->where('app_id', $info->app_id)
            ->where('client_id', $info->client_id)
            ->first();

        if (!$basic) {
            return $this->error(404, "not found device of appId($info->app_id), clientId($info->client_id)");
        }

        $location = DB::table('device_locations')
            ->where('client_id', $info->client_id)
            ->first();

        $secret = DB::table('device_keys')
            ->where('client_id', $info->client_id)
            ->first();

        $attrs = DB::table('dp_values')
            ->join('dp_defines', 'dp_defines.property', '=', 'dp_values.property')
            ->where('app_id', $info->app_id)
            ->where('client_id', $info->client_id)
            //->select('dp_values.property', 'dp_defines.name', 'dp_values.device_val', 'dp_defines.data_type', 'dp_defines.unit_str',
            //        DB::raw("FROM_UNIXTIME(report_time/1000, '%Y-%m-%d %H:%i:%S') as report_time"))
            ->select(
                'dp_values.property',
                'dp_defines.name',
                'dp_values.device_val',
                'dp_defines.data_type',
                'dp_defines.unit_str',
                "report_time"
            )
            ->get();

        foreach ($attrs as $t) {
            if (strtolower($t->data_type) == 'bool') {
                $t->device_val = $t->device_val ? 'true' : 'false';
            } else if (trim($t->unit_str)) {
                $t->device_val = $t->device_val . " ($t->unit_str)";
            }
            $t->report_time = date('Y-m-d H:i:s', floor($t->report_time / 1000));
        }

        unset($secret->private_key);

        return $this->success([
            'basic' => $basic,
            'location' => $location,
            'secret' => $secret,
            'attrs' => $attrs
        ]);
    }
}
