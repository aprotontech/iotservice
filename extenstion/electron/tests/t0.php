<?php

function println($s)
{
    echo $s . "\n";
}

println(extension_loaded("electron"));

quark_enable_logger(1);

function test()
{
    println(var_export(proton\context(), true));
    for ($i = 0; $i < 3; ++$i) {
        println("test-$i");
    }
}

proton\go('test');


println("FINISHED");
