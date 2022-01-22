<?php

function println($s)
{
    echo $s . "\n";
}



function test1($v)
{
    println("status=" . proton\context()->status());
    for ($i = 0; $i < 3; ++$i) {
        println("test1-$v-$i");
    }
}

$coroutine = new proton\coroutine('test1', 100);

println("status=" . $coroutine->status());

$coroutine->resume();

println("status=" . $coroutine->status());

println("FINISHED");
