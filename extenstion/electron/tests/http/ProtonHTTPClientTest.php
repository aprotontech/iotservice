<?php

require_once dirname(__DIR__) . '/proton_test.php';

class ProtonHTTPClientTest extends ProtonTestCase
{

    public function testCurlAprotonTech()
    {
        Proton\go(function () {
            $http = new Proton\HttpClient("82.157.138.167", 80);
            $response = $http->get("http://api.aproton.tech/not-exists");

            $this->assertNotNull($response);
            $this->assertNotNull($response->getConnect());

            $s = $response->getBody();
            utlog($s);
            $this->assertNotEmpty($s);

            $this->assertEquals($response->StatusCode, 404);

            utlog("headers %s", json_encode($response->getHeaders()));
            $this->assertNotEmpty($response->getHeaders());

            $response->getConnect()->close();


            Proton\sleep(500);
            Proton\Runtime::stop();
        });

        Proton\Runtime::start();

        $this->assertNull(Proton\Runtime::getLastError());
    }

    public function testCurlMultiTimes()
    {
        Proton\go(function () {
            $http = new Proton\HttpClient("82.157.138.167", 80);

            for ($i = 0; $i < 2; ++$i) {
                $response = $http->get("http://api.aproton.tech/not-exists");

                $this->assertNotNull($response);
                $this->assertNotNull($response->getConnect());

                $s = $response->getBody();
                utlog($s);
                $this->assertNotEmpty($s);

                $this->assertEquals($response->StatusCode, 404);

                utlog("headers %s", json_encode($response->getHeaders()));
                $this->assertNotEmpty($response->getHeaders());

                Proton\sleep(100);
            }

            $http->getConnect()->close();


            Proton\sleep(500);
            Proton\Runtime::stop();
        });

        Proton\Runtime::start();

        $this->assertNull(Proton\Runtime::getLastError());
    }
}
