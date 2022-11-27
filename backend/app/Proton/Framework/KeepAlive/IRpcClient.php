<?php

namespace Proton\Framework\KeepAlive;


interface IRpcClient
{
    public function isAvailable();
    public function sendCommand(string $appId, string $clientId, string $command, $content);
}
