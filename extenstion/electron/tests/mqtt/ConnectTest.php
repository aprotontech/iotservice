<?php

require_once dirname(__DIR__) . '/proton_test.php';

class ConnectTest extends ProtonTestCase
{
    public function testConnect()
    {
        $channel = new \Proton\Channel(1);
        Proton\go(function ($test, $channel) {

            $client = new \Proton\MqttClient("F1010F2F001001", "127.0.0.1", "1883");
            $ret = $client->connect([
                'keepAliveInterval' => 60,
                'cleansession' => 0,
                'reliable' => 1,
                'connectTimeout' => 2,
                'retryInterval' => 30,
                'username' => 'F1010F2F001001',
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
            $test->assertEquals(0, $status);

            Proton\Runtime::stop();
        });

        Proton\Runtime::start();
    }
}
