<?php

function println($s)
{
    echo $s . "\n";
}

println(extension_loaded("electron"));
quark_enable_logger(1);

function test2()
{
    for ($i = 0; $i < 3; ++$i) {
        println("test2-$i");
    }
}

function test1()
{
    for ($i = 0; $i < 3; ++$i) {
        println("test1-$i");
        proton\go('test2');
    }
}

proton\go('test1');


println("FINISHED");
