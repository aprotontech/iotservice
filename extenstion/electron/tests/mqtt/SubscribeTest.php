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

        Proton\go(function ($test, $options) {

            $client = new \Proton\MqttClient("127.0.0.1", "1883");
            $ret = $client->connect($options, function ($client, $state) {
                echo "$client,$state\n";
            });
            $test->assertEquals(0, $ret);

            \Proton\sleep(500);

            $ret = $client->close();
            $test->assertEquals(0, $ret);


            $client->subscribe("/test/topic", function ($client, $topic, $msg, $qos) {
            });


            Proton\Runtime::stop();
        }, $this, $options);

        Proton\Runtime::start();
    }
}
