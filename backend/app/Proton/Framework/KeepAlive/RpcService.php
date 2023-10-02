<?php

namespace Proton\Framework\KeepAlive;


class RpcService
{
    private array $clients;

    public function __construct()
    {
        $this->clients = [];

        if (strtolower(php_sapi_name()) == 'cli') {
            $count = intval(env('PROTON_MQTT_RPCCLIENT_COUNT', '1'));
            for ($i = 0; $i < $count; ++$i) {
                $this->clients[$i] = new MqttRpcClient(env('PROTON_MQTT_HOST'), env('PROTON_MQTT_PORT'));
            }
        }
    }

    public function getRemoteClient($appId, $clientId)
    {
        $rpcClient = null;
        $csize = count($this->clients);
        for ($idx = rand(0, $csize), $i = 0; $i < $csize; ++$i, ++$idx) {
            if ($this->clients[$idx] != null && $this->clients[$idx]->isAvailable()) {
                $rpcClient = $this->clients[$idx];
            }
        }

        if ($rpcClient == null) {
            return null;
        }

        return new RemoteClient($appId, $clientId, $rpcClient);
    }
}
