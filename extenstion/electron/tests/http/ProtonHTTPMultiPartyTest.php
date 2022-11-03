<?php

require_once dirname(__DIR__) . '/proton_test.php';

class ProtonHTTPMultiPartyTest extends ProtonTestCase
{

    public function __construct()
    {
        parent::__construct();
        Proton\Runtime::setErrorHandler(function ($coroutine, $error) {
            utlog("Coroutinue[$coroutine]");
            utlog("Error=" . $error->getMessage());
            $lines = explode("\n", var_export($error, true));
            foreach ($lines as $line) {
                utlog($line);
            }
        });
    }

    public function testPost()
    {
        Proton\go(function () {
            utlog("startup");
            $test = $this;
            $server = new Proton\HttpServer("127.0.0.1", 18180, function ($server, $request) use ($test) {
                utlog("[testHttpServer] server($server) new request($request)");

                $request->end(200, "testHttpServer");

                $test->assertEquals($request->Method, "POST");

                utlog("request headers %s", json_encode($request->getHeaders()));

                utlog("_POST= %s", json_encode($request->_POST));

                $test->assertEquals(count($request->_POST), 2);
                $test->assertEquals($request->_POST["key1"], "value1");
                $test->assertEquals($request->_POST["key2"], "value2");
            });
            $this->assertEquals(0, $server->start());

            Proton\sleep(200);

            $server->stop();
        });


        Proton\go(function () {
            $http = new Proton\HttpClient("127.0.0.1", 18180);
            $headers = [
                "Content-Type:multipart/form-data; boundary=ZnGpDtePMx0KrHh"
            ];
            $body = implode("\r\n", [
                "--ZnGpDtePMx0KrHh",
                'Content-Disposition: form-data;name="key1"',
                'Content-Type: text/plain; charset=UTF-8',
                '',
                'value1',
                "--ZnGpDtePMx0KrHh",
                'Content-Disposition: form-data; name="key2"',
                'Content-Type: text/plain; charset=UTF-8',
                '',
                'value2',
                "--ZnGpDtePMx0KrHh--"
            ]);


            $response = $http->post("http://127.0.0.1:18180/hello", $body, $headers);

            $this->assertNotNull($response);
            $this->assertNotNull($response->getConnect());

            $s = $response->getBody();
            utlog($s);
            $this->assertEquals("testHttpServer", $s);

            $this->assertEquals($response->StatusCode, 200);

            utlog("headers %s", json_encode($response->getHeaders()));
            $this->assertNotEmpty($response->getHeaders());

            $response->getConnect()->close();


            Proton\sleep(500);
            Proton\Runtime::stop();
        });

        Proton\Runtime::start();

        $this->assertNull(Proton\Runtime::getLastError());
    }


    public function testFiles()
    {
        Proton\go(function () {
            utlog("startup");
            $test = $this;
            $server = new Proton\HttpServer("127.0.0.1", 18180, function ($server, $request) use ($test) {
                utlog("[testHttpServer] server($server) new request($request)");

                $request->end(200, "testHttpServer");

                $test->assertEquals($request->Method, "POST");

                utlog("request headers %s", json_encode($request->getHeaders()));
                utlog("_POST= %s", json_encode($request->_POST));
                utlog("_FILES= %s", json_encode($request->_FILES));


                $test->assertEquals(count($request->_POST), 1);
                $test->assertEquals($request->_POST["key1"], "value1");

                $test->assertEquals(count($request->_FILES), 1);
                $test->assertNotNull($request->_FILES[0]);
                $test->assertEquals($request->_FILES[0]['name'], "key2");
                $test->assertEquals($request->_FILES[0]['error'], 0);
                $test->assertEquals($request->_FILES[0]['size'], 6);
                $test->assertEquals($request->_FILES[0]['type'], "text/plain; charset=UTF-8");
                $test->assertNotEmpty($request->_FILES[0]['tmp_name']);
                $test->assertTrue(file_exists($request->_FILES[0]['tmp_name']));
                $test->assertEquals(file_get_contents($request->_FILES[0]['tmp_name']), "value2");

                unlink($request->_FILES[0]['tmp_name']);

                utlog("finished");
            });
            $test->assertEquals(0, $server->start());

            Proton\sleep(200);

            $server->stop();
        });


        Proton\go(function () {
            $http = new Proton\HttpClient("127.0.0.1", 18180);
            $headers = [
                "Content-Type:multipart/form-data; boundary=ZnGpDtePMx0KrHh"
            ];
            $body = implode("\r\n", [
                "--ZnGpDtePMx0KrHh",
                'Content-Disposition: form-data;name="key1"',
                'Content-Type: text/plain; charset=UTF-8',
                '',
                'value1',
                "--ZnGpDtePMx0KrHh",
                'Content-Disposition: form-data; name="key2";filename="test.md"',
                'Content-Type: text/plain; charset=UTF-8',
                '',
                'value2',
                "--ZnGpDtePMx0KrHh--"
            ]);


            $response = $http->post("http://127.0.0.1:18180/hello", $body, $headers);

            $this->assertNotNull($response);
            $this->assertNotNull($response->getConnect());

            $s = $response->getBody();
            utlog($s);
            $this->assertEquals("testHttpServer", $s);

            $this->assertEquals($response->StatusCode, 200);

            utlog("headers %s", json_encode($response->getHeaders()));
            $this->assertNotEmpty($response->getHeaders());

            $response->getConnect()->close();


            Proton\sleep(500);
            Proton\Runtime::stop();
        }, $this);

        Proton\Runtime::start();

        $this->assertNull(Proton\Runtime::getLastError());
    }
}
