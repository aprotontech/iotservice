<?php

function println($s)
{
    echo $s . "\n";
}

println(extension_loaded("electron"));


function test1()
{
    println("test1[" . time() . "]");
    proton\sleep(2000);

    println("test1[" . time() . "]");

    proton\runtime::stop();
}

proton\go('test1');

proton\runtime::start();


println("FINISHED");
