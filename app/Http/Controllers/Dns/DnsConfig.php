<?php

namespace App\Http\Controllers\Dns;

class DnsConfig 
{
    private $dnsServicesMap;

    public function __construct() 
    {
        $this->dnsServicesMap = [
            'device' => [
                'host' => 'device.aproton.tech',
                'ip' => ['192.168.0.106'],
                'protcol' => ['http'],
            ],
        ];
    }

    public function query($appId, $clientId, $services)
    {
        $ipList = [];
        foreach ($services as $svr) {
            if (array_key_exists($svr, $this->dnsServicesMap)) {
                $ipList[$svr] = $this->dnsServicesMap[$svr];
            }
        }

        return $ipList;
    }
}