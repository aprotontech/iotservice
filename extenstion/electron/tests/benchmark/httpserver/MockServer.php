<?php

require_once dirname(dirname(__DIR__)) . '/vendor/autoload.php';


$server = new Proton\HttpServer("127.0.0.1", 18180, function ($server, $request) {

    $request->end(200, "<h1>\nHello world!\n</h1>\n");
});


Proton\go(function ($server) {
    $server->start();
}, $server);

Proton\Runtime::start();
