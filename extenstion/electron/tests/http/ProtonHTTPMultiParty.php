<?php

require_once dirname(__DIR__) . '/proton_test.php';

class ProtonHTTPMultiParty extends ProtonTestCase
{

    public function __construct()
    {
        parent::__construct();
        $log = $this->log();
        Proton\Runtime::setErrorHandler(function ($coroutine, $error) use ($log) {
            $log->info("Coroutinue[$coroutine]");
            $log->error("Error=" . $error->getMessage());
            $lines = explode("\n", var_export($error, true));
            foreach ($lines as $line) {
                $log->info($line);
            }
        });
    }

    public function testPost()
    {
        Proton\go(function ($test) {
            $test->log()->info("startup");
            $server = new Proton\HttpServer("127.0.0.1", 18180, function ($server, $request) use ($test) {
                $test->log()->info("[testHttpServer] server($server) new request($request)");

                $request->end(200, "testHttpServer");

                $test->assertEquals($request->Method, "POST");

                $test->log()->info("request headers", $request->getHeaders());

                $test->log()->info("_POST=", $request->_POST);

                $test->assertEquals(count($request->_POST), 2);
                $test->assertEquals($request->_POST["key1"], "value1");
                $test->assertEquals($request->_POST["key2"], "value2");
            });
            $test->assertEquals(0, $server->start());

            Proton\sleep(200);

            $server->stop();
        }, $this);


        Proton\go(function ($test) {
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


    public function testFiles()
    {
        Proton\go(function ($test) {
            $test->log()->info("startup");
            $server = new Proton\HttpServer("127.0.0.1", 18180, function ($server, $request) use ($test) {
                $test->log()->info("[testHttpServer] server($server) new request($request)");

                $request->end(200, "testHttpServer");

                $test->assertEquals($request->Method, "POST");

                $test->log()->info("request headers", $request->getHeaders());

                $test->log()->info("_POST=", $request->_POST);
                $test->log()->info("_FILES=", $request->_FILES);

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

                $test->log()->info("finished");
            });
            $test->assertEquals(0, $server->start());

            Proton\sleep(200);

            $server->stop();
        }, $this);


        Proton\go(function ($test) {
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
}
