<?php

require_once dirname(__DIR__) . '/proton_test.php';

class ProtonTcpServerTest extends ProtonTestCase
{

    public function testServer()
    {
        Proton\go(function ($test) {
            $test->log()->info("startup");
            $server = new Proton\TcpServer();
            $test->assertEquals(0, $server->listen("127.0.0.1", 18180));
            $c = $server->accept();
            $test->assertNotNull($c);
            Proton\sleep(100);
            $r = $c->write("hello test\n");
            $test->assertEquals(0, $r);

            $test->assertEquals(0, $c->close());
            $test->assertEquals(0, $server->close());
        }, $this);

        Proton\go(function () {
            Proton\sleep(1000);
            Proton\Runtime::stop();
        });

        $this->log()->info("system");
        system("telnet 127.0.0.1 18180 &");


        Proton\Runtime::start();
    }

    public function testClient()
    {
        Proton\go(function ($test) {
            $c = new Proton\TcpClient();
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

            Proton\Runtime::stop();
        }, $this);

        Proton\Runtime::start();
    }

    public function testServerClient()
    {
        Proton\go(function ($test) {
            $test->log()->info("startup");
            $server = new Proton\TcpServer();
            $test->assertEquals(0, $server->listen("127.0.0.1", 18180));
            $c = $server->accept();
            $test->assertNotNull($c);
            $s = $c->read(1024);
            $test->assertEquals("client", $s);
            $r = $c->write("hello $s");
            $test->assertEquals(0, $r);

            $test->assertEquals(0, $c->close());
            $test->assertEquals(0, $server->close());
        }, $this);

        Proton\go(function ($test) {
            Proton\sleep(100);
            $c = new Proton\TcpClient();
            $test->assertEquals(0, $c->connect("127.0.0.1", 18180));
            $test->assertEquals(0, $c->write("client"));
            $test->assertEquals("hello client", $c->read(1024));
            $test->assertEquals(0, $c->close());
        }, $this);

        Proton\go(function () {
            Proton\sleep(1000);
            Proton\Runtime::stop();
        });

        Proton\Runtime::start();
    }
}
