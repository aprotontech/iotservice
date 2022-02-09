<?php

require_once dirname(__DIR__) . '/proton_test.php';

class ProtonHTTPServerTest extends ProtonTestCase
{

    public function testHttpServer()
    {
        Proton\go(function ($test) {
            $test->log()->info("startup");
            $server = new Proton\HttpServer("127.0.0.1", 18180, function ($server, $request) use ($test) {
                $test->log()->info("[testHttpServer] server($server) new request($request)");
                $request->end(200, "testHttpServer");
            });
            $test->assertEquals(0, $server->start());

            Proton\sleep(500);

            $server->stop();

            Proton\Runtime::stop();
        }, $this);

        Proton\go(function ($test) {
            $c = new Proton\TcpClient();
            $test->assertEquals(0, $c->connect("127.0.0.1", 18180));

            $r = $c->write(implode("\r\n", [
                "GET /version HTTP/1.1",
                "Accept: */*",
                "User-Agent: aproton1.0",
                "",
                ""
            ]));

            $test->assertEquals(0, $r);

            $s = $c->read(1024);
            $test->log()->info($s);

            $v = explode("\r\n", $s);
            $test->assertTrue(count($v) > 0);
            $test->assertEquals("testHttpServer", $v[count($v) - 1]);

            $test->assertTrue(strlen($s) > 0);

            $test->assertEquals(0, $c->close());
        }, $this);


        Proton\Runtime::start();
    }

    public function testCurlHttpServer()
    {
        $count = 0;
        Proton\go(function ($test) use (&$count) {
            $test->log()->info("startup");

            $server = new Proton\HttpServer("127.0.0.1", 18180, function ($server, $request) use ($test, &$count) {
                $test->log()->info("[testCurlHttpServer] server($server) new request($request)");
                $request->end(200, "testCurlHttpServer");
                $test->log()->info("client=" . strval($request->getConnect()));
                $request->getConnect()->close();

                ++$count;
            });
            $test->assertEquals(0, $server->start());

            Proton\sleep(500);

            $server->stop();
            Proton\Runtime::stop();
        }, $this);

        system("curl -v http://127.0.0.1:18180/version >/dev/null 2>&1 &");

        Proton\Runtime::start();

        $this->assertEquals(1, $count);
    }
}
