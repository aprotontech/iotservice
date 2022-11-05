<?php

require_once dirname(__DIR__) . '/proton_test.php';

class ProtonTcpServerTest extends ProtonTestCase
{
    private $testResult;
    public function testServer()
    {
        $this->testResult = 0;
        Proton\Electron\go(function () {
            utlog("startup");
            $server = new Proton\Electron\TcpServer();
            $this->assertEquals(0, $server->listen("127.0.0.1", 18180));
            $c = $server->accept();
            $this->assertNotNull($c);

            for ($i = 0; $i < 1; ++$i) {
                Proton\Electron\sleep(100);
                $r = $c->write("hello test\n");
                $this->assertEquals(0, $r);
            }

            $this->assertEquals(0, $c->close());
            $this->assertEquals(0, $server->close());

            $this->testResult = 1;
        });

        Proton\Electron\go(function () {
            Proton\Electron\sleep(1000);
            Proton\Electron\Runtime::stop();
        });

        utlog("system command");
        system("echo abc | telnet 127.0.0.1 18180 &");


        Proton\Electron\Runtime::start();

        $this->assertEquals(1, $this->testResult);
    }

    public function testClient()
    {
        $this->testResult = 0;
        Proton\Electron\go(function () {
            $c = new Proton\Electron\TcpClient();
            $this->assertEquals(0, $c->connect("39.156.66.18", 80));

            $r = $c->write(implode("\r\n", [
                "GET /version HTTP/1.1",
                "Host: www.baidu.com",
                "Accept: */*",
                "User-Agent: aproton1.0",
                "",
                ""
            ]));

            $this->assertEquals(0, $r);

            $s = $c->read(1024);
            utlog($s);

            $this->assertTrue(strlen($s) > 0);

            $this->assertEquals(0, $c->close());

            Proton\Electron\Runtime::stop();
            $this->testResult = 1;
        });

        Proton\Electron\Runtime::start();
        $this->assertEquals(1, $this->testResult);
    }

    public function testServerClient()
    {
        Proton\Electron\go(function () {
            utlog("startup");
            $server = new Proton\Electron\TcpServer();
            $this->assertEquals(0, $server->listen("127.0.0.1", 18180));
            $c = $server->accept();
            $this->assertNotNull($c);
            $s = $c->read(1024);
            $this->assertEquals("client", $s);
            $r = $c->write("hello $s");
            $this->assertEquals(0, $r);

            $this->assertEquals(0, $c->close());
            $this->assertEquals(0, $server->close());
        });

        Proton\Electron\go(function () {
            Proton\Electron\sleep(100);
            $c = new Proton\Electron\TcpClient();
            $this->assertEquals(0, $c->connect("127.0.0.1", 18180));
            $this->assertEquals(0, $c->write("client"));
            $this->assertEquals("hello client", $c->read(1024));
            $this->assertEquals(0, $c->close());
        });

        Proton\Electron\go(function () {
            Proton\Electron\sleep(1000);
            Proton\Electron\Runtime::stop();
        });

        Proton\Electron\Runtime::start();
    }
}
