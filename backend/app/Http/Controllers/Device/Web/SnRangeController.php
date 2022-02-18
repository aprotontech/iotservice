<?php

namespace App\Http\Controllers\Device\Web;

use DB;
use Illuminate\Http\Request;

class SnRangeController extends \App\Http\Controllers\WebController
{
    use \Proton\Framework\ApiControllerTrait;

    public $permission = 'device_view;appId=app_id';

    public function getFilter()
    {
        return [
            'app_id' => 'is_string',
        ];
    }

    public function handle(Request $request)
    {
        $info = $this->reqToObject($request);
        $minSn = DB::table('devices')
            ->where('app_id', $info->app_id)
            ->orderBy('client_id', 'asc')
            ->first();

        $maxSn = DB::table('devices')
            ->where('app_id', $info->app_id)
            ->orderBy('client_id', 'desc')
            ->first();

        return $this->success([
            'minSn' => $minSn ? $minSn->client_id : '',
            'maxSn' => $maxSn ? $maxSn->client_id : ''
        ]);
    }
}
