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

        Proton\Electron\go(function () {
            $server = new Proton\Electron\TcpServer();
            $this->assertEquals(0, $server->listen("127.0.0.1", 18180));

            $c = $server->accept();
            $this->assertNotNull($c);

            // recv connect package
            $s = $c->read(1024);
            $this->assertTrue(strlen($s) > 0);
            // connect success
            $r = $c->write(pack("cccc", 0x20, 0x02, 0x00, 0x00));
            $this->assertEquals(0, $r);

            // recv subscribe package
            $s = $c->read(1024);
            $this->assertTrue(strpos($s, "/test/topic") !== false);
            $c->write(pack('cc', 0x90, 0x03) . substr($s, 2, 2) . pack('c', 0x00));

            Proton\Electron\sleep(100);

            // write publish message
            $n = strlen("/test/topic");
            $c->write(pack("cccc", 0x30, $n + 2 + 4, 0x00, $n) . "/test/topic" . 'stop');

            Proton\Electron\sleep(100);

            $this->assertEquals(0, $c->close());
            $this->assertEquals(0, $server->close());
        });

        Proton\Electron\go(function ($options) {

            $client = new \Proton\Electron\MqttClient("127.0.0.1", "18180");

            $ret = $client->connect($options);
            $this->assertEquals(0, $ret);

            $channel = new \Proton\Electron\Channel(1);

            \Proton\Electron\go(function ($client, $channel) {
                $ret = $client->loop();
                $channel->push("loop=$ret");
            }, $client, $channel);

            \Proton\Electron\sleep(100);

            $test = $this;
            $client->subscribe("/test/topic", function ($client, $msg) use ($test) {
                $test->assertNotEquals(null, $client);
                $test->assertNotEquals(null, $msg);
                $test->assertTrue(is_array($msg));
                $test->assertTrue(isset($msg['topic']));
                $test->assertTrue(isset($msg['content']));
                utlog("%s", json_encode($msg));
                if ($msg['content'] == 'stop') {
                    utlog("recv stop command");
                    $ret = $client->close();
                    $test->assertEquals(0, $ret);
                }
            });

            $ret = $channel->pop();
            $this->assertEquals($ret, "loop=0");

            Proton\Electron\Runtime::stop();
        }, $options);

        Proton\Electron\Runtime::start();
    }
}
