<?php

namespace App\Http\Controllers\IotApi\Location;

use Proton\Framework\Http\HttpClient;

class GaodeService
{
    protected $apiKey;

    public function __construct()
    {
    }

    public function getLocation($wifiList)
    {
        // remove too many wifis
        $wifi = [];
        foreach ($this->removeTooManyWifis($wifiList) as $t) {
            $ssid = urlencode($t->ssid);
            array_push(
                $wifi,
                "$t->mac,$t->signal,$ssid"
            );
        }

        $gaodeKey = env('PROTON_GAODE_APIKEY', '');
        if (!$gaodeKey) {
            return rc_error(1508, "not found gaode key, please config it first");
        }

        $macs = join("%7C", $wifi);
        $url = 'http://apilocate.amap.com/position?' .
            "key=$gaodeKey&" .
            "imei=356823030198134&smac=E0:DB:55:E4:C7:49&" .
            "output=json&accesstype=1&" .
            "macs=$macs";
        $http = new HttpClient("GAODE", 3);
        $response = $http->get($url);

        @$r = json_decode($response, false);

        $cs = is_value_match($r, [
            'status' => 'is_numeric',
            'result' => [
                'type' => 'is_numeric',
                'location' => 'is_string',
            ]
        ]);

        if ($cs !== true) {
            rclog_warning("gaode response($response) is not match format. error($cs)");
            return null;
        }

        list($lng, $lat) = explode(',', $r->result->location);

        return [
            'latitude' => floatval($lat),
            'longitude' => floatval($lng),
            'city' => $r->result->city,
            'address' => $r->result->desc,
            'province' => $r->result->province,
            'country' => $r->result->country,
            'radius' => floatval($r->result->radius),
        ];
    }

    private function removeTooManyWifis($wifiList)
    {
        $max_wifi_count = 15;
        if (defined('APROTON_MAX_WIFI_COUNT')) {
            $max_wifi_count = env('APROTON_MAX_WIFI_COUNT');
        }

        if (!$wifiList || count($wifiList) <= $max_wifi_count) {
            return $wifiList;
        }
        $signals = array();
        foreach ($wifiList as $wifi) {
            array_push($signals, $wifi->signal);
        }

        sort($signals);
        $signal = $signals[$max_wifi_count];

        $new_array = array();
        foreach ($wifiList as $wifi) {
            if ($wifi->signal <= $signal && count($new_array) < $max_wifi_count) {
                array_push($new_array, $wifi);
            }
        }

        return $new_array;
    }
}
