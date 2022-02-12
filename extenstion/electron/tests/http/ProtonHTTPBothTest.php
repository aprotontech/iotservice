<?php

require_once dirname(__DIR__) . '/proton_test.php';

class ProtonHTTPBothTest extends ProtonTestCase
{

    public function testCurlMySelf()
    {
        Proton\go(function ($test) {
            $test->log()->info("startup");
            $server = new Proton\HttpServer("127.0.0.1", 18180, function ($server, $request) use ($test) {
                $test->log()->info("[testHttpServer] server($server) new request($request)");
                $request->end(200, "testHttpServer");
            });
            $test->assertEquals(0, $server->start());

            Proton\sleep(200);

            $server->stop();
        }, $this);


        Proton\go(function ($test) {
            $http = new Proton\HttpClient("127.0.0.1", 18180);
            $response = $http->get("http://127.0.0.1:18180/hello");

            $test->assertNotNull($response);
            $test->assertNotNull($response->getConnect());

            $s = $response->getBody();
            $test->log()->info($s);
            $test->assertEquals("testHttpServer", $s);

            $test->assertEquals($response->StatusCode, 200);

            $test->log()->info("headers", $response->getHeaders());
            $test->assertNotEmpty($response->getHeaders());

            $response->getConnect()->close();


            Proton\sleep(500);
            Proton\Runtime::stop();
        }, $this);

        Proton\Runtime::start();
    }


    public function testCurlMySelfMulittTimes()
    {
        Proton\go(function ($test) {
            $test->log()->info("startup");
            $server = new Proton\HttpServer("127.0.0.1", 18180, function ($server, $request) use ($test) {
                $test->log()->info("[testHttpServer] server($server) new request($request)");
                $request->end(200, "testHttpServer");
            });
            $test->assertEquals(0, $server->start());

            Proton\sleep(200);

            $server->stop();
        }, $this);


        $times = 0;
        Proton\go(function ($test) use (&$times) {
            $http = new Proton\HttpClient("127.0.0.1", 18180);

            for ($i = 0; $i < 10; ++$i) {
                $response = $http->get("http://127.0.0.1:18180/hello");

                $test->assertNotNull($response);
                $test->assertNotNull($response->getConnect());

                $s = $response->getBody();
                $test->log()->info($s);
                $test->assertEquals("testHttpServer", $s);

                $test->assertEquals($response->StatusCode, 200);

                $test->log()->info("headers", $response->getHeaders());
                $test->assertNotEmpty($response->getHeaders());

                ++$times;
            }

            $response->getConnect()->close();


            Proton\sleep(500);
            Proton\Runtime::stop();
        }, $this);

        Proton\Runtime::start();

        $this->assertEquals(10, $times);
    }
}
