<?php

require_once dirname(__DIR__) . '/proton_test.php';

class ConnectTest extends ProtonTestCase
{
    public function testConnect()
    {
        $channel = new \Proton\Channel(1);
        Proton\go(function ($test, $channel) {

            $client = new \Proton\MqttClient("127.0.0.1", "1883");
            $ret = $client->connect([
                'keepAliveInterval' => 10,
                'cleanSession' => 1,
                'reliable' => 1,
                'connectTimeout' => 2,
                'retryInterval' => 30,
                'clientId' => "F1010F2F001001",
                'userName' => 'F1010F2F001001',
                'password' => '_mqtt_admin_password'
            ], $channel);
            $test->assertEquals(0, $ret);

            \Proton\sleep(500);

            $ret = $client->close();
            $test->assertEquals(0, $ret);
        }, $this, $channel);

        Proton\go(function ($test, $channel) {
            // wait for closed
            $status = $channel->pop();
            $test->assertEquals(0, $status['status']);

            Proton\Runtime::stop();
        }, $this, $channel);

        Proton\Runtime::start();
    }
}
