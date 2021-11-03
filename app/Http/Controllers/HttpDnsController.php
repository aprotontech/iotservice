<?php

namespace App\Http\Controllers;

use Illuminate\Http\Request;

class HttpDnsController extends Controller
{
    private $dnsConfig;

    public function __construct()
    {
        $this->dnsConfig = new Dns\DnsConfig();
    }

    public function show($svr)
    {
        $ipList = $this->dnsConfig->query('', '', [$svr]);
        if (!$ipList) {
            return $this->apiResponse(404);
        }
        return $this->apiResponse(200)
                ->set('service', $ipList[$svr]);
    }
}