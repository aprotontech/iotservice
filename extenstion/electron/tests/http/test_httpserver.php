<?php

require_once dirname(__DIR__) . '/proton_test.php';

class ProtonHTTPServerTest extends ProtonTestCase
{

    public function testHttpServer()
    {
        proton\go(function ($test) {
            $test->log()->info("startup");
            $server = new proton\httpserver("127.0.0.1", 18180, function ($server, $request) use ($test) {
                $test->log()->info("new request($request)");
                $request->end(200, "OK");
            });
            $server->start();
        }, $this);

        proton\go(function ($test) {
            $c = new proton\tcpclient();
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
            $test->assertEquals("OK", $v[count($v) - 1]);

            $test->assertTrue(strlen($s) > 0);

            $test->assertEquals(0, $c->close());

            proton\runtime::stop();
        }, $this);


        proton\runtime::start();
    }
}
