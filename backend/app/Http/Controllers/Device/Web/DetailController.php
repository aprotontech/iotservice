<?php

namespace App\Http\Controllers\Device\Web;

use Illuminate\Support\Facades\DB;
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

        $attrs = DB::table('device_attributes')
            ->where('app_id', $info->app_id)
            ->where('client_id', $info->client_id)
            ->select('attr', 'value', 'report_time', 'type')
            ->get();

        foreach ($attrs as $t) {
            $t->report_time = rc_datetime($t->report_time / 1000.0, 'Y-m-d H:i:s.u');
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
