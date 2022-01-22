<?php


function println($s)
{
    echo $s . "\n";
}

println(extension_loaded("electron"));

quark_enable_logger(1);

$server = new proton\tcpserver();


$server->listen("127.0.0.1", 18180);
$server->accept();
$server->close();

proton\runtime::start();
println("FINISHED");
