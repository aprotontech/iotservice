<?php

require_once dirname(__DIR__) . '/proton_test.php';

class ProtonHTTPServerTest extends ProtonTestCase
{

    public function testHttpServer()
    {
        Proton\go(function () {
            utlog("startup");
            $server = new Proton\HttpServer("127.0.0.1", 18180, function ($server, $request) {
                utlog("[testHttpServer] server($server) new request($request)");
                $request->end(200, "testHttpServer");
            });
            $this->assertEquals(0, $server->start());

            Proton\sleep(500);

            $server->stop();

            Proton\Runtime::stop();
        });

        Proton\go(function () {
            $c = new Proton\TcpClient();
            $this->assertEquals(0, $c->connect("127.0.0.1", 18180));

            $r = $c->write(implode("\r\n", [
                "GET /version HTTP/1.1",
                "Accept: */*",
                "User-Agent: aproton1.0",
                "",
                ""
            ]));

            $this->assertEquals(0, $r);

            $s = $c->read(1024);
            utlog($s);

            $v = explode("\r\n", $s);
            $this->assertTrue(count($v) > 0);
            $this->assertEquals("testHttpServer", $v[count($v) - 1]);

            $this->assertTrue(strlen($s) > 0);

            $this->assertEquals(0, $c->close());
        });


        Proton\Runtime::start();


        $this->assertNull(Proton\Runtime::getLastError());
    }

    public function testCurlHttpServer()
    {
        $count = 0;
        Proton\go(function () use (&$count) {
            utlog("startup");

            $server = new Proton\HttpServer("127.0.0.1", 18180, function ($server, $request) use (&$count) {
                utlog("[testCurlHttpServer] server($server) new request($request)");
                $request->end(200, "testCurlHttpServer");
                utlog("client=" . strval($request->getConnect()));
                $request->getConnect()->close();

                ++$count;
            });
            $this->assertEquals(0, $server->start());

            Proton\sleep(500);

            $server->stop();
            Proton\Runtime::stop();
        });

        system("curl -v http://127.0.0.1:18180/version >/dev/null 2>&1 &");

        Proton\Runtime::start();

        $this->assertEquals(1, $count);
    }
}
