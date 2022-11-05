<?php

require_once dirname(__DIR__) . '/proton_test.php';

class ProtonHTTPClientTest extends ProtonTestCase
{

    public function testCurlAprotonTech()
    {
        Proton\Electron\go(function () {
            $http = new Proton\Electron\HttpClient("82.157.138.167", 80);
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


            Proton\Electron\sleep(500);
            Proton\Electron\Runtime::stop();
        });

        Proton\Electron\Runtime::start();

        $this->assertNull(Proton\Electron\Runtime::getLastError());
    }

    public function testCurlMultiTimes()
    {
        Proton\Electron\go(function () {
            $http = new Proton\Electron\HttpClient("82.157.138.167", 80);

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

                Proton\Electron\sleep(100);
            }

            $http->getConnect()->close();


            Proton\Electron\sleep(500);
            Proton\Electron\Runtime::stop();
        });

        Proton\Electron\Runtime::start();

        $this->assertNull(Proton\Electron\Runtime::getLastError());
    }
}
