<?php

require_once dirname(__DIR__) . '/proton_test.php';

class SubscribeTest extends ProtonTestCase
{
    public function testConnect()
    {
        $options = [
            'keepAliveInterval' => 10,
            'cleanSession' => 1,
            'reliable' => 1,
            'connectTimeout' => 2,
            'retryInterval' => 30,
            'clientId' => "_proton_admin_1",
            'userName' => '_proton_admin_1',
            'password' => '_mqtt_admin_password'
        ];

        Proton\go(function ($options) {

            $client = new \Proton\MqttClient("127.0.0.1", "1883");

            $ret = $client->connect($options);
            $this->assertEquals(0, $ret);

            $channel = new \Proton\Channel(1);

            \Proton\go(function ($client, $channel) {
                $ret = $client->loop();
                $channel->push("loop=$ret");
            }, $client, $channel);

            \Proton\sleep(100);

            $test = $this;
            $client->subscribe("/test/topic", function ($client, $msg) use ($test) {
                $test->assertNotEquals(null, $client);
                $test->assertNotEquals(null, $msg);
                $test->assertTrue(is_array($msg));
                $test->assertTrue(isset($msg['topic']));
                $test->assertTrue(isset($msg['content']));
                utlog("%s", json_encode($msg));
                if ($msg['content'] == 'stop') {
                    $ret = $client->close();
                    $test->assertEquals(0, $ret);
                }
            });

            Proton\go(function ($client) {
                Proton\sleep(300);
                $ret = $client->publish("/test/topic", "stop", 0);
                $this->assertEquals(0, $ret);
            }, $client);

            $ret = $channel->pop();
            $this->assertEquals($ret, "loop=0");

            Proton\Runtime::stop();
        }, $options);

        Proton\Runtime::start();
    }
}
