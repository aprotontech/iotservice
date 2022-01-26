<?php


function println($s)
{
    echo $s . "\n";
}

println(extension_loaded("electron"));

quark_enable_logger(1);

function handler($server, $request)
{
    println("$server-$request");
    $request->end(200, "OK");
    proton\sleep(300);
}

proton\go(function () {
    $server = new proton\httpserver("127.0.0.1", 18088, 'handler');
    $server->start();
});

proton\runtime::start();
println("FINISHED");
