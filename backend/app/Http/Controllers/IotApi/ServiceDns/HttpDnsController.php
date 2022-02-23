<?php

namespace App\Http\Controllers\IotApi\ServiceDns;

use Illuminate\Http\Request;

class HttpDnsController extends \App\Http\Controllers\ApiController
{
    use \Proton\Framework\ApiControllerTrait;

    public function getFilter()
    {
        return [
            'timestamp' => 'is_int',
            'deviceId' => 'is_string',
            'services' => is_array_of('is_string')
        ];
    }

    public function handle(Request $request)
    {
        $input = $this->reqToObject($request);

        $services = array_unique(array_merge($input->services, ['default']));

        $rs = $this->success();
        foreach ($services as $service) {
            $m = $this->getConfig($service);
            if (!$m) {
                continue;
            }

            $r = is_value_match($m, [
                'host' => 'is_string',
                'ip' => is_array_of('is_string', 1),
                'protcol' => is_map_of('is_int'),
            ]);

            if ($r !== true) {
                rclog_warning("service($service) config format error: $r");
                continue;
            }

            $rs->set($service, $m);
        }

        return $rs;
    }

    private function getConfig($service)
    {
        $configPath = env('PROTON_API_SERVICE_CONFIG_PATH', '');
        if (!$configPath) {
            $configPath = storage_path('app/services');
        }

        $servicePath = "$configPath/$service.json";

        @$m = json_decode(file_get_contents($servicePath));
        return $m;
    }
}
