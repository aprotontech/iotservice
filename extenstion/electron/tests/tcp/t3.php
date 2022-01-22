<?php


function println($s)
{
    echo $s . "\n";
}

println(extension_loaded("electron"));

quark_enable_logger(0);

define('QUARK_TEST_PORT', 18180);

function mock_server()
{
    $server = new proton\tcpserver();

    $server->listen("127.0.0.1", QUARK_TEST_PORT);

    $client = $server->accept();
    println("[SERVER] [accept] client=$client");

    $body = $client->read(1024);
    println("[SERVER] [recv] body=$body");

    $err = $client->write("hello " . $body . "!");
    println("[SERVER] [send] err=$err");

    $client->close();

    println("sleep 1s");
    proton\sleep(1000);
    println("sleep done");

    $server->close();

    proton\runtime::stop();
}

function mock_client()
{
    proton\sleep(100);

    $client = new proton\tcpclient();

    $err = $client->connect("127.0.0.1", QUARK_TEST_PORT);
    println("[CLIENT] [connect] err=$err");
    $err = $client->write("client");
    println("[CLIENT] [send] err=$err");
    $body = $client->read(1024);
    println("[CLIENT] [recv] body=$body");

    $client->close();
}

proton\go('mock_server');
proton\go('mock_client');

proton\runtime::start();


println("FINISHED");
