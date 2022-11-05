<?php

require_once dirname(__DIR__) . '/proton_test.php';

class ProtonProcessGroupTest extends ProtonTestCase
{

    public function testMultiProcess()
    {
        $pg = new Proton\Electron\ProcessGroup();

        $server = new Proton\Electron\HttpServer("127.0.0.1", 18180, function ($server, $request) {
            utlog("[testHttpServer] server($server) new request($request)");
            utlog("pid=" . getmypid());
            $request->end(200, strval(getmypid()));
        });

        $channel = new Proton\Electron\Channel(2);
        Proton\Electron\go(function ($pg, $server, $channel) {
            utlog("startup");

            $this->assertEquals(0, $server->start($pg));

            Proton\Electron\sleep(500);

            $server->stop();

            $channel->push(1);
        }, $pg, $server, $channel);

        if ($pg->start()) { // master
            Proton\Electron\go(function ($channel) {
                Proton\Electron\sleep(100);
                $http = new Proton\Electron\HttpClient("127.0.0.1", 18180);
                $response = $http->get("http://127.0.0.1:18180/hello");

                $this->assertNotNull($response);
                $this->assertNotNull($response->getConnect());

                $s = $response->getBody();
                utlog("response: $s");

                //$this->assertNotEquals(strval(getmypid()), $s);

                $this->assertEquals($response->StatusCode, 200);

                utlog("headers %s", json_encode($response->getHeaders()));
                $this->assertNotEmpty($response->getHeaders());

                $response->getConnect()->close();

                Proton\Electron\sleep(500);
                $channel->push(1);
            }, $channel);

            Proton\Electron\go(function ($channel) {
                $channel->pop();
                $channel->pop();
                Proton\Electron\Runtime::stop();
            }, $channel);
        }

        Proton\Electron\Runtime::start();
    }
}
