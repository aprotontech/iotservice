<?php

require_once dirname(__DIR__) . '/proton_test.php';

class ProtonProcessGroupTest extends ProtonTestCase
{

    public function testMultiProcess()
    {
        $pg = new Proton\ProcessGroup();

        $server = new Proton\HttpServer("127.0.0.1", 18180, function ($server, $request) {
            $server->log->info("[testHttpServer] server($server) new request($request)");
            $server->log->info("pid=" . getmypid());
            $request->end(200, strval(getmypid()));
        });
        $server->log = $this->log();

        $channel = new Proton\Channel(2);
        Proton\go(function ($test, $pg, $server, $channel) {
            $test->log()->info("startup");

            $test->assertEquals(0, $server->start($pg));

            Proton\sleep(500);

            $server->stop();

            $channel->push(1);
        }, $this, $pg, $server, $channel);

        if ($pg->start()) { // master
            Proton\go(function ($test, $channel) {
                Proton\sleep(100);
                $http = new Proton\HttpClient("127.0.0.1", 18180);
                $response = $http->get("http://127.0.0.1:18180/hello");

                $test->assertNotNull($response);
                $test->assertNotNull($response->getConnect());

                $s = $response->getBody();
                $test->log()->info("response: $s");

                //$test->assertNotEquals(strval(getmypid()), $s);

                $test->assertEquals($response->StatusCode, 200);

                $test->log()->info("headers", $response->getHeaders());
                $test->assertNotEmpty($response->getHeaders());

                $response->getConnect()->close();

                Proton\sleep(500);
                $channel->push(1);
            }, $this, $channel);

            Proton\go(function ($channel) {
                $channel->pop();
                $channel->pop();
                Proton\Runtime::stop();
            }, $channel);
        }

        Proton\Runtime::start();
    }
}
