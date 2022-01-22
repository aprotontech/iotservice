<?php

function println($s)
{
    echo $s . "\n";
}

println(extension_loaded("electron"));

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

    println("sleep 2s");
    proton\sleep(2000);
    println("sleep done");
    proton\runtime::stop();
}

proton\go('test1');

proton\runtime::start();


println("FINISHED");
