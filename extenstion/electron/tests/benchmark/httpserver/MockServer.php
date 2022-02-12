<?php

require_once dirname(dirname(__DIR__)) . '/vendor/autoload.php';

use Monolog\Logger;
use Monolog\Handler\StreamHandler;

//proton_set_logger_level(0);

$server = new Proton\HttpServer("127.0.0.1", 18180, function ($server, $request) {
    //$server->logger->info("headers", $request->getHeaders());
    $request->end(200, "OK");
});

$log = new Logger('Tester');
$log->pushHandler((new StreamHandler('php://stdout', Logger::DEBUG, true)));
$server->logger = $log;

Proton\go(function ($server) {
    $server->start();
}, $server);

Proton\Runtime::start();
