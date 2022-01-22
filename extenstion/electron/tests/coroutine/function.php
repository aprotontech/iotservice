<?php

function println($s)
{
    echo $s . "\n";
}

quark_enable_logger(0);


function test1()
{
    for ($i = 0; $i < 3; ++$i) {
        println("test1-$i");
    }
}

proton\go('test1');

proton\go(function () {
    for ($i = 0; $i < 3; ++$i) {
        println("test2-$i");
    }
});

$v1 = 3;
$v2 = 4;
$v3 = &$v1;

proton\go(function ($v1, $v3) use (&$v2) {
    for ($i = 0; $i < 3; ++$i) {
        println("test2-$v2-$i");
        ++$v1;
        ++$v3;
    }
    $v2 = 0;
}, $v1, $v3);

println("v1=$v1,v2=$v2,v3=$v3");

println("FINISHED");
