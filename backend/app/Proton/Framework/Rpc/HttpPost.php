<?php

namespace Proton\Framework\Rpc;

class HttpPost extends \Proton\Framework\Http\HttpClient implements IMethod
{
    private $serverSession = '';

    public function __construct($service, $config)
    {
        if (!isset($config['api'])) {
            throw new \Exception("not found config api of service($service)");
        }
        if (!isset($config['timeout'])) {
            throw new \Exception("not found config timeout of service($service)");
        }

        if (!isset($config['session'])) {
            throw new \Exception("not found config server session of service($service)");
        }

        parent::__construct(strtoupper($service), $config['timeout']);
        $this->method = "POST";
        $this->setUrlPrefix($config['api']);
        $this->setResponseFormat(\Proton\Framework\Http\HttpClient::RESPONSE_RC_OBJECT);
        $this->serverSession = $config['session'];
    }

    public function __call($name, $arguments)
    {
        $data = isset($arguments[0]) ? json_encode($arguments[0]) : '';
        $path = strtolower(preg_replace('/([a-z])([A-Z])/', "$1" . '/' . "$2", $name));
        return $this->runPostRequest("/$path", $data, [
            'User-Agent: Proton-RPC-CLIENT/1.0',
            'SERVER-SUPER-SESSION: ' . $this->serverSession
        ], 0);
    }
}
