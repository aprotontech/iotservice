<?php

function println($s)
{
    echo $s . "\n";
}

quark_enable_logger(1);


function test1()
{
    for ($i = 0; $i < 3; ++$i) {
        println("test1-$i");
        if ($i == 1) {
            proton\context()->yield();
        }
    }
}

$coroutinue = proton\go('test1');

$coroutinue->resume();


println("FINISHED");
