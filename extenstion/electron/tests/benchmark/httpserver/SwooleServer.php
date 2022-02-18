<?php

use Swoole\Coroutine\Http\Server;
use function Swoole\Coroutine\run;

run(function () {
    $server = new Server('127.0.0.1', 18180, false);
    $server->handle('/', function ($request, $response) {
        $response->end("<h1>\nHello world!\n</h1>\n");
    });
    $server->start();
});
