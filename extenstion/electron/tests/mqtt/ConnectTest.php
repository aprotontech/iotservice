<?php

require_once dirname(__DIR__) . '/proton_test.php';

class ConnectTest extends ProtonTestCase
{
    public function testConnect()
    {
        $options = [
            'keepAliveInterval' => 10,
            'cleanSession' => 1,
            'reliable' => 1,
            'connectTimeout' => 2,
            'retryInterval' => 30,
            'clientId' => "F1010F2F001001",
            'userName' => 'F1010F2F001001',
            'password' => '_mqtt_admin_password'
        ];

        Proton\go(function ($test, $options) {
            $connect_test = function ($test, $options, $connRet) {
                $client = new \Proton\MqttClient("127.0.0.1", "1883");
                $ret = $client->connect($options, function ($client, $state) {
                    echo "$client,$state\n";
                });
                $test->assertEquals($connRet, $ret);

                \Proton\sleep(500);

                $ret = $client->close();
                $test->assertEquals(0, $ret);
            };

            $connect_test($test, $options, -1);

            $options['clientId'] = '_proton_admin_1';
            $options['userName'] = '_proton_admin_1';
            $connect_test($test, $options, 0);

            Proton\Runtime::stop();
        }, $this, $options);

        Proton\Runtime::start();
    }
}
