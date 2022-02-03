<?php


function println($s)
{
    echo "[PHP] " . $s . "\n";
}

println(extension_loaded("electron"));

proton_set_logger_level(0);

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
