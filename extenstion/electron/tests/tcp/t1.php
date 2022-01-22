<?php


function println($s)
{
    echo $s . "\n";
}

println(extension_loaded("electron"));

quark_enable_logger(1);

$client = new proton\tcpclient();

$client->connect();
$client->send();
$client->recv();
$client->close();


proton\runtime::start();
