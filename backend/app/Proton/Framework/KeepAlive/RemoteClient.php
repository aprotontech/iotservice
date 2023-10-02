<?php

namespace Proton\Framework\KeepAlive;

class RemoteClient
{
    private $appId;
    private $clientId;
    private $rpcClient;

    public function __construct(string $appId, string $clientId, IRpcClient $rpcClient)
    {
        $this->appId = $appId;
        $this->clientId = $clientId;
        $this->rpcClient = $rpcClient;
    }

    public function sendRpcCommand(string $cmd, $content)
    {
        return $this->rpcClient->sendCommand($this->appId, $this->clientId, $cmd, $content);
    }

    public function __call(string $func, array $params)
    {
        return $this->rpcClient->sendCommand($this->appId, $this->clientId, $func, $params);
    }
}
