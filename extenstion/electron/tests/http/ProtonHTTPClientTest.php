<?php

require_once dirname(__DIR__) . '/proton_test.php';

class ProtonHTTPClientTest extends ProtonTestCase
{

    public function testCurlAprotonTech()
    {
        Proton\go(function ($test) {
            $http = new Proton\HttpClient("82.157.138.167", 80);
            $response = $http->get("http://api.aproton.tech/not-exists");

            $test->assertNotNull($response);
            $test->assertNotNull($response->getConnect());

            $s = $response->getBody();
            $test->log()->info($s);
            $test->assertNotEmpty($s);

            $test->assertEquals($response->StatusCode, 404);

            $test->log()->info("headers", $response->getHeaders());
            $test->assertNotEmpty($response->getHeaders());

            $response->getConnect()->close();


            Proton\sleep(500);
            Proton\Runtime::stop();
        }, $this);

        Proton\Runtime::start();
    }

    public function testCurlMultiTimes()
    {
        Proton\go(function ($test) {
            $http = new Proton\HttpClient("82.157.138.167", 80);

            for ($i = 0; $i < 2; ++$i) {
                $response = $http->get("http://api.aproton.tech/not-exists");

                $test->assertNotNull($response);
                $test->assertNotNull($response->getConnect());

                $s = $response->getBody();
                $test->log()->info($s);
                $test->assertNotEmpty($s);

                $test->assertEquals($response->StatusCode, 404);

                $test->log()->info("headers", $response->getHeaders());
                $test->assertNotEmpty($response->getHeaders());

                Proton\sleep(100);
            }

            $http->getConnect()->close();


            Proton\sleep(500);
            Proton\Runtime::stop();
        }, $this);

        Proton\Runtime::start();
    }
}
