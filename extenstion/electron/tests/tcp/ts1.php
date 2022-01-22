<?php


function println($s)
{
    echo $s . "\n";
}

println(extension_loaded("electron"));

quark_enable_logger(1);



function mock_server()
{
    $server = new proton\tcpserver();
    $server->listen("127.0.0.1", 18180);
    $c = $server->accept();
    println("accept=" . var_export($c, true));
    $d = $c->read(1024);
    println("read=$d");
    $r = $c->write("hello $d\n");
    println("write=$r");
    $server->close();
}


proton\go('mock_server');

proton\runtime::start();
println("FINISHED");
