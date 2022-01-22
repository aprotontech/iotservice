<?php

function println($s)
{
    echo $s . "\n";
}

println(extension_loaded("electron"));

quark_enable_logger(0);

function child1()
{
    for ($i = 0; $i < 3; ++$i) {
        println("child1-$i");
        if ($i == 1) {
            proton\pause();
        }
    }
}

function child2()
{
    for ($i = 0; $i < 3; ++$i) {
        println("child2-$i");
        if ($i == 1) {
            proton\pause();
        }
    }
}


function test1()
{
    println("parent-1");
    proton\go('child1');
    println("parent-2");
    proton\go('child2');
}

proton\go('test1');


println("FINISHED");
