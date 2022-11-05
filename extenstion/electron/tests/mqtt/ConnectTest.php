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

        Proton\Electron\go(function () {
            $server = new Proton\Electron\TcpServer();
            $this->assertEquals(0, $server->listen("127.0.0.1", 18180));

            for ($i = 0; $i < 2; ++$i) {
                $c = $server->accept();
                $this->assertNotNull($c);
                $s = $c->read(1024); // recv connect package

                // connect success
                if (strpos($s, "_proton_admin_1") !== false) {
                    $r = $c->write(pack("cccc", 0x20, 0x02, 0x00, 0x00));
                } else {
                    $r = $c->write(pack("cccc", 0x20, 0x02, 0x00, 0x04));
                }
                $this->assertEquals(0, $r);

                Proton\Electron\sleep(100);

                $this->assertEquals(0, $c->close());
            }
            $this->assertEquals(0, $server->close());
        });

        Proton\Electron\go(function ($options) {
            $connect_test = function ($test, $options, $connRet) {
                $client = new \Proton\Electron\MqttClient("127.0.0.1", "18180");
                $ret = $client->connect($options);
                $test->assertEquals($connRet, $ret);

                \Proton\Electron\sleep(500);

                $ret = $client->close();
                $test->assertEquals(0, $ret);
            };

            $connect_test($this, $options, -1);

            $options['clientId'] = '_proton_admin_1';
            $options['userName'] = '_proton_admin_1';
            $connect_test($this, $options, 0);

            Proton\Electron\Runtime::stop();
        }, $options);

        Proton\Electron\Runtime::start();
    }
}
