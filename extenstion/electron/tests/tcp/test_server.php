<?php

require_once dirname(__DIR__) . '/proton_test.php';

class ProtonTcpServerTest extends ProtonTestCase
{

    public function testServer()
    {
        proton\go(function ($test) {
            $test->log()->info("startup");
            $server = new proton\tcpserver();
            $test->assertEquals(0, $server->listen("127.0.0.1", 18180));
            $c = $server->accept();
            $test->assertNotNull($c);
            proton\sleep(100);
            $r = $c->write("hello test\n");
            $test->assertEquals(0, $r);

            $test->assertEquals(0, $c->close());
            $test->assertEquals(0, $server->close());
        }, $this);

        proton\go(function () {
            proton\sleep(1000);
            proton\runtime::stop();
        });

        $this->log()->info("system");
        system("telnet 127.0.0.1 18180 &");


        proton\runtime::start();
    }

    public function testClient()
    {
        proton\go(function ($test) {
            $c = new proton\tcpclient();
            $test->assertEquals(0, $c->connect("39.156.66.18", 80));

            $r = $c->write(implode("\r\n", [
                "GET /version HTTP/1.1",
                "Host: www.baidu.com",
                "Accept: */*",
                "User-Agent: aproton1.0",
                "",
                ""
            ]));

            $test->assertEquals(0, $r);

            $s = $c->read(1024);
            $test->log()->info($s);

            $test->assertTrue(strlen($s) > 0);

            $test->assertEquals(0, $c->close());

            proton\runtime::stop();
        }, $this);

        proton\runtime::start();
    }

    public function testServerClient()
    {
    }
}
