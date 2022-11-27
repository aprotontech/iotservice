<?php

namespace Proton\Framework\KeepAlive;

use Proton\Electron;

class MqttRpcClient implements IRpcClient
{
    private $host;
    private $port;
    private $client;
    private $isStop  = false;
    private $tag;
    private $clientId;
    private $resChannels;

    public function __construct($host, $port)
    {
        $this->resChannels = [];
        $this->tag = substr(md5(microtime(true) . __FILE__), 8, 8);
        $this->clientId = sprintf(
            '%s%s',
            env('PROTON_MQTT_ADMIN_USERNAME_PREFIX'),
            $this->tag
        );
        $passwd = env('PROTON_MQTT_ADMIN_PASSWORD');

        $options = [
            'keepAliveInterval' => 120,
            'cleanSession' => 1,
            'reliable' => 1,
            'connectTimeout' => 2,
            'clientId' => $this->clientId,
            'userName' => $this->clientId,
            'password' => $passwd
        ];


        $this->clientLoop($options);
    }

    public function isAvailable()
    {
        return $this->client != null;
    }

    public function sendCommand(string $appId, string $clientId, string $command, $content)
    {
        $msgId = substr(md5($appId . $clientId . $command . $content . microtime(true) . 'proton'), 8, 6);
        return $this->sendCommandOfMsgId($msgId, $appId, $clientId, $command, $content);
    }

    public function sendCommandOfMsgId($msgId, string $appId, string $clientId, string $command, $content)
    {
        if ($this->client == null) {
            rclog_info("mqtt client is not connected");
            return false;
        }

        if (!preg_match('/^([a-zA-Z0-9_]|-)+$/', $command)) {
            rclog_warning("input command(%s) is invalidate", $command);
            return false;
        }

        if (!is_string($content)) {
            $content = json_encode($content);
        }

        $topic = sprintf('/aproton/%s/%s/rpc/%s/%s', $appId, $clientId, $command, $msgId);

        $this->resChannels[$msgId] = new Electron\Channel(1);

        $ret = $this->client->publish($topic, $content, 0);
        rclog_info("mqtt[%s] publish to %s ret %d", $this->tag, $topic, $ret);
        if ($ret != 0) { // 
            unset($this->resChannels[$msgId]);
            return false;
        }

        // wait for ack
        $wg = Electron\wait(Electron\go(function ($msgId) {
            $channel = $this->resChannels[$msgId];
            return $channel->pop();
        }, $msgId), Electron\after(500));

        unset($this->resChannels[$msgId]);

        $co = $wg->wait();
        if ($co && $co->getReturnValue()) {
            return $co->getReturnValue();
        }

        return false;
    }

    private function clientLoop($options)
    {
        Electron\go(function ($options) {
            while (!$this->isStop) {
                $client = new Electron\MqttClient($this->host, $this->port);
                if ($client->connect($options) != 0) {
                    Electron\sleep(1000);
                    continue;
                }

                $channel = new Electron\Channel(10);
                Electron\go(function ($client) {
                    $channel->push($client->loop());
                }, $client);

                $ret = $client->subscribe("/aproton/server/$this->clientId/rpc/ack/#", function ($topic, $content) {
                    $pos = strrpos($topic, '/');
                    if ($pos !== false) {
                        $msgId = substr($topic, $pos);
                        if (isset($this->resChannels[$msgId])) {
                            $this->resChannels[$msgId]->push($content);
                        } else {
                            rclog_warning("[MQTT] not found msgId($msgId)'s ack channel");
                        }
                    }
                });

                if ($ret != 0) {
                    $client->close();
                    Electron\sleep(1000);
                    continue;
                }

                // current client is ok now
                $this->client = $client;

                $ret = $channel->pop();
                rclog_info("mqtt client loop ret=%d", $ret);

                $this->client = null;
            }
        }, $options);
    }
}
