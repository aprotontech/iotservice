<?php

namespace App\Http\Controllers\IotApi\Location;

use Illuminate\Http\Request;

use Illuminate\Support\Facades\DB;

class ReportController extends \App\Http\Controllers\Controller
{
    use \Proton\Framework\ApiControllerTrait;

    public function getFilter()
    {
        return [
            'mac' => 'is_string',
            'wifiList' => is_array_of([
                'mac' => 'is_string',
                'signal' => 'is_int',
                'ssid' => 'is_string'
            ], 1)
        ];
    }

    public function handle(Request $request)
    {
        $info = $this->reqToObject($request);

        //list($appId, $clientId) = $this->getClientInfo($request);
        //$arr['ip'] = $request->getClientIp();


        $service = new GaodeService();
        $m = $service->getLocation($info->mac, $info->wifiList);
        if (!$m) {
            return $this->error("1530", "get location failed! please config LOG_GEO_DETAIL to see more info in logger");
        }

        //$this->saveLocation($clientId, $request->getClientIp(), (object)$m);
        //unset($m['city']);
        //unset($m['type']);

        return $this->success($m);
    }

    private function saveLocation($clientId, $ip, $loc)
    {
        $params = [
            'latitude' => $loc->lat,
            'longitude' => $loc->lng,
            'address' => $loc->addr,
            'country' => isset($loc->city['country']) ? $loc->city['country'] : '',
            'province' => isset($loc->city['province']) ? $loc->city['province'] : '',
            'city' => isset($loc->city['city']) ? $loc->city['city'] : '',
            'ip' => $ip,
            'updated_at' => rc_datetime(),
        ];

        $r = DB::table('device_locations')
            ->where('client_id', $clientId)
            ->update($params);

        if (!$r) {
            try {
                $params['created_at'] = $params['updated_at'];
                $params['client_id'] = $clientId;
                $r = DB::table('device_locations')
                    ->where('client_id', $clientId)
                    ->insert($params);
            } catch (\Exception $e) {
                rclog_exception($e);
            }
        }

        return $r;
    }
}
