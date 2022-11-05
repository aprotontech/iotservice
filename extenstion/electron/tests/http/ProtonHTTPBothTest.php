<?php

require_once dirname(__DIR__) . '/proton_test.php';

class ProtonHTTPBothTest extends ProtonTestCase
{

    public function testCurlMySelf()
    {
        Proton\Electron\go(function () {
            utlog("startup");
            $server = new Proton\Electron\HttpServer("127.0.0.1", 18180, function ($server, $request) {
                utlog("[testHttpServer] server($server) new request($request)");
                $request->end(200, "testHttpServer");
            });
            $this->assertEquals(0, $server->start());

            Proton\Electron\sleep(200);

            $server->stop();
        });


        Proton\Electron\go(function () {
            $http = new Proton\Electron\HttpClient("127.0.0.1", 18180);
            $response = $http->get("http://127.0.0.1:18180/hello");

            $this->assertNotNull($response);
            $this->assertNotNull($response->getConnect());

            $s = $response->getBody();
            utlog($s);
            $this->assertEquals("testHttpServer", $s);

            $this->assertEquals($response->StatusCode, 200);

            utlog("headers %s", json_encode($response->getHeaders()));
            $this->assertNotEmpty($response->getHeaders());

            $response->getConnect()->close();


            Proton\Electron\sleep(500);
            Proton\Electron\Runtime::stop();
        });

        Proton\Electron\Runtime::start();

        $this->assertNull(Proton\Electron\Runtime::getLastError());
    }

    public function testPost()
    {
        Proton\Electron\go(function () {
            utlog("startup");
            $test = $this;
            $server = new Proton\Electron\HttpServer("127.0.0.1", 18180, function ($server, $request) use ($test) {
                utlog("[testHttpServer] server($server) new request($request)");


                $request->end(200, "testHttpServer");

                $test->assertEquals($request->Method, "POST");
                $test->assertEquals($request->getBody(), "post-content");
                $test->assertNotNull($request->getHeaders());
                $test->assertTrue(count($request->getHeaders()) > 0);
                $test->assertTrue(isset($request->getHeaders()['TEST-HEADER']));
                $test->assertEquals($request->getHeaders()['TEST-HEADER'], "xyz");

                utlog("request headers", $request->getHeaders());
            });
            $this->assertEquals(0, $server->start());

            Proton\Electron\sleep(200);

            $server->stop();
        });


        Proton\Electron\go(function () {
            $http = new Proton\Electron\HttpClient("127.0.0.1", 18180);
            $response = $http->post("http://127.0.0.1:18180/hello", "post-content", ["TEST-HEADER: xyz"]);

            $this->assertNotNull($response);
            $this->assertNotNull($response->getConnect());

            $s = $response->getBody();
            utlog($s);
            $this->assertEquals("testHttpServer", $s);

            $this->assertEquals($response->StatusCode, 200);

            utlog("headers %s", json_encode($response->getHeaders()));
            $this->assertNotEmpty($response->getHeaders());

            $response->getConnect()->close();

            Proton\Electron\sleep(500);
            Proton\Electron\Runtime::stop();
        });

        Proton\Electron\Runtime::start();


        $this->assertNull(Proton\Electron\Runtime::getLastError());
    }


    public function testCurlMySelfMulittTimes()
    {
        Proton\Electron\go(function () {
            utlog("startup");
            $server = new Proton\Electron\HttpServer("127.0.0.1", 18180, function ($server, $request) {
                utlog("[testHttpServer] server($server) new request($request)");
                $request->end(200, "testHttpServer");
            });
            $this->assertEquals(0, $server->start());

            Proton\Electron\sleep(200);

            $server->stop();
        }, $this);


        $times = 0;
        Proton\Electron\go(function () use (&$times) {
            $http = new Proton\Electron\HttpClient("127.0.0.1", 18180);

            for ($i = 0; $i < 10; ++$i) {
                $response = $http->get("http://127.0.0.1:18180/hello");

                $this->assertNotNull($response);
                $this->assertNotNull($response->getConnect());

                $s = $response->getBody();
                utlog($s);
                $this->assertEquals("testHttpServer", $s);

                $this->assertEquals($response->StatusCode, 200);

                utlog("headers %s", json_encode($response->getHeaders()));
                $this->assertNotEmpty($response->getHeaders());

                ++$times;
            }

            $response->getConnect()->close();


            Proton\Electron\sleep(500);
            Proton\Electron\Runtime::stop();
        }, $this);

        Proton\Electron\Runtime::start();

        $this->assertEquals(10, $times);
        $this->assertNull(Proton\Electron\Runtime::getLastError());
    }
}
