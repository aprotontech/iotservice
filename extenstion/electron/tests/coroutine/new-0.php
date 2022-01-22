<?php

function println($s)
{
    echo $s . "\n";
}



function test1()
{
    for ($i = 0; $i < 3; ++$i) {
        println("test1-$i");
    }
}

$coroutine = new proton\coroutine('test1');

$coroutine->resume();


println("FINISHED");
